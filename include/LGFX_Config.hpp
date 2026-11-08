#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_Parallel8 _bus_instance;
  lgfx::Light_PWM _light_instance;

public:
  LGFX(void) {
    // 1. Enable power rail for LCD and touch (GPIO 15)
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);

    // 2. Configure 8-bit parallel bus (I8080)
    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = 16000000;
      cfg.pin_wr = 8;
      cfg.pin_rd = 9;
      cfg.pin_rs = 7;   // D/C (Data/Command)
      cfg.pin_d0 = 39;
      cfg.pin_d1 = 40;
      cfg.pin_d2 = 41;
      cfg.pin_d3 = 42;
      cfg.pin_d4 = 45;
      cfg.pin_d5 = 46;
      cfg.pin_d6 = 47;
      cfg.pin_d7 = 48;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    // 3. Configure ST7789 panel
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 6;
      cfg.pin_rst = 5;
      cfg.pin_busy = -1;

      cfg.memory_width = 240;
      cfg.memory_height = 320;
      cfg.panel_width = 170;
      cfg.panel_height = 320;
      cfg.offset_x = 35;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;

      _panel_instance.config(cfg);
    }

    // 4. Backlight PWM (GPIO 38)
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 38;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 0;

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};
