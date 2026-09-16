#pragma once
//#define USESPRITE  //ook in display.h
//define  USEFONT
//define  TYPELETTER
// ============================================================
// YoRadio-Light V2.3 display.cpp
// ============================================================
#include    "tftkeuze.h"
#ifdef      USE24
  #include  "DIYTZT_ESP32_024_ILI9342.h"
#else
  #include  "AOKIN_ESP32_2432S028_ST7789.h"
#endif
#include    "display.h"
#include    "display_layout.h"
#include    <SD.h>
#include    <Audio.h>
//#define   CORE_DEBUG_LEVEL 5
//#include  "esp_heap_caps.h"
//weer
#include    <HTTPClient.h>
#include    <ArduinoJson.h>

RadioDisplay* RadioDisplay::digiInstance = nullptr;
RadioDisplay::RadioDisplay():digi(digiLineH, digiLineV, digiRect){}

const char* RadioDisplay::getDisplayTitle()
{ // commercial break -> do wheather
  if (!title[0] || strcmp(title, "commercial-in") == 0|| strcmp(title, "Qmusic nieuws") == 0){return alttitle;}
  return title;
}

// ------------------------------------------------------------
// begin
// ------------------------------------------------------------
void  RadioDisplay::begin(LGFX* lcd) {
  tft = lcd;
  digiInstance =    this;
  digi.setSize7     (18, 28, 4, 2);
  digi.setSpacing   (2);//3
  digi.setColors    (COLOR_ACCENT, BG_COLOR);
  initSeg7          ();
  tft->setRotation  (def_rot);
  Serial.println    ("dev_rot voor dit scherm ="+String(def_rot));
  tft->setSwapBytes (true);
  tft->fillScreen   (BG_COLOR);
  tft->setTextSize  (DEFTXTSIZE);
  #ifdef USESPRITE
    titleSprite   = new LGFX_Sprite(tft);
    titleSprite->   setColorDepth(16);
    if (!titleSprite->createSprite(TITLE_W, TITLE_H)) { Serial.println("TITLE SPRITE MISLUKT"); }
  #endif
  dirtyLogo     =   true;
  dirtyStation  =   true;
  dirtyTitle    =   true;
  dirtyVolume   =   true;
  dirtyBuffer   =   true;
  dirtyVU       =   true;
  dirtyPlay     =   false;//true;
}
//digifont
void RadioDisplay::digiLineH(int x0, int x1, int y, int c){
  if (!digiInstance || !digiInstance->tft){return;}
  digiInstance->tft->drawFastHLine(x0,y,x1-x0+1,c);
}
void RadioDisplay::digiLineV(int x, int y0, int y1, int c){
  if (!digiInstance || !digiInstance->tft){return;}
  digiInstance->tft->drawFastVLine(x,y0,y1-y0+1,c);
}
void RadioDisplay::digiRect(int x, int y, int w, int h, int c){
  if (!digiInstance || !digiInstance->tft){return;}
  digiInstance->tft->fillRect(x,y,w,h,c);
}
// ------------------------------------------------------------
// audio koppeling
// ------------------------------------------------------------
void RadioDisplay::attachAudio(Audio* a) {
  audio = a;
}
// ------------------------------------------------------------
// hoofdloop
// ------------------------------------------------------------
void RadioDisplay::loop() {
  if (millis() - lastUpdate < 50) {return;}
  lastUpdate            = millis();

  updateAudio             ();
  updatePlayTime          ();

  if (dirtyPlayTime)      {
    drawPlayTime          ();
    dirtyPlayTime       = false;
  }

  drawTime();
  drawDate();
  
  #ifndef USESPRITE
    lastTitleOffset     = titleOffset;
  #endif
  #define TYPEWRITE
  uint32_t now          = millis();
  #ifdef  TYPEWRITE     //typing text
    if (now - lastTitleScroll >= 180){
      lastTitleScroll   = now;
      dirtyTitle        = true;
    }
  #else                   //scrolling text
    if (titleScrolling){  // Titel ticker
    if (now - lastTitleScroll >= 33){
      lastTitleScroll   = now;
      const char* text  = getDisplayTitle();
      #ifdef USESPRITE
        int textWidth   = titleSprite->textWidth(text);
      #else
        int textWidth   = tft->textWidth(text);
      #endif
      const int gap     = 40;
      titleOffset      += 2;
      if (titleOffset  >= textWidth + gap){titleOffset = 0;}
      dirtyTitle        = true;
    }
  }
  #endif
  
  if (dirtyStation)       {
    drawStation();        //under and before logo
    dirtyStation        = false;
  } 
  
  if (dirtyLogo)          {
    drawLogo();
    dirtyLogo           = false;
  }

  if (dirtyTitle) {
    char extendedTitle[130];
    const char* src =       (!title[0]||strcmp(title, "commercial-in")==0||strcmp(title,"Qmusic Nieuws")==0) ? alttitle : title;
    snprintf                (extendedTitle,sizeof(extendedTitle),"%s ",src);
    if (newtitle) {
      titleSrcIndex = 0;
      titleXPos = TITLE_X;
    }
#ifdef TYPELETTER
    drawTitleTypewriter   (extendedTitle);// use a font
#else
    drawDigiTitle         (extendedTitle);// use digifont
#endif
    dirtyTitle = false;
  }
  
  if (dirtyBuffer)        {
    drawBuffer();
    dirtyBuffer         = false;
  }

  if (dirtyVU&& millis()- lastVUUpdate >= 150) {
  //drawVU();
    drawVU2();
    dirtyVU             = false;
  }

  if (dirtyPlay)          {
    drawPlay();
    dirtyPlay           = false;
  }

  if (dirtyAudioInfo)     {
    drawAudioInfo();
    dirtyAudioInfo      = false;
    dirtyVolume         = true;

  }
  if (dirtyVolume)        {
    drawVolume();
    dirtyVolume         = false;
  }
}

