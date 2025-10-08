#pragma once
#include "lemlib/api.hpp"

namespace subsystems {

enum class AUTON_ROUTINE { NONE, RED_RUSH, RED_HIGH_SCORE, BLUE_RUSH, BLUE_HIGH_SCORE, SKILLS, TEST};


class Auton {
public:
    Auton(lemlib::Chassis* ichassis);
    AUTON_ROUTINE get_selected_auton();
    void run_auton(AUTON_ROUTINE routine);
    void set_selected_auton(AUTON_ROUTINE routine);
    void go_forward(float distance, float time, float spd);
    void go_backward(float distance, float time);

    // ========================================================================
    // PUSH BACK AUTONOMOUS ROUTINES
    // ========================================================================

    /**
     * @brief Simple Push Back autonomous - Intake and score
     *
     * Routine:
     * 1. Start intake
     * 2. Move to collect balls
     * 3. Move to scoring position
     * 4. Score balls
     */
    void pushBackSimple();

    // ========================================================================
    // AUTONOMOUS TEMPLATES (Student Examples)
    // ========================================================================

    /**
     * @brief Template 1: Simple 2-Ball Autonomous (Beginner)
     *
     * Goal: Score 2 balls with high reliability
     * Strategy: Minimal movements, guaranteed points
     * Time: ~8 seconds
     * Complexity: ⭐ (Beginner)
     *
     * Students: Copy this as starting point for simple autonomous
     */
    void template_simple_2ball();

    /**
     * @brief Template 2: 4-Ball with Checkpoints (Intermediate)
     *
     * Goal: Score 4 balls with odometry checkpoints
     * Strategy: Use wall alignment after each scoring cycle
     * Time: ~12 seconds
     * Complexity: ⭐⭐⭐ (Intermediate)
     *
     * Students: Shows wall alignment and error recovery
     */
    void template_4ball_checkpoints();

    /**
     * @brief Template 3: 6-Ball Optimized (Advanced)
     *
     * Goal: Score 6 balls with motion chaining
     * Strategy: Seamless movements, parallel subsystem actions
     * Time: ~14 seconds
     * Complexity: ⭐⭐⭐⭐⭐ (Advanced)
     *
     * Students: Shows all advanced techniques
     */
    void template_6ball_optimized();

    /**
     * @brief Template 4: Skills Run (60 seconds)
     *
     * Goal: Maximum score in 60 seconds
     * Strategy: Loop scoring with periodic resets
     * Time: 60 seconds
     * Complexity: ⭐⭐⭐⭐ (Advanced)
     *
     * Students: Endurance autonomous with error handling
     */
    void template_skills();

    /**
     * @brief Simple drive test - Verify tracking wheels work
     *
     * Drives robot 24" forward to verify Y-axis tracking.
     * Check brain screen to ensure Y increases and X stays near 0.
     */
    void odomDriveTest();

    /**
     * @brief Odometry tuning test - Drive in a 24" x 24" square
     *
     * Tests odometry accuracy by driving in a square pattern.
     * Robot should return to starting position with minimal drift.
     * Check brain screen for final position error after test completes.
     */
    void odomSquareTest();

    /**
     * @brief Motor diagnostics test - Check for motor imbalance
     *
     * Drives 48" forward while displaying individual motor metrics:
     * - Velocities (RPM) for each motor
     * - Current draw (mA) for each motor
     * - Temperatures before and after drive
     * - Motor efficiency ratings
     * - Real-time imbalance calculation (L-R)
     *
     * Use this to identify:
     * - Weak/damaged motors (lower velocity, higher temp)
     * - Port configuration errors (motors not responding)
     * - Systematic imbalance causing drift
     */
    void motorDiagnostics();

    // ========================================================================
    // OLD HIGH STAKES AUTONOMOUS ROUTINES (Archived)
    // ========================================================================

    /**
     * @brief use with brandon to place in corner and sit
     *
     */
    void elimSAFERED();
    void elimSAFEBLUE();

    /**
     * @brief solo win point
     *
     */
    void swpRED();
    void swpBLUE();

    /**
     * @brief ring rush using doinker and intake
     *
     */
    void ringRushRed();
    void ringRushBlue();

    /**
     * @brief mogo rush
     *
     */
    void mogoRushRed();
    void mogoRushBlue();

    /**
     * @brief safe mogo side using second mogo and end under ladder
     *
     */
    void safeMogoRed();
    void safeMogoBlue();

    /**
     * @brief prevent other alliance awp
     *
     */
    void interferenceRed();
    void interferneceBlue();

    /**
     * @brief skills route get 65~
     *
     */
    void skills();


private:
    lemlib::Chassis* chassis;
    AUTON_ROUTINE selected_auton;
};

} // namespace subsystems
