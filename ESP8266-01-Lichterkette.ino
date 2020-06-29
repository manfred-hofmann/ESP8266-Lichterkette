
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <FS.h>

#define HOSTNAME "esp01_test"
#define WIFI_SETUP_TIMEOUT 180

String color1 = "#dbd854";
uint8_t color1_red;
uint8_t color1_green;
uint8_t color1_blue;

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
  WiFi.hostname(HOSTNAME);
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
}

void loop() {
  // put your main code here, to run repeatedly:
  webServer.handleClient();
  MDNS.update();
  delay(0);
}


void handleRoot()
{
  String message = "<!doctype html>"
    "<html>"
    "<head>"
    "<title>" + String(HOSTNAME) + "</title>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<meta http-equiv=\"refresh\" content=\"60\" charset=\"UTF-8\">"
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
    "<h1>" + String(HOSTNAME) + "</h1>\n";
  message += "<form action=\"/commitSettings\">";
  message += "<h2> Hallo </h2>\n";
  message += "<br>\n";
//  message += "<img src=\"/android-icon-192x192.png\">\n";
  message += "<br>\n"
      "<div class=\"example-wrap example-wrap-test-1\">\n"        
      "<p class=\"example-top\">\n"
      "<a href=\"\">Home</a>\n"
      "<br>"
      "<button type=\"submit\">Submit</button>"
      "</p>\n"
      "<canvas id=\"picker\"></canvas>\n"
      "<br>\n"
      "<input id=\"color\" name=\"co\" value=\"" + color1 + "\">\n" 
      "<script>\n"
      " new KellyColorPicker({place : 'picker', input : 'color', size : 150});\n"
      "</script>\n"
      "</div>\n";
  
  message += "</body></html>";
  webServer.send(200, "text/html", message);
}

int StrToHex(char str[])
{
  return (int) strtol(str, 0, 16);
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
  Serial.println(webServer.arg("co"));
  color1 = webServer.arg("co");
  color1.toCharArray(c_color,color1.length()+1);
  color1.substring(1, 3).toCharArray(c_color1_red,3);
  color1.substring(3, 5).toCharArray(c_color1_green,3);
  color1.substring(5, 7).toCharArray(c_color1_blue,3);
  Serial.printf("Farbcode: rot = %s gruen = %s blau = %s \r\n", c_color1_red, c_color1_green, c_color1_blue);
  color1_red = StrToHex(c_color1_red);
  color1_green = StrToHex(c_color1_green);
  color1_blue = StrToHex(c_color1_blue);
  Serial.printf("Farbcode: rot = %i gruen = %i blau = %i \r\n", color1_red, color1_green, color1_blue );
  
  callRoot();
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