// ------------------------------------------------------------
// setters
// ------------------------------------------------------------
void RadioDisplay::       setLogo(const char* filename) {
  strncpy(logo,filename,sizeof(logo)-1);
  logo[sizeof(logo)-1]  = 0;
  dirtyLogo             = true;
}
void RadioDisplay::       setPlaying(bool p) {
  if (playing != p) {
    playing             = p;
    dirtyPlay           = true;
  }
}
void RadioDisplay::       setStation(const char* txt) {
  strncpy(station,txt,sizeof(station)-1);
  station[sizeof(station) - 1] = 0;
  //dirtyStation        = true;
}
void RadioDisplay::       setTitle(const char* txt){
  strncpy                 (title, txt, sizeof(title) - 1);
  title[sizeof(title)-1]= 0;
  const char* text      = getDisplayTitle();// Bepaal welke tekst daadwerkelijk getoond wordt
  #ifdef USEFONT
    tft->setFont          (FONT_SMALL); tft->setTextSize(2);
  #endif
  int textWidth         = tft->textWidth(text);
  titleScrolling        = (textWidth > TITLE_W);
  titleOffset           = 0; // Nieuwe tekst altijd vanaf het begin
  lastTitleScroll       = millis();
  dirtyTitle            = true;
  newtitle              = true;
}
void RadioDisplay:: drawDigiLetter2(char ch, int x, int y){
  uint8_t             s = seg7[(uint8_t)ch];
  if (!s) return;         // spatie of onbekend
  // --- verticale correctie wanneer segment 3 (onderste balk) uit staat ---
  if ((s & 0b0001000) == 0) {
  //y += digi.segThick;                 // schuif het hele karakter iets omlaag
    digi.setSize7       (18, 34, 4, 2); // of maak het hoger (dan gaar het vanzelf omlaag)
  } else {digi.setSize7 (18, 32, 4, 2);}
  int offs            = digi.segSt + 1;
  digi.drawSeg7         (0, x+offs                    , y,                                   s & 0b0000001);
  digi.drawSeg7         (1, x+digi.digWd-digi.segThick, y+offs,                              s & 0b0000010);
  digi.drawSeg7         (2, x+digi.digWd-digi.segThick, y+offs+digi.segHt+1,                 s & 0b0000100);
  digi.drawSeg7         (3, x+offs                    , y+digi.segHt*2+offs-digi.segThick+digi.segSt+2, s & 0b0001000);
  digi.drawSeg7         (4, x                         , y+offs+digi.segHt+1,                 s & 0b0010000);
  digi.drawSeg7         (5, x                         , y+offs,                              s & 0b0100000);
  digi.drawSeg7         (6, x+offs                    , y+digi.segHt+offs,                   s & 0b1000000);
}
  // Segmenten DigiFont:
  //       0
  //     -----
  //   5       1
  //     --6--
  //   4       2
  //     -----
  //       3
