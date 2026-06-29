/*
 * Copyright (c) 2026 Marcos Chow Castro
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

struct zmk_oled_brightness_state {
    uint8_t brightness;
    uint8_t contrast;
};

struct zmk_oled_brightness_state zmk_oled_brightness_get_state(void);
int zmk_oled_brightness_set(uint8_t brightness);
int zmk_oled_brightness_step(int16_t delta);
int zmk_oled_contrast_set(uint8_t contrast);
int zmk_oled_contrast_step(int16_t delta);
int zmk_oled_power_set(uint8_t mode);
int zmk_oled_power_profile_set(uint8_t profile);
