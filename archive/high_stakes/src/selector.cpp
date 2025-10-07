#include "robot/selector.hpp"
#include "robot/auton.hpp"

    namespace subsystems
{

    Selector::Selector(Intake * intake, Auton * auton)
        : intake(intake), auton(auton), current_page(AUTON_PAGE),
        selected_auton(AUTON_ROUTINE::NONE), selected_color(DONUT_COLOR::NONE) {}

    void Selector::init()
    {
        pros::lcd::initialize();
        draw_auton_selector();
    }

    void Selector::update()
    {

        if (current_page == AUTON_PAGE)
        {
            handle_auton_selection();
        }
        else if (current_page == COLOR_PAGE)
        {
            handle_color_selection();
        }
        else if (current_page == CONFIRMATION_PAGE)
        {
            handle_confirmation_page();
        }
    }

    int page = 1;
    void Selector::draw_auton_selector()
    {

        if (page == 1)
        {
            pros::lcd::clear();
            pros::lcd::set_text(0, "Auton Selection");
            pros::lcd::set_text(1, "Red Rush");
            pros::lcd::set_text(2, "Red High Score");
            pros::lcd::set_text(3, "[empty]");
            pros::lcd::set_text(4, "[empty]");
            pros::lcd::set_text(7, "Next page >>");
            pros::lcd::set_text(8, "Confirm");
        }
        else if (page == 2)
        {
            pros::lcd::clear();
            pros::lcd::set_text(0, "Auton Selection");
            pros::lcd::set_text(1, "Blue Rush");
            pros::lcd::set_text(2, "Blue High Score");
            pros::lcd::set_text(3, "Skills");
            pros::lcd::set_text(7, "Previous page <<");
            pros::lcd::set_text(8, "Confirm");
        }
    }
    int selectedNumber = 1;
    void Selector::handle_auton_selection()
    {

        if (pros::lcd::read_buttons() & LCD_BTN_LEFT)
        {
            if (selectedNumber != 1)
            {
                if (selectedNumber == 5)
                    page = 1;

                selectedNumber--;
            }
        }
        else if (pros::lcd::read_buttons() & LCD_BTN_RIGHT)
        {
            if (selectedNumber != 7)
            {
                if (selectedNumber == 4)
                    page = 2;
                selectedNumber++;
            }
        }
        draw_auton_selector();
        switch (selectedNumber)
        {
        case 1:
            selected_auton = AUTON_ROUTINE::RED_RUSH;
            pros::lcd::set_text(6, "Selected: Red Rush");
            break;
        case 2:
            selected_auton = AUTON_ROUTINE::RED_HIGH_SCORE;
            pros::lcd::set_text(6, "Selected: Red High Scoring");
            break;
        case 3:
            selected_auton = AUTON_ROUTINE::TEST;
            pros::lcd::set_text(6, "Selected: [empty]");
            break;
        case 4:
            selected_auton = AUTON_ROUTINE::TEST;
            pros::lcd::set_text(6, "Selected: [empty]");
            break;
        case 5:
            selected_auton = AUTON_ROUTINE::BLUE_RUSH;
            pros::lcd::set_text(6, "Selected: Blue Rush");
            break;
        case 6:
            selected_auton = AUTON_ROUTINE::BLUE_HIGH_SCORE;
            pros::lcd::set_text(6, "Selected: Blue High Scoring");
            break;
        case 7:
            selected_auton = AUTON_ROUTINE::SKILLS;
            pros::lcd::set_text(6, "Selected: Skills");
            break;
        }
        // else if (pros::lcd::read_buttons() & (LCD_BTN_LEFT | LCD_BTN_RIGHT)) {
        //     selected_auton = AUTON_ROUTINE::BLUE_RIGHT;
        //     pros::lcd::set_text(5, "Selected: Blue Right");
        // }

        if (pros::lcd::read_buttons() & LCD_BTN_CENTER &&
            selected_auton != AUTON_ROUTINE::NONE)
        {
            auton->set_selected_auton(selected_auton);
            current_page = COLOR_PAGE;
            draw_color_selector();
        }
    }
    int color_selection = 1;
    void Selector::draw_color_selector()
    {
        pros::lcd::clear();
        pros::lcd::set_text(0, "Color Selection");
        pros::lcd::set_text(1, "Red");
        pros::lcd::set_text(2, "Blue");
        pros::lcd::set_text(4, "Confirm");
    }

    void Selector::handle_color_selection()
    {
        if (pros::lcd::read_buttons() & LCD_BTN_LEFT)
        {
            selected_color = DONUT_COLOR::RED;
            color_selection = 1;
            pros::lcd::set_text(3, "Selected: RED");
            pros::delay(200);
        }
        else if (pros::lcd::read_buttons() & LCD_BTN_RIGHT)
        {
            selected_color = DONUT_COLOR::BLUE;
            color_selection = 2;
            pros::lcd::set_text(3, "Selected: BLUE");
            pros::delay(200);
        }

        if (pros::lcd::read_buttons() & LCD_BTN_CENTER &&
            selected_color != DONUT_COLOR::NONE)
        {
            intake->set_target_color(selected_color);
            pros::lcd::set_text(5, "Selection Confirmed!");
            // current_page = AUTON_PAGE;
            current_page = CONFIRMATION_PAGE;
            draw_confirmation_page();
            // draw_auton_selector();
        }
    }

    void Selector::draw_confirmation_page()
    {

        pros::lcd::clear();
        switch (selectedNumber)
        {
        case 1:
            pros::lcd::set_text(0, "Auto: Red Rush");
            break;
        case 2:
            pros::lcd::set_text(0, "Auto: Red High Scoring");
            break;
        case 3:
            pros::lcd::set_text(0, "Auto: [empty]");
            break;
        case 4:
            pros::lcd::set_text(0, "Auto: [empty]");
            break;
        case 5:
            pros::lcd::set_text(0, "Auto: Blue Rush");
            break;
        case 6:
            pros::lcd::set_text(0, "Auto: Blue High Scoring");
            break;
        case 7:
            pros::lcd::set_text(0, "Auto: Skills");
            break;
        }

        if (color_selection == 1)
        {
            pros::lcd::set_text(1, "Your Color Selection: RED");
        }
        else
        {
            pros::lcd::set_text(1, "Your Color Selection: BLUE");
        }

        pros::lcd::set_text(3, "Press right button to modify.");
    }

    void Selector::handle_confirmation_page()
    {
        if (pros::lcd::read_buttons() & LCD_BTN_RIGHT)
        {
            current_page = AUTON_PAGE;
            draw_auton_selector();
        }
    }

} // namespace subsystems