void RadioDisplay:: drawDigiLetter(char ch, int x, int y){
  int s;
  switch (toupper(ch)){
  case 'A': s = 0b1110111; break;// 0,1,2,4,5,6 (right 2 left bits)
  case 'B': s = 0b1111100; break;
  case 'D': s = 0b1011110; break;
  case 'E': s = 0b1111001; break;
  case 'F': s = 0b1110001; break;
  case 'G': s = 0b1101111; break;
  case 'I': s = 0b0000110; break;
  case 'J': s = 0b0001110; break;
  case 'K': s = 0b1110110; break;
  case 'L': s = 0b0111000; break;
  case 'N': s = 0b1110110; break;
  case 'O': s = 0b0111111; break;
  case 'P': s = 0b1110011; break;
  case 'R': s = 0b1110011; break;
  case 'S': s = 0b1101101; break;
  case 'T': s = 0b1110000; break;
  case 'U': s = 0b0111110; break;
  case 'M': s = 0b1110110; break;
  case 'V': s = 0b0111110; break;
  case 'W': s = 0b0111110; break;
  case 'Y': s = 0b0110110; break;
  default:                 return;
  }
  int offs = digi.segSt + 1;
  digi.drawSeg7(0,x+offs                    ,y                  ,s & 0b0000001);
  digi.drawSeg7(1,x+digi.digWd-digi.segThick,y+offs             ,s & 0b0000010);
  digi.drawSeg7(2,x+digi.digWd-digi.segThick,y+offs+digi.segHt+1,s & 0b0000100);
  digi.drawSeg7(3,x+offs                    ,y+digi.segHt+digi.segHt+offs-digi.segThick+digi.segSt+2,s & 0b0001000);
  digi.drawSeg7(4,x                         ,y+offs+digi.segHt+1,s & 0b0010000);
  digi.drawSeg7(5,x                         ,y+offs             ,s & 0b0100000);
  digi.drawSeg7(6,x+offs                    ,y+digi.segHt+offs  ,s & 0b1000000);
}
void RadioDisplay:: drawDigiMonth(const char* month,int x,int y){
  for (uint8_t i = 0; i < 3; i++){
    char c = toupper(month[i]);
    if  (c == 'M'){c = 'N';}   // M mag als N worden weergegeven
    drawDigiLetter(c,x+i*14,y);
  }
}
// ------------------------------------------------------------
// draw AudioInfo
// ------------------------------------------------------------
void RadioDisplay::     drawAudioInfo() {
  clearArea             (AUDIOINFO_X, AUDIOINFO_Y, AUDIOINFO_W, AUDIOINFO_H);
  tft->drawRect         (AUDIOINFO_X, AUDIOINFO_Y, AUDIOINFO_W, AUDIOINFO_H, COLOR_AUDIOINFO);
  //onderste helft inverse
  tft->setTextColor     (COLOR_AUDIOINFO, BG_COLOR);
  #ifdef USEFONT
    tft->setTextSize    (1);
    tft->setFont        (FONT_NORMAL);
  #else
    tft->setTextSize    (2);
  #endif
  // bitrate
  tft->setCursor        (AUDIOINFO_X + 4, AUDIOINFO_Y + 2);
  tft->printf           ("%lu", audioBitrate / 1000);
  tft->fillRect         (AUDIOINFO_X, AUDIOINFO_Y + (AUDIOINFO_H / 2), AUDIOINFO_W, AUDIOINFO_H / 2, COLOR_AUDIOINFO);
  // codec
  if (audio) {
    const char* codec = audio->getCodecname();
    if (strcmp          (codec, "unknown") != 0) {
      tft->setTextColor (BG_COLOR, COLOR_AUDIOINFO);
      tft->setCursor    (AUDIOINFO_X + 4, 1+AUDIOINFO_Y + AUDIOINFO_H / 2);
      tft->print        (codec);
      tft->setTextColor (COLOR_TEXT, BG_COLOR);
    }
  }
}
// ------------------------------------------------------------
// draw Logo
// ------------------------------------------------------------
void RadioDisplay::drawLogo() {
  clearArea             (LOGO_X,LOGO_Y,LOGO_W,LOGO_H);
  if (strlen(logo) == 0){dirtyStation = true;return;}
  File file = SD.open   (logo,FILE_READ);
  if (!file) {
    Serial.print        ("Logo ontbreekt: ");
    Serial.println      (logo);
    dirtyStation =      true;// of tft.print logo/station
    return;
  }
//Serial.print          ("RAW logo laden: ");
//Serial.println        (logo);
  uint16_t line[LOGO_W];
  int y               = LOGO_Y;
  while (file.available() && y < LOGO_Y + LOGO_H) {
    int len =
      file.read(
        (uint8_t*)line,
        sizeof(line));
    if (len != sizeof(line))
      break;
    tft->pushImage(LOGO_X,y,LOGO_W,1,line);
    y++;
  }
  file.close();
}
// ------------------------------------------------------------
// draw Title Typewriter
// ------------------------------------------------------------

