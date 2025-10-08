#!/usr/bin/env python3
"""
VEX Interactive Path Planner - Game-Changing Autonomous Route Designer

Inspired by PATH.JERRYIO - The industry-leading VEX path planning tool

Features:
- Click to place waypoints on the field
- Drag to edit waypoint positions
- Real-time simulation as you design
- Auto-generate C++ code for LemLib
- Export directly to your auton.cpp
- Import existing autonomous routes

Controls:
- LEFT CLICK: Place waypoint
- DRAG: Move waypoint
- RIGHT CLICK: Delete waypoint
- SPACE: Play/Pause simulation
- C: Copy C++ code to clipboard
- E: Export to file
- R: Reset path
- S: Save path
- L: Load path

Usage:
    python3 scripts/interactive_path_planner.py --field images/VRC_Push_Back_Field.png
"""

import pygame
import numpy as np
import argparse
import sys
import os
import json
from pathlib import Path
from dataclasses import dataclass, asdict
from typing import List, Tuple, Optional
from datetime import datetime

# ============================================================================
# CONFIGURATION
# ============================================================================

FIELD_SIZE = 144  # inches
WINDOW_SIZE = 700  # pixels
SCALE = WINDOW_SIZE / FIELD_SIZE
FPS = 60

# Colors
FIELD_GREEN = (34, 139, 34)
GRID_LINE = (60, 160, 60)
WAYPOINT_COLOR = (255, 215, 0)  # Gold
WAYPOINT_HOVER = (255, 165, 0)  # Orange
PATH_COLOR = (100, 149, 237)  # Cornflower blue
ROBOT_COLOR = (255, 69, 0)  # Red-orange
TEXT_COLOR = (255, 255, 255)
BACKGROUND = (20, 20, 20)
AXIS_COLOR = (200, 200, 200)

# Interaction
WAYPOINT_RADIUS = 8  # pixels
SNAP_GRID = 6  # inches (snap to 6" grid)

# Physics (calibrated from real robot)
MAX_LINEAR_VEL = 75.0  # in/s
LINEAR_ACCEL = 50.0  # in/s^2
TIME_STEP = 0.01  # 10ms

# ============================================================================
# DATA STRUCTURES
# ============================================================================

@dataclass
class Waypoint:
    """A single waypoint in the path."""
    x: float  # inches
    y: float  # inches
    heading: Optional[float] = None  # degrees (None = auto)
    max_speed: float = 100.0  # percentage
    timeout: float = 3000.0  # milliseconds
    curve_smooth: float = 0.3  # Curve smoothing factor (0 = sharp, 1 = very smooth)

    def to_dict(self):
        return asdict(self)

    @classmethod
    def from_dict(cls, data):
        return cls(**data)


@dataclass
class RobotState:
    """Current robot state during simulation."""
    x: float = 0.0
    y: float = 0.0
    theta: float = 0.0
    velocity: float = 0.0


# ============================================================================
# COORDINATE CONVERSION
# ============================================================================

def field_to_screen(x: float, y: float) -> Tuple[int, int]:
    """Convert field coordinates (inches) to screen pixels."""
    screen_x = (x + FIELD_SIZE / 2) * SCALE
    screen_y = (FIELD_SIZE / 2 - y) * SCALE  # Flip Y
    return int(screen_x), int(screen_y)


def screen_to_field(screen_x: int, screen_y: int) -> Tuple[float, float]:
    """Convert screen pixels to field coordinates (inches)."""
    x = (screen_x / SCALE) - FIELD_SIZE / 2
    y = (FIELD_SIZE / 2) - (screen_y / SCALE)  # Flip Y
    return x, y


def snap_to_grid(x: float, y: float, grid_size: float) -> Tuple[float, float]:
    """Snap coordinates to grid."""
    x_snapped = round(x / grid_size) * grid_size
    y_snapped = round(y / grid_size) * grid_size
    return x_snapped, y_snapped


def inches_to_pixels(inches: float) -> int:
    """Convert inches to pixels."""
    return int(inches * SCALE)


