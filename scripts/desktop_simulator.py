#!/usr/bin/env python3
"""
VEX Desktop Simulator - Pure Python Autonomous Simulator

Run autonomous simulations entirely on your laptop WITHOUT the robot!

Features:
- Parse autonomous code from C++ files
- Simulate LemLib chassis commands (moveToPoint, turnToHeading, etc.)
- Real-time field visualization with custom field image
- Generate telemetry CSV for analysis
- No robot required!

Usage:
    # With custom field image
    python3 scripts/desktop_simulator.py --field images/pushback_field.png

    # With generated field (no image)
    python3 scripts/desktop_simulator.py

    # Specify autonomous function to run
    python3 scripts/desktop_simulator.py --auton odomDriveTest

    # Export CSV for later replay
    python3 scripts/desktop_simulator.py --export simulation_output.csv
"""

import pygame
import numpy as np
import argparse
import sys
import os
import time
import csv
from pathlib import Path
from dataclasses import dataclass
from typing import List, Tuple, Optional

# ============================================================================
# CONFIGURATION
# ============================================================================

# Field dimensions (inches)
FIELD_SIZE = 144  # 12 feet = 144 inches

# Display settings
WINDOW_SIZE = 700  # Window size in pixels (smaller window)
SCALE = WINDOW_SIZE / FIELD_SIZE  # Pixels per inch
FPS = 60  # Frame rate

# Colors (RGB)
FIELD_GREEN = (34, 139, 34)
FIELD_BLUE_ZONE = (100, 149, 237)  # Alliance zones
FIELD_RED_ZONE = (220, 20, 60)
GRID_LINE = (60, 160, 60)
PATH_COLOR = (255, 215, 0)  # Gold path
ROBOT_COLOR = (255, 69, 0)  # Orange robot
TEXT_COLOR = (255, 255, 255)
BACKGROUND = (20, 20, 20)

# Robot display
ROBOT_SIZE = 18  # inches (VEX robot is ~18" square)

# Physics (calibrated from real robot data: 48" in 1.8s @ 60% speed)
MAX_LINEAR_VEL = 75.0   # inches/sec (100% speed)
MAX_ANGULAR_VEL = 180.0  # degrees/sec
LINEAR_ACCEL = 50.0     # in/s^2
ANGULAR_ACCEL = 360.0   # deg/s^2

# Simulation time step
TIME_STEP = 0.01  # 10ms


# ============================================================================
# DATA STRUCTURES
# ============================================================================

@dataclass
class RobotState:
    """Robot position and motion state."""
    x: float = 0.0          # inches
    y: float = 0.0          # inches
    theta: float = 0.0      # degrees
    velocity: float = 0.0   # in/s
    ang_velocity: float = 0.0  # deg/s


@dataclass
class AutonomousCommand:
    """Autonomous movement command."""
    command_type: str  # 'moveToPoint', 'turnToHeading', 'delay', 'setPose'
    params: dict       # Command parameters


# ============================================================================
# COORDINATE CONVERSION
# ============================================================================

def field_to_screen(x, y):
    """Convert field coordinates to screen coordinates."""
    screen_x = (x + FIELD_SIZE / 2) * SCALE
    screen_y = (FIELD_SIZE / 2 - y) * SCALE  # Flip Y
    return int(screen_x), int(screen_y)


def inches_to_pixels(inches):
    """Convert inches to pixels."""
    return int(inches * SCALE)


# ============================================================================
# AUTONOMOUS COMMAND PARSER
# ============================================================================

