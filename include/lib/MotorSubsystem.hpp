#pragma once

#include "pros/motors.hpp"
#include <vector>

// ============================================================================
// GENERIC MOTOR SUBSYSTEM BASE CLASS
// ============================================================================
// This is a universal base class for ANY motor-driven mechanism in ANY VEX game:
//   - Intakes (any game)
//   - Arms/Lifts (any game)
//   - Flywheels/Catapults (any game)
//   - Conveyors (any game)
//
// Provides common motor operations so students don't rewrite the same code.
//
// STUDENTS: Extend this class for game-specific behavior!
// Example:
//   class MyGameIntake : public lib::MotorSubsystem {
//       // Add game-specific methods here
//   };
// ============================================================================

namespace lib {

class MotorSubsystem {
public:
    /**
     * @brief Construct a motor subsystem
     *
     * @param motors Vector of PROS motors (supports 1 or more motors)
     *
     * Example:
     *   MotorSubsystem intake({motor1, motor2, motor3});
     */
    MotorSubsystem(std::vector<pros::Motor> motors);

    // ========================================================================
    // BASIC MOTOR CONTROL
    // ========================================================================

    /**
     * @brief Move motors at specified voltage
     *
     * @param voltage Voltage in millivolts (-12000 to 12000)
     *                Positive = forward, Negative = backward
     *
     * Example:
     *   move(12000);   // Full speed forward
     *   move(-6000);   // Half speed backward
     */
    void move(int voltage);

    /**
     * @brief Move to absolute position (encoder-based)
     *
     * @param position Target position in encoder ticks
     * @param speed Speed (0-127), defaults to 100
     *
     * Example:
     *   moveAbsolute(1800, 127);  // Move to 1800 ticks at full speed
     */
    void moveAbsolute(float position, float speed = 100);

    /**
     * @brief Move relative to current position
     *
     * @param delta Position change in encoder ticks
     * @param speed Speed (0-127), defaults to 100
     *
     * Example:
     *   moveRelative(360, 100);  // Move 360 ticks forward
     *   moveRelative(-180, 50);  // Move 180 ticks backward at half speed
     */
    void moveRelative(float delta, float speed = 100);

    /**
     * @brief Stop all motors immediately
     */
    void stop();

    // ========================================================================
    // STATE QUERIES
    // ========================================================================

    /**
     * @brief Get current position of first motor
     *
     * @return Position in encoder ticks
     *
     * Note: If multiple motors, returns position of first motor only
     */
    float getPosition();

    /**
     * @brief Get current velocity of first motor
     *
     * @return Velocity in RPM
     */
    float getVelocity();

    /**
     * @brief Get temperature of first motor
     *
     * @return Temperature in degrees Celsius
     *
     * Note: Useful for detecting overheating
     */
    float getTemperature();

    /**
     * @brief Check if motors are moving
     *
     * @return true if velocity > threshold, false if stopped
     */
    bool isMoving();

    // ========================================================================
    // CONFIGURATION
    // ========================================================================

    /**
     * @brief Set brake mode for all motors
     *
     * @param mode Brake mode:
     *   - pros::E_MOTOR_BRAKE_COAST (freewheel when stopped)
     *   - pros::E_MOTOR_BRAKE_BRAKE (slow down quickly)
     *   - pros::E_MOTOR_BRAKE_HOLD  (actively hold position)
     *
     * Example:
     *   setBrakeMode(pros::E_MOTOR_BRAKE_HOLD);  // Hold position
     */
    void setBrakeMode(pros::motor_brake_mode_e mode);

    /**
     * @brief Reset encoder position to zero (or specified value)
     *
     * @param position Value to set as "zero" (defaults to 0)
     *
     * Example:
     *   setZeroPosition();      // Reset to 0
     *   setZeroPosition(1000);  // Set current position as 1000
     */
    void setZeroPosition(float position = 0);

protected:
    /**
     * Motors controlled by this subsystem
     * Protected so derived classes can access if needed
     */
    pros::MotorGroup motors;

    /**
     * Velocity threshold for isMoving() check (RPM)
     */
    static constexpr float VELOCITY_THRESHOLD = 5.0;
};

} // namespace lib
