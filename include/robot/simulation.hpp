#pragma once

#include "lemlib/api.hpp"
#include "pros/motors.hpp"
#include <cstdint>

namespace subsystems {

/**
 * @brief Simulation system for testing autonomous without physical robot
 *
 * Features:
 * - Mock sensor data (IMU, motors, odometry)
 * - Virtual chassis that follows LemLib commands
 * - Console visualization of robot position
 * - Deterministic physics for reproducible testing
 */
class Simulation {
public:
    Simulation(lemlib::Chassis* chassis);

    /**
     * @brief Enable or disable simulation mode
     * @param enabled true = use mock data, false = use real hardware
     */
    void setEnabled(bool enabled);

    /**
     * @brief Check if simulation is active
     */
    bool isEnabled() const;

    /**
     * @brief Update simulation state (call every 10ms in background task)
     *
     * Updates virtual robot position based on chassis commands.
     * Simulates motor velocities, IMU heading, and odometry.
     */
    void update();

    /**
     * @brief Reset simulation to starting position
     */
    void reset();

    /**
     * @brief Print current simulation state to console
     *
     * Displays:
     * - Robot position (X, Y, heading)
     * - Motor velocities
     * - Target position (if moving)
     */
    void printState();

    /**
     * @brief Draw ASCII field map showing robot position
     *
     * Visualizes 144" x 144" VEX field with robot location.
     */
    void drawField();

    /**
     * @brief Validate simulation accuracy against real robot data
     *
     * Tests:
     * - 48" drive in ~1.8s @ 60% speed
     * - Prints timing comparison
     */
    void validateCalibration();

    // ========================================================================
    // MOCK SENSOR DATA (used when simulation enabled)
    // ========================================================================

    /**
     * @brief Get simulated IMU heading
     */
    float getMockHeading();

    /**
     * @brief Get simulated motor temperature
     * @param motor_index 0-5 for drivetrain motors
     */
    float getMockMotorTemp(int motor_index);

    /**
     * @brief Get simulated motor current draw
     * @param motor_index 0-5 for drivetrain motors
     */
    float getMockMotorCurrent(int motor_index);

    /**
     * @brief Get simulated battery voltage (mV)
     */
    uint32_t getMockBatteryVoltage();

private:
    lemlib::Chassis* chassis;
    bool enabled;

    // Virtual robot state
    float virtualX;          // Robot X position (inches)
    float virtualY;          // Robot Y position (inches)
    float virtualHeading;    // Robot heading (degrees)
    float virtualVelocity;   // Linear velocity (in/s)
    float virtualAngVel;     // Angular velocity (deg/s)

    // Target tracking
    float targetX;
    float targetY;
    float targetHeading;
    bool isMoving;

    // Simulation physics constants (calibrated from real robot data)
    // Real test: 48" in 1.8s @ 60% speed → avg 26.67 in/s
    static constexpr float MAX_LINEAR_VEL = 75.0;   // inches/sec (100% speed)
    static constexpr float MAX_ANGULAR_VEL = 180.0; // degrees/sec
    static constexpr float LINEAR_ACCEL = 50.0;     // in/s^2 (calibrated for 1.8s @ 48")
    static constexpr float ANGULAR_ACCEL = 360.0;   // deg/s^2

    // Mock sensor values
    float mockMotorTemps[6];
    float mockMotorCurrents[6];
    uint32_t mockBatteryVoltage;
    uint32_t simulationStartTime;

    /**
     * @brief Update virtual robot position based on velocity
     * @param dt Time step in seconds
     */
    void updatePhysics(float dt);

    /**
     * @brief Check if chassis is currently executing a motion command
     */
    void updateMovementTarget();
};

} // namespace subsystems
