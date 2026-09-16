//https://nl.aliexpress.com/item/1005007524304778.html 2.8inch
//https://randomnerdtutorials.com/esp32-cheap-yellow-display-cyd-pinout-esp32-2432s028r/
/*ESP32 touchscreen 2,8 inch TFT LCD-scherm ESP-WROOM-32 st7789 resistieve aanraakmodule 240 * 320 ESP32-2432S028R Bluetooth WIFI*/
/*
CYD clone 2.8inch
opdruk:ESP32-2432S028
bijzonderheden:vermoedelijk 2432S028R 
Sound: LTK8002D 5424KKS GPIO26
connectors:
jst4 linkerzijde boven USBC
VIN
TX
RX
GND

jst2 bovenzijde links SPEAK
BAT-
BAT+

jst4 onderzijde rechts
GND
GPIO33?
GPIO22
GPIO21

jst4 onderzijde 2
GND
GPIO22
GPIO21
3V3
*/
#pragma once
// ---------------- SD ----------------
#include  <SD.h>
#include  <SPI.h>
#include <LovyanGFX.hpp>

const int buzzer    = 26;
const int PIN_RED   = 4;
const int PIN_GREEN = 17;
const int PIN_BLUE  = 16;

#define SD_CS   5
#define SD_SCK  18 
#define SD_MISO 19 
#define SD_MOSI 23
  
  #define           def_rot 3//1//2//3
//SPIClass          sdSPI(VSPI);

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789   _panel;
  lgfx::Bus_SPI        _bus;
  lgfx::Light_PWM      _light;
  lgfx::Touch_XPT2046  _touch;

public:
  LGFX() {
    // --- TFT SPI-bus (HSPI) ---
    {
      auto cfg = _bus.config();
      cfg.spi_host  = HSPI_HOST;
      cfg.spi_mode  = 0;
      cfg.freq_write= 40000000;
      cfg.freq_read = 16000000;

      cfg.pin_sclk  = 14;
      cfg.pin_mosi  = 13;
      cfg.pin_miso  = 12;
      cfg.pin_dc    = 2;

      _bus.config     (cfg);
      _panel.setBus   (&_bus);
    }
    // --- TFT panel ---
    {
      auto cfg =        _panel.config();
      cfg.pin_cs        = 15;
      cfg.pin_rst       = -1;
      cfg.panel_width   = 240;
      cfg.panel_height  = 320;
      cfg.memory_width  = 240;
      cfg.memory_height = 320;
      cfg.invert        = false;
      cfg.rgb_order     = false;
      _panel.config     (cfg);
    }
    // --- Backlight ---
    {
      auto cfg =        _light.config();
      cfg.pin_bl        = 21;
      cfg.freq          = 44100;
      cfg.pwm_channel   = 7;
      _light.config     (cfg);
      _panel.setLight   (&_light);
    }
    // --- Touch (XPT2046 op software-SPI) ---
    {
      auto cfg =        _touch.config();
      cfg.spi_host      = -1;        // software SPI
      cfg.freq          = 2500000;

      cfg.pin_sclk      = 25;
      cfg.pin_mosi      = 32;
      cfg.pin_miso      = 39;

      cfg.pin_cs        = 33;
      cfg.pin_int       = 36;

      cfg.x_min         = 3860;
      cfg.x_max         = 280;
      cfg.y_min         = 340;
      cfg.y_max         = 3860;

      _touch.config     (cfg);
      _panel.setTouch   (&_touch);
    }

    setPanel(&_panel);
  }
};

  extern LGFX tft;        // in AOKIN_ESP32_2432S028_ST7789.cpp
  extern SPIClass sdSPI;  // in AOKIN_ESP32_2432S028_ST7789.cpp

/*
#define use_cpp
#ifdef  use_cpp
  extern LGFX tft;        // in AOKIN_ESP32_2432S028_ST7789.cpp
  extern SPIClass sdSPI;  // in AOKIN_ESP32_2432S028_ST7789.cpp
#else
  SPIClass sdSPI(VSPI);
  LGFX tft;
#endif
*/
/*
//AOKIN_ESP32_2432S028_ST7789.cpp:
#include "AOKIN_ESP32_2432S028_ST7789.h"
SPIClass sdSPI(VSPI);
LGFX     tft;
*/
