/*
 * Copyright (c) 2026 Marcos Chow Castro
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_oled_brightness_behavior_oled_brightness

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/oled_brightness.h>

static int behavior_oled_brightness_init(const struct device *dev) { return 0; }

static int on_oled_brightness_pressed(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return zmk_oled_brightness_set((uint8_t)binding->param1);
}

static int on_oled_brightness_released(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_oled_brightness_driver_api = {
    .binding_pressed = on_oled_brightness_pressed,
    .binding_released = on_oled_brightness_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, behavior_oled_brightness_init, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_oled_brightness_driver_api);
