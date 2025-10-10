#!/usr/bin/env python3
"""
VEX V5 Telemetry Data Visualizer
Plots position, velocity, and other telemetry data for PID tuning analysis.

Usage:
    python3 plot_telemetry.py <path_to_csv_file>
    python3 plot_telemetry.py /Volumes/V5-DATA/auton_1009_201819.csv

Features:
- Position vs Time (X, Y, distance from origin)
- Velocity vs Time (left, right, average)
- Position Error vs Time (for target distance)
- Battery Voltage vs Time
- Automatic detection of movement phases
"""

import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

def load_telemetry(filepath):
    """Load telemetry CSV file and parse data."""
    try:
        df = pd.read_csv(filepath)

        # Rename columns if needed (handle different formats)
        column_map = {
            't_ms': 'time_ms',
            'x(in)': 'x',
            'y(in)': 'y',
            'theta(deg)': 'theta',
            'v_l(ips)': 'v_left',
            'v_r(ips)': 'v_right',
            'batt_V': 'battery'
        }

        df = df.rename(columns=column_map)

        # Convert time to seconds
        df['time_s'] = df['time_ms'] / 1000.0

        # Calculate total distance from origin
        df['distance'] = np.sqrt(df['x']**2 + df['y']**2)

        # Calculate average velocity
        df['v_avg'] = (df['v_left'] + df['v_right']) / 2.0

        return df

    except Exception as e:
        print(f"Error loading file: {e}")
        sys.exit(1)

def detect_movement_phase(df, velocity_threshold=1.0):
    """Detect when robot starts and stops moving."""
    # Find indices where velocity is above threshold
    moving = df['v_avg'].abs() > velocity_threshold

    if not moving.any():
        print("Warning: No significant movement detected")
        return 0, len(df) - 1

    # Find start and end of movement
    start_idx = moving.idxmax()  # First True

    # Find last movement
    moving_indices = df[moving].index
    end_idx = moving_indices[-1] if len(moving_indices) > 0 else len(df) - 1

    return start_idx, end_idx

