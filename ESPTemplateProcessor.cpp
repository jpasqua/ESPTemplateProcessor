/*
 * ESPTemplateProcessor
 *    Reads a textual template from SPIFFS, processes it, and sends the result to
 *    to a web client
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <FS.h>
#if defined(ESP32)
  #include <SPIFFS.h>
#endif
//                                  Third Party Libraries
#include <ArduinoLog.h>
//                                  Local Includes
#include "ESPTemplateProcessor.h"
//--------------- End:    Includes ---------------------------------------------


ESPTemplateProcessor::ESPTemplateProcessor(WebServer *_server) : server(_server) { }

// Provided for legacy code
bool ESPTemplateProcessor::send(const char *filePath, const ProcessorCallback& processor, char bookend) {
  auto refMapper = [processor](const String& key, String& val) -> void { val = processor(key); };

  return send(filePath, refMapper, bookend);
}

bool ESPTemplateProcessor::send(const char *filePath, const Mapper& mapper, char bookend) {
  static const char EscapeChar = '\\';
  static const uint16_t BufferSize = 100;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  if(!SPIFFS.exists(filePath)) {
    Log.warning("Cannot process %s: Does not exist.", filePath);
    return false;
  }

  File file = SPIFFS.open(filePath, "r");
  if (!file) {
    Log.warning("Cannot process %s: Failed to open.", filePath); 
    return false;
  }
#pragma GCC diagnostic pop

  // Assume that caller has already sent appropriate headers!!
  char buffer[BufferSize];
  int bufferLen = 0;
  String value;
  value.reserve(BufferSize);

  int val;
  while ((val = file.read()) != -1) {
    char ch = char(val);
    
    bool honorBookend = true;
    if (ch == EscapeChar) {   // Allows escaping the bookend
      int next = file.read();
      if (next != -1) {
        ch = char(next);
        if (ch == bookend) honorBookend = false;
      }
    }

    if ((ch == bookend) && honorBookend) {
      if (bufferLen != 0) {
        server->sendContent(buffer, bufferLen);   // Send whatever we have already accumulated...
        bufferLen = 0;                            // ...and reset the length
      }

      // Process substitution
      String key = "";
      bool found = false;
      while (!found && (val = file.read()) != -1) {
        ch = char(val);
        if (ch == bookend) { found = true; }
        else { key += ch; }
      }
      
      // Check whether we actually found a key
      if (val == -1 && !found) {
        Log.warning("Cannot process %s: Unable to parse.", filePath); 
        file.close();
        return false;
      }

      // Get substitution
      value.clear();
      mapper(key, value);
      if (!value.isEmpty()) {
        // Log.verbose("'%s' -> '%s'", key.c_str(), value.c_str()); 
        server->sendContent(value);
      } else {
        // Log.verbose("'%s' -> ''", key.c_str()); 
      }
    } else {
      if (bufferLen == BufferSize) {
        server->sendContent(buffer, bufferLen);   // Send whatever we have already accumulated...
        bufferLen = 0;                            // ...and reset the length
      }
      buffer[bufferLen++] = ch;
    }
  }

  file.close();
  if (val != -1) {  // We never hit the end of the file
    Log.warning("Failed to process %s: Didn't reach end of file", filePath);
    return false;
  }
  if (bufferLen != 0) { server->sendContent(buffer, bufferLen); }
  return true;
}

bool ESPTemplateProcessor::process(const String& theTemplate, const Mapper& mapper, char bookend, String& result) {
  static const char EscapeChar = '\\';

  String value;
  value.reserve(64);

  int val;
  int index = 0;
  int templateLength = theTemplate.length();

  while (index < templateLength) {
    val  = theTemplate[index++];
    char ch = char(val);
    
    bool honorBookend = true;
    if (ch == EscapeChar) {   // Allows escaping the bookend
      if (index < templateLength) {
        int next = theTemplate[index++];
        ch = char(next);
        if (ch == bookend) honorBookend = false;
      }
    }

    if ((ch == bookend) && honorBookend) {
      // Process substitution
      String key = "";
      bool found = false;
      while (!found && index < templateLength) {
        val = theTemplate[index++];
        ch = char(val);
        if (ch == bookend) { found = true; }
        else { key += ch; }
      }
      
      // Check whether we actually found a key
      if (index == templateLength && !found) {
        Log.warning("Cannot process %s: Unable to parse.", theTemplate.c_str()); 
        return false;
      }

      // Get substitution
      value.clear();
      mapper(key, value);
      if (!value.isEmpty()) {
        // Log.verbose("'%s' -> '%s'", key.c_str(), value.c_str()); 
        result += value;
      } else {
        // Log.verbose("'%s' -> ''", key.c_str()); 
      }
    } else {
      result += ch;
    }
  }

  if (index < templateLength) {  // We never hit the end of the template
    Log.warning("Failed to process %s: Didn't reach end of template", theTemplate.c_str());
    return false;
  }
  return true;
}
