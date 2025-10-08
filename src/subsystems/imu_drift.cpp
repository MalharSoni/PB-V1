#include "robot/imu_drift.hpp"
#include "pros/rtos.hpp"
#include "pros/misc.h"
#include <cmath>
#include <cstdio>

namespace subsystems {

IMUDrift::IMUDrift(pros::Imu* imu)
    : imu(imu), driftRate(0.0), characterized(false),
      compensationStart(0), baselineHeading(0.0) {}

bool IMUDrift::characterize() {
    printf("[IMU Drift] Starting characterization...\n");
    printf("[IMU Drift] IMPORTANT: Robot must be completely still!\n");

    // Wait for IMU to be ready
    if (imu->is_calibrating()) {
        printf("[IMU Drift] Waiting for IMU calibration...\n");
        while (imu->is_calibrating()) {
            pros::delay(100);
        }
    }

    // Record initial heading
    float initialHeading = imu->get_heading();
    pros::delay(100);

    // Sample heading over 5 seconds (50 samples @ 100ms intervals)
    const int numSamples = 50;
    const int sampleInterval = 100;  // milliseconds
    float headings[numSamples];

    printf("[IMU Drift] Sampling for 5 seconds...\n");
    for (int i = 0; i < numSamples; i++) {
        headings[i] = imu->get_heading();
        pros::delay(sampleInterval);
    }

    // Calculate drift rate using linear regression
    // y = mx + b, where y = heading, x = time, m = drift rate
    float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    for (int i = 0; i < numSamples; i++) {
        float x = i * sampleInterval / 1000.0;  // Convert to seconds
        float y = headings[i];
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }

    // Calculate slope (drift rate)
    float n = numSamples;
    driftRate = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);

    // Check if robot moved during characterization
    float finalHeading = headings[numSamples - 1];
    float totalChange = std::abs(finalHeading - initialHeading);

    // If change is more than 2 degrees, robot probably moved
    if (totalChange > 2.0 && std::abs(driftRate) < 0.01) {
        printf("[IMU Drift] WARNING: Robot may have moved during characterization\n");
        printf("[IMU Drift] Total heading change: %.2f degrees\n", totalChange);
        characterized = false;
        return false;
    }

    characterized = true;

    printf("[IMU Drift] Characterization complete!\n");
    printf("[IMU Drift] Measured drift rate: %.4f deg/sec\n", driftRate);
    printf("[IMU Drift] Total drift over 60s: %.2f degrees\n", driftRate * 60.0);

    return true;
}

void IMUDrift::startCompensation() {
    compensationStart = pros::millis();
    baselineHeading = imu->get_heading();
    printf("[IMU Drift] Compensation started at %.2f degrees\n", baselineHeading);
}

float IMUDrift::getCompensatedHeading() {
    if (!characterized) {
        // No characterization - return raw heading
        return imu->get_heading();
    }

    // Calculate elapsed time
    float elapsedSeconds = (pros::millis() - compensationStart) / 1000.0;

    // Calculate total drift
    float totalDrift = driftRate * elapsedSeconds;

    // Get current heading and apply correction
    float rawHeading = imu->get_heading();
    float compensated = rawHeading - totalDrift;

    return compensated;
}

float IMUDrift::getRawHeading() {
    return imu->get_heading();
}

float IMUDrift::getTotalCorrection() const {
    if (!characterized) return 0.0;

    float elapsedSeconds = (pros::millis() - compensationStart) / 1000.0;
    return driftRate * elapsedSeconds;
}

void IMUDrift::resetCompensation() {
    compensationStart = pros::millis();
    baselineHeading = imu->get_heading();
    printf("[IMU Drift] Compensation reset at %.2f degrees\n", baselineHeading);
}

} // namespace subsystems
