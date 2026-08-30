# KRYONOS JAVASCRIPT API REFERENCE (HARIXKERNEL)

**Context:** Embedded JavaScript environment using Duktape 2.x on ESP32 for KryonOS.

**Runtime:** Duktape 2.x
**JavaScript API Level:** 1
**Global Namespaces:** `System` and `FS`

> **Important:** KryonOS JavaScript uses an ES5-compatible syntax. Use `var` instead of `let`/`const`, traditional functions instead of arrow functions, and avoid unsupported modern syntax.

---

# 1. ENGINE & JAVASCRIPT RUNTIME

## 1.1 ECMAScript Support

KryonOS JavaScript is based on **Duktape 2.x**.

### Supported

* ECMAScript 5 / ES5.1 syntax
* Traditional functions
* Objects and prototypes
* Arrays
* JSON
* TypedArrays
* `Promise`
* `Proxy`
* `Reflect`

### Unsupported / Do Not Use

```javascript
let value = 10;
const name = "KryonOS";
var fn = function() {};
```

Use `var` and traditional functions.

The following syntax is not supported:

```javascript
var fn = () => {};
class MyClass {}
var text = `Hello ${name}`;
```

Use:

```javascript
var fn = function() {};
```

instead.

---

## 1.2 Memory

JavaScript execution is designed for low-memory ESP32 environments.

* Approximate usable heap per script: ~90 KB when WiFi is disabled.
* Avoid unnecessary dynamic allocations.
* Avoid creating large arrays repeatedly inside loops.
* `System.delay(ms)` allows the kernel to perform background garbage collection.
* Infinite loops **must** call `System.delay()` periodically.

Example:

```javascript
while (true) {
    // Application code

    System.delay(10);
}
```

---

# 2. APPLICATION LIFECYCLE & EXIT

KryonOS applications normally remain running inside their JavaScript execution loop.

For an application to remain responsive to the operating system, input APIs must be polled.

The recommended application loop is:

```javascript
while (true) {
    var touch = System.getTouch();
    var key = System.getKey();
    var character = System.getChar();

    // Application logic

    System.delay(10);
}
```

## 2.1 Touchscreen Exit

`System.getTouch()` must be called regularly by applications that use the touchscreen.

The **top-right corner** is reserved as the system/application exit area.

Touch condition:

```javascript
touch.touched &&
touch.x >= System.screenWidth() - 40 &&
touch.y <= 40
```

The KryonOS kernel recognizes this area as an OS exit trigger.

Example:

```javascript
while (true) {
    var touch = System.getTouch();

    if (touch.touched) {
        if (touch.x >= System.screenWidth() - 40 &&
            touch.y <= 40) {
            break;
        }
    }

    System.delay(10);
}
```

When the JavaScript execution reaches the end of the script or exits its main loop, control is returned to KryonOS.

---

## 2.2 Keyboard Exit

`System.getKey()` can be used to detect navigation and system keys.

`ESC` is reserved as an application exit action.

Example:

```javascript
while (true) {
    var key = System.getKey();

    if (key === "ESC") {
        break;
    }

    System.delay(10);
}
```

---

## 2.3 Character Input and Exit

`System.getChar()` translates keyboard input into characters.

Applications can use it for text-oriented input.

Example:

```javascript
while (true) {
    var character = System.getChar();

    if (character === "\x1B") {
        break;
    }

    System.delay(10);
}
```

Applications should normally use `System.getKey()` for system/navigation actions and `System.getChar()` for text input.

---

# 3. GRAPHICS & DISPLAY (`System.*`)

## `System.fillScreen(color)`

**Params:**

* `color` — integer RGB565 color

**Returns:** `undefined`

**Description:** Fills the entire physical display or active sprite with the specified color.

Example:

```javascript
System.fillScreen(0x001F);
```

---

## `System.screenWidth()`

**Params:** none

**Returns:** `integer`

**Description:** Returns the physical display width in pixels.

Example:

```javascript
var width = System.screenWidth();
```

---

## `System.screenHeight()`

**Params:** none

**Returns:** `integer`

**Description:** Returns the physical display height in pixels.

Example:

```javascript
var height = System.screenHeight();
```

