#pragma once
// ============================================================
// Display layout 320x240 
// ============================================================
// ------------------------------------------------------------
// Scherm
// ------------------------------------------------------------
#define SCREEN_W        320
#define SCREEN_H        240
// ------------------------------------------------------------
// Kleuren
// ------------------------------------------------------------
#define BG_COLOR        TFT_BLUE
#define PANEL_COLOR     TFT_DARKGREY
#define COLOR_TEXT      TFT_WHITE
#define COLOR_ACCENT    TFT_CYAN
#define COLOR_VOLUME    TFT_GREEN
#define COLOR_BUFFER    TFT_CYAN
#define COLOR_DATE      TFT_WHITE
#define COLOR_TIME      TFT_CYAN
#define COLOR_VU        TFT_GREEN
#define COLOR_PANEL     TFT_DARKGREY
#define COLOR_ERROR     TFT_RED
#define COLOR_AUDIOINFO 0xFFCC48//!=TFT_GOLD
// ------------------------------------------------------------
// Fonts https://github.com/lovyan03/LovyanGFX/blob/master/src/lgfx/v1/lgfx_fonts.hpp
// lgfx::FixedBMPfont AsciiFont8x16;
// lgfx::FixedBMPfont AsciiFont24x48;
// ------------------------------------------------------------
#define DEFTXTSIZE  1
#define FONT_SMALL  (&fonts::FreeSans9pt7b)
#define FONT_MEDIUM (&fonts::FreeSans18pt7b)
//#define FONT_MEDIUM (&fonts::FreeSansBold18pt7b)
//#define FONT_LARGE  (&fonts::FreeSans24pt7b)
#define FONT_NORMAL FONT_SMALL
// ------------------------------------------------------------
// Audio info -> volume
// ------------------------------------------------------------
#define AUDIOINFO_X 230
#define AUDIOINFO_Y 8
#define AUDIOINFO_W 50
#define AUDIOINFO_H 38
// ------------------------------------------------------------
// Logo gebied
// ------------------------------------------------------------
#define LOGO_X      40
#define LOGO_Y      58
#define LOGO_W      240
#define LOGO_H      120
// ------------------------------------------------------------
// Tijd
// ------------------------------------------------------------
#define CLOCK_X     0//LOGO_X
#define CLOCK_Y     12//8
#define CLOCK_W     50
#define CLOCK_H     20
// ------------------------------------------------------------
// Playing time     PTIME
// ------------------------------------------------------------
#define PT_X        125//120//130//110//120
#define PT_Y        12//8
#define PT_W        50
#define PT_H        20
// ------------------------------------------------------------
// Datum
// ------------------------------------------------------------
#define DATE_X      292//280
#define DATE_Y      LOGO_Y-3
#define DATE_W      30//40
#define DATE_H      120//6*17
// ------------------------------------------------------------
// Station naam
// ------------------------------------------------------------
#define STATION_X   LOGO_X//+(LOGO_W/4)//0//LOGO_X  // gelijk lijnen met logo
#define STATION_Y   LOGO_Y+(LOGO_H/2)//30
#define STATION_W   LOGO_W/2//280
#define STATION_H   20
// ------------------------------------------------------------
// Titel ticker
// ------------------------------------------------------------
#define TITLE_X     8
#define TITLE_Y     190
#define TITLE_W     304
#define TITLE_H     35      //titleSprite->fontHeight(FreeSans18pt7b)
// ------------------------------------------------------------
// play/pause 
// ------------------------------------------------------------
#define STATUS_X    96//90//100//290
#define STATUS_Y    14//12//(6*17)+LOGO_Y
#define STATUS_W    35
#define STATUS_H    22
// ------------------------------------------------------------
// VU meter - naast play/pause VU_X+VU_W (135) tot BUFFER_X (160) wissen w=BUFFER_X-VU_X
// ------------------------------------------------------------
#define VU_X        45
#define VU_Y        211
#define VU_W        90
#define VU_H        20
// ------------------------------------------------------------
// VU2 meter - links van logo, verticaal, blokjes
// ------------------------------------------------------------
#define VU2_X       0
#define VU2_Y       LOGO_Y
#define VU2_W       40
#define VU2_H       120
// ------------------------------------------------------------
// Buffer meter W = 280 LOGO_X+LOGO_W-BUFFER_X
// ------------------------------------------------------------
#define BUFFER_X    0
#define BUFFER_Y    232
#define BUFFER_W    320
#define BUFFER_H    8
/* ------------------------------------------------------------
// Volume meter
// ------------------------------------------------------------
#define VOLUME_X    160
#define VOLUME_Y    222
#define VOLUME_W    60
#define VOLUME_H    10
// ------------------------------------------------------------
// Volume waarde
// ------------------------------------------------------------
#define VOLUME_X    290
#define VOLUME_Y    20
#define VOLUME_W    35
#define VOLUME_H    17
*/
#define VOLUME_X    290//245//290
#define VOLUME_Y    12//8//20
#define VOLUME_W    35//70//35
#define VOLUME_H    32//17
