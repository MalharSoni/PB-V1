#!/usr/bin/env python3
"""
VEX Field Visualizer - Real-time robot position visualization

Displays robot movement on a 144" x 144" VEX field using telemetry data.
Supports real-time monitoring during simulation and replay of recorded runs.

Usage:
    # Real-time mode (watch simulation as it runs)
    python scripts/field_visualizer.py --live /usd/telemetry.csv

    # Replay mode (play back recorded autonomous)
    python scripts/field_visualizer.py --replay path/to/telemetry.csv

    # Adjust playback speed
    python scripts/field_visualizer.py --replay file.csv --speed 2.0

    # Custom robot color
    python scripts/field_visualizer.py --replay file.csv --robot-color red
"""

import pygame
import pandas as pd
import numpy as np
import argparse
import sys
import os
import time
from pathlib import Path

# ============================================================================
# CONFIGURATION
# ============================================================================

# Field dimensions (inches)
FIELD_SIZE = 144  # VEX field is 144" x 144"

# Display settings
WINDOW_SIZE = 800  # Window size in pixels
SCALE = WINDOW_SIZE / FIELD_SIZE  # Pixels per inch
FPS = 60  # Frame rate

# Colors (RGB)
FIELD_GREEN = (34, 139, 34)
FIELD_BORDER = (200, 200, 200)
GRID_LINE = (60, 160, 60)
PATH_COLOR = (100, 149, 237)  # Cornflower blue
ROBOT_COLOR = (255, 69, 0)     # Red-orange
TEXT_COLOR = (255, 255, 255)
BACKGROUND = (20, 20, 20)

# Robot display
ROBOT_SIZE = 18  # inches (VEX robot is ~18" square)
ROBOT_THICKNESS = 2  # Outline thickness

# Path tracing
PATH_THICKNESS = 2
MAX_PATH_POINTS = 1000  # Limit path history

# ============================================================================
# COORDINATE CONVERSION
# ============================================================================

def field_to_screen(x, y):
    """
    Convert field coordinates (inches, origin at center) to screen coordinates.

    Field coordinates: (-72, -72) to (72, 72), origin at center
    Screen coordinates: (0, 0) at top-left, Y increases downward

    Args:
        x: Field X coordinate (inches, -72 to +72)
        y: Field Y coordinate (inches, -72 to +72)

    Returns:
        (screen_x, screen_y): Screen pixel coordinates
    """
    # Shift origin to center, then scale to pixels
    screen_x = (x + FIELD_SIZE / 2) * SCALE

    # Flip Y axis (field Y+ is up, screen Y+ is down)
    screen_y = (FIELD_SIZE / 2 - y) * SCALE

    return int(screen_x), int(screen_y)


def inches_to_pixels(inches):
    """Convert distance in inches to pixels."""
    return int(inches * SCALE)


# ============================================================================
# FIELD RENDERER
# ============================================================================

class FieldRenderer:
    """Renders VEX 144" x 144" field with grid lines."""

    def __init__(self, surface):
        self.surface = surface
        self.grid_spacing = 24  # Grid every 24 inches (tiles)

    def draw(self):
        """Draw the VEX field background."""
        # Fill with field green
        self.surface.fill(FIELD_GREEN)

        # Draw grid lines (24" spacing)
        for i in range(0, int(FIELD_SIZE / self.grid_spacing) + 1):
            pos = i * self.grid_spacing

            # Vertical lines
            x1, y1 = field_to_screen(pos - FIELD_SIZE/2, -FIELD_SIZE/2)
            x2, y2 = field_to_screen(pos - FIELD_SIZE/2, FIELD_SIZE/2)
            pygame.draw.line(self.surface, GRID_LINE, (x1, y1), (x2, y2), 1)

            # Horizontal lines
            x1, y1 = field_to_screen(-FIELD_SIZE/2, pos - FIELD_SIZE/2)
            x2, y2 = field_to_screen(FIELD_SIZE/2, pos - FIELD_SIZE/2)
            pygame.draw.line(self.surface, GRID_LINE, (x1, y1), (x2, y2), 1)

        # Draw center cross
        center_x, center_y = field_to_screen(0, 0)
        cross_size = 10
        pygame.draw.line(self.surface, FIELD_BORDER,
                        (center_x - cross_size, center_y),
                        (center_x + cross_size, center_y), 2)
        pygame.draw.line(self.surface, FIELD_BORDER,
                        (center_x, center_y - cross_size),
                        (center_x, center_y + cross_size), 2)

        # Draw field border
        border_rect = pygame.Rect(0, 0, WINDOW_SIZE, WINDOW_SIZE)
        pygame.draw.rect(self.surface, FIELD_BORDER, border_rect, 3)


# ============================================================================
# ROBOT SPRITE
# ============================================================================