Applications should use these functions instead of hard-coding screen dimensions.

Supported layouts include, for example:

```text
240 x 320
240 x 135
```

Example:

```javascript
var width = System.screenWidth();
var height = System.screenHeight();

if (height >= 200) {
    // Portrait / tall layout
} else {
    // Compact layout
}
```

---

## `System.color(r, g, b)`

**Params:**

* `r` — integer 0-255
* `g` — integer 0-255
* `b` — integer 0-255

**Returns:** `integer` RGB565 color

**Description:** Converts 24-bit RGB values to the 16-bit RGB565 format used by the TFT display.

Example:

```javascript
var red = System.color(255, 0, 0);
var blue = System.color(0, 0, 255);

System.fillScreen(blue);
```

---

## Drawing Primitives

### `System.drawPixel(x, y, color)`

### `System.drawLine(x0, y0, x1, y1, color)`

### `System.drawRect(x, y, w, h, color)`

### `System.fillRect(x, y, w, h, color)`

### `System.drawCircle(x, y, r, color)`

### `System.fillCircle(x, y, r, color)`

### `System.drawTriangle(x0, y0, x1, y1, x2, y2, color)`

### `System.fillTriangle(x0, y0, x1, y1, x2, y2, color)`

### `System.drawRoundRect(x, y, w, h, r, color)`

### `System.fillRoundRect(x, y, w, h, r, color)`

### `System.drawFastVLine(x, y, h, color)`

### `System.drawFastHLine(x, y, w, color)`

**Returns:** `undefined`

**Description:** Hardware rendering primitives. Drawing operations are directed to the physical display unless a sprite is currently bound.

---

## `System.drawBMP(path, x, y)`

**Params:**

* `path` — string
* `x` — integer
* `y` — integer

**Returns:** `boolean`

**Description:** Loads and renders a BMP image from KryonOS storage.

Supported paths:

```javascript
"/sd/image.bmp"
"/local/image.bmp"
```

Returns `true` on successful rendering and `false` if the file is missing or unsupported.

---

# 4. SPRITES (`System.*`)

Sprites provide off-screen rendering.

## `System.createSprite(w, h)`

**Params:**

* `w` — width
* `h` — height

**Returns:** `boolean`

**Description:** Allocates a sprite buffer in RAM.

The kernel attempts 16-bit color and may fall back to 8-bit color when RAM is fragmented or insufficient.

Example:

```javascript
if (System.createSprite(240, 32)) {
    System.bindSprite(true);

    // Draw into sprite

    System.bindSprite(false);
    System.pushSprite(0, 0);

    System.deleteSprite();
}
```

Large full-screen buffers should be avoided on ESP32 hardware.

A 240×320 16-bit framebuffer requires approximately 153.6 KB.

Use smaller sliced buffers whenever possible.

---

## `System.bindSprite(enable)`

**Params:**

* `enable` — boolean

**Returns:** `undefined`

**Description:** Enables or disables drawing into the active sprite.

```javascript
System.bindSprite(true);

// Drawing goes to sprite

System.bindSprite(false);

// Drawing goes directly to TFT
```

---

## `System.pushSprite(x, y)`

**Params:**

* `x`
* `y`

**Returns:** `undefined`

**Description:** Copies the active sprite to the physical TFT display.

---

## `System.deleteSprite()`

**Params:** none

**Returns:** `undefined`

**Description:** Releases the active sprite and frees its RAM.

Always call this when a sprite is no longer required.

---

# 5. TEXT & FONTS (`System.*`)

## `System.drawString(str, x, y, font)`

**Params:**

* `str` — string
* `x` — integer
* `y` — integer
* `font` — optional integer

**Returns:** `undefined`

**Description:** Draws text on the physical display or active sprite.

Default font:

```text
2
```

Available hardware font selections:

```text
1
2
4
```

Example:

```javascript
System.drawString("Hello KryonOS!", 10, 20);
System.drawString("Large Text", 10, 60, 4);
```

---

## `System.setTextColor(fg, bg)`

**Params:**

* `fg` — foreground RGB565 color
* `bg` — optional background RGB565 color

**Returns:** `undefined`

