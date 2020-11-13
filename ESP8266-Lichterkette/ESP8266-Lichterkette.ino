#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <Adafruit_NeoPixel.h>
#include <FS.h>
#include "Configuration.h"
#include <Espalexa.h>
#include "Colors.h"
#include "Settings.h"
#include "Modes.h"



#define VERSION 20201028



#define WIFI_SETUP_TIMEOUT 60

//callback functions
void handle_alexa(EspalexaDevice* dev);
void Alexa_update(int bis_auf = 100);

//create devices yourself
EspalexaDevice* alexa[ESPALEXA_MAXDEVICES];

bool alexa_aktiv = false;

Espalexa espalexa;
Adafruit_NeoPixel *pixels;

//BUTTON
uint32_t lastButtonChange = 0;
bool BUTTON_INTBOOL = false;
bool BUTTON_BOOLSTATUS = false;
bool BUTTON_BOOLAKTIV = false;
uint8_t button_status = 0;
uint32_t ButtonEinTS = 0;
uint32_t ButtonAusTS = 0;
uint32_t changedelay = 0;
bool helldirection = true;
bool speeddirection = true;

s_mysettings mysetting;
s_mode modedef[12];
s_parameter aktsetting;


uint32_t ticker = 0;
int aktmillis = 0;
String speedval = "500";
int delayval = 50;
int delayval2 = 50;
int old_mode = 99;
int LK_setting = 0;
int old_LK_setting = 99;
int web_main_speed;
bool old_LK_ein = false;

uint32_t color_line[MAXNUMPIXELS];
uint32_t color_line_neu[MAXNUMPIXELS];
uint32_t pixel_uAmpere;

//#### hilfsvar
bool sparks = false;
int clearled = 0;
int colbuff;
int colanz;
int posbuff;
int hellbuff;
int hellbuff2 = 0;
int randbuff = 1;
bool startpunkt;
int c_red_b;
int c_green_b;
int c_blue_b;

// WebServer
#ifdef ARDUINO_ARCH_ESP32
WebServer webServer(80);
#else
ESP8266WebServer webServer(80);
#endif
// DNS server 
const byte DNS_PORT = 53; 
DNSServer dnsServer; 


// WiFi
IPAddress myIP;
String STA_ssid;
String STA_pass;

bool ESP_Reboot = false;


//######################################## SETUP ######################################## 
void setup() {
  int versuche = 0;
  int aktstatuspixel = 0;
  alexa_aktiv = false;

  Serial.begin(115200);
  delay(500); // Kurze Pause, damit wir Zeit haben den Seriellen Monitor zu öffnen.
#ifdef PIN_LED
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
#endif
// Button 
  pinMode(BUTTONPIN, INPUT); 
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), buttonInterrupt, CHANGE);

  loadFromEEPROM();
  fillmodedef(mysetting.lk_anzahlpixel);
  
#ifdef DEBUG
  Serial.println("Starte Pixel: ");
  Serial.print("Pixel Pin: ");
  Serial.println(PIXELPIN);
  Serial.printf("Anzahl Pixel: %i\n",mysetting.lk_anzahlpixel );
  Serial.printf("LK Typ: %i LK kHz: %i\n",mysetting.lk_typ, mysetting.lk_khz );
#endif
  pixels = new Adafruit_NeoPixel(mysetting.lk_anzahlpixel, PIXELPIN, mysetting.lk_typ + mysetting.lk_khz);
  pixels->begin();
  
#ifdef DEBUG
  Serial.println("Filesystem mounten :");
#endif
  aktstatuspixel=status2led(aktstatuspixel, 1, WEISS);
  if (SPIFFS.begin() == false)
  {
#ifdef DEBUG
    Serial.printf("Failed to mount file system - restart\n");
#endif
    aktstatuspixel=status2led(0, 1, ROT);
    delay(10000);
    ESP.restart();  /*force reset on failure*/
  } 

  
  status2led(aktstatuspixel, 1, WEISS);
  if (ESP.getResetReason() == "External System" || ESP.getResetReason() == "Power On" || ESP.getResetReason() == "Software/System restart")
  {
    mysetting.esp_undef_Reset_count=0;
    aktstatuspixel=status2led(aktstatuspixel, 1, GRUEN);
#ifdef DEBUG
    Serial.printf("Normaler Start\n");
#endif
  }
  else
  {
    mysetting.esp_undef_Reset_count++;
    saveToEEPROM();
    aktstatuspixel=status2led(aktstatuspixel, 1, LILA);
    delay (3000);
#ifdef DEBUG
    Serial.println("letzer Start: " + ESP.getResetReason());
#endif
  }
  if ( mysetting.esp_undef_Reset_count > 9 ) {
#ifdef DEBUG
    Serial.println("zu viele fehlerhafte Resets"); 
    Serial.println("letzer Start: " + ESP.getResetReason());
    Serial.println("Reset to Default"); 
#endif
    resetToDefault();
    saveToEEPROM();
    aktstatuspixel=status2led(aktstatuspixel, 1, ROT);
    delay (3000);
  }
  aktstatuspixel++;
  LK_setting = mysetting.akt_num_settings;
  aktsetting = mysetting.parameter[LK_setting];
  if ( mysetting.nach_strom == AUS ) {
    mysetting.lk_ein = false;
  }
  else if ( mysetting.nach_strom == EIN ) {
    mysetting.lk_ein = true;
  }

  old_LK_ein = mysetting.lk_ein;

  web_main_speed = mysetting.parameter[LK_setting].main_speed;
  delayval = get_delayval(web_main_speed);
  delayval2 = delayval;
//  WiFi.disconnect(true);
  status2led(aktstatuspixel, 20, WEISS);
  WiFi.mode(WIFI_STA);
 
  while (WiFi.status() != WL_CONNECTED  && versuche < WIFI_SETUP_TIMEOUT * 2 ) {
    delay(500);
        // Flash ESP LED
#ifdef PIN_LED
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
#endif
#ifdef DEBUGWLAN
    Serial.print(".");
#endif

    status2led(versuche+aktstatuspixel, 1, ROT);
    versuche++;
  }
   WiFi.setAutoReconnect(true);

  
  if (!WiFi.isConnected())
  {

    aktstatuspixel = status2led(aktstatuspixel, 20, ROT);
    aktstatuspixel++;
    WiFi.softAP(String(mysetting.systemname));
#ifdef DEBUGWLAN
    Serial.println("No WLAN connected. Staying in AP mode.");
#endif
#ifdef PIN_LED
    digitalWrite(PIN_LED, HIGH);
#endif
    delay(3000);
    WiFi.mode(WIFI_AP);
    myIP = WiFi.softAPIP();
#ifdef DEBUGWLAN
    Serial.println("AP IP address: ");
    Serial.println(myIP);
#endif
    // Starte DNS Server für captive portal
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError); 
    dnsServer.start(DNS_PORT, "*", myIP);
  }
  else
  {
    aktstatuspixel=status2led(aktstatuspixel, 20, GRUEN);
    aktstatuspixel++;
#ifdef DEBUGWLAN
    Serial.println("WLAN connected. Switching to STA mode.");
#endif
#ifdef PIN_LED
    digitalWrite(PIN_LED, LOW);
#endif
    WiFi.mode(WIFI_STA);
    delay(1000);
    myIP = WiFi.localIP();
    STA_ssid = WiFi.SSID();
    STA_pass = WiFi.psk();
#ifdef DEBUGWLAN
    Serial.print("Hostname: ");
    Serial.println(String(mysetting.systemname));
    Serial.print("STA IP address: ");
    Serial.println(myIP);
    Serial.print("SSID: ");
    Serial.println(STA_ssid);
#endif
//    Serial.print("Passwort: ");
//    Serial.println(STA_pass);    
    // mDNS is needed to see HOSTNAME in Arduino IDE

    
// ########################## ALEXA #########################
    if ( mysetting.alexa_aktiv )
    {
#ifdef ALEXA_DEBUG
    Serial.println("Starte ALEXA");
#endif
      status2led(aktstatuspixel, mysetting.alexa_anzahl, WEISS);
      for (int z = 0; z < mysetting.alexa_anzahl; z++) 
      {
        alexa[z] = new EspalexaDevice(String(mysetting.parameter[z].settingtext), handle_alexa, EspalexaDeviceType::dimmable);
        if ( espalexa.addDevice(alexa[z]) ) 
        { 
          aktstatuspixel = status2led(aktstatuspixel, mysetting.alexa_anzahl, GRUEN);
#ifdef ALEXA_DEBUG
          Serial.printf("Alexa %i als %s angelegt.\n",z,mysetting.parameter[z].settingtext);
#endif
        }
        else
        {
          aktstatuspixel = status2led(aktstatuspixel, mysetting.alexa_anzahl, ROT);
          delay (1000);
#ifdef ALEXA_DEBUG
          Serial.printf("Alexa %i als %s nicht angelegt!!!\n",z,mysetting.parameter[z].settingtext);
#endif
        }
      }
      Alexa_update();

      espalexa.begin(&webServer);
      alexa_aktiv = true;
      aktstatuspixel++;
    }
    
#ifdef DEBUG
    Serial.println("Starting mDNS responder.");
#endif
    MDNS.begin(String(mysetting.systemname));

#ifdef DEBUG
    Serial.println("Starting OTA service.");

    ArduinoOTA.onStart([]()
      {
        Serial.println("Start OTA update.");
      });
    ArduinoOTA.onError([](ota_error_t error)
      {
        Serial.println("OTA Error: " + String(error));
        if (error == OTA_AUTH_ERROR) Serial.println("Auth failed.");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin failed.");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect failed.");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive failed.");
        else if (error == OTA_END_ERROR) Serial.println("End failed.");
      });
    ArduinoOTA.onEnd([]()
      {
        Serial.println("End OTA update.");
      });
#endif
#ifdef OTA_PASS
    ArduinoOTA.setPassword(OTA_PASS);
#endif
    ArduinoOTA.begin();
  }

#ifdef DEBUG
  Serial.println("Starting webserver.");
#endif
  status2led(aktstatuspixel, mysetting.alexa_anzahl, WEISS);
  setupWebServer();
  aktstatuspixel = status2led(aktstatuspixel, mysetting.alexa_anzahl, GRUEN);
  aktstatuspixel++;
  delay (1000);
  pixel_uAmpere = MILLIAMPERE_LK / mysetting.lk_anzahlpixel / 3 * 1000;
#ifdef MAMPERE_DEBUG
      Serial.printf("max. Pixelstrom %u uA\n",pixel_uAmpere );
#endif
}


