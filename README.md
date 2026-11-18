# ZMK OLED Brightness

Runtime brightness/contrast behaviors for ZMK display devices.

For standard Corne OLEDs using Zephyr's SSD1306 driver, brightness is handled as
display contrast. The SSD1306 driver implements `display_set_contrast()` and
returns `-ENOTSUP` for `display_set_brightness()`, so `&ol_brt` falls back to
contrast when no PWM backlight is configured.

For I2C SSD1306/SH1106 OLEDs, the module sends the same contrast command style
used by QMK: control byte `0x00`, command `0x81`, and value `0..255`. If that
direct I2C path is not available, it falls back to Zephyr's
`display_set_contrast()`.

For LCD/TFT displays with a physical backlight, enable
`CONFIG_ZMK_OLED_BRIGHTNESS_PWM_BACKLIGHT=y` and provide a `pwm-leds` child
labeled `disp_bl`. In that mode, `&ol_brt <0-100>` uses `led_set_brightness()`
like Prospector-style display backlights.

Example bindings:

```dts
&ol_con 0
&ol_cond
&ol_con 255
&ol_cons ZMK_OLED_CONTRAST_UP 16
&ol_cons ZMK_OLED_CONTRAST_DOWN 16
&ol_brt 50
&ol_brts ZMK_OLED_BRIGHTNESS_UP 10
&ol_brts ZMK_OLED_BRIGHTNESS_DOWN 10
&ol_pwr ZMK_OLED_POWER_TOGGLE
&ol_prof ZMK_OLED_POWER_PROFILE_LOW
```

`&ol_cond` uses `CONFIG_ZMK_OLED_BRIGHTNESS_DEFAULT`, which defaults to
Zephyr/ZMK's SSD1306 default contrast (`CONFIG_SSD1306_DEFAULT_CONTRAST`).

PWM backlight example:

```dts
/ {
    pwmleds {
        compatible = "pwm-leds";
        disp_bl: pwm_led_0 {
            pwms = <&pwm0 0 PWM_MSEC(1) PWM_POLARITY_NORMAL>;
        };
    };
};
```

Build module:

```sh
-DZMK_EXTRA_MODULES="/boards/zmk-config-modules-local/zmk-oled-brightness"
-DEXTRA_DTC_OVERLAY_FILE="/boards/zmk-config-modules-local/zmk-oled-brightness/examples/corne/oled_brightness.overlay"
```

See `examples/corne/oled_brightness.conf` and
`examples/corne/oled_brightness.keymap` for the config and keymap fragments
users should copy into their keyboard config.

## Versions, branches and tags

All my ZMK projects follow the same rules. The idea is that every project can be paired with ZMK in its different versions (v0.3, v0.4, ...), so you always know which branch or tag fits the ZMK version you use.

What to write as `revision:` in your `west.yml`:

| You want | `revision:` |
|---|---|
| The latest stable version | `main` |
| Stay on ZMK v0.3 and keep receiving fixes | `zmk-v0.3` |
| A fixed version that never changes | a tag, for example `v0.0.3` |

Branches:

- `main`: the stable version. It follows the newest ZMK version that the project supports.
- `zmk-v0.3`: maintenance for ZMK v0.3. It only receives fixes, never new features. When the project moves to a newer ZMK, the next branch is created the same way (`zmk-v0.4`, ...).
- `dev`: development. It can be broken at any time, do not use it for your keyboard.

Tags:

- `v0.0.3`: the common starting point of all my ZMK projects (Nov 18, 2026), tested with ZMK v0.3.
- `v0.0.4`, `v0.0.5`, ...: new versions, released from `main`. `v0.0.4` is planned as the update to the newest ZMK.
- `v0.0.3.1`, `v0.0.3.2`, ...: fixes for the ZMK v0.3 line, released from `zmk-v0.3` (`v0.0.3.1-fix` while a fix is still being tested).

Every release says which ZMK version it was tested with. The full list of changes is in [CHANGELOG.txt](CHANGELOG.txt).

## References and Thanks

This module was built after reviewing real ZMK display/backlight work from:

- Prospector ZMK module:
  https://github.com/carrefinho/prospector-zmk-module
  The `prospector_adapter/src/brightness.c` implementation showed the practical
  `pwm-leds` + `led_set_brightness()` path for physical LCD/TFT backlights.

- QMK Firmware:
  https://github.com/qmk/qmk_firmware
  The OLED driver under `drivers/oled/oled_driver.c` was the reference for the
  SSD1306/SH1106 contrast command path: command `0x81` followed by a `0..255`
  contrast value over I2C.

- RolioFirmware:
  https://github.com/MickiusMousius/RolioFirmware
  Its shield configuration uses ZMK backlight support with PWM-backed LEDs for
  display/backlight control.

- zmk-userspace:
  https://github.com/elpekenin/zmk-userspace
  Used by RolioFirmware and useful as a reference for extending ZMK behavior and
  hardware control patterns.

Thanks to those developers for publishing the projects that made this approach
easier to understand and adapt.
