/**
 * @file brain_ui.cpp
 * @brief LVGL-based brain screen UI implementation for Team 839Y
 */

#include "robot/brain_ui.hpp"
#include "pros/apix.h"
#include "main.h"
#include "globals.hpp"

// Include logo image (C array generated from LVGL converter)
#include "logo.h"

namespace subsystems {

// ============================================================================
// LVGL TASK HANDLER
// ============================================================================

/**
 * @brief Background task to handle LVGL updates
 * Required for LVGL to process events and render UI
 * Runs every 10ms in separate PROS task
 */
void lvgl_task_handler(void* param) {
    while (true) {
        lv_task_handler();
        pros::delay(10);  // Run every 10ms
    }
}

// ============================================================================
// STATIC MEMBERS INITIALIZATION
// ============================================================================

BrainUI* BrainUI::instance = nullptr;

// Define styles (must be static to prevent deallocation)
lv_style_t BrainUI::style_screen;
lv_style_t BrainUI::style_btn_rel;
lv_style_t BrainUI::style_btn_pr;
lv_style_t BrainUI::style_btn_selected;
lv_style_t BrainUI::style_title;
lv_style_t BrainUI::style_text;
lv_style_t BrainUI::style_confirm_btn;

// ============================================================================
// CONSTRUCTOR
// ============================================================================

BrainUI::BrainUI(Auton* auton)
    : auton(auton),
      current_screen(UIScreen::SPLASH),
      selected_auton(AUTON_ROUTINE::NONE),
      auton_confirmed(false),
      splash_screen(nullptr),
      splash_logo(nullptr),
      splash_status_label(nullptr),
      auton_screen(nullptr),
      auton_logo(nullptr),
      auton_title_label(nullptr),
      auton_btn_red_rush(nullptr),
      auton_btn_blue_rush(nullptr),
      auton_btn_red_high(nullptr),
      auton_btn_blue_high(nullptr),
      auton_btn_skills(nullptr),
      auton_btn_test(nullptr),
      auton_confirm_btn(nullptr),
      auton_selection_label(nullptr),
      op_screen(nullptr),
      op_logo(nullptr),
      op_position_label(nullptr),
      op_battery_label(nullptr),
      op_temp_label(nullptr),
      op_status_label(nullptr),
      confirm_screen(nullptr),
      confirm_label(nullptr),
      confirm_back_btn(nullptr)
{
    instance = this;
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

void BrainUI::init() {
    // Start LVGL task handler in background
    pros::Task lvgl_task(lvgl_task_handler, nullptr, "LVGL Task");

    // Small delay to let LVGL task start
    pros::delay(20);

    // Initialize styles
    initStyles();

    // Create all screens (but don't show them yet)
    createSplashScreen();
    createAutonScreen();
    createOperationScreen();
    createConfirmationScreen();

    // Hide all screens initially
    lv_obj_set_hidden(splash_screen, true);
    lv_obj_set_hidden(auton_screen, true);
    lv_obj_set_hidden(op_screen, true);
    lv_obj_set_hidden(confirm_screen, true);
}

void BrainUI::showSplash() {
    // Hide all other screens
    lv_obj_set_hidden(auton_screen, true);
    lv_obj_set_hidden(op_screen, true);
    lv_obj_set_hidden(confirm_screen, true);

    // Show splash screen
    lv_obj_set_hidden(splash_screen, false);
    lv_scr_load(splash_screen);

    current_screen = UIScreen::SPLASH;
}

void BrainUI::showAutonSelector() {
    // Hide all other screens
    lv_obj_set_hidden(splash_screen, true);
    lv_obj_set_hidden(op_screen, true);
    lv_obj_set_hidden(confirm_screen, true);

    // Show autonomous selector screen
    lv_obj_set_hidden(auton_screen, false);
    lv_scr_load(auton_screen);

    current_screen = UIScreen::AUTON_SELECTOR;
}

void BrainUI::showOperationScreen() {
    // Hide all other screens
    lv_obj_set_hidden(splash_screen, true);
    lv_obj_set_hidden(auton_screen, true);
    lv_obj_set_hidden(confirm_screen, true);

    // Show operation screen
    lv_obj_set_hidden(op_screen, false);
    lv_scr_load(op_screen);

    current_screen = UIScreen::OPERATION;
}

void BrainUI::updateTelemetry() {
    if (current_screen != UIScreen::OPERATION) return;

    // Update position
    lemlib::Pose pose = chassis.getPose();
    char pos_text[100];
    sprintf(pos_text, "Position: X:%.1f Y:%.1f Theta:%.1f",
            pose.x, pose.y, pose.theta);
    lv_label_set_text(op_position_label, pos_text);

    // Update battery
    double battery = pros::battery::get_capacity();
    char bat_text[50];
    sprintf(bat_text, "Battery: %.0f%%", battery);
    lv_label_set_text(op_battery_label, bat_text);

    // Update temperatures (example: average of left drivetrain motors)
    std::vector<double> temps = leftMotors.get_temperatures();
    double avg_temp = 0;
    if (!temps.empty()) {
        for (double t : temps) avg_temp += t;
        avg_temp /= temps.size();
    }
    char temp_text[50];
    sprintf(temp_text, "Motor Temp: %.1fC", avg_temp);
    lv_label_set_text(op_temp_label, temp_text);

    // Status indicator
    lv_label_set_text(op_status_label, "Status: OK");
}

// ============================================================================
// PRIVATE METHODS - SCREEN CREATION
// ============================================================================

void BrainUI::createSplashScreen() {
    // Create screen
    splash_screen = lv_obj_create(NULL, NULL);
    lv_obj_set_style(splash_screen, &style_screen);

    // Team logo - CTRC 839Y (100x100 pixel image)
    splash_logo = lv_img_create(splash_screen, NULL);
    lv_img_set_src(splash_logo, &logo_ctrc_100x100);
    lv_obj_align(splash_logo, NULL, LV_ALIGN_CENTER, 0, -30);

    // Status label
    splash_status_label = lv_label_create(splash_screen, NULL);
    lv_obj_set_style(splash_status_label, &style_text);
    lv_label_set_text(splash_status_label, "Initializing...");
    lv_obj_align(splash_status_label, NULL, LV_ALIGN_CENTER, 0, 80);

    // Team number label
    lv_obj_t* team_label = lv_label_create(splash_screen, NULL);
    lv_obj_set_style(team_label, &style_title);
    lv_label_set_text(team_label, "Team 839Y");
    lv_obj_align(team_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
}

void BrainUI::createAutonScreen() {
    // Create screen
    auton_screen = lv_obj_create(NULL, NULL);
    lv_obj_set_style(auton_screen, &style_screen);

    // Title
    auton_title_label = lv_label_create(auton_screen, NULL);
    lv_obj_set_style(auton_title_label, &style_title);
    lv_label_set_text(auton_title_label, "Select Autonomous");
    lv_obj_align(auton_title_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    // Create 6 buttons in 2 rows of 3
    int btn_width = 145;
    int btn_height = 60;
    int start_y = 50;
    int row_spacing = 70;
    int col_spacing = 155;

    // Row 1
    auton_btn_red_rush = createButton(auton_screen, 10, start_y,
                                      btn_width, btn_height, "Red Rush");
    auton_btn_blue_rush = createButton(auton_screen, 10 + col_spacing, start_y,
                                       btn_width, btn_height, "Blue Rush");
    auton_btn_skills = createButton(auton_screen, 10 + 2*col_spacing, start_y,
                                    btn_width, btn_height, "Skills");

    // Row 2
    auton_btn_red_high = createButton(auton_screen, 10, start_y + row_spacing,
                                      btn_width, btn_height, "Red High");
    auton_btn_blue_high = createButton(auton_screen, 10 + col_spacing, start_y + row_spacing,
                                       btn_width, btn_height, "Blue High");
    auton_btn_test = createButton(auton_screen, 10 + 2*col_spacing, start_y + row_spacing,
                                  btn_width, btn_height, "Test");

    // Selection display label
    auton_selection_label = lv_label_create(auton_screen, NULL);
    lv_obj_set_style(auton_selection_label, &style_text);
    lv_label_set_text(auton_selection_label, "No routine selected");
    lv_obj_align(auton_selection_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -40);

    // Confirm button
    auton_confirm_btn = createButton(auton_screen, 165, 195, 150, 35, "CONFIRM");
    lv_btn_set_style(auton_confirm_btn, LV_BTN_STYLE_REL, &style_confirm_btn);
}

void BrainUI::createOperationScreen() {
    // Create screen
    op_screen = lv_obj_create(NULL, NULL);
    lv_obj_set_style(op_screen, &style_screen);

    // Small logo in corner (when available)
    // op_logo = lv_img_create(op_screen, NULL);
    // lv_img_set_src(op_logo, &logo_ctrc_60x60);
    // lv_obj_align(op_logo, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 5);

    // Title
    lv_obj_t* title = lv_label_create(op_screen, NULL);
    lv_obj_set_style(title, &style_title);
    lv_label_set_text(title, "839Y - Push Back Robot");
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 5);

    // Telemetry labels
    op_position_label = lv_label_create(op_screen, NULL);
    lv_obj_set_style(op_position_label, &style_text);
    lv_label_set_text(op_position_label, "Position: X:0.0 Y:0.0 Theta:0.0");
    lv_obj_align(op_position_label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 35);

    op_battery_label = lv_label_create(op_screen, NULL);
    lv_obj_set_style(op_battery_label, &style_text);
    lv_label_set_text(op_battery_label, "Battery: ---%");
    lv_obj_align(op_battery_label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 65);

    op_temp_label = lv_label_create(op_screen, NULL);
    lv_obj_set_style(op_temp_label, &style_text);
    lv_label_set_text(op_temp_label, "Motor Temp: --C");
    lv_obj_align(op_temp_label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 95);

    op_status_label = lv_label_create(op_screen, NULL);
    lv_obj_set_style(op_status_label, &style_text);
    lv_label_set_text(op_status_label, "Status: Initializing");
    lv_obj_align(op_status_label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 125);
}

void BrainUI::createConfirmationScreen() {
    // Create screen
    confirm_screen = lv_obj_create(NULL, NULL);
    lv_obj_set_style(confirm_screen, &style_screen);

    // Confirmation label (will be updated when shown)
    confirm_label = lv_label_create(confirm_screen, NULL);
    lv_obj_set_style(confirm_label, &style_title);
    lv_label_set_text(confirm_label, "Autonomous Selected!");
    lv_obj_align(confirm_label, NULL, LV_ALIGN_CENTER, 0, -20);

    // Back button
    confirm_back_btn = createButton(confirm_screen, 165, 140, 150, 40, "Go Back");
}

void BrainUI::showConfirmation() {
    // Update confirmation text based on selection
    const char* auton_name = "Unknown";
    switch (selected_auton) {
        case AUTON_ROUTINE::RED_RUSH:
            auton_name = "Red Rush";
            break;
        case AUTON_ROUTINE::BLUE_RUSH:
            auton_name = "Blue Rush";
            break;
        case AUTON_ROUTINE::RED_HIGH_SCORE:
            auton_name = "Red High Score";
            break;
        case AUTON_ROUTINE::BLUE_HIGH_SCORE:
            auton_name = "Blue High Score";
            break;
        case AUTON_ROUTINE::SKILLS:
            auton_name = "Skills";
            break;
        case AUTON_ROUTINE::TEST:
            auton_name = "Test Routine";
            break;
        default:
            break;
    }

    char confirm_text[100];
    sprintf(confirm_text, "Selected:\n%s\n\nReady!", auton_name);
    lv_label_set_text(confirm_label, confirm_text);

    // Hide all other screens
    lv_obj_set_hidden(splash_screen, true);
    lv_obj_set_hidden(auton_screen, true);
    lv_obj_set_hidden(op_screen, true);

    // Show confirmation screen
    lv_obj_set_hidden(confirm_screen, false);
    lv_scr_load(confirm_screen);

    current_screen = UIScreen::CONFIRMATION;
}

// ============================================================================
// STYLE INITIALIZATION
// ============================================================================

void BrainUI::initStyles() {
    // Screen background style (black)
    lv_style_copy(&style_screen, &lv_style_plain);
    style_screen.body.main_color = LV_COLOR_BLACK;
    style_screen.body.grad_color = LV_COLOR_BLACK;

    // Button released style (dark gray with gold border)
    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_MAKE(40, 40, 40);
    style_btn_rel.body.grad_color = LV_COLOR_MAKE(40, 40, 40);
    style_btn_rel.body.border.color = LV_COLOR_MAKE(255, 215, 0); // Gold
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.radius = 5;
    style_btn_rel.text.color = LV_COLOR_WHITE;

    // Button pressed style (lighter gray with gold)
    lv_style_copy(&style_btn_pr, &lv_style_btn_pr);
    style_btn_pr.body.main_color = LV_COLOR_MAKE(80, 80, 80);
    style_btn_pr.body.grad_color = LV_COLOR_MAKE(80, 80, 80);
    style_btn_pr.body.border.color = LV_COLOR_MAKE(255, 215, 0);
    style_btn_pr.body.border.width = 3;
    style_btn_pr.body.radius = 5;
    style_btn_pr.text.color = LV_COLOR_WHITE;

    // Button selected style (gold background)
    lv_style_copy(&style_btn_selected, &style_btn_rel);
    style_btn_selected.body.main_color = LV_COLOR_MAKE(255, 215, 0);
    style_btn_selected.body.grad_color = LV_COLOR_MAKE(255, 180, 0);
    style_btn_selected.body.border.color = LV_COLOR_WHITE;
    style_btn_selected.text.color = LV_COLOR_BLACK;

    // Confirm button style (green)
    lv_style_copy(&style_confirm_btn, &style_btn_rel);
    style_confirm_btn.body.main_color = LV_COLOR_MAKE(0, 150, 0);
    style_confirm_btn.body.grad_color = LV_COLOR_MAKE(0, 100, 0);
    style_confirm_btn.body.border.color = LV_COLOR_WHITE;

    // Title text style (large, gold)
    lv_style_copy(&style_title, &lv_style_plain);
    style_title.text.color = LV_COLOR_MAKE(255, 215, 0);
    style_title.text.font = &lv_font_dejavu_20;

    // Regular text style (white)
    lv_style_copy(&style_text, &lv_style_plain);
    style_text.text.color = LV_COLOR_WHITE;
    style_text.text.font = &lv_font_dejavu_20;
}

// ============================================================================
// HELPER METHODS
// ============================================================================

lv_obj_t* BrainUI::createButton(lv_obj_t* parent, lv_coord_t x, lv_coord_t y,
                                lv_coord_t width, lv_coord_t height, const char* text) {
    // Create button
    lv_obj_t* btn = lv_btn_create(parent, NULL);
    lv_obj_set_size(btn, width, height);
    lv_obj_set_pos(btn, x, y);
    lv_btn_set_style(btn, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(btn, LV_BTN_STYLE_PR, &style_btn_pr);
    lv_obj_set_free_num(btn, 0);  // Will be set to identify button
    lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, btnCallback);

    // Create label on button
    lv_obj_t* label = lv_label_create(btn, NULL);
    lv_label_set_text(label, text);
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

    return btn;
}

// ============================================================================
// CALLBACK HANDLERS
// ============================================================================

lv_res_t BrainUI::btnCallback(lv_obj_t* btn) {
    if (!instance) return LV_RES_OK;

    // Identify which button was pressed based on pointer comparison
    if (btn == instance->auton_btn_red_rush) {
        instance->selected_auton = AUTON_ROUTINE::RED_RUSH;
        lv_label_set_text(instance->auton_selection_label, "Selected: Red Rush");
        lv_btn_set_style(btn, LV_BTN_STYLE_REL, &style_btn_selected);
    }
    else if (btn == instance->auton_btn_blue_rush) {
        instance->selected_auton = AUTON_ROUTINE::BLUE_RUSH;
        lv_label_set_text(instance->auton_selection_label, "Selected: Blue Rush");
        lv_btn_set_style(btn, LV_BTN_STYLE_REL, &style_btn_selected);
    }
    else if (btn == instance->auton_btn_red_high) {
        instance->selected_auton = AUTON_ROUTINE::RED_HIGH_SCORE;
        lv_label_set_text(instance->auton_selection_label, "Selected: Red High Score");
        lv_btn_set_style(btn, LV_BTN_STYLE_REL, &style_btn_selected);
    }
    else if (btn == instance->auton_btn_blue_high) {
        instance->selected_auton = AUTON_ROUTINE::BLUE_HIGH_SCORE;
        lv_label_set_text(instance->auton_selection_label, "Selected: Blue High Score");
        lv_btn_set_style(btn, LV_BTN_STYLE_REL, &style_btn_selected);
    }
    else if (btn == instance->auton_btn_skills) {
        instance->selected_auton = AUTON_ROUTINE::SKILLS;
        lv_label_set_text(instance->auton_selection_label, "Selected: Skills");
        lv_btn_set_style(btn, LV_BTN_STYLE_REL, &style_btn_selected);
    }
    else if (btn == instance->auton_btn_test) {
        instance->selected_auton = AUTON_ROUTINE::TEST;
        lv_label_set_text(instance->auton_selection_label, "Selected: Test");
        lv_btn_set_style(btn, LV_BTN_STYLE_REL, &style_btn_selected);
    }
    else if (btn == instance->auton_confirm_btn) {
        if (instance->selected_auton != AUTON_ROUTINE::NONE) {
            instance->auton_confirmed = true;
            instance->auton->set_selected_auton(instance->selected_auton);
            instance->showConfirmation();
        }
    }
    else if (btn == instance->confirm_back_btn) {
        instance->auton_confirmed = false;
        instance->selected_auton = AUTON_ROUTINE::NONE;
        instance->showAutonSelector();
    }

    return LV_RES_OK;
}

} // namespace subsystems
