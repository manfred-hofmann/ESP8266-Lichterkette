# ESP8266-Lichterkette 
# Mit Alexa Steuerung
# Eine Lichterkette mit bis zu 30 verschiedenen Settings, die alle vom User frei einstellbar sind.  
Hier gehts zur Wiki: https://github.com/manfred-hofmann/ESP8266-Lichterkette/wiki  
Ich verwende als ESP8266 einen WEMOS D1 mini.  
Aber es funktioniert auch mit einem ESP8266-01. Allerdings gehen beim ESP8266-01 keine OTA-Uploads mehr, da beim ESP8266-01 der Speicher zu klein ist.  
In der Configuration.h können folgende Parameter gepflegt werden:  
// folgende Werte bitte anpassen:  
#define BUTTONPIN               D3  // da wo der Button angeschlossen ist  
#define PIXELPIN                D2  // da wo die Lichterkette angeschlossen ist  
#define PIN_LED        LED_BUILTIN  // Die eingebaute LED beim WEMOS D1 mini (hier muss nichts geändert werden!) Für einen ESP8266-01 bitte auskommentieren!  
#define MILLIAMPERE_PIXEL     57.9  // max. Stromaufnahme (mA) eines Pixels (3*LED) 
#define MILLIAMPERE_NETZTEIL 10000  // max. Strom des Netzteils (mA)  
#define ESPALEXA_MAXDEVICES     30  // max. Anzahl von Alexa   
  
Danach einfach den Sketch und SketchData Upload durchführen.  
Dann das WLAN "Lichterkette" z.B. mit dem Handy anwählen und im Netzwerk anmelden auswählen.  
https://github.com/manfred-hofmann/ESP8266-Lichterkette/blob/master/Hauptmenue.JPG  
Dann wird die Webseite der Lichterkette geöffnet. Hier dann mit dem Schraubenschlüssel Button die WLAN Parameter des eigenen WLANs eingeben.  
https://github.com/manfred-hofmann/ESP8266-Lichterkette/blob/master/Einstellungen.JPG  
Außerdem kann hier der Lichterkettentyp und die Anzahl der Pixel eingestellt werden.  
Zusätzlich kann das Verhalten nach einem Stromausfall definiert werden.  
Hier kann auch eingestellt werden, wieviel Alexa Geräte zur Verfügung gestellt werden.  
Diese werden danach durch ein A vor der jeweiligen Zahl im Hauptmenü gekennzeichnet.
Nachdem die Alexa die Geräte gefunden hat ("Alexa suche Geräte"), kann durch z.B. "Alexa Regenbogen an" auf dem Regenbogen umgeschaltet werden.
Es wird immer der Name aus dem Menü verwendet. Also darauf achten, das der Name nicht doppelt vorhanden ist.

Über die Leuchtschrift oben kann die Lichterkette aus und eingeschaltet werden.  

Funktion des Buttons:  
- einmal kurz drücken: einen Settingpunkt weiter  
- zweimal kurz drücken: Aus/Ein  
- lang drücken: Verändern der Helligkeit  
- einmal kurz und danach lang drücken: Verändern der Geschwindigkeit  

Die einzelnen Settings können jeweils durch den ändern Button geändert werden:  
https://github.com/manfred-hofmann/ESP8266-Lichterkette/blob/master/Modes.JPG

Viel Spaß damit.