class AutonomousParser:
    """Parse autonomous commands from simple script format."""

    @staticmethod
    def parse_script(script: str) -> List[AutonomousCommand]:
        """
        Parse autonomous script into commands.

        Script format:
            setPose 0 0 0
            moveToPoint 48 0 2000
            turnToHeading 90 1000
            delay 500
        """
        commands = []

        for line in script.strip().split('\n'):
            line = line.strip()
            if not line or line.startswith('#'):
                continue

            parts = line.split()
            cmd_type = parts[0]

            if cmd_type == 'setPose':
                commands.append(AutonomousCommand(
                    'setPose',
                    {'x': float(parts[1]), 'y': float(parts[2]), 'theta': float(parts[3])}
                ))

            elif cmd_type == 'moveToPoint':
                commands.append(AutonomousCommand(
                    'moveToPoint',
                    {'x': float(parts[1]), 'y': float(parts[2]), 'timeout': float(parts[3]),
                     'maxSpeed': float(parts[4]) if len(parts) > 4 else 100.0}
                ))

            elif cmd_type == 'turnToHeading':
                commands.append(AutonomousCommand(
                    'turnToHeading',
                    {'theta': float(parts[1]), 'timeout': float(parts[2])}
                ))

            elif cmd_type == 'delay':
                commands.append(AutonomousCommand(
                    'delay',
                    {'ms': float(parts[1])}
                ))

        return commands

    @staticmethod
    def get_builtin_auton(name: str) -> List[AutonomousCommand]:
        """Get built-in autonomous routine."""
        routines = {
            'odomDriveTest': [
                AutonomousCommand('setPose', {'x': 0, 'y': 0, 'theta': 0}),
                AutonomousCommand('moveToPoint', {'x': 48, 'y': 0, 'timeout': 3000, 'maxSpeed': 60}),
            ],
            'odomSquareTest': [
                AutonomousCommand('setPose', {'x': 0, 'y': 0, 'theta': 0}),
                AutonomousCommand('moveToPoint', {'x': 24, 'y': 0, 'timeout': 2000, 'maxSpeed': 100}),
                AutonomousCommand('moveToPoint', {'x': 24, 'y': 24, 'timeout': 2000, 'maxSpeed': 100}),
                AutonomousCommand('moveToPoint', {'x': 0, 'y': 24, 'timeout': 2000, 'maxSpeed': 100}),
                AutonomousCommand('moveToPoint', {'x': 0, 'y': 0, 'timeout': 2000, 'maxSpeed': 100}),
            ],
            'simple_path': [
                AutonomousCommand('setPose', {'x': -60, 'y': -60, 'theta': 45}),
                AutonomousCommand('moveToPoint', {'x': 0, 'y': 0, 'timeout': 3000, 'maxSpeed': 80}),
                AutonomousCommand('turnToHeading', {'theta': 90, 'timeout': 1000}),
                AutonomousCommand('moveToPoint', {'x': 0, 'y': 24, 'timeout': 2000, 'maxSpeed': 60}),
            ]
        }

        return routines.get(name, routines['odomDriveTest'])


# ============================================================================
# PHYSICS ENGINE
# ============================================================================

