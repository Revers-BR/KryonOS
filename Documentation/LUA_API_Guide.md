# KRYONOS LUA BINDINGS API REFERENCE (HARIXKERNEL)

**Context:** Embedded Lua 5.1 environment on ESP32 for KryonOS.

**Runtime:** Lua 5.1
**API Level:** 1
**Global Namespaces:** `System` and `FS`

> **Important:** Lua arrays/lists returned by KryonOS are **1-indexed**, following the standard Lua convention.

---

# 1. APPLICATION LIFECYCLE & EXIT

KryonOS Lua applications may run continuously inside an application loop.

Applications that remain active must continuously poll the available input APIs so the kernel can process user interaction and application exit requests.

Recommended structure:

```lua
while true do
    local touch = System.getTouch()
    local key = System.getKey()
    local char = System.getChar()

    -- Application logic

    System.delay(10)
end
```

## 1.1 Touchscreen Exit

`System.getTouch()` must be polled continuously by applications using the touchscreen.

The **top-right corner of the display is reserved for system/application exit**.

Recommended detection:

```lua
local touch = System.getTouch()

if touch.touched and
   touch.x >= System.screenWidth() - 40 and
   touch.y <= 40 then

    break
end
```

The KryonOS kernel recognizes this region as an exit trigger.

---

## 1.2 Keyboard Exit

`System.getKey()` provides keyboard/navigation state.

The `ESC` key is the standard keyboard application-exit action.

```lua
local key = System.getKey()

if key == "ESC" then
    break
end
```

---

## 1.3 Character Input

`System.getChar()` provides character-oriented keyboard input.

Use `System.getKey()` for navigation/system actions and `System.getChar()` for text input.

```lua
local char = System.getChar()

if char ~= "" then
    System.print(char)
end
```

---

## 1.4 Required Loop Delay

Long-running loops must periodically call:

```lua
System.delay(10)
```

This prevents CPU starvation and allows the kernel to perform garbage collection.

Avoid:

```lua
while true do
    -- Infinite loop without delay
end
```

Prefer:

```lua
while true do
    -- Application logic

    System.delay(10)
end
```

---

# 2. GRAPHICS & DISPLAY (`System.*`)

## `System.fillScreen(color)`

**Params:**

* `color` — RGB565 integer

**Returns:** `none`

**Description:** Fills the entire physical display or active sprite with the specified color.

```lua
System.fillScreen(0x001F)
```

---

## `System.screenWidth()`

**Params:** none

**Returns:** `integer`

**Description:** Returns the physical display width in pixels.

---

## `System.screenHeight()`

**Params:** none

**Returns:** `integer`

**Description:** Returns the physical display height in pixels.

Applications should use these functions instead of assuming a fixed resolution.

Example supported layouts:

```text
240 x 320
240 x 135
```

Responsive example:

```lua
local width = System.screenWidth()
local height = System.screenHeight()

if height >= 200 then
    -- Tall display layout
else
    -- Compact display layout
end
```

---

## `System.color(r, g, b)`

**Params:**

* `r` — integer 0-255
* `g` — integer 0-255
* `b` — integer 0-255

**Returns:** `integer`

**Description:** Converts 24-bit RGB values into RGB565.

```lua
local blue = System.color(0, 0, 255)

System.fillScreen(blue)
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

**Returns:** `none`

**Description:** Hardware rendering primitives. Drawing is performed directly on the physical TFT unless a sprite is currently bound.

---

## `System.drawBMP(path, x, y)`

**Params:**

* `path` — string
* `x` — integer
* `y` — integer

**Returns:** `boolean`

**Description:** Loads and renders a BMP image from KryonOS storage.

Examples:

```lua
System.drawBMP("/local/image.bmp", 0, 0)
System.drawBMP("/sd/image.bmp", 20, 20)
```

Returns `true` on success and `false` when the file cannot be loaded or is unsupported.

---

# 3. SPRITES (`System.*`)

Sprites provide off-screen rendering for reduced flicker and frame-based rendering.

## `System.createSprite(w, h)`

**Params:**

* `w` — width
* `h` — height

**Returns:** `boolean`

**Description:** Allocates RAM for an active sprite.

The kernel attempts 16-bit color and may fall back to 8-bit color when contiguous RAM is insufficient.

Example:

```lua
if System.createSprite(240, 32) then
    System.bindSprite(true)

    -- Draw sprite

    System.bindSprite(false)
    System.pushSprite(0, 0)

    System.deleteSprite()
