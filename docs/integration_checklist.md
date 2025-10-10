# Telemetry Integration Checklist

## ✅ Pre-Integration (Done!)

- [x] Core logger (`slog.hpp/.cpp`)
- [x] LemLib adapter (`telemetry_adapter.hpp`)
- [x] CSV formatter (`telemetry_stream.hpp`)
- [x] Orchestration (`tuning_logger.hpp`)
- [x] Runtime controls (`runtime_controls.hpp`)
- [x] Main app integration (`main.cpp`)

## 🔧 Hardware Setup

### SD Card Requirements
- [ ] **Class 10 or better** SD card (recommended: SanDisk Extreme 32GB)
- [ ] **FAT32 formatted** (V5 brain requirement)
- [ ] **Inserted at boot time** (before `initialize()` runs)
- [ ] **Not write-protected** (check physical switch)

### Verification
1. Insert SD card into V5 brain
2. Power on robot
3. Check brain screen for "LOG OFF" message
4. Press controller **A button** → should show "LOG ON 100Hz"
5. Check SD card → should have file `/usd/run_0_MMDD_HHMMSS.csv`

## 🎮 Controller Button Mapping

**CHANGED**: Telemetry now uses A/B/X/Y buttons

| Button | Old Function | New Function |
|--------|--------------|--------------|
| **A** | Wall angle align | **Toggle logging on/off** |
| **B** | *(unused)* | **Cycle rate (100/50/25/10 Hz)** |
| **X** | Wall distance align | **Set waypoint marker** |
| **Y** | Clamp | **Rotate log file** |
| **Down** | *(unused)* | Clamp (moved from Y) |
| **Left** | *(unused)* | Wall angle align (moved from A) |
| **Up** | *(unused)* | Wall distance align (moved from X) |

**If you need old button mapping**, edit `src/main.cpp` opcontrol() section.

## 📋 Usage Workflow

### Basic Logging (Driver Control)
1. **Power on robot** with SD card inserted
2. **Press A** to start logging
3. **Drive robot** (data captured in background)
4. **Press A** to stop logging
5. **Disable robot** (flushes data to SD)
6. **Remove SD card** and analyze CSV

### Advanced Features
- **Press B** to change logging rate (50 Hz recommended for long sessions)
- **Press X** to tag waypoints in data (e.g., at specific game objectives)
- **Press Y** to start a new log file (without stopping robot)

### Autonomous Logging
1. **Press A** to start logging **before** autonomous starts
2. **Run autonomous routine**
3. **Robot disabled** → automatically closes log file
4. **Retrieve SD card** for analysis

## 🧪 Testing Procedure

### Smoke Test (Quick Verification)
1. Insert SD card
2. Power on robot
3. Press **A** button (start logging)
4. Wait 5 seconds
5. Press **A** button (stop logging)
6. Disable robot
7. Remove SD card
8. Check file exists: `/usd/run_0_*.csv`
9. Verify ~500 lines (5 sec × 100 Hz)

### Motion Test (Full Verification)
1. Start logging (press A)
2. Drive robot forward 24 inches
3. Stop logging (press A)
4. Check CSV file:
   - ✅ Position changes from ~0 → ~24 inches
   - ✅ Velocity shows acceleration → constant → deceleration
   - ✅ No dropped samples (`Drops: 0` on LCD)
   - ✅ Smooth data (no jumps)

## ⚠️ Common Issues

See [troubleshooting.md](troubleshooting.md) for detailed solutions.

**Quick fixes:**
- **Empty file**: SD card not inserted at boot → reboot with card inserted
- **No logging**: Press A button to enable
- **Stuttering robot**: Lower rate to 50 Hz (press B)
- **Dropped samples**: Use Class 10 SD card

## 🔬 PID Tuning Workflow

1. **Record baseline**: Log lateral PID test, note final error
2. **Adjust kP/kI/kD** in `src/globals.cpp`
3. **Rebuild** and upload code
4. **Log new test**
5. **Compare CSVs**: Look for reduced overshoot, faster settling
6. **Iterate** until satisfied

## 📊 Data Analysis

### Excel
1. Open CSV in Excel
2. Insert → Chart → Scatter
3. X-axis: `t_ms`, Y-axis: `y(in)`
4. Analyze overshoot and settling time

### Python
```python
import pandas as pd
import matplotlib.pyplot as plt

# Load data
df = pd.read_csv('/path/to/telemetry.csv')

# Plot position
plt.figure(figsize=(12, 6))
plt.subplot(2, 1, 1)
plt.plot(df['t_ms'], df['y(in)'])
plt.ylabel('Y Position (in)')
plt.title('Lateral Movement')
plt.grid(True)

# Plot velocity
plt.subplot(2, 1, 2)
plt.plot(df['t_ms'], df['v_l(ips)'], label='Left')
plt.plot(df['t_ms'], df['v_r(ips)'], label='Right')
plt.xlabel('Time (ms)')
plt.ylabel('Velocity (ips)')
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.show()
```

## ✅ Acceptance Criteria

- [ ] Can toggle logging with A button
- [ ] CSV file appears on SD card
- [ ] Data logged **during** motion (not just before/after)
- [ ] No dropped samples after 2 minutes at 50 Hz
- [ ] Robot drives smoothly (no stuttering)
- [ ] Velocity profile shows smooth S-curve
- [ ] Position data matches actual robot movement

## 🚀 Ready to Use!

Once all checkboxes are complete, the telemetry system is ready for PID tuning and autonomous development.
