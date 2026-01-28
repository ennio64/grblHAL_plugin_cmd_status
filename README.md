# cmd_status plugin for grblHAL

**cmd_status** is a lightweight, non-invasive plugin for **grblHAL** that adds an extra field to the status report, indicating the G-code command currently being executed.

This plugin queries the **grblHAL planner** directly to show what the machine is **physically executing** at that exact moment.

---

## 🚀 Features

* **Real-time Feedback:** Adds the `|Cmd:...|` tag to the status report `<...>` only during the `RUN` state.
* **Motion Mode Detection:** Correctly identifies `G0`, `G1`, `G2`, `G3`, `G38`, `G33`, `G76`, `G5/5.1`, and canned cycles (`G73`, `G81–G89`).
* **Extended Command Reconstruction (Experimental):** Can reconstruct target coordinates and programmed feed rate (`$451=1`).
* **Zero Overhead:** Does not interfere with motor timing or critical planner logic.

---

## 📊 Output Examples

**Standard Mode (Default):**
> `<Run|WPos:10.000,5.000,0.000|Bf:15,128|FS:1000,8000|Cmd:G1>`

**Extended Mode ($451=1):**
> `<Run|WPos:1.713,0.000,-10.000|Bf:15,128|FS:200,8000|Cmd:G1 X1.713 Z-10.000 F200.0>`

---

## 🛠 Installation

> [!IMPORTANT]  
> Requires **grblHAL version 20260126** or higher.

### 1. Copy Files
Copy the `cmd_status/` folder into your grblHAL project's root directory:

```text
your_grblhal_project/
├── cmd_status/           
│   ├── plugin_cmd_status.c
│   ├── plugin_cmd_status.h
│   ├── library.json
│   └── CMakeLists.txt
├── grbl/
├── boards/
└── platformio.ini (or main CMakeLists.txt)
```

### 2. Activate in Core (plugins_init.h)
Open grbl/plugins_init.h and add the plugin initialization:
```text
#if CMD_STATUS_ENABLE
    extern void cmd_status_init (void);
    cmd_status_init();
#endif
```

### 3. Build Configuration
Using PlatformIO (platformio.ini)
Add the folder to your library dependencies and enable the necessary build flags:
```text
[common]
lib_deps =
    # ... other libs ...
    cmd_status

[env:your_board]
build_flags = 
    ${common.build_flags} 
    -D CMD_STATUS_ENABLE=1
    -D PLANNER_ADD_MOTION_MODE=1
```

---

## ⚙️ Settings ($)

| Setting | Description             | Values                     | Default |
|--------|--------------------------|-----------------------------|---------|
| $450   | Enable/Disable Plugin    | 0 = Off, 1 = On             | 1       |
| $451   | Extended Reconstruction  | 0 = Simple, 1 = Extended | 0       |


### ⚠️ Reconstruction Details & Limitations ($451=1)

In extended mode, the plugin reconstructs the specific motion pass currently being executed.

G0 / G1: Full and accurate reconstruction of target and feedrate.

G2 / G3: Displays Target (X, Y, Z) and Feed (F). Parameters I, J, K (arc centers) are not preserved in the planner block and cannot be reconstructed.

G33 (Synced Motion): 

	During cutting passes, F represents the K (pitch) parameter (e.g., F1.5 = 1.5mm pitch).

G76 (Threading Cycle):

	During cutting passes, F represents the thread P (pitch) parameter (e.g., F0.5 = 0.5mm pitch).
	During retract/positioning, F represents the actual Feedrate (rapid movement).

Canned Cycles (G81-G89): (Not tested).

---

## 📜 License

Free for personal and professional use.
