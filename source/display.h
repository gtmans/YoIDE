#pragma once
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <DigiFont.h>
//#define USESPRITE //ook in display.cpp
//#include "display_layout.h"
class LGFX;
class Audio;

class RadioDisplay
{
public:
    RadioDisplay            ();        // <-- DIT ONTBREEKT
    void begin              (LGFX* lcd);
    void attachAudio        (Audio* a);
    void loop               ();
    // content
    void setLogo            (const char* filename);
    void setStation         (const char* txt);
    void setTitle           (const char* txt);
    // time
    void drawTime           ();
    void drawDate           ();
    void drawAudioInfo      ();
    void drawPlayTime       ();
    void updatePlayTime     ();
    void resetPlayTime      ();
    // status
    void setBuffer          (uint8_t b);
    void setVU              (uint8_t v);
    void setPlaying         (bool p);
    const char*             getDisplayTitle();
    //weer
    void updateWeather      ();

private:
    LGFX*   tft     =       nullptr;
    Audio*  audio   =       nullptr;
#ifdef USESPRITE
    LGFX_Sprite*            titleSprite = nullptr;
#else
    uint16_t lastTitleOffset      = 0;//OPT
#endif
    DigiFont                digi;
    static RadioDisplay*    digiInstance;
    static void digiLineH   (int x0, int x1, int y, int c);
    static void digiLineV   (int x, int y0, int y1, int c);
    static void digiRect    (int x, int y, int w, int h, int c);

    // ===== Typewriter =====
//  static char visible[256];     // wat er nu op het scherm staat
//  static int  visLen = 0;       // lengte van zichtbare tekst
//  static int  cursorX = TITLE_X;
  //char visible[256];     // wat er nu op het scherm staat
  //int  visLen=0;           // lengte van zichtbare tekst
  //int  cursorX= 8;
  int titleXPos = 8;//TITLE_X;
  int titleSrcIndex = 0;

    // --------------------
    // data
    // --------------------
    char     logo[64]         = "";
    char  station[64]         = "";
    char    title[128]        = "";
  //char alttitle[128]        = "Nu even tijd voor heeeeel iets anders want voor je het weet is de tijd voorbij";
    char alttitle[128]        = "";
  //char extendedTitle[130];
    char   oldDate[10]        = "";
    bool     newtitle         = false;
    uint16_t titleOffset      = 0;
    uint32_t lastTitleScroll  = 0;
    bool     titleScrolling   = false;
    bool     playing          = false;
    uint8_t  lastVolume       = 255;   // force eerste update
    uint8_t  buffer           = 0;
    uint8_t  vuLeft           = 0;
    uint8_t  vuRight          = 0;
    uint8_t  peakLeft         = 0;
    uint8_t  peakRight        = 0;
    uint32_t lastVUUpdate     = 0;
    uint32_t titleStartMillis = 0;
    uint32_t lastPlayTimeDraw = 0;
    uint32_t lastUpdate       = 0;
    uint32_t audioBitrate     = 0;
    int      audioCodec       = -1;
    String   oldClock         = "";
    char     playTime[6]      = "00:00";

uint8_t seg7[128];
void initSeg7() {
  // digits
  seg7['0'] = 0b0111111;
  seg7['1'] = 0b0000110;
  seg7['2'] = 0b1011011;
  seg7['3'] = 0b1001111;
  seg7['4'] = 0b1100110;
  seg7['5'] = 0b1101101;
  seg7['6'] = 0b1111101;
  seg7['7'] = 0b0000111;
  seg7['8'] = 0b1111111;
  seg7['9'] = 0b1101111;
  // uppercase letters
  seg7['A'] = 0b1110111;
  seg7['B'] = 0b1111100;
  seg7['C'] = 0b1011000;//0b0111001
  seg7['D'] = 0b1011110;
  seg7['E'] = 0b1111001;
  seg7['F'] = 0b1110001;
  seg7['G'] = 0b1101111;
  seg7['H'] = 0b1110100;//0b1110110
  seg7['I'] = 0b0000100;//0b0000110
  seg7['J'] = 0b0001110;
  seg7['K'] = 0b1110110;
  seg7['L'] = 0b0111000;
  seg7['M'] = 0b1010100;
  seg7['N'] = 0b1010100;//0b0110111;
  seg7['O'] = 0b1011100;//0b0111111;
  seg7['P'] = 0b1110011;
  seg7['Q'] = 0b1100111;
  seg7['R'] = 0b1010000;//0b0110011;
  seg7['S'] = 0b1101101;
  seg7['T'] = 0b1111000;
  seg7['U'] = 0b0011100;//0b0111110;
  seg7['V'] = 0b0101010;//0b0111110
  seg7['W'] = 0b0011100;//
  seg7['X'] = 0b1110110;
  seg7['Y'] = 0b1101110;
  seg7['Z'] = 0b1011011;
  // extra tekens
  seg7['('] = 0b0111001;
  seg7[')'] = 0b0001111;
  seg7['-'] = 0b1000000;
  seg7['\'']= 0b0000010;//'
  seg7['\"']= 0b0100010;//"
  seg7['\\']= 0b1100100;//backward slash
  seg7['/'] = 0b1010010;//forward slash
  seg7['&'] = seg7['-'];
  seg7['*'] = seg7['-'];//˚C
  seg7['˚'] = 0b1100011;// nul boven werkt niet
  seg7['$'] = 0b1100011;// nul boven
  seg7['<'] = 0b1100011;// nul boven
  seg7['>'] = 0b1011100;// nul onder
  seg7['^'] = 0b0111001;// caps C
}

  //uint32_t titlePauseUntil  = 0;
  //bool     titleNeedsScroll = false;
  //bool vuChanged = false;
  //bool VU_LEVEL = false;
  //uint32_t lastTime = 0;

    // --------------------
    // dirty flags
    // --------------------
    bool dirtyAudioInfo = true;
    bool dirtyPlayTime  = false;//true;
    bool dirtyLogo      = false;
    bool dirtyStation   = false;
    bool dirtyTitle     = false;
    bool dirtyVolume    = false;
    bool dirtyBuffer    = false;
    bool dirtyVU        = false;
    bool dirtyPlay      = false;
    bool alttxt         = false;
    // --------------------
    // drawing
    // --------------------
    void updateAudio    ();
    void drawLogo       ();
    void drawStation    ();
    void drawTitle      (const char* text);
    void drawTitleTypewriter (const char* fullText);//Typewriter
    void drawDigiTitle  (const char* fullText);
    void drawVolume     ();
    void drawBuffer     ();
    void drawVU         ();
    void drawVU2        ();
    void drawPlay       ();
    void drawDigiMonth  (const char* month, int x, int y);
    void drawDigiLetter (char c, int x, int y);
    void drawDigiLetter2(char c, int x, int y);
    void clearArea      (int x,int y,int w,int h);
};