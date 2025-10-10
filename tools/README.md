# Telemetry Analysis Tools

Tools for analyzing VEX V5 robot telemetry data for PID tuning.

## Prerequisites

Install required Python packages:

```bash
pip3 install pandas matplotlib numpy
```

## Quick Start

1. **Run autonomous** on your robot with auto-logging enabled
2. **Eject SD card** and insert into computer
3. **Run visualization script**:

```bash
# Basic usage (no target distance)
python3 tools/plot_telemetry.py /Volumes/V5-DATA/auton_MMDD_HHMMSS.csv

# With target distance (for error analysis)
python3 tools/plot_telemetry.py /Volumes/V5-DATA/auton_MMDD_HHMMSS.csv 24
```

Replace `auton_MMDD_HHMMSS.csv` with your actual filename.

## Example

```bash
# Analyze lateral PID test (24 inch target)
python3 tools/plot_telemetry.py /Volumes/V5-DATA/auton_1009_201819.csv 24
```

## What You'll See

The script generates 4 plots:

### 1. Position vs Time
- X position (blue)
- Y position (green)
- Total distance from origin (red dashed)
- Shows movement start/end markers

**Look for:**
- Smooth position curve (no jitter)
- Reaches target distance
- Minimal overshoot/undershoot

### 2. Velocity vs Time
- Left wheel velocity (blue)
- Right wheel velocity (green)
- Average velocity (red)

**Look for:**
- Smooth acceleration/deceleration
- Left and right velocities match (no drift)
- Peak velocity appropriate for robot

### 3. Position Error vs Time
- Distance from target over time
- Zero error line (green)
- ±1 inch tolerance bands (orange)

**Look for:**
- Final error < 1 inch
- No oscillation around target
- Smooth approach to zero

### 4. Battery Voltage vs Time
- Battery voltage throughout test
- Shows voltage drop during movement

**Look for:**
- Voltage drop < 1V (healthy battery)
- Voltage recovers after movement

## Console Output

The script also prints:

```
PID TUNING ANALYSIS
======================================================================

Target Distance:     24.00 inches
Final Distance:      26.41 inches
Overshoot:           2.41 inches (10.0%)

RECOMMENDATIONS:
⚠ Significant overshoot detected!
  → Decrease kP (proportional gain) in globals.cpp
  → Increase kD (derivative gain) for more damping

Peak Velocity:       33.15 inches/second
✓ Velocity looks reasonable

Settling Time:       4.23 seconds
✓ Settling time acceptable

======================================================================
```

## PID Tuning Workflow

### Step 1: Collect Data
Run autonomous with auto-logging:
```cpp
// In src/main.cpp autonomous()
auton.lateralPIDTest();  // or angularPIDTest()
```

### Step 2: Analyze
```bash
python3 tools/plot_telemetry.py /Volumes/V5-DATA/auton_*.csv 24
```

### Step 3: Tune PIDs in globals.cpp

**If overshoot detected:**
```cpp
// Decrease kP by 10-20%
lemlib::ControllerSettings lateralPID(
    8.0,  // kP (was 10)
    0,    // kI
    3,    // kD (increase for more damping)
    // ... rest of config
);
```

**If undershoot detected:**
```cpp
// Increase kP by 10-20%
lemlib::ControllerSettings lateralPID(
    12.0,  // kP (was 10)
    0,     // kI
    1,     // kD
    // ... rest of config
);
```

**If oscillation detected:**
```cpp
// Increase kD for damping
lemlib::ControllerSettings lateralPID(
    10,  // kP
    0,   // kI
    5,   // kD (increase to reduce oscillation)
    // ... rest of config
);
```

### Step 4: Rebuild and Test
```bash
make
pros upload
# Run autonomous again
```

### Step 5: Repeat
Continue tuning until:
- ✓ Final error < 1 inch
- ✓ No overshoot/oscillation
- ✓ Fast settling time (< 3 seconds)

## Advanced Usage

### Compare Multiple Runs

```bash
# Run 1 (before tuning)
python3 tools/plot_telemetry.py /Volumes/V5-DATA/auton_1009_201819.csv 24

# Tune PID...

# Run 2 (after tuning)
python3 tools/plot_telemetry.py /Volumes/V5-DATA/auton_1009_202345.csv 24
```

Compare the plots side-by-side to verify improvement.

### Save Plots to File

Modify the script to save instead of display:

```python
# In plot_telemetry() function, change:
plot_telemetry(df, target_distance=target_distance, output_dir='.')
```

This saves `telemetry_analysis.png` in current directory.

## Troubleshooting

### "No module named pandas"
```bash
pip3 install pandas matplotlib numpy
```

### "No significant movement detected"
- Check if robot actually moved during autonomous
- Verify motors are plugged in
- Check autonomous routine is uncommented in main.cpp

### Empty or 0-byte CSV files
- Make sure to close manual logging (press A button) before running autonomous
- Check SD card is inserted and formatted as FAT32
- Verify `#define ENABLE_AUTON_LOGGING` is enabled in globals.hpp

### File not found
```bash
# List recent files
ls -lh /Volumes/V5-DATA/*.csv | tail -5

# Use actual filename
python3 tools/plot_telemetry.py /Volumes/V5-DATA/auton_1009_201819.csv 24
```

## Files Created

- `tools/plot_telemetry.py` - Main visualization script
- `tools/README.md` - This documentation

## Telemetry Schema

CSV format (v=1):
```
v=1,t_ms,x(in),y(in),theta(deg),v_l(ips),v_r(ips),batt_V,mark
5979,0.000,0.000,0.002,-0.000,0.000,12.88,
9219,0.000,-0.002,0.002,25.611,30.188,12.88,
```

Fields:
- `t_ms` - Timestamp in milliseconds
- `x(in)` - X position in inches
- `y(in)` - Y position in inches
- `theta(deg)` - Heading in degrees
- `v_l(ips)` - Left wheel velocity (inches/second)
- `v_r(ips)` - Right wheel velocity (inches/second)
- `batt_V` - Battery voltage
- `mark` - Optional waypoint marker

---

**Team 839Y - Caution Tape**
*Generated with Claude Code*