//#####################################################################
// START L O O P
//#####################################################################
void loop() {
  
  aktmillis = millis();
  dnsServer.processNextRequest();
  MDNS.update();

  button_check();

  if (alexa_aktiv) 
  { 
     espalexa.loop();
  }
  else
  {
     webServer.handleClient();
  }
  sparks = false;
  clearled = 0;
  if ( old_mode  != mysetting.parameter[LK_setting].lmode || old_LK_setting != LK_setting || old_LK_ein != mysetting.lk_ein ) 
  {
    old_mode = mysetting.parameter[LK_setting].lmode;
    old_LK_setting = LK_setting;
    ticker = 0;
    colbuff = 0;
    colanz = 0;
    hellbuff = 0;
    hellbuff2 = 0;
    posbuff = 0;
    startpunkt = false;
    clearLines();
//    Serial.printf("loop_mode: LK_setting:%i , aktsetting.lmode: %i, mysetting.parameter[LK_setting].lmode; %i \n", LK_setting, aktsetting.lmode, mysetting.parameter[LK_setting].lmode);
  }
  
  if ( ticker > modedef[mysetting.parameter[LK_setting].lmode].maxticker) ticker = 0;
  if ( old_LK_ein != mysetting.lk_ein )
  {
    old_LK_ein = mysetting.lk_ein;
    if ( mysetting.nach_strom == WIE_VORHER ) saveToEEPROM();
#ifdef DEBUG
    if (mysetting.lk_ein)
    {
      Serial.println ("EIN!");
    }
    else
    {
      Serial.println ("AUS!");
    }
#endif
  }
  
  if ( mysetting.lk_ein )
  {
    if ( delayval < 1020 ) 
    {
      switch (mysetting.parameter[LK_setting].lmode)
      {
        case REGENBOGEN:
          Regenbogen();
        break;
        case LAUFLICHT:
          Lauflicht();
        break;
        case FERNSEHSIMULATOR:
          Fernsehsimulator();
        break;
        case FEUER:
          Feuer();
        break;
        case STROBOSKOP:
          Stroboskop();
        break;
        case LAUFPUNKT1:
          Laufpunkt1();
        break;
        case LAUFPUNKT2:
          Laufpunkt2();
        break;
        case SINUS:
          sinus();
        break;
        case FARBVERLAUF:
          farbverlauf();
        break;
        case BLUBS:
          blubs();
        break;
        default:
          ticker = ticker + 1;
          pixels->clear();
      }
    }
    zeigePixel();
  }
  else
  {
    pixels->clear();
    pixels->show();
//    delayval = 100;
  }
  while ( aktmillis + delayval > millis() )  
  {
    delay(0);
    button_check();
    if (alexa_aktiv) 
    { 
      espalexa.loop();
    }
    else
    {
      webServer.handleClient();
    }
  }
  ArduinoOTA.handle();
}

//#####################################################################
// END L O O P
//#####################################################################

//######################### BUTTON ####################################
ICACHE_RAM_ATTR void buttonInterrupt()
{
  if (millis() > lastButtonChange + 50)
  {
    lastButtonChange = millis();
    BUTTON_INTBOOL = true;
  }
//  Serial.println ("Raise Button!");
}


void button_check()
{
  bool bstatusloop = true;
  bool bstatus = true;
  bool bstatus2 = true;
  bool bstatus3 = true;

  bool button_click = false;
  bool button_doppelclick = false;
  bool button_long = false;
  bool button_short_long = false;
  bool button_long_off = false;
  bool button_short_long_off = false;
  
  if ( BUTTON_INTBOOL )
  {
    BUTTON_INTBOOL = false;
    while (bstatusloop)
    {
      bstatus = digitalRead(BUTTONPIN);
      delay(10);
      bstatus2 = digitalRead(BUTTONPIN);
      delay(10);
      bstatus3 = digitalRead(BUTTONPIN);
      delay(5);
      if ( bstatus == bstatus2 && bstatus2 == bstatus3 && digitalRead(BUTTONPIN) == bstatus ) bstatusloop = false;
    }
    BUTTON_BOOLSTATUS = !bstatus;
    if ( BUTTON_BOOLSTATUS ) //Ein
    {
      BUTTON_BOOLAKTIV = true;
      ButtonEinTS = millis();
      ButtonAusTS = 0;
//      Serial.println ("Raise Button Ein!"); 
    }
    if ( !BUTTON_BOOLSTATUS )
    {
      ButtonAusTS = millis();
      ButtonEinTS = 0;
//      Serial.println ("Raise Button Aus!");
    }
  }
  
  if ( BUTTON_BOOLAKTIV )
  {
    if ( button_status == 0 && BUTTON_BOOLSTATUS ) button_status = 1;                                                               // ein
    if ( (button_status == 1 || button_status == 9) && BUTTON_BOOLSTATUS && (millis() - ButtonEinTS) > 600  ) {button_status = 9;}  // * 9 = lang ein
    if ( button_status == 9  && !BUTTON_BOOLSTATUS ) {button_status = 10;}                                                          // * 10 = nach lang ein - aus
    if ( button_status == 1 && !BUTTON_BOOLSTATUS ) {button_status = 2;}                                                            // 2 = ein - aus
    if ( button_status == 2 && !BUTTON_BOOLSTATUS && (millis() - ButtonAusTS) > 300) {button_status = 3;}                           // * 3 = ein - aus kein doppelklick
    if ( button_status == 2 && BUTTON_BOOLSTATUS) {button_status = 4;}                                                              // 4 = ein - aus - ein
    if ( (button_status == 4 || button_status == 19) && BUTTON_BOOLSTATUS && (millis() - ButtonEinTS) > 600 ) {button_status = 19;} // * 19 = ein - aus - lang ein
    if ( button_status == 19  && !BUTTON_BOOLSTATUS ) {button_status = 20;}                                                         // * 20 = nach kurz ein lang ein - aus
    if ( button_status == 4 && !BUTTON_BOOLSTATUS && (millis() - ButtonAusTS) > 300) {button_status = 5;}                           // * 5 = ein - aus - kurz ein (doppelklick) 
     
    if (button_status == 5 ) { BUTTON_BOOLAKTIV = false;button_doppelclick=true;}
    if (button_status == 3 ) { BUTTON_BOOLAKTIV = false;button_click=true;}
    if (button_status == 9 ) button_long = true; 
    if (button_status == 10 ) { BUTTON_BOOLAKTIV = false;button_long_off = true;}
    if (button_status == 19 ) button_short_long = true;
    if (button_status == 20 ) { BUTTON_BOOLAKTIV = false;button_short_long_off = true;}

    if ( !BUTTON_BOOLAKTIV) button_status = 0;

    if (button_click)
    {
#ifdef DEBUG_BUTTON
      Serial.printf ("Button Click! button_status: %i BUTTON_BOOLSTATUS: %i \n", button_status,BUTTON_BOOLSTATUS);
#endif
      button_click_action();
    }
    if (button_doppelclick)
    {
#ifdef DEBUG_BUTTON
      Serial.printf ("Button doppelClick! button_status: %i BUTTON_BOOLSTATUS: %i \n", button_status,BUTTON_BOOLSTATUS);
#endif
      button_doubleclick_action();
    }
    if (button_long)
    {
#ifdef DEBUG_BUTTON
      Serial.printf ("Button Long! button_status: %i BUTTON_BOOLSTATUS: %i \n", button_status,BUTTON_BOOLSTATUS);
#endif
      button_long_action();
    }
    if (button_long_off)
    {
#ifdef DEBUG_BUTTON
      Serial.printf ("Button Long Aus! button_status: %i BUTTON_BOOLSTATUS: %i \n", button_status,BUTTON_BOOLSTATUS);
#endif
      helldirection = !helldirection;
    }
    if (button_short_long)
    {
#ifdef DEBUG_BUTTON
      Serial.printf ("Button kurz Long! button_status: %i BUTTON_BOOLSTATUS: %i \n", button_status,BUTTON_BOOLSTATUS);
#endif
      button_short_long_action();
    }
    if (button_short_long_off)
    {
#ifdef DEBUG_BUTTON
      Serial.printf ("Button kurz Long Aus! button_status: %i BUTTON_BOOLSTATUS: %i \n", button_status,BUTTON_BOOLSTATUS);
#endif
      speeddirection = !speeddirection;
    }
    
  }
  
}

void button_click_action() // ein Setting weiter
{
#ifdef DEBUG_BUTTON
      Serial.println ("Button click!");
#endif
  LK_setting++;
  if ( LK_setting > NUM_SETTINGS -1) LK_setting = 0;
  if ( strcmp (mysetting.parameter[LK_setting].settingtext, "-frei-") == 0 ) LK_setting = 0;
  mysetting.akt_num_settings = LK_setting;
      if ( alexa_aktiv) 
      {
        Alexa_update();
      }
      web_main_speed = mysetting.parameter[LK_setting].main_speed;
      delayval = get_delayval(web_main_speed);
      delayval2 = delayval; 
#ifdef DEBUG
      Serial.printf ("Neue Setting: %i\n",LK_setting);
      Serial.print ("Neue Geschwindigkeit: ");
      Serial.println (web_main_speed);
#endif
}

void button_doubleclick_action() // Ein/Aus
{

  mysetting.lk_ein = !mysetting.lk_ein;
#ifdef DEBUG
    Serial.printf ("Button ein/aus: %i\n",mysetting.lk_ein);
#endif
  if ( alexa_aktiv) 
  {
    Alexa_update();
  } 
}

void button_long_action()  // Helligkeit
{
  if ( (millis() > changedelay + 75 ) && helldirection ) 
  {
    changedelay = millis();
    mysetting.main_helligkeit++;
#ifdef DEBUG
    Serial.printf ("Button heller: %i\n",mysetting.main_helligkeit);
#endif
    if ( mysetting.main_helligkeit == 100 ) helldirection = false;
  }
  if ( (millis() > changedelay + 75 ) && !helldirection ) 
  {
    changedelay = millis();
    mysetting.main_helligkeit--;
#ifdef DEBUG
    Serial.printf ("Button dunkler: %i\n",mysetting.main_helligkeit);
#endif
    if ( mysetting.main_helligkeit == 0 ) helldirection = true;
  }
  
}

void button_short_long_action()  //Geschwindigkeit
{
  if ( (millis() > changedelay + 75 ) && speeddirection ) 
  {
    changedelay = millis();
    web_main_speed++;
    delayval = get_delayval(web_main_speed);
    delayval2 = delayval;
#ifdef DEBUG
    Serial.printf ("Button schneller: %i\n",web_main_speed);
#endif
    if ( web_main_speed == 100 ) speeddirection = false;
  }
  if ( (millis() > changedelay + 75 ) && !speeddirection ) 
  {
    changedelay = millis();
    web_main_speed--;
    delayval = get_delayval(web_main_speed);
    delayval2 = delayval;
#ifdef DEBUG
    Serial.printf ("Button langsamer: %i\n",web_main_speed);
#endif
    if ( web_main_speed == 0 ) speeddirection = true;
  }
}

void clearLines()
{
  for (int i=0; i<mysetting.lk_anzahlpixel; i++) 
  {
    color_line[i] = 0;
    color_line_neu[i] = 0;
  }
}

int status2led(int startled, int led_anzahl, uint32_t farbe)
{
  color_s pixelcolor;
  int s_red;
  int s_green;
  int s_blue;
  int i;
  
  pixelcolor = get_w_color(farbe);
  s_red = pixelcolor.red;
  s_green = pixelcolor.green;
  s_blue = pixelcolor.blue;
  for (int z = 0; z <= led_anzahl -1; z++) 
  { 
    i = startled + z;
    
    pixels->setPixelColor(i,pixels->Color(s_red, s_green, s_blue));
//    Serial.printf("pixelnr: %i %X\n",i, farbe);
  }
  pixels->show();
  return i + 1;
}