class PhysicsEngine:
    """Simulate robot physics and movement."""

    def __init__(self):
        self.state = RobotState()
        self.target_x = 0.0
        self.target_y = 0.0
        self.target_theta = 0.0
        self.is_moving = False
        self.command_timeout = 0
        self.command_start_time = 0

    def set_pose(self, x: float, y: float, theta: float):
        """Set robot position."""
        self.state.x = x
        self.state.y = y
        self.state.theta = theta
        self.state.velocity = 0
        self.state.ang_velocity = 0

    def move_to_point(self, x: float, y: float, timeout: float, max_speed: float = 100.0):
        """Start moving to a point."""
        self.target_x = x
        self.target_y = y
        self.is_moving = True
        self.command_timeout = timeout / 1000.0  # Convert ms to seconds
        self.command_start_time = 0

    def turn_to_heading(self, theta: float, timeout: float):
        """Start turning to heading."""
        self.target_theta = theta
        self.target_x = self.state.x  # Stay in place
        self.target_y = self.state.y
        self.is_moving = True
        self.command_timeout = timeout / 1000.0
        self.command_start_time = 0

    def is_command_done(self, elapsed_time: float) -> bool:
        """Check if current command is complete."""
        if not self.is_moving:
            return True

        # Check timeout
        if elapsed_time - self.command_start_time > self.command_timeout:
            self.is_moving = False
            return True

        # Check position tolerance
        dx = self.target_x - self.state.x
        dy = self.target_y - self.state.y
        dist = np.sqrt(dx * dx + dy * dy)

        # Check heading tolerance
        heading_error = abs(self.target_theta - self.state.theta)
        while heading_error > 180:
            heading_error -= 360
        heading_error = abs(heading_error)

        if dist < 0.5 and heading_error < 2.0 and abs(self.state.velocity) < 1.0:
            self.is_moving = False
            return True

        return False

    def update(self, dt: float):
        """Update physics simulation."""
        if not self.is_moving:
            # Decelerate to stop
            self.state.velocity *= 0.9
            self.state.ang_velocity *= 0.9
            return

        # Calculate error
        dx = self.target_x - self.state.x
        dy = self.target_y - self.state.y
        dist_to_target = np.sqrt(dx * dx + dy * dy)
        angle_to_target = np.degrees(np.atan2(dy, dx))

        # Heading error
        heading_error = self.target_theta - self.state.theta
        while heading_error > 180:
            heading_error -= 360
        while heading_error < -180:
            heading_error += 360

        # Determine if turning in place or moving
        turning_in_place = (abs(heading_error) > 5.0 and dist_to_target < 1.0)

        if turning_in_place:
            # Pure rotation
            desired_ang_vel = heading_error * 2.0
            desired_ang_vel = np.clip(desired_ang_vel, -MAX_ANGULAR_VEL, MAX_ANGULAR_VEL)

            # Accelerate towards desired
            if abs(desired_ang_vel - self.state.ang_velocity) > ANGULAR_ACCEL * dt:
                self.state.ang_velocity += np.sign(desired_ang_vel - self.state.ang_velocity) * ANGULAR_ACCEL * dt
            else:
                self.state.ang_velocity = desired_ang_vel

            # Update heading
            self.state.theta += self.state.ang_velocity * dt
            while self.state.theta > 180:
                self.state.theta -= 360
            while self.state.theta < -180:
                self.state.theta += 360

        elif dist_to_target > 0.5:
            # Moving to point - trapezoidal velocity profile
            decel_dist = (self.state.velocity ** 2) / (2.0 * LINEAR_ACCEL)

            if dist_to_target > decel_dist * 2.0:
                # Acceleration phase
                desired_vel = min(dist_to_target * 8.0, MAX_LINEAR_VEL)
                if self.state.velocity < desired_vel:
                    self.state.velocity += LINEAR_ACCEL * dt
                    self.state.velocity = min(self.state.velocity, desired_vel)
            else:
                # Deceleration phase
                desired_vel = np.sqrt(2.0 * LINEAR_ACCEL * dist_to_target)
                self.state.velocity -= LINEAR_ACCEL * dt
                self.state.velocity = max(self.state.velocity, desired_vel)

            # Move towards target
            move_angle = angle_to_target
            self.state.x += self.state.velocity * np.cos(np.radians(move_angle)) * dt
            self.state.y += self.state.velocity * np.sin(np.radians(move_angle)) * dt

            # Gradually turn towards target
            self.state.theta += heading_error * 0.1 * dt
            while self.state.theta > 180:
                self.state.theta -= 360
            while self.state.theta < -180:
                self.state.theta += 360
        else:
            # Reached target
            self.is_moving = False
            self.state.velocity = 0
            self.state.ang_velocity = 0


# ============================================================================
# FIELD RENDERER
# ============================================================================

