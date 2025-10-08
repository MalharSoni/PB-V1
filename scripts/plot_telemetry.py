#!/usr/bin/env python3
"""
Telemetry Analysis Script for VEX V5 Robotics
Team 839Y - Caution Tape

Reads telemetry CSV files from SD card and generates comprehensive analysis plots.

Usage:
    python plot_telemetry.py telemetry.csv
    python plot_telemetry.py /path/to/telemetry_MMDD_HHMMSS.csv
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def plot_telemetry(csv_file):
    """
    Generate comprehensive telemetry analysis plots

    Args:
        csv_file: Path to telemetry CSV file
    """
    # Read CSV file
    print(f"Reading telemetry data from: {csv_file}")
    df = pd.read_csv(csv_file)

    # Convert time from milliseconds to seconds
    df['time_s'] = df['time_ms'] / 1000.0

    # Convert battery from millivolts to volts
    df['battery_v'] = df['battery_mv'] / 1000.0

    # Create figure with 2x3 grid of subplots
    fig, axes = plt.subplots(2, 3, figsize=(18, 10))
    fig.suptitle('VEX V5 Robot Telemetry Analysis - Team 839Y', fontsize=16, fontweight='bold')

    # ========================================================================
    # Plot 1: Robot Path (X-Y Position)
    # ========================================================================
    ax = axes[0, 0]
    ax.plot(df['x'], df['y'], 'b-', linewidth=2, label='Path')
    ax.plot(df['x'].iloc[0], df['y'].iloc[0], 'go', markersize=10, label='Start')
    ax.plot(df['x'].iloc[-1], df['y'].iloc[-1], 'ro', markersize=10, label='End')
    ax.set_xlabel('X Position (inches)', fontsize=10)
    ax.set_ylabel('Y Position (inches)', fontsize=10)
    ax.set_title('Robot Path on Field', fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend()
    ax.axis('equal')

    # Add position error text
    start_x, start_y = df['x'].iloc[0], df['y'].iloc[0]
    end_x, end_y = df['x'].iloc[-1], df['y'].iloc[-1]
    position_error = np.sqrt((end_x - start_x)**2 + (end_y - start_y)**2)
    ax.text(0.02, 0.98, f'Position Error: {position_error:.2f}"',
            transform=ax.transAxes, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

    # ========================================================================
    # Plot 2: Heading Over Time
    # ========================================================================
    ax = axes[0, 1]
    ax.plot(df['time_s'], df['theta'], 'r-', linewidth=2)
    ax.set_xlabel('Time (seconds)', fontsize=10)
    ax.set_ylabel('Heading (degrees)', fontsize=10)
    ax.set_title('Heading Over Time', fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.axhline(y=0, color='k', linestyle='--', alpha=0.3, label='Target (0°)')

    # Add heading drift text
    heading_drift = df['theta'].iloc[-1] - df['theta'].iloc[0]
    ax.text(0.02, 0.98, f'Total Drift: {heading_drift:.2f}°',
            transform=ax.transAxes, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

    # ========================================================================
    # Plot 3: Motor Temperatures
    # ========================================================================
    ax = axes[0, 2]
    ax.plot(df['time_s'], df['lf_temp'], label='L15 (Front)', linewidth=2)
    ax.plot(df['time_s'], df['lm_temp'], label='L14 (Mid)', linewidth=2)
    ax.plot(df['time_s'], df['rf_temp'], label='R16 (Front)', linewidth=2)
    ax.plot(df['time_s'], df['rm_temp'], label='R13 (Mid)', linewidth=2)
    ax.axhline(y=55, color='orange', linestyle='--', alpha=0.5, label='Warning (55°C)')
    ax.axhline(y=60, color='red', linestyle='--', alpha=0.5, label='Critical (60°C)')
    ax.set_xlabel('Time (seconds)', fontsize=10)
    ax.set_ylabel('Temperature (°C)', fontsize=10)
    ax.set_title('Motor Temperatures', fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend(loc='best', fontsize=8)

    # ========================================================================
    # Plot 4: Motor Currents
    # ========================================================================
    ax = axes[1, 0]
    ax.plot(df['time_s'], df['lf_curr'], label='L15 (Front)', linewidth=2)
    ax.plot(df['time_s'], df['lm_curr'], label='L14 (Mid)', linewidth=2)
    ax.plot(df['time_s'], df['rf_curr'], label='R16 (Front)', linewidth=2)
    ax.plot(df['time_s'], df['rm_curr'], label='R13 (Mid)', linewidth=2)
    ax.set_xlabel('Time (seconds)', fontsize=10)
    ax.set_ylabel('Current (mA)', fontsize=10)
    ax.set_title('Motor Current Draw', fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend(loc='best', fontsize=8)

    # ========================================================================
    # Plot 5: Battery Voltage
    # ========================================================================
    ax = axes[1, 1]
    ax.plot(df['time_s'], df['battery_v'], 'g-', linewidth=2)
    ax.axhline(y=11.5, color='orange', linestyle='--', alpha=0.5, label='Warning (11.5V)')
    ax.axhline(y=11.0, color='red', linestyle='--', alpha=0.5, label='Critical (11.0V)')
    ax.set_xlabel('Time (seconds)', fontsize=10)
    ax.set_ylabel('Battery Voltage (V)', fontsize=10)
    ax.set_title('Battery Voltage', fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend(loc='best')

    # Calculate voltage drop
    voltage_drop = df['battery_v'].iloc[0] - df['battery_v'].iloc[-1]
    ax.text(0.02, 0.98, f'Voltage Drop: {voltage_drop:.2f}V',
            transform=ax.transAxes, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

    # ========================================================================
    # Plot 6: Robot Velocity
    # ========================================================================
    ax = axes[1, 2]
    ax.plot(df['time_s'], df['velocity'], 'purple', linewidth=2)
    ax.set_xlabel('Time (seconds)', fontsize=10)
    ax.set_ylabel('Velocity (RPM avg)', fontsize=10)
    ax.set_title('Robot Velocity', fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)

    # ========================================================================
    # Save and Display
    # ========================================================================
    plt.tight_layout()

    # Generate output filename
    output_file = csv_file.replace('.csv', '_analysis.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Analysis plot saved to: {output_file}")

    # Print summary statistics
    print("\n" + "="*60)
    print("TELEMETRY SUMMARY")
    print("="*60)
    print(f"Duration: {df['time_s'].iloc[-1]:.2f} seconds")
    print(f"Start Position: X={start_x:.2f}\", Y={start_y:.2f}\", H={df['theta'].iloc[0]:.2f}°")
    print(f"End Position:   X={end_x:.2f}\", Y={end_y:.2f}\", H={df['theta'].iloc[-1]:.2f}°")
    print(f"Position Error: {position_error:.2f}\"")
    print(f"Heading Drift:  {heading_drift:.2f}°")
    print(f"\nMax Motor Temps: L15={df['lf_temp'].max():.1f}°C, L14={df['lm_temp'].max():.1f}°C, "
          f"R16={df['rf_temp'].max():.1f}°C, R13={df['rm_temp'].max():.1f}°C")
    print(f"Battery Start:   {df['battery_v'].iloc[0]:.2f}V")
    print(f"Battery End:     {df['battery_v'].iloc[-1]:.2f}V")
    print(f"Voltage Drop:    {voltage_drop:.2f}V")
    print("="*60 + "\n")

    # Show plot
    plt.show()

def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("Usage: python plot_telemetry.py <telemetry_file.csv>")
        print("\nExample:")
        print("  python plot_telemetry.py /usd/telemetry.csv")
        print("  python plot_telemetry.py telemetry_1215_103045.csv")
        sys.exit(1)

    csv_file = sys.argv[1]

    if not os.path.exists(csv_file):
        print(f"Error: File not found: {csv_file}")
        sys.exit(1)

    plot_telemetry(csv_file)

if __name__ == "__main__":
    main()