end
```

### Memory Recommendation

Avoid unnecessarily large sprites.

A 240×320 framebuffer at 16-bit color requires approximately:

```text
153.6 KB
```

Use smaller slices whenever possible, for example:

```text
240 x 16
240 x 32
240 x 64
```

---

## `System.bindSprite(enable)`

**Params:**

* `enable` — boolean

**Returns:** `none`

**Description:** Enables or disables rendering into the active sprite.

```lua
System.bindSprite(true)

-- Drawing goes to sprite

System.bindSprite(false)

-- Drawing goes directly to TFT
```

---

## `System.pushSprite(x, y)`

**Params:**

* `x`
* `y`

**Returns:** `none`

**Description:** Pushes the active sprite buffer to the physical display.

---

## `System.deleteSprite()`

**Params:** none

**Returns:** `none`

**Description:** Releases the active sprite and frees allocated RAM.

Always call this when finished with a sprite.

---

# 4. TEXT & FONTS (`System.*`)

## `System.drawString(str, x, y, font)`

**Params:**

* `str` — string
* `x` — integer
* `y` — integer
* `font` — optional integer, default `2`

**Returns:** `none`

**Description:** Draws text on the physical display or active sprite.

Supported hardware font selections:

```text
1
2
4
```

Example:

```lua
System.drawString("Hello KryonOS!", 10, 20)
System.drawString("Large Text", 10, 60, 4)
```

---

## `System.setTextColor(fg, bg)`

**Params:**

* `fg` — foreground RGB565 color
* `bg` — optional background RGB565 color

**Returns:** `none`

**Description:** Sets foreground and optional background text color.

```lua
System.setTextColor(0xFFFF, 0x001F)
```

---

## `System.setTextSize(size)`

**Params:**

* `size` — integer scale factor

**Returns:** `none`

**Description:** Changes text rendering scale.

```lua
System.setTextSize(2)
```

---

# 5. GPIO & HARDWARE (`System.gpio.*`)

## Constants

```lua
System.gpio.INPUT
System.gpio.OUTPUT
System.gpio.INPUT_PULLUP

System.gpio.HIGH
System.gpio.LOW
```

---

## `System.gpio.pinMode(pin, mode)`

Configures GPIO pin mode.

```lua
System.gpio.pinMode(2, System.gpio.OUTPUT)
```

---

## `System.gpio.digitalWrite(pin, value)`

Writes HIGH or LOW to a GPIO.

```lua
System.gpio.digitalWrite(2, System.gpio.HIGH)
```

---

## `System.gpio.digitalRead(pin)`

**Returns:** `integer`

```text
1 = HIGH
0 = LOW
```

---

## `System.gpio.analogRead(pin)`

**Returns:** `integer`

ESP32 ADC range:

```text
0 - 4095
```

---

## `System.gpio.analogWrite(pin, value)`

**Params:**

* `pin`
* `value` — 0-255

**Returns:** `none`

Uses hardware PWM.

---

## `System.gpio.pulseIn(pin, state, timeout)`

**Params:**

* `pin`
* `state`
* `timeout` — optional microseconds

**Returns:** `integer`

Returns measured pulse duration in microseconds.

Default timeout:

```text
1,000,000 µs
```

Returns `0` if the timeout expires without detecting the requested pulse.

---

# 6. KEYBOARD & INPUT (`System.*`)

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

**Description:** Reads keyboard/navigation state.

`ESC` is the standard application-exit key.

Example:

```lua
local key = System.getKey()

if key == "ENTER" then
    -- Confirm
elseif key == "ESC" then
    -- Exit
end
```

---

## `System.isKeyPressed(keyName)`

**Params:**

* `keyName` — string

**Returns:** `boolean`

Checks whether a specific key is currently pressed.

```lua
if System.isKeyPressed("ENTER") then
    -- Enter is pressed
end
```

---

## `System.getKeyInput()`

**Params:** none

**Returns:** `table`

Structure:

```lua
{
    key = "ENTER",
    code = 13,
    pressed = true
}
```

Fields:

* `key` — key name
* `code` — numeric key code
* `pressed` — boolean

Example:

```lua
local input = System.getKeyInput()

if input.pressed then
    System.print(input.key)
end
```

---

## `System.getChar()`

**Params:** none

**Returns:** `string`

Returns a translated keyboard character.

Possible input includes:

* normal characters
* tab
* newline
* backspace
* empty string when no character is available

Example:

```lua
local char = System.getChar()

if char ~= "" then
    System.print(char)
