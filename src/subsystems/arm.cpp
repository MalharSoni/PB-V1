#include "robot/arm.hpp"
#include "globals.hpp"
#include "robot/intake.hpp"



namespace subsystems {
    Arm::Arm(std::vector<pros::Motor> imotors, int ipiston) : armMotor(imotors), armPist(ipiston, false) {}

    void Arm::goToPickup () {
        if(isArmUp){
            toggleArm();
        }

        armMotor.move_absolute(armState::PICKUP, 127);
        isPickup = true;
        currentArmState = armState::PICKUP;
    }

    void Arm::getPosition() {
        pros::lcd::print(6, "%.2f", leftArmMotor.get_position());

    }

    void Arm::goToIdle () {
        if(isArmUp){
            toggleArm();
        }

        armMotor.move_absolute(armState::IDLE, 127);
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


        armMotor.move_absolute(armState::READY, 127);
        isPickup = false;
        currentArmState = armState::READY;
    }

    void Arm::readyScoreAlliance () {
        if(!isArmUp){
            toggleArm();
        }

        armMotor.move_absolute(armState::ALLIANCE_STAKE_READY, 127);
        isPickup = false;
        currentArmState = armState::ALLIANCE_STAKE_READY;
    }

    void Arm::score () {
        armMotor.move_relative(SCORE_MOVE, 127);
        isPickup = false;
    }

    void Arm::scoreAlliance () {
        if(isArmUp){
            toggleArm();
            isPickup = false;
        }

        armMotor.move_absolute(armState::ALLIANCE_STAKE, 127);
        isPickup = false;
        currentArmState = armState::ALLIANCE_STAKE;
    }

    void Arm::setArmMotorVoltage(int voltage){
        armMotor.move_voltage(voltage);
    }

    void Arm::testControl(pros::controller_digital_e_t upButton, pros::controller_digital_e_t downButton, pros::controller_digital_e_t pistonButton) {
        if(master.get_digital(upButton)) {
            armMotor.move_voltage(-12000);
        } else if(master.get_digital(downButton)) {
            armMotor.move_voltage(12000);
        } else {
            armMotor.move_voltage(0);
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
            armMotor.move_voltage(-12000);
            isMotorMoving = true;
        } else if(master.get_digital(downButton)){
            armMotor.move_voltage(12000);
            isMotorMoving = true;
        } else if(isMotorMoving){
            armMotor.move_voltage(0);
            isMotorMoving = false;
        }
    }
}
