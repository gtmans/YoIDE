/*
/ Created 2026 by github.com/gtmans/ free for use and development
/ Yoradio clone for Arduino IDE and LovyanGFX for CYD clone 
/ 2.8inch ESP32 2432S028 ST7789 or 2.4 inch DIYTZT ESP32024 ILI9342
/ uses optional SD for stations.txt and station logo's
*/
// YoRadioIDE V3.3 LovyanGFX CYD github/gtmans
// for ESP32 DIYTZT_ESP32_024_ILI9342.h or ESP32 AOKIN_2432S028_ST7789.h
// add MAX98357A see pinlayout.h for connections
// Compile with Arduino IDE choose board ESP32 DEV module if Too Big: use tools/partition scheme: 3mb huge APP
// install ESP32 3.3.11 (not V4!) and LovyanGFX 1.2.26 
// install ESP32-audioI2S-master max. 3.2.1 when no PSRAM!
// non default options
// #define USESPRITE  (in display.h  and display.cpp) uses a lot of memory and probable crashes or no sound
// #define USEFONT    (in display.cpp) uses some memory to display nice font (display_layout.h) when not using digifont
// #define TYPEWRITE  (in display.cpp) this is default use letter for letter i/o scrolling tekst (scrolling text can be less pretty)
// #define TYPELETTER (in display.cpp) uses normal characters in titles i/o digifont

#include        <Arduino.h>
//#include      "esp_heap_caps.h"
#include        "tftkeuze.h"
#ifdef          USE24                   // 2.4 inch defined in tftkeuze.h
  #include      "DIYTZT_ESP32_024_ILI9342.h"
#else                                   // 2.8 inch defined in tftkeuze.h
  #include      "AOKIN_ESP32_2432S028_ST7789.h"
#endif
#define         SDSTATIONS              // read stations from stations.txt -> Station name;URL;logoname.raw
                                        // audio hardware config see pinlayout.h
#define         AUDIO_BCLK 22
#define         AUDIO_LRC  21
#define         AUDIO_DOUT 26
#define         AUDIO_BUFFER_SIZE 6000  //11903
//
#include        "secrets.h"             // username and passconst char* WIFI_SSID   = "yourSSID"; char* WIFI_PASS   = "yourpasswd"; 
#include        "display.h"
#include        "display_layout.h"
#include        <WiFi.h>
#include        <Audio.h>               // use ESP32-audioI2S-master 3.2.1 when no PSRAM! libraries\ESP32-audioI2S-master\src\Audio.h
#include        <Preferences.h>
#include        <time.h>

int ERRCNT    = 0;
int volume    = 10;
int stationNr = 4, oldStatNr = 4;
Preferences     prefs;
bool mute     = false;
bool playing  = false;
bool EEPDirty = false;
uint32_t        lastPrefsSave = 0;
const uint32_t  PREFS_SAVE_INTERVAL = 5UL * 60UL * 1000UL;  //5 minuten
static uint32_t lastWeatherUpdate = 0;

Audio audio;
RadioDisplay    display;

#ifdef  SDSTATIONS  // read from SD
  struct Station {
    String naam;
    String url;
    String logo;
  };
  #define MAX_STATIONS 10
  Station stations[MAX_STATIONS];
  int NUM_STATIONS = 0;

  void loadStationsFromSD() {
    File f = SD.open("/stations.txt");
    if (!f) {
      Serial.println("stations.txt niet gevonden — fallback naar Radio 1");
      tft.println   ("no stations.txt fallback 2 Radio 1");
      delay   	    (2000);
      NUM_STATIONS = 1;
      stations[0].naam = "NPO Radio 1";
      stations[0].url  = "http://icecast.omroep.nl/radio1-bb-mp3";
      stations[0].logo = "/logos/npo1.raw";
      return;
    }
    NUM_STATIONS = 0;
    while (f.available() && NUM_STATIONS < MAX_STATIONS) {
      String line = f.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) continue;
      int p1 = line.indexOf(';');
      int p2 = line.indexOf(';', p1 + 1);
      if (p1 < 0 || p2 < 0) {
        Serial.println("Ongeldige regel: " + line);
        continue;
      }
      stations[NUM_STATIONS].naam = line.substring(0, p1);
      stations[NUM_STATIONS].url  = line.substring(p1 + 1, p2);
      stations[NUM_STATIONS].logo = line.substring(p2 + 1);
      NUM_STATIONS++;
    }
    f.close();
    Serial.printf("Stations geladen: %d\n", NUM_STATIONS);
  }
