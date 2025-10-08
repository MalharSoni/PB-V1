#include "robot/simulation.hpp"
#include "pros/rtos.hpp"
#include <cmath>
#include <cstdio>

namespace subsystems {

Simulation::Simulation(lemlib::Chassis* chassis)
    : chassis(chassis), enabled(false),
      virtualX(0), virtualY(0), virtualHeading(0),
      virtualVelocity(0), virtualAngVel(0),
      targetX(0), targetY(0), targetHeading(0),
      isMoving(false), mockBatteryVoltage(12600),
      simulationStartTime(0) {

    // Initialize mock motor data
    for (int i = 0; i < 6; i++) {
        mockMotorTemps[i] = 35.0;      // 35°C starting temp
        mockMotorCurrents[i] = 0.0;    // No current at rest
    }
}

void Simulation::setEnabled(bool enabled) {
    this->enabled = enabled;

    if (enabled) {
        printf("\n[Simulation] ENABLED - Using mock sensor data\n");
        printf("[Simulation] Robot can be tested without hardware\n");
        simulationStartTime = pros::millis();
        reset();
    } else {
        printf("\n[Simulation] DISABLED - Using real hardware\n");
    }
}

bool Simulation::isEnabled() const {
    return enabled;
}

void Simulation::reset() {
    // Reset to chassis starting pose
    lemlib::Pose pose = chassis->getPose();
    virtualX = pose.x;
    virtualY = pose.y;
    virtualHeading = pose.theta;
    virtualVelocity = 0;
    virtualAngVel = 0;
    isMoving = false;

    printf("[Simulation] Reset to X:%.1f Y:%.1f H:%.1f\n",
           virtualX, virtualY, virtualHeading);
}

void Simulation::update() {
    if (!enabled) return;

    // Time step (assuming 10ms update rate)
    float dt = 0.01; // 10ms = 0.01 seconds

    // Update movement target from chassis commands
    updateMovementTarget();

    // Update physics simulation
    updatePhysics(dt);

    // Update mock sensor data
    uint32_t elapsedMs = pros::millis() - simulationStartTime;

    // Simulate motor heating during movement
    float velocityFactor = std::abs(virtualVelocity) / MAX_LINEAR_VEL;
    for (int i = 0; i < 6; i++) {
        // Temperature rises slowly during movement, cools when stopped
        if (isMoving) {
            mockMotorTemps[i] += 0.01 * velocityFactor;
            mockMotorTemps[i] = std::min(mockMotorTemps[i], 55.0f);
        } else {
            mockMotorTemps[i] -= 0.005;
            mockMotorTemps[i] = std::max(mockMotorTemps[i], 35.0f);
        }

        // Current draw proportional to velocity
        mockMotorCurrents[i] = 500.0 + (velocityFactor * 1500.0);
    }

    // Battery slowly drains (loses ~10mV per second)
    if (elapsedMs > 0) {
        mockBatteryVoltage = 12600 - (elapsedMs / 100); // 10mV per 100ms
        if (mockBatteryVoltage < 11000) mockBatteryVoltage = 11000;
    }

    // Update chassis pose with virtual position
    chassis->setPose(virtualX, virtualY, virtualHeading);
}

void Simulation::updatePhysics(float dt) {
    if (!isMoving) {
        // Decelerate to stop
        virtualVelocity *= 0.9;
        virtualAngVel *= 0.9;

        if (std::abs(virtualVelocity) < 0.1) virtualVelocity = 0;
        if (std::abs(virtualAngVel) < 0.1) virtualAngVel = 0;
        return;
    }

    // Calculate distance and angle to target
    float dx = targetX - virtualX;
    float dy = targetY - virtualY;
    float distToTarget = std::sqrt(dx * dx + dy * dy);
    float angleToTarget = std::atan2(dy, dx) * 180.0 / M_PI;

    // Heading error (for turning)
    float headingError = targetHeading - virtualHeading;
    while (headingError > 180) headingError -= 360;
    while (headingError < -180) headingError += 360;

    // Determine if we're turning in place or moving to point
    bool turningInPlace = (std::abs(headingError) > 5.0 && distToTarget < 1.0);

    if (turningInPlace) {
        // Pure rotation
        float desiredAngVel = headingError * 2.0; // Proportional control
        desiredAngVel = std::max(-MAX_ANGULAR_VEL, std::min(MAX_ANGULAR_VEL, desiredAngVel));

        // Accelerate towards desired angular velocity
        if (std::abs(desiredAngVel - virtualAngVel) > ANGULAR_ACCEL * dt) {
            virtualAngVel += (desiredAngVel > virtualAngVel ? 1 : -1) * ANGULAR_ACCEL * dt;
        } else {
            virtualAngVel = desiredAngVel;
        }

        virtualHeading += virtualAngVel * dt;
        while (virtualHeading > 180) virtualHeading -= 360;
        while (virtualHeading < -180) virtualHeading += 360;

    } else if (distToTarget > 0.5) {
        // Moving to point - use trapezoidal velocity profile
        // Acceleration phase -> constant velocity -> deceleration phase

        // Calculate deceleration distance needed to stop
        float decelDist = (virtualVelocity * virtualVelocity) / (2.0 * LINEAR_ACCEL);

        float maxVel = MAX_LINEAR_VEL;  // Default 100%
        // TODO: Read maxSpeed from LemLib if exposed

        if (distToTarget > decelDist * 2.0) {
            // Still accelerating or at max velocity
            float desiredVel = std::min(distToTarget * 8.0f, maxVel);

            // Accelerate
            if (virtualVelocity < desiredVel) {
                virtualVelocity += LINEAR_ACCEL * dt;
                virtualVelocity = std::min(virtualVelocity, desiredVel);
            }
        } else {
            // Deceleration phase - slow down to stop at target
            float desiredVel = std::sqrt(2.0 * LINEAR_ACCEL * distToTarget);
            desiredVel = std::min(desiredVel, virtualVelocity);

            // Decelerate
            virtualVelocity -= LINEAR_ACCEL * dt;
            virtualVelocity = std::max(virtualVelocity, desiredVel);
        }

        // Move towards target
        float moveAngle = angleToTarget;
        virtualX += virtualVelocity * std::cos(moveAngle * M_PI / 180.0) * dt;
        virtualY += virtualVelocity * std::sin(moveAngle * M_PI / 180.0) * dt;

        // Gradually turn towards target
        virtualHeading += headingError * 0.1 * dt;
        while (virtualHeading > 180) virtualHeading -= 360;
        while (virtualHeading < -180) virtualHeading += 360;

    } else {
        // Reached target
        isMoving = false;
        virtualVelocity = 0;
        virtualAngVel = 0;
    }
}

void Simulation::updateMovementTarget() {
    // NOTE: LemLib doesn't expose current target directly
    // This is a simplified approach - assumes chassis target changed if pose changed significantly

    // For now, we'll assume if the robot isn't at the current target, it's moving
    lemlib::Pose currentPose = chassis->getPose();

    float dx = targetX - currentPose.x;
    float dy = targetY - currentPose.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    // If chassis pose changed externally, update target
    if (dist < 0.5 && !isMoving) {
        // Not moving, target reached
    } else if (dist > 0.5) {
        // Still moving towards target
        isMoving = true;
    }
}

void Simulation::printState() {
    printf("[Sim] X:%.1f Y:%.1f H:%.1f | Vel:%.1f in/s | Motors: %.0fC %.0fmA | Battery: %.2fV\n",
           virtualX, virtualY, virtualHeading,
           virtualVelocity,
           mockMotorTemps[0], mockMotorCurrents[0],
           mockBatteryVoltage / 1000.0);
}

void Simulation::drawField() {
    // Draw 144" x 144" field as ASCII grid (scaled down)
    // Field: -72 to +72 inches in both X and Y
    // Grid: 30x30 characters (each cell = ~5")

    printf("\n========== FIELD VISUALIZATION ==========\n");
    printf("  Field: 144\" x 144\" | Robot: (%.1f, %.1f) @ %.0f°\n\n",
           virtualX, virtualY, virtualHeading);

    const int gridSize = 30;
    const float fieldSize = 144.0;
    const float cellSize = fieldSize / gridSize;

    // Calculate robot grid position
    int robotGridX = (int)((virtualX + fieldSize/2) / cellSize);
    int robotGridY = (int)((virtualY + fieldSize/2) / cellSize);

    // Draw grid
    for (int y = gridSize - 1; y >= 0; y--) {
        for (int x = 0; x < gridSize; x++) {
            if (x == robotGridX && y == robotGridY) {
                // Robot position
                printf("R");
            } else if (x == gridSize/2 && y == gridSize/2) {
                // Field center
                printf("+");
            } else if (x == 0 || x == gridSize-1 || y == 0 || y == gridSize-1) {
                // Field boundary
                printf("#");
            } else if (x % 5 == 0 || y % 5 == 0) {
                // Grid lines
                printf(".");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

    printf("=========================================\n\n");
}

// ============================================================================
// MOCK SENSOR DATA GETTERS
// ============================================================================

float Simulation::getMockHeading() {
    return virtualHeading;
}

float Simulation::getMockMotorTemp(int motor_index) {
    if (motor_index < 0 || motor_index >= 6) return 35.0;
    return mockMotorTemps[motor_index];
}

float Simulation::getMockMotorCurrent(int motor_index) {
    if (motor_index < 0 || motor_index >= 6) return 0.0;
    return mockMotorCurrents[motor_index];
}

uint32_t Simulation::getMockBatteryVoltage() {
    return mockBatteryVoltage;
}

void Simulation::validateCalibration() {
    printf("\n========== SIMULATION CALIBRATION TEST ==========\n");
    printf("Real robot data: 48\" in 1.8s @ 60%% speed\n\n");

    // Save current state
    float savedX = virtualX;
    float savedY = virtualY;
    float savedHeading = virtualHeading;

    // Reset to origin
    virtualX = 0;
    virtualY = 0;
    virtualHeading = 0;
    virtualVelocity = 0;
    virtualAngVel = 0;
    chassis->setPose(0, 0, 0);

    // Set target for 48" forward
    targetX = 48;
    targetY = 0;
    targetHeading = 0;
    isMoving = true;

    // Simulate movement
    uint32_t startTime = pros::millis();
    float dt = 0.01;  // 10ms time step

    printf("Simulating 48\" drive...\n");

    while (isMoving) {
        updatePhysics(dt);

        // Check if reached target
        float dx = targetX - virtualX;
        float dy = targetY - virtualY;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < 0.5 && std::abs(virtualVelocity) < 1.0) {
            isMoving = false;
        }

        // Safety timeout
        if (pros::millis() - startTime > 5000) {
            printf("WARNING: Simulation timeout after 5s\n");
            break;
        }

        pros::delay(10);  // Match dt
    }

    uint32_t elapsedMs = pros::millis() - startTime;
    float elapsedSec = elapsedMs / 1000.0;

    printf("\n--- RESULTS ---\n");
    printf("Simulated time: %.2f seconds\n", elapsedSec);
    printf("Real robot time: 1.8 seconds\n");
    printf("Error: %.2f seconds (%.1f%%)\n",
           std::abs(elapsedSec - 1.8),
           std::abs(elapsedSec - 1.8) / 1.8 * 100.0);
    printf("Final position: X:%.1f Y:%.1f (target: 48.0, 0.0)\n",
           virtualX, virtualY);

    if (std::abs(elapsedSec - 1.8) < 0.3) {
        printf("✓ CALIBRATION GOOD (within 0.3s)\n");
    } else {
        printf("✗ CALIBRATION NEEDS TUNING\n");
        printf("  Adjust LINEAR_ACCEL in simulation.hpp\n");
    }

    printf("=================================================\n\n");

    // Restore state
    virtualX = savedX;
    virtualY = savedY;
    virtualHeading = savedHeading;
    virtualVelocity = 0;
    virtualAngVel = 0;
    isMoving = false;
    chassis->setPose(savedX, savedY, savedHeading);
}

} // namespace subsystems
