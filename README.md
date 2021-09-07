# ESPTemplateProcessor
Reads a textual template from SPIFFS, processes it, and sends the result to an established web client.

### Notes

* This class was forked from the [original](https://github.com/winder/ESPTemplateProcessor) by [Will Winder](https://github.com/winder).
	* There are no copyright notices or licenses that I could reproduce here.
	* Please see the original version for additional information.
* It is designed for use with the [Arduino Arduino ESP8266 SDK](https://github.com/esp8266/) and requires SPIFFS to store templates.
* It also supports the [ESP32](https://github.com/espressif/arduino-esp32)
* The callback signature is changed from the original class to make it easier to use lambdas as the processor callback.
* I have not optimized for speed or space beyond what was done in the original implementation.

### Preparation

Create one or more HTML template files that have substitution variables which are "book ended" by the character of your choice. Below is an example which uses `%` as the bookend character. It has two substitution variables, %TITLE% and %POWER%

	<html>
	  <title>%TITLE%</title>
	  <body>%POWER% \%</body>
	</html>

You must upload your template files to SPIFFS using a tool like the [ESP8266 Sketch Data Upload](https://github.com/esp8266/arduino-esp8266fs-plugin) plugin for this. For ESP32, use the [ESP32 Sketch Data Upload](https://github.com/me-no-dev/arduino-esp32fs-plugin) plugin.

### Processing

#### Template Files

A template file, often an HTML file, will typically contain keys that will be replaced by the processor before it sends out the template content. A key is represented by a string enclosed by a matching pair of "bookend" characters. The default bookend is `'%'`. Consider a template that contains:

`Hello %WHO%`

Before the content is sent, %WHO% will be replaced by a new string - see below for details. Sometimes you may need to use your bookend character as a content character. For example:

`Power Remaining: %POWER% %`

In this case you want the final `%` to be part of the output. To achieve this you can escape the bookend by preceding it with a `\`. For the example above, we would write:

`Power Remaining: %POWER% \%`

In this case `%POWER%` will be mapped, but the following `\%` will be seen as a literal `%`, resulting in output like:

Power Remaining: 75 %


#### Matching

Before the content is sent, all keys will be replaced. It is up to the client of `ESPTemplateProcessor` to decide what to replace them with. It does so by providing a mapping function which maps a key, like `%WHO%`, to a value. There are two variants of the mapping function that can be used. The first accepts a key and returns a value:

`  typedef std::function<String(const String&)> ProcessorCallback;`

The other also takes a key, but it returns its value via an 'out' parameter:

`  typedef std::function<void(const String&, String&)> Mapper;`

The former is written in terms of the latter. The latter helps to avoid unintended String object allocations and should be preferred. See `examples/SimpleTest.ino` for examples of both approaches.

### Usage

Before using this class, your code must have initialized SPIFFS. Upon each new client connection, the caller must send any required headers. It must have set the content length to `CONTENT_LENGTH_UNKNOWN`. The caller may have also sent other content directly to the client using `server->sendContent()`.

It is also the caller's responsibility to finalize the the connection when all processing is complete.


### Lambdas

It is sometimes convenient to declare the processing callback locally to the function that is using it. For one thing, it allows for easy access to local variables of the enclosing function. Below is an example of a lambda that captures the local variable `power` and uses it in the mapping process.

	  auto mapper = [power](const String &key, String& val) -> void {
	    if (key == "TITLE") val = TitleString;
	    else if (key == "POWER") val = power;
	  };
