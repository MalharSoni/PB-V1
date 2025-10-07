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
