#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ESPTemplateProcessor.h>

const char *ssid = "NoRoom";
const char *password = "feedface";
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

  auto mapper = [power, langTarget](String &key) -> String {
    if (key == "TITLE") return TitleString;
    if (key == "POWER") return (power);
    if (key == langTarget) return "selected";
    return "";
  };

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