# Telemetry Troubleshooting Guide

## 🔍 Diagnosis: Logs Only Before/After Movement

**Symptoms:**
- CSV file exists but only has ~10 samples
- No data logged during robot motion
- Position frozen at same value
- Velocity always 0.0

**Root Cause:** Telemetry task is being starved by blocking motion loops.

**Solutions:**

### 1. Verify Task Priority
Check `src/main.cpp::initialize()`:
```cpp
pros::Task tuning_telem_task(tuning_telemetry_task, nullptr,
                             TASK_PRIORITY_DEFAULT + 1,  // MUST be +1!
                             TASK_STACK_DEPTH_DEFAULT,
                             "Tuning Telemetry");
```

If priority is not `+1`, telemetry will be starved.

### 2. Check Motion Loops for Blocking
**❌ BAD** (blocks telemetry):
```cpp
while (!chassis->isSettled()) {
    // No delay - starves telemetry!
}
```

**✅ GOOD** (yields to telemetry):
```cpp
while (!chassis->isSettled()) {
    pros::delay(10);  // Always yield!
}
```

### 3. Verify Logging is Active
- Press **A button** to enable logging
- Check LCD line 0: Should show "LOG ON 100Hz"
- If shows "LOG OFF", press A again

---

## 📁 Diagnosis: Empty CSV File

**Symptoms:**
- File exists on SD card
- File size is 0 bytes
- No header row

**Root Causes & Solutions:**

### 1. SD Card Not Inserted at Boot
**Fix:**
- Power off robot
- Insert SD card
- Power on robot
- Check for "LOG OFF" message on LCD

### 2. Wrong Path
The logger uses `/usd/` path (VEX V5 standard).

**Verify:**
```cpp
// In slog.cpp
snprintf(filename, sizeof(filename), "/usd/%s_...", hint);
```

### 3. File Not Closed
**Fix:** Always disable robot after logging to trigger `disabled()`:
```cpp
void disabled() {
    telemetry::tuning_logger_close();  // Flushes data
}
```

---

## 💥 Diagnosis: Dropped Samples

**Symptoms:**
- LCD shows "Drops: >0"
- CSV has gaps in timestamps
- Missing data during fast movements

**Root Causes & Solutions:**

### 1. Slow SD Card
**Fix:** Use **Class 10** or better SD card (SanDisk Extreme recommended).

**Check:** Look at SD card label for speed class:
- ❌ Class 4, Class 6 → Too slow
- ✅ Class 10, U1, U3 → Good
- ✅ V30, V60, V90 → Excellent

### 2. Queue Overflow
**Fix:** Increase ring buffer size in `include/slog.hpp`:
```cpp
constexpr int QUEUE_SLOTS = 1024;  // Was 512
```

**Trade-off:** Uses more RAM (1024 × 200 bytes = 200 KB).

### 3. Logging Rate Too High
**Fix:** Reduce rate to 50 Hz:
- Press **B button** during operation
- Or set default in code:
```cpp
// In tuning_logger.hpp
inline int& log_div() {
    static int divisor = 2;  // 50 Hz instead of 1 (100 Hz)
    return divisor;
}
```

---

## 🐌 Diagnosis: Robot Stuttering

**Symptoms:**
- Robot moves jerkily
- Motion is not smooth
- High CPU usage

**Root Causes & Solutions:**

### 1. Writer Task Priority Too High
**Fix:** Ensure writer task is **below** motion priority:
```cpp
// In slog.cpp
writer_task = new pros::Task(writer_task_fn, nullptr,
                            TASK_PRIORITY_DEFAULT - 1,  // MUST be -1!
                            ...);
```

### 2. Logging Rate Too High
**Fix:** Reduce to 50 Hz or 25 Hz (press B button).

### 3. Telemetry Task Too Slow
**Check:** Verify telemetry loop has 10ms delay:
```cpp
// In main.cpp
void tuning_telemetry_task(void* param) {
    while (true) {
        telemetry::tuning_logger_tick();
        pros::delay(10);  // MUST be present!
    }
}
```

---

## ❓ Diagnosis: LCD Shows "LOG FAILED!"

