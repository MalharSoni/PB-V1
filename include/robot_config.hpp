#pragma once

// ============================================================================
// ROBOT CONFIGURATION SYSTEM
// ============================================================================
// This file provides a centralized initialization system for all robot hardware.
//
// WHY THIS EXISTS:
// - Single location for all robot setup
// - Easy to understand hardware configuration
// - Simple to modify for new seasons
// - Clear separation: port definitions (globals.hpp) vs hardware (robot_config.cpp)
//
// USAGE:
//   #include "robot_config.hpp"
//
//   int main() {
//       robot_config::initialize();  // Setup all hardware
//       // ... rest of your code
//   }
//
// FOR NEW SEASONS:
// 1. Update port definitions in globals.hpp
// 2. Modify robot_config.cpp hardware instantiation
// 3. Keep the same initialization pattern
// ============================================================================

namespace robot_config {

/**
 * @brief Initialize all robot hardware and subsystems
 *
 * Call this ONCE at the start of main() before any other robot operations.
 *
 * Initialization order:
 * 1. Controller
 * 2. Drivetrain motors
 * 3. Sensors (IMU, tracking wheels)
 * 4. LemLib chassis
 * 5. Subsystems (Intake, Arm, etc.)
 * 6. Pneumatics
 *
 * @note This function blocks until IMU calibration completes
 */
void initialize();

/**
 * @brief Calibrate the IMU sensor
 *
 * Called automatically by initialize().
 * Displays status on controller screen.
 *
 * @note Robot must be stationary during calibration
 */
void calibrate_imu();

/**
 * @brief Check if IMU is healthy and functioning
 *
 * Use this to detect IMU failures during operation.
 * Can be called periodically to monitor IMU health.
 *
 * @return true if IMU is connected and working
 * @return false if IMU has an error or is disconnected
 */
bool check_imu_status();

} // namespace robot_config
