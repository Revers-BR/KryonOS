# KRYONOS LUA BINDINGS API REFERENCE (HARIXKERNEL)

Context: Embedded Lua 5.1 environment on ESP32 for KryonOS.  
Global Namespaces: `System` and `FS`.  
Arrays and lists are 1-indexed.

---

## 1. GRAPHICS & DISPLAY (`System.*`)

* `System.fillScreen(color)`
  * **Params:** `color` (uint32_t RGB565)
  * **Returns:** `none`
  * **Desc:** Fills the entire screen or active sprite with color.

* `System.screenWidth()` / `System.screenHeight()`
  * **Params:** `none`
  * **Returns:** `integer` (width/height pixels)
  * **Desc:** Returns physical screen dimensions.

* `System.color(r, g, b)`
  * **Params:** `r` (0-255), `g` (0-255), `b` (0-255)
  * **Returns:** `integer` (RGB565 uint16 color)
  * **Desc:** Converts RGB components into 16-bit color format.

* `System.drawPixel(x, y, color)`
* `System.drawLine(x0, y0, x1, y1, color)`
* `System.drawRect(x, y, w, h, color)` / `System.fillRect(x, y, w, h, color)`
* `System.drawCircle(x, y, r, color)` / `System.fillCircle(x, y, r, color)`
* `System.drawTriangle(x0, y0, x1, y1, x2, y2, color)` / `System.fillTriangle(x0, y0, x1, y1, x2, y2, color)`
* `System.drawRoundRect(x, y, w, h, r, color)` / `System.fillRoundRect(x, y, w, h, r, color)`
* `System.drawFastVLine(x, y, h, color)` / `System.drawFastHLine(x, y, w, color)`
  * **Params:** Coordinates, dimensions, color
  * **Returns:** `none`
  * **Desc:** Rendering primitives for active sprite or direct screen.

* `System.drawBMP(path, x, y)`
  * **Params:** `path` (string, e.g., `"/sd/img.bmp"` or `"/local/img.bmp"`), `x`, `y`
  * **Returns:** `boolean` (success)
  * **Desc:** Parses and renders uncompressed/compressed BMP files from storage.

---

## 2. SPRITES (`System.*`)

* `System.createSprite(w, h)`
  * **Params:** `w` (width), `h` (height)
  * **Returns:** `boolean` (success)
  * **Desc:** Allocates RAM for a sprite (tries 16-bit, falls back to 8-bit).

* `System.deleteSprite()`
  * **Params:** `none`
  * **Returns:** `none`
  * **Desc:** Frees memory allocated for active sprite.

* `System.pushSprite(x, y)`
  * **Params:** `x`, `y`
  * **Returns:** `none`
  * **Desc:** Pushes active sprite buffer to display at target coordinates.

* `System.bindSprite(enable)`
  * **Params:** `enable` (boolean)
  * **Returns:** `none`
  * **Desc:** Toggles drawing operations between direct screen and active sprite.

---

## 3. TEXT & FONTS (`System.*`)

* `System.drawString(str, x, y, font)`
  * **Params:** `str` (string), `x`, `y`, `font` (optional integer, default 2)
  * **Returns:** `none`
  * **Desc:** Draws text string on screen/sprite.

* `System.setTextColor(fg, bg)`
  * **Params:** `fg` (uint32_t color), `bg` (optional uint32_t color for background)
  * **Returns:** `none`
  * **Desc:** Sets text foreground and optional background color.

* `System.setTextSize(size)`
  * **Params:** `size` (integer scale factor)
  * **Returns:** `none`
  * **Desc:** Scales text rendering size.

---

## 4. GPIO & HARDWARE (`System.gpio.*`)

* `System.gpio.pinMode(pin, mode)`
* `System.gpio.digitalWrite(pin, val)`
* `System.gpio.digitalRead(pin)`
* `System.gpio.analogRead(pin)`
* `System.gpio.analogWrite(pin, val)`
* `System.gpio.pulseIn(pin, state, timeout)`
  * **Params:** Standard Arduino pin control parameters (`pin`, `mode`, `val`, `state`, `timeout`)
  * **Returns:** Digital/Analog value or pulse duration integer.