def calculate_bezier_curve(p0, p1, p2, p3, num_points=50):
    """Calculate points along a cubic Bézier curve."""
    points = []
    for i in range(num_points + 1):
        t = i / num_points
        # Cubic Bézier formula
        x = (1-t)**3 * p0[0] + 3*(1-t)**2*t * p1[0] + 3*(1-t)*t**2 * p2[0] + t**3 * p3[0]
        y = (1-t)**3 * p0[1] + 3*(1-t)**2*t * p1[1] + 3*(1-t)*t**2 * p2[1] + t**3 * p3[1]
        points.append((x, y))
    return points


def calculate_control_points(waypoints, smooth_factor=0.3):
    """Calculate Bézier control points for smooth curves between waypoints."""
    if len(waypoints) < 2:
        return []

    control_points = []

    for i in range(len(waypoints) - 1):
        p0 = (waypoints[i].x, waypoints[i].y)
        p3 = (waypoints[i + 1].x, waypoints[i + 1].y)

        # Calculate direction vectors
        if i > 0:
            prev = (waypoints[i - 1].x, waypoints[i - 1].y)
            dx_in = p0[0] - prev[0]
            dy_in = p0[1] - prev[1]
        else:
            dx_in = p3[0] - p0[0]
            dy_in = p3[1] - p0[1]

        if i < len(waypoints) - 2:
            next_p = (waypoints[i + 2].x, waypoints[i + 2].y)
            dx_out = next_p[0] - p3[0]
            dy_out = next_p[1] - p3[1]
        else:
            dx_out = p3[0] - p0[0]
            dy_out = p3[1] - p0[1]

        # Use waypoint's curve smoothing factor
        smooth = waypoints[i].curve_smooth

        # Control point 1 (leaving p0)
        p1 = (
            p0[0] + dx_in * smooth * 0.5,
            p0[1] + dy_in * smooth * 0.5
        )

        # Control point 2 (approaching p3)
        p2 = (
            p3[0] - dx_out * smooth * 0.5,
            p3[1] - dy_out * smooth * 0.5
        )

        control_points.append((p0, p1, p2, p3))

    return control_points


# ============================================================================
# CODE GENERATION
# ============================================================================

class CodeGenerator:
    """Generate C++ code for LemLib from waypoints."""

    @staticmethod
    def generate_cpp(waypoints: List[Waypoint], route_name: str = "myRoute") -> str:
        """Generate C++ autonomous function."""
        if not waypoints:
            return "// No waypoints defined"

        code = f"""void Auton::{route_name}() {{
    // ========================================================================
    // {route_name.upper()} - Generated by Interactive Path Planner
    // ========================================================================
    // Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
    // Waypoints: {len(waypoints)}

    // Set starting position
    chassis->setPose({waypoints[0].x:.1f}, {waypoints[0].y:.1f}, 0);
    pros::delay(500);

"""

        # Generate moveToPoint commands
        for i, wp in enumerate(waypoints[1:], 1):
            code += f"    // Waypoint {i}: ({wp.x:.1f}\", {wp.y:.1f}\")\n"
            code += f"    chassis->moveToPoint({wp.x:.1f}, {wp.y:.1f}, {int(wp.timeout)}"

            if wp.max_speed != 100.0:
                code += f", {{.maxSpeed = {int(wp.max_speed)}}}"

            code += ");\n"

            if wp.heading is not None:
                code += f"    chassis->turnToHeading({wp.heading:.1f}, 1000);\n"

            code += "    chassis->waitUntilDone();\n\n"

        code += "}\n"
        return code

    @staticmethod
    def generate_desktop_sim(waypoints: List[Waypoint]) -> str:
        """Generate Python code for desktop simulator."""
        if not waypoints:
            return "[]"

        code = "[\n"
        code += f"    AutonomousCommand('setPose', {{'x': {waypoints[0].x:.1f}, 'y': {waypoints[0].y:.1f}, 'theta': 0}}),\n"

        for wp in waypoints[1:]:
            code += f"    AutonomousCommand('moveToPoint', {{'x': {wp.x:.1f}, 'y': {wp.y:.1f}, 'timeout': {wp.timeout:.0f}, 'maxSpeed': {wp.max_speed:.0f}}}),\n"
            if wp.heading is not None:
                code += f"    AutonomousCommand('turnToHeading', {{'theta': {wp.heading:.1f}, 'timeout': 1000}}),\n"

        code += "]"
        return code


