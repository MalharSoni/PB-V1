#pragma once

#include "lemlib/chassis/chassis.hpp"
#include <vector>

namespace tuning {

/**
 * @brief Helper class for Ziegler-Nichols PID tuning
 *
 * This runs a single test movement and analyzes the data to detect oscillation.
 * You manually update kP in globals.cpp between tests until oscillation is detected.
 *
 * Workflow:
 * 1. Set kP=2, kD=0 in globals.cpp, build, upload
 * 2. Run autonomous (calls ZNTestHelper::runTest())
 * 3. Check brain LCD - does it say "OSCILLATING"?
 * 4. If no: increase kP by 2, rebuild, repeat
 * 5. If yes: Note the Ku and Tu values, calculate optimal PID
 */
class ZNTestHelper {
public:
    /**
     * @brief Run a test movement and analyze for oscillation
     *
     * Call this from autonomous() function.
     * Results are displayed on brain LCD.
     *
     * @param chassis LemLib chassis
     * @param test_distance Distance to travel (inches)
     * @param current_kP Current kP value being tested (from globals.cpp)
     */
    static void runTest(lemlib::Chassis* chassis,
                       float test_distance = 48.0,
                       float current_kP = 0);

    /**
     * @brief Calculate optimal PID values from Ku and Tu
     *
     * Once oscillation is detected, use this to calculate recommended values.
     *
     * @param Ku Critical gain (kP where oscillation started)
     * @param Tu Oscillation period (in seconds)
     */
    static void calculateRecommendedPID(float Ku, float Tu);

private:
    static bool detectOscillation(const std::vector<float>& errors,
                                  float& Tu_out,
                                  int& num_oscillations);
};

} // namespace tuning
