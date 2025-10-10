# Current PID Tuning Test

## Test Configuration

**Test Routine:** 48" forward + 90° turn
- **Location:** `src/main.cpp` autonomous() function (lines 184-212)
- **Auto-logging:** ✅ Enabled (file prefix: "auton")

### Test Sequence:
1. Set starting position (0, 0, 0°)
2. Drive forward 48 inches (maxSpeed=60)
3. Wait 500ms
4. Turn to 90° heading (maxSpeed=60)
5. Wait 500ms
6. Display results on LCD

## Current PID Values

**File:** `src/globals.cpp` (lines 131-154)

### Lateral PID (Forward/Backward)
```cpp
lateralPID(
    10,     // kP - Proportional gain
    0,      // kI - Integral gain
    1,      // kD - Derivative gain
    3,      // Anti-windup
    2,      // Small error (inches)
    100,    // Small error timeout (ms)
    5,      // Large error (inches)
    500,    // Large error timeout (ms)
    20      // Max acceleration
);
```

### Angular PID (Turning)
```cpp
angularPID(
    2.2,    // kP - Proportional gain
    0,      // kI - Integral gain
    10,     // kD - Derivative gain
    3,      // Anti-windup
    2,      // Small error (degrees)
    100,    // Small error timeout (ms)
    5,      // Large error (degrees)
    500,    // Large error timeout (ms)
    0       // Max acceleration (no slew on turns)
);
```

## How to Run Test

### 1. Build and Upload
```bash
make
pros upload
```

### 2. Run Autonomous
- Insert SD card
- **IMPORTANT:** If you pressed A button in driver control, press A again to close logger
- Switch to autonomous mode
- Robot will:
  - Drive forward 48"
  - Turn 90°
  - Display results on LCD

### 3. Analyze Results
```bash
./tools/analyze_and_tune.sh
```

Or manually:
```bash
python3 tools/plot_telemetry.py /Volumes/V5-DATA/auton_*.csv 48
open /Volumes/V5-DATA/telemetry_analysis.png
```

## Expected Results

### Ideal Performance:
- ✅ Forward distance: 48.0" ± 1"
- ✅ Final heading: 90.0° ± 2°
- ✅ No overshoot/oscillation
- ✅ Smooth velocity curves

### Previous Test Results (24" test):
- Overshoot: 2.44" (10.2%)
- Peak velocity: 29.66 ips
- **Recommendation:** Decrease kP from 10 → 8, increase kD from 1 → 3

## Tuning Workflow

### If Forward Movement Has Overshoot:
```cpp
// In src/globals.cpp lateralPID:
lateralPID(
    8,      // kP: Decrease from 10
    0,      // kI: Keep at 0
    3,      // kD: Increase from 1 for more damping
    // ... rest unchanged
);
```

### If Forward Movement Has Undershoot:
```cpp
lateralPID(
    12,     // kP: Increase from 10
    0,      // kI: Keep at 0
    1,      // kD: Keep or slightly decrease
    // ... rest unchanged
);
```

### If Turn Overshoots:
```cpp
angularPID(
    2.0,    // kP: Decrease from 2.2
    0,      // kI: Keep at 0
    12,     // kD: Increase from 10
    // ... rest unchanged
);
```

### If Turn Undershoots:
```cpp
angularPID(
    2.5,    // kP: Increase from 2.2
    0,      // kI: Keep at 0
    10,     // kD: Keep at 10
    // ... rest unchanged
);
```

## Files Involved

- **Test code:** `src/main.cpp` (lines 184-212)
- **PID config:** `src/globals.cpp` (lines 131-154)
- **Analysis script:** `tools/analyze_and_tune.sh`
- **Visualization:** `tools/plot_telemetry.py`
- **Log files:** `/Volumes/V5-DATA/auton_*.csv`
- **Plots:** `/Volumes/V5-DATA/telemetry_analysis.png`

## Troubleshooting

### Empty log files (0 bytes):
- **Cause:** Manual logging still active
- **Fix:** Press A button to toggle logging OFF before running autonomous

### Robot doesn't move:
- Check motors are plugged in (ports 15, 14, 16, 13)
- Verify autonomous routine is uncommented in main.cpp
- Check battery level

### No SD card detected:
- Ensure SD card is inserted
- Format as FAT32 (not exFAT)
- Use 32GB or smaller

---

**Status:** Ready to test!

**Next Step:** Upload code, run autonomous, then say "check sd" for analysis.
