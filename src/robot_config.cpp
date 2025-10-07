#include "robot_config.hpp"
#include "globals.hpp"
#include <iostream>

namespace robot_config {

// ============================================================================
// IMU CALIBRATION
// ============================================================================

void calibrate_imu() {
    std::cout << "Calibrating IMU..." << std::endl;
    master.print(0, 0, "IMU calibrating...");

    inertial.reset();

    // Wait for IMU to finish calibration
    int calibration_time = 0;
    while (inertial.is_calibrating()) {
        pros::delay(10);
        calibration_time += 10;

        // Timeout after 3 seconds
        if (calibration_time > 3000) {
            std::cout << "IMU calibration timeout!" << std::endl;
            master.print(0, 0, "IMU timeout!");
            return;
        }
    }

    std::cout << "IMU calibration complete (" << calibration_time << "ms)" << std::endl;
    master.print(0, 0, "IMU ready!");
    pros::delay(500);
    master.clear();
}

// ============================================================================
// MAIN INITIALIZATION
// ============================================================================

void initialize() {
    std::cout << "\n=== Robot Configuration Starting ===" << std::endl;

    // ========================================================================
    // 1. CONTROLLER
    // ========================================================================
    std::cout << "Controller: Ready" << std::endl;

    // ========================================================================
    // 2. DRIVETRAIN MOTORS
    // ========================================================================
    std::cout << "Drivetrain Motors: 6 motors configured" << std::endl;
    // Motors are already instantiated as globals
    // Left: " << LEFT_MOTOR_FRONT << ", " << LEFT_MOTOR_MID << ", " << LEFT_MOTOR_REAR
    // Right: " << RIGHT_MOTOR_FRONT << ", " << RIGHT_MOTOR_MID << ", " << RIGHT_MOTOR_REAR

    // ========================================================================
    // 3. SENSORS
    // ========================================================================
    std::cout << "Sensors:" << std::endl;
    std::cout << "  - IMU (Port " << INERTIAL << ")" << std::endl;
    std::cout << "  - Left Rotation (Port " << LEFT_ROTATION << ")" << std::endl;
    std::cout << "  - Rear Rotation (Port " << REAR_ROTATION << ")" << std::endl;

    // Calibrate IMU (blocks until complete)
    calibrate_imu();

    // ========================================================================
    // 4. LEMLIB CHASSIS
    // ========================================================================
    std::cout << "LemLib Chassis:" << std::endl;
    std::cout << "  - Track width: 9.9 inches" << std::endl;
    std::cout << "  - Wheel diameter: 3.25 inches" << std::endl;
    std::cout << "  - Lateral PID: kP=10, kD=1" << std::endl;
    std::cout << "  - Angular PID: kP=2.2, kD=10" << std::endl;

    // Calibrate chassis
    chassis.calibrate();
    std::cout << "Chassis: Calibrated" << std::endl;

    // ========================================================================
    // 5. SUBSYSTEMS
    // ========================================================================
    std::cout << "Subsystems:" << std::endl;
    std::cout << "  - Intake: 3 motors + color sensor" << std::endl;
    std::cout << "  - Arm: 1 motor + pneumatic" << std::endl;
    std::cout << "  - Movement: Initialized" << std::endl;
    std::cout << "  - Auton: Ready" << std::endl;
    std::cout << "  - Selector: Ready" << std::endl;
    std::cout << "  - DistanceAlign: 2 sensors" << std::endl;

    // ========================================================================
    // 6. PNEUMATICS (Game-Agnostic Components)
    // ========================================================================
    std::cout << "Pneumatics:" << std::endl;
    std::cout << "  - Clamp (Port " << CLAMP_PORT << ")" << std::endl;
    std::cout << "  - Doinker (Port " << DOINKER_PORT << ")" << std::endl;

    // ========================================================================
    // INITIALIZATION COMPLETE
    // ========================================================================
    std::cout << "=== Robot Configuration Complete ===" << std::endl;
    std::cout << std::endl;

    // Display ready message on controller
    master.print(0, 0, "Robot Ready!");
    pros::delay(1000);
    master.clear();
}

} // namespace robot_config
