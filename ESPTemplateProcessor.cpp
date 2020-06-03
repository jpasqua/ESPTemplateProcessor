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
#include <ESP8266WebServer.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
//                                  Local Includes
#include "ESPTemplateProcessor.h"
//--------------- End:    Includes ---------------------------------------------


ESPTemplateProcessor::ESPTemplateProcessor(ESP8266WebServer *_server) : server(_server) { }

bool ESPTemplateProcessor::send(const char *filePath, const ProcessorCallback &processor, char bookend) {
  static const char EscapeChar = '\\';


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
  static const uint16_t BufferSize = 100;
  char buffer[BufferSize];
  int bufferLen = 0;
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
      String processed = processor(key);
      //Log.verbose("Lookup '%s' received: '%s'", key.c_str(), processed.c_str()); 
      if (!processed.isEmpty()) server->sendContent(processed);
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
