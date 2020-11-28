# ESPTemplateProcessor
Reads a textual template from SPIFFS, processes it, and sends the result to an established web client.

### Notes

* This class was forked from the [original](https://github.com/winder/ESPTemplateProcessor) by [Will Winder](https://github.com/winder).
	* There are no copyright notices or licenses that I could reproduce here.
	* Please see the original version for additional information.
* It is designed for use with the [Arduino Arduino ESP8266 SDK](https://github.com/esp8266/) and requires SPIFFS to store templates.
* It also supports the [ESP32](https://github.com/espressif/arduino-esp32)
* The callback signature is changed from the original class to make it easier to use lambdas as the processor callback
* I have not optimized for speed or space beyond what was done in the original implementation.

### Preparation

Create one or more HTML template files that have substitution variables which are "book ended" by the character of your choice. Below is an example which uses `%` as the bookend character. It has two substitution variables, %TITLE% and %POWER%

	<html>
	  <title>%TITLE%</title>
	  <body>%POWER% \%</body>
	</html>

You must upload your template files to SPIFFS using a tool like the [ESP8266 Sketch Data Upload](https://github.com/esp8266/arduino-esp8266fs-plugin) plugin for this. For ESP32, use the [ESP32 Sketch Data Upload](https://github.com/me-no-dev/arduino-esp32fs-plugin) plugin.

### Usage

Before using this class, your code must have initialized SPIFFS. Upon each new client connection, the caller must send any required headers. It must have set the content length to `CONTENT_LENGTH_UNKNOWN`. The caller may have also sent other content directly to the client using `server->sendContent()`.

It is also the caller's responsibility tofinalize the the connection when all processing is complete.

Note that the bookend character may be escaped by preceding it with a `\`. For example, if the bookend character is `%`, then for the pattern: `%POWER%\%`, `%POWER%` will be mapped, but the following `\%` will be seen as a literal `%`.

See `examples/SimpleTest.ino` for examples.

### Lambdas

It is sometimes convenient to declare the processing callback locally to the function that is using it. For one thing, it allows for easy access to local variables of the enclosing function. Below is an example of a lambda that captures the local variable `power` and uses it in the mapping process.

	  auto mapper = [power](String &key) -> String {
	    if (key == "TITLE") return TitleString;
	    if (key == "POWER") return power;
	    return "";
	  };