#else
  struct Station  {
    const char*   naam;
    const char*   url;
    const char*   logo;
  };
  Station stations[] = {//https://www.mp3streams.nl/zender/slam
    { "NPO Radio 1" ,"http://icecast.omroep.nl/radio1-bb-mp3", "/logos/npo1.raw" },
    { "NPO Radio 2" ,"http://icecast.omroep.nl/radio2-bb-mp3", "/logos/npo2.raw" },
    { "NPO Radio 3" ,"http://icecast.omroep.nl/3fm-bb-mp3"   , "/logos/npo3.raw" },
    { "NPO Radio 4" ,"http://icecast.omroep.nl/radio4-bb-mp3", "/logos/npo4.raw" },
    { "NPO Radio 5" ,"http://icecast.omroep.nl/radio5-bb-mp3", "/logos/npo5.raw" },
    { "Qmusic"      ,"http://icecast-qmusicnl-cdp.triple-it.nl/Qmusic_nl_live_96.mp3", "/logos/qmusic.raw" },
    { "538"         ,"http://playerservices.streamtheworld.com/api/livestream-redirect/RADIO538.mp3", "/logos/538.raw"      },
    { "538 NS"      ,"http://playerservices.streamtheworld.com/api/livestream-redirect/TLPSTR09.mp3", "/logos/538ns.raw"   },
    { "Veronica"    ,"http://playerservices.streamtheworld.com/api/livestream-redirect/VERONICA.mp3", "/logos/veronica.raw" },
    { "Sky Radio"   ,"http://playerservices.streamtheworld.com/api/livestream-redirect/SKYRADIO.mp3", "/logos/sky.raw"      },
    { "Radio 10"    ,"http://playerservices.streamtheworld.com/api/livestream-redirect/RADIO10.mp3",  "/logos/radio10.raw"     }
  };
  #define NUM_STATIONS \
    (sizeof(stations) / sizeof(stations[0]))
#endif

// ------------------------------------------------
// Preferences
// ------------------------------------------------
void loadSettings () {
  digitalWrite    (PIN_BLUE,0);
  prefs.begin     ("yoradio", false);
  stationNr   =   prefs.getInt("station", 0);
  volume      =   prefs.getInt("volume", 8);
  //prefs.end     ();
  if (stationNr >=NUM_STATIONS || stationNr < 0) { stationNr = 0; }
  oldStatNr   =   stationNr;
  delay           (1000);
  digitalWrite    (PIN_BLUE,1);
}
void saveSettings () {
  digitalWrite    (PIN_GREEN,0);
  prefs.putInt    ("station", stationNr);
  prefs.putInt    ("volume", volume);
  delay           (1000);
  digitalWrite    (PIN_GREEN,1);
}
// --------------------------------------------------
// prototypes
// --------------------------------------------------
bool setupSD()    {
  Serial.println  ("SD starten");
  sdSPI.begin     (
    SD_SCK,
    SD_MISO,
    SD_MOSI,
    SD_CS);
  if (!SD.begin   (
        SD_CS,
        sdSPI))   {
    Serial.println("Geen SD");
    return false;
  }
  Serial.println  ("SD OK");
  return true;
}
// ------------------------------------------------
// NTP
// ------------------------------------------------
void setupTime()  {configTzTime("CET-1CEST,M3.5.0,M10.5.0/3","pool.ntp.org","time.nist.gov");}
// ------------------------------------------------
// WiFi
// ------------------------------------------------
void printWiFiStatus(){
  Serial.printf(    "WIFI RSSI=%d dBm CH=%d IP=%s BSSID=%s\n",WiFi.RSSI(),WiFi.channel(),WiFi.localIP().toString().c_str(),WiFi.BSSIDstr().c_str());
}