# ============================================================================
# SIMPLE PHYSICS ENGINE
# ============================================================================

class SimplePhysics:
    """Simple physics for real-time preview."""

    def __init__(self):
        self.state = RobotState()
        self.target_x = 0.0
        self.target_y = 0.0
        self.is_moving = False

    def set_pose(self, x: float, y: float, theta: float = 0):
        self.state.x = x
        self.state.y = y
        self.state.theta = theta
        self.state.velocity = 0

    def move_to(self, x: float, y: float):
        self.target_x = x
        self.target_y = y
        self.is_moving = True

    def update(self, dt: float):
        if not self.is_moving:
            return

        dx = self.target_x - self.state.x
        dy = self.target_y - self.state.y
        dist = np.sqrt(dx * dx + dy * dy)

        if dist < 0.5:
            self.state.x = self.target_x
            self.state.y = self.target_y
            self.state.velocity = 0
            self.is_moving = False
            return

        # Simple proportional movement
        max_vel = min(MAX_LINEAR_VEL, dist * 10)
        angle = np.atan2(dy, dx)

        self.state.velocity = max_vel
        self.state.x += max_vel * np.cos(angle) * dt
        self.state.y += max_vel * np.sin(angle) * dt
        self.state.theta = np.degrees(angle)


# ============================================================================
# FIELD RENDERER
# ============================================================================

class FieldRenderer:
    """Render VEX field with coordinate axes."""

    def __init__(self, surface, field_image: Optional[str] = None):
        self.surface = surface
        self.field_image = None

        if field_image and os.path.exists(field_image):
            try:
                self.field_image = pygame.image.load(field_image)
                self.field_image = pygame.transform.scale(self.field_image, (WINDOW_SIZE, WINDOW_SIZE))
                print(f"[Field] Loaded: {field_image}")
            except Exception as e:
                print(f"[Field] Error loading image: {e}")

    def draw(self):
        if self.field_image:
            self.surface.blit(self.field_image, (0, 0))
            self._draw_axes_overlay()
        else:
            self._draw_generated_field()

    def _draw_axes_overlay(self):
        """Draw coordinate axes on top of field image."""
        center_x, center_y = field_to_screen(0, 0)
        font = pygame.font.Font(None, 18)

        # X axis
        pygame.draw.line(self.surface, AXIS_COLOR,
                        (0, center_y), (WINDOW_SIZE, center_y), 1)
        x_label = font.render("+X", True, AXIS_COLOR)
        self.surface.blit(x_label, (WINDOW_SIZE - 25, center_y + 5))

        # Y axis
        pygame.draw.line(self.surface, AXIS_COLOR,
                        (center_x, 0), (center_x, WINDOW_SIZE), 1)
        y_label = font.render("+Y", True, AXIS_COLOR)
        self.surface.blit(y_label, (center_x + 5, 5))

        # Origin
        pygame.draw.circle(self.surface, AXIS_COLOR, (center_x, center_y), 4)

    def _draw_generated_field(self):
        """Draw simple field when no image available."""
        self.surface.fill(FIELD_GREEN)

        # Grid
        for i in range(7):
            pos = i * 24
            x1, y1 = field_to_screen(pos - 72, -72)
            x2, y2 = field_to_screen(pos - 72, 72)
            pygame.draw.line(self.surface, GRID_LINE, (x1, y1), (x2, y2), 1)

            x1, y1 = field_to_screen(-72, pos - 72)
            x2, y2 = field_to_screen(72, pos - 72)
            pygame.draw.line(self.surface, GRID_LINE, (x1, y1), (x2, y2), 1)

        self._draw_axes_overlay()