void farbverlauf()
{
  color_s pixelcolor_a;
  color_s pixelcolor_b;
  int colbuff_b;
  float c_red_diff = 0;
  float c_green_diff = 0;
  float c_blue_diff = 0;
  int tickeradd = 1;


  if (ticker == 0) colbuff++;
  if ( colbuff > mysetting.parameter[LK_setting].slider1-1 ) colbuff = 0;
  
  if ( colbuff == mysetting.parameter[LK_setting].slider1-1 ) 
  {
    colbuff_b = 0;
  }
  else
  {
    colbuff_b = colbuff + 1;
  }
  
  pixelcolor_a = get_w_color(mysetting.parameter[LK_setting].color[colbuff]);
  if ( ticker == 0 ) 
  {
    c_red_b = pixelcolor_a.red;
    c_green_b = pixelcolor_a.green;
    c_blue_b = pixelcolor_a.blue;
  }
  pixelcolor_b = get_w_color(mysetting.parameter[LK_setting].color[colbuff_b]);
  c_red_diff = (float(pixelcolor_a.red - pixelcolor_b.red) / 255);
  c_green_diff = (float(pixelcolor_a.green - pixelcolor_b.green) / 255);
  c_blue_diff = (float(pixelcolor_a.blue - pixelcolor_b.blue) / 255);
  

  c_red_b   = float(pixelcolor_a.red - ticker * c_red_diff);
  c_green_b = float(pixelcolor_a.green - ticker * c_green_diff);
//    Serial.printf("colbuff: %i ticker: %i posbuff: %i c_green_diff: %f pixelcolor_a.green: %i pixelcolor_b.green: %i green neu: %i\n", colbuff, ticker, posbuff, c_green_diff, pixelcolor_a.green, pixelcolor_b.green, c_green_b);
  c_blue_b  = float(pixelcolor_a.blue - ticker * c_blue_diff);

  for (int z = 1; z < mysetting.lk_anzahlpixel; z++) 
  { 
    color_line[z] = pixels->Color(c_red_b,c_green_b,c_blue_b);
  }
  if ( delayval < 50 ) tickeradd = 1 + (( 50 - delayval) / 10);
  ticker = ticker + tickeradd;
}

void sinus()
{
  float sin1wert;
  float sin2wert;
  float farbwert;
  int sinwa1;
  int sinwa2;
  int farbwert_i;
  int farbwert_anteil;
  uint32_t farbe;
  int c_red_a = 0;
  int c_green_a = 0;
  int c_blue_a = 0;
  int c_red_b = 0;
  int c_green_b = 0;
  int c_blue_b = 0;
  int c_red_e = 0;
  int c_green_e = 0;
  int c_blue_e = 0;
  color_s pixelcolor;
//  if ( ticker % 10 == 0 ) {
//   Serial.printf("Ticker: %i\n", ticker);
//  }
  sparks = mysetting.parameter[LK_setting].button2;
  for (int z = 0; z < mysetting.lk_anzahlpixel; z++)
  {
    sinwa1 = 1 + ticker / (randbuff + 1 );

      if ( sinwa1 % 6280 == 0 ) {
 //     Serial.printf("Ticker auf 0: %i\n", ticker);
      ticker = 0;
      if ( startpunkt )
      {
        colanz--;
      }
      else
      {
        colanz++;
      }
      randbuff = randbuff + 1 + random(1,3) * posbuff;
      if ( randbuff < 4 ) posbuff = 1;
      if ( randbuff > 9 ) posbuff = -2;
 //     Serial.printf("randbuff: %i colanz: %i\n", randbuff, colanz);
      
    }
    ticker++;
    sin1wert = sin(float(sinwa1)/1000) + 1.0;

    sinwa2 = 50 * (colanz + z) / (mysetting.parameter[LK_setting].slider2 * 2 );
    if ( colanz > randbuff * mysetting.lk_anzahlpixel  )
    {
      startpunkt = true;
    }
    if ( colanz <= 0)
    {
      startpunkt = false;
    }
    
    sin2wert = sin(float(sinwa2)/100) + 1.0;
    farbwert = float((sin1wert + sin2wert) * (mysetting.parameter[LK_setting].slider1 - 1) / 4) ;
    farbwert_i = farbwert;
    farbwert_anteil = 1000 - ( farbwert * 1000 - farbwert_i * 1000);
//   if ( z == 10 ) Serial.printf("farbwert_i : %i, farbwert_anteil : %i\n",farbwert_i, farbwert_anteil);
//   if ( z == 0 ) Serial.printf("neue Farbe: %i : colanz: %i, sinwa1: %i, sin1wert: %f sin2wert: %f farbwert: %f\n", z, colanz, sinwa1, sin1wert, sin2wert, farbwert);
    pixelcolor = get_w_color(mysetting.parameter[LK_setting].color[farbwert_i]);
    c_red_a = pixelcolor.red;
    c_green_a = pixelcolor.green;
    c_blue_a = pixelcolor.blue;
    pixelcolor = get_w_color(mysetting.parameter[LK_setting].color[farbwert_i+1]);
    c_red_b = pixelcolor.red;
    c_green_b = pixelcolor.green;
    c_blue_b = pixelcolor.blue;
    c_red_e = c_red_a * farbwert_anteil / 1000 + c_red_b * (1000-farbwert_anteil) / 1000; 
    c_green_e = c_green_a * farbwert_anteil / 1000 + c_green_b * (1000-farbwert_anteil) / 1000;
    c_blue_e = c_blue_a * farbwert_anteil / 1000 + c_blue_b * (1000-farbwert_anteil) / 1000;
    color_line[z] = pixels->Color(c_red_e,c_green_e,c_blue_e);
  }

}

void blubs()
{
  uint32_t farbe;
  color_s pixelcolor;
  int c_red_l = 0;
  int c_green_l = 0;
  int c_blue_l = 0;
  int c_red_m = 0;
  int c_green_m = 0;
  int c_blue_m = 0;
  int c_red_r = 0;
  int c_green_r = 0;
  int c_blue_r = 0;
  int breite = mysetting.parameter[LK_setting].slider2;
  
  for (int z = 0; z < mysetting.lk_anzahlpixel; z++)
  {
    pixelcolor = get_w_color(color_line[z]);
    c_red_m = pixelcolor.red;
    c_green_m = pixelcolor.green;
    c_blue_m = pixelcolor.blue;
    c_red_m = c_red_m * breite / 10;
    c_green_m = c_green_m * breite / 10;
    c_blue_m = c_blue_m * breite / 10;
    if ( z > 0 ) 
    {
      pixelcolor = get_w_color(color_line[z-1]);
      c_red_l = pixelcolor.red;
      c_green_l = pixelcolor.green;
      c_blue_l = pixelcolor.blue;
      c_red_l = c_red_l * breite / 19;
      c_green_l = c_green_l * breite / 19;
      c_blue_l = c_blue_l * breite / 19;
      c_red_m = c_red_m + c_red_l;
      c_green_m = c_green_m + c_green_l;
      c_blue_m = c_blue_m + c_blue_l;
    }
    if ( z < mysetting.lk_anzahlpixel-1 ) 
    {
      pixelcolor = get_w_color(color_line[z+1]);
      c_red_r = pixelcolor.red;
      c_green_r = pixelcolor.green;
      c_blue_r = pixelcolor.blue;
      c_red_r = c_red_r * breite / 22;
      c_green_r = c_green_r * breite / 22;
      c_blue_r = c_blue_r * breite /22;
       c_red_m = c_red_m + c_red_r;
      c_green_m = c_green_m + c_green_r;
      c_blue_m = c_blue_m + c_blue_r;
    }
    color_line_neu[z] = pixels->Color(c_red_m,c_green_m,c_blue_m);
  }
  for (int z = 0; z < mysetting.lk_anzahlpixel; z++)
  {
    color_line[z] = color_line_neu[z];
  }
  ticker++;
      
  if ( ticker % 3 == 0 ) 
  {
    
    farbe = mysetting.parameter[LK_setting].color[random (0,mysetting.parameter[LK_setting].slider1)]; //Zufallsfarbe
    color_line[random(1,mysetting.lk_anzahlpixel-1)]=farbe; // auf Zufallsposition
//    Serial.printf("neue Farbe: %i\n", farbe);
  }
}

void Laufpunkt1()
{
  uint32_t farbe;
  if (mysetting.parameter[LK_setting].button2) { //Zündschnur
    clearled = 15;
    delayval = 40;
    posbuff = posbuff + 1;
    if ( posbuff  > ( 1 + delayval2 / 50 )) 
    {
      posbuff = 0;
      startpunkt = true;
    }
  }
  else
  {
    delayval = delayval2;
    startpunkt = true;
  }
  if ( startpunkt ) 
  {
//    if (ticker == 1 ) Serial.printf("delayval2: %i\n", delayval2 / 50);
    startpunkt = false;
    if ( colanz >= mysetting.parameter[LK_setting].slider2 ) 
    {
      farbe = 0;
    }
    else
    {
      farbe = mysetting.parameter[LK_setting].color[colbuff];
    }
//    for (int z = mysetting.lk_anzahlpixel-1; z > 0; z--) 
    for (int z = 1; z < mysetting.lk_anzahlpixel; z++) 
    { 
      if (mysetting.parameter[LK_setting].button1) {
        color_line[z-1] = color_line[z];
      }
      else
      {
        color_line[mysetting.lk_anzahlpixel-z] = color_line[mysetting.lk_anzahlpixel-z-1];
      }
    }
    if (mysetting.parameter[LK_setting].button1) {
      color_line[mysetting.lk_anzahlpixel-1] = farbe;
    }
    else
    {
      color_line[0] = farbe;
    }
    
    ticker = ticker + 1;
    colanz = colanz + 1;

    if ( ticker == ( mysetting.lk_anzahlpixel + mysetting.parameter[LK_setting].slider2)  ) 
    {
      ticker = 0;
      colanz = 0;
      colbuff = colbuff + 1;
      if ( colbuff > mysetting.parameter[LK_setting].slider1 -1 ) colbuff = 0;
    }

  }
}

void Laufpunkt2()
{
  uint32_t farbe;
  if (mysetting.parameter[LK_setting].button2) { //Zündschnur
    clearled = 15;
    delayval = 40;
    posbuff = posbuff + 1;
    if ( posbuff  > ( 1 + delayval2 / 50 )) 
    {
      posbuff = 0;
      startpunkt = true;
    }
  }
  else
  {
    delayval = delayval2;
    startpunkt = true;
  }
  if ( startpunkt ) 
  {
//    if (ticker == 1 ) Serial.printf("delayval2: %i\n", delayval2 / 50);
    startpunkt = false;
    if ( colanz >= mysetting.parameter[LK_setting].slider2 ) 
    {
      farbe = 0;
    }
    else
    {
      farbe = mysetting.parameter[LK_setting].color[colbuff];
    }
//    for (int z = mysetting.lk_anzahlpixel-1; z > 0; z--) 
    for (int z = 1; z < mysetting.lk_anzahlpixel; z++) 
    { 
      if (mysetting.parameter[LK_setting].button1) {
        color_line[z-1] = color_line[z];
      }
      else
      {
        color_line[mysetting.lk_anzahlpixel-z] = color_line[mysetting.lk_anzahlpixel-z-1];
      }
    }
    if (mysetting.parameter[LK_setting].button1) {
      color_line[mysetting.lk_anzahlpixel-1] = farbe;
    }
    else
    {
      color_line[0] = farbe;
    }
    
    ticker = ticker + 1;
    colanz = colanz + 1;

    if ( ticker == ( mysetting.lk_anzahlpixel + mysetting.parameter[LK_setting].slider2)  ) 
    {
      ticker = 0;
      colanz = 0;
      mysetting.parameter[LK_setting].button1 = !mysetting.parameter[LK_setting].button1;
      if (mysetting.parameter[LK_setting].button1) colbuff = colbuff + 1;
      if ( colbuff > mysetting.parameter[LK_setting].slider1 -1 ) colbuff = 0;
    }

  }
}


