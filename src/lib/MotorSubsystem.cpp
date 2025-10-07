#include "lib/MotorSubsystem.hpp"

namespace lib {

MotorSubsystem::MotorSubsystem(std::vector<pros::Motor> imotors)
    : motors(imotors) {}

// ============================================================================
// BASIC MOTOR CONTROL
// ============================================================================

void MotorSubsystem::move(int voltage) {
    motors.move_voltage(voltage);
}

void MotorSubsystem::moveAbsolute(float position, float speed) {
    motors.move_absolute(position, speed);
}

void MotorSubsystem::moveRelative(float delta, float speed) {
    motors.move_relative(delta, speed);
}

void MotorSubsystem::stop() {
    motors.move_voltage(0);
}

// ============================================================================
// STATE QUERIES
// ============================================================================

float MotorSubsystem::getPosition() {
    // Return position of first motor in the group
    auto positions = motors.get_positions();
    return positions.empty() ? 0.0f : positions.at(0);
}

float MotorSubsystem::getVelocity() {
    // Return velocity of first motor in the group
    auto velocities = motors.get_actual_velocities();
    return velocities.empty() ? 0.0f : velocities.at(0);
}

float MotorSubsystem::getTemperature() {
    // Return temperature of first motor in the group
    auto temperatures = motors.get_temperatures();
    return temperatures.empty() ? 0.0f : temperatures.at(0);
}

bool MotorSubsystem::isMoving() {
    float velocity = getVelocity();
    return (velocity > VELOCITY_THRESHOLD || velocity < -VELOCITY_THRESHOLD);
}

// ============================================================================
// CONFIGURATION
// ============================================================================

void MotorSubsystem::setBrakeMode(pros::motor_brake_mode_e mode) {
    motors.set_brake_modes(mode);
}

void MotorSubsystem::setZeroPosition(float position) {
    motors.set_zero_position(position);
}

} // namespace lib
