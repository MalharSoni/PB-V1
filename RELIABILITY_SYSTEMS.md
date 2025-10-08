# 🛡️ Reliability Systems Documentation

## Overview

This codebase includes production-grade reliability systems for competition-ready autonomous routines:

1. **Telemetry Logger** - CSV logging to SD card for post-match analysis
2. **Controller Alert System** - Real-time warnings for driver awareness
3. **Python Analysis Tools** - Comprehensive telemetry visualization

---

## 🔧 Telemetry Logger

### What It Does
- Logs robot state to CSV file on SD card every 50ms
- Captures: position (x, y, theta), motor temps, motor currents, battery voltage, velocity
- Automatically generates timestamped filenames
- Zero impact on performance (background task)

### Usage

**Autonomous:**
```cpp
void autonomous() {
    telemetry.init();  // Start logging

    // Your autonomous code here
    auton.odomDriveTest();

    telemetry.close();  // Stop logging
}
```

**Driver Control:**
```cpp
void opcontrol() {
    telemetry.init();  // Start logging

    // Background task automatically logs every 50ms
    pros::Task telemetry_bg(telemetry_task, nullptr, "Telemetry");

    // Your driver control code
    while (true) {
        // ...
    }

    telemetry.close();  // Stop logging at end
}
```

### Output Files
Files are saved to SD card as: `/usd/telemetry_MMDD_HHMMSS.csv`

Example:
- `/usd/telemetry_1215_103045.csv` = December 15, 10:30:45 AM

### CSV Format
```csv
time_ms,x,y,theta,lf_temp,lm_temp,rf_temp,rm_temp,lf_curr,lm_curr,rf_curr,rm_curr,battery_mv,velocity
0,0.00,0.00,0.0,32.0,32.5,31.8,32.2,0,0,0,0,12650,0.0
50,0.12,0.05,0.1,32.1,32.6,31.9,32.3,1250,1230,1270,1260,12648,45.2
100,0.45,0.18,0.2,32.3,32.8,32.1,32.5,1280,1250,1290,1275,12645,87.5
...
```

---

## 🚨 Controller Alert System

### What It Monitors

**Motor Health:**
- ✅ Temperature warnings (>55°C = WARNING, >60°C = CRITICAL)
- ✅ Disconnection detection (zero current + zero temp = motor unplugged)
- ✅ Individual motor alerts (L15, L14, R16, R13, INT1, INT2, INT3)

**Battery:**
- ✅ Low voltage warning (<11.5V = WARNING, <11.0V = CRITICAL)

**IMU:**
- ✅ IMU error detection
- ✅ Calibration status monitoring

**Odometry:**
- ✅ Drift timer (warns if >15 seconds without wall alignment reset)

### Alert Priorities

**INFO** - No rumble, informational only
- IMU calibrating

**WARNING** - Single rumble pulse `.`
- Motor temperature >55°C
- Battery <11.5V
- Odometry drift >15s

**CRITICAL** - Triple rumble pulse `...`
- Motor temperature >60°C
- Motor disconnected
- Battery <11.0V
- IMU error

### Usage

**Automatic** - Just call `alerts.check()` periodically:
```cpp
void opcontrol() {
    alerts.resetDriftTimer();  // Reset on startup

    // Background task checks every 100ms
    pros::Task alerts_bg(alerts_task, nullptr, "Alerts");

    while (true) {
        // Your code...

        // Reset drift timer after wall alignment
        if (master.get_digital_new_press(DIGITAL_A)) {
            distanceAlign.calculateAngleOneWall(0.0);
            alerts.resetDriftTimer();  // <-- Important!
        }
    }
}
```

### Controller Display
Alerts appear on controller screen (line 0):
```
L15 HOT         (Motor warning)
BAT LOW!        (Battery warning)
R13 DISC!       (Motor disconnected)
IMU ERROR!      (Critical IMU failure)
ODOM DRIFT      (Need wall alignment)
```

---

## 📊 Python Analysis Tools

### Installation

```bash
# Install required packages
pip install pandas matplotlib numpy
```

### Usage

```bash
# Basic usage
python scripts/plot_telemetry.py /path/to/telemetry.csv

# Example with SD card mounted
python scripts/plot_telemetry.py /usd/telemetry_1215_103045.csv

# On macOS (SD card auto-mount)
python scripts/plot_telemetry.py /Volumes/NO\ NAME/telemetry.csv
```

### Output

**Generated Plot:** `telemetry_MMDD_HHMMSS_analysis.png` (6-panel analysis)

**Panels:**
1. **Robot Path** - X-Y trajectory with start/end markers, position error
2. **Heading Over Time** - Heading drift analysis
3. **Motor Temperatures** - All 4 drive motors, warning/critical thresholds
4. **Motor Currents** - Current draw for each motor (detects stuck motors)
5. **Battery Voltage** - Voltage drop over time
6. **Robot Velocity** - Average motor RPM

