#include "robot/intake_pushback.hpp"
#include "globals.hpp"

namespace subsystems {

// ============================================================================
// CONSTRUCTOR
// ============================================================================

IntakePushback::IntakePushback(std::vector<pros::Motor> motors)
    : lib::MotorSubsystem(motors),  // Pass all motors to base class
      stage1_motor(motors.at(0)),    // 11W - Main intake
      stage2_motor(motors.at(1)),    // 5.5W - Transfer
      stage3_motor(motors.at(2))     // 5.5W - Scoring
{
    // Set brake modes for precise control
    stage1_motor.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
    stage2_motor.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
    stage3_motor.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
}
// ============================================================================
// INTAKE BUTTONS
// ============================================================================
        //            DIGITAL_R1,   // R1: Intake from floor and store
        //            DIGITAL_R2,   // R2: Score Level 1 / Outtake
        //            DIGITAL_L1,   // L1: Score Level 2
        //            DIGITAL_L2);  // L2: Score Level 3
// ============================================================================
// MODE 1: INTAKE
// ============================================================================

void IntakePushback::intake() {
    // Pull balls in from floor
    // NOTE: Tune these speeds during testing!
    stage1_motor.move_voltage(12000);   // Full forward (main intake)
    stage2_motor.move_voltage(12000);    // Half forward (transfer)
    stage3_motor.move_voltage(0);       // Off
}

// ============================================================================
// MODE 2: SCORE LEVEL 1 / OUTTAKE
// ============================================================================

void IntakePushback::scoreLevel1() {
    // Reverse to outtake or score low
    // NOTE: Tune these speeds during testing!
    stage1_motor.move_voltage(-12000);  // Full reverse (outtake)
    stage2_motor.move_voltage(-12000);   // Half reverse (assist)
    stage3_motor.move_voltage(0);       // Off
}

// ============================================================================
// MODE 3: STORE
// ============================================================================

void IntakePushback::store() {
    // Hold balls internally between intake and scoring
    // NOTE: Tune these speeds during testing!
    stage1_motor.move_voltage(12000);       // Off
    stage2_motor.move_voltage(-6000);    // Slow hold (prevent dropping)
    stage3_motor.move_voltage(0);       // Off
}

// ============================================================================
// MODE 4: SCORE LEVEL 2
// ============================================================================

void IntakePushback::scoreLevel2() {
    // Route balls to mid-level scoring
    // NOTE: Tune these speeds during testing!
    stage1_motor.move_voltage(0);       // Off
    stage2_motor.move_voltage(12000);   // Full forward (routing)
    stage3_motor.move_voltage(12000);    // 3/4 forward (scoring)
}

// ============================================================================
// MODE 5: SCORE LEVEL 3
// ============================================================================

void IntakePushback::scoreLevel3() {
    // Route balls to high-level scoring
    // NOTE: Tune these speeds during testing!
    stage1_motor.move_voltage(0);       // Off
    stage2_motor.move_voltage(12000);   // Full forward (routing)
    stage3_motor.move_voltage(-12000);   // Full forward (high scoring)
}

// ============================================================================
// STOP ALL
// ============================================================================

void IntakePushback::stopAll() {
    // Stop all stages immediately
    stage1_motor.move_voltage(0);
    stage2_motor.move_voltage(0);
    stage3_motor.move_voltage(0);
}

// ============================================================================
// DRIVER CONTROL
// ============================================================================

void IntakePushback::run(pros::controller_digital_e_t intakeBtn,
                         pros::controller_digital_e_t scoreL1Btn,
                         pros::controller_digital_e_t scoreL2Btn,
                         pros::controller_digital_e_t scoreL3Btn) {
    // 4-button control: priority order
    if (master.get_digital(intakeBtn)) {
        store();           // Intake from floor
    }
    else if (master.get_digital(scoreL1Btn)) {
        scoreLevel1();      // Outtake / score low
    }
    else if (master.get_digital(scoreL2Btn)) {
        scoreLevel2();      // Score mid
    }
    else if (master.get_digital(scoreL3Btn)) {
        scoreLevel3();      // Score high
    }
    else {
        stopAll();          // No button pressed: stop
    }
}

} // namespace subsystems
