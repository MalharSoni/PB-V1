#!/usr/bin/env python3
"""
Automated PID Gain Ranking Tool

Analyzes gain sweep CSV data and ranks PD combinations by performance metrics.

Usage:
    python tools/rank_gains.py                    # Auto-find latest CSV
    python tools/rank_gains.py path/to/sweep.csv  # Specific file

Metrics:
    1. Settling time (lower = better)
    2. Peak overshoot (lower = better)
    3. Steady-state error (lower = better)
    4. Velocity reversals (0 = best, smooth motion)
    5. Overall smoothness score

Output:
    - Ranked table of top 10 gain sets
    - Recommended kP, kD values
    - Individual plots for top 3 performers
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys
from pathlib import Path
from typing import Tuple, Dict, List

# Performance weights (tune these if needed)
WEIGHTS = {
    'settling_time': 0.3,
    'overshoot': 0.3,
    'steady_state_error': 0.2,
    'velocity_reversals': 0.15,
    'smoothness': 0.05
}

def find_latest_csv(directory: Path = Path("/Volumes/V5-DATA")) -> Path:
    """Find most recent gain_sweep CSV file."""
    csv_files = list(directory.glob("gain_sweep*.csv"))
    if not csv_files:
        raise FileNotFoundError(f"No gain_sweep*.csv files found in {directory}")

    latest = max(csv_files, key=lambda p: p.stat().st_mtime)
    print(f"📁 Found: {latest.name}")
    return latest

def load_sweep_data(filepath: Path) -> pd.DataFrame:
    """Load and validate CSV data."""
    df = pd.read_csv(filepath)

    required_cols = ['time_ms', 'kP', 'kD', 'position', 'error', 'motor_power']
    if not all(col in df.columns for col in required_cols):
        raise ValueError(f"Missing required columns. Expected: {required_cols}")

    print(f"✓ Loaded {len(df)} samples")
    return df

def calculate_metrics(group: pd.DataFrame, target_distance: float = 24.0) -> Dict[str, float]:
    """Calculate performance metrics for a single gain set."""

    # 1. Settling time (time to reach and stay within 2% of target)
    tolerance = 0.02 * target_distance  # 2% = 0.48 inches
    settled_mask = np.abs(group['error']) < tolerance

    if settled_mask.any():
        settling_idx = settled_mask.idxmax()
        settling_time = group.loc[settling_idx, 'time_ms']
    else:
        settling_time = 2000  # Never settled = max penalty

    # 2. Peak overshoot
    max_position = group['position'].max()
    overshoot = max(0, max_position - target_distance)

    # 3. Steady-state error (average error in last 500ms)
    final_period = group[group['time_ms'] >= 1500]
    steady_state_error = np.abs(final_period['error']).mean() if len(final_period) > 0 else 999

    # 4. Velocity reversals (motor power sign changes = rough motion)
    motor_signs = np.sign(group['motor_power'])
    sign_changes = np.sum(np.diff(motor_signs) != 0)
    velocity_reversals = sign_changes

    # 5. Smoothness (motor power variance - lower = smoother)
    smoothness = group['motor_power'].std()

    # 6. Final position error
    final_error = np.abs(group['error'].iloc[-1])

    return {
        'settling_time_ms': settling_time,
        'overshoot_in': overshoot,
        'steady_state_error_in': steady_state_error,
        'velocity_reversals': velocity_reversals,
        'smoothness': smoothness,
        'final_error_in': final_error
    }

def normalize_metrics(df: pd.DataFrame) -> pd.DataFrame:
    """Normalize metrics to 0-1 scale for scoring."""
    df_norm = df.copy()

    # Lower is better for all metrics - normalize to 0-1
    for col in ['settling_time_ms', 'overshoot_in', 'steady_state_error_in',
                'velocity_reversals', 'smoothness', 'final_error_in']:
        min_val = df[col].min()
        max_val = df[col].max()

        if max_val > min_val:
            df_norm[col + '_norm'] = (df[col] - min_val) / (max_val - min_val)
        else:
            df_norm[col + '_norm'] = 0.0

    return df_norm

def calculate_scores(df: pd.DataFrame) -> pd.DataFrame:
    """Calculate weighted performance score (lower = better)."""
    df['score'] = (
        WEIGHTS['settling_time'] * df['settling_time_ms_norm'] +
        WEIGHTS['overshoot'] * df['overshoot_in_norm'] +
        WEIGHTS['steady_state_error'] * df['steady_state_error_in_norm'] +
        WEIGHTS['velocity_reversals'] * df['velocity_reversals_norm'] +
        WEIGHTS['smoothness'] * df['smoothness_norm']
    )

    return df.sort_values('score')

def plot_top_performers(df_full: pd.DataFrame, top_gains: pd.DataFrame,
                        output_dir: Path, target_distance: float = 24.0):
    """Plot position vs time for top 3 gain sets."""

    fig, axes = plt.subplots(3, 1, figsize=(12, 10))
    fig.suptitle('Top 3 Gain Sets - Position vs Time', fontsize=16, fontweight='bold')

    for i, (idx, row) in enumerate(top_gains.head(3).iterrows()):
        kP, kD = row['kP'], row['kD']

        # Filter data for this gain set
        mask = (df_full['kP'] == kP) & (df_full['kD'] == kD)
        data = df_full[mask].copy()

        ax = axes[i]

        # Plot position
        ax.plot(data['time_ms'], data['position'], 'b-', linewidth=2, label='Position')
        ax.axhline(y=target_distance, color='g', linestyle='--', label='Target', linewidth=1)

        # Shade overshoot region
        overshoot_region = data[data['position'] > target_distance]
        if len(overshoot_region) > 0:
            ax.fill_between(overshoot_region['time_ms'],
                           overshoot_region['position'],
                           target_distance,
                           alpha=0.3, color='red', label='Overshoot')

        # Annotations
        ax.set_title(f"#{i+1}: kP={kP:.1f} kD={kD:.1f} | Score={row['score']:.3f} | "
                    f"Overshoot={row['overshoot_in']:.2f}\" | Settle={row['settling_time_ms']:.0f}ms",
                    fontsize=12, fontweight='bold')
        ax.set_ylabel('Position (inches)', fontsize=10)
        ax.grid(True, alpha=0.3)
        ax.legend(loc='lower right')
        ax.set_ylim(0, max(30, data['position'].max() * 1.1))

    axes[-1].set_xlabel('Time (ms)', fontsize=10)

    plt.tight_layout()

    output_file = output_dir / "top_3_gains.png"
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"📊 Saved: {output_file}")

    plt.close()

def main():
    # Parse arguments
    if len(sys.argv) > 1:
        filepath = Path(sys.argv[1])
    else:
        filepath = find_latest_csv()

    if not filepath.exists():
        print(f"❌ File not found: {filepath}")
        sys.exit(1)

    # Load data
    print(f"\n🔍 Analyzing: {filepath.name}\n")
    df = load_sweep_data(filepath)

    # Calculate metrics for each gain set
    gain_groups = df.groupby(['kP', 'kD'])

    metrics_list = []
    for (kP, kD), group in gain_groups:
        metrics = calculate_metrics(group, target_distance=24.0)
        metrics['kP'] = kP
        metrics['kD'] = kD
        metrics_list.append(metrics)

    df_metrics = pd.DataFrame(metrics_list)

    # Normalize and score
    df_metrics = normalize_metrics(df_metrics)
    df_metrics = calculate_scores(df_metrics)

    # Display rankings
    print("=" * 100)
    print(" " * 35 + "🏆 GAIN RANKING RESULTS 🏆")
    print("=" * 100)
    print(f"\n{'Rank':<6}{'kP':<8}{'kD':<8}{'Score':<10}{'Overshoot':<12}{'Settle (ms)':<14}"
          f"{'SS Error':<12}{'Reversals':<12}{'Final Err':<12}")
    print("-" * 100)

    for i, (idx, row) in enumerate(df_metrics.head(10).iterrows(), 1):
        print(f"{i:<6}{row['kP']:<8.1f}{row['kD']:<8.1f}{row['score']:<10.3f}"
              f"{row['overshoot_in']:<12.2f}{row['settling_time_ms']:<14.0f}"
              f"{row['steady_state_error_in']:<12.2f}{row['velocity_reversals']:<12.0f}"
              f"{row['final_error_in']:<12.2f}")

    # Recommendation
    best = df_metrics.iloc[0]
    print("\n" + "=" * 100)
    print(f"✅ RECOMMENDED GAINS: kP = {best['kP']:.1f}, kD = {best['kD']:.1f}")
    print(f"   Performance: {best['overshoot_in']:.2f}\" overshoot, {best['settling_time_ms']:.0f}ms settle time")
    print("=" * 100)

    # Save detailed report
    output_dir = filepath.parent
    report_file = output_dir / "gain_ranking.txt"

    with open(report_file, 'w') as f:
        f.write("GAIN SWEEP RANKING REPORT\n")
        f.write("=" * 80 + "\n\n")
        f.write(f"Source: {filepath.name}\n")
        f.write(f"Target Distance: 24.0 inches\n")
        f.write(f"Total Gain Sets Tested: {len(df_metrics)}\n\n")

        f.write("TOP 10 PERFORMERS:\n")
        f.write("-" * 80 + "\n")
        f.write(df_metrics.head(10).to_string(index=False))
        f.write(f"\n\n{'=' * 80}\n")
        f.write(f"RECOMMENDED: kP={best['kP']:.1f}, kD={best['kD']:.1f}\n")
        f.write(f"{'=' * 80}\n")

    print(f"\n💾 Saved report: {report_file}")

    # Generate plots
    plot_top_performers(df, df_metrics, output_dir, target_distance=24.0)

    print(f"\n✅ Analysis complete! Apply kP={best['kP']:.1f}, kD={best['kD']:.1f} to globals.cpp\n")

if __name__ == "__main__":
    main()