void Stroboskop()
{
  int colnum = 0;
  uint32_t farbe;
  color_s pixelcolor;
  int c_red;
  int c_green;
  int c_blue;

  colnum = ticker % 10;
//  Serial.printf("Farbe: %i\n", colnum);
  farbe = mysetting.parameter[LK_setting].color[colnum];
  if ( mysetting.parameter[LK_setting].button1 )
  {
  //  Serial.printf("Farbe: %x\n", farbe);
    pixelcolor = get_w_color(farbe);
    hellbuff = hellbuff + hellbuff2 + ( hellbuff2 * hellbuff / 10 );
    if ( hellbuff <= 0 ) {
      hellbuff2 = 1;
      ticker = ticker + 1;
      hellbuff = 0;
    }
    
    if ( hellbuff > 255 ) {
      hellbuff = 255;
      hellbuff2 = -1;
    }
  //   Serial.printf("hellbuff: %i hellbuff2: %i\n", hellbuff , hellbuff2);
    c_red = pixelcolor.red;
    c_green = pixelcolor.green;
    c_blue = pixelcolor.blue;
    c_red = c_red * hellbuff / 255 ;
    c_green = c_green * hellbuff / 255;
    c_blue = c_blue * hellbuff / 255;
   
    for (int i=0; i<mysetting.lk_anzahlpixel; i++) 
    { 
      color_line[i] = pixels->Color(c_red,c_green,c_blue);
    }
    if (ticker == 10 ) ticker = 0;
  }
  else
  {
    for (int i=0; i<mysetting.lk_anzahlpixel; i++) 
    { 
      color_line[i] = farbe;
    }
    clearled = 20;
    ticker = ticker + 1;
    if (ticker == 10 ) ticker = 0;
  }
}

void Feuer()
{
  uint32_t farbe;
  int colnum = 0;
  color_s pixelcolor;
  color_s pixelcolor_neu;
  uint8_t hell;
  int c_red;
  int c_green;
  int c_blue;
  ticker++;
  sparks = mysetting.parameter[LK_setting].button2;
  for (int i=0; i<mysetting.lk_anzahlpixel; i++) 
  { 
     
     pixelcolor = get_w_color(color_line[i]);
     if ( color_line[i] == color_line_neu[i] ) {
//      if ( i == 0 ) Serial.printf("neue Farbe1: alt: %u -> neu: %u\n", color_line[i], color_line_neu[i] );
        colnum = random (0,5);
//        if ( i == 0 ) Serial.printf("neue Farbe1 colnum: %i\n", colnum );
        hell = random(1,mysetting.parameter[LK_setting].slider1+1);
        pixelcolor_neu = get_w_color(mysetting.parameter[LK_setting].color[colnum]);
        pixelcolor_neu.red = pixelcolor_neu.red / hell;
        pixelcolor_neu.green = pixelcolor_neu.green / hell;
        pixelcolor_neu.blue = pixelcolor_neu.blue / hell;
        color_line_neu[i] = pixels->Color(pixelcolor_neu.red,pixelcolor_neu.green,pixelcolor_neu.blue);
     }
     pixelcolor_neu = get_w_color(color_line_neu[i]);
     c_red = pixelcolor.red;
     c_green = pixelcolor.green;
     c_blue = pixelcolor.blue;
     c_red = c_red + ( pixelcolor_neu.red - c_red) / 5;
     c_green = c_green + ( pixelcolor_neu.green - c_green) / 5;
     c_blue = c_blue + ( pixelcolor_neu.blue - c_blue) / 5;
     if ( c_red < pixelcolor_neu.red ) c_red++;
     if ( c_red > pixelcolor_neu.red ) c_red--;
     if ( c_green < pixelcolor_neu.green ) c_green++;
     if ( c_green > pixelcolor_neu.green ) c_green--;
     if ( c_blue < pixelcolor_neu.blue ) c_blue++;
     if ( c_blue > pixelcolor_neu.blue ) c_blue--; 
     color_line[i] = pixels->Color(c_red, c_green, c_blue);
  }
}

void Fernsehsimulator()
{
  uint32_t farbe;
  uint32_t color_line_bck[mysetting.lk_anzahlpixel];
  int colnum = 0;
  int block = 0;
  uint8_t hell;
  color_s pixelcolor;
   if ( ticker < 100 ) 
   {
     if ( (ticker % 10) == 0 )
     {
      for (int i=0; i<mysetting.lk_anzahlpixel; i++) 
      { 
        if (  block < 1 ) 
        {
          block = random (1,mysetting.lk_anzahlpixel/10);
          colnum = random (0,10);
        }
        farbe = mysetting.parameter[LK_setting].color[colnum];
//        if ( ticker == 0 ) Serial.printf("colnum: %i farbe %i\n", colnum, farbe );
        pixelcolor = get_w_color(farbe);
        hell = random (1,20);
        pixelcolor.red = pixelcolor.red / hell;
        pixelcolor.blue = pixelcolor.blue / hell;
        pixelcolor.green = pixelcolor.green / hell;
        color_line[i] = pixels->Color(pixelcolor.red, pixelcolor.green, pixelcolor.blue);
        block = block - 1;
      }
     }  
     if ( (ticker % 10) == 5 )
     {
      for (int i=0; i<mysetting.lk_anzahlpixel; i++) 
      { 
        if (  block < 1 ) 
        {
          block = random (1,mysetting.lk_anzahlpixel/20);
          colnum = random (1,4);
        }
        if ( colnum == 1 ) {
          farbe = 0;
          color_line[i] = farbe;
        }
        block = block - 1;
      }
     }
   }  
   else 
   {
     for (int i=0; i<(mysetting.lk_anzahlpixel); i++)
     {
       color_line_bck[i] = color_line[i];
     }
     for (int i=1; i<(mysetting.lk_anzahlpixel-1); i++) 
     {  
        if ( i > (mysetting.lk_anzahlpixel/2))
        { 
           color_line[i+1] = color_line_bck[i];
           color_line[i] = color_line_bck[i-1]; 
        }
        else
        {
          color_line[i-1] = color_line[i];
          color_line[i] = 0;
        }
     }
   }
   ticker = ticker + random(1,4);
}


void Lauflicht()
{
  uint8_t colnum;
  uint32_t farbe;
  int colanzsave = colanz;
  int colbuffsave = colbuff;

//  farbe = mysetting.parameter[LK_setting].color[colbuff];
//  color_line[0] = farbe;
  sparks = mysetting.parameter[LK_setting].button2;
  for (int i=0; i<mysetting.lk_anzahlpixel; i++) 
  { 
    if ( colanz >= mysetting.parameter[LK_setting].slider2 ) 
    {
      colanz = 0;
      colbuff = colbuff + 1;
      if ( colbuff > mysetting.parameter[LK_setting].slider1-1 ) colbuff = 0; 
    }
    colanz = colanz + 1;
//    Serial.printf("colbuff: %i colanz %i\n", colbuff, colanz );
    farbe = mysetting.parameter[LK_setting].color[colbuff];
    if ( mysetting.parameter[LK_setting].button1 )
    {
      color_line[i] = farbe;
    }
    else
    {
      color_line[mysetting.lk_anzahlpixel-i-1] = farbe;
    }
  }
//  Serial.printf("colbuff: %i colanz %i\n", colbuff, colanz );
  colbuff = colbuffsave;
  colanz = colanzsave - 1;
  if ( colanz < 0 ) { 
    colanz = mysetting.parameter[LK_setting].slider2 - 1;
    colbuff = colbuffsave - 1;
    if ( colbuff < 0 ) colbuff = mysetting.parameter[LK_setting].slider1-1;
  }
//  Serial.printf("colbuff: %i colanz %i\n", colbuff, colanz );
}


void Regenbogen()
{
  int pixcolor;
  int c_red;
  int c_green;
  int c_blue;
  int anzahl_regenbogen = 1;
  int farbintervall;
  uint32_t startfarbe;
  sparks = mysetting.parameter[LK_setting].button2;
  anzahl_regenbogen = mysetting.parameter[LK_setting].slider1;
  farbintervall = 1 + ((1003  - delayval) / 100);
  ticker = ticker + farbintervall;
  startfarbe = ticker;
//  for (int startfarbe = 0; startfarbe < 765; startfarbe = startfarbe + farbintervall)
//  {
    farbintervall = 1 + ((1003  - delayval) / 100);
    for(int i=0; i<mysetting.lk_anzahlpixel; i++) 
    { 
      delay(0);
      pixcolor = startfarbe + i * anzahl_regenbogen * 765 / mysetting.lk_anzahlpixel;
      if ( pixcolor > 765 ) pixcolor = pixcolor % 765;
      
      if (pixcolor < 256 ) {
        c_red = 255 - pixcolor;
        c_green = pixcolor;
        c_blue = 0;
      }
      if (pixcolor > 255 && pixcolor < 512 ) {
        c_red = 0 ;
        c_green = 511 - pixcolor;
        c_blue = (256 - pixcolor) * -1;
      }
      if (pixcolor > 511 )
      {
//        c_red = (512 - pixcolor) * -1;
        c_green = 0;
//        c_blue = (pixcolor - 767) * -1;
        c_blue = (pixcolor - 767) * (pixcolor - 767) / 255;    // Farbkorrektur da sonst zuviel blau Anteil   
        c_red = 255 - c_blue;
      }
      if ( mysetting.parameter[LK_setting].button1 )
      {
        color_line[i]=pixels->Color(c_red, c_green, c_blue);
//        pixels->setPixelColor(i, pixels->Color(c_red, c_green, c_blue));
      }
      else
      {
        color_line[mysetting.lk_anzahlpixel-i-1]=pixels->Color(c_red, c_green, c_blue);
//        pixels->setPixelColor(mysetting.lk_anzahlpixel-i-1, pixels->Color(c_red, c_green, c_blue));
      }
    }
 
}

void zeigePixel()
{
  uint8_t c_red;
  uint8_t c_green;
  uint8_t c_blue;
  uint32_t lk_uAmpere=0;
  uint8_t lk_brightness = 0;
  
    for(int i=0; i<mysetting.lk_anzahlpixel; i++) 
    { 
       c_red = (color_line[i] >> 16 & 0xFF)  * mysetting.main_helligkeit * 0.01;
       c_green = (color_line[i] >> 8 & 0xFF )  * mysetting.main_helligkeit * 0.01;
       c_blue = (color_line[i] & 0xFF) * mysetting.main_helligkeit * 0.01;
       
       if ( sparks ) {
        if ( random (1,51) == 1 ) {
          if ( modedef[mysetting.parameter[LK_setting].lmode].anzahlfarben == 10 || modedef[mysetting.parameter[LK_setting].lmode].anzahlfarben == 0 )
          {
            c_red = 255;
            c_green = 255;
            c_blue = 255;
          }
          else
          {
            c_red = mysetting.parameter[LK_setting].color[modedef[mysetting.parameter[LK_setting].lmode].anzahlfarben -1] >> 16 & 0xFF;
            c_green = mysetting.parameter[LK_setting].color[modedef[mysetting.parameter[LK_setting].lmode].anzahlfarben -1] >> 8 & 0xFF;
            c_blue = mysetting.parameter[LK_setting].color[modedef[mysetting.parameter[LK_setting].lmode].anzahlfarben -1] & 0xFF;
          }
        }
       }
//       if ( i == 0 ) Serial.printf("color_line: %i; c_red: %i c_green %i c_blue\n", color_line[i], c_red, c_green,c_blue  );
      lk_uAmpere = lk_uAmpere + pixel_uAmpere * c_red / 255 ;
      lk_uAmpere = lk_uAmpere + pixel_uAmpere * c_green / 255 ;
      lk_uAmpere = lk_uAmpere + pixel_uAmpere * c_blue / 255 ;

      pixels->setPixelColor(i,pixels->Color(c_red, c_green, c_blue));
    }
#ifdef MAMPERE_DEBUG
      Serial.printf("ges. Kettenstrom %u mA\n",lk_uAmpere/1000 );
#endif
    if ( lk_uAmpere/1000 > MILLIAMPERE_NETZTEIL ) 
    {
      lk_brightness=float(255 * MILLIAMPERE_NETZTEIL / (lk_uAmpere/1000));
#ifdef MAMPERE_DEBUG
      Serial.printf("Helligkeit red. %i (255)\n",lk_brightness );
#endif
      pixels->setBrightness(lk_brightness);
    }
    else
    {
      pixels->setBrightness(255);
    }
    pixels->show();
    if ( clearled > 0 ) {
      delay (clearled);
      pixels->clear();
      pixels->show();
    }
}