void RadioDisplay::       drawDigiTitle(const char* fullText){
  digi.setColors          (COLOR_ACCENT, BG_COLOR);
  int fullLen  =          strlen(fullText);
  if (fullLen == 0){      Serial.println(">>>>>>>>> GEEN TEXT <<<<<<<<<<<<");return;} 
    uint32_t drawingtime= micros();
    int maxhgt          = TITLE_H;
    int charwdt         = digi.digWd;
    // 2. Haal huidige karakter
    char c = toupper      (fullText[titleSrcIndex]);
    if (c == '\0')        {// 3. Einde tekst? -> opnieuw
         titleSrcIndex  =  0;
         c = toupper      (fullText[titleSrcIndex]);
    }
    if (c=='%'){c='<';}   // build %

    // 4. wis karakter op huidige titleXPos en iets meer
    tft->fillRect         (titleXPos, TITLE_Y, charwdt*2, maxhgt, BG_COLOR);

    // 5. Print karakter op huidige titleXPos
  //if                    (c=='i'||c=='I'||c=='1') {titleXPos-=charwdt/2};
    drawDigiLetter2       (c,titleXPos,TITLE_Y);
    // m=nn
    if                    (c=='m'||c=='M'||c=='w'||c=='W'){titleXPos+=charwdt;drawDigiLetter2(c,titleXPos,TITLE_Y);}
    //% ˚o
    if                    (c=='<')                        {titleXPos+=charwdt;drawDigiLetter2('O',titleXPos,TITLE_Y);}
    //$ ˚C
    if                    (c=='$')                        {titleXPos+=charwdt;drawDigiLetter2('^',titleXPos,TITLE_Y);}

    // 6. Volgende karakter
    titleSrcIndex++;

    // 7. X‑positie opschuiven
    titleXPos += charwdt;

    // 8. Als buiten titelvak → terug naar begin en wis naar rechts
    // if (titleXPos>300){titleXPos=8;}
  //if                   (titleXPos + charwdt*2 >= TITLE_X + TITLE_W) {
    if                   (titleXPos + charwdt   >= TITLE_X + TITLE_W) {
        tft->fillRect    (titleXPos, TITLE_Y, 320-titleXPos, TITLE_H, BG_COLOR);
        titleXPos          = TITLE_X;
    }
  
//Serial.print        ("index="+String(titleSrcIndex)+" xpos="+String(titleXPos)+" char=");
//Serial.println      (c);
  if (newtitle) {
    Serial.print(" TYPE-TITLE DRAWINGTIME:");
    Serial.print(micros() - drawingtime);
    Serial.print("us. FREE:");
    Serial.print(ESP.getFreeHeap());
    Serial.print(" MAX:");
    Serial.println(ESP.getMaxAllocHeap());
    newtitle = false;
  }
}

void RadioDisplay::       drawTitleTypewriter(const char* fullText){
  if (!fullText || !fullText[0]){return;}
  uint32_t drawingtime  = micros();
  int fullLen = strlen    (fullText);
  // Veiligheidscontrole
  if (titleSrcIndex >= fullLen) {titleSrcIndex = 0;titleXPos = TITLE_X;}
  #ifdef USEFONT
    tft->setFont          (FONT_SMALL);
  #endif  
  tft->setTextSize        (2);
  tft->setTextColor       (COLOR_TEXT);
  tft->setTextWrap        (false);
  int maxwdt            = tft->textWidth("W");
  int maxhgt            = tft->fontHeight();
  char              c   = fullText[titleSrcIndex];
  char buf[2];buf[0]=c;buf[1]='\0';
  int charwdt           = tft->textWidth(buf);
  // karakter wissen
  tft->fillRect           (titleXPos,TITLE_Y,maxwdt,maxhgt,BG_COLOR);

  // karakter tekenen
  tft->setCursor          (titleXPos, TITLE_Y);
  tft->print              (buf);

  // positie opschuiven
  titleXPos            += charwdt;

  // volgende positie
  titleSrcIndex++;

  // einde tekst
  if (titleSrcIndex    >= fullLen) {
    titleSrcIndex       = 0;
    titleXPos           = TITLE_X;
  }

  // einde titelgebied
  if (titleXPos + charwdt >= TITLE_X + TITLE_W) {
   int clearWidth = TITLE_X + TITLE_W - titleXPos;
   if (clearWidth > 0) {tft->fillRect(titleXPos,TITLE_Y,clearWidth,TITLE_H,BG_COLOR);
  }
  titleXPos = TITLE_X;
  }

  if (newtitle) {
    Serial.print(" TYPE-TITLE DRAWINGTIME:");
    Serial.print(micros() - drawingtime);
    Serial.print("us. FREE:");
    Serial.print(ESP.getFreeHeap());
    Serial.print(" MAX:");
    Serial.println(ESP.getMaxAllocHeap());
    newtitle = false;
  }
}

