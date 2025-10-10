#pragma once

#include "lemlib/chassis/chassis.hpp"
#include "pros/misc.h"
#include <vector>

namespace tuning {

/**
 * @brief Automatic PID tuner using Ziegler-Nichols method
 *
 * This class automatically finds optimal PID values by:
 * 1. Running test movements with increasing kP
 * 2. Detecting oscillation from position data
 * 3. Calculating Ku (critical gain) and Tu (oscillation period)
 * 4. Computing optimal kP, kI, kD using Ziegler-Nichols formulas
 */
class PIDAutoTuner {
public:
    /**
     * @brief Auto-tune lateral (forward/backward) PID
     *
     * @param chassis LemLib chassis object
     * @param test_distance Distance to travel for each test (inches)
     * @param max_kP Maximum kP value to try
     * @param kP_step Step size for increasing kP
     * @return true if tuning succeeded, false if failed
     */
    static bool tuneLateralPID(lemlib::Chassis* chassis,
                               float test_distance = 48.0,
                               float max_kP = 20.0,
                               float kP_step = 2.0);

    /**
     * @brief Auto-tune angular (turning) PID
     *
     * @param chassis LemLib chassis object
     * @param test_angle Angle to turn for each test (degrees)
     * @param max_kP Maximum kP value to try
     * @param kP_step Step size for increasing kP
     * @return true if tuning succeeded, false if failed
     */
    static bool tuneAngularPID(lemlib::Chassis* chassis,
                              float test_angle = 90.0,
                              float max_kP = 5.0,
                              float kP_step = 0.5);

    /**
     * @brief Get the last tuned PID values
     */
    struct TunedValues {
        float kP;
        float kI;
        float kD;
        float Ku;  // Critical gain
        float Tu;  // Oscillation period
    };

    static TunedValues getLastTunedValues() { return last_tuned; }

private:
    /**
     * @brief Run a single test movement and collect position data
     */
    static std::vector<lemlib::Pose> runTestMovement(lemlib::Chassis* chassis,
                                                      float kP, float kD,
                                                      float distance);

    /**
     * @brief Detect if the robot is oscillating around the target
     * @return true if oscillating, false otherwise
     */
    static bool detectOscillation(const std::vector<lemlib::Pose>& positions,
                                  float target_distance,
                                  float& Tu_out);

    /**
     * @brief Calculate optimal PID values using Ziegler-Nichols method
     */
    static TunedValues calculateZieglerNichols(float Ku, float Tu);

    /**
     * @brief Display tuning progress on brain LCD
     */
    static void displayProgress(int test_num, float current_kP, bool oscillating);

    static TunedValues last_tuned;
};

} // namespace tuning
