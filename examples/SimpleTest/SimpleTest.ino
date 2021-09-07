#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ESPTemplateProcessor.h>

// ESPTemplateProcessor will accept two types of functions that map
// from keys to values. The first is a function that takes a String as
// a paramter representing the key and returns a String which is the
// value, The other takes a String as a key and an "out" string param
// for the value. To test the former, comment out the line below. To
// Test the latter, leave it defined.
#define USE_REF_MAPPER

const char *ssid = "YOUR SSID";
const char *password = "YOUR PSK";
String checkedOrNot[2] = {"", "checked='checked'"};


ESP8266WebServer *server;
ESPTemplateProcessor *templateHandler;

String TitleString = "Hello World!";
String language = "en";

int port = 80;
int elevation = 22;
float lat = 12.34567;
float lng = -122.23456;
bool useBasicAuth = true;
String gmapsKey = "abc123-456-GMK";
String tzdbKey = "XO023-45t-TZDB";
String weatherKey = "0x34TH-543-WKEY";
String blynkKey = "654-AXD-890-BLYNK";
String blynkDesc = "Back Yard";
String hostname = "pm1";
String themeColor = "deep-orange";


#ifdef USE_REF_MAPPER
  void formKeyProcessor(const String &key, String& val) {
    if (key == "LAT") val.concat(lat);
    else if (key == "LNG") val.concat(lng);
    else if (key == "ELEV") val.concat(elevation);
    else if (key == "GMAPS_KEY") val = gmapsKey;
    else if (key == "TZDB_KEY") val = tzdbKey;
    else if (key == "HOSTNAME") val = hostname;
    else if (key == "SERVER_PORT") val.concat(port);
    else if (key == "BASIC_AUTH") val = checkedOrNot[useBasicAuth];
    else if (key == "WEB_UNAME") val= "admin";
    else if (key == "WEB_PASS") val = "password";  
    else if (key.startsWith("SL")) {
      if (key.endsWith(themeColor)) val = "selected";
      else val = " ";
    }
    else val = "ERROR_NO_MATCH";
  }
  void weatherFromProcessor(const String &key, String& val) {
    if (key == "WEATHER_KEY") val = weatherKey;
    else if (key == "BLYNK_KEY") val = blynkKey;
    else if (key == "BLYNK_DESC") val = blynkDesc;
    else if (key == "CITY_NAME") val = "Menlo Park";
    else if (key == "CITY") val.concat(553278);
    else if (key.startsWith("SL")) {
      if (key.endsWith(language)) val = "selected";
      else val = " ";
    }
    else val = "ERROR_NO_MATCH";
  }
#else // Don't USE_REF_MAPPER
  String formKeyProcessor(const String &key) {
    if (key == "LAT") return String(lat, 6);
    if (key == "LNG") return String(lng, 6);
    if (key == "ELEV") return String(elevation);
    if (key == "GMAPS_KEY") return gmapsKey;
    if (key == "TZDB_KEY") return tzdbKey;
    if (key == "HOSTNAME") return hostname;
    if (key == "SERVER_PORT") return String(port);
    if (key == "BASIC_AUTH") return checkedOrNot[useBasicAuth];
    if (key == "WEB_UNAME") return "admin";
    if (key == "WEB_PASS") return "password";  
    if (key.startsWith("SL")) {
      if (key.endsWith(themeColor)) return "selected";
      else return " ";
    }
    return "ERROR_NO_MATCH";
  }

  String weatherFromProcessor(const String &key) {
    if (key == "WEATHER_KEY") return weatherKey;
    if (key == "BLYNK_KEY") return blynkKey;
    if (key == "BLYNK_DESC") return blynkDesc;
    if (key == "CITY_NAME") return "Menlo Park";
    if (key == "CITY") return String(553278);
    if (key.startsWith("SL")) {
      if (key.endsWith(language)) return "selected";
      else return " ";
    }
    return "ERROR_NO_MATCH";
  }
#endif // USE_REF_MAPPER


void sendHeaders() {
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/html", "");
}

void handleWeatherForm() {
  sendHeaders();
  server->sendContent("<html><body>");
  if (templateHandler->send("/WeatherForm.html", weatherFromProcessor)) {
    Serial.println("SUCCESS");
  } else {
    Serial.println("FAIL");
    server->sendContent("Unable to process page");
  }
  server->sendContent("</body></html>");
  server->sendContent("");  // Required when using CONTENT_LENGTH_UNKNOWN
}

void handleForm() {
  sendHeaders();
  server->sendContent("<html><body>");
  if (templateHandler->send("/ConfigForm.html", formKeyProcessor)) {
    Serial.println("SUCCESS");
  } else {
    Serial.println("FAIL");
    server->sendContent("Unable to process page");
  }
  server->sendContent("</body></html>");
  server->sendContent("");  // Required when using CONTENT_LENGTH_UNKNOWN
}

void handleRoot() {
  String power = "25";
  String langTarget = "SL_fr";

  #ifdef USE_REF_MAPPER
    auto mapper = [power, langTarget](const String &key, String& val) -> void {
      if (key == "TITLE") val = TitleString;
      else if (key == "POWER") val = power;
      else if (key == langTarget) val = "selected";
    };
  #else // Don't USE_REF_MAPPER
    auto mapper = [power, langTarget](const String &key) -> String {
      if (key == "TITLE") return TitleString;
      if (key == "POWER") return (power);
      if (key == langTarget) return "selected";
      return "";
    };
  #endif

  sendHeaders();
  if (templateHandler->send("/index.html", mapper)) {
    Serial.println("SUCCESS");
  } else {
    Serial.println("FAIL");
    server->sendContent("Unable to process page");
  }
  server->sendContent("");  // Required when using CONTENT_LENGTH_UNKNOWN
}

void setup(void){
  SPIFFS.begin();

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server = new ESP8266WebServer(80);
  templateHandler = new ESPTemplateProcessor(server);
  server->on("/", handleRoot);
  server->on("/config", handleForm);
  server->on("/wconfig", handleWeatherForm);

  server->begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server->handleClient();
}