// ------------------------------------------------------------
// draw Title
// ------------------------------------------------------------
void RadioDisplay::drawTitle(const char* text)
{
  uint32_t drawingtime = micros();
#ifdef USESPRITE
  if (!titleSprite)         {return;}
  titleSprite->fillSprite   (BG_COLOR);
  titleSprite->setTextColor (COLOR_TEXT);
  #ifdef USEFONT
    titleSprite->setFont    (FONT_SMALL);//titleSprite->setFont(FONT_MEDIUM);
  #endif
  titleSprite->setTextSize  (2)         ;//titleSprite->setTextSize  (DEFTXTSIZE);
  titleSprite->setTextWrap  (false);
  int textWidth   =         titleSprite->textWidth(text);
  bool scrolling  =         (textWidth > TITLE_W);
  if (!scrolling)
  {
    int x         =         (TITLE_W - textWidth) / 2;
    if (x < 0)              {x = 0;}
    titleSprite->setCursor  (x, 0);
    titleSprite->print      (text);
  }
  else
  {
    const int gap =         40;
    int x         =         -titleOffset;
    titleSprite->setCursor  (x, 0);
    titleSprite->print      (text);
    titleSprite->setCursor  (x+textWidth+gap,0);
    titleSprite->print      (text);
  }
    titleSprite->pushSprite   (TITLE_X, TITLE_Y);
#else //NO SPRITES
//#define FILLALL
//if (!titleSprite)           {return;}
  #ifdef USEFONT
//tft->setFont                (FONT_MEDIUM);tft->setTextSize(DEFTXTSIZE);
  tft->setFont                (FONT_SMALL); tft->setTextSize(2);
  #endif
  tft->setTextWrap            (false);
  int textWidth             = tft->textWidth(text);
  bool scrolling            = (textWidth > TITLE_W);
  #ifdef FILLALL
  //tft->fillRect             (TITLE_X, TITLE_Y, TITLE_W, TITLE_H, BG_COLOR);
    tft->fillRect             (TITLE_X-8, TITLE_Y, TITLE_W+16, TITLE_H, BG_COLOR);
  #else
  // oude tekst wissen ALTERNATIEF
    tft->setTextColor         (BG_COLOR);
    if (!scrolling)           {
      int oldX = TITLE_X +    ((TITLE_W - textWidth) / 2);
      if (oldX < 0) oldX    = 0;
      tft->setCursor          (oldX, TITLE_Y);
      tft->print              (text);
    } else                    {
      const int gap         = 40;
      int oldX              = TITLE_X - lastTitleOffset;
      // oude eerste tekst wissen
      tft->setCursor          (oldX, TITLE_Y);
      tft->print              (text);
      // oude tweede tekst wissen
      tft->setCursor          (oldX + textWidth + gap, TITLE_Y);
      tft->print              (text);
    }

  #endif
  // oude tekst gewist nu nieuwe tekenen
  tft->setTextColor         (COLOR_TEXT);
  if (!scrolling)           {
    int                 x = (TITLE_W - textWidth) / 2;
    if (x < 0)          x = 0;
    tft->setCursor          (TITLE_X + x, TITLE_Y);
    tft->print              (text);
  } else                    {
    const int         gap = 40;
    int                 x = TITLE_X - titleOffset;
    // eerste tekst
    tft->setCursor          (x, TITLE_Y);
    tft->print              (text);
    // tweede tekst (achter de eerste)
    tft->setCursor          (x + textWidth + gap, TITLE_Y);
    tft->print              (text);
  }
#endif

  if (titleOffset<4){
    Serial.print              ("USING ");
    #ifdef USESPRITE 
    Serial.print              ("SPRITES");
    #else
    Serial.print              ("NO SPRITES");
    #ifdef FILLALL
    Serial.print              ("FILLALL ");
    #else
    Serial.print              ("REDRAW ");
    #endif
    Serial.print              ("AND ");
    #endif
    Serial.print              (" -> TITLE DRAWINGTIME:"+String(micros()-drawingtime)+"us.");
    Serial.println            (" FREE:"+String(ESP.getFreeHeap())+" MAX:"+String(ESP.getMaxAllocHeap()));
  }

}
    