//################################################################################################################################################
//##### Root
//################################################################################################################################################

void handleRoot()
{
  String checked = "checked";

  int web_main_hell;
#ifdef DEBUG
  Serial.println ( webServer.hostHeader());
#endif
  if ( webServer.arg("myselect") != "" )
  {

    if ( webServer.arg("myselect").toInt() != LK_setting ) 
    {
#ifdef DEBUG
      Serial.println("Neuer Setting");
      Serial.println("myselect:" + webServer.arg("myselect"));
#endif
      LK_setting = webServer.arg("myselect").toInt();
      aktsetting = mysetting.parameter[LK_setting];
//    aktsetting = mysetting.parameter[LK_setting];
      mysetting.akt_num_settings = LK_setting;
      web_main_speed = mysetting.parameter[LK_setting].main_speed;
      delayval = get_delayval(web_main_speed);
      delayval2 = delayval;
    }
  }
  
  web_main_hell = mysetting.main_helligkeit;
  if ( alexa_aktiv && !ESP_Reboot ) 
  {
    Alexa_update();
  }
  
  String message = F("<!doctype html>"
"<html><head>"
"<title>Lichterkette</title>"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<meta charset=\"UTF-8\">"
"<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/android-icon-192x192.png\">"
"<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/favicon-32x32.png\">"
"<script src=\"/jquery-3.1.1.min.js\"></script>"
"<link rel=\"stylesheet\" href=\"/common1.css\" >"

"</head>\n"
"<body>"

"<form name=\"lk_setting\" action=\"/\" method=\"GET\">"
"<div class=\"section over-hide z-bigger\">"
  "<div class=\"background-color\"></div>"
  "<div class=\"section over-hide z-bigger\">"
    "<div class=\"container pb-5\">"
      "<div class=\"row justify-content-center pb-5\">"
        "<div class=\"mb-4 pb-2\">");
        if (mysetting.lk_ein) 
        {
          message += F("<img class=\"lk_g\" src=\"/lk_g.gif\" value=\"on\">");
        }
        else
        {
          message += F("<img class=\"lk_g\" src=\"/lk0.png\" value=\"off\">");
        }
    
        message += F("</div>"
        "<div class=\"checkboxcontainer\">\n");

        for ( int setting=0; setting < NUM_SETTINGS; setting++ )
        {
          checked = "";
          if ( setting == LK_setting ) checked = "checked";
            message += "<input class=\"checkbox-tools\" type=\"radio\" name=\"myselect\" id=\"tool-" + String(setting+1) + "\" " + checked +" value=\"" + String(setting) + "\">\n"
          "<label class=\"for-checkbox-tools\" for=\"tool-" + String(setting+1) + "\">\n";
          if ( mysetting.alexa_aktiv && mysetting.alexa_anzahl > setting )
          {
            message += "<i class='uil'>A" + String(setting+1) + "</i>" + mysetting.parameter[setting].settingtext + "</label>\n";
          } 
          else
          {
            message += "<i class='uil'>" + String(setting+1) + "</i>" + mysetting.parameter[setting].settingtext + "</label>\n";
          }
        }
        message += F("</div>"
      "</div>"
    "</div>"
    "<br>"
    "<div class=\"hellslidercontainer\">"
        "Helligkeit:"
        "<output id=\"houtput\" for=\"hellslider\"></output>"
      "<br>");
       message += "<input id=\"hellslider\" type=\"range\"  min=\"1\" max=\"100\" value=\"" + String(web_main_hell) + "\" step=\"1.0\" class=\"hellslider\">";
      message += F("</div>"
    "<br>"
    "<div class=\"speedslidercontainer\">"
        "Geschwindigkeit:"
        "<output id=\"soutput\" for=\"speedslider\"></output>"
      "<br>");
      message += "<input id=\"speedslider\" type=\"range\"  min=\"0\" max=\"100\" value=\"" + String(web_main_speed) + "\" step=\"1.0\" class=\"speedslider\">";
      message += F("</div>"
     "<br>"
    "<div class=\"buttonscontainer\">"
       "<button class=\"buttons\" type=\"submit\" formaction=\"/handlewifisettings\">&#128295;</button>"
       "<button class=\"buttons\" type=\"submit\" formaction=\"/handlemodes\">&auml;ndern</button>"
    "</div>"
    "</div>"
"</div>"
  
"</form>"
    
 "<script>"
  "var urlBase = \"/\";"
  "var postValueTimer = {};"

  "$(document).ready(function(){"
   "var value = $(\"#speedslider\").val();"
   "$(\"#soutput\").val(value);"
   "var value = $(\"#hellslider\").val();"
   "$(\"#houtput\").val(value); "
  "});" 
  
  "$(\"#speedslider\").on(\"input\", function() {"
   "var newvalue = $(\"#speedslider\").val();"
   "$(\"#soutput\").val(newvalue); "
   "delayPostValue(\"speed\",newvalue);"
  "});"
   
  "$(\"#hellslider\").on(\"input\", function() {"
   "var newvalue = $(\"#hellslider\").val();"
    "$(\"#houtput\").val(newvalue);"
    "delayPostValue(\"hell\",newvalue);"
  "});"

   "$(\".checkbox-tools\").click(function() {"
    "newvalue = $(\"input[class='checkbox-tools']:checked\").val();"
    "var body = { name: \"myselect\", value: newvalue };"
    "$.post(urlBase + \"myselect\" + \"?value=\" + newvalue, body, function(res){ $(\"#soutput\").val(res);$(\"#speedslider\").val(res); });"
    
   "});"

   "$(\".lk_g\").click(function() {"
    "newvalue = $(\"input[class='checkbox-tools']:checked\").val();"
    "postValue(\"myselect\",newvalue);"
   "});"

   "bild_on = \"lk_g.gif\";"
   "bild_off = \"lk0.png\";"

   "$(\".lk_g\").click(function() {"
     "aktvalue = $(this).attr(\"value\");"
    
    "if( aktvalue == \"off\" ) {"
      "newvalue = \"on\";"
      "$(this).attr(\"value\",newvalue);"
      "$(this).attr('src',bild_on);"
    "}"
    "else {"
      "newvalue = \"off\";"
      "$(this).attr(\"value\",newvalue);"
      "$(this).attr('src',bild_off);"
    "}"
    "postValue(\"onoff\",newvalue);"

    "});"


  "function delayPostValue(name, value) {"
   "clearTimeout(postValueTimer);"
   "postValueTimer = setTimeout(function() {"
    "postValue(name, value);"
    "}, 300);"
  "}"
  
  "function postValue(name, value) {"
   "var body = { name: name, value: value };"
   "$.post(urlBase + name + \"?value=\" + value, body );"
  "}"
  
"</script>"
  "</body></html>");
  webServer.send(200, "text/html", message);
}
//################################################################################################################################################
//##### Settings (WiFi)
//################################################################################################################################################

void handleWiFiSettings()
{
    String cssid;
    String ownssid;

    ownssid = WiFi.SSID();
    if ( ownssid == "" ) ownssid = "SID";
    int n = WiFi.scanNetworks();
    int indices[n];
    
    if (n == 0) {
#ifdef DEBUG
          Serial.println("no networks found");
#endif
    } else {
     //sort networks

      for (int i = 0; i < n; i++) {
        indices[i] = i;
      }
      // RSSI SORT
      for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
          }
        }
      }
      // remove duplicates
      for (int i = 0; i < n; i++) {
        if (indices[i] == -1) continue;
        cssid = WiFi.SSID(indices[i]);
        for (int j = i + 1; j < n; j++) {
          if (cssid == WiFi.SSID(indices[j])) {
            indices[j] = -1; // set dup aps to index -1
          }
        }
      }
    }
    
    String message = F("<html><head>"
    "<title>WiFi Settings</title>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<meta charset=\"UTF-8\">"
    "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/android-icon-192x192.png\">"
    "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/favicon-32x32.png\">"
    "<script src=\"/jquery-3.1.1.min.js\"></script>"
    "<link rel=\"stylesheet\" href=\"/common2.css\" >"
    "<style>"
    "table{border-collapse:collapse;margin:0px auto;} td{padding:12px;padding-left:0px;padding-right:1px;border-bottom:1px solid #111111;} tr:first-child{border-top:1px solid #111111;} td:first-child{text-align:right;} td:last-child{text-align:left;}"
    "</style>"
    "</head><body>"
      "<h2> Einstellungen </h2>"
      "<div class=\"wifisetting\">"
      "<form name=\"wifi\" action=\"/WiFi\" method=\"GET\">"
      "<table>"
      "<tr><td width=50>Name:</td><td>");
