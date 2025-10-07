#pragma once

#include "lib/MotorSubsystem.hpp"
#include "pros/motors.hpp"
#include "pros/misc.hpp"
#include <vector>

namespace subsystems {

// ============================================================================
// PUSH BACK INTAKE - 3-Stage Roller System
// ============================================================================
// This subsystem controls a 3-stage intake system with 8 rollers for
// intaking, storing, and scoring game pieces at different levels.
//
// Hardware:
//   - Stage 1: 11W motor (main intake from floor)
//   - Stage 2: 5.5W motor (transfer/routing)
//   - Stage 3: 5.5W motor (scoring mechanism)
//   - 8 rollers total (driven by chains/gears from these 3 motors)
//
// Modes:
//   1. Intake - Bring balls in from floor
//   2. Score Level 1 - Outtake/score low
//   3. Store - Hold balls internally
//   4. Score Level 2 - Score mid-level
//   5. Score Level 3 - Score high-level
// ============================================================================

class IntakePushback : public lib::MotorSubsystem {
public:
    /**
     * @brief Construct Push Back intake system
     *
     * @param motors Vector of 3 motors: [stage1, stage2, stage3]
     *               - motors[0]: Stage 1 (11W - main intake)
     *               - motors[1]: Stage 2 (5.5W - transfer)
     *               - motors[2]: Stage 3 (5.5W - scoring)
     */
    IntakePushback(std::vector<pros::Motor> motors);

    // ========================================================================
    // INTAKE MODES (5 operational modes)
    // ========================================================================

    /**
     * @brief Mode 1: Intake balls from floor
     *
     * Spins rollers to pull balls into the robot.
     * Stage 1: Full forward
     * Stage 2: Partial forward
     * Stage 3: Off
     */
    void intake();

    /**
     * @brief Mode 2: Outtake / Score Level 1
     *
     * Reverses intake to push balls out or score low.
     * Stage 1: Full reverse
     * Stage 2: Partial reverse
     * Stage 3: Off
     */
    void scoreLevel1();

    /**
     * @brief Mode 3: Store balls internally
     *
     * Holds balls in storage position between intake and scoring.
     * Typically called automatically before scoring.
     *
     * Stage 1: Off
     * Stage 2: Slow hold
     * Stage 3: Off
     */
    void store();

    /**
     * @brief Mode 4: Score Level 2 (mid-level)
     *
     * Routes balls to mid-level scoring position.
     * Stage 1: Off
     * Stage 2: Full forward
     * Stage 3: Partial forward
     */
    void scoreLevel2();

    /**
     * @brief Mode 5: Score Level 3 (high-level)
     *
     * Routes balls to high-level scoring position.
     * Stage 1: Off
     * Stage 2: Full forward
     * Stage 3: Full forward
     */
    void scoreLevel3();

    /**
     * @brief Stop all intake stages
     *
     * Stops all motors immediately.
     */
    void stopAll();

    // ========================================================================
    // DRIVER CONTROL
    // ========================================================================

    /**
     * @brief Driver control interface (4 buttons)
     *
     * @param intakeBtn Button to intake from floor
     * @param scoreL1Btn Button to score level 1 / outtake
     * @param scoreL2Btn Button to score level 2
     * @param scoreL3Btn Button to score level 3
     *
     * Usage in opcontrol():
     *   intake.run(DIGITAL_R1, DIGITAL_R2, DIGITAL_L1, DIGITAL_L2);
     */
    void run(pros::controller_digital_e_t intakeBtn,
             pros::controller_digital_e_t scoreL1Btn,
             pros::controller_digital_e_t scoreL2Btn,
             pros::controller_digital_e_t scoreL3Btn);

private:
    // Individual stage motors (not using MotorGroup since different speeds needed)
    pros::Motor stage1_motor;  // 11W - Main intake
    pros::Motor stage2_motor;  // 5.5W - Transfer/routing
    pros::Motor stage3_motor;  // 5.5W - Scoring mechanism
};

} // namespace subsystems
