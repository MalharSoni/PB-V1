#pragma once

/**
 * @file telemetry_adapter.hpp
 * @brief Adapter layer for LemLib odometry and sensor data
 *
 * *** THIS IS THE ONLY FILE THAT TOUCHES LemLib ***
 *
 * Modify this file to match your LemLib fork's API.
 * All other telemetry code is fork-agnostic.
 */

#include "lemlib/api.hpp"
#include "globals.hpp"
#include "pros/misc.h"

namespace telem {

// ============================================================================
// POSE DATA
// ============================================================================

struct Pose {
    double x;       // X position (inches)
    double y;       // Y position (inches)
    double theta;   // Heading (degrees)
};

/**
 * @brief Get current robot pose from LemLib
 * @return Pose struct with x, y, theta
 *
 * TODO: Update this to match your LemLib fork's getPose() method
 */
inline Pose get_pose() {
    lemlib::Pose pose = chassis.getPose();
    return {pose.x, pose.y, pose.theta};
}

// ============================================================================
// VELOCITY DATA
// ============================================================================

/**
 * @brief Get left and right wheel velocities
 * @param vl Output: left wheel velocity (inches per second)
 * @param vr Output: right wheel velocity (inches per second)
 *
 * TODO: Update this to match your drivetrain configuration
 *
 * Current implementation estimates from motor RPM:
 * - velocity (ips) = RPM * wheel_circumference / 60
 * - wheel_circumference = π * diameter
 */
inline void get_wheel_vel(double& vl, double& vr) {
    // Get average motor velocities (RPM)
    float lf_vel = leftFrontMotor.get_actual_velocity();
    float lm_vel = leftMidMotor.get_actual_velocity();
    float rf_vel = rightFrontMotor.get_actual_velocity();
    float rm_vel = rightMidMotor.get_actual_velocity();

    float left_rpm = (lf_vel + lm_vel) / 2.0;
    float right_rpm = (rf_vel + rm_vel) / 2.0;

    // Convert RPM to inches per second
    // Wheel diameter: 3.25" (from globals.cpp)
    const double wheel_diameter = 3.25;
    const double wheel_circumference = 3.14159265 * wheel_diameter;

    vl = left_rpm * wheel_circumference / 60.0;
    vr = right_rpm * wheel_circumference / 60.0;
}

// ============================================================================
// BATTERY DATA
// ============================================================================

/**
 * @brief Get battery voltage
 * @return Battery voltage in volts
 */
inline double get_batt_V() {
    return pros::battery::get_voltage() / 1000.0;  // millivolts → volts
}

// ============================================================================
// MARKER SYSTEM
// ============================================================================

static char marker_buf[32] = "";
static int marker_idx = 0;

/**
 * @brief Get current marker string (for waypoint tracking)
 * @return Marker string or empty string ""
 *
 * Markers are single-use: once read, they are cleared.
 */
inline const char* get_mark() {
    if (marker_buf[0] == '\0') {
        return "";
    }

    // Return marker and clear it
    static char temp[32];
    strncpy(temp, marker_buf, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    marker_buf[0] = '\0';  // Clear marker
    return temp;
}

/**
 * @brief Set a waypoint marker
 * Increments internal counter and creates "MARK:wp=N" tag
 */
inline void set_waypoint_marker() {
    marker_idx++;
    snprintf(marker_buf, sizeof(marker_buf), "MARK:wp=%d", marker_idx);
}

/**
 * @brief Set a custom marker
 * @param tag Custom tag string
 */
inline void set_custom_marker(const char* tag) {
    snprintf(marker_buf, sizeof(marker_buf), "MARK:%s", tag);
}

} // namespace telem
