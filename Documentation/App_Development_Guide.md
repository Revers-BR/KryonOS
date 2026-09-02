# KryonOS App Development Guide (JavaScript & Lua)

Welcome to the KryonOS App Development Guide! Developing apps for KryonOS is flexible and simple. KryonOS features a **Dual Scripting Runtime** that natively supports both **JavaScript** (via Duktape) and **Lua** (via `LuaBindings`). 

Apps are structured as standard folders containing metadata (`app.json`) and the main execution script (`main.js` or `main.lua`).

---

## 1. App Folder Structure

In KryonOS, every app is a standalone folder. When uploading or installing an app, the OS reads the entire folder structure.

Depending on your preferred language, your app folder will look like one of the following:

### JavaScript App Folder:
```text
MyAwesomeApp/
├── app.json
└── main.js
```

### Lua App Folder:
```text
MyAwesomeLuaApp/
├── app.json
└── main.lua
```

---

## 2. The `app.json` File (App Metadata)

The `app.json` file defines your app's identity, entry point, versioning, and category. The KryonOS Installer reads this file to securely manage installation, updates, and UI rendering in the Launcher.

### Example Format:
```json
{
  "name": "My App",
  "packageName": "com.developer.myapp",
  "version": "1.0.0",
  "entry": "main.js",
  "metaUrl": "[https://raw.githubusercontent.com/.../myapp/app.json](https://raw.githubusercontent.com/.../myapp/app.json)",
  "author": "John Doe",
  "description": "A cool app built for KryonOS.",
  "type": "App",
  "category": "Utility",
  "api": 1,
  "changelog": "Initial release with dual-engine support."
}
```

### Field Details:
- **`name`**: Display name shown in the KryonOS Launcher.
- **`packageName`**: Unique identifier. **Format: lowercase, dot-separated, no spaces** (e.g., `com.yourname.appname`). Used to prevent duplicate installs and handle updates.
- **`version`**: Semantic versioning (e.g., `1.0.0`, `1.1.0`). Uploading a package with the same `packageName` but a higher version prompts a system update.
- **`entry`** *(Optional)*: Specifies the entry point script (e.g., `"main.js"` or `"main.lua"`). If omitted, KryonOS automatically checks for `main.js` or `main.lua`.
- **`metaUrl`**: Direct raw URL to `app.json` for remote App Store version checking.
- **`author`**: Developer or organization name.
- **`description`**: Summary displayed during installation/overview.
- **`type`**: Broad classification (e.g., `App`, `Game`).
- **`category`**: Category in the Launcher (e.g., `Utility`, `Benchmark`, `Arcade`, `Tools`).
- **`api`**: KryonOS API level (currently `1`).
- **`changelog`**: Release notes displayed under "What's New" during updates.

---

## 3. Writing App Logic

KryonOS abstracts underlying hardware C++ calls into high-level APIs available to both JavaScript and Lua environments.

---

### Option A: JavaScript (`main.js`)
JavaScript apps run on the embedded **Duktape** engine (ES5 with async support).

```javascript
// Clear screen with blue background
Graphics.fillScreen(Graphics.COLOR_BLUE);

// Draw white text at coordinates (120, 160)
Graphics.setTextColor(Graphics.COLOR_WHITE);
Graphics.drawString("Hello KryonOS (JS)!", 120, 160, 2);

// Delay for 3 seconds
System.delay(3000);

// Return to Launcher
System.exit();
```

---

### Option B: Lua (`main.lua`)
Lua apps run directly via `LuaBindings`, giving fast, low-overhead access to display primitives, GPIO control, and system utilities.

```lua
-- Clear screen with blue background
Graphics.fillScreen(Graphics.COLOR_BLUE)

-- Draw white text at coordinates (120, 160)
Graphics.setTextColor(Graphics.COLOR_WHITE)
Graphics.drawString("Hello KryonOS (Lua)!", 120, 160, 2)

-- Delay for 3 seconds (3000 ms)
System.delay(3000)

-- Exit back to Launcher
System.exit()
```

---

## 4. Hardware API Capabilities

Both JavaScript and Lua engines expose identical underlying core hardware capabilities:

| Feature Category | Functions Provided |
| :--- | :--- |
| **Graphics & Display** | `fillScreen()`, `drawPixel()`, `drawLine()`, `drawRect()`, `drawString()`, `setRotation()`, `pushSprite()` |
| **System & Memory** | `delay()`, `exit()`, `getFreeHeap()`, `getBatteryPercent()`, `getBatteryVoltage()` |
| **Input Systems** | `isTouched()`, `getTouch()`, `getKeyInput()`, `isShiftActive()`, `isFnActive()` |
| **GPIO & Hardware** | `pinMode()`, `digitalWrite()`, `digitalRead()`, `analogRead()` |
| **Networking & VFS** | `wifiConnect()`, `isWifiConnected()`, `readFile()`, `writeFile()` |

---

## 5. Next Steps & References

* **[JS  API Guide](JS_API_Guide.md)** - Detailed method signatures for JavaScript.
* **[Lua API Guide](LUA_API_Guide.md)** - Detailed method signatures for LUA.
* **[WREN API Guide](WREN_API_Guide.md)** - Detailed method signatures for WREN.