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

    /**
     * @brief Simple drive test - Verify tracking wheels work
     *
     * Drives robot 24" forward to verify Y-axis tracking.
     * Check brain screen to ensure Y increases and X stays near 0.
     */
    void odomDriveTest();

    /**
     * @brief ISOLATED lateral PID test - Forward movement ONLY
     *
     * Tests lateral PID tuning with NO turns:
     * - Drive 24" forward at different speeds
     * - Logs: time, X, Y, target, error every 50ms
     * - Use to tune: kP, kI, kD, exit conditions
     */
    void lateralPIDTest();

    /**
     * @brief ISOLATED angular PID test - Turning ONLY
     *
     * Tests angular PID tuning with NO driving:
     * - Turn to: 90°, 180°, 270°, 0° (stay in place)
     * - Logs: time, heading, target, error every 50ms
     * - Use to tune: kP, kI, kD, exit conditions
     */
    void angularPIDTest();

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