**Symptoms:**
- Press A button
- LCD shows "LOG FAILED!" instead of "LOG ON"

**Root Causes & Solutions:**

### 1. SD Card Not Inserted
**Fix:** Insert SD card and reboot robot.

### 2. SD Card Full
**Fix:**
- Remove old files from SD card
- Or use new SD card

**Check:** SD card capacity:
- Minimum: 1 GB
- Recommended: 8-32 GB

### 3. SD Card Write-Protected
**Fix:** Check physical write-protect switch on SD card (slide to unlock position).

---

## 🔄 Diagnosis: File Not Rotating

**Symptoms:**
- Press Y button
- LCD shows "ROTATE FAILED!"
- OR new file not created

**Solutions:**

### 1. Not Logging
**Fix:** Start logging first (press A), then rotate (press Y).

### 2. SD Card Full
**Fix:** Delete old files or use new SD card.

---

## 🎯 Diagnosis: Markers Not Working

**Symptoms:**
- Press X button
- No "MARK:wp=N" in CSV file
- OR markers appear on wrong rows

**Solutions:**

### 1. Marker Cleared Too Fast
Markers are single-use and clear after one read.

**Expected behavior:**
```csv
1250,12.345,24.678,45.123,36.500,36.450,12.45,MARK:wp=1
1260,12.456,24.789,45.234,35.000,35.100,12.44,
```

### 2. Logging Rate Too Low
If logging at 10 Hz, marker might be missed between samples.

**Fix:** Use 50 Hz or 100 Hz when using markers.

---

## 📊 Diagnosis: Position Not Changing

**Symptoms:**
- Robot moves physically
- CSV shows same X/Y/theta for all rows
- Velocity is 0.0

**Root Causes & Solutions:**

### 1. Odometry Not Calibrated
**Check:** `src/robot_config.cpp::initialize()`:
```cpp
chassis.calibrate();  // MUST be called!
```

**Fix:** Ensure `robot_config::initialize()` is called before telemetry starts.

### 2. Tracking Wheels Not Connected
**Check:**
- Rotation sensors plugged in (ports 7, 8)
- Tracking wheels touching ground
- No mechanical binding

**Test:** Manually push robot → check LCD line 5 for sensor readings:
```
L:<changes>  R:<changes>
```

### 3. LemLib Not Running
**Check:** Background odometry task should be started by `chassis.calibrate()`.

**Verify:** In autonomous or opcontrol, call:
```cpp
lemlib::Pose p = chassis.getPose();
pros::lcd::print(0, "X:%.1f Y:%.1f", p.x, p.y);
```

If position doesn't change, LemLib is not updating.

---

## 🧰 Quick Diagnostic Commands

### Check SD Card Status
```cpp
// In initialize() or opcontrol()
FILE* test = fopen("/usd/test.txt", "w");
if (test != nullptr) {
    fprintf(test, "SD card working\n");
    fclose(test);
    pros::lcd::print(0, "SD: OK");
} else {
    pros::lcd::print(0, "SD: FAIL");
}
```

### Check Telemetry Stats
Press **A** button to start logging, then check LCD:
```
L0: LOG ON 100Hz
L1: Q: 45/512 Hi: 128
L2: Drops: 0 Lines: 1234
```

**Good signs:**
- Queue depth (Q) stays below 200
- Drops = 0
- Lines increasing

**Bad signs:**
- Queue depth near capacity (512)
- Drops > 0
- Lines not increasing

---

## 🆘 Last Resort: Factory Reset

If nothing works:

1. **Delete all telemetry code** from `main.cpp`
2. **Rebuild** and upload
3. **Test** basic robot operation
4. **Re-add** telemetry one file at a time
5. **Test** after each addition

---

## 📞 Getting Help

If still stuck:

1. **Check SD card** → Try different card
2. **Check build logs** → Look for compile warnings
3. **Check console** → Run `pros terminal` to see printf output
4. **Reduce complexity** → Test with minimal code first

**Include in bug report:**
- SD card brand/model/class
- Exact error message from LCD
- Console output (`pros terminal`)
- CSV file (if created)
- Relevant code snippets