void setupWiFi()    {
  tft.setCursor     (0, 0);
  tft.println       ("WiFi verbinden...");
  WiFi.begin        (WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay           (250);
    tft.print       (".");
  }
  Serial.println    ();
  Serial.print      ("IP: ");
  Serial.println    (WiFi.localIP());
  WiFi.setSleep     (false);
}

void audio_info     (const char* info) {
  Serial.print      ("audio_info: ");
  Serial.println    (info);
}
void audio_showstreamtitle(const char* info) {
  Serial.print      ("TITLE: ");
  Serial.println    (info);
  display.setTitle  (info);
}
void audio_showstation(const char* info) {
  Serial.print      ("STATION: ");
  Serial.println    (info);
  display.setStation(info);
  display.resetPlayTime();
}

void playStation    (int nr) {
  ERRCNT=0;
  if (nr < 0)       { nr = NUM_STATIONS - 1; }
  if (nr >= NUM_STATIONS) { nr = 0; }
  stationNr     =   nr;
  Serial.print      ("Start station: ");
  Serial.println    (stations[stationNr].naam);
  audio.stopSong    ();
  display.setPlaying(false);
  delay             (100);
  display.setTitle  ("");
  display.setLogo   (stations[stationNr].logo.c_str());
  display.setStation(stations[stationNr].naam.c_str());
  audio.connecttohost(stations[stationNr].url.c_str());
  display.setPlaying(true);
  if (oldStatNr != stationNr) {
    EEPDirty = true;
    lastPrefsSave = millis();
  }
  printHeap         ("NA STATION");
}
/*
  Bediening:
  links       vorige zender
  rechts      volgende zender
  midden      mute
  boven       volume +
  onder       volume -
  play/pause  play/pause icoon aanraken
*/
// ------------------------------------------------
// Touch bediening
// ------------------------------------------------
void checkTouch() {
  static uint32_t       lastTouch = 0;
  if (millis() - lastTouch < 300){return;}
  uint16_t x,y;
  if (tft.getTouch      (&x, &y)) {
    lastTouch         = millis();
    Serial.printf       ("Touch %d,%d\n",x,y);
    if (y >= STATUS_Y - 10 && y <= STATUS_Y + STATUS_H && x >= STATUS_X && x <= STATUS_X + 30) {
      Serial.println    ("PLAY/PAUSE");
      if (audio.isRunning())
        audio.stopSong  ();
      else
        playStation     (stationNr);
      return;
    }
    if (x < 80) {
      playStation       (stationNr - 1);
      EEPDirty      =   true;
      lastPrefsSave =   millis();
    } else if (x > 240) {
      playStation       (stationNr + 1);
      EEPDirty      =   true;
      lastPrefsSave =   millis();
    } else if (y < 70)  {
      volume++;
      if (volume > 21)  {volume = 21;}
      audio.setVolume   (volume);
      EEPDirty      =   true;
      lastPrefsSave =   millis();
    } else if (y > 170) {
      if (volume > 0)   {volume--;}
      audio.setVolume   (volume);
      EEPDirty      =   true;
      lastPrefsSave =   millis();
    } else              {
      mute          =   !mute;
      if (mute)         {audio.setVolume(0);} else {audio.setVolume(volume);}
    }
  }
}

