/*
 * Copyright (c) 2026 Marcos Chow Castro
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <zmk/event_manager.h>

struct zmk_oled_brightness_changed {
    uint8_t brightness;
    uint8_t contrast;
};

ZMK_EVENT_DECLARE(zmk_oled_brightness_changed);
