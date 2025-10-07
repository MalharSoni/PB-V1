#include "robot/movement.hpp"
#include "globals.hpp"
#include "pros/motors.hpp"
#include "pros/misc.hpp"

namespace subsystems {
    Movement::Movement(lemlib::Chassis* chassis)
        : chassis(chassis) {    }


    void Movement::arcade_drive(double leftY, double rightX) {
        chassis->arcade(leftY, rightX);
    }

}
