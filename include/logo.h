/**
 * @file logo.h
 * @brief Team 839Y CTRC Logo - LVGL 5.3 Compatible
 *
 * Logo placeholder for VEX V5 Brain display
 * To add your actual logo, follow instructions in LOGO_CONVERSION_INSTRUCTIONS.md
 */

#ifndef LOGO_H
#define LOGO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "display/lvgl.h"

// Placeholder logo data (1x1 pixel black image)
static const uint8_t logo_ctrc_200x200_map[] = {
    0x00, 0x00  // Single black pixel in RGB565
};

// Logo image descriptor (LVGL 5.3 format)
const lv_img_dsc_t logo_ctrc_200x200 = {
    .header = {
        .cf = LV_IMG_CF_TRUE_COLOR,
        .always_zero = 0,
        .reserved = 0,
        .w = 1,  // Placeholder 1x1 pixel
        .h = 1,
    },
    .data_size = sizeof(logo_ctrc_200x200_map),
    .data = logo_ctrc_200x200_map,
};

#ifdef __cplusplus
}
#endif

#endif // LOGO_H
