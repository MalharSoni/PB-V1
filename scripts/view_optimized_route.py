#!/usr/bin/env python3
"""
Quick viewer for the AI-optimized 15-second autonomous route
Loads the saved path and displays it on the field
"""

import sys
import os

# Add parent directory to path to import planner
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# Import the interactive planner
import subprocess

# Load the optimized route file and launch the planner
print("="*80)
print("AI-OPTIMIZED 15-SECOND AUTONOMOUS ROUTE")
print("="*80)
print("\nRoute: Corner Sweep Strategy")
print("- Starts: (-60, -60) touching park barrier")
print("- Collects: 3 balls along back wall")
print("- Scores: At scoring zone")
print("- Parks: Returns to barrier")
print("- Time: ~14-15 seconds")
print("\n" + "="*80)
print("\nInstructions:")
print("1. The path planner will open with the optimized route loaded")
print("2. Press 'L' to load the optimized_15s_route.json file")
print("3. Press SPACE to simulate the route")
print("4. Press 'W' to export directly to your auton.cpp")
print("5. Adjust waypoints by dragging if needed")
print("="*80 + "\n")

# Launch the interactive planner
subprocess.run([
    "python3",
    "scripts/interactive_path_planner.py",
    "--field", "images/VRC_Push_Back_Field.png"
])
