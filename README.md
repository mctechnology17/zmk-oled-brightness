<!-- mc-follow-top:start -->
<div align="center">

  <a href="https://youtube.com/@mctechnology17">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="https://github.com/mctechnology17/mctechnology17/blob/main/src/mctechnology_banner_dark.png?raw=true" />
      <img alt="MC Technology" width="420px" src="https://github.com/mctechnology17/mctechnology17/blob/main/src/mctechnology_banner_light.png?raw=true" />
    </picture>
  </a>
  <br>
  <a href="https://youtube.com/@mctechnology17"><img align="center" alt="MC Technology | YouTube" width="22px" src="https://github.com/mctechnology17/mctechnology17/blob/main/src/youtube.png" /></a>
  <a href="https://www.instagram.com/mctechnology17/"><img align="center" alt="MC Technology | Instagram" width="22px" src="https://github.com/mctechnology17/mctechnology17/blob/main/src/instagram.png" /></a>
  <a href="https://www.facebook.com/mctechnology17/"><img align="center" alt="MC Technology | Facebook" width="22px" src="https://github.com/mctechnology17/mctechnology17/blob/main/src/facebook.png" /></a>
  <a href="https://www.tiktok.com/@mctechnology17"><img align="center" alt="MC Technology | TikTok" width="22px" src="https://github.com/mctechnology17/mctechnology17/blob/main/src/tiktok.png" /></a>
  <a href="https://t.me/mctechnology17"><img align="center" alt="MC Technology | Telegram" width="22px" src="https://github.com/mctechnology17/mctechnology17/blob/main/src/telegram.png" /></a>
  <a href="https://discord.gg/7zs2fezBbv"><img align="center" alt="MC Technology | Discord" width="22px" src="https://github.com/mctechnology17/mctechnology17/blob/main/src/discord.png" /></a>

</div>
<!-- mc-follow-top:end -->

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

<!-- mc-follow:start -->
## Support this project

This is a personal project that I build in my free time. If it is useful to
you, the best way to help is to follow and subscribe:

[YouTube](https://youtube.com/@mctechnology17) · [Instagram](https://www.instagram.com/mctechnology17/) · [Facebook](https://www.facebook.com/mctechnology17/) · [TikTok](https://www.tiktok.com/@mctechnology17)

News and community: [Telegram](https://t.me/mctechnology17) · [Discord](https://discord.gg/7zs2fezBbv)

I make videos about split keyboards, ZMK and open source (in Spanish, long
videos with English subtitles). Every follow and every subscription helps me a
lot and keeps me motivated to bring more to the open source community.

Thank you for every report and every issue: they make these projects better.
<!-- mc-follow:end -->
