/*
 * ESPTemplateProcessor
 *    Reads a textual template from SPIFFS, processes it, and sends the result to
 *    an established web client
 *
 * NOTES:
 * o This library was forked from https://github.com/winder/ESPTemplateProcessor.
 *   There are no copyright notices or licenses that I could reproduce here.
 * o The callback signature is changed from the original libraries to make it
 *   easier to use lambdas as the process callback
 * o The implementation now allows the bookend character to be escaped by a '\'
 *   E.g., if the bookend character is '%', then for the pattern: %POWER%\%
 *   %POWER% will be mapped, but the following '\%' will be seen as a literal '%'
 * o The implementations has NOT been optimized for speed or space.
 *
 * ASSUMPTIONS
 * o SPIFFS will have been initialized by the time this class is used
 * o The caller of the send() method will have already sent headers
 *   (and perhaps other content) to the client. The caller will so finalize the
 *   the connection when all processing is complete.
 * o The caller will be using CONTENT_LENGTH_UNKNOWN
 *
 */

#ifndef ESPTemplateProcessor_h
#define ESPTemplateProcessor_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WebServer.h>
  using WebServer = ESP8266WebServer;
#elif defined(ESP32)
  #include <WebServer.h>
#else
  #error "Must be an ESP8266 or ESP32"
#endif
//                                  Third Party Libraries
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


class ESPTemplateProcessor {
public:
  typedef std::function<String(String&)> ProcessorCallback;

  /*
   * Create a new ESPTemplateProcessor object which can be used to
   * read html templates, proess them, and send them to the client.
   *
   * @param   _server The Web Server object that should be used to send content 
   */
  ESPTemplateProcessor(WebServer *_server);

  /*
   * Process and send a template which is stored in a file
   *
   * @param   filePath  Path to the file that contains the template to process/send
   * @param   processor A callback which accepts a key and returns its replacement value
   * @param   bookend   The character the denotes the beginning/end of a key;
   *                    e.g. %REPLACE_ME%
   * @return  true:     The template was successfully read, processesed, and sent
   *          false:    An error occured and processing/send was unsuccesful
   */
  bool send(const char *filePath, const ProcessorCallback &processor, char bookend = '%');

private:
  WebServer* server;
};

#endif  // ESPTemplateProcessor_h