**Description:** Configures text foreground and background colors.

Example:

```javascript
System.setTextColor(0xFFFF, 0x001F);
```

---

## `System.setTextSize(size)`

**Params:**

* `size` — integer scale factor

**Returns:** `undefined`

**Description:** Changes the text rendering scale.

Example:

```javascript
System.setTextSize(2);
```

---

# 6. GPIO & HARDWARE (`System.gpio.*`)

## Constants

```javascript
System.gpio.INPUT
System.gpio.OUTPUT
System.gpio.INPUT_PULLUP

System.gpio.HIGH
System.gpio.LOW
```

---

## `System.gpio.pinMode(pin, mode)`

Configures GPIO pin mode.

```javascript
System.gpio.pinMode(2, System.gpio.OUTPUT);
```

---

## `System.gpio.digitalWrite(pin, value)`

Writes a digital HIGH/LOW value.

```javascript
System.gpio.digitalWrite(2, System.gpio.HIGH);
```

---

## `System.gpio.digitalRead(pin)`

**Returns:** `integer`

Returns:

```text
1 = HIGH
0 = LOW
```

---

## `System.gpio.analogRead(pin)`

**Returns:** `integer`

ESP32 ADC value:

```text
0 - 4095
```

---

## `System.gpio.analogWrite(pin, value)`

**Params:**

* `pin`
* `value` — 0-255

Uses hardware PWM.

---

## `System.gpio.pulseIn(pin, state, timeout)`

**Params:**

* `pin`
* `state`
* `timeout` — optional microseconds

**Returns:** `integer`

Measures hardware pulse duration in microseconds.

Default timeout:

```text
1,000,000 µs
```

---

# 7. KEYBOARD & INPUT (`System.*`)

## `System.getKey()`

**Params:** none

**Returns:** `string`

Possible values:

```text
"UP"
"DOWN"
"LEFT"
"RIGHT"
"ENTER"
"ESC"
"BACK"
"DEL"
"NONE"
```

**Description:** Reads the current keyboard/navigation state.

`ESC` is recognized as an application exit action.

Example:

```javascript
var key = System.getKey();

if (key === "ENTER") {
    // Confirm
}

if (key === "ESC") {
    // Exit application
}
```

---

## `System.isKeyPressed(keyName)`

**Params:**

* `keyName` — string

**Returns:** `boolean`

Checks whether a specific key is currently pressed.

Example:

```javascript
if (System.isKeyPressed("ENTER")) {
    // Enter is pressed
}
```

---

## `System.getKeyInput()`

**Params:** none

**Returns:** object

Structure:

```javascript
{
    key: "ENTER",
    code: 13,
    pressed: true
}
```

Fields:

* `key` — key name
* `code` — numeric key code
* `pressed` — boolean

Example:

```javascript
var input = System.getKeyInput();

if (input.pressed) {
    System.print(input.key);
}
```

---

## `System.getChar()`

**Params:** none

**Returns:** `string`

Returns a translated keyboard character.

Depending on the keyboard event, it can return:

* Normal characters
* Tab
* Newline
* Backspace
* Empty string when no character is available

Example:

```javascript
var ch = System.getChar();

if (ch !== "") {
    System.print(ch);
}
```

For application exit handling, `System.getKey()` should be preferred for detecting `ESC`.

---

## `System.prompt(msg, initialText)`

**Params:**

* `msg` — optional string
* `initialText` — optional string

**Returns:** `string`

**Description:** Opens the native KryonOS on-screen keyboard/text input interface.

Example:

```javascript
var name = System.prompt("Enter your name", "");

System.print(name);
```

The JavaScript execution is suspended while the native input interface is active.

---

## `System.getTouch()`

**Params:** none

**Returns:** object

Structure:

```javascript
{
    x: 120,
    y: 80,
    touched: true
}
```

Fields:

* `x` — X coordinate
* `y` — Y coordinate
* `touched` — boolean

When no touch is active:

```javascript
{
    x: 0,
    y: 0,
    touched: false
}
```

### System Exit Area

The top-right region is reserved for application/OS exit.

Recommended check:

```javascript
var touch = System.getTouch();

if (touch.touched &&
    touch.x >= System.screenWidth() - 40 &&
    touch.y <= 40) {

    break;
}
```

