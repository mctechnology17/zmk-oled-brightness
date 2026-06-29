/*
 * Copyright (c) 2026 Marcos Chow Castro
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/init.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#if IS_ENABLED(CONFIG_ZMK_OLED_BRIGHTNESS_SETTINGS)
#include <zephyr/settings/settings.h>
#endif

#include <dt-bindings/zmk-oled-brightness/oled-brightness.h>
#include <zmk/events/oled_brightness_changed.h>
#include <zmk/oled_brightness.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_CHOSEN(zephyr_display)
static const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
#else
static const struct device *display;
#endif

#if IS_ENABLED(CONFIG_ZMK_OLED_BRIGHTNESS_PWM_BACKLIGHT) && DT_NODE_EXISTS(DT_NODELABEL(disp_bl))
#include <zephyr/drivers/led.h>
#define ZMK_OLED_BRIGHTNESS_HAS_PWM_BACKLIGHT 1
static const struct device *backlight = DEVICE_DT_GET(DT_PARENT(DT_NODELABEL(disp_bl)));
#define BACKLIGHT_LED DT_NODE_CHILD_IDX(DT_NODELABEL(disp_bl))
#else
#define ZMK_OLED_BRIGHTNESS_HAS_PWM_BACKLIGHT 0
#endif

#if IS_ENABLED(CONFIG_ZMK_OLED_BRIGHTNESS_DIRECT_I2C_CONTRAST) && DT_HAS_CHOSEN(zephyr_display) &&            \
    DT_ON_BUS(DT_CHOSEN(zephyr_display), i2c) &&                                                             \
    (DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_display), solomon_ssd1306fb) ||                                      \
     DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_display), sinowealth_sh1106))
#include <zephyr/drivers/i2c.h>
#define ZMK_OLED_BRIGHTNESS_HAS_DIRECT_I2C_CONTRAST 1
static const struct i2c_dt_spec oled_i2c = I2C_DT_SPEC_GET(DT_CHOSEN(zephyr_display));
#else
#define ZMK_OLED_BRIGHTNESS_HAS_DIRECT_I2C_CONTRAST 0
#endif

static struct zmk_oled_brightness_state state = {
    .brightness = CONFIG_ZMK_OLED_BRIGHTNESS_DEFAULT,
    .contrast = CONFIG_ZMK_OLED_BRIGHTNESS_DEFAULT,
};

static bool oled_powered = true;

#if IS_ENABLED(CONFIG_ZMK_OLED_BRIGHTNESS_SETTINGS)
static int settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg);

static struct settings_handler settings_conf = {
    .name = "zmk_oled_brightness",
    .h_set = settings_set,
};

static int save_state(void) {
    int ret = settings_save_one("zmk_oled_brightness/state", &state, sizeof(state));
    if (ret < 0) {
        LOG_WRN("Failed to persist OLED brightness state: %d", ret);
    }

    return ret;
}
#else
static int save_state(void) { return 0; }
#endif

static uint8_t clamp_u8(uint16_t value) {
    if (value < CONFIG_ZMK_OLED_BRIGHTNESS_MIN) {
        return CONFIG_ZMK_OLED_BRIGHTNESS_MIN;
    }

    if (value > CONFIG_ZMK_OLED_BRIGHTNESS_MAX) {
        return CONFIG_ZMK_OLED_BRIGHTNESS_MAX;
    }

    return value;
}

static void raise_changed(void) {
    raise_zmk_oled_brightness_changed((struct zmk_oled_brightness_changed){
        .brightness = state.brightness,
        .contrast = state.contrast,
    });
}

struct zmk_oled_brightness_state zmk_oled_brightness_get_state(void) { return state; }

static int set_oled_contrast(uint8_t contrast) {
#if ZMK_OLED_BRIGHTNESS_HAS_DIRECT_I2C_CONTRAST
    if (i2c_is_ready_dt(&oled_i2c)) {
        uint8_t cmd[] = {
            0x81,
            contrast,
        };

        int ret = i2c_burst_write_dt(&oled_i2c, 0x00, cmd, sizeof(cmd));
        if (ret == 0) {
            return 0;
        }

        LOG_WRN("OLED direct I2C contrast rejected: %d; trying display API", ret);
    }
#endif

    if (display == NULL || !device_is_ready(display)) {
        return -ENODEV;
    }

    return display_set_contrast(display, contrast);
}

static int send_oled_commands(const uint8_t *commands, size_t len) {
#if ZMK_OLED_BRIGHTNESS_HAS_DIRECT_I2C_CONTRAST
    if (i2c_is_ready_dt(&oled_i2c)) {
        return i2c_burst_write_dt(&oled_i2c, 0x00, (uint8_t *)commands, len);
    }
#endif

    return -ENOTSUP;
}

static int set_oled_brightness_internal(uint8_t brightness, bool persist) {
#if ZMK_OLED_BRIGHTNESS_HAS_PWM_BACKLIGHT
    if (!device_is_ready(backlight)) {
        return -ENODEV;
    }

    uint8_t next = CLAMP(brightness, 0, 100);
    int ret = led_set_brightness(backlight, BACKLIGHT_LED, next);
    if (ret < 0) {
        LOG_WRN("OLED PWM backlight rejected brightness: %d", ret);
        return ret;
    }

    if (state.brightness != next) {
        state.brightness = next;
        raise_changed();

        if (persist) {
            return save_state();
        }
    }

    return 0;
#else
    if (IS_ENABLED(CONFIG_ZMK_OLED_BRIGHTNESS_PWM_BACKLIGHT)) {
        LOG_WRN("OLED PWM backlight enabled but no disp_bl node found; using display API");
    }

    if (display == NULL || !device_is_ready(display)) {
        return -ENODEV;
    }

    uint8_t next = clamp_u8(brightness);
    int ret = set_oled_contrast(next);
    if (ret < 0) {
        LOG_WRN("OLED brightness-as-contrast unsupported or rejected: %d", ret);
        return ret;
    }

    if (state.brightness != next || state.contrast != next) {
        state.brightness = next;
        state.contrast = next;
        raise_changed();

        if (persist) {
            return save_state();
        }
    }

    return 0;
#endif
}

int zmk_oled_brightness_set(uint8_t brightness) {
    return set_oled_brightness_internal(brightness, true);
}

int zmk_oled_brightness_step(int16_t delta) {
    int16_t next = (int16_t)state.brightness + delta;

#if ZMK_OLED_BRIGHTNESS_HAS_PWM_BACKLIGHT
    if (next < 0) {
        next = 0;
    } else if (next > 100) {
        next = 100;
    }
#else
    if (next < CONFIG_ZMK_OLED_BRIGHTNESS_MIN) {
        next = CONFIG_ZMK_OLED_BRIGHTNESS_MIN;
    } else if (next > CONFIG_ZMK_OLED_BRIGHTNESS_MAX) {
        next = CONFIG_ZMK_OLED_BRIGHTNESS_MAX;
    }
#endif

    return zmk_oled_brightness_set((uint8_t)next);
}

static int set_oled_contrast_internal(uint8_t contrast, bool persist) {
    uint8_t next = clamp_u8(contrast);
    int ret = set_oled_contrast(next);
    if (ret < 0) {
        LOG_WRN("OLED contrast unsupported or rejected: %d", ret);
        return ret;
    }

    if (state.contrast != next) {
        state.contrast = next;
        raise_changed();

        if (persist) {
            return save_state();
        }
    }

    return 0;
}

int zmk_oled_contrast_set(uint8_t contrast) {
    return set_oled_contrast_internal(contrast, true);
}

int zmk_oled_contrast_step(int16_t delta) {
    int16_t next = (int16_t)state.contrast + delta;

    if (next < CONFIG_ZMK_OLED_BRIGHTNESS_MIN) {
        next = CONFIG_ZMK_OLED_BRIGHTNESS_MIN;
    } else if (next > CONFIG_ZMK_OLED_BRIGHTNESS_MAX) {
        next = CONFIG_ZMK_OLED_BRIGHTNESS_MAX;
    }

    return zmk_oled_contrast_set((uint8_t)next);
}

int zmk_oled_power_set(uint8_t mode) {
    bool next;

    switch (mode) {
    case ZMK_OLED_POWER_OFF:
        next = false;
        break;
    case ZMK_OLED_POWER_ON:
        next = true;
        break;
    case ZMK_OLED_POWER_TOGGLE:
        next = !oled_powered;
        break;
    default:
        return -EINVAL;
    }

    uint8_t command = next ? 0xAF : 0xAE;
    int ret = send_oled_commands(&command, sizeof(command));
    if (ret < 0) {
        if (display == NULL || !device_is_ready(display)) {
            return ret;
        }

        ret = next ? display_blanking_off(display) : display_blanking_on(display);
    }

    if (ret == 0) {
        oled_powered = next;
    }

    return ret;
}

int zmk_oled_power_profile_set(uint8_t profile) {
    uint8_t commands[6];
    uint8_t contrast;

    switch (profile) {
    case ZMK_OLED_POWER_PROFILE_LOW:
        contrast = 0x01;
        commands[0] = 0xD5; /* Display clock divide / oscillator frequency */
        commands[1] = 0x10;
        commands[2] = 0xD9; /* Pre-charge period */
        commands[3] = 0x11;
        commands[4] = 0xDB; /* VCOMH deselect level */
        commands[5] = 0x00;
        break;
    case ZMK_OLED_POWER_PROFILE_DEFAULT:
        contrast = CONFIG_ZMK_OLED_BRIGHTNESS_DEFAULT;
        commands[0] = 0xD5;
        commands[1] = 0x80;
        commands[2] = 0xD9;
        commands[3] = 0xF1;
        commands[4] = 0xDB;
        commands[5] = 0x20;
        break;
    case ZMK_OLED_POWER_PROFILE_HIGH:
        contrast = 0xFF;
        commands[0] = 0xD5;
        commands[1] = 0x80;
        commands[2] = 0xD9;
        commands[3] = 0xF1;
        commands[4] = 0xDB;
        commands[5] = 0x20;
        break;
    default:
        return -EINVAL;
    }

    int ret = send_oled_commands(commands, sizeof(commands));
    if (ret < 0) {
        LOG_WRN("OLED power profile command unsupported or rejected: %d", ret);
        return ret;
    }

    return zmk_oled_brightness_set(contrast);
}