def plot_telemetry(df, output_dir=None, target_distance=None, filepath=None):
    """Generate telemetry plots for PID tuning."""

    # Detect movement phase
    start_idx, end_idx = detect_movement_phase(df)
    start_time = df.loc[start_idx, 'time_s']
    end_time = df.loc[end_idx, 'time_s']

    print(f"Movement detected: {start_time:.2f}s to {end_time:.2f}s")
    print(f"Total distance traveled: {df['distance'].iloc[-1]:.2f} inches")
    print(f"Peak velocity: {df['v_avg'].max():.2f} ips")
    print(f"Final position: X={df['x'].iloc[-1]:.2f}, Y={df['y'].iloc[-1]:.2f}, θ={df['theta'].iloc[-1]:.2f}°")

    # Create figure with subplots
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle('VEX V5 Telemetry Analysis - PID Tuning', fontsize=16, fontweight='bold')

    # ========================================================================
    # PLOT 1: Position vs Time
    # ========================================================================
    ax1 = axes[0, 0]
    ax1.plot(df['time_s'], df['x'], label='X position', color='blue', linewidth=1.5)
    ax1.plot(df['time_s'], df['y'], label='Y position', color='green', linewidth=1.5)
    ax1.plot(df['time_s'], df['distance'], label='Total distance', color='red', linewidth=2, linestyle='--')

    # Mark movement phase
    ax1.axvline(start_time, color='gray', linestyle=':', alpha=0.5, label='Movement start')
    ax1.axvline(end_time, color='gray', linestyle='-.', alpha=0.5, label='Movement end')

    # Mark target if provided
    if target_distance is not None:
        ax1.axhline(target_distance, color='orange', linestyle='--', alpha=0.7, label=f'Target: {target_distance}" ')

    ax1.set_xlabel('Time (seconds)', fontweight='bold')
    ax1.set_ylabel('Position (inches)', fontweight='bold')
    ax1.set_title('Position vs Time', fontweight='bold')
    ax1.legend(loc='best')
    ax1.grid(True, alpha=0.3)

    # ========================================================================
    # PLOT 2: Velocity vs Time
    # ========================================================================
    ax2 = axes[0, 1]
    ax2.plot(df['time_s'], df['v_left'], label='Left velocity', color='blue', alpha=0.6, linewidth=1)
    ax2.plot(df['time_s'], df['v_right'], label='Right velocity', color='green', alpha=0.6, linewidth=1)
    ax2.plot(df['time_s'], df['v_avg'], label='Average velocity', color='red', linewidth=2)

    # Mark movement phase
    ax2.axvline(start_time, color='gray', linestyle=':', alpha=0.5)
    ax2.axvline(end_time, color='gray', linestyle='-.', alpha=0.5)

    ax2.set_xlabel('Time (seconds)', fontweight='bold')
    ax2.set_ylabel('Velocity (inches/second)', fontweight='bold')
    ax2.set_title('Velocity vs Time', fontweight='bold')
    ax2.legend(loc='best')
    ax2.grid(True, alpha=0.3)

    # ========================================================================
    # PLOT 3: Position Error vs Time (if target provided)
    # ========================================================================
    ax3 = axes[1, 0]

    if target_distance is not None:
        # Calculate error from target
        df['error'] = target_distance - df['distance']

        ax3.plot(df['time_s'], df['error'], color='purple', linewidth=2)
        ax3.axhline(0, color='green', linestyle='--', alpha=0.7, label='Zero error')
        ax3.axhline(1, color='orange', linestyle=':', alpha=0.5, label='±1" tolerance')
        ax3.axhline(-1, color='orange', linestyle=':', alpha=0.5)

        # Mark movement phase
        ax3.axvline(start_time, color='gray', linestyle=':', alpha=0.5)
        ax3.axvline(end_time, color='gray', linestyle='-.', alpha=0.5)

        # Calculate final error
        final_error = df['error'].iloc[-1]
        ax3.text(0.02, 0.98, f'Final Error: {final_error:.2f}"',
                transform=ax3.transAxes, fontsize=10, verticalalignment='top',
                bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

        ax3.set_xlabel('Time (seconds)', fontweight='bold')
        ax3.set_ylabel('Position Error (inches)', fontweight='bold')
        ax3.set_title(f'Position Error vs Time (Target: {target_distance}")', fontweight='bold')
        ax3.legend(loc='best')
        ax3.grid(True, alpha=0.3)
    else:
        # No target - show theta instead
        ax3.plot(df['time_s'], df['theta'], color='purple', linewidth=2)
        ax3.axhline(0, color='green', linestyle='--', alpha=0.7, label='Zero heading')

        # Mark movement phase
        ax3.axvline(start_time, color='gray', linestyle=':', alpha=0.5)
        ax3.axvline(end_time, color='gray', linestyle='-.', alpha=0.5)

        ax3.set_xlabel('Time (seconds)', fontweight='bold')
        ax3.set_ylabel('Heading (degrees)', fontweight='bold')
        ax3.set_title('Heading vs Time', fontweight='bold')
        ax3.legend(loc='best')
        ax3.grid(True, alpha=0.3)

    # ========================================================================
    # PLOT 4: Battery Voltage vs Time
    # ========================================================================
    ax4 = axes[1, 1]
    ax4.plot(df['time_s'], df['battery'], color='darkblue', linewidth=2)
    ax4.axhline(12.0, color='orange', linestyle='--', alpha=0.7, label='12V nominal')

    # Mark movement phase
    ax4.axvline(start_time, color='gray', linestyle=':', alpha=0.5)
    ax4.axvline(end_time, color='gray', linestyle='-.', alpha=0.5)

    # Show voltage drop during movement
    start_voltage = df.loc[start_idx, 'battery']
    min_voltage = df['battery'].min()
    voltage_drop = start_voltage - min_voltage

    ax4.text(0.02, 0.02, f'Voltage drop: {voltage_drop:.2f}V',
            transform=ax4.transAxes, fontsize=10, verticalalignment='bottom',
            bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.5))

    ax4.set_xlabel('Time (seconds)', fontweight='bold')
    ax4.set_ylabel('Battery Voltage (V)', fontweight='bold')
    ax4.set_title('Battery Voltage vs Time', fontweight='bold')
    ax4.legend(loc='best')
    ax4.grid(True, alpha=0.3)

    plt.tight_layout()

    # Save plot to current directory by default
    if output_dir is None:
        output_dir = os.path.dirname(filepath) or '.'

    output_path = Path(output_dir) / 'telemetry_analysis.png'
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    print(f"\n✓ Plot saved to: {output_path}")

