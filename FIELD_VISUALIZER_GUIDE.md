# VEX Field Visualizer - GUI Simulation

**Visual autonomous route testing** - See your robot move on a 144" × 144" field in real-time!

![Field Visualizer Preview](https://via.placeholder.com/800x800/228B22/FFFFFF?text=VEX+Field+Visualizer)

## Features

✅ **Real-time visualization** - Watch robot move as simulation runs
✅ **Colored VEX field** - Green field with 24" grid lines
✅ **Robot sprite** - Moves and rotates, shows heading with arrow
✅ **Path tracing** - Blue line shows where robot has traveled
✅ **Live telemetry** - Position, heading, velocity displayed on screen
✅ **Playback controls** - Pause, restart, speed up/slow down
✅ **Replay mode** - Play back recorded autonomous runs
✅ **Customizable** - Change robot color, playback speed

## Installation

### 1. Install Python Dependencies

```bash
pip3 install pygame pandas numpy
```

### 2. Make Script Executable

```bash
chmod +x scripts/field_visualizer.py
```

## Usage

### Mode 1: Real-Time Visualization

Watch robot move **as simulation runs**:

```bash
# Step 1: Enable simulation in src/main.cpp
# (uncomment simulation.setEnabled(true))

# Step 2: Build and upload to robot
make && pros upload

# Step 3: Start visualizer (in separate terminal)
python3 scripts/field_visualizer.py --live /usd/telemetry.csv

# Step 4: Run autonomous on robot
# Watch robot move in Python window!
```

**What you'll see:**
- Empty green field appears
- When autonomous starts, robot sprite appears and moves
- Blue path trace follows robot
- Position updates in real-time at bottom of screen

### Mode 2: Replay Recorded Run

Play back **previously recorded** autonomous:

```bash
# After autonomous has run and created CSV file
python3 scripts/field_visualizer.py --replay /usd/telemetry_1008_143022.csv
```

**What you'll see:**
- Autonomous runs from start to finish
- Loops back to beginning when done
- Can pause/speed up/slow down

## Controls

While visualizer is running:

| Key | Action |
|-----|--------|
| **SPACE** | Pause/Resume |
| **R** | Restart (go back to beginning) |
| **+** or **=** | Speed up playback (1.5x) |
| **-** | Slow down playback (0.67x) |
| **Q** or **ESC** | Quit |

## Examples

### Example 1: Watch 48" Drive Test

```bash
# In src/main.cpp, enable simulation:
simulation.setEnabled(true);
simulation.reset();

# In autonomous(), run:
auton.odomDriveTest();  # Drives 48" forward

# Start visualizer:
python3 scripts/field_visualizer.py --live /usd/telemetry.csv

# Upload and run autonomous
# You'll see robot move from center to 48" forward
```

### Example 2: Replay with Custom Speed

```bash
# Play back at 2x speed
python3 scripts/field_visualizer.py --replay /usd/telemetry.csv --speed 2.0

# Play in slow motion (0.5x)
python3 scripts/field_visualizer.py --replay /usd/telemetry.csv --speed 0.5
```

### Example 3: Change Robot Color

```bash
# Blue robot
python3 scripts/field_visualizer.py --replay /usd/telemetry.csv --robot-color blue

# Red robot
python3 scripts/field_visualizer.py --replay /usd/telemetry.csv --robot-color red

# Available: red, blue, green, orange, yellow, purple
```

### Example 4: Compare Two Routes

```bash
# Run route 1
auton.route1();
# Save: /usd/telemetry_route1.csv

# Run route 2
auton.route2();
# Save: /usd/telemetry_route2.csv

# Replay both and compare visually
python3 scripts/field_visualizer.py --replay /usd/telemetry_route1.csv --robot-color red
python3 scripts/field_visualizer.py --replay /usd/telemetry_route2.csv --robot-color blue
```

## What You'll See

### Field Display

```
┌─────────────────────────────────────────┐
│         VEX Field (144" × 144")         │
│                                         │
│    .    .    .    .    .    .    .     │
│                                         │
│    .    .    .    .    .    .    .     │
│                                         │
│    .    .    .   +🤖   .    .    .     │ ← Robot at center
│                  │                      │
│    .    .    .   └─→ Heading arrow    │
│                                         │
│    .    .    .    .    .    .    .     │
│                                         │
│  ═══════════ Path trace ═══════════    │
│                                         │
└─────────────────────────────────────────┘
┌─────────────────────────────────────────┐
│ X: 24.1"  Y: -12.3"  θ: 45.2°          │
│ Time: 3.45s  |  Velocity: 52.3 in/s    │
│ Status: REPLAY (2.0x)  |  Frame: 67/150│
│ SPACE: Pause | R: Restart | +/-: Speed │
└─────────────────────────────────────────┘
```

### Field Features

- **Green background** - VEX field mat
- **Grid lines** - 24" spacing (matches VEX tiles)
- **Center cross** - Field origin (0, 0)
- **White border** - Field perimeter

### Robot Sprite

- **Square outline** - Represents 18" × 18" robot
- **Arrow** - Shows which direction robot is facing
- **Center dot** - Robot's exact position
- **Color** - Customizable (default orange)

### Path Trace

- **Blue line** - Shows everywhere robot has been
- **Updated continuously** - Draws as robot moves
- **Limited history** - Last 1000 points to prevent lag

## Troubleshooting

### "File not found" Error

**Problem:**
```
[ERROR] Telemetry file not found: /usd/telemetry.csv
```

**Solution:**
1. Make sure simulation is enabled: `simulation.setEnabled(true)`
2. Run autonomous to create CSV file
3. Check file path is correct (use actual filename if timestamped)

### Visualizer Shows Empty Field

**Problem:** Window opens but robot doesn't appear

**Causes & Solutions:**

1. **CSV file is empty**
   - Check autonomous actually ran
   - Verify telemetry.init() was called

2. **Robot position is off-screen**
   - Check CSV data: `cat /usd/telemetry.csv | head`
   - Position should be within -72 to +72 inches

3. **Wrong CSV file**
   - Use actual filename: `ls /usd/telemetry*.csv`
   - File might be timestamped

### Robot Moves Too Fast/Slow

**Solution:** Adjust playback speed

```bash
# Slow down
python3 scripts/field_visualizer.py --replay file.csv --speed 0.5

# Speed up
python3 scripts/field_visualizer.py --replay file.csv --speed 3.0

# Or use +/- keys while running
```

### Path Doesn't Match Expectations

**This is useful!** It means:
- Route planning has errors
- Robot drifts during movement
- Odometry calibration needs tuning
- PID values need adjustment

Use the visualization to **debug these issues**.

## Advanced Usage

### Extract Specific Timeframe

```bash
# View only first 5 seconds
python3 -c "
import pandas as pd
df = pd.read_csv('/usd/telemetry.csv')
df[df['time_ms'] <= 5000].to_csv('first_5_sec.csv', index=False)
"

python3 scripts/field_visualizer.py --replay first_5_sec.csv
```

### Screenshot Robot Path

While visualizer is running:
1. Pause with SPACE
2. Take screenshot (OS screenshot tool)
3. Use for documentation or team presentations

### Overlay Multiple Paths (Manual)

1. Run autonomous route 1
2. Take screenshot of path
3. Run autonomous route 2
4. Compare screenshots side-by-side

(Automatic overlay feature could be added in future)

### Export Animation (Future Feature)

Currently not implemented, but could add:
- Record frames to video file
- Export as GIF
- Create MP4 of autonomous run

## Integration with Workflow

### Development Workflow

```
1. Write autonomous in src/auton.cpp
   ↓
2. Enable simulation + telemetry
   ↓
3. Build and upload
   ↓
4. Start field visualizer
   ↓
5. Run autonomous
   ↓
6. Watch robot move on screen
   ↓
7. See path is wrong? → Fix code → Repeat
```

### Competition Prep Workflow

```
1. Record autonomous run at practice field
   ↓
2. Copy CSV to laptop
   ↓
3. Replay with visualizer at home
   ↓
4. Show team: "This is what robot will do"
   ↓
5. Identify improvements
   ↓
6. Code new route
   ↓
7. Test in simulation
   ↓
8. Bring to practice → Test on real robot
```

## Technical Details

### Coordinate System

**Field Coordinates (CSV data):**
- Origin at field center
- X-axis: -72" (left) to +72" (right)
- Y-axis: -72" (bottom) to +72" (top)
- Heading: 0° = right, 90° = up, 180° = left, 270° = down

**Screen Coordinates (pygame):**
- Origin at top-left corner
- X increases right
- Y increases down
- Converted automatically by `field_to_screen()`

### Performance

- **Frame rate:** 60 FPS
- **Update rate:** Reads CSV every 50ms (matches telemetry)
- **Path history:** Stores last 1000 points
- **CPU usage:** Low (~5-10% on modern laptop)

### File Format

Expects telemetry CSV with these columns:
```
time_ms,x,y,theta,lf_temp,lm_temp,rf_temp,rm_temp,lf_curr,lm_curr,rf_curr,rm_curr,battery_mv,velocity
```

Minimum required: `time_ms, x, y, theta`

### Scaling

- Field: 144" × 144" → 800 × 800 pixels
- Scale: 800 / 144 = 5.56 pixels per inch
- Robot: 18" × 18" → 100 × 100 pixels

## Limitations

❌ **Does not simulate:**
- Collisions with walls or other robots
- Game elements (balls, goals, obstacles)
- Intake/arm movements
- Real-time physics (just playback)

✅ **Only shows:**
- Chassis position and path
- Heading/rotation
- Telemetry data

## Future Enhancements

Potential features to add:

- [ ] Multiple robot colors (overlay multiple runs)
- [ ] Game element overlay (load field image)
- [ ] Export video/GIF of autonomous
- [ ] Zoom and pan controls
- [ ] Measurement tool (click two points to measure distance)
- [ ] Waypoint markers (show target points)
- [ ] Velocity vector visualization
- [ ] Compare two CSV files side-by-side
- [ ] 3D view (future)

## Examples in Practice

### Debug Turning Error

**Problem:** Robot ends up at wrong angle

**Solution:**
```bash
python3 scripts/field_visualizer.py --replay /usd/telemetry.csv
# Watch heading arrow
# See robot over-rotates by 5°
# → Fix angular PID in globals.cpp
```

### Verify Path Planning

**Problem:** Does autonomous route make sense?

**Solution:**
```bash
# Run simulation
auton.myNewRoute();

# Visualize
python3 scripts/field_visualizer.py --replay /usd/telemetry.csv

# Check:
# ✓ Does robot stay on field?
# ✓ Are turns smooth?
# ✓ Does path avoid obstacles?
# ✓ Is route efficient?
```

### Compare Before/After PID Tuning

**Before tuning:**
```bash
auton.driveTest();
# Save as telemetry_before.csv
python3 scripts/field_visualizer.py --replay telemetry_before.csv
# Path is wiggly
```

**After tuning:**
```bash
# Tune PID in globals.cpp
auton.driveTest();
# Save as telemetry_after.csv
python3 scripts/field_visualizer.py --replay telemetry_after.csv
# Path is straight!
```

---

## Quick Reference

**Install:**
```bash
pip3 install pygame pandas numpy
```

**Real-time:**
```bash
python3 scripts/field_visualizer.py --live /usd/telemetry.csv
```

**Replay:**
```bash
python3 scripts/field_visualizer.py --replay /usd/telemetry_1008_143022.csv
```

**Options:**
```bash
--speed 2.0           # 2x playback speed
--robot-color blue    # Blue robot
```

**Controls:** SPACE=Pause, R=Restart, +/-=Speed, Q=Quit

---

*Have fun visualizing your autonomous routes! 🤖*
