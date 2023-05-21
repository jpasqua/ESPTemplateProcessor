#include <ESPTemplateProcessor.h>

ESPTemplateProcessor *templateHandler;

void weatherMapper(const String &key, String& val) {
  if (key == "CITY") val = "Carmel";
  else if (key == "TEMP") val = "54";
  else if (key == "WIND") val = "4NNE";
  else if (key == "DESC") val = "Cloudy";
  else val = "";
}

void setup(void){
  Serial.begin(115200); while (!Serial) delay(50); Serial.println();
  templateHandler = new ESPTemplateProcessor(nullptr);
  String theTemplate = "%CITY% %TEMP%F %WIND% %DESC%";
  String result;
  bool success = templateHandler->process(theTemplate, weatherMapper, '%', result);
  if (success) {
    Serial.print("Result = "); Serial.println(result);
  } else {
    Serial.println("Failed to process template");
  }
}

void loop(void){
}