void checkPrefsSave()   {
  if (!EEPDirty)        { return; }
  if (millis() - lastPrefsSave >= PREFS_SAVE_INTERVAL) {
    Serial.println      ("Preferences opslaan");
    saveSettings        ();
    EEPDirty          = false;
    lastPrefsSave     = millis();
  }
}

void printHeap(const char* txt)
{
  Serial.printf(
    "%s free=%u max=%u largest=%u internal=%u internalLargest=%u\n",
    txt,
    ESP.getFreeHeap(),
    ESP.getMaxAllocHeap(),
    heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
    heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
    heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)
  );
}

// --------------------------------------------------
// SETUP
// --------------------------------------------------
void setup() {
  Serial.begin      (115200);
  delay             (500);
  printHeap         ("BOOT");
  Serial.printf     ("Chip model: %s\n", ESP.getChipModel());
  Serial.printf     ("PSRAM size: %u\n", ESP.getPsramSize());
  Serial.printf     ("Free PSRAM: %u\n", ESP.getFreePsram());
  loadSettings      ();
  lastPrefsSave =   millis();
  // AUDIO
  audio.setPinout   (
    AUDIO_BCLK,     //22
    AUDIO_LRC,      //21
    AUDIO_DOUT      //26
  );
  // TFT
  tft.init          ();
  tft.setTextSize   (2);
  tft.setRotation   (def_rot);
  //LED
  pinMode           (PIN_RED, OUTPUT);
  pinMode           (PIN_GREEN, OUTPUT);
  pinMode           (PIN_BLUE, OUTPUT);
  digitalWrite      (PIN_RED,1);
  digitalWrite      (PIN_BLUE,1);
  digitalWrite      (PIN_GREEN,1);
  // SD
  bool SDOK     =   setupSD();
  #ifdef  SDSTATIONS
    loadStationsFromSD();
  #endif
  // WiFi
  setupWiFi         ();
  WiFi.onEvent([]     (WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event)  {
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.printf(
                "WIFI DISCONNECTED reason=%d\n",
                info.wifi_sta_disconnected.reason
            );
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.printf(
                "WIFI CONNECTED CH=%d\n",
                info.wifi_sta_connected.channel
            );
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.printf(
                "WIFI GOT IP %s\n",
                WiFi.localIP().toString().c_str()
            );
            break;
        default:
            break;
    }
  });
  setupTime         ();
  printHeap         ("VOOR WEER");
  display.updateWeather     ();
  printHeap         ("NA   WEER");
  printWiFiStatus   ();
  display.begin     (&tft);
  display.attachAudio(&audio);
  playStation       (stationNr);
  printHeap         ("NA STATION");
  audio.setVolume   (volume);
  printWiFiStatus   ();
  Serial.println    ("Setup klaar");
}

// --------------------------------------------------
// LOOP
// --------------------------------------------------
void loop()         {
  uint32_t t          = micros();
  audio.loop            ();
  uint32_t audioTime  = micros() - t;
  if  (audioTime>10000) {Serial.printf(">>>> audio.loop() = %lu us heap=%u largest=%u\n",audioTime,ESP.getFreeHeap(),heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));}

  t                   = micros();
  display.loop          ();
  uint32_t videoTime  = micros() - t;
  if (videoTime>15000)  {Serial.printf(">>>> video.loop() = %lu us\n", videoTime);}

  checkTouch            ();

  bool newPlaying = audio.isRunning();
  if (newPlaying != playing) {
    playing = newPlaying;
    display.setPlaying(playing);
  }

  if (!audio.isRunning()){
    ERRCNT++;
    if (ERRCNT<2){
      Serial.printf(
        "MP3 ALLOC FAIL: free=%u largest=%u internalLargest=%u\n",
        ESP.getFreeHeap(),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)
      );
      printWiFiStatus();
    }
  }
  checkPrefsSave    ();
  if (millis() - lastWeatherUpdate >= 600000){
    lastWeatherUpdate = millis();
    display.updateWeather();
  }
}
