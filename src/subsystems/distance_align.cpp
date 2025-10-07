#include "robot/distance_align.hpp"
#include "globals.hpp"
#include "math.h"
#include "pros/llemu.hpp"

namespace subsystems {
    DistanceAlign::DistanceAlign(int rightPort, int leftPort, double sensorSeperation, double turnCenterDiff) : rightDistance(rightPort), leftDistance(leftPort) {
        seperation = sensorSeperation;
        distToTurnCenter = turnCenterDiff;
    }

    void DistanceAlign::calculateAngleOneWall(double wallAngle){
        // convert to inches
        double distDiff = ((double)(rightDistance.get()-leftDistance.get()))/25.4;
        pros::lcd::print(5, "dist diff: %Lf", distDiff);
        pros::lcd::print(7, "right: %d, left: %d", rightDistance.get(), leftDistance.get());

        // set angle based on their difference (make sure to convert from radians)
        chassis.setPose(chassis.getPose().x, chassis.getPose().y, wallAngle + (atan(distDiff/seperation)*180/3.14159));
        pros::lcd::print(6, "angle: %Lf", atan(distDiff/seperation));
    }

    void DistanceAlign::calculateDistOneWall(double wallAngle, double wallDist){
        // average distance (center of 2 sensors) plus distance to the turning center (what odom tracks)
        double distTotal = (((double)(rightDistance.get()+leftDistance.get()) / 2.0) / 25.4) + distToTurnCenter;
        double offWall = distTotal*cos((wallAngle-chassis.getPose().theta)*(3.14159/180));
        
        // choose what to change based on wall angle
        if(wallAngle == 180 || wallAngle == 0){ // changing y
            // calculate new y, choosing +/- based on wall angle
            double newY = (wallDist-offWall)*cos(wallAngle*(3.14159/180));

            // set the position accordingly
            chassis.setPose(chassis.getPose().x, newY, chassis.getPose().theta);
        } else if(wallAngle == 90 || wallAngle == 270){ // changing x
            // calculate new x, choosing +/- based on wall angle
            double newX = -(wallDist-offWall)*sin(wallAngle*(3.14159/180));

            // set new position accordingly
            chassis.setPose(newX, chassis.getPose().y, chassis.getPose().theta);
        }
        // if the wall angle was not a valid choice, don't use it
    }
}