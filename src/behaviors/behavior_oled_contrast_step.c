/*
 * Copyright (c) 2026 Marcos Chow Castro
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_oled_brightness_behavior_oled_contrast_step

#include <errno.h>

#include <drivers/behavior.h>
#include <dt-bindings/zmk-oled-brightness/oled-brightness.h>
#include <zmk/behavior.h>
#include <zmk/oled_brightness.h>

static int behavior_oled_contrast_step_init(const struct device *dev) { return 0; }

static int on_oled_contrast_step_pressed(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    uint8_t step = binding->param2 ? binding->param2 : CONFIG_ZMK_OLED_BRIGHTNESS_STEP;

    switch (binding->param1) {
    case ZMK_OLED_CONTRAST_UP:
        return zmk_oled_contrast_step(step);
    case ZMK_OLED_CONTRAST_DOWN:
        return zmk_oled_contrast_step(-(int16_t)step);
    default:
        return -EINVAL;
    }
}

static int on_oled_contrast_step_released(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_oled_contrast_step_driver_api = {
    .binding_pressed = on_oled_contrast_step_pressed,
    .binding_released = on_oled_contrast_step_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, behavior_oled_contrast_step_init, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_oled_contrast_step_driver_api);
