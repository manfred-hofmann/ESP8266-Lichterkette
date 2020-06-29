
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <Adafruit_NeoPixel.h>
#include <FS.h>
#include "Colors.h"

#define HOSTNAME "esp01_test"
#define WIFI_SETUP_TIMEOUT 180

#define PIXELPIN        2
#define NUMPIXELS      10
#define DELAYVAL      200

Adafruit_NeoPixel pixels(NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);

uint8_t color1_red;
uint8_t color1_green;
uint8_t color1_blue;
int aktmillis = 0;

ESP8266WebServer webServer(80);

IPAddress myIP = { 0,0,0,0 };

void setup() {
  Serial.begin(115200);
  delay(1000); // Kurze Pause, damit wir Zeit haben den Seriellen Monitor zu öffnen.
  Serial.println("Filesystem mounten :)");
  if (SPIFFS.begin() == false)
  {
    Serial.printf("Failed to mount file system - restart\n");
    delay(10000);
    ESP.restart();  /*force reset on failure*/
  } 
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.setTimeout(WIFI_SETUP_TIMEOUT);
  wifiManager.autoConnect(HOSTNAME);
  WiFi.setAutoReconnect(true);
  if (!WiFi.isConnected())
  {
//    WiFi.mode(WIFI_AP);
    Serial.println("No WLAN connected. Staying in AP mode.");
    WiFi.mode(WIFI_AP);
    delay(1000);
    myIP = WiFi.softAPIP();
  }
  else
  {
    WiFi.mode(WIFI_STA);
    Serial.println("WLAN connected. Switching to STA mode.");
    delay(1000);
    myIP = WiFi.localIP();

    // mDNS is needed to see HOSTNAME in Arduino IDE
    Serial.println("Starting mDNS responder.");
    MDNS.begin(HOSTNAME);
    //MDNS.addService("http", "tcp", 80);
  }
  Serial.println("connected...yeey :)");
  Serial.println("Starting webserver.");
  setupWebServer();
  pixels.begin();
}

void loop() {
  aktmillis = millis();
  pixels.clear();
  // put your main code here, to run repeatedly:
  webServer.handleClient();
  MDNS.update();
    for(int i=0; i<NUMPIXELS; i++) 
    { 
    delay(0);
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(i, pixels.Color(w_color[0].red, w_color[0].green, w_color[0].blue));

    pixels.show();   // Send the updated pixel colors to the hardware.

    delay(0);
    while ( aktmillis + DELAYVAL > millis() )  webServer.handleClient();
    aktmillis = millis();
    }
    delay(0);
}