**Terminal Output:**
```
TELEMETRY SUMMARY
============================================================
Duration: 8.75 seconds
Start Position: X=0.00", Y=0.00", H=0.00°
End Position:   X=-0.37", Y=48.98", H=4.90°
Position Error: 48.99"
Heading Drift:  4.90°

Max Motor Temps: L15=45.0°C, L14=45.0°C, R16=40.0°C, R13=43.0°C
Battery Start:   12.65V
Battery End:     12.58V
Voltage Drop:    0.07V
============================================================
```

---

## 🎓 Competition Workflow

### Pre-Match Checklist
1. ✅ Check SD card is inserted (lights should show on brain)
2. ✅ Verify IMU calibration completes without timeout
3. ✅ Run motor diagnostics to check all motors connected
4. ✅ Charge battery to >12.0V

### During Match
1. **Watch Controller Display** - Alerts will show critical issues
2. **Listen for Rumble** - Triple pulse = critical problem
3. **Take Action** - If motor overheats, reduce usage or swap battery

### Post-Match Analysis
1. **Remove SD card** from V5 Brain
2. **Copy telemetry CSV** to computer
3. **Run analysis script**:
   ```bash
   python scripts/plot_telemetry.py telemetry_MMDD_HHMMSS.csv
   ```
4. **Review plots** - Look for:
   - Position accuracy issues (path plot)
   - Heading drift patterns (heading plot)
   - Motor overheating (temperature plot)
   - Stuck motors (current spikes)
   - Battery health (voltage drop)

### Iterative Improvement
1. Identify issues in telemetry
2. Tune PID constants or motion paths
3. Test again
4. Compare telemetry before/after
5. Repeat until autonomous is reliable

---

## 🐛 Troubleshooting

### Telemetry Not Logging

**Problem:** No CSV file created on SD card

**Solutions:**
- Check SD card is inserted and formatted (FAT32)
- Verify SD card has space (<2GB cards work best)
- Check console for "[Telemetry] ERROR: Failed to open log file"
- Try formatting SD card and reinserting

---

### Alerts Not Showing

**Problem:** No alerts on controller screen

**Solutions:**
- Verify alerts background task is started: `pros::Task alerts_bg(alerts_task, ...)`
- Check motors are actually connected and reading temps
- Confirm `alerts.check()` is being called periodically
- Check console for "[Alert WARN/CRIT] ..." messages

---

### Python Script Errors

**Problem:** `ModuleNotFoundError: No module named 'pandas'`

**Solution:**
```bash
pip install pandas matplotlib numpy
```

**Problem:** `FileNotFoundError: [Errno 2] No such file or directory`

**Solution:**
- Verify correct path to CSV file
- Use absolute path: `/full/path/to/telemetry.csv`
- Check SD card is mounted (macOS: `/Volumes/NO NAME/`)

---

## 📈 Advanced Usage

### Custom Telemetry Fields

Edit `src/subsystems/telemetry.cpp` to add custom data:

```cpp
void Telemetry::log() {
    // ... existing code ...

    // Add custom field
    float custom_value = mySubsystem.getValue();
    fprintf(logfile, ",%.2f", custom_value);
}
```

Update header in `init()`:
```cpp
fprintf(logfile, "time_ms,x,y,theta,...,custom_field\n");
```

---

### Custom Alert Conditions

Edit `src/subsystems/alerts.cpp` to add custom alerts:

```cpp
void Alerts::check() {
    // ... existing checks ...

    // Custom condition
    if (mySubsystem.isStuck()) {
        show("SUBSYS STUCK", CRITICAL);
    }
}
```

---

## 🎯 Performance Impact

**Telemetry:**
- CPU: <1% (background task at 50ms interval)
- Memory: ~1KB for file buffer
- Disk: ~100 bytes per log entry (~200KB per 60s match)

**Alerts:**
- CPU: <1% (background task at 100ms interval)
- Memory: ~100 bytes
- No disk usage

**Total Impact:** Negligible - safe for competition use

---

## 📚 Next Steps

Now that you have telemetry and alerts:

1. **Run baseline tests** - Collect telemetry on current autonomous
2. **Identify bottlenecks** - Find where drift/errors occur
3. **Implement fixes** - Tune PID, add wall alignment, improve paths
4. **Verify improvements** - Compare telemetry before/after
5. **Iterate** - Repeat until autonomous is competition-ready

**Coming Soon:**
- Sensor fusion (weighted odometry + distance sensors + GPS)
- IMU drift characterization and compensation
- Stuck detection with auto-recovery
- Enhanced wall alignment with median filtering

---

*Team 839Y - Caution Tape*
*"Measure everything, optimize systematically"*
