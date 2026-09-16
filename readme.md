YOIDE Yoradio clone for Arduino IDE and LovyanGFX for CYD clone 2.8inch ESP32 2432S028 ST7789 or 2.4 inch DIYTZT ESP32024 ILI9342

Uses Arduino IDE v2.x and specific CYD boards. Test your board before trying YOIDEGIT scripts!
(put #define USE24 for the 2.4 inch board or //#define USE24 for the 2.8 inch in tftkeuze.h) 
The main challenche for this project was memory because the boards do not have PSRAM. 
Scrolling text together with wifi streaming is almost impossible. For that reason I use a sort of typewriter wipe.
For fun I used digifont but that is quite difficult to read, so you can also use a normal font. 
All main actions are in display.h and display.cpp. Most display options in display_layout.h 

```text
test your board first: https://github.com/gtmans/CYD/blob/main/DIYTZT_ESP32-024_ILI9342/CYD_TEST_ESP32-024.ino or https://github.com/gtmans/CYD/blob/main/AOKIN_ESP32-2432S028/CYD_TEST_ESP32-2432S028.ino
```
Compile with Arduino IDE choose board ESP32 DEV module if Too Big: use tools/partition scheme: 3mb huge APP
install ESP32 3.3.11 (not V4!) and LovyanGFX 1.2.26 
install ESP32-audioI2S-master max. 3.2.1 when no PSRAM!
non default options
#define USESPRITE  (in display.h  and display.cpp) uses a lot of memory and probable crashes or no sound
#define USEFONT    (in display.cpp) uses some memory to display nice font (display_layout.h) when not using digifont
#define TYPEWRITE  (in display.cpp) this is default use letter for letter i/o scrolling tekst (scrolling text can be less pretty)
#define TYPELETTER (in display.cpp) uses normal characters in titles i/o digifont

```text
hardware configurations:
ESP32 + ILI9342 + XPT2046 + MAX98357A or
ESP32 + ILI9342 + XPT2046 + PCM5102MK 2.0 + GF1002/GS27076 + XLY-D43 3 W / 4 Ω x2

SD:
stations.txt            -> optional SD for stations.txt and station logo's: Station name;URL;logoname.raw
/logos
/logos/*.raw            -> optional station logo's
/logosjpg/mkraws.py     -> script 2 convert all *.jp to *.raw on PC: win+R cmd,cd a:\logosjpg, a: , python mkraws.py (install python from microsoft store free app)

connections:
5V       jst4 linkerzijde boven USBC bovenste pin of van soldeerpad bord 
GND      jst4 linkerzijde boven USBC onderste pin of van soldeerpad bord 
GPIO22   jst4 onderzijde rechts  
GPIO21   jst4 onderzijde rechts  
GPIO26   solderen aan pin 11 (4e van de hoek)

                ┌──────────────┐
GPIO22 ─BCLK───►│              │
GPIO21 ─LCK────►│  MAX98357A   │
GPIO26 ─DIN────►│              │
                │    L ────────┼────► L ──► speaker
                │    R ────────┼────► R ──► speaker
                │   GND ───────┼────► GND
                └──────────────┘

zet bij de GF1002 een flinke buffercondensator tussen + en - bv. 470 µF (als die niet al op het bord zit)
plus bijvoorbeeld 100 nF keramisch dicht bij de voedingsaansluiting van de versterker.
                ┌──────────────┐
GPIO22 ─BCLK───►│              │
GPIO21 ─LCK────►│   PCM5102    │
GPIO26 ─DIN────►│              │
                │    L ────────┼────► GF1002/GS27076 L ──► speaker
                │    R ────────┼────► GF1002/GS27076 R ──► speaker
                │   GND ───────┼────► GND
                └──────────────┘

pinout      ESP-WROOM-32
             ANTENNE
01 GND                            38 GND
02 3V3                            37 GPIO23
03                                36 GPIO22 JST SCL
04                                35 
05                                34
06                                33 GPIO21 JST SDA
07 GPIO35 JST                     32
08                                31
09                                30
10 GPIO25                         29
11 GPIO26 <<<<SOLDEREN            28
12                                27
13                                26
14 GPIO12                         25 GPIO0
   15 16 17 18 19 20 21 22 23 24

CYD 2.4" ESP32-024
jst4 linkerzijde boven USBC
5V
TX
RX
GND

jst2 bovenzijde links
BAT-
BAT+

jst2 bovenzijde 2
SPK-
SPK+

jst4 onderzijde rechts
GND
GPIO22
GPIO21
3V3

jst4 onderzijde 2
GND
GPIO35
GPIO22
GPIO21
```