class Robot:
    """Represents the robot on the field."""

    def __init__(self, surface, color=ROBOT_COLOR):
        self.surface = surface
        self.color = color
        self.x = 0  # Field coordinates (inches)
        self.y = 0
        self.theta = 0  # Heading (degrees)

        # Path history
        self.path = []

    def update(self, x, y, theta):
        """Update robot position and heading."""
        self.x = x
        self.y = y
        self.theta = theta

        # Add to path history
        screen_pos = field_to_screen(x, y)
        self.path.append(screen_pos)

        # Limit path length
        if len(self.path) > MAX_PATH_POINTS:
            self.path.pop(0)

    def draw(self):
        """Draw robot and its path."""
        # Draw path trace
        if len(self.path) > 1:
            pygame.draw.lines(self.surface, PATH_COLOR, False, self.path, PATH_THICKNESS)

        # Get screen position
        screen_x, screen_y = field_to_screen(self.x, self.y)
        robot_pixel_size = inches_to_pixels(ROBOT_SIZE)

        # Draw robot body (square)
        robot_rect = pygame.Rect(
            screen_x - robot_pixel_size // 2,
            screen_y - robot_pixel_size // 2,
            robot_pixel_size,
            robot_pixel_size
        )
        pygame.draw.rect(self.surface, self.color, robot_rect, ROBOT_THICKNESS)

        # Draw heading indicator (arrow pointing forward)
        arrow_length = robot_pixel_size // 2
        theta_rad = np.deg2rad(self.theta)

        # Arrow endpoint
        arrow_end_x = screen_x + arrow_length * np.cos(theta_rad)
        arrow_end_y = screen_y - arrow_length * np.sin(theta_rad)  # Negative because screen Y is flipped

        pygame.draw.line(self.surface, self.color,
                        (screen_x, screen_y),
                        (arrow_end_x, arrow_end_y), 3)

        # Draw center dot
        pygame.draw.circle(self.surface, self.color, (screen_x, screen_y), 3)


# ============================================================================
# TELEMETRY READER
# ============================================================================

class TelemetryReader:
    """Reads telemetry CSV file (supports live and replay modes)."""

    def __init__(self, filepath, live=False):
        self.filepath = filepath
        self.live = live
        self.data = None
        self.current_index = 0
        self.last_modified = 0

        if not live:
            # Load entire file for replay
            self.load_data()

    def load_data(self):
        """Load CSV data from file."""
        try:
            self.data = pd.read_csv(self.filepath)
            print(f"[Telemetry] Loaded {len(self.data)} rows from {self.filepath}")
        except FileNotFoundError:
            print(f"[ERROR] File not found: {self.filepath}")
            sys.exit(1)
        except pd.errors.EmptyDataError:
            print(f"[ERROR] Empty CSV file: {self.filepath}")
            sys.exit(1)

    def check_for_updates(self):
        """Check if file has been modified (live mode only)."""
        if not self.live:
            return False

        try:
            mtime = os.path.getmtime(self.filepath)
            if mtime > self.last_modified:
                self.last_modified = mtime
                self.load_data()
                return True
        except FileNotFoundError:
            pass

        return False

    def get_row(self, index):
        """Get telemetry data at specific index."""
        if self.data is None or index >= len(self.data):
            return None

        row = self.data.iloc[index]
        return {
            'time_ms': row['time_ms'],
            'x': row['x'],
            'y': row['y'],
            'theta': row['theta'],
            'velocity': row.get('velocity', 0)  # Optional field
        }

    def get_total_rows(self):
        """Get total number of data rows."""
        return len(self.data) if self.data is not None else 0


# ============================================================================
# VISUALIZER MAIN CLASS
# ============================================================================

