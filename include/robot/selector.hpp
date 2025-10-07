#pragma once
#ifndef SELECTOR_H
#define SELECTOR_H

#include "pros/llemu.hpp"
#include "robot/intake.hpp"
#include "robot/auton.hpp"

namespace subsystems {

class Selector {
public:
    Selector(Intake* intake, Auton* auton);
    void init();
    void update();

private:
    static constexpr int AUTON_PAGE = 0;
    static constexpr int COLOR_PAGE = 1;
    static constexpr int CONFIRMATION_PAGE = 2;
    Intake* intake;
    Auton* auton;
    int current_page;
    AUTON_ROUTINE selected_auton;
    DONUT_COLOR selected_color;
    void draw_auton_selector();
    void draw_confirmation_page();
    void handle_auton_selection();
    void draw_color_selector();
    void handle_color_selection();
    void handle_confirmation_page();
};

}

#endif // SELECTOR_H
