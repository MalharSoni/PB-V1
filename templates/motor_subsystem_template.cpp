// ============================================================================
// MOTOR SUBSYSTEM TEMPLATE - Copy-Paste Example
// ============================================================================
// This template shows how to create a custom motor subsystem by extending
// lib::MotorSubsystem. Use this for ANY motor-driven mechanism in ANY game.
//
// WHEN TO USE:
// - Your mechanism needs custom behavior beyond basic motor control
// - You have game-specific logic (sensors, algorithms, etc.)
// - You want to encapsulate complex operations
//
// WHEN NOT TO USE:
// - For simple motor control, just use lib::MotorSubsystem directly
// - Don't create unnecessary classes
//
// STEPS:
// 1. Copy this template to include/robot/your_mechanism.hpp
// 2. Rename "MyMechanism" to your actual mechanism name
// 3. Add game-specific methods and members
// 4. Implement in src/subsystems/your_mechanism.cpp
// ============================================================================

#pragma once
#include "lib/MotorSubsystem.hpp"
#include "pros/motors.hpp"
#include "pros/optical.hpp"  // Example: color sensor
#include "pros/adi.hpp"       // Example: limit switch

namespace subsystems {

// ============================================================================
// EXAMPLE 1: Simple Flywheel (Minimal Extension)
// ============================================================================
class Flywheel : public lib::MotorSubsystem {
public:
    Flywheel(std::vector<pros::Motor> motors)
        : lib::MotorSubsystem(motors) {}

    // Game-specific methods
    void spinUp() {
        move(12000);  // Use inherited move() method
    }

    void spinDown() {
        stop();  // Use inherited stop() method
    }

    bool atSpeed() {
        return getVelocity() > 500;  // Use inherited getVelocity()
    }
};

// ============================================================================
// EXAMPLE 2: Intake with Color Sorting (Complex Extension)
// ============================================================================
class ColorSortingIntake : public lib::MotorSubsystem {
public:
    enum class ObjectColor { NONE, RED, BLUE };

    ColorSortingIntake(std::vector<pros::Motor> motors, int sensor_port, int limit_port)
        : lib::MotorSubsystem(motors),
          color_sensor(sensor_port),
          limit_switch(limit_port) {}

    // ========================================================================
    // MOTOR CONTROL (inherited from lib::MotorSubsystem)
    // ========================================================================
    // Available methods:
    //   - move(int voltage)
    //   - moveAbsolute(float position, float speed)
    //   - moveRelative(float delta, float speed)
    //   - stop()
    //   - getPosition()
    //   - getVelocity()
    //   - getTemperature()
    //   - setBrakeMode(mode)

    // ========================================================================
    // GAME-SPECIFIC METHODS
    // ========================================================================

    void intake() {
        move(12000);  // Full speed forward
    }

    void outtake() {
        move(-12000);  // Full speed backward
    }

    void setTargetColor(ObjectColor color) {
        target_color = color;
    }

    bool hasObject() {
        return limit_switch.get_value() == 1;
    }

    ObjectColor detectColor() {
        int hue = color_sensor.get_hue();
        if (hue >= 0 && hue <= 30) return ObjectColor::RED;
        if (hue >= 200 && hue <= 240) return ObjectColor::BLUE;
        return ObjectColor::NONE;
    }

    // Game-specific: Reject wrong color objects
    void startColorSorting() {
        if (detectColor() != target_color && detectColor() != ObjectColor::NONE) {
            outtake();
            pros::delay(200);
            intake();
        }
    }

private:
    pros::Optical color_sensor;
    pros::ADIDigitalIn limit_switch;
    ObjectColor target_color = ObjectColor::NONE;
};

// ============================================================================
// EXAMPLE 3: Lift with Position Presets (Moderate Extension)
// ============================================================================
class Lift : public lib::MotorSubsystem {
public:
    enum Position {
        GROUND = 0,
        LOW_GOAL = 500,
        MID_GOAL = 1000,
        HIGH_GOAL = 1500
    };

    Lift(std::vector<pros::Motor> motors)
        : lib::MotorSubsystem(motors) {
        setBrakeMode(pros::E_MOTOR_BRAKE_HOLD);  // Hold position when stopped
    }

    // Game-specific position control
    void goToGround() {
        moveAbsolute(GROUND, 127);  // Use inherited method
    }

    void goToLowGoal() {
        moveAbsolute(LOW_GOAL, 127);
    }

    void goToMidGoal() {
        moveAbsolute(MID_GOAL, 127);
    }

    void goToHighGoal() {
        moveAbsolute(HIGH_GOAL, 127);
    }

    // Manual control
    void manualUp() {
        move(8000);
    }

    void manualDown() {
        move(-8000);
    }

    void manualStop() {
        stop();
    }
};

} // namespace subsystems

// ============================================================================
// IMPLEMENTATION EXAMPLE (in .cpp file)
// ============================================================================
/*
#include "robot/my_mechanism.hpp"
#include "globals.hpp"

namespace subsystems {

// Constructor implementation (if complex)
ColorSortingIntake::ColorSortingIntake(...)
    : lib::MotorSubsystem(motors),
      color_sensor(sensor_port),
      limit_switch(limit_port) {
    // Additional setup if needed
}

// Method implementations
void ColorSortingIntake::startColorSorting() {
    while (true) {
        intake();

        if (detectColor() != target_color && detectColor() != ObjectColor::NONE) {
            outtake();
            pros::delay(200);
            intake();
        }

        pros::delay(10);
    }
}

} // namespace subsystems
*/

// ============================================================================
// USAGE IN GLOBALS.CPP
// ============================================================================
/*
// Motor setup
pros::Motor intakeMotor1(INTAKE_PORT_1, pros::E_MOTOR_GEARSET_18);
pros::Motor intakeMotor2(INTAKE_PORT_2, pros::E_MOTOR_GEARSET_18);

// Subsystem instantiation
subsystems::ColorSortingIntake intake(
    {intakeMotor1, intakeMotor2},  // Motors
    COLOR_SENSOR_PORT,              // Sensor port
    LIMIT_SWITCH_PORT               // Limit switch port
);
*/

// ============================================================================
// USAGE IN MAIN.CPP
// ============================================================================
/*
void opcontrol() {
    // Set alliance color at start
    intake.setTargetColor(ColorSortingIntake::ObjectColor::RED);

    while (true) {
        // Simple controls
        if (master.get_digital(DIGITAL_R1)) {
            intake.intake();
        } else if (master.get_digital(DIGITAL_L1)) {
            intake.outtake();
        } else {
            intake.stop();
        }

        // Color sorting
        if (master.get_digital(DIGITAL_A)) {
            intake.startColorSorting();
        }

        pros::delay(10);
    }
}
*/

// ============================================================================
// KEY PRINCIPLES
// ============================================================================
// 1. ALWAYS extend lib::MotorSubsystem for motor-driven mechanisms
// 2. Use inherited methods (move, stop, getPosition, etc.) for basic control
// 3. Add game-specific methods for custom behavior
// 4. Keep it SIMPLE - don't over-engineer
// 5. Document your game-specific methods clearly
// 6. Use descriptive names that make sense for YOUR game
