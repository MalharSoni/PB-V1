/**
 * @file brain_ui.hpp
 * @brief LVGL-based brain screen UI system for Team 839Y
 *
 * Provides professional UI with logo, autonomous selector, and telemetry display
 * for the VEX V5 Brain's 480x240 touchscreen.
 */

#ifndef BRAIN_UI_HPP
#define BRAIN_UI_HPP

#include "pros/apix.h"
#include "robot/auton.hpp"

namespace subsystems {

/**
 * @brief UI screen modes
 */
enum class UIScreen {
    SPLASH,          // Initial startup screen with logo
    AUTON_SELECTOR,  // Autonomous routine selection
    OPERATION,       // Live telemetry during operation
    CONFIRMATION     // Confirmation of selected autonomous
};

/**
 * @brief Brain screen UI manager
 *
 * Manages all LVGL UI screens including splash, autonomous selector,
 * and operation telemetry display with Team 839Y logo.
 */
class BrainUI {
public:
    /**
     * @brief Construct BrainUI
     * @param auton Pointer to autonomous subsystem for integration
     */
    BrainUI(Auton* auton);

    /**
     * @brief Initialize LVGL and create all UI components
     * Call this in initialize() before showing any screens
     */
    void init();

    /**
     * @brief Show splash screen with logo and initialization status
     * Displays team logo and "Initializing..." message
     */
    void showSplash();

    /**
     * @brief Show autonomous selector screen
     * Interactive touchscreen buttons for selecting autonomous routine
     */
    void showAutonSelector();

    /**
     * @brief Show operation screen with telemetry
     * Displays robot position, battery, temperatures, and status
     */
    void showOperationScreen();

    /**
     * @brief Update telemetry values on operation screen
     * Call this repeatedly in opcontrol() to refresh live data
     */
    void updateTelemetry();

    /**
     * @brief Get currently selected autonomous routine
     * @return Selected autonomous routine enum
     */
    AUTON_ROUTINE getSelectedAuton() const { return selected_auton; }

    /**
     * @brief Check if autonomous selection is confirmed
     * @return true if user has confirmed selection
     */
    bool isAutonConfirmed() const { return auton_confirmed; }

private:
    // ========================================================================
    // MEMBER VARIABLES
    // ========================================================================

    Auton* auton;                    // Pointer to autonomous subsystem
    UIScreen current_screen;         // Current active screen
    AUTON_ROUTINE selected_auton;    // Selected autonomous routine
    bool auton_confirmed;            // Whether selection is confirmed

    // LVGL Objects - Splash Screen
    lv_obj_t* splash_screen;
    lv_obj_t* splash_logo;
    lv_obj_t* splash_status_label;

    // LVGL Objects - Autonomous Selector
    lv_obj_t* auton_screen;
    lv_obj_t* auton_logo;
    lv_obj_t* auton_title_label;
    lv_obj_t* auton_btn_red_rush;
    lv_obj_t* auton_btn_blue_rush;
    lv_obj_t* auton_btn_red_high;
    lv_obj_t* auton_btn_blue_high;
    lv_obj_t* auton_btn_skills;
    lv_obj_t* auton_btn_test;
    lv_obj_t* auton_confirm_btn;
    lv_obj_t* auton_selection_label;

    // LVGL Objects - Operation Screen
    lv_obj_t* op_screen;
    lv_obj_t* op_logo;
    lv_obj_t* op_position_label;
    lv_obj_t* op_battery_label;
    lv_obj_t* op_temp_label;
    lv_obj_t* op_status_label;

    // LVGL Objects - Confirmation Screen
    lv_obj_t* confirm_screen;
    lv_obj_t* confirm_label;
    lv_obj_t* confirm_back_btn;

    // Styles (must be static/global to prevent deallocation)
    static lv_style_t style_screen;
    static lv_style_t style_btn_rel;
    static lv_style_t style_btn_pr;
    static lv_style_t style_btn_selected;
    static lv_style_t style_title;
    static lv_style_t style_text;
    static lv_style_t style_confirm_btn;

    // ========================================================================
    // PRIVATE METHODS - Screen Creation
    // ========================================================================

    /**
     * @brief Create splash screen objects
     */
    void createSplashScreen();

    /**
     * @brief Create autonomous selector screen objects
     */
    void createAutonScreen();

    /**
     * @brief Create operation screen objects
     */
    void createOperationScreen();

    /**
     * @brief Create confirmation screen objects
     */
    void createConfirmationScreen();

    /**
     * @brief Initialize all LVGL styles
     */
    void initStyles();

    /**
     * @brief Helper to create a button with label
     * @param parent Parent object
     * @param x X position
     * @param y Y position
     * @param width Button width
     * @param height Button height
     * @param text Button text
     * @return Created button object
     */
    lv_obj_t* createButton(lv_obj_t* parent, lv_coord_t x, lv_coord_t y,
                          lv_coord_t width, lv_coord_t height, const char* text);

    /**
     * @brief Show confirmation screen with selected autonomous
     */
    void showConfirmation();

    // ========================================================================
    // STATIC CALLBACK HANDLERS
    // ========================================================================

    /**
     * @brief Button callback handler
     * @param btn Button that was pressed
     * @return LV_RES_OK
     */
    static lv_res_t btnCallback(lv_obj_t* btn);

    /**
     * @brief Static pointer to instance for callbacks
     */
    static BrainUI* instance;
};

} // namespace subsystems

#endif // BRAIN_UI_HPP
