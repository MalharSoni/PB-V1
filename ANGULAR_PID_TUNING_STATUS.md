# Angular PID Tuning Session Status Report
**Date:** 2024-10-09
**Team:** 839Y
**Session Duration:** ~3 hours

---

## 🎯 GOAL
Tune the angular PID controller for LemLib's `turnToHeading()` function to eliminate overshoot on 90° turns.

---

## ✅ WHAT'S WORKING

### 1. **Telemetry System - Driver Control Mode**
- ✅ Manual logging with A button in driver control works perfectly
- ✅ Files save successfully after SD card was formatted
- ✅ LCD shows "FLUSHING..." → "LOG SAVED!" → controller vibrates twice
- ✅ File format: `run_0_MMDD_HHMMSS.csv`
- ✅ Queue flush system works (waits for all data to write before closing)
- ✅ Creates proper CSV files with headers: `time_ms,x,y,theta,lf_temp,lm_temp,rf_temp,rm_temp,lf_curr,lm_curr,rf_curr,rm_temp,battery_mv,velocity`

### 2. **Robot Hardware & Motion**
- ✅ Robot performs 4x 90° turns correctly in autonomous mode
- ✅ LemLib `chassis.turnToHeading()` executes turns
- ✅ Turns complete successfully (user observed them working well)
- ✅ Current PID values: **kP=2.2, kD=25** (increased from kD=15)

### 3. **Build System**
- ✅ Code compiles successfully
- ✅ `make` and `pros upload` work
- ✅ Hot/cold linking operational

### 4. **SD Card**
- ✅ SD card is writable (formatted as FAT32)
- ✅ Files can be created from computer
- ✅ Files save properly when using driver control manual logging

---

## ❌ WHAT'S NOT WORKING

### 1. **Autonomous Mode Telemetry Logging (CRITICAL ISSUE)**
- ❌ Autonomous mode ends after ~5 seconds (only captures setup, not the actual turns)
- ❌ Telemetry files created in autonomous mode contain NO turn data
- ❌ File shows theta staying at ~355° (original pose before `setPose(0,0,0)` takes effect)
- ❌ Only 100-200 samples captured over 4-6 seconds
- ❌ Autonomous function appears to be interrupted or timing out early

**Root Cause Unknown:**
- Autonomous mode may have a timeout when run without competition switch
- `disabled()` function may be called prematurely
- Autonomous function may be exiting before completing the test

### 2. **Manual Turn Logging Unreliable**
- ❌ User manually driving turns with controller produces inconsistent data
- ❌ Manual turns show 17° overshoot (vs 4° reported by user for autonomous)
- ❌ Not accurate enough for PID tuning

---

## 🔧 CURRENT CODE STATE

### Files Modified (Session Summary)

1. **`src/slog.cpp`**
   - ✅ FIXED: Added queue drain wait in `close()` function (waits up to 10 seconds)
   - ✅ FIXED: Prevents premature file closing
   - ✅ Status: **WORKING in driver control**

2. **`include/ui/runtime_controls.hpp`**
   - ✅ ADDED: Visual feedback "FLUSHING..." → "LOG SAVED!"
   - ✅ ADDED: Controller vibration on save complete
   - ✅ Status: **WORKING**

3. **`src/main.cpp`**
   - ✅ Angular PID test code added (4x 90° turns)
   - ✅ Debug printf statements added
   - ⚠️ Telemetry auto-logging disabled (`#ifdef ENABLE_AUTON_LOGGING` commented out)
   - ❌ Status: **Test works, but telemetry doesn't capture it**

4. **`src/main.cpp` - `disabled()` function**
   - ⚠️ `telem::tuning_logger_close()` commented out (to prevent premature closing)
   - ❌ Didn't solve the autonomous logging issue

5. **`src/globals.cpp`**
   - ✅ Angular PID updated: kD increased from 10 → 15 → 25
   - ✅ Tuning history documented in comments
   - **Current values:**
     ```cpp
     kP = 2.2
     kI = 0
     kD = 25  // Increased to reduce overshoot
     ```

6. **`include/globals.hpp`**
   - ⚠️ `ENABLE_AUTON_LOGGING` currently commented out (line 103)

---

## 📊 DATA COLLECTED

### Successful Manual Logging Session (Driver Control)
- **File:** `run_0_1009_232623.csv`
- **Size:** 27KB
- **Duration:** ~8 seconds of manual turns
- **Result:** Shows ~17° overshoot on 90° turns
- **Issue:** Manual turns aren't consistent enough for accurate PID tuning

### Failed Autonomous Logging Sessions
- Multiple files created: `telemetry_1009_*.csv`
- All files: 5-6KB, 4-6 seconds duration
- All show theta ~355° (no turn data captured)
- Robot DID perform turns, but telemetry closed before capturing them

---

## 🚫 WHAT NOT TO DO

### 1. **DON'T Use Gain Sweep**
- ❌ Abandoned due to file persistence issues
- ❌ Files deleted: `include/tuning/gain_sweep.hpp`, `src/tuning/gain_sweep.cpp`
- ❌ Same underlying problem as autonomous telemetry
- **Status:** Removed from codebase

