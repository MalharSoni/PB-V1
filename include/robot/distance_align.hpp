#pragma once

#include "pros/misc.hpp"
#include "pros/adi.hpp"
#include "pros/distance.hpp"

namespace subsystems {
    class DistanceAlign {
        public:
        /**
         * @brief define the distance sensor localisation
         *
         * @param rightPort port of the right distance sensor
         * @param leftPort port of the left distance sensor
         * @param sensorSeperation distance between right and left distance sensor
         * @param turnCenterDiff distance from the sensors to the turning center (ONLY in y direction and from end of sensor, NOT base)
         */
        DistanceAlign(int rightPort, int leftPort, double sensorSeperation, double turnCenterDiff);
        
        /**
         * @brief calculate the angle the robot is facing given both distance sensors are points at one wall
         *
         * @param wallAngle angle at which the robot would be at if it was facing the wall directly (0, 90, 180, 270)
         */
        void calculateAngleOneWall(double wallAngle);

        /**
         * @brief calculate the x or y depending on what wall you'r facing
         * 
         * @param wallAngle angle signifying what wall you're looking at (0, 90, 180, 270)
         * @param wallDist how far from the origin the wall is
         */
        void calculateDistOneWall(double wallAngle, double wallDist);

        private:
        pros::Distance rightDistance;
        pros::Distance leftDistance;
        double seperation;
        double distToTurnCenter;
    };
}