Applications should poll `System.getTouch()` continuously when running a touch interface.

---

# 8. SYSTEM UTILITIES & HARDWARE INFORMATION

## `System.millis()`

**Returns:** `integer`

Returns system uptime in milliseconds.

---

## `System.micros()`

**Returns:** `integer`

Returns system uptime in microseconds.

The 32-bit timer rolls over approximately every 71 minutes.

---

## `System.delay(ms)`

**Params:**

* `ms` — integer milliseconds

**Returns:** `undefined`

Pauses JavaScript execution.

The delay also gives the kernel an opportunity to perform garbage collection.

This should be used inside long-running loops.

Example:

```javascript
while (true) {
    // Application logic

    System.delay(10);
}
```

---

## `System.delayMicroseconds(us)`

**Params:**

* `us` — integer microseconds

**Returns:** `undefined`

Provides a high-resolution blocking delay.

Unlike `System.delay()`, this should not be relied upon for garbage collection.

---

## `System.print(msg)`

**Params:**

* `msg` — string/value

**Returns:** `undefined`

Prints diagnostic information to the USB Serial Monitor.

Default baud rate:

```text
115200
```

Example:

```javascript
System.print("Application started");
```

---

## `System.getTemperature()`

**Returns:** `float`

Returns ESP32 internal temperature in degrees Celsius when supported.

---

## `System.hasTemperatureSensor()`

**Returns:** `boolean`

Returns `true` when the installed ESP32 chip supports the internal temperature sensor.

---

## `System.getInfo()`

**Returns:** object

Structure:

```javascript
{
    totalRAM: 0,
    freeRAM: 0,
    minFreeRAM: 0,
    maxAllocRAM: 0,
    cpuFreqMHz: 0,
    chipModel: "",
    chipCores: 0,
    chipRevision: 0,
    flashSize: 0,
    uptimeMs: 0
}
```

Fields:

* `totalRAM`
* `freeRAM`
* `minFreeRAM`
* `maxAllocRAM`
* `cpuFreqMHz`
* `chipModel`
* `chipCores`
* `chipRevision`
* `flashSize`
* `uptimeMs`

Example:

```javascript
var info = System.getInfo();

System.print("Free RAM: " + info.freeRAM);
System.print("CPU: " + info.cpuFreqMHz + " MHz");
```

---

## `System.restart()`

**Params:** none

**Returns:** `undefined`

Immediately reboots the ESP32.

```javascript
System.restart();
```

---

# 9. TIME, DATE & NETWORK (`System.*`)

## `System.getTime()`

**Returns:** `string`

Returns the OS-formatted local time according to the configured 12/24-hour preference.

---

## `System.getSeconds()`

**Returns:** `integer`

Returns current seconds:

```text
0 - 59
```

---

## `System.getDate()`

**Returns:** `string`

Returns local date in OS format.

Example:

```text
15/06/2026
```

---

## `System.getYear()`

**Returns:** `integer`

Returns four-digit year.

---

## `System.getMonth()`

**Returns:** `integer`

Returns month:

```text
1 - 12
```

---

## `System.getDay()`

**Returns:** `integer`

Returns day of month:

```text
1 - 31
```

---

## `System.getTimezone()`

**Returns:** `string`

Returns configured timezone.

---

## `System.getOSVersion()`

**Returns:** `string`

Returns KryonOS version.

Example:

```javascript
var version = System.getOSVersion();
```

---

## `System.getAPILevel()`

**Returns:** `integer`

Returns current KryonOS JavaScript API level.

---

## `System.getIPAddress()`

**Returns:** `string`

Returns the ESP32 local IP address when WiFi is connected.

---

## `System.isWiFiActive()`

**Returns:** `boolean`

Returns `true` when WiFi is active/connected.

---

# 10. FILE SYSTEM (`FS.*`)

KryonOS provides a unified filesystem interface.

Storage prefixes:

```text
/local/
```

Internal flash storage.

```text
/sd/
```

SD card storage.

---

## `FS.exists(path)`

**Returns:** `boolean`

Checks whether a file or directory exists.

---