static int zmk_oled_brightness_init(void) {
#if IS_ENABLED(CONFIG_ZMK_OLED_BRIGHTNESS_SETTINGS)
    settings_subsys_init();

    int ret = settings_register(&settings_conf);
    if (ret < 0) {
        LOG_ERR("Could not register OLED brightness settings (%d).", ret);
        return ret;
    }

    settings_load_subtree("zmk_oled_brightness");
#endif

    return 0;
}

SYS_INIT(zmk_oled_brightness_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#if IS_ENABLED(CONFIG_ZMK_OLED_BRIGHTNESS_SETTINGS)
static int settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
    if (!settings_name_steq(name, "state", NULL)) {
        return -ENOENT;
    }

    if (len != sizeof(state)) {
        LOG_WRN("Invalid OLED brightness state size: %d", len);
        return -EINVAL;
    }

    struct zmk_oled_brightness_state loaded_state;
    int ret = read_cb(cb_arg, &loaded_state, sizeof(loaded_state));
    if (ret <= 0) {
        LOG_WRN("Failed to read OLED brightness state: %d", ret);
        return ret;
    }

    loaded_state.brightness = clamp_u8(loaded_state.brightness);
    loaded_state.contrast = clamp_u8(loaded_state.contrast);
    state = loaded_state;

#if ZMK_OLED_BRIGHTNESS_HAS_PWM_BACKLIGHT
    ret = set_oled_brightness_internal(state.brightness, false);
#else
    ret = set_oled_contrast_internal(state.contrast, false);
#endif
    if (ret < 0) {
        LOG_WRN("Failed to apply persisted OLED brightness state: %d", ret);
        return ret;
    }

    raise_changed();
    return 0;
}

#endif