end
```

For application/system exit, prefer:

```lua
System.getKey()
```

and check for:

```text
"ESC"
```

---

## `System.prompt(msg, initialText)`

**Params:**

* `msg` — optional string
* `initialText` — optional string

**Returns:** `string`

Opens the native KryonOS on-screen keyboard/text-input interface.

```lua
local name = System.prompt("Enter your name", "")

System.print(name)
```

Execution is suspended while the native input interface is active.

---

## `System.getTouch()`

**Params:** none

**Returns:** `table`

Structure:

```lua
{
    x = 120,
    y = 80,
    touched = true
}
```

Fields:

* `x` — X coordinate
* `y` — Y coordinate
* `touched` — boolean

When no touch is active:

```lua
{
    x = 0,
    y = 0,
    touched = false
}
```

### Exit Area

The top-right region is reserved for application/OS exit.

Recommended check:

```lua
local touch = System.getTouch()

if touch.touched and
   touch.x >= System.screenWidth() - 40 and
   touch.y <= 40 then

    break
end
```

---

# 7. SYSTEM UTILITIES & HARDWARE INFORMATION

## `System.millis()`

**Returns:** `integer`

Returns system uptime in milliseconds.

---

## `System.micros()`

**Returns:** `integer`

Returns system uptime in microseconds.

The 32-bit counter rolls over approximately every 71 minutes.

---

## `System.delay(ms)`

**Params:**

* `ms` — integer milliseconds

**Returns:** `none`

Pauses Lua execution and gives the kernel an opportunity to perform garbage collection.

Required for long-running loops.

---

## `System.delayMicroseconds(us)`

**Params:**

* `us` — integer microseconds

**Returns:** `none`

Provides a high-resolution blocking delay.

Does not provide the same garbage-collection opportunity as `System.delay()`.

---

## `System.print(msg)`

**Params:**

* `msg` — value/string

**Returns:** `none`

Prints debugging information to the USB Serial Monitor.

Default baud rate:

```text
115200
```

---

## `System.getTemperature()`

**Returns:** `float`

Returns ESP32 internal temperature in degrees Celsius when supported.

---

## `System.hasTemperatureSensor()`

**Returns:** `boolean`

Returns `true` if the installed ESP32 chip supports the internal temperature sensor.

---

## `System.getInfo()`

**Returns:** `table`

Structure:

```lua
{
    totalRAM = 0,
    freeRAM = 0,
    minFreeRAM = 0,
    maxAllocRAM = 0,
    cpuFreqMHz = 0,
    chipModel = "",
    chipCores = 0,
    chipRevision = 0,
    flashSize = 0,
    uptimeMs = 0
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

---

## `System.restart()`

**Returns:** `none`

Immediately reboots the ESP32.

```lua
System.restart()
```

---

# 8. TIME, DATE & NETWORK (`System.*`)

## `System.getTime()`

**Returns:** `string`

Returns OS-formatted local time according to the configured 12/24-hour preference.

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

Returns local date using the OS date format.

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

Returns current KryonOS version.

---

## `System.getAPILevel()`

**Returns:** `integer`

Returns current KryonOS Lua API level.

---

## `System.getIPAddress()`

**Returns:** `string`

Returns local ESP32 IP address when WiFi is connected.

---

## `System.isWiFiActive()`

**Returns:** `boolean`

Returns `true` when WiFi is active/connected.

---

# 9. FILE SYSTEM (`FS.*`)

KryonOS provides a unified filesystem interface.

Internal flash:

```text
/local/
```

SD card:

```text
/sd/
```

---

## `FS.exists(path)`

**Returns:** `boolean`

Checks whether a file or directory exists.

---

## `FS.readTextFile(path)`

**Returns:** `string` or `nil`

Reads a complete text file into memory.

For large files, avoid loading the entire file into the Lua heap.

---

## `FS.writeTextFile(path, content)`

**Returns:** `boolean`

Writes text content to a file.

Existing content is replaced.

---

## `FS.appendTextFile(path, content)`

**Returns:** `boolean`

Appends text content to a file.

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

**Returns:** `table`

Returns a Lua table containing file/directory paths.

Lua arrays are **1-indexed**.

Example:

```lua
local files = FS.listDir("/local")

for i = 1, #files do
    System.print(files[i])
end
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

```lua
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

Returns free storage space in bytes.

---

## `FS.getFileMD5(path)`

**Returns:** `string`

Returns the MD5 hash of the specified file.

Example:

```text
d41d8cd98f00b204e9800998ecf8427e
```

---

## `FS.mountSD()`

**Returns:** `boolean`

Mounts the SD card filesystem.

Returns `true` when successful.

---

## `FS.unmountSD()`

**Returns:** `none`

Unmounts the SD card filesystem.

---

# 10. BINARY FILES (`FS.*`)

KryonOS Lua supports raw binary file access.

Lua strings can contain null bytes (`0x00`) and therefore can be used to represent binary data.

---

## `FS.readBinaryFile(path)`

**Params:**

* `path` — string

**Returns:**

* `string` containing raw binary data
* `nil` if the file does not exist or cannot be read

**Description:** Reads the entire file without text conversion.

Example:

```lua
local data = FS.readBinaryFile("/local/data.bin")

if data ~= nil then
    System.print("Binary file loaded")
end
```

An existing empty file returns:

```lua
""
```

The returned string length corresponds to the number of bytes read.

> Avoid loading large binary files into Lua memory because the ESP32 has limited RAM.

---

## `FS.writeBinaryFile(path, data)`

**Params:**

* `path` — string
* `data` — string containing binary data

**Returns:** `boolean`

Writes raw binary data to storage.

Example:

```lua
local data = string.char(1, 2, 3, 255)

local success = FS.writeBinaryFile(
    "/local/test.bin",
    data
)
```

Empty data is not written and returns `false`.

---

# 11. RESPONSIVE DISPLAY DESIGN

Applications should not assume that the display is always 240×320.

Use:

```lua
local width = System.screenWidth()
local height = System.screenHeight()
```

Example:

```lua
local width = System.screenWidth()
local height = System.screenHeight()

if height >= 200 then

    System.drawString("240x320 Layout", 10, 20)

else

    System.drawString("240x135 Layout", 10, 20)

end
```

For a responsive button:

```lua
local width = System.screenWidth()

local buttonX = width - 45
local buttonY = 5
local buttonW = 40
local buttonH = 25

System.fillRoundRect(
    buttonX,
    buttonY,
    buttonW,
    buttonH,
    5,
    0xF800
)
```

---

# 12. RECOMMENDED APPLICATION TEMPLATE

A standard KryonOS Lua application should follow this structure:

```lua
local width = System.screenWidth()
local height = System.screenHeight()

local BLUE = 0x001F
local WHITE = 0xFFFF
local RED = 0xF800

System.fillScreen(BLUE)
System.setTextColor(WHITE, BLUE)

System.drawString(
    "KryonOS Lua",
    10,
    15
)

if height >= 200 then
    System.drawString(
        "240x320 Layout",
        10,
        50
    )
else
    System.drawString(
        "240x135 Layout",
        10,
        50
    )
end

while true do

    -- Touch input
    local touch = System.getTouch()

    if touch.touched and
       touch.x >= width - 40 and
       touch.y <= 40 then

        break
    end

    -- Keyboard input
    local key = System.getKey()

    if key == "ESC" then
        break
    end

    -- Character input
    local char = System.getChar()

    if char ~= "" then
        -- Process character
    end

    -- Required for kernel/GC responsiveness
    System.delay(10)
end

-- Application cleanup
System.fillScreen(BLUE)
```

---

# 13. API SUMMARY

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

# 14. IMPORTANT LUA APPLICATION RULES

1. **Poll input continuously** in applications that remain active.
2. Call `System.getTouch()` to allow touchscreen interaction and system exit processing.
3. Call `System.getKey()` for keyboard/navigation events.
4. Use `System.getChar()` for character-oriented input.
5. `ESC` is the standard keyboard exit action.
6. The top-right touchscreen area is reserved for application/OS exit.
7. Long-running loops must call `System.delay(10)` or another appropriate delay.
8. Avoid unnecessary RAM allocations.
9. Avoid full-screen sprites on memory-constrained ESP32 hardware.
10. Prefer sliced rendering for large graphics.
11. Always call `System.deleteSprite()` after finishing with a sprite.
12. Use `System.screenWidth()` and `System.screenHeight()` for responsive layouts.
13. Use `/local/` for internal storage.
14. Use `/sd/` for SD card storage.
15. Use binary file APIs for binary data.
16. Lua tables returned as arrays/lists are **1-indexed**.
17. `nil` is used to indicate missing/unreadable file content where specified.
18. `System.getKey()` should be preferred over interpreting raw characters for system/navigation actions.

---

**Document Version:** 2.0
**Target:** KryonOS Lua Runtime / HarixKernel
**Platform:** ESP32
**Runtime:** Embedded Lua 5.1
**API Level:** 1