class FieldRenderer:
    """Render VEX field with optional custom image."""

    def __init__(self, surface, field_image_path: Optional[str] = None):
        self.surface = surface
        self.field_image = None

        if field_image_path and os.path.exists(field_image_path):
            # Load and scale custom field image
            try:
                self.field_image = pygame.image.load(field_image_path)
                self.field_image = pygame.transform.scale(self.field_image, (WINDOW_SIZE, WINDOW_SIZE))
                print(f"[Field] Loaded custom field image: {field_image_path}")
            except Exception as e:
                print(f"[Field] Failed to load image: {e}")
                self.field_image = None

    def draw(self):
        """Draw field."""
        if self.field_image:
            # Use custom field image
            self.surface.blit(self.field_image, (0, 0))
        else:
            # Draw generated field
            self._draw_generated_field()

    def _draw_generated_field(self):
        """Draw programmatic field representation."""
        # Green background
        self.surface.fill(FIELD_GREEN)

        # Grid lines (24" tiles)
        for i in range(7):
            pos = i * 24
            x1, y1 = field_to_screen(pos - 72, -72)
            x2, y2 = field_to_screen(pos - 72, 72)
            pygame.draw.line(self.surface, GRID_LINE, (x1, y1), (x2, y2), 1)

            x1, y1 = field_to_screen(-72, pos - 72)
            x2, y2 = field_to_screen(72, pos - 72)
            pygame.draw.line(self.surface, GRID_LINE, (x1, y1), (x2, y2), 1)

        # Center cross (origin)
        center_x, center_y = field_to_screen(0, 0)
        pygame.draw.circle(self.surface, (255, 255, 255), (center_x, center_y), 5)

        # Draw coordinate axes
        font = pygame.font.Font(None, 20)

        # +X axis label (right)
        x_label = font.render("+X", True, (255, 255, 255))
        self.surface.blit(x_label, (WINDOW_SIZE - 30, center_y - 15))

        # +Y axis label (top)
        y_label = font.render("+Y", True, (255, 255, 255))
        self.surface.blit(y_label, (center_x - 15, 10))

        # Alliance zones (corners)
        # Red alliance (negative X, negative Y)
        red_zone = pygame.Rect(0, WINDOW_SIZE - WINDOW_SIZE//4, WINDOW_SIZE//4, WINDOW_SIZE//4)
        pygame.draw.rect(self.surface, FIELD_RED_ZONE, red_zone, 3)

        # Blue alliance (positive X, negative Y)
        blue_zone = pygame.Rect(WINDOW_SIZE - WINDOW_SIZE//4, WINDOW_SIZE - WINDOW_SIZE//4, WINDOW_SIZE//4, WINDOW_SIZE//4)
        pygame.draw.rect(self.surface, FIELD_BLUE_ZONE, blue_zone, 3)

        # Field border
        pygame.draw.rect(self.surface, (200, 200, 200), (0, 0, WINDOW_SIZE, WINDOW_SIZE), 3)


# ============================================================================
# ROBOT SPRITE
# ============================================================================

class Robot:
    """Robot visualization."""

    def __init__(self, surface):
        self.surface = surface
        self.path = []

    def update(self, state: RobotState):
        """Update robot state."""
        screen_pos = field_to_screen(state.x, state.y)
        self.path.append(screen_pos)

        # Limit path length
        if len(self.path) > 2000:
            self.path.pop(0)

    def draw(self, state: RobotState):
        """Draw robot."""
        # Draw path
        if len(self.path) > 1:
            pygame.draw.lines(self.surface, PATH_COLOR, False, self.path, 3)

        # Robot position
        screen_x, screen_y = field_to_screen(state.x, state.y)
        robot_pixel_size = inches_to_pixels(ROBOT_SIZE)

        # Robot body (square)
        robot_rect = pygame.Rect(
            screen_x - robot_pixel_size // 2,
            screen_y - robot_pixel_size // 2,
            robot_pixel_size,
            robot_pixel_size
        )
        pygame.draw.rect(self.surface, ROBOT_COLOR, robot_rect, 3)

        # Heading arrow
        arrow_length = robot_pixel_size // 2
        theta_rad = np.radians(state.theta)
        arrow_end_x = screen_x + arrow_length * np.cos(theta_rad)
        arrow_end_y = screen_y - arrow_length * np.sin(theta_rad)
        pygame.draw.line(self.surface, ROBOT_COLOR, (screen_x, screen_y), (arrow_end_x, arrow_end_y), 4)

        # Center dot
        pygame.draw.circle(self.surface, ROBOT_COLOR, (screen_x, screen_y), 4)


# ============================================================================
# DESKTOP SIMULATOR
# ============================================================================

class DesktopSimulator:
    """Main desktop simulator."""

    def __init__(self, auton_name: str, field_image: Optional[str] = None, export_csv: Optional[str] = None):
        pygame.init()

        self.screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE + 120))
        pygame.display.set_caption("VEX Desktop Simulator - No Robot Required!")

        self.field_surface = pygame.Surface((WINDOW_SIZE, WINDOW_SIZE))
        self.field_renderer = FieldRenderer(self.field_surface, field_image)
        self.robot = Robot(self.field_surface)
        self.physics = PhysicsEngine()

        # Load autonomous
        self.commands = AutonomousParser.get_builtin_auton(auton_name)
        self.current_command_index = 0
        self.command_start_time = 0

        # Simulation state
        self.paused = False
        self.sim_time = 0.0
        self.clock = pygame.time.Clock()

        # CSV export
        self.export_csv = export_csv
        self.telemetry_data = []

        # Font
        self.font = pygame.font.Font(None, 24)
        self.font_small = pygame.font.Font(None, 18)

        print(f"[Simulator] Desktop simulator initialized")
        print(f"[Simulator] Running autonomous: {auton_name}")
        print(f"[Simulator] Commands loaded: {len(self.commands)}")

    def draw_hud(self):
        """Draw HUD."""
        hud_y = WINDOW_SIZE + 10
        hud_rect = pygame.Rect(0, WINDOW_SIZE, WINDOW_SIZE, 120)
        pygame.draw.rect(self.screen, BACKGROUND, hud_rect)

        state = self.physics.state

        # Position
        pos_text = self.font.render(
            f"X: {state.x:.1f}\"  Y: {state.y:.1f}\"  θ: {state.theta:.1f}°",
            True, TEXT_COLOR
        )
        self.screen.blit(pos_text, (10, hud_y))

        # Time and velocity
        time_text = self.font_small.render(
            f"Time: {self.sim_time:.2f}s  |  Velocity: {state.velocity:.1f} in/s",
            True, TEXT_COLOR
        )
        self.screen.blit(time_text, (10, hud_y + 30))

        # Command info
        if self.current_command_index < len(self.commands):
            cmd = self.commands[self.current_command_index]
            cmd_text = self.font_small.render(
                f"Command: {cmd.command_type} {cmd.params}",
                True, (150, 255, 150)
            )
            self.screen.blit(cmd_text, (10, hud_y + 55))
        else:
            done_text = self.font.render("AUTONOMOUS COMPLETE", True, (0, 255, 0))
            self.screen.blit(done_text, (10, hud_y + 55))

        # Status
        status = "PAUSED" if self.paused else "RUNNING"
        status_text = self.font_small.render(
            f"Status: {status}  |  Command: {self.current_command_index + 1}/{len(self.commands)}",
            True, TEXT_COLOR
        )
        self.screen.blit(status_text, (10, hud_y + 80))

        # Help
        help_text = self.font_small.render(
            "SPACE: Pause  |  R: Restart  |  Q/ESC: Quit",
            True, (150, 150, 150)
        )
        self.screen.blit(help_text, (10, hud_y + 110))

        # Desktop simulator badge
        badge_text = self.font_small.render(
            "🖥️  DESKTOP SIMULATOR - NO ROBOT NEEDED!",
            True, (255, 215, 0)
        )
        self.screen.blit(badge_text, (10, hud_y + 130))

    def handle_input(self):
        """Handle input."""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False

            if event.type == pygame.KEYDOWN:
                if event.key in (pygame.K_q, pygame.K_ESCAPE):
                    return False
                elif event.key == pygame.K_SPACE:
                    self.paused = not self.paused
                elif event.key == pygame.K_r:
                    self.restart()

        return True

    def restart(self):
        """Restart simulation."""
        self.physics = PhysicsEngine()
        self.robot.path = []
        self.current_command_index = 0
        self.sim_time = 0.0
        self.command_start_time = 0
        self.telemetry_data = []
        print("[Simulator] Restarted")

    def update(self):
        """Update simulation."""
        if self.paused:
            return

        # Update physics
        self.physics.update(TIME_STEP)
        self.robot.update(self.physics.state)
        self.sim_time += TIME_STEP

        # Log telemetry
        state = self.physics.state
        self.telemetry_data.append({
            'time_ms': int(self.sim_time * 1000),
            'x': state.x,
            'y': state.y,
            'theta': state.theta,
            'velocity': state.velocity,
            'lf_temp': 35.0,
            'lm_temp': 35.0,
            'rf_temp': 35.0,
            'rm_temp': 35.0,
            'lf_curr': 500.0,
            'lm_curr': 500.0,
            'rf_curr': 500.0,
            'rm_curr': 500.0,
            'battery_mv': 12600,
        })

        # Check if command done
        if self.current_command_index < len(self.commands):
            elapsed = self.sim_time - self.command_start_time

            if self.physics.is_command_done(elapsed):
                # Move to next command
                self.current_command_index += 1
                self.command_start_time = self.sim_time

                # Execute next command
                if self.current_command_index < len(self.commands):
                    self.execute_command(self.commands[self.current_command_index])

        # Start first command
        if self.current_command_index == 0 and self.sim_time < 0.1:
            self.execute_command(self.commands[0])

    def execute_command(self, cmd: AutonomousCommand):
        """Execute autonomous command."""
        if cmd.command_type == 'setPose':
            self.physics.set_pose(cmd.params['x'], cmd.params['y'], cmd.params['theta'])
        elif cmd.command_type == 'moveToPoint':
            self.physics.move_to_point(cmd.params['x'], cmd.params['y'], cmd.params['timeout'], cmd.params.get('maxSpeed', 100))
        elif cmd.command_type == 'turnToHeading':
            self.physics.turn_to_heading(cmd.params['theta'], cmd.params['timeout'])
        elif cmd.command_type == 'delay':
            # Handled by timeout system
            pass

    def draw(self):
        """Draw everything."""
        self.field_renderer.draw()
        self.robot.draw(self.physics.state)
        self.screen.blit(self.field_surface, (0, 0))
        self.draw_hud()
        pygame.display.flip()

    def export_telemetry(self):
        """Export telemetry to CSV."""
        if not self.export_csv:
            return

        with open(self.export_csv, 'w', newline='') as f:
            if not self.telemetry_data:
                return

            writer = csv.DictWriter(f, fieldnames=self.telemetry_data[0].keys())
            writer.writeheader()
            writer.writerows(self.telemetry_data)

        print(f"[Simulator] Exported telemetry to: {self.export_csv}")

    def run(self):
        """Main loop."""
        print("[Simulator] Starting simulation...")
        print("[Simulator] Controls: SPACE=Pause, R=Restart, Q=Quit")

        running = True
        while running:
            running = self.handle_input()
            self.update()
            self.draw()
            self.clock.tick(FPS)

        self.export_telemetry()
        pygame.quit()
        print("[Simulator] Exited")


# ============================================================================
# MAIN
# ============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="VEX Desktop Simulator - Run autonomous without robot!"
    )

    parser.add_argument('--auton', type=str, default='odomDriveTest',
                       help='Autonomous routine to run (odomDriveTest, odomSquareTest, simple_path)')
    parser.add_argument('--field', type=str, help='Path to custom field image (PNG/JPG)')
    parser.add_argument('--export', type=str, help='Export telemetry CSV to this path')

    args = parser.parse_args()

    # Check field image
    if args.field and not os.path.exists(args.field):
        print(f"[ERROR] Field image not found: {args.field}")
        print(f"[INFO] Continuing with generated field...")
        args.field = None

    # Run simulator
    simulator = DesktopSimulator(args.auton, args.field, args.export)
    simulator.run()


if __name__ == "__main__":
    main()
