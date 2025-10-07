#pragma once

#include "pros/motors.hpp"
#include "pros/misc.hpp"
#include "pros/adi.hpp"

namespace subsystems {
    enum armState {
        PICKUP = 240, // position to get ring from intake
        IDLE = 0, // idle position
        READY = 1140, // position BEFORE scoring wall stakes
        SCORE_MOVE = 100, // RELATIVE movement to score wall stakes
        ALLIANCE_STAKE = 2200, // position AFTER scoring alliance stakes
        ALLIANCE_STAKE_READY = 2200, // position BEFORE scoring alliance stakes //matt score stake position
        MANUAL_CONTROL
    };

    class Arm {
        public:
        bool isPickup = false;
            /**
             * @brief Construct a new Arm object
             *
             * @param imotors vector list of all arm motors
             * @param ipiston arm piston (if non-existent, do it anyways to an empty port)
             */
            Arm(std::vector<pros::Motor> imotors, int ipiston);

            /**
             * @brief go to the pickup position (will toggle arm piston if needed)
             *
             */
            void goToPickup ();

            void getPosition();

            /**
             * @brief go to idle position (will toggle arm piston if needed)
             *
             */
            void goToIdle ();

            /**
             * @brief change state of arm position
             *
             */
            void toggleArm ();

            /**
             * @brief go to position ready to score on wall stakes
             *
             */
            void readyScore ();

            /**
             * @brief go to position ready to score on alliance stakes
             *
             */
            void readyScoreAlliance ();

            /**
             * @brief move amount needed to score on wall stakes
             *
             */
            void score ();

            /**
             * @brief move to position at which alliance stake is scored
             *
             */
            void scoreAlliance ();

            /**
             * @brief Set the voltage of the motors contained in the arm motor group
             *
             * @param voltage voltage being set to
             */
            void setArmMotorVoltage(int voltage);

            /**
             * @brief control setup for simple testing (no macros, all independent)
             *
             * @param upButton button to move arm up
             * @param downButton button to move arm down
             * @param pistonButton button to toggle intake piston
             */
            void testControl(pros::controller_digital_e_t upButton, pros::controller_digital_e_t downButton, pros::controller_digital_e_t pistonButton);

            /**
             * @brief control setup for driver (macros, interdependent)
             *
             * @param pickupIdleButton all buttons subject to change because Jovan is weird
             * @param readyScoreButton
             * @param scoreButton
             * @param upButton
             * @param downButton
             */
            void run(pros::controller_digital_e_t pickupIdleButton, pros::controller_digital_e_t readyScoreButton, pros::controller_digital_e_t scoreButton, pros::controller_digital_e_t upButton, pros::controller_digital_e_t downButton);
        private:
            pros::MotorGroup armMotor;
            pros::ADIDigitalOut armPist;
            armState currentArmState = armState::PICKUP;
            bool isMotorMoving = false;
            bool isArmUp = false;
    };
}
