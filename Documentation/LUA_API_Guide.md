KRYONOS LUA BINDINGS API REFERENCE (HARIXKERNEL)
Context: Embedded Lua 5.1 environment on ESP32 for KryonOS. All functions are registered under the global "System" namespace. Arrays and lists are 1-indexed.

--- 1. GRAPHICS & DISPLAY ---
- System.fillScreen(color)
  Params: color (uint32_t RGB565)
  Returns: none
  Desc: Fills the entire screen or active sprite with color.

- System.screenWidth() / System.screenHeight()
  Params: none
  Returns: integer (width/height pixels)
  Desc: Returns physical screen dimensions.

- System.color(r, g, b)
  Params: r (0-255), g (0-255), b (0-255)
  Returns: integer (RGB565 uint16 color)
  Desc: Converts RGB components into 16-bit color format.

- System.drawPixel(x, y, color)
- System.drawLine(x0, y0, x1, y1, color)
- System.drawRect(x, y, w, h, color) / System.fillRect(x, y, w, h, color)
- System.drawCircle(x, y, r, color) / System.fillCircle(x, y, r, color)
- System.drawTriangle(x0, y0, x1, y1, x2, y2, color) / System.fillTriangle(...)
- System.drawRoundRect(x, y, w, h, r, color) / System.fillRoundRect(...)
- System.drawFastVLine(x, y, h, color) / System.drawFastHLine(x, y, w, color)
  Params: Coordinates, dimensions, color
  Returns: none
  Desc: Primitives supporting direct screen drawing or active sprite rendering.

- System.drawBMP(path, x, y)
  Params: path (string, e.g., "/sd/img.bmp" or "/local/img.bmp"), x, y
  Returns: boolean (success)
  Desc: Parses and renders uncompressed/compressed BMP files from SD or LittleFS.

--- 2. SPRITES ---
- System.createSprite(w, h)
  Params: w (width), h (height)
  Returns: boolean (success)
  Desc: Allocates RAM for a sprite (tries 16-bit, falls back to 8-bit).

- System.deleteSprite()
  Params: none
  Returns: none
  Desc: Frees sprite memory.

- System.pushSprite(x, y)
  Params: x, y
  Returns: none
  Desc: Pushes active sprite buffer to display at coordinates.

- System.bindSprite(enable)
  Params: boolean enable
  Returns: none
  Desc: Toggles drawing operations between direct screen and active sprite.

--- 3. TEXT & FONTS ---
- System.drawString(str, x, y, font)
  Params: str (string), x, y, font (optional integer, default 2)
  Returns: none
  Desc: Draws text string on screen/sprite.

- System.setTextColor(fg, bg)
  Params: fg (uint32_t color), bg (optional uint32_t color for background)
  Returns: none
  Desc: Sets text foreground and optional background color.

- System.setTextSize(size)
  Params: size (integer scale factor)
  Returns: none
  Desc: Scales text size.

--- 4. GPIO & HARDWARE ---
- System.pinMode(pin, mode)
- System.digitalWrite(pin, val)
- System.digitalRead(pin)
- System.analogRead(pin)
- System.analogWrite(pin, val)
- System.pulseIn(pin, state, timeout)
  Params: Standard Arduino pin control parameters (pin, mode, value, timeout)
  Returns: Digital/Analog value or pulse duration.

--- 5. KEYBOARD & INPUT ---
- System.getKey()
  Params: none
  Returns: string ("UP", "DOWN", "LEFT", "RIGHT", "ENTER", "ESC", "BACK", "DEL", "NONE")
  Desc: Reads keyboard state. ESC triggers OS exit.

- System.isKeyPressed(keyName)
  Params: keyName (string)
  Returns: boolean
  Desc: Checks if a specific key is currently active.

- System.getKeyInput()
  Params: none
  Returns: table {key: string, code: integer, pressed: boolean}
  Desc: Returns comprehensive keyboard event data.

- System.getChar()
  Params: none
  Returns: string (character or empty string)
  Desc: Translates key press into typed character, tab, newline, or backspace.

- System.prompt(msg, initialText)
  Params: msg (optional string prompt), initialText (optional string)
  Returns: string (user input result)
  Desc: Opens on-screen text input dialog/keyboard.

- System.getTouch()
  Params: none
  Returns: table {x: integer, y: integer, touched: boolean}
  Desc: Reads touchscreen coordinates. Top-right corner triggers OS exit.

--- 6. SYSTEM UTILITIES & INFO ---
- System.millis() / System.micros() -> returns integer timestamp
- System.delay(ms) / System.delayMicroseconds(us) -> pauses execution (delay triggers GC)
- System.print(msg) -> prints to Serial monitor
- System.getTemperature() -> returns float chip temp
- System.hasTemperatureSensor() -> returns boolean
- System.getInfo() -> returns table {totalRAM, freeRAM, minFreeRAM, maxAllocRAM, cpuFreqMHz, chipModel, chipCores, chipRevision, flashSize, uptimeMs}
- System.restart() -> reboots ESP32

--- 7. TIME & NETWORK ---
- System.getTime() -> returns formatted time string
- System.getSeconds() -> returns integer seconds
- System.getDate() -> returns formatted date string
- System.getYear() / System.getMonth() / System.getDay() -> returns integer date components
- System.getTimezone() -> returns string timezone
- System.getOSVersion() -> returns string version
- System.getAPILevel() -> returns integer API level
- System.getIPAddress() -> returns string IP address
- System.isWiFiActive() -> returns boolean

--- 8. FILE SYSTEM ---
- System.readTextFile(path) -> returns string content or nil
- System.writeTextFile(path, content) -> returns boolean
- System.appendTextFile(path, content) -> returns boolean
- System.deleteFile(path) -> returns boolean
- System.fileExists(path) -> returns boolean
- System.listDir(path) -> returns table (1-indexed array of strings)
- System.renameFile(from, to) -> returns boolean
- System.mkdir(path) / System.rmdir(path) -> returns boolean
- System.isDirectory(path) / System.isFile(path) -> returns boolean
- System.getFileSize(path) -> returns integer bytes
- System.getTotalSpace(drive) / System.getUsedSpace(drive) / System.getFreeSpace(drive) -> returns integer bytes
- System.getFileMD5(path) -> returns string MD5 hash
- System.mountSD() -> returns boolean
- System.unmountSD() -> unmounts storage