### 2. **DON'T Try Manual Turns for Tuning**
- ❌ User manually driving 90° turns is unreliable
- ❌ Produces inconsistent data (17° overshoot vs 4° observed in autonomous)
- ❌ Not suitable for accurate PID tuning

### 3. **DON'T Format SD Card Again**
- ✅ SD card is already formatted and working
- ✅ Driver control logging works fine
- Only needed if autonomous logging starts working and files disappear again

### 4. **DON'T Modify File Creation Code**
- ✅ `slog.cpp` file creation works correctly
- ✅ Problem is NOT with file creation - it's with autonomous mode timing

---

## 🔍 ROOT CAUSE ANALYSIS

### Problem: Autonomous Mode Exits Early

**Evidence:**
1. Telemetry captures 4-6 seconds of data
2. Autonomous test needs 12-15 seconds (1s delay + 4 turns × ~3s each)
3. User confirms turns execute successfully
4. Telemetry shows no turn data (theta stays at ~355°)

**Hypothesis:**
- Autonomous mode may have built-in timeout when run without competition switch
- V5 brain may be limiting autonomous duration in test mode
- PROS autonomous() function may be exiting prematurely

**What Was Tried:**
- ✅ Disabled `ENABLE_AUTON_LOGGING` to prevent auto-close
- ✅ Commented out `telem::tuning_logger_close()` in `disabled()`
- ✅ Added delays before close section
- ✅ Added debug printf statements
- ❌ None of these solved the autonomous timing issue

---

## 📋 RECOMMENDED NEXT STEPS

### Option 1: Use Competition Switch (RECOMMENDED)
- Connect field control or competition switch
- Run autonomous in actual competition mode (60 second duration)
- Telemetry should capture full test

### Option 2: Investigate Autonomous Timing
- Add more printf debug statements to determine when autonomous() exits
- Check PROS documentation for autonomous mode duration limits
- Determine if `disabled()` is being called early

### Option 3: Alternative Testing Method
- Create a button-triggered test in `opcontrol()` that calls the turn sequence
- Start manual logging (A button)
- Press test trigger button (e.g., X button)
- Robot executes 4x 90° turns while logging is active
- Stop logging (A button) → saves file

### Option 4: Move Test to `initialize()`
- Run the angular PID test in `initialize()` instead of `autonomous()`
- `initialize()` has no time limits
- Start telemetry, run test, close telemetry

---

## 💾 FILES TO REFERENCE

### Working Files
- `/Volumes/V5-DATA/run_0_1009_232623.csv` - Manual turn data (17° overshoot)
- `include/ui/runtime_controls.hpp` - Working manual logging system
- `src/slog.cpp` - Fixed queue flush logic

### Failed Telemetry Files (No Turn Data)
- `/Volumes/V5-DATA/telemetry_1009_233142.csv` - 6KB, 5.3s, theta=355.9°
- `/Volumes/V5-DATA/telemetry_1009_233901.csv` - 12KB, 9.9s, theta=354.9°
- `/Volumes/V5-DATA/telemetry_1009_234148.csv` - 5KB, 4.3s, theta=354.4°
- `/Volumes/V5-DATA/telemetry_1009_234434.csv` - 6KB, 5.3s, theta=355.3°

### Analysis Tools
- `tools/rank_gains.py` - Python script for analyzing gain sweep data (unused)
- Can be adapted to analyze angular PID telemetry once data is captured

---

## 🎓 WHAT WE LEARNED

### Successful Debugging
1. ✅ SD card filesystem corruption was causing file loss → **Fixed by formatting**
2. ✅ Queue wasn't draining before close → **Fixed with wait loop in `slog::close()`**
3. ✅ No visual feedback on save completion → **Fixed with LCD messages and vibration**

### Outstanding Issue
1. ❌ Autonomous mode duration/timing prevents full test capture
2. ❌ Unable to get telemetry data from LemLib `turnToHeading()` in autonomous

### Key Insight
**Manual logging in driver control works perfectly.** The telemetry system itself is reliable. The problem is specifically with autonomous mode ending prematurely or being interrupted.

---

## 🔄 STATE FOR NEXT SESSION

### Current Configuration
- `ENABLE_AUTON_LOGGING` is **disabled** (commented out in `globals.hpp`)
- Angular PID: kP=2.2, kD=25
- Telemetry manual logging fully functional
- SD card formatted and working
- Angular PID test code ready in `autonomous()`

### To Resume Tuning
1. Re-enable `ENABLE_AUTON_LOGGING` in `globals.hpp` (uncomment line 103)
2. Try Option 1 (competition switch) or Option 3 (button-triggered test)
3. Upload code and test
4. Analyze theta data from telemetry CSV
5. Adjust kD based on overshoot measurements

---

## 📞 CONTEXT FOR NEXT CLAUDE SESSION

**Quick Summary:**
"We're tuning angular PID for 90° turns. The robot performs turns correctly with kP=2.2, kD=25, but telemetry logging in autonomous mode only captures 5 seconds before the autonomous function exits (needs 12-15 seconds). Manual logging in driver control works perfectly. Need to either use competition switch for full autonomous duration, or create a button-triggered test in driver control that executes the turn sequence while manual logging is active."

**First Action:**
Read this file, then implement Option 3 (button-triggered test in opcontrol) as the fastest path to getting telemetry data.

---

*End of Report*