//############ HOSTNAME
      message += "<input type =\"text\" class=\"wifisetting_input\" id=\"systemname\" name=\"systemname\" size=\"29\" minlength=\"5\" maxlength=\"29\" value= \"" + String(mysetting.systemname) + "\">";
      message += F("</td></tr>"
      
      "<tr><td width=50>SID:</td><td>");
  message += "<input type=\"search\" class=\"wifisetting_input\" id=\"wlansid\" list=\"wlanliste\" placeholder=\"" + ownssid +"\" name=\"wlansid\">";
  message += F("<datalist id=\"wlanliste\">");
     for (int i = 0; i < n; i++) {
       if (indices[i] == -1) continue; // skip dups
       if ( WiFi.RSSI(indices[i]) < -90 ) continue;  // zu schlechter Empfangspegel
#ifdef DEBUG
       Serial.print(WiFi.SSID(indices[i]));
       Serial.print(" : ");
       Serial.println (WiFi.RSSI(indices[i]));
#endif
       message += "<option value=\"" + WiFi.SSID(indices[i]) +"\">\n";
     }
  message += F("</datalist></td></tr>"
      "<tr><td width=50>Passwort:</td><td>"
      "<input id=\"wlanpw\" class=\"wifisetting_input\" name=\"wlanpw\" minlength=\"6\" >"
      "</td></tr>"
      "<tr><td width=50>NeoPixel Typ:</td><td>"
      "<select class=\"lktypselect\" name=\"neopixeltyp\">"
      "<option value=\"");
      message += String(NEO_RGB) + "\"";
      if (mysetting.lk_typ == NEO_RGB ) message += " selected";
        message += F(">"
        "NEO_RGB</option>"      
      "<option value=\"");
      message += String(NEO_RBG) + "\"";
      if (mysetting.lk_typ == NEO_RBG ) message += " selected";
        message += F(">"
        "NEO_RBG</option>" 
      "<option value=\"");
      message += String(NEO_GRB) + "\"";
      if (mysetting.lk_typ == NEO_GRB ) message += " selected";
        message += F(">"
        "NEO_GRB</option>" 
      "<option value=\"");
      message += String(NEO_BRG) + "\"";
      if (mysetting.lk_typ == NEO_BRG ) message += " selected";
        message += F(">"
        "NEO_BRG</option>"
      "<option value=\"");
      message += String(NEO_BGR) + "\"";
      if (mysetting.lk_typ == NEO_BGR ) message += " selected";
        message += F(">"
        "NEO_BGR</option>"
      "<option value=\"");
      message += String(NEO_WRGB) + "\"";
      if (mysetting.lk_typ == NEO_WRGB ) message += " selected";
        message += F(">"
        "NEO_WRGB</option>"
      "<option value=\"");
      message += String(NEO_WRBG) + "\"";
      if (mysetting.lk_typ == NEO_WRBG ) message += " selected";
        message += F(">"
        "NEO_WRBG</option>" 
      "<option value=\"");
      message += String(NEO_WGRB) + "\"";
      if (mysetting.lk_typ == NEO_WGRB ) message += " selected";
        message += F(">"
        "NEO_WGRB</option>" 
      "<option value=\"");
      message += String(NEO_WGBR) + "\"";
      if (mysetting.lk_typ == NEO_WGBR ) message += " selected";
        message += F(">"
        "NEO_WGBR</option>" 
      "<option value=\"");
      message += String(NEO_WBRG) + "\"";
      if (mysetting.lk_typ == NEO_WBRG ) message += " selected";
        message += F(">"
        "NEO_WBRG</option>" 
      "<option value=\"");
      message += String(NEO_WBGR) + "\"";
      if (mysetting.lk_typ == NEO_WBGR ) message += " selected";
        message += F(">"
        "NEO_RWGB</option>" 
      "<option value=\"");
      message += String(NEO_RWGB) + "\"";
      if (mysetting.lk_typ == NEO_RWGB ) message += " selected";
        message += F(">"
        "NEO_RWBG</option>" 
      "<option value=\"");
      message += String(NEO_RWBG) + "\"";
      if (mysetting.lk_typ == NEO_RWBG ) message += " selected";
        message += F(">"
        "NEO_RWBG</option>" 
      "<option value=\"");
      message += String(NEO_RGWB) + "\"";
      if (mysetting.lk_typ == NEO_RGWB ) message += " selected";
        message += F(">"
        "NEO_RGWB</option>" 
        "</select>"
        "</td></tr>"
        
      "<tr><td width=50>NeoPixel kHz:</td><td>"
      "<select class=\"lktypselect\" name=\"neopixelkhz\">"
      "<option value=\"");
      message += String(NEO_KHZ800) + "\"";
      if (mysetting.lk_khz == NEO_KHZ800 ) message += F(" selected");
        message += F(">"
        "NEO_KHZ800</option>"      
      "<option value=\"");
      message += String(NEO_KHZ400) + "\"";
      if (mysetting.lk_khz == NEO_KHZ400 ) message += F(" selected");
        message += F(">"
        "NEO_KHZ400</option>" 
        "</select>"
        "</td></tr>");    
      message += F("<tr><td width=50>Anzahl Pixel:");
      message += F("</td><td>");
      message += "<input id=\"pixelslider\" name=\"pixel_anzahl\" type=\"number\"  min=\"" + String(MINNUMPIXELS) + "\" max=\"" + String(MAXNUMPIXELS) + "\" value=\"" + String(mysetting.lk_anzahlpixel) + "\" step=\"1.0\" class=\"wifisetting_input\">";
      message += F("</td></tr>" );

                                                                                                              
      message += F( "<tr><td width=50>nach Stromausfall:</td><td>"
      "<input type=\"radio\" name=\"stromausfall\" value=\"0\"");
      if ( mysetting.nach_strom == 0 ) message += F(" checked");
      message += F("> Aus "
       "<input type=\"radio\" name=\"stromausfall\" value=\"1\"");
      if ( mysetting.nach_strom == 1 ) message += F(" checked");
      message += F("> Ein "
       "<input type=\"radio\" name=\"stromausfall\" value=\"2\"");
      if ( mysetting.nach_strom == 2 ) message += F(" checked");
      message += F("> wie&nbsp;vorher"
       "</td></tr>");
    if ( WiFi.isConnected() )
    {
      message += F("<tr><td width=50>Alexamode:</td><td>"
      "<input type=\"radio\" id=\"alexaaus\" name=\"alexamode\" value=\"0\"");
      if ( !mysetting.alexa_aktiv ) message += F(" checked");
      message += F("> Aus "
       "<input type=\"radio\" id=\"alexaan\" name=\"alexamode\" value=\"1\"");
      if ( mysetting.alexa_aktiv ) message += F(" checked");
      message += F("> Ein "
       "</td></tr>");
      message += F("<tr id=\"alexageraete\"> <td width=50>Alexa<br>Anzahl Geräte: ");
      message += F("<output align=\"center\" id=\"alexaoutput\" for=\"alexaslider\"></output></td><td>");
//      message += F("<div class=\"wifisetting_input\">");
      message += "<input id=\"alexaslider\" name=\"alexa_anzahl\" type=\"range\"  min=\"1\" max=\"" + String(ESPALEXA_MAXDEVICES) + "\" value=\"" + String(mysetting.alexa_anzahl) + "\" step=\"1.0\" class=\"speedslider\">";
      message += F("</td></tr>" );
    }
 message += F(
      "</table>"
      "<br>"
      "<div class=\"buttonscontainer\">"
      "<button class=\"buttons\" name=\"action\" value=\"3\">Übernehmen und Neustart</button>"
      "</div>");
 message += "<br>Version: " + String(VERSION);
 message += F("</div>"
    "</form>");

 message += F("<script>"

  "$(document).ready(function(){"
    "if ( $(\"#alexaaus\").prop('checked') ) {$(\"#alexageraete\").hide();};"
    "$(\"#alexaaus\").click(function(){"
      "$(\"#alexageraete\").hide();"
     "});"
    "$(\"#alexaan\").click(function(){"
      "$(\"#alexageraete\").show();"
     "});"
   "var value = $(\"#alexaslider\").val();"
   "$(\"#alexaoutput\").val(value);"

//   "var value = $(\"#pixelslider\").val();"
//   "$(\"#pixeloutput\").val(value);"
  "});" 
  
  "$(\"#alexaslider\").on(\"input\", function() {"
   "var newvalue = $(\"#alexaslider\").val();"
   "$(\"#alexaoutput\").val(newvalue); "
  "});"

//   "$(\"#pixelslider\").on(\"input\", function() {"
//   "var newvalue = $(\"#pixelslider\").val();"
//   "$(\"#pixeloutput\").val(newvalue); "
//  "});"
  
  "</script>"
    
  "</body></html>");
    webServer.send(200, "text/html", message);
}

//################################################################################################################################################
//##### Modes
//################################################################################################################################################

void handleModes()
{
  String farbe;
  if ( webServer.arg("myselect") != "" )
  {
#ifdef DEBUG 
    Serial.println("handleModes: myselect:" + webServer.arg("myselect"));
#endif
    LK_setting = webServer.arg("myselect").toInt();
   }
  String message = F("<!doctype html>");

  message += F("<html><head>"
"<title>Lichterkette Effekte</title>"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<meta charset=\"UTF-8\">"
"<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/android-icon-192x192.png\">"
"<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/favicon-32x32.png\">"
"<script src=\"html5colorpicker.js\"></script>"
"<script src=\"/jquery-3.1.1.min.js\"></script>"
"<link rel=\"stylesheet\" href=\"/common2.css\" >"

"</head><body>"
"<form name=\"lk_type\" action=\"/Settings\" method=\"POST\">");
  message += "<div class=\"settingtextcontainer\">" + String(LK_setting+1) + ".&nbsp;"
//############ SETTINGTEXT
    "<input type =\"text\" id=\"modetext\" name=\"modetext\" size=\"29\" maxlength=\"29\" class=\"settingtext\" value= \"" + String(mysetting.parameter[LK_setting].settingtext) + "\">";
  message += F("</div>"
//############ MODEAUSWAHL
  "<div class=\"modeselectcontainer\">"
    "<select name=\"mymodeselect\" id=\"myselect\" onchange=\"document.forms['lk_type'].submit()\" class=\"modeselect\">");
    for ( int modenum = 0; modenum <= MODE_COUNT; modenum ++)
    {
      if (mysetting.parameter[LK_setting].lmode == modenum) 
      {
        message +="<option selected value=\""+ String(modenum)+ "\">" + modedef[modenum].modetext + "</option>";
      }
      else
      {
        message +="<option value=\""+ String(modenum)+ "\">" + modedef[modenum].modetext + "</option>";
      }
    }
    message += F("</select>"
  "</div>"
  "<hr>");
//############ FARBENAUSWAHL
  if ( modedef[mysetting.parameter[LK_setting].lmode].anzahlfarben > 0 )
  {
    message += F("<div class=\"cpickercontainer\">"
    "<p class=\"cpickertext\">Farben Auswahl</p> "
    "<canvas id=\"canvas\"></canvas>"
    "<div class=\"inputs-list\">");
    for ( int colnum = 0; colnum<modedef[mysetting.parameter[LK_setting].lmode].anzahlfarben; colnum++)
    {
      farbe = "000000" + String(mysetting.parameter[LK_setting].color[colnum],HEX);
      farbe = farbe.substring( farbe.length()-6,farbe.length());
      message += "<input id=\"color" + String(colnum) +"\" name=\"co_" + String(colnum)+ "\" onclick=\"picker.editInput(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"#" + farbe + "\" readonly>\n";
      if (colnum == 4) message +="<br>\n";
    }
    message += F("</div>"
    "</div>"
    "<br>");
  }
//############ OPTSLIDER1
  if ( modedef[mysetting.parameter[LK_setting].lmode].optslider1 )
  {
    message += F("<div class=\"speedslidercontainer\">"); 
    message += modedef[mysetting.parameter[LK_setting].lmode].optslider1txt;
    message += F(":<output id=\"opt1output\" for=\"opt1slider\"></output>"
   "<br>");
   message += "<input id=\"opt1slider\" name=\"opt1\" type=\"range\"  min=\"" + String(modedef[mysetting.parameter[LK_setting].lmode].optslider1_min) +"\" max=\"" + String(modedef[mysetting.parameter[LK_setting].lmode].optslider1_max) + "\" value=\"" + String(mysetting.parameter[LK_setting].slider1) + "\" step=\"1.0\" class=\"speedslider\">"
   "</div>";
  }
//############ OPTSLIDER2
  if ( modedef[mysetting.parameter[LK_setting].lmode].optslider2 )
  {
  message += F("<div class=\"speedslidercontainer\">");
  message += modedef[mysetting.parameter[LK_setting].lmode].optslider2txt;
  message += F(":<output id=\"opt2output\" for=\"opt2slider\"></output>"
  "<br>");
  message += "<input id=\"opt2slider\" name=\"opt2\" type=\"range\"  min=\"" + String(modedef[mysetting.parameter[LK_setting].lmode].optslider2_min) +"\" max=\"" + String(modedef[mysetting.parameter[LK_setting].lmode].optslider2_max) + "\" value=\"" + String(mysetting.parameter[LK_setting].slider2) + "\" step=\"1.0\" class=\"speedslider\">"
  "</div>";
  }
//############ OPTSWITCH1
  if ( modedef[mysetting.parameter[LK_setting].lmode].optswitch1 )
  {
    message += F("<input type=\"hidden\" id=\"chk1av\" name=\"chk1av\" value=\"1\">"
    "<label class=\"chk_switchlabe1\" id=\"chk1label\" for=\"chk1\">");
    message += modedef[mysetting.parameter[LK_setting].lmode].optswitch1txt; 
    message += F(":</label>");
    if ( mysetting.parameter[LK_setting].button1 )
    {
      message += F("<input type=\"checkbox\" id=\"chk1\" name=\"chk1\" class=\"chk_switchbutton\" value=\"1\" checked>");
    }
    else
    {
      message += F("<input type=\"checkbox\" id=\"chk1\" name=\"chk1\" class=\"chk_switchbutton\" value=\"1\">");
    }  
  }
//############ OPTSWITCH2
  if ( modedef[mysetting.parameter[LK_setting].lmode].optswitch2 )
  {
    message += F("<input type=\"hidden\" id=\"chk2av\" name=\"chk2av\" value=\"1\">");
    message += F("<label class=\"chk_switchlabel\" id=\"chk2label\" for=\"chk2\">");
    message += modedef[mysetting.parameter[LK_setting].lmode].optswitch2txt; 
    message += F(":</label> ");
    if ( mysetting.parameter[LK_setting].button2 )
    {
      message += F("<input type=\"checkbox\" id=\"chk2\" name=\"chk2\" class=\"chk_switchbutton\" value=\"1\" checked>");
    }
    else
    {
      message += F("<input type=\"checkbox\" id=\"chk2\" name=\"chk2\" class=\"chk_switchbutton\" value=\"1\">");
    }  
  }
//############ SPEEDSLIDER
  message += F("<hr>"
  "<div class=\"speedslidercontainer\">"
  "Geschwindigkeit:"
  "<output id=\"soutput\" for=\"speedslider\"></output>"
  "<br>");
  message += "<input id=\"speedslider\" name=\"speed\" type=\"range\"  min=\"1\" max=\"100\" value=\"" + String(mysetting.parameter[LK_setting].main_speed) + "\" step=\"1.0\" class=\"speedslider\">";
  message += F( "</div>"
//  "<br>"
  "<hr>"
//############ BUTTONS
  "<div class=\"buttonscontainer\">"
    "<button class=\"buttons\" name=\"action\" value=\"2\">abbrechen</button>"
//    "<button class=\"buttons\" name=\"action\" value=\"0\">aktuallisieren</button>"
    "<button class=\"buttons\" name=\"action\" value=\"1\">speichern</button>"
  "</div>"

"</form>"
  "<script>");
  
  message += F(
  "var urlBase = \"/\";"
  "var postValueTimer = {};"

  "$(document).ready(function(){"
  
   "var value = $(\"#opt1slider\").val();"
   "$(\"#opt1output\").val(value);"

   "var value = $(\"#opt2slider\").val();"
   "$(\"#opt2output\").val(value);"
  
   "var value = $(\"#speedslider\").val();"
   "$(\"#soutput\").val(value);"

  "});" 

  "$(\"#opt1slider\").on(\"input\", function() {"
   "var newvalue = $(\"#opt1slider\").val();"
   "$(\"#opt1output\").val(newvalue); "
   "delayPostValue(\"opt1slider\",newvalue);"
  "});"

  "$(\"#opt2slider\").on(\"input\", function() {"
   "var newvalue = $(\"#opt2slider\").val();"
   "$(\"#opt2output\").val(newvalue); "
   "delayPostValue(\"opt2slider\",newvalue);"
  "});"

  "$(\"#chk1\").on(\"input\", function() {"
   "if ( $(\"#chk1\").prop('checked') ) {var newvalue = \"1\"} else {var newvalue = \"0\"};"
   "postValue(\"chk1\",newvalue);"
  "});"

  "$(\"#chk2\").on(\"input\", function() {"
   "if ( $(\"#chk2\").prop('checked') ) {var newvalue = \"1\"} else {var newvalue = \"0\"};"
   "postValue(\"chk2\",newvalue);"
  "});"
  
  "$(\"#speedslider\").on(\"input\", function() {"
   "var newvalue = $(\"#speedslider\").val();"
   "$(\"#soutput\").val(newvalue); "
   "delayPostValue(\"speed\",newvalue);"
  "});"

  "function delayPostValue(name, value) {"
   "clearTimeout(postValueTimer);"
   "postValueTimer = setTimeout(function() {"
    "postValue(name, value);"
    "}, 300);"
  "}"
  "function postValue(name, value) {"
   "var body = { name: name, value: value };"
   "$.post(urlBase + name + \"?value=\" + value, body);"
  "}"

   " var picker = new KellyColorPicker({\n" 
      " place : 'canvas',\n"
      " size  : 190,\n"
      " userEvents : {\n"
         " change : function(self) {\n"
            " if (!self.selectedInput) return;\n"
            " if (self.getCurColorHsv().v < 0.5)\n"
               " self.selectedInput.style.color = \"#FFFFFF\";\n"
            " else\n"
               " self.selectedInput.style.color = \"#000000\";\n"
               " self.selectedInput.value = self.getCurColorHex();\n"    
               " self.selectedInput.style.background = self.selectedInput.value;\n"  
               " farb_wert = self.getCurColorHex();"
               " farb_name = self.selectedInput.name;"
               " farbausgabe = farb_name + \":\" + farb_wert.replace(\"#\",\"\");"
               " delayPostValue(\"farbwahl\", farbausgabe);" 
         " }\n"
      " }\n"
   " });\n"
   " picker.editInput = function(target) {\n"
      " if (picker.selectedInput) picker.selectedInput.classList.remove('selected');\n"
      " if (target) picker.selectedInput = target;\n"
      " if (!picker.selectedInput) return false;\n"
      
      " picker.selectedInput.classList.add('selected');\n"    
      " picker.setColor(picker.selectedInput.value);\n"
      " picker.selectedInput.style.background = picker.selectedInput.value;\n"
      
   " }\n"
   " var mInputs = document.getElementsByClassName('multi-input');\n"
   " for (var i = mInputs.length; i > -1; i--) {\n"
     " picker.editInput(mInputs[i]);\n"
   " }\n"
   "</script>");
  message += F("</body></html>");
  webServer.send(200, "text/html", message);
}