# ============================================================================
# PATH EDITOR
# ============================================================================

class PathEditor:
    """Interactive path editor with waypoint management."""

    def __init__(self, surface):
        self.surface = surface
        self.waypoints: List[Waypoint] = []
        self.selected_waypoint: Optional[int] = None
        self.hover_waypoint: Optional[int] = None
        self.dragging = False
        self.snap_enabled = True
        self.show_curves = True  # Toggle smooth curves
        self.show_control_points = False  # Show Bézier control points

    def add_waypoint(self, x: float, y: float):
        """Add waypoint at field coordinates."""
        if self.snap_enabled:
            x, y = snap_to_grid(x, y, SNAP_GRID)

        wp = Waypoint(x=x, y=y)
        self.waypoints.append(wp)
        print(f"[Path] Added waypoint {len(self.waypoints)}: ({x:.1f}, {y:.1f})")

    def remove_waypoint(self, index: int):
        """Remove waypoint by index."""
        if 0 <= index < len(self.waypoints):
            wp = self.waypoints.pop(index)
            print(f"[Path] Removed waypoint: ({wp.x:.1f}, {wp.y:.1f})")

    def find_waypoint_at(self, screen_x: int, screen_y: int) -> Optional[int]:
        """Find waypoint index at screen position."""
        for i, wp in enumerate(self.waypoints):
            wp_screen_x, wp_screen_y = field_to_screen(wp.x, wp.y)
            dist = np.sqrt((screen_x - wp_screen_x)**2 + (screen_y - wp_screen_y)**2)
            if dist <= WAYPOINT_RADIUS + 5:
                return i
        return None

    def move_waypoint(self, index: int, x: float, y: float):
        """Move waypoint to new field coordinates."""
        if 0 <= index < len(self.waypoints):
            if self.snap_enabled:
                x, y = snap_to_grid(x, y, SNAP_GRID)
            self.waypoints[index].x = x
            self.waypoints[index].y = y

    def draw(self):
        """Draw path and waypoints."""
        if len(self.waypoints) < 2:
            self._draw_waypoints()
            return

        if self.show_curves:
            # Draw smooth Bézier curves
            self._draw_curved_path()
        else:
            # Draw straight lines
            points = [field_to_screen(wp.x, wp.y) for wp in self.waypoints]
            pygame.draw.lines(self.surface, PATH_COLOR, False, points, 3)

        # Draw waypoints
        self._draw_waypoints()

    def _draw_curved_path(self):
        """Draw smooth Bézier curves between waypoints."""
        control_points = calculate_control_points(self.waypoints)

        # Draw each curve segment
        for p0, p1, p2, p3 in control_points:
            # Calculate Bézier curve points
            curve_points_field = calculate_bezier_curve(p0, p1, p2, p3, num_points=30)
            curve_points_screen = [field_to_screen(x, y) for x, y in curve_points_field]

            # Draw the curve
            if len(curve_points_screen) > 1:
                pygame.draw.lines(self.surface, PATH_COLOR, False, curve_points_screen, 3)

        # Optionally show control points for debugging
        if self.show_control_points:
            for p0, p1, p2, p3 in control_points:
                # Draw control point lines (light gray)
                p0_screen = field_to_screen(p0[0], p0[1])
                p1_screen = field_to_screen(p1[0], p1[1])
                p2_screen = field_to_screen(p2[0], p2[1])
                p3_screen = field_to_screen(p3[0], p3[1])

                pygame.draw.line(self.surface, (150, 150, 150), p0_screen, p1_screen, 1)
                pygame.draw.line(self.surface, (150, 150, 150), p2_screen, p3_screen, 1)

                # Draw control points (small circles)
                pygame.draw.circle(self.surface, (150, 150, 150), p1_screen, 3)
                pygame.draw.circle(self.surface, (150, 150, 150), p2_screen, 3)

    def _draw_waypoints(self):
        """Draw individual waypoints."""
        for i, wp in enumerate(self.waypoints):
            screen_x, screen_y = field_to_screen(wp.x, wp.y)

            # Color based on state
            if i == self.selected_waypoint:
                color = (255, 0, 0)  # Red for selected
            elif i == self.hover_waypoint:
                color = WAYPOINT_HOVER
            else:
                color = WAYPOINT_COLOR

            # Draw waypoint
            pygame.draw.circle(self.surface, color, (screen_x, screen_y), WAYPOINT_RADIUS)
            pygame.draw.circle(self.surface, (0, 0, 0), (screen_x, screen_y), WAYPOINT_RADIUS, 2)

            # Draw number
            font = pygame.font.Font(None, 16)
            if i == 0:
                label = "S"  # Start
            else:
                label = str(i)
            text = font.render(label, True, (0, 0, 0))
            text_rect = text.get_rect(center=(screen_x, screen_y))
            self.surface.blit(text, text_rect)

    def clear(self):
        """Clear all waypoints."""
        self.waypoints = []
        self.selected_waypoint = None
        print("[Path] Cleared all waypoints")

    def save_to_file(self, filepath: str):
        """Save path to JSON file."""
        data = {
            'waypoints': [wp.to_dict() for wp in self.waypoints],
            'created': datetime.now().isoformat()
        }
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)
        print(f"[Path] Saved to: {filepath}")

    def load_from_file(self, filepath: str):
        """Load path from JSON file."""
        with open(filepath, 'r') as f:
            data = json.load(f)
        self.waypoints = [Waypoint.from_dict(wp) for wp in data['waypoints']]
        print(f"[Path] Loaded {len(self.waypoints)} waypoints from: {filepath}")