---

## 5. KEYBOARD & INPUT (`System.*`)

* `System.getKey()`
  * **Params:** `none`
  * **Returns:** `string` (`"UP"`, `"DOWN"`, `"LEFT"`, `"RIGHT"`, `"ENTER"`, `"ESC"`, `"BACK"`, `"DEL"`, `"NONE"`)
  * **Desc:** Reads keyboard state. `ESC` triggers OS exit.

* `System.isKeyPressed(keyName)`
  * **Params:** `keyName` (string)
  * **Returns:** `boolean`
  * **Desc:** Checks if a specific key is currently active.

* `System.getKeyInput()`
  * **Params:** `none`
  * **Returns:** `table` `{key: string, code: integer, pressed: boolean}`
  * **Desc:** Returns comprehensive keyboard event data structure.

* `System.getChar()`
  * **Params:** `none`
  * **Returns:** `string` (character or empty string)
  * **Desc:** Translates key press into typed character, tab, newline, or backspace.

* `System.prompt(msg, initialText)`
  * **Params:** `msg` (optional string), `initialText` (optional string)
  * **Returns:** `string` (user input result)
  * **Desc:** Opens on-screen text input dialog/keyboard interface.

* `System.getTouch()`
  * **Params:** `none`
  * **Returns:** `table` `{x: integer, y: integer, touched: boolean}`
  * **Desc:** Reads touchscreen coordinates. Top-right corner triggers OS exit.

---

## 6. SYSTEM UTILITIES & INFO (`System.*`)

* `System.millis()` / `System.micros()` -> Returns integer timestamp.
* `System.delay(ms)` / `System.delayMicroseconds(us)` -> Pauses execution (delay triggers GC).
* `System.print(msg)` -> Prints string to Serial monitor.
* `System.getTemperature()` -> Returns float chip temperature (°C).
* `System.hasTemperatureSensor()` -> Returns boolean.
* `System.getInfo()` -> Returns table `{totalRAM, freeRAM, minFreeRAM, maxAllocRAM, cpuFreqMHz, chipModel, chipCores, chipRevision, flashSize, uptimeMs}`.
* `System.restart()` -> Reboots ESP32 microcontroller.

---

## 7. TIME & NETWORK (`System.*`)

* `System.getTime()` -> Returns formatted time string.
* `System.getSeconds()` -> Returns integer seconds.
* `System.getDate()` -> Returns formatted date string.
* `System.getYear()` / `System.getMonth()` / `System.getDay()` -> Returns integer date components.
* `System.getTimezone()` -> Returns string timezone.
* `System.getOSVersion()` -> Returns string OS version.
* `System.getAPILevel()` -> Returns integer API level.
* `System.getIPAddress()` -> Returns string IP address.
* `System.isWiFiActive()` -> Returns boolean.

---

## 8. FILE SYSTEM (`FS.*`)

* `FS.readTextFile(path)` -> Returns string content or `nil`.
* `FS.writeTextFile(path, content)` -> Returns boolean.
* `FS.appendTextFile(path, content)` -> Returns boolean.
* `FS.deleteFile(path)` -> Returns boolean.
* `FS.renameFile(from, to)` -> Returns boolean.
* `FS.exists(path)` -> Returns boolean.
* `FS.listDir(path)` -> Returns table (1-indexed array of string paths/files).
* `FS.mkdir(path)` / `FS.rmdir(path)` -> Returns boolean.
* `FS.isDirectory(path)` / `FS.isFile(path)` -> Returns boolean.
* `FS.getFileSize(path)` -> Returns integer bytes.
* `FS.getTotalSpace(drive)` / `FS.getUsedSpace(drive)` / `FS.getFreeSpace(drive)` -> Returns integer bytes.
* `FS.getFileMD5(path)` -> Returns string MD5 hash.
* `FS.mountSD()` -> Returns boolean.
* `FS.unmountSD()` -> Unmounts SD card storage.