// ------------------------------------------------------------
// draw Time
// ------------------------------------------------------------
void RadioDisplay::     drawTime() {
  struct tm timeinfo;
  if (!getLocalTime     (&timeinfo)){return;}
  char now[6];
  snprintf(now,sizeof(now),"%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min);
  if (oldClock == now)  {return;}
  oldClock = now;
  tft->fillRect         (CLOCK_X,CLOCK_Y,CLOCK_W,CLOCK_H,BG_COLOR);
  digi.setSize7         (18, 28, 4, 2);
  digi.setColors        (COLOR_TEXT,BG_COLOR);
  digi.printNumber7     (now,CLOCK_X,CLOCK_Y);
}

void RadioDisplay::     drawDate(){
  struct tm timeinfo;
  if (!getLocalTime     (&timeinfo)){return;}
  char date[8];
  snprintf              (date,sizeof(date),"%02d-%02d",timeinfo.tm_mday,timeinfo.tm_mon+1);
  if (strcmp            (oldDate, date) == 0){return;}
  /*
  style (optional) – Appearance style of the segments:
  0 → Default style (same as setSize1)
  1, 2, 3 → Alternative segment shapes
  4 or "F" → Filled style
  5 or "O" → Outline style
  */
  //setSize7            (width, height, thickness, style)
  digi.setColors        (COLOR_DATE, BG_COLOR);
  strcpy                (oldDate, date);
  clearArea             (DATE_X,DATE_Y,DATE_W,DATE_H);
  // -------------------------
  // dag
  // -------------------------
  int OFSNR1=5,VH=25;   //offset for number 1
  int VO=0,HO=0;        //offset Vert/Hor
  digi.setSize7         (16, VH-2, 4, 2);
  int day       =       timeinfo.tm_mday;
  // als eerste digit is 1 → iets naar links
  if (day >= 10 && day/10==1){HO=OFSNR1;}
  digi.drawDigit7       (day/10   ,DATE_X-HO,DATE_Y);
  // als tweede digit is 1 → iets naar links
  if (day % 10 == 1)    {HO=OFSNR1;}else{HO=0;}
  digi.drawDigit7       (day % 10 ,DATE_X-HO,DATE_Y+VH);
  VO  +=  4;
  // -------------------------
  // maand
  // -------------------------
  static const char*    months[] = {"JAN","FEB","MRT","APR","MEI","JUN","JUL","AUG","SEP","OKT","NOV","DEC"};
  const char* month =   months[timeinfo.tm_mon];
  for (uint8_t i = 0; i < 3; i++){
    char c        =     month[i];
    if  (c == 'M'){c = 'N';}
    switch (toupper(c)) {
     // Geen bovenste segment → letter optisch 3 px omhoog
     case 'L':
      VO  -= 3;
      break;
    }
    drawDigiLetter      (c,DATE_X,DATE_Y+(VH*2)+(i*VH)+VO);
    switch (toupper(c)) {
    // Geen onderste segment → volgende letter optisch 3 px omhoog
      case 'A':
      case 'F':
      case 'N':
      case 'P':
      case 'R':
      VO  -= 3;
    break;
    }
  }
  digi.setSize7         (18, 28, 4, 2);//zet oude waarden terug
}
void RadioDisplay::     drawPlayTime(){
  clearArea             (PT_X,PT_Y,PT_W,PT_H);
  digi.setColors        (COLOR_ACCENT,BG_COLOR);
  digi.setSize7         (18, 28, 4, 2);
  digi.printNumber7     (playTime,PT_X,PT_Y);
}
// ------------------------------------------------------------
// draw Station naam
// ------------------------------------------------------------
void RadioDisplay::     drawStation() {
  clearArea             (STATION_X,STATION_Y,STATION_W,STATION_H);
  tft->setTextColor     (COLOR_ACCENT);
  #ifdef USEFONT
    tft->setFont        (FONT_SMALL); tft->setTextSize(2);
  #endif
  int textWidth   =     tft->textWidth(station);
  int x = STATION_X;
  if (x < STATION_X)    {x=STATION_X;}
  tft->setCursor        (x,STATION_Y);
  tft->print            (station);
  dirtyAudioInfo =      true;  //overlap
}
/*
// ------------------------------------------------------------
// draw Volume balk
// ------------------------------------------------------------
void RadioDisplay::drawVolume() {
  clearArea(
    VOLUME_X,
    VOLUME_Y,
    VOLUME_W,
    VOLUME_H);
  int w = map(
    lastVolume,
    0,
    21,
    0,
    VOLUME_W);
  tft->fillRect(
    VOLUME_X,
    VOLUME_Y,
    w,
    VOLUME_H,
    COLOR_VOLUME);
  tft->drawRect(
    VOLUME_X,
    VOLUME_Y,
    VOLUME_W,
    VOLUME_H,
    TFT_WHITE);
}
*/
// ------------------------------------------------------------
// draw Volume getal
// ------------------------------------------------------------
void RadioDisplay::   drawVolume()
{
  clearArea           (VOLUME_X,VOLUME_Y,VOLUME_W,VOLUME_H);
  char vol[4];
  int offset=0;
  if (lastVolume>9)   {offset=-10;} 
  snprintf            (vol,sizeof(vol),"%d",lastVolume);
  digi.setSize7       (18, 28, 4, 2);
  digi.setColors      (COLOR_ACCENT,BG_COLOR);
  digi.printNumber7   (vol,VOLUME_X+offset,VOLUME_Y);
}
// ------------------------------------------------------------
// draw Buffer balk
// ------------------------------------------------------------
void RadioDisplay::   drawBuffer() {
  clearArea           (BUFFER_X,BUFFER_Y,BUFFER_W,BUFFER_H);
  int w =             (BUFFER_W * buffer) / 100;
  tft->fillRect       (BUFFER_X,BUFFER_Y,w,BUFFER_H,COLOR_BUFFER);
  tft->drawRect       (BUFFER_X,BUFFER_Y,BUFFER_W,BUFFER_H,TFT_WHITE);
}
// ------------------------------------------------------------
// draw VU meter
// ------------------------------------------------------------
void RadioDisplay::   drawVU() {
  clearArea           (VU_X,VU_Y,BUFFER_X-VU_X,VU_H);
  int wLeft  = map    (vuLeft,0,127,0,VU_W);
  int wRight = map    (vuRight,0,127,0,VU_W);
  tft->fillRect       (VU_X,VU_Y,wLeft,(VU_H/2)-2,COLOR_VU);            // Links
  tft->fillRect       (VU_X,1+VU_Y+VU_H/2,wRight,(VU_H/2)-2,COLOR_VU);  // Rechts
  }
// ------------------------------------------------------------
// draw VU meter
// ------------------------------------------------------------
void RadioDisplay::   drawVU2(){
  clearArea           (VU2_X,VU2_Y,VU2_W,VU2_H);
  const int blocks =  12;
  const int gap    =  1;
  const int blockH =  (VU2_H - (blocks - 1) * gap) / blocks;
  const int channelW= 16;//VU2_W/2 - 4
  const int leftX   = VU2_X + 1;
  const int rightX  = leftX + channelW + 5;
  int levelLeft     = map(vuLeft, 0, 127, 0, blocks);
  int levelRight    = map(vuRight, 0, 127, 0, blocks);
  // --------------------------------------------------------
  // Blokjes
  // --------------------------------------------------------
  for (int i = 0; i < blocks; i++){
    int y    =        VU2_Y+VU2_H-blockH-i*(blockH+gap);
    uint16_t color;   // kleur: groen -> geel -> rood
    if (i < 6)
      color =         TFT_GREEN;
    else if (i < 8)
      color =         TFT_YELLOW;
    else
      color =         TFT_RED;
    if (i < levelLeft){tft->fillRect(leftX+1 ,y,channelW,blockH,color);}  // links
    if (i <levelRight){tft->fillRect(rightX-1,y,channelW,blockH,color);}  // rechts
  }
  lastVUUpdate =      millis();
}
// ------------------------------------------------------------
// Play / Pause icoon
// ------------------------------------------------------------
void RadioDisplay::   drawPlay() {
  clearArea           (STATUS_X,STATUS_Y,30,STATUS_H);
  int x   =           STATUS_X + 4;
  int y   =           STATUS_Y + 3;
//int h=14,b=7;
  int h=16,b=8;
  if (playing)        {
    // driehoek PLAY
    tft->fillTriangle (x,y,x,y+h,x+h,y+b,COLOR_ACCENT);
  } else {
    // twee balkjes PAUSE
    tft->fillRect     (x  ,y,5,h,COLOR_ACCENT);
    tft->fillRect     (x+9,y,5,h,COLOR_ACCENT);
  }
}
// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------
void RadioDisplay::   resetPlayTime()
{
    titleStartMillis= millis();
    dirtyPlayTime   = true;
}
void RadioDisplay::   clearArea(int x,int y,int w,int h) {
  tft->fillRect       (x,y,w,h,BG_COLOR);
}
void RadioDisplay::   updatePlayTime(){
  if (millis() -      lastPlayTimeDraw < 1000){return;}
  lastPlayTimeDraw  = millis();
  uint32_t sec      = (millis() - titleStartMillis) / 1000;
  uint16_t min      = sec / 60;
  uint8_t  s        = sec % 60;
  snprintf            (playTime,sizeof(playTime),"%02u:%02u",min,s);
  dirtyPlayTime     = true;
}
// ------------------------------------------------------------
// Audio info uitlezen
// ------------------------------------------------------------
void RadioDisplay::   updateAudio() {
  if (!audio)         {return;}
      static uint32_t bufTimer = 0;//DEBUG
      uint32_t t = micros();
  uint8_t vol       = audio->getVolume();  // get volume
  if (vol          != lastVolume) {
    lastVolume      = vol;
    dirtyVolume     = true;
  }
  uint32_t bitrate  = audio->getBitRate();  // get stream info
  int codec         = audio->getCodec();
  if (bitrate      != audioBitrate || codec != audioCodec) {
    audioBitrate    = bitrate;
    audioCodec      = codec;
    dirtyAudioInfo  = true;
  }
  #define buf         // get buffer
  #ifdef buf

uint32_t t0 = micros();

    uint32_t size   = audio->inBufferSize();

uint32_t t1 = micros() - t0;

    uint32_t filled = audio->inBufferFilled();

uint32_t t2 = micros() - t0;
//if (t1 > 500 || t2 > 500) {
if (t1 + t2 > 50) {
  Serial.printf(
    "BUFFER CALLS: size=%lu us filled=%lu us\n",
    t1, t2
  );
}

    if (size > 0) {
      uint8_t newBuffer = (filled * 100UL) / size;
      if (newBuffer > 100) { newBuffer = 100; }
      if (newBuffer != buffer) {
        buffer       = newBuffer;
        dirtyBuffer  = true;
      //Serial.printf  ("BUF %lu/%lu  HEAP %u\n", filled, size, ESP.getFreeHeap());
        if (filled<10000){
      //Serial.printf  ("BUF %lu/%lu  HEAP %u internalLargest=%u\n", filled, size, ESP.getFreeHeap(),heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
      // Serial.printf  ("BUF %lu/%lu  HEAP %u CALLS: size=%lu us filled=%lu us\n", filled, size, t1, t2);
        Serial.println ("BUF "+String(filled)+"/"+String(size)+" inBufferSize:"+String(t1)+" inBufferFilled:"+String(t2)+"us.");
        }
/*  Serial.printf  ("BUF free=%u max=%u largest=%u internal=%u internalLargest=%u\n",
    txt,
    ESP.getFreeHeap(),
    ESP.getMaxAllocHeap(),
    heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
    heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
    heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)
  );
*/
      }
    }
  #endif
  uint16_t vu        = audio->getVUlevel(); // VU
  uint8_t newLeft    = vu >> 8;
  uint8_t newRight   = vu & 0xFF;
  if (newLeft!=vuLeft||newRight!=vuRight) {
    vuLeft           = newLeft;
    vuRight          = newRight;
    dirtyVU          = true;
  }
  uint32_t dt = micros() - t;
  if (dt > 50) {Serial.printf(">>>> updateAudio() = %lu us\n", dt);}
}
// weer HTTP
void RadioDisplay::   updateWeather()
{
  Serial.println("updateWeather()<<<<<<<<<<<<<<<<<<<<<");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WEER: geen WiFi");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  const char* url =
    "http://api.open-meteo.com/v1/forecast"
    "?latitude=52.22"
    "&longitude=6.00"
    "&current=temperature_2m,weather_code"
    "&daily=temperature_2m_max,precipitation_probability_max"
    "&timezone=Europe%2FAmsterdam";

  Serial.printf("WEER URL: %s\n", url);

  if (!http.begin(client, url)) {
    Serial.println("WEER: http.begin mislukt");
    return;
  }

  http.setTimeout(10000);
  int httpCode = http.GET();
  Serial.printf("WEER HTTP: %d\n", httpCode);

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("WEER fout: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return;
  }

  // ---------------------------
  // JSON ophalen via payload
  // ---------------------------
  String payload = http.getString();
  http.end();   // veilig om nu te sluiten
//client.end();

  Serial.println("---- JSON PAYLOAD ----");
  Serial.println(payload);
  Serial.println("-----------------------");

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("JSON parse fout: ");
    Serial.println(error.c_str());
    return;
  }

  // ---------------------------
  // JSON debug
  // ---------------------------
  if (!doc["current"]["temperature_2m"].is<float>()) {
    Serial.println("GEEN current.temperature_2m!");
  }
  if (!doc["current"]["weather_code"].is<int>()) {
    Serial.println("GEEN current.weather_code!");
  }
  if (!doc["daily"]["temperature_2m_max"][0].is<float>()) {
    Serial.println("GEEN daily.temperature_2m_max[0]!");
  }
  if (!doc["daily"]["precipitation_probability_max"][0].is<int>()) {
    Serial.println("GEEN daily.precipitation_probability_max[0]!");
  }

  // ---------------------------
  // Waarden uitlezen
  // ---------------------------
  float temp =
    doc["current"]["temperature_2m"] | 0.0;

  int weatherCode =
    doc["current"]["weather_code"] | 0;

  float maxTemp =
    doc["daily"]["temperature_2m_max"][0] | 0.0;

  int rainChance =
    doc["daily"]["precipitation_probability_max"][0] | 0;

  const char* condition = "onbekend";

  switch (weatherCode)
  {
    case 0:  condition = "is zonnig"; break;
    case 1:
    case 2:  condition = "is licht bewolkt"; break;
    case 3:  condition = "is bewolkt"; break;
    case 45:
    case 48: condition = "is mistig"; break;
    case 51:
    case 53:
    case 55: condition = "motregent"; break;
    case 61:
    case 63:
    case 65: condition = "regent"; break;
    case 71:
    case 73:
    case 75: condition = "sneeuwt"; break;
    case 80:
    case 81:
    case 82: condition = "is buiig"; break;
    case 95:
    case 96:
    case 99: condition = "onweert"; break;
  }

  snprintf(
    alttitle,
    sizeof(alttitle),
    "Het %s in Apeldoorn, %.0f$, "//$=˚C
    "kans op regen %d%%, maximaal %.0f$",
    condition,
    temp,
    rainChance,
    maxTemp
  );

  Serial.printf("ALTTITLE: %s\n", alttitle);

  dirtyTitle = true;
}

