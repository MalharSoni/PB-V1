#pragma once

#include "lib/MotorSubsystem.hpp"
#include "pros/motors.hpp"
#include "pros/misc.hpp"
#include "pros/adi.hpp"

namespace subsystems {
    // ============================================================================
    // GAME-SPECIFIC: HIGH STAKES ARM POSITIONS
    // ============================================================================
    enum armState {
        PICKUP = 240,               // Position to get ring from intake
        IDLE = 0,                   // Idle position
        READY = 1140,               // Position BEFORE scoring wall stakes
        SCORE_MOVE = 100,           // RELATIVE movement to score wall stakes
        ALLIANCE_STAKE = 2200,      // Position AFTER scoring alliance stakes
        ALLIANCE_STAKE_READY = 2200, // Position BEFORE scoring alliance stakes
        MANUAL_CONTROL
    };

    class Arm : public lib::MotorSubsystem {
        public:
        bool isPickup = false;

            /**
             * @brief Construct a new Arm object
             *
             * @param imotors vector list of all arm motors
             * @param ipiston arm piston (if non-existent, use an empty port)
             */
            Arm(std::vector<pros::Motor> imotors, int ipiston);

            // ====================================================================
            // MOTOR CONTROL (uses inherited MotorSubsystem methods)
            // ====================================================================

            // Inherited from MotorSubsystem:
            //   - moveAbsolute(float position, float speed)
            //   - moveRelative(float delta, float speed)
            //   - move(int voltage)
            //   - stop()
            //   - getPosition()
            //   - getVelocity()

            /**
             * @brief Set the voltage of the arm motors (wrapper for move())
             *
             * @param voltage voltage being set to
             */
            void setArmMotorVoltage(int voltage);

            // ====================================================================
            // GAME-SPECIFIC: HIGH STAKES ARM CONTROL
            // ====================================================================

            /**
             * @brief Go to the pickup position (will toggle arm piston if needed)
             */
            void goToPickup();

            /**
             * @brief Go to idle position (will toggle arm piston if needed)
             */
            void goToIdle();

            /**
             * @brief Change state of arm piston
             */
            void toggleArm();

            /**
             * @brief Go to position ready to score on wall stakes
             */
            void readyScore();

            /**
             * @brief Go to position ready to score on alliance stakes
             */
            void readyScoreAlliance();

            /**
             * @brief Move amount needed to score on wall stakes
             */
            void score();

            /**
             * @brief Move to position at which alliance stake is scored
             */
            void scoreAlliance();

            /**
             * @brief Control setup for simple testing (no macros, all independent)
             *
             * @param upButton button to move arm up
             * @param downButton button to move arm down
             * @param pistonButton button to toggle arm piston
             */
            void testControl(pros::controller_digital_e_t upButton, pros::controller_digital_e_t downButton, pros::controller_digital_e_t pistonButton);

            /**
             * @brief Control setup for driver (macros, interdependent)
             *
             * @param pickupIdleButton Toggle between pickup and idle
             * @param readyScoreButton Move to ready score position
             * @param scoreButton Execute score movement
             * @param upButton Manual arm up
             * @param downButton Manual arm down
             */
            void run(pros::controller_digital_e_t pickupIdleButton, pros::controller_digital_e_t readyScoreButton, pros::controller_digital_e_t scoreButton, pros::controller_digital_e_t upButton, pros::controller_digital_e_t downButton);

        private:
            // Game-specific hardware (High Stakes)
            pros::ADIDigitalOut armPist;

            // State tracking
            armState currentArmState = armState::PICKUP;
            bool isMotorMoving = false;
            bool isArmUp = false;
    };
}
