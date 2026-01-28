#include "cmd_status.h"

#include "grbl/hal.h"
#include "grbl/grbl.h"
#include "grbl/planner.h"
#include "grbl/settings.h"
#include "grbl/system.h"

#include <stdio.h>   // necessario per snprintf

#define CMD_STATUS_SETTING_ID      450
#define CMD_STATUS_RECONSTRUCT_ID  451

static bool cmd_status_enabled = true;
static bool cmd_status_reconstruct = false;

static on_realtime_report_ptr prev_on_realtime_report = NULL;

// Memorizza l’ultimo motion mode valido
static motion_mode_t last_motion_mode = MotionMode_None;

// Motion mode → G-code
static const char *motion_mode_to_gcode (motion_mode_t mode)
{
    switch(mode) {
        case MotionMode_Seek:                     return "G0";
        case MotionMode_Linear:                   return "G1";
        case MotionMode_CwArc:                    return "G2";
        case MotionMode_CcwArc:                   return "G3";

        case MotionMode_ProbeToward:
        case MotionMode_ProbeTowardNoError:
        case MotionMode_ProbeAway:
        case MotionMode_ProbeAwayNoError:         return "G38";

        case MotionMode_SpindleSynchronized:      return "G33";
        case MotionMode_Threading:                return "G76";

        case MotionMode_CubicSpline:              return "G5";
        case MotionMode_QuadraticSpline:          return "G5.1";

        case MotionMode_CannedCycle81:            return "G81";
        case MotionMode_CannedCycle82:            return "G82";
        case MotionMode_CannedCycle83:            return "G83";
        case MotionMode_CannedCycle84:            return "G84";
        case MotionMode_CannedCycle85:            return "G85";
        case MotionMode_CannedCycle86:            return "G86";
        case MotionMode_CannedCycle89:            return "G89";

        case MotionMode_DrillChipBreak:           return "G73";

        default:                                  return "G?";
    }
}

static bool is_known_motion_mode (motion_mode_t mode)
{
    switch(mode) {
        case MotionMode_Seek:
        case MotionMode_Linear:
        case MotionMode_CwArc:
        case MotionMode_CcwArc:
        case MotionMode_ProbeToward:
        case MotionMode_ProbeTowardNoError:
        case MotionMode_ProbeAway:
        case MotionMode_ProbeAwayNoError:
        case MotionMode_SpindleSynchronized:
        case MotionMode_Threading:
        case MotionMode_CubicSpline:
        case MotionMode_QuadraticSpline:
        case MotionMode_CannedCycle81:
        case MotionMode_CannedCycle82:
        case MotionMode_CannedCycle83:
        case MotionMode_CannedCycle84:
        case MotionMode_CannedCycle85:
        case MotionMode_CannedCycle86:
        case MotionMode_CannedCycle89:
        case MotionMode_DrillChipBreak:
            return true;

        default:
            return false;
    }
}

static void append_axis(stream_write_ptr stream_write, char axis, float value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), " %c%.3f", axis, value);
    stream_write(buf);
}

static void cmd_status_on_realtime_report (stream_write_ptr stream_write,
                                           report_tracking_flags_t report)
{
    if(!cmd_status_enabled)
        goto exit;

    plan_block_t *block = plan_get_current_block();

    if(block) {

        // Aggiorna motion mode valido
        if(block->motion_mode != MotionMode_None &&
           is_known_motion_mode(block->motion_mode))
            last_motion_mode = block->motion_mode;

        stream_write("|Cmd:");
        stream_write(motion_mode_to_gcode(last_motion_mode));

        // Se $451=1 → mostra assi + feed
        if(cmd_status_reconstruct) {

            append_axis(stream_write, 'X', block->target_mm[X_AXIS]);
            append_axis(stream_write, 'Y', block->target_mm[Y_AXIS]);
            append_axis(stream_write, 'Z', block->target_mm[Z_AXIS]);

            char buf[32];
            snprintf(buf, sizeof(buf), " F%.1f", block->programmed_rate);
            stream_write(buf);
        }
    }

exit:
    if(prev_on_realtime_report)
        prev_on_realtime_report(stream_write, report);
}

// Settings
static setting_detail_t cmd_status_settings[] = {
    { CMD_STATUS_SETTING_ID,     Group_Root, "CmdStatus", NULL, Format_Bool, NULL, NULL, NULL, Setting_NonCore, &cmd_status_enabled, NULL, NULL },
    { CMD_STATUS_RECONSTRUCT_ID, Group_Root, "CmdReconstruct", NULL, Format_Bool, NULL, NULL, NULL, Setting_NonCore, &cmd_status_reconstruct, NULL, NULL }
};

static setting_details_t cmd_status_settings_info = {
    .settings = cmd_status_settings,
    .n_settings = sizeof(cmd_status_settings) / sizeof(setting_detail_t),
    .descriptions = NULL,
    .n_descriptions = 0,
    .save = NULL,
    .load = NULL,
    .restore = NULL
};

void cmd_status_init(void)
{
    settings_register(&cmd_status_settings_info);

    prev_on_realtime_report = grbl.on_realtime_report;
    grbl.on_realtime_report = cmd_status_on_realtime_report;
}
