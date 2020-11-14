//******************************************************************************
// Configuration.h
//******************************************************************************
#ifndef CONFIGURATION_H
#define CONFIGURATION_H


//******************************************************************************
// folgende Werte bitte anpassen:
#define BUTTONPIN               D3  // da wo der Button angeschlossen ist
#define PIXELPIN                D2  // da wo die Lichterkette angeschlossen ist
#define PIN_LED        LED_BUILTIN
#define MILLIAMPERE_PIXEL     57.9  // max. Stromaufnahme (mA) eines Pixels (3*LED)
#define MILLIAMPERE_NETZTEIL 10000  // max. Strom des Netzteils (mA)
#define ESPALEXA_MAXDEVICES     30  // max. Anzahl von Alexa Geräten
//#define OTA_PASS "1234"             // OTA Passwort (für die Übertragung des SPIFFS auskommentieren!)


//******************************************************************************
//DEBUG:
//#define DEBUG
#define DEBUGWLAN
//#define MAMPERE_DEBUG
//#define ESPALEXA_DEBUG
//#define ALEXA_DEBUG
//#define DEBUG_BUTTON


//******************************************************************************
// nicht ändern:
#define NUM_SETTINGS        30            // nicht Ändern
#define MAXNUMPIXELS       300            // max Anzahl Pixel
#define MINNUMPIXELS        30            // min Anzahl Pixel
#define DEFAULT_NUMPIXELS  240            // DefaultAnzahl der Pixels
#define DEFAULT_NEO_TYP   NEO_GRB         // Default Pixeltyp
#define DEFAULT_NEO_KHZ   NEO_KHZ800      // Default PixelkHz
#define DEFAULT_HOSTNAME  "Lichterkette"  // Default Hostname
#define ALEXA_DEVICES  3


//******************************************************************************
#endif
