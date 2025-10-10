# Telemetry CSV Schema (v1)

## File Format

- **Path**: `/usd/<hint>_MMDD_HHMMSS.csv`
- **Encoding**: ASCII
- **Line ending**: `\n`
- **Max line length**: 200 bytes

## Header

```csv
v=1,t_ms,x(in),y(in),theta(deg),v_l(ips),v_r(ips),batt_V,mark
```

## Column Definitions

| Column | Type | Unit | Description |
|--------|------|------|-------------|
| `v` | int | - | Schema version (always 1) |
| `t_ms` | int | milliseconds | Timestamp since logger init |
| `x(in)` | float | inches | X position (lateral) |
| `y(in)` | float | inches | Y position (forward/backward) |
| `theta(deg)` | float | degrees | Heading (0° = along +X axis, CCW positive) |
| `v_l(ips)` | float | inches/sec | Left wheel velocity |
| `v_r(ips)` | float | inches/sec | Right wheel velocity |
| `batt_V` | float | volts | Battery voltage |
| `mark` | string | - | Marker/waypoint tag (empty or `MARK:tag=value`) |

## Data Row Examples

### Normal row (no marker)
```csv
1250,12.345,24.678,45.123,36.500,36.450,12.45,
```

### Marker row (waypoint)
```csv
2500,15.234,28.912,47.250,0.000,0.000,12.42,MARK:wp=1
```

## Marker Format

Markers are single-line tags inserted at specific events:

- **Waypoint**: `MARK:wp=<N>` (N = sequential number)
- **Custom**: `MARK:<custom_tag>=<value>`

Markers appear only once (on the row where the event occurred), then the field is empty for subsequent rows.

## Precision

- **Position** (x, y, theta): 3 decimal places (0.001)
- **Velocity** (v_l, v_r): 3 decimal places (0.001 ips)
- **Battery**: 2 decimal places (0.01 V)
- **Time**: Integer (1 ms)

## Sampling Rate

Default: **100 Hz** (10 ms per sample)

Configurable rates:
- 100 Hz (divisor=1)
- 50 Hz (divisor=2)
- 25 Hz (divisor=4)
- 10 Hz (divisor=10)

## File Size Estimates

| Duration | Rate | Rows | Size |
|----------|------|------|------|
| 15 sec | 100 Hz | 1,500 | ~180 KB |
| 1 min | 50 Hz | 3,000 | ~360 KB |
| 2 min | 50 Hz | 6,000 | ~720 KB |

**Note**: Actual size depends on number of markers and data precision.

## Analysis Tools

### Excel/Google Sheets
1. Import CSV
2. Plot `t_ms` vs `y(in)` for position profile
3. Plot `t_ms` vs `v_l(ips)` for velocity profile
4. Identify overshoot, settling time

### Python (pandas)
```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('/path/to/telemetry.csv')
plt.plot(df['t_ms'], df['y(in)'])
plt.xlabel('Time (ms)')
plt.ylabel('Y Position (inches)')
plt.title('Lateral Movement Profile')
plt.show()
```

## Version History

- **v1** (2025-01-09): Initial schema with position, velocity, battery, markers