class FieldVisualizer:
    """Main visualizer application."""

    def __init__(self, telemetry_file, live=False, speed=1.0, robot_color=None):
        pygame.init()

        # Create window
        self.screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE + 100))  # Extra space for HUD
        pygame.display.set_caption("VEX Field Visualizer")

        # Components
        self.field_surface = pygame.Surface((WINDOW_SIZE, WINDOW_SIZE))
        self.field_renderer = FieldRenderer(self.field_surface)

        # Robot color
        if robot_color:
            color_map = {
                'red': (255, 0, 0),
                'blue': (0, 0, 255),
                'green': (0, 255, 0),
                'orange': ROBOT_COLOR,
                'yellow': (255, 255, 0),
                'purple': (128, 0, 128)
            }
            robot_color = color_map.get(robot_color.lower(), ROBOT_COLOR)
        else:
            robot_color = ROBOT_COLOR

        self.robot = Robot(self.field_surface, robot_color)
        self.telemetry = TelemetryReader(telemetry_file, live)

        # Playback state
        self.live = live
        self.speed = speed
        self.paused = False
        self.current_index = 0
        self.last_update_time = 0

        # Clock
        self.clock = pygame.time.Clock()

        # Font for HUD
        self.font = pygame.font.Font(None, 24)
        self.font_small = pygame.font.Font(None, 18)

    def draw_hud(self):
        """Draw heads-up display with telemetry info."""
        hud_y = WINDOW_SIZE + 10

        # Get current telemetry
        data = self.telemetry.get_row(self.current_index)
        if data is None:
            return

        # Draw background
        hud_rect = pygame.Rect(0, WINDOW_SIZE, WINDOW_SIZE, 100)
        pygame.draw.rect(self.screen, BACKGROUND, hud_rect)

        # Position info
        pos_text = self.font.render(
            f"X: {data['x']:.1f}\"  Y: {data['y']:.1f}\"  θ: {data['theta']:.1f}°",
            True, TEXT_COLOR
        )
        self.screen.blit(pos_text, (10, hud_y))

        # Time and velocity
        time_text = self.font_small.render(
            f"Time: {data['time_ms']/1000:.2f}s  |  Velocity: {data.get('velocity', 0):.1f} in/s",
            True, TEXT_COLOR
        )
        self.screen.blit(time_text, (10, hud_y + 30))

        # Playback controls
        mode_text = "LIVE" if self.live else f"REPLAY ({self.speed}x)"
        status = "PAUSED" if self.paused else mode_text
        control_text = self.font_small.render(
            f"Status: {status}  |  Frame: {self.current_index}/{self.telemetry.get_total_rows()}",
            True, TEXT_COLOR
        )
        self.screen.blit(control_text, (10, hud_y + 55))

        # Help text
        help_text = self.font_small.render(
            "SPACE: Pause  |  R: Restart  |  +/-: Speed  |  Q/ESC: Quit",
            True, (150, 150, 150)
        )
        self.screen.blit(help_text, (10, hud_y + 75))

    def handle_input(self):
        """Handle keyboard input."""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False

            if event.type == pygame.KEYDOWN:
                if event.key in (pygame.K_q, pygame.K_ESCAPE):
                    return False

                elif event.key == pygame.K_SPACE:
                    self.paused = not self.paused

                elif event.key == pygame.K_r:
                    self.current_index = 0
                    self.robot.path = []

                elif event.key in (pygame.K_PLUS, pygame.K_EQUALS):
                    self.speed = min(self.speed * 1.5, 10.0)
                    print(f"[Visualizer] Speed: {self.speed}x")

                elif event.key == pygame.K_MINUS:
                    self.speed = max(self.speed / 1.5, 0.1)
                    print(f"[Visualizer] Speed: {self.speed}x")

        return True

    def update(self):
        """Update simulation state."""
        if self.paused:
            return

        # Check for new data in live mode
        if self.live:
            self.telemetry.check_for_updates()

        # Calculate time step based on speed
        current_time = time.time()
        elapsed = (current_time - self.last_update_time) * 1000  # ms

        if self.last_update_time == 0 or elapsed * self.speed >= 50:  # 50ms per frame
            data = self.telemetry.get_row(self.current_index)

            if data:
                self.robot.update(data['x'], data['y'], data['theta'])
                self.current_index += 1
                self.last_update_time = current_time

            # Loop in replay mode
            if not self.live and self.current_index >= self.telemetry.get_total_rows():
                if self.telemetry.get_total_rows() > 0:
                    self.current_index = 0
                    self.robot.path = []

    def draw(self):
        """Draw everything."""
        # Draw field
        self.field_renderer.draw()

        # Draw robot
        self.robot.draw()

        # Blit field to main screen
        self.screen.blit(self.field_surface, (0, 0))

        # Draw HUD
        self.draw_hud()

        pygame.display.flip()

    def run(self):
        """Main application loop."""
        print(f"[Visualizer] Starting in {'LIVE' if self.live else 'REPLAY'} mode")
        print(f"[Visualizer] Speed: {self.speed}x")
        print(f"[Visualizer] Controls: SPACE=Pause, R=Restart, +/-=Speed, Q=Quit")

        running = True
        while running:
            running = self.handle_input()
            self.update()
            self.draw()
            self.clock.tick(FPS)

        pygame.quit()
        print("[Visualizer] Exited")


# ============================================================================
# MAIN
# ============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="VEX Field Visualizer - Real-time robot position visualization"
    )

    # Mode selection (mutually exclusive)
    mode_group = parser.add_mutually_exclusive_group(required=True)
    mode_group.add_argument('--live', metavar='CSV', help='Live monitoring mode (watch simulation)')
    mode_group.add_argument('--replay', metavar='CSV', help='Replay mode (play back recorded run)')

    # Optional arguments
    parser.add_argument('--speed', type=float, default=1.0,
                       help='Playback speed multiplier (default: 1.0)')
    parser.add_argument('--robot-color', type=str, choices=['red', 'blue', 'green', 'orange', 'yellow', 'purple'],
                       help='Robot color (default: orange)')

    args = parser.parse_args()

    # Determine mode and file
    if args.live:
        telemetry_file = args.live
        live_mode = True
    else:
        telemetry_file = args.replay
        live_mode = False

    # Check file exists
    if not os.path.exists(telemetry_file):
        print(f"[ERROR] Telemetry file not found: {telemetry_file}")
        print(f"[INFO] Make sure simulation has created the CSV file")
        sys.exit(1)

    # Run visualizer
    visualizer = FieldVisualizer(telemetry_file, live_mode, args.speed, args.robot_color)
    visualizer.run()


if __name__ == "__main__":
    main()