void handleRoot()
{
  String message = "<!doctype html>"
    "<html>"
    "<head>"
    "<title>" + String(HOSTNAME) + "</title>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<meta http-equiv=\"refresh\" content=\"300\" charset=\"UTF-8\">"
    "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/android-icon-192x192.png\">"
    "<script src=\"/html5kellycolorpicker.min.js\"></script>"
    "<link href=\"/common.css\" rel=\"stylesheet\">"
    "<style>"
    "body{background-color:#FFFFFF;text-align:center;color:#333333;font-family:Sans-serif;font-size:16px;}"
    "button{background-color:#1FA3EC;text-align:center;color:#FFFFFF;width:200px;padding:10px;border:5px solid #FFFFFF;font-size:24px;border-radius:10px;}"
    "span {color: Black;}"
    "</style>"
    "</head>"
    "<body>"
    "<h2>" + String(HOSTNAME) + "</h2>\n";
  message += "<form action=\"/commitSettings\" method=\"get\">";
//  message += "<h2> Hallo </h2>\n";
  message += "<br>\n"
"  <div class=\"example-wrap example-wrap-test-5\">\n"
"  <p class=\"example-top\">Farben Auswahl</p> \n"
"  <canvas id=\"canvas\"></canvas>\n"
"  <div class=\"inputs-list\">\n"
"    <input id=\"color1\" name=\"co_0\" onclick=\"picker.editInput(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"" + s_color[0] + "\" readonly>\n"
"    <input id=\"color2\" name=\"co_1\" onclick=\"picker.editInput(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"" + s_color[1] + "\" readonly>\n"
"    <input id=\"color3\" name=\"co_2\" onclick=\"picker.editInput(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"" + s_color[2] + "\" readonly>\n"
"    <input id=\"color4\" name=\"co_3\" onclick=\"picker.editInput(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"" + s_color[3] + "\" readonly>\n"
"    <br>\n"
"    <input id=\"color5\" name=\"co_4\" onclick=\"picker.editInput(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"" + s_color[4] + "\" readonly>\n"
"    <input id=\"color6\" name=\"co_5\" onclick=\"picker.editInput(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"" + s_color[5] + "\" readonly>\n"
"    <input id=\"color7\" name=\"co_6\" onclick=\"picker.editInput(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"" + s_color[6] + "\" readonly>\n"
"    <input id=\"color8\" name=\"co_7\" onclick=\"picker.editInput(event.target)\" onchange=\"picker.editInput(event.target)\"  class=\"multi-input input-quad\" value=\"" + s_color[7] + "\" readonly>\n"
"  </div>\n"

"   <script>\n"
"   var picker = new KellyColorPicker({ \n"
"      place : 'canvas', \n"
"      size  : 200,         \n"
"      userEvents : { \n"
"         change : function(self) {\n"
"            if (!self.selectedInput) return;\n"
"            if (self.getCurColorHsv().v < 0.5)\n"
"               self.selectedInput.style.color = \"#FFF\";\n"
"            else\n"
"               self.selectedInput.style.color = \"#000\";\n"
"            self.selectedInput.value = self.getCurColorHex();    \n"
"            self.selectedInput.style.background = self.selectedInput.value;   \n"
"         }\n"
"      }\n"
"   });\n"

"   picker.editInput = function(target) {\n"

"      if (picker.selectedInput) picker.selectedInput.classList.remove('selected');   \n"
"      if (target) picker.selectedInput = target;\n"
"      if (!picker.selectedInput) return false;\n"
"      \n"
"      picker.selectedInput.classList.add('selected');    \n"
"      picker.setColor(picker.selectedInput.value);\n"
"   }\n"

"   var mInputs = document.getElementsByClassName('multi-input');\n"
"   for (var i = 0; i < mInputs.length; i++) {\n"
"      picker.editInput(mInputs[i]);\n"
"   }\n"

"   </script>\n"

"</div> \n"
"<br>\n"
"   <button type=\"submit\" formaction=\"/commitSettings\">aktuallisieren</button>\n";
  
  message += "</body></html>";
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
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)

  Serial.println("handleFileRead: " + path);

  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = webServer.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }

  Serial.println("\tFile Not Found");

  return false;                                         // If the file doesn't exist, return false
}
void callRoot()
{
  webServer.send(200, "text/html", "<!doctype html><html><head><script>window.onload=function(){window.location.replace('/');}</script></head></html>");
}
//##########################################################################################
void handleCommitSettings()
{
  char c_color[10];
  char c_color1_red[3] = "";
  char c_color1_green[3] = "";
  char c_color1_blue[3] = "";
  Serial.println("Commit settings pressed.");
  for ( uint8_t i = 0; i <= 8; i++)
  { 
    Serial.println("co_" + String(i) + ":" + webServer.arg("co_" + String(i)));
    s_color[i] = webServer.arg("co_" + String(i));
    w_color[i] = get_w_color(s_color[i]);
    Serial.printf("Farbcode %i: rot = %i gruen = %i blau = %i \r\n", i, w_color[i].red, w_color[i].green, w_color[i].blue );
  }
  callRoot();
}

color_s get_w_color(String in_color)
{
  char c_color_red[3] = "";
  char c_color_green[3] = "";
  char c_color_blue[3] = "";
  color_s s_color;

  in_color.substring(1, 3).toCharArray(c_color_red,3);
  in_color.substring(3, 5).toCharArray(c_color_green,3);
  in_color.substring(5, 7).toCharArray(c_color_blue,3);
  s_color.red = (int) strtol(c_color_red, 0, 16);
  s_color.green = (int) strtol(c_color_green, 0, 16);
  s_color.blue = (int) strtol(c_color_blue, 0, 16);
  return s_color;
}

/******************************************************************************
  Webserver
******************************************************************************/

void setupWebServer()
{
  webServer.on("/", handleRoot);
  webServer.on("/commitSettings", handleCommitSettings);
  webServer.onNotFound([]() {
     if (!handleFileRead(webServer.uri()))                     // send it if it exists
      webServer.send(404, "text/plain", "404: Not Found");     // otherwise, respond with a 404 (Not Found) error
  });
  webServer.begin();
  Serial.println("HTTP server started");
}