## `FS.readTextFile(path)`

**Returns:** `string` or `null`

Reads an entire text file into memory.

Use only for reasonably small files.

---

## `FS.writeTextFile(path, content)`

**Returns:** `boolean`

Writes text content to a file.

Existing content is replaced.

---

## `FS.appendTextFile(path, content)`

**Returns:** `boolean`

Appends text to an existing file.

---

## `FS.deleteFile(path)`

**Returns:** `boolean`

Deletes a file.

---

## `FS.renameFile(from, to)`

**Returns:** `boolean`

Renames or moves a file within the same storage partition.

---

## `FS.listDir(path)`

**Returns:** `Array`

Returns an array of file/directory paths.

JavaScript arrays are **0-indexed**.

Example:

```javascript
var files = FS.listDir("/local");

for (var i = 0; i < files.length; i++) {
    System.print(files[i]);
}
```

---

## `FS.mkdir(path)`

**Returns:** `boolean`

Creates a directory.

---

## `FS.rmdir(path)`

**Returns:** `boolean`

Removes an empty directory.

---

## `FS.isDirectory(path)`

**Returns:** `boolean`

Checks whether a path is a directory.

---

## `FS.isFile(path)`

**Returns:** `boolean`

Checks whether a path is a file.

---

## `FS.getFileSize(path)`

**Returns:** `integer`

Returns file size in bytes.

---

## `FS.getTotalSpace(drive)`

**Returns:** `integer`

Returns total storage capacity in bytes.

Valid drives:

```javascript
"/local"
"/sd"
```

---

## `FS.getUsedSpace(drive)`

**Returns:** `integer`

Returns used storage space in bytes.

---

## `FS.getFreeSpace(drive)`

**Returns:** `integer`

Returns available storage space in bytes.

---

## `FS.getFileMD5(path)`

**Returns:** `string`

Returns the MD5 hash of the specified file.

Example result:

```text
d41d8cd98f00b204e9800998ecf8427e
```

---

## `FS.mountSD()`

**Returns:** `boolean`

Mounts the SD card filesystem.

Returns `true` when mounting succeeds.

---

## `FS.unmountSD()`

**Returns:** `boolean` or `undefined`

Unmounts the SD card filesystem.

---

# 11. BINARY FILES (`FS.*`)

KryonOS JavaScript supports raw binary file access.

Binary data should be treated as byte-oriented data rather than text.

## `FS.readBinaryFile(path)`

**Params:**

* `path` — string

**Returns:**

* binary data/string when successful
* `null` when the file does not exist or cannot be read

**Description:** Reads the complete file as raw binary data.

Example:

```javascript
var data = FS.readBinaryFile("/local/data.bin");

if (data !== null) {
    System.print("Binary file loaded");
}
```

An existing empty file returns an empty string.

The returned data length corresponds to the number of bytes read.

> Avoid loading large binary files into JavaScript memory because the JavaScript heap is limited.

---

## `FS.writeBinaryFile(path, data)`

**Params:**

* `path` — string
* `data` — binary string/data

**Returns:** `boolean`

Writes raw binary data to storage.

Example:

```javascript
var data = "\x01\x02\x03\xFF";

var success = FS.writeBinaryFile(
    "/local/test.bin",
    data
);
```

Empty data is not written and returns `false`.

---

# 12. RESPONSIVE DISPLAY DESIGN

Applications should not assume that the display is always 240×320.

Use:

```javascript
var width = System.screenWidth();
var height = System.screenHeight();
```

Example:

```javascript
var width = System.screenWidth();
var height = System.screenHeight();

if (height >= 200) {
    // 240x320-style layout
    System.drawString("Tall Display", 10, 20);
} else {
    // 240x135-style layout
    System.drawString("Wide/Compact Display", 10, 20);
}
```

For centered text:

```javascript
var width = System.screenWidth();

var text = "KryonOS";
var textWidth = 7 * text.length;

var x = (width - textWidth) / 2;

System.drawString(text, x, 20);
```

---

# 13. RECOMMENDED APPLICATION TEMPLATE

A basic KryonOS JavaScript application should follow this structure:

```javascript
var width = System.screenWidth();
var height = System.screenHeight();

var BLUE = 0x001F;
var WHITE = 0xFFFF;
var RED = 0xF800;

System.fillScreen(BLUE);
System.setTextColor(WHITE, BLUE);

System.drawString("KryonOS JavaScript", 10, 15);

if (height >= 200) {
    System.drawString("240x320 Layout", 10, 50);
} else {
    System.drawString("240x135 Layout", 10, 50);
}

// Application loop
while (true) {

    // Touch must be polled
    var touch = System.getTouch();

    if (touch.touched &&
        touch.x >= width - 40 &&
        touch.y <= 40) {

        break;
    }

    // Keyboard must also be polled
    var key = System.getKey();

    if (key === "ESC") {
        break;
    }

    // Character input
    var character = System.getChar();

    // Application logic can process character here.

    // Allows GC and prevents CPU starvation
    System.delay(10);
}

// Application cleanup
System.fillScreen(BLUE);
```

---

# 14. API SUMMARY

## Display

```text
System.fillScreen()
System.screenWidth()
System.screenHeight()
System.color()
System.drawPixel()
System.drawLine()
System.drawRect()
System.fillRect()
System.drawCircle()
System.fillCircle()
System.drawTriangle()
System.fillTriangle()
System.drawRoundRect()
System.fillRoundRect()
System.drawFastVLine()
System.drawFastHLine()
System.drawBMP()
```

## Sprites

```text
System.createSprite()
System.deleteSprite()
System.pushSprite()
System.bindSprite()
```

## Text

```text
System.drawString()
System.setTextColor()
System.setTextSize()
```

## Input

```text
System.getKey()
System.isKeyPressed()
System.getKeyInput()
System.getChar()
System.prompt()
System.getTouch()
```

## GPIO

```text
System.gpio.pinMode()
System.gpio.digitalWrite()
System.gpio.digitalRead()
System.gpio.analogRead()
System.gpio.analogWrite()
System.gpio.pulseIn()
```

## System

```text
System.millis()
System.micros()
System.delay()
System.delayMicroseconds()
System.print()
System.getTemperature()
System.hasTemperatureSensor()
System.getInfo()
System.restart()
```

## Time & Network

```text
System.getTime()
System.getSeconds()
System.getDate()
System.getYear()
System.getMonth()
System.getDay()
System.getTimezone()
System.getOSVersion()
System.getAPILevel()
System.getIPAddress()
System.isWiFiActive()
```

## File System

```text
FS.exists()
FS.readTextFile()
FS.writeTextFile()
FS.appendTextFile()
FS.deleteFile()
FS.renameFile()
FS.listDir()
FS.mkdir()
FS.rmdir()
FS.isDirectory()
FS.isFile()
FS.getFileSize()
FS.getTotalSpace()
FS.getUsedSpace()
FS.getFreeSpace()
FS.getFileMD5()
FS.mountSD()
FS.unmountSD()
FS.readBinaryFile()
FS.writeBinaryFile()
```

---

# 15. IMPORTANT APPLICATION RULES

1. **Always poll input in long-running applications.**
2. Applications using touch should call `System.getTouch()` continuously.
3. Applications using keyboard should call `System.getKey()` continuously.
4. `System.getChar()` is available for character-oriented keyboard input.
5. `ESC` is the standard keyboard exit action.
6. The top-right touchscreen area is reserved for exiting the application.
7. Long-running loops must contain `System.delay(10)` or a similar delay.
8. Do not allocate unnecessarily large JavaScript arrays or strings.
9. Avoid full-screen sprites on memory-constrained ESP32 devices.
10. Always release sprites using `System.deleteSprite()` when finished.
11. Use `System.screenWidth()` and `System.screenHeight()` for responsive layouts.
12. Prefer sliced rendering for large graphics.
13. Use `/local/` for internal storage and `/sd/` for SD card storage.
14. Use binary file APIs for non-text files.
15. Use ES5-compatible JavaScript syntax for maximum KryonOS compatibility.

---

**Document Version:** 2.0
**Target:** KryonOS JavaScript Runtime / HarixKernel
**Platform:** ESP32
**Runtime:** Duktape 2.x
**API Level:** 1
