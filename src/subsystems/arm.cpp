#include "robot/arm.hpp"
#include "globals.hpp"
#include "robot/intake.hpp"



namespace subsystems {
    Arm::Arm(std::vector<pros::Motor> imotors, int ipiston)
        : lib::MotorSubsystem(imotors),  // Pass motors to base class
          armPist(ipiston, false) {}

    void Arm::goToPickup () {
        if(isArmUp){
            toggleArm();
        }

        moveAbsolute(armState::PICKUP, 127);  // Use base class method
        isPickup = true;
        currentArmState = armState::PICKUP;
    }

    void Arm::goToIdle () {
        if(isArmUp){
            toggleArm();
        }

        moveAbsolute(armState::IDLE, 127);  // Use base class method
        isPickup = false;
        currentArmState = armState::IDLE;
    }

    void Arm::toggleArm () {
        isPickup = false;
        isArmUp = !isArmUp;
        armPist.set_value(isArmUp);
        pros::delay(25);
    }

    void Arm::readyScore () {
        if(!isArmUp){
            toggleArm();
        }
        intake.moveRelative(-60, 12000);

        moveAbsolute(armState::READY, 127);  // Use base class method
        isPickup = false;
        currentArmState = armState::READY;
    }

    void Arm::readyScoreAlliance () {
        if(!isArmUp){
            toggleArm();
        }

        moveAbsolute(armState::ALLIANCE_STAKE_READY, 127);  // Use base class method
        isPickup = false;
        currentArmState = armState::ALLIANCE_STAKE_READY;
    }

    void Arm::score () {
        moveRelative(SCORE_MOVE, 127);  // Use base class method
        isPickup = false;
    }

    void Arm::scoreAlliance () {
        if(isArmUp){
            toggleArm();
            isPickup = false;
        }

        moveAbsolute(armState::ALLIANCE_STAKE, 127);  // Use base class method
        isPickup = false;
        currentArmState = armState::ALLIANCE_STAKE;
    }

    void Arm::setArmMotorVoltage(int voltage){
        move(voltage);  // Use base class method
    }

    void Arm::testControl(pros::controller_digital_e_t upButton, pros::controller_digital_e_t downButton, pros::controller_digital_e_t pistonButton) {
        if(master.get_digital(upButton)) {
            move(-12000);  // Use base class method
        } else if(master.get_digital(downButton)) {
            move(12000);  // Use base class method
        } else {
            move(0);  // Use base class method
        }

        if(master.get_digital_new_press(pistonButton)){
            toggleArm();
        }
    }

    void Arm::run (pros::controller_digital_e_t pickupIdleButton, pros::controller_digital_e_t readyScoreButton, pros::controller_digital_e_t scoreButton, pros::controller_digital_e_t upButton, pros::controller_digital_e_t downButton) {
        if(master.get_digital_new_press(pickupIdleButton)) {
            if(currentArmState != armState::PICKUP) {
                goToPickup();
            } else {
                goToIdle();
            }
        }

        if(master.get_digital_new_press(readyScoreButton)) {
            readyScore();
        }

        if(master.get_digital_new_press(scoreButton)){
            score();
        }

        if(master.get_digital(upButton)){
            move(-12000);  // Use base class method
            isMotorMoving = true;
        } else if(master.get_digital(downButton)){
            move(12000);  // Use base class method
            isMotorMoving = true;
        } else if(isMotorMoving){
            move(0);  // Use base class method
            isMotorMoving = false;
        }
    }
}