//##########################################################################################
//### Files von SPIFFS
//##########################################################################################
String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)

  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  String pathWithzip = path + ".zip";
  if (SPIFFS.exists(pathWithzip) || SPIFFS.exists(path)) {                            // If the file exists
    if(SPIFFS.exists(pathWithzip))                          // If there's a compressed version available
      path += ".zip";   
#ifdef DEBUG
  Serial.println("handleFileRead: " + path);
#endif      
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = webServer.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  } 


  if (path.startsWith("/generate_204") || path.startsWith("/fwlink") ) 
  {
    if (webServer.hostHeader() != String(mysetting.systemname) ) {
#ifdef DEBUG
      Serial.print ("captive portal: ");
      Serial.println ( webServer.hostHeader());
#endif
      webServer.sendHeader("Location", String("http://" + String(mysetting.systemname) ), true);
      webServer.send(302, "text/plain", "");
      webServer.client().stop();
    }
    else
    {
      handleRoot;
    }
    return true;
  }
#ifdef DEBUG
  Serial.println("\tFile Not Found");
#endif
  return false;                                         // If the file doesn't exist, return false
}


/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

void callRoot()
{
  webServer.send(200, "text/html", "<!doctype html><html><head><script>window.onload=function(){window.location.replace('/');}</script></head></html>");
}
void callMode()
{
  webServer.send(200, "text/html", "<!doctype html><html><head><script>window.onload=function(){window.location.replace('/handlemodes');}</script></head></html>");
}

//################################################################################################################################################
//##### handleCommitWiFi
//################################################################################################################################################

void handleCommitWiFi()
{
  int versuche = 0;
  bool alexamode;
  bool save = false;
  String new_wlansidstring = "";
  String new_wlanpwstring = "";
  String new_systemname = "";
  
  ESP_Reboot = true;
  
  new_systemname = webServer.arg("systemname");
  if (new_systemname.length() > 5 && new_systemname != String(mysetting.systemname))
  {
#ifdef DEBUG
    Serial.println ("Systemname von " + String(mysetting.systemname) + " nach " + webServer.arg("systemname") + " geändert!");
#endif
    new_systemname.toCharArray(mysetting.systemname,new_systemname.length()+1);
    save = true;
  }
  
  #ifdef DEBUG
    Serial.println ("Nach Stromausfall: " + webServer.arg("stromausfall"));
  #endif
  if ( mysetting.nach_strom != webServer.arg("stromausfall").toInt() ){
    mysetting.nach_strom = webServer.arg("stromausfall").toInt();
    save = true;
  }
  
  if ( webServer.arg("alexamode") == "0") alexamode = false;
  if ( webServer.arg("alexamode") == "1") alexamode = true;
  if ( mysetting.alexa_aktiv != alexamode )
  {
 #ifdef DEBUG
   if ( alexamode ) Serial.println ("Alexamode Ein!");
   if ( !alexamode ) Serial.println ("Alexamode Aus!");
 #endif
    mysetting.alexa_aktiv = alexamode;
    save = true;
  }
 #ifdef DEBUG
    Serial.println ("Anzahl Alexa: " + webServer.arg("alexa_anzahl"));
 #endif
  if ( mysetting.alexa_anzahl != webServer.arg("alexa_anzahl").toInt() ){
    mysetting.alexa_anzahl = webServer.arg("alexa_anzahl").toInt();
    save = true;
  }
  
  if ( webServer.arg("neopixeltyp").toInt() != mysetting.lk_typ )
  {
#ifdef DEBUG
    Serial.println ("Neopixeltyp: " + webServer.arg("neopixeltyp"));
#endif
    mysetting.lk_typ = webServer.arg("neopixeltyp").toInt();
    save = true;
  }
  
  if ( webServer.arg("neopixelkhz").toInt() != mysetting.lk_khz )
  {
#ifdef DEBUG
    Serial.println ("NeopixelkHz: " + webServer.arg("neopixelkhz"));
#endif
    mysetting.lk_khz = webServer.arg("neopixelkhz").toInt();
    save = true;
  }
  
  if ( webServer.arg("pixel_anzahl").toInt() != mysetting.lk_anzahlpixel )
  {
#ifdef DEBUG
    Serial.println ("Anzahl Pixel: " + webServer.arg("pixel_anzahl"));
#endif
    mysetting.lk_anzahlpixel = webServer.arg("pixel_anzahl").toInt();
    save = true;
  }

  if ( save ) saveToEEPROM();

  new_wlansidstring = webServer.arg("wlansid");
  if ( new_wlansidstring == "" ) new_wlansidstring = WiFi.SSID();
  
  new_wlanpwstring = webServer.arg("wlanpw");
  
#ifdef DEBUG
  Serial.println ("Aktion: " + webServer.arg("action"));
  Serial.println ("SSID: " + new_wlansidstring);
  Serial.println ("PW: " + new_wlanpwstring);
#endif

  callRoot();
  for (int z = 1; z < 50; z++) 
  { 
    delay(100);
    if (alexa_aktiv) 
    { 
      espalexa.loop();
    }
    else
    {
      webServer.handleClient();
    }
  }
  delay(1000);
  if (new_wlanpwstring.length() > 6 && new_wlansidstring.length() > 3 )
  {
     WiFi.disconnect(true);
     WiFi.mode(WIFI_STA);
     WiFi.begin(new_wlansidstring, new_wlanpwstring);
     while (WiFi.status() != WL_CONNECTED  && versuche < 20 ) {
     delay(500);
#ifdef DEBUG
      Serial.print(".");
#endif
      versuche++;
    }
#ifdef DEBUG
    Serial.println(".");
#endif
    delay(1000);
    WiFi.setAutoReconnect(true);
    delay(1000);
  }
  delay(2000);
  ESP.restart();

}

//################################################################################################################################################
//##### handleCommitSettings
//################################################################################################################################################