# ============================================================================
# INTERACTIVE PATH PLANNER
# ============================================================================

class InteractivePathPlanner:
    """Main interactive path planning application."""

    def __init__(self, field_image: Optional[str] = None):
        pygame.init()

        self.screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE + 200))
        pygame.display.set_caption("VEX Interactive Path Planner")

        self.field_surface = pygame.Surface((WINDOW_SIZE, WINDOW_SIZE))
        self.field_renderer = FieldRenderer(self.field_surface, field_image)
        self.path_editor = PathEditor(self.field_surface)
        self.physics = SimplePhysics()

        # Simulation state
        self.simulating = False
        self.sim_waypoint_index = 0
        self.sim_time = 0.0

        # UI state
        self.show_code = False

        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 20)
        self.font_small = pygame.font.Font(None, 16)

        print("[Planner] Interactive Path Planner initialized")
        print("[Planner] Click to place waypoints, drag to move, right-click to delete")

    def handle_input(self):
        """Handle user input."""
        mouse_pos = pygame.mouse.get_pos()

        # Update hover state
        if mouse_pos[1] < WINDOW_SIZE:
            self.path_editor.hover_waypoint = self.path_editor.find_waypoint_at(mouse_pos[0], mouse_pos[1])

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False

            elif event.type == pygame.KEYDOWN:
                if event.key in (pygame.K_q, pygame.K_ESCAPE):
                    return False
                elif event.key == pygame.K_SPACE:
                    self.toggle_simulation()
                elif event.key == pygame.K_r:
                    self.path_editor.clear()
                    self.stop_simulation()
                elif event.key == pygame.K_c:
                    self.copy_code()
                elif event.key == pygame.K_e:
                    self.export_code()
                elif event.key == pygame.K_w:
                    self.write_to_auton()
                elif event.key == pygame.K_s:
                    self.save_path()
                elif event.key == pygame.K_l:
                    self.load_path()
                elif event.key == pygame.K_g:
                    self.path_editor.snap_enabled = not self.path_editor.snap_enabled
                    status = "ON" if self.path_editor.snap_enabled else "OFF"
                    print(f"[Planner] Grid snap: {status}")
                elif event.key == pygame.K_b:
                    self.path_editor.show_curves = not self.path_editor.show_curves
                    status = "SMOOTH" if self.path_editor.show_curves else "STRAIGHT"
                    print(f"[Planner] Path mode: {status}")
                elif event.key == pygame.K_v:
                    self.path_editor.show_control_points = not self.path_editor.show_control_points
                    status = "ON" if self.path_editor.show_control_points else "OFF"
                    print(f"[Planner] Control points: {status}")
                elif event.key == pygame.K_LEFTBRACKET:
                    # Decrease curve smoothness for selected waypoint
                    if self.path_editor.selected_waypoint is not None:
                        idx = self.path_editor.selected_waypoint
                        self.path_editor.waypoints[idx].curve_smooth = max(0.0, self.path_editor.waypoints[idx].curve_smooth - 0.1)
                        print(f"[Planner] Waypoint {idx} smoothness: {self.path_editor.waypoints[idx].curve_smooth:.1f}")
                    elif self.path_editor.waypoints:
                        # Adjust all waypoints
                        for wp in self.path_editor.waypoints:
                            wp.curve_smooth = max(0.0, wp.curve_smooth - 0.1)
                        print(f"[Planner] All waypoints smoothness decreased")
                elif event.key == pygame.K_RIGHTBRACKET:
                    # Increase curve smoothness for selected waypoint
                    if self.path_editor.selected_waypoint is not None:
                        idx = self.path_editor.selected_waypoint
                        self.path_editor.waypoints[idx].curve_smooth = min(1.0, self.path_editor.waypoints[idx].curve_smooth + 0.1)
                        print(f"[Planner] Waypoint {idx} smoothness: {self.path_editor.waypoints[idx].curve_smooth:.1f}")
                    elif self.path_editor.waypoints:
                        # Adjust all waypoints
                        for wp in self.path_editor.waypoints:
                            wp.curve_smooth = min(1.0, wp.curve_smooth + 0.1)
                        print(f"[Planner] All waypoints smoothness increased")

            elif event.type == pygame.MOUSEBUTTONDOWN:
                if mouse_pos[1] >= WINDOW_SIZE:
                    continue

                if event.button == 1:  # Left click
                    waypoint_index = self.path_editor.find_waypoint_at(mouse_pos[0], mouse_pos[1])
                    if waypoint_index is not None:
                        # Start dragging
                        self.path_editor.selected_waypoint = waypoint_index
                        self.path_editor.dragging = True
                    else:
                        # Add new waypoint
                        field_x, field_y = screen_to_field(mouse_pos[0], mouse_pos[1])
                        self.path_editor.add_waypoint(field_x, field_y)
                        self.stop_simulation()

                elif event.button == 3:  # Right click
                    waypoint_index = self.path_editor.find_waypoint_at(mouse_pos[0], mouse_pos[1])
                    if waypoint_index is not None:
                        self.path_editor.remove_waypoint(waypoint_index)
                        self.stop_simulation()

            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1:
                    self.path_editor.dragging = False
                    self.path_editor.selected_waypoint = None

            elif event.type == pygame.MOUSEMOTION:
                if self.path_editor.dragging and self.path_editor.selected_waypoint is not None:
                    field_x, field_y = screen_to_field(mouse_pos[0], mouse_pos[1])
                    self.path_editor.move_waypoint(self.path_editor.selected_waypoint, field_x, field_y)
                    self.stop_simulation()

        return True

    def toggle_simulation(self):
        """Start/stop simulation."""
        if not self.path_editor.waypoints:
            return

        if self.simulating:
            self.stop_simulation()
        else:
            self.start_simulation()

    def start_simulation(self):
        """Start simulating the path."""
        if not self.path_editor.waypoints:
            return

        self.simulating = True
        self.sim_waypoint_index = 0
        wp = self.path_editor.waypoints[0]
        self.physics.set_pose(wp.x, wp.y, 0)

        if len(self.path_editor.waypoints) > 1:
            next_wp = self.path_editor.waypoints[1]
            self.physics.move_to(next_wp.x, next_wp.y)

        print("[Sim] Started")

    def stop_simulation(self):
        """Stop simulation."""
        self.simulating = False
        self.sim_waypoint_index = 0
        print("[Sim] Stopped")

    def update_simulation(self):
        """Update simulation state."""
        if not self.simulating:
            return

        self.physics.update(TIME_STEP)

        # Check if reached current waypoint
        if not self.physics.is_moving:
            self.sim_waypoint_index += 1

            if self.sim_waypoint_index < len(self.path_editor.waypoints):
                next_wp = self.path_editor.waypoints[self.sim_waypoint_index]
                self.physics.move_to(next_wp.x, next_wp.y)
            else:
                # Finished
                self.stop_simulation()

    def draw_robot(self):
        """Draw robot sprite during simulation."""
        if not self.simulating:
            return

        state = self.physics.state
        screen_x, screen_y = field_to_screen(state.x, state.y)
        robot_size = inches_to_pixels(18)

        # Robot body
        robot_rect = pygame.Rect(
            screen_x - robot_size // 2,
            screen_y - robot_size // 2,
            robot_size, robot_size
        )
        pygame.draw.rect(self.field_surface, ROBOT_COLOR, robot_rect, 3)

        # Heading arrow
        arrow_len = robot_size // 2
        theta_rad = np.radians(state.theta)
        arrow_end_x = screen_x + arrow_len * np.cos(theta_rad)
        arrow_end_y = screen_y - arrow_len * np.sin(theta_rad)
        pygame.draw.line(self.field_surface, ROBOT_COLOR,
                        (screen_x, screen_y), (arrow_end_x, arrow_end_y), 3)

    def draw_hud(self):
        """Draw HUD with controls and info."""
        hud_y = WINDOW_SIZE + 10
        hud_rect = pygame.Rect(0, WINDOW_SIZE, WINDOW_SIZE, 200)
        pygame.draw.rect(self.screen, BACKGROUND, hud_rect)

        # Title
        title = self.font.render("VEX Interactive Path Planner", True, (255, 215, 0))
        self.screen.blit(title, (10, hud_y))

        # Waypoint count and mode
        mode = "CURVED" if self.path_editor.show_curves else "STRAIGHT"
        wp_count = self.font_small.render(
            f"Waypoints: {len(self.path_editor.waypoints)}  |  Mode: {mode}  |  Grid: {'ON' if self.path_editor.snap_enabled else 'OFF'}",
            True, TEXT_COLOR
        )
        self.screen.blit(wp_count, (10, hud_y + 25))

        # Controls
        controls = [
            "CLICK: Place  |  DRAG: Move  |  RIGHT-CLICK: Delete  |  SPACE: Simulate",
            "B: Toggle curves  |  [ ]: Adjust smoothness  |  V: Show control points",
            "C: Show code  |  W: Write to auton.cpp  |  R: Reset  |  Q: Quit"
        ]

        for i, line in enumerate(controls):
            text = self.font_small.render(line, True, (150, 150, 150))
            self.screen.blit(text, (10, hud_y + 50 + i * 20))

        # Simulation status
        if self.simulating:
            sim_text = self.font_small.render(
                f"⚡ SIMULATING - Waypoint {self.sim_waypoint_index + 1}/{len(self.path_editor.waypoints)}",
                True, (0, 255, 0)
            )
            self.screen.blit(sim_text, (10, hud_y + 120))

        # Code preview
        if len(self.path_editor.waypoints) > 0:
            code_preview = self.font_small.render(
                f"💾 Ready to export {len(self.path_editor.waypoints)} waypoints to C++",
                True, (100, 149, 237)
            )
            self.screen.blit(code_preview, (10, hud_y + 145))

    def copy_code(self):
        """Display C++ code for copying."""
        if not self.path_editor.waypoints:
            print("[Code] No waypoints to generate code from")
            return

        code = CodeGenerator.generate_cpp(self.path_editor.waypoints, "myGeneratedRoute")

        print("\n" + "="*80)
        print("C++ CODE - Copy this to src/auton.cpp:")
        print("="*80)
        print(code)
        print("="*80)
        print("\nINSTRUCTIONS:")
        print("1. Select and copy the code above")
        print("2. Open src/auton.cpp in your editor")
        print("3. Paste before the closing } of namespace subsystems")
        print("4. Add declaration in include/robot/auton.hpp:")
        print("   void myGeneratedRoute();")
        print("5. Call it from main.cpp autonomous() function:")
        print("   auton.myGeneratedRoute();")
        print("="*80 + "\n")

    def export_code(self):
        """Export C++ code to file."""
        if not self.path_editor.waypoints:
            print("[Export] No waypoints to export")
            return

        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"generated_route_{timestamp}.cpp"

        code = CodeGenerator.generate_cpp(self.path_editor.waypoints, "generatedRoute")

        with open(filename, 'w') as f:
            f.write(code)

        print(f"[Export] Saved to: {filename}")

    def write_to_auton(self):
        """Write code directly to auton.cpp file."""
        if not self.path_editor.waypoints:
            print("[Write] No waypoints to write")
            return

        auton_file = "../src/auton.cpp"
        if not os.path.exists(auton_file):
            # Try alternative path
            auton_file = "src/auton.cpp"
            if not os.path.exists(auton_file):
                print("[Write] Error: Could not find src/auton.cpp")
                print("[Write] Use 'E' to export to a file instead")
                return

        # Generate code
        route_name = f"visualRoute_{datetime.now().strftime('%m%d_%H%M')}"
        code = CodeGenerator.generate_cpp(self.path_editor.waypoints, route_name)

        # Read existing file
        with open(auton_file, 'r') as f:
            lines = f.readlines()

        # Find insertion point (before closing namespace)
        insert_idx = -1
        for i in range(len(lines) - 1, -1, -1):
            if "} // namespace subsystems" in lines[i]:
                insert_idx = i
                break

        if insert_idx == -1:
            print("[Write] Error: Could not find '} // namespace subsystems' in auton.cpp")
            return

        # Insert code
        lines.insert(insert_idx, "\n" + code + "\n")

        # Write back
        with open(auton_file, 'w') as f:
            f.writelines(lines)

        print("\n" + "="*80)
        print(f"[Write] ✅ Successfully added {route_name}() to src/auton.cpp!")
        print("="*80)
        print("\nNEXT STEPS:")
        print(f"1. Add to include/robot/auton.hpp:")
        print(f"   void {route_name}();")
        print(f"\n2. Call from main.cpp autonomous():")
        print(f"   auton.{route_name}();")
        print("\n3. Build and upload:")
        print("   make && pros upload")
        print("="*80 + "\n")

    def save_path(self):
        """Save path to file."""
        if not self.path_editor.waypoints:
            print("[Save] No waypoints to save")
            return

        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"path_{timestamp}.json"
        self.path_editor.save_to_file(filename)

    def load_path(self):
        """Load path from file (loads most recent)."""
        json_files = sorted(Path('.').glob('path_*.json'), reverse=True)
        if not json_files:
            print("[Load] No saved paths found")
            return

        self.path_editor.load_from_file(str(json_files[0]))
        self.stop_simulation()

    def draw(self):
        """Draw everything."""
        self.field_renderer.draw()
        self.path_editor.draw()
        self.draw_robot()
        self.screen.blit(self.field_surface, (0, 0))
        self.draw_hud()
        pygame.display.flip()

    def run(self):
        """Main loop."""
        print("\n" + "="*80)
        print("INTERACTIVE PATH PLANNER - Game-Changing Autonomous Route Designer")
        print("="*80)
        print("Inspired by PATH.JERRYIO")
        print("="*80 + "\n")

        running = True
        while running:
            running = self.handle_input()
            self.update_simulation()
            self.draw()
            self.clock.tick(FPS)

        pygame.quit()


# ============================================================================
# MAIN
# ============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="VEX Interactive Path Planner - Game-changing autonomous route designer"
    )

    parser.add_argument('--field', type=str, help='Path to field image')

    args = parser.parse_args()

    if args.field and not os.path.exists(args.field):
        print(f"[Warning] Field image not found: {args.field}")
        args.field = None

    planner = InteractivePathPlanner(args.field)
    planner.run()


if __name__ == "__main__":
    main()
