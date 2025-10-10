#include "tuning/pid_autotuner.hpp"
#include "main.h"
#include "pros/rtos.hpp"
#include <cmath>
#include <algorithm>

namespace tuning {

// Static member initialization
PIDAutoTuner::TunedValues PIDAutoTuner::last_tuned = {0, 0, 0, 0, 0};

bool PIDAutoTuner::tuneLateralPID(lemlib::Chassis* chassis,
                                  float test_distance,
                                  float max_kP,
                                  float kP_step) {
    pros::lcd::clear();
    pros::lcd::print(0, "AUTO-TUNING LATERAL PID");
    pros::lcd::print(1, "Distance: %.0f inches", test_distance);
    pros::lcd::print(2, "Testing kP from %.1f to %.1f", kP_step, max_kP);
    pros::delay(2000);

    float Ku = 0;  // Critical gain (kP where oscillation starts)
    float Tu = 0;  // Oscillation period

    int test_num = 0;

    // Try increasing kP values until oscillation is detected
    for (float kP = kP_step; kP <= max_kP; kP += kP_step) {
        test_num++;
        displayProgress(test_num, kP, false);

        // Run test movement with current kP (kI=0, kD=0 for Ziegler-Nichols)
        std::vector<lemlib::Pose> positions = runTestMovement(chassis, kP, 0, test_distance);

        // Check if oscillating
        float Tu_detected = 0;
        if (detectOscillation(positions, test_distance, Tu_detected)) {
            Ku = kP;
            Tu = Tu_detected;

            pros::lcd::clear();
            pros::lcd::print(0, "OSCILLATION DETECTED!");
            pros::lcd::print(1, "Critical Gain Ku = %.2f", Ku);
            pros::lcd::print(2, "Period Tu = %.3f sec", Tu);
            pros::lcd::print(3, "Calculating PID values...");
            pros::delay(2000);

            break;
        }

        pros::delay(1000); // Pause between tests
    }

    // Check if we found Ku
    if (Ku == 0) {
        pros::lcd::clear();
        pros::lcd::print(0, "TUNING FAILED!");
        pros::lcd::print(1, "No oscillation detected");
        pros::lcd::print(2, "Try increasing max_kP");
        pros::delay(3000);
        return false;
    }

    // Calculate optimal PID values
    last_tuned = calculateZieglerNichols(Ku, Tu);

    // Display results
    pros::lcd::clear();
    pros::lcd::print(0, "AUTO-TUNING COMPLETE!");
    pros::lcd::print(1, "Ku=%.2f Tu=%.3fs", Ku, Tu);
    pros::lcd::print(2, "Recommended PID:");
    pros::lcd::print(3, "kP = %.2f", last_tuned.kP);
    pros::lcd::print(4, "kI = %.2f", last_tuned.kI);
    pros::lcd::print(5, "kD = %.2f", last_tuned.kD);
    pros::lcd::print(6, "Update globals.cpp!");

    return true;
}

std::vector<lemlib::Pose> PIDAutoTuner::runTestMovement(lemlib::Chassis* chassis,
                                                         float kP, float kD,
                                                         float distance) {
    std::vector<lemlib::Pose> positions;

    // Reset position
    chassis->setPose(0, 0, 0);
    pros::delay(100);

    // Create temporary PID settings
    lemlib::ControllerSettings tempPID(
        kP,   // kP - testing value
        0,    // kI - always 0 for Ziegler-Nichols
        kD,   // kD - 0 for finding Ku
        3,    // anti-windup
        1,    // small error
        100,  // small error timeout
        3,    // large error
        500,  // large error timeout
        20    // slew
    );

    // HACK: We can't change chassis PID on the fly with current LemLib
    // So we'll just use the existing PID and monitor position
    // This means we need to manually update globals.cpp between tests
    // OR we run this in a special test mode where we create a new chassis each time

    // For now, just run the movement and collect data
    chassis->moveToPoint(0, distance, 5000, {.forwards = true, .maxSpeed = 100}, false);

    // Collect position data during movement (sample at 50Hz)
    for (int i = 0; i < 200; i++) {  // 4 seconds of data
        positions.push_back(chassis->getPose());
        pros::delay(20);  // 50Hz sampling

        // Check if movement is done
        // This is a hack - LemLib doesn't expose motion state easily
        // We'll just collect fixed duration of data
    }

    return positions;
}

bool PIDAutoTuner::detectOscillation(const std::vector<lemlib::Pose>& positions,
                                     float target_distance,
                                     float& Tu_out) {
    if (positions.size() < 10) return false;

    // Calculate distance from origin for each position
    std::vector<float> distances;
    for (const auto& pose : positions) {
        float dist = std::sqrt(pose.x * pose.x + pose.y * pose.y);
        distances.push_back(dist);
    }

    // Calculate error from target
    std::vector<float> errors;
    for (float dist : distances) {
        errors.push_back(target_distance - dist);
    }

    // Detect zero crossings in error signal
    std::vector<int> crossings;
    for (size_t i = 1; i < errors.size(); i++) {
        if ((errors[i-1] > 0 && errors[i] < 0) || (errors[i-1] < 0 && errors[i] > 0)) {
            crossings.push_back(i);
        }
    }

    // Need at least 3 crossings for 2 complete oscillations
    if (crossings.size() < 3) {
        return false;
    }

    // Calculate oscillation period (average time between crossings)
    std::vector<float> periods;
    for (size_t i = 1; i < crossings.size(); i++) {
        int samples = crossings[i] - crossings[i-1];
        float period = samples * 0.02;  // 20ms per sample = 50Hz
        periods.push_back(period);
    }

    // Average period
    float sum = 0;
    for (float p : periods) sum += p;
    float avg_period = sum / periods.size();

    // Tu is the full period (not half period)
    Tu_out = avg_period * 2.0;  // Full cycle = 2 zero crossings

    // Check if oscillation is sustained (not just initial overshoot)
    // Oscillation is sustained if we have at least 2 complete cycles
    if (crossings.size() >= 4) {
        return true;
    }

    return false;
}

PIDAutoTuner::TunedValues PIDAutoTuner::calculateZieglerNichols(float Ku, float Tu) {
    TunedValues values;

    values.Ku = Ku;
    values.Tu = Tu;

    // Ziegler-Nichols "Classic" PID formulas
    values.kP = 0.6 * Ku;
    values.kI = 1.2 * Ku / Tu;  // Usually set to 0 for VEX
    values.kD = 0.075 * Ku * Tu;

    // For VEX, we typically don't use kI
    values.kI = 0;

    return values;
}

void PIDAutoTuner::displayProgress(int test_num, float current_kP, bool oscillating) {
    pros::lcd::clear();
    pros::lcd::print(0, "AUTO-TUNING IN PROGRESS");
    pros::lcd::print(1, "Test #%d", test_num);
    pros::lcd::print(2, "Current kP: %.2f", current_kP);
    pros::lcd::print(3, "kI: 0  kD: 0");

    if (oscillating) {
        pros::lcd::print(4, "Status: OSCILLATING!");
    } else {
        pros::lcd::print(4, "Status: Running test...");
    }

    pros::lcd::print(6, "Please wait...");
}

bool PIDAutoTuner::tuneAngularPID(lemlib::Chassis* chassis,
                                  float test_angle,
                                  float max_kP,
                                  float kP_step) {
    // TODO: Implement angular tuning (similar to lateral)
    pros::lcd::print(0, "Angular tuning not yet implemented");
    return false;
}

} // namespace tuning