def print_pid_analysis(df, target_distance=None):
    """Print PID tuning recommendations based on telemetry data."""

    print("\n" + "="*70)
    print("PID TUNING ANALYSIS")
    print("="*70)

    # Calculate metrics
    final_distance = df['distance'].iloc[-1]
    max_velocity = df['v_avg'].max()

    # Detect overshoot
    if target_distance is not None:
        overshoot = final_distance - target_distance
        percent_overshoot = (overshoot / target_distance) * 100

        print(f"\nTarget Distance:     {target_distance:.2f} inches")
        print(f"Final Distance:      {final_distance:.2f} inches")
        print(f"Overshoot:           {overshoot:.2f} inches ({percent_overshoot:.1f}%)")

        # Recommendations
        print("\nRECOMMENDATIONS:")
        if abs(overshoot) < 0.5:
            print("✓ Excellent accuracy! PID is well-tuned.")
        elif overshoot > 1.0:
            print("⚠ Significant overshoot detected!")
            print("  → Decrease kP (proportional gain) in globals.cpp")
            print("  → Increase kD (derivative gain) for more damping")
        elif overshoot < -1.0:
            print("⚠ Undershoot detected!")
            print("  → Increase kP (proportional gain) in globals.cpp")
            print("  → May need to increase timeout if robot is too slow")
        else:
            print("✓ Acceptable accuracy (within 1 inch)")

    # Velocity analysis
    print(f"\nPeak Velocity:       {max_velocity:.2f} inches/second")

    if max_velocity < 20:
        print("⚠ Low peak velocity - robot may be too slow")
        print("  → Increase maxSpeed parameter in moveToPoint()")
        print("  → Check motor gearing and wheel size")
    elif max_velocity > 60:
        print("⚠ Very high velocity - may cause instability")
        print("  → Decrease maxSpeed parameter")
    else:
        print("✓ Velocity looks reasonable")

    # Settling time
    start_idx, end_idx = detect_movement_phase(df)
    settling_time = df.loc[end_idx, 'time_s'] - df.loc[start_idx, 'time_s']
    print(f"\nSettling Time:       {settling_time:.2f} seconds")

    if settling_time > 5.0:
        print("⚠ Slow settling time")
        print("  → Consider increasing kP for faster response")
        print("  → Check for mechanical friction")

    print("\n" + "="*70)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 plot_telemetry.py <csv_file> [target_distance]")
        print("\nExample:")
        print("  python3 plot_telemetry.py /Volumes/V5-DATA/auton_1009_201819.csv")
        print("  python3 plot_telemetry.py /Volumes/V5-DATA/auton_1009_201819.csv 24")
        sys.exit(1)

    # Parse arguments
    filepath = sys.argv[1]
    target_distance = float(sys.argv[2]) if len(sys.argv) > 2 else None

    # Check file exists
    if not os.path.exists(filepath):
        print(f"Error: File not found: {filepath}")
        sys.exit(1)

    print(f"Loading telemetry from: {filepath}")

    # Load data
    df = load_telemetry(filepath)

    print(f"Loaded {len(df)} samples")
    print(f"Duration: {df['time_s'].iloc[-1]:.2f} seconds")
    print(f"Sample rate: {len(df) / df['time_s'].iloc[-1]:.1f} Hz")

    # Print analysis
    print_pid_analysis(df, target_distance)

    # Plot data
    plot_telemetry(df, target_distance=target_distance, filepath=filepath)

if __name__ == '__main__':
    main()