void handleCommitSettings()
{

  if ( webServer.arg("action") == "2" ) //abbrechen
  {
#ifdef DEBUG
    Serial.println("handleCommitSettings: abbrechen"); 
#endif
    mysetting.parameter[LK_setting] = aktsetting; 
    callRoot();
  }
  else
  {
    for ( uint8_t i = 0; i <= 9; i++)
    { 
      if ( webServer.arg("co_" + String(i)) != "" )
      {
#ifdef DEBUG
        Serial.println("co_" + String(i) + ":" + webServer.arg("co_" + String(i)));
#endif
        mysetting.parameter[LK_setting].color[i] = string_to_num(webServer.arg("co_" + String(i)));
 //       Serial.printf("Farbcode %x\n", mysetting.parameter[LK_setting].color[i], HEX); 
      }
    }

    if ( webServer.arg("opt1") != "" )
    {
      mysetting.parameter[LK_setting].slider1 = webServer.arg("opt1").toInt();
    }
    if ( webServer.arg("opt2") != "" )
    {
      mysetting.parameter[LK_setting].slider2 = webServer.arg("opt2").toInt();
    }
    if ( webServer.arg("chk1av") == "1" )
    {
      if ( webServer.arg("chk1") == "1" )
      {
      mysetting.parameter[LK_setting].button1 =true;
      }
      else
      {
        mysetting.parameter[LK_setting].button1 =false;
      }
    }
    if ( webServer.arg("chk2av") == "1" )
    {
      if ( webServer.arg("chk2") == "1" )
      {
        mysetting.parameter[LK_setting].button2 =true;
      }
      else
      {
        mysetting.parameter[LK_setting].button2 =false;
      }
    }

    speedval = webServer.arg("speed");
    mysetting.parameter[LK_setting].main_speed = speedval.toInt();
    delayval = get_delayval(speedval.toInt());
    delayval2 = delayval;

    webServer.arg("modetext").toCharArray(mysetting.parameter[LK_setting].settingtext,webServer.arg("modetext").length()+1);

    if ( webServer.arg("action") == "" ) //aktuallisieren (Neuer Mode)
    {
      if ( webServer.arg("mymodeselect") != "" ) mysetting.parameter[LK_setting].lmode = webServer.arg("mymodeselect").toInt();
      if ( mysetting.parameter[LK_setting].slider1 < modedef[mysetting.parameter[LK_setting].lmode].optslider1_min or 
           mysetting.parameter[LK_setting].slider1 > modedef[mysetting.parameter[LK_setting].lmode].optslider1_max )
           mysetting.parameter[LK_setting].slider1 = modedef[mysetting.parameter[LK_setting].lmode].optslider1_min;
      if ( mysetting.parameter[LK_setting].slider2 < modedef[mysetting.parameter[LK_setting].lmode].optslider2_min or 
           mysetting.parameter[LK_setting].slider2 > modedef[mysetting.parameter[LK_setting].lmode].optslider2_max )
           mysetting.parameter[LK_setting].slider2 = modedef[mysetting.parameter[LK_setting].lmode].optslider2_min;
      callMode();
    }
    if ( webServer.arg("action") == "0" ) //aktuallisieren
    {
#ifdef DEBUG
      Serial.println("handleCommitSettings: aktuallisieren"); 
#endif
      callMode();
    }
    if ( webServer.arg("action") == "1" ) //sichern
    {
      aktsetting = mysetting.parameter[LK_setting];
#ifdef DEBUG
      Serial.println("handleCommitSettings: sichern");
#endif
      saveToEEPROM();
      callRoot();
    }
  }
}

//################################################################################################################################################
//##### root handles
//################################################################################################################################################

void handle_hell()
{
  mysetting.main_helligkeit = webServer.arg("value").toInt();
   if ( alexa_aktiv) 
   {
     Alexa_update();
   }
#ifdef DEBUG
  Serial.print ("Neue Helligkeit: ");
  Serial.println ( mysetting.main_helligkeit);
#endif
  webServer.send(200, "text/plain", "OK");
}

void handle_speed()
{
  web_main_speed = webServer.arg("value").toInt();
  delayval = get_delayval(web_main_speed);
  delayval2 = delayval;
#ifdef DEBUG
  Serial.print ("Neue Geschwindigkeit: ");
  Serial.println ( webServer.arg("value").toInt());
#endif
  webServer.send(200, "text/plain", "OK");
}

void handle_myselect()
{

  if ( webServer.arg("value").toInt() != LK_setting ) 
  {
#ifdef DEBUG
      Serial.print("Neuer Setting: ");
      Serial.println("myselect:" + webServer.arg("value"));
#endif   
      LK_setting = webServer.arg("value").toInt();
      aktsetting = mysetting.parameter[LK_setting];
      mysetting.akt_num_settings = LK_setting;
      if ( alexa_aktiv) 
      {
        Alexa_update();
      }
      web_main_speed = mysetting.parameter[LK_setting].main_speed;
      delayval = get_delayval(web_main_speed);
      delayval2 = delayval; 
#ifdef DEBUG
      Serial.print ("Neue Geschwindigkeit: ");
      Serial.println (web_main_speed);
#endif
  }
  webServer.send(200, "text/plain", String(mysetting.parameter[LK_setting].main_speed));
}


void handle_onoff()
{
#ifdef DEBUG
  Serial.print ("Ein/Aus: ");
  Serial.println ( webServer.arg("value"));
#endif
  if (webServer.arg("value") == "on" );
  {
    mysetting.lk_ein = true;
    if ( alexa_aktiv) 
    {
      Alexa_update();
    }
  }
  if ( webServer.arg("value") == "off" )
  {
    mysetting.lk_ein = false;
    if ( alexa_aktiv) 
    {
      Alexa_update();
    }
  }  

  webServer.send(200, "text/plain", "OK");
}

//################################################################################################################################################
//##### mode handles 
//################################################################################################################################################
void handle_opt1slider()
{
  mysetting.parameter[LK_setting].slider1 = webServer.arg("value").toInt();
#ifdef DEBUG
  Serial.print ("Neue Wert Slider1: ");
  Serial.println ( mysetting.parameter[LK_setting].slider1);
#endif
  webServer.send(200, "text/plain", "OK");
}

void handle_opt2slider()
{
  mysetting.parameter[LK_setting].slider2 = webServer.arg("value").toInt();
#ifdef DEBUG
  Serial.print ("Neue Wert Slider2: ");
  Serial.println ( mysetting.parameter[LK_setting].slider2);
#endif
  webServer.send(200, "text/plain", "OK");
}

void handle_chk1()
{
  if ( webServer.arg("value") == "1" )
  {
    mysetting.parameter[LK_setting].button1 =true;
  }
  else
  {
    mysetting.parameter[LK_setting].button1 =false;
  }
  
#ifdef DEBUG
  Serial.print ("Neue Wert CHK1: ");
  Serial.println ( mysetting.parameter[LK_setting].button1);
#endif
  webServer.send(200, "text/plain", "OK");
}

void handle_chk2()
{
  if ( webServer.arg("value") == "1" )
  {
    mysetting.parameter[LK_setting].button2 =true;
  }
  else
  {
    mysetting.parameter[LK_setting].button2 =false;
  }
  
#ifdef DEBUG
  Serial.print ("Neue Wert CHK2: ");
  Serial.println ( mysetting.parameter[LK_setting].button2);
#endif
  webServer.send(200, "text/plain", "OK");
}

void handle_farbwahl()
{
  
  int farbnummer = webServer.arg("value").substring(3,4).toInt();
  String farbwert = "#" + webServer.arg("value").substring(5,11);
  mysetting.parameter[LK_setting].color[farbnummer] = string_to_num(farbwert);
#ifdef DEBUG
  Serial.print ("Neue Farbwert Nr. ");
  Serial.print (farbnummer);
  Serial.print (" : ");
  Serial.printf("Farbcode %x\n", mysetting.parameter[LK_setting].color[farbnummer], HEX);
#endif
  webServer.send(200, "text/plain", "OK");
}

//################################################################################################################################################
int get_delayval(int webval)
{
  int delval;
  delval =  1 + (101 - webval) * (101 - webval) / 10;
  if ( webval == 99 ) delval = 2;
  if ( webval == 98 ) delval = 3;
  if ( webval == 97 ) delval = 4;
  if ( webval == 96 ) delval = 5;
  if ( webval == 95 ) delval = 6;
  if ( webval == 94 ) delval = 7;
  if ( webval == 93 ) delval = 8;
  return delval;
  
}

uint32_t string_to_num(String in_color)
{
  char buffer[9];
  in_color.substring(1, 7).toCharArray(buffer,7);
  return strtol(buffer, 0, 16);
}

color_s get_w_color(uint32_t in_color)
{
  color_s s_color;
  s_color.red = in_color >> 16 & 0xFF;
  s_color.green = in_color >> 8 & 0xFF;
  s_color.blue = in_color & 0xFF;
  return s_color;
}

void handle_alexa(EspalexaDevice* d)
{
  int alexa_id;
  if (d == nullptr) return;
  alexa_id = d->getId();
#ifdef ALEXA_DEBUG
    Serial.print("Alexa Setting ändert: Nr: ");
    Serial.print(d->getId() );
    Serial.print(" Prozent: ");
    Serial.println(d->getPercent());
#endif
  if ( d->getPercent() > 0 ) 
  {
    LK_setting = d->getId();
    mysetting.main_helligkeit = d->getPercent();
    if ( mysetting.lk_ein == false ) 
    {
        mysetting.lk_ein = true;
#ifdef ALEXA_DEBUG
        Serial.print("Alexa Ein: ");
#endif
    }
  }
  if ( d->getPercent() == 0 && d->getId() == LK_setting ) 
  {  
      mysetting.lk_ein = false;
#ifdef ALEXA_DEBUG
    Serial.print("Alexa Aus: ");
    Serial.println(d->getPercent());
#endif
  }

      aktsetting = mysetting.parameter[LK_setting];
      mysetting.akt_num_settings = LK_setting;
      web_main_speed = mysetting.parameter[LK_setting].main_speed;
      delayval = get_delayval(web_main_speed);
      delayval2 = delayval; 
      Alexa_update(LK_setting);

}

void Alexa_update(int bis_auf)
{
  
  for (int z = 0; z < mysetting.alexa_anzahl; z++) 
  {
    if ( z != bis_auf )
    {
      if ( z == mysetting.akt_num_settings && mysetting.lk_ein)
      {
  #ifdef ALEXA_DEBUG
      Serial.printf("Alexa %i auf %i \n",z,mysetting.main_helligkeit);
  #endif
        alexa[z]->setPercent(mysetting.main_helligkeit);
      }
      else
      {
  #ifdef ALEXA_DEBUG
      Serial.printf("Alexa %i auf 0 \n",z);
  #endif
        alexa[z]->setPercent(0);
      }
    }
  }
}


/******************************************************************************
  Webserver
******************************************************************************/

void setupWebServer()
{
  webServer.on("/", handleRoot);
  webServer.on("/onoff", HTTP_POST, handle_onoff);
  webServer.on("/hell", HTTP_POST, handle_hell);
  webServer.on("/speed", HTTP_POST, handle_speed);
  webServer.on("/myselect", HTTP_POST, handle_myselect);
  webServer.on("/opt1slider", HTTP_POST, handle_opt1slider);
  webServer.on("/opt2slider", HTTP_POST, handle_opt2slider);
  webServer.on("/chk1", HTTP_POST, handle_chk1);
  webServer.on("/chk2", HTTP_POST, handle_chk2); 
  webServer.on("/farbwahl", HTTP_POST, handle_farbwahl); 
  webServer.on("/Settings", HTTP_POST, handleCommitSettings);
  webServer.on("/WiFi", HTTP_GET, handleCommitWiFi);
  webServer.on("/handlemodes", HTTP_GET, handleModes);
  webServer.on("/handlewifisettings", HTTP_GET, handleWiFiSettings);
  webServer.onNotFound([]() {
    if (!alexa_aktiv) {
      if (!handleFileRead(webServer.uri()))                     // send it if it exists
        {
          //whatever you want to do with 404s
          webServer.send(404, "text/plain", "Not found");
        }
    }
    else
    {
      if (!espalexa.handleAlexaApiCall(webServer.uri(),webServer.arg(0))) //if you don't know the URI, ask espalexa whether it is an Alexa control request
      {
        if (!handleFileRead(webServer.uri()))                     // send it if it exists
        {
          //whatever you want to do with 404s
          webServer.send(404, "text/plain", "Not found");
        }
      }
    }
  });
  if (!alexa_aktiv) webServer.begin();
#ifdef DEBUG
  Serial.println("HTTP server started");
#endif
}
