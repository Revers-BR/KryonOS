// // ============================================================================
// // SYSTEM.JS - ESP32 / KryonOS Hardware Abstraction Layer & Emulation Engine
// // ============================================================================

// // --- Configurações e Constantes de Tela ---
// var TFT_WIDTH = 240;
// var TFT_HEIGHT = 320;
// var bootTime = Date.now();

// // --- Estados Internos do Hardware Emulado ---
// var textSize = 1;
// var textFgColor = 0xffff; // Branco em RGB565
// var textBgColor = undefined;
// var spriteBinding = false;
// var activeSprite = null;

// var touchState = { x: 0, y: 0, touched: false };

// var hwState = {
//   wifiActive: false,
//   pinModes: {},
//   digitalOutputs: {},
//   analogOutputs: {},
// };

// var wifiMemory = {
//   connectedSSID: null,
//   savedNetworks: [],
// };

// // --- Helpers Privados de Renderização ---
// function rgb565ToCss(color565) {
//   var r = ((color565 >> 11) & 0x1f) * 8;
//   var g = ((color565 >> 5) & 0x3f) * 4;
//   var b = (color565 & 0x1f) * 8;
//   return "rgb(" + r + "," + g + "," + b + ")";
// }

// function getCurrentTargetCtx() {
//   var canvas = document.getElementById("tftDisplay");
//   var targetCtx =
//     spriteBinding && activeSprite
//       ? activeSprite.ctx
//       : canvas
//         ? canvas.getContext("2d")
//         : null;

//   if (targetCtx) {
//     targetCtx.imageSmoothingEnabled = false;
//     targetCtx.webkitImageSmoothingEnabled = false;
//     targetCtx.mozImageSmoothingEnabled = false;
//     targetCtx.msImageSmoothingEnabled = false;
//   }
//   return targetCtx;
// }

// ============================================================================
// SYSTEM.JS - ESP32 / KryonOS Hardware Abstraction Layer & Emulation Engine
// ============================================================================

// --- Configuração dos Perfis de Hardware ---
// ============================================================================
// SYSTEM.JS - ESP32 / KryonOS Hardware Abstraction Layer & Emulation Engine
// ============================================================================

var DEVICE_PROFILES = {
  "TOUCH_320": { width: 240, height: 320, hasTouch: true,  hasKeyboard: false, badge: "ESP32 Touch 240x320", title: "KryonOS ESP32 Touch Screen" },
  "TOUCH_135": { width: 240, height: 135, hasTouch: true,  hasKeyboard: false, badge: "ESP32 Mini 240x135",  title: "KryonOS ESP32 Mini Touch" },
  "KEY_320":   { width: 240, height: 320, hasTouch: false, hasKeyboard: true,  badge: "ESP32 Keypad 240x320", title: "KryonOS ESP32 Keypad Kit" },
  "KEY_135":   { width: 240, height: 135, hasTouch: false, hasKeyboard: true,  badge: "ESP32 Cardputer 240x135", title: "KryonOS ESP32 Pocket Keyboard" }
};

var currentProfileKey = "TOUCH_320";
var TFT_WIDTH = 240;
var TFT_HEIGHT = 320;
var bootTime = Date.now();

var textSize = 1;
var textFgColor = 0xFFFF;
var textBgColor = undefined;
var spriteBinding = false;
var activeSprite = null;

var touchState = { x: 0, y: 0, touched: false };

var activeKeyboardState = {
  key: "NONE",
  code: 0,
  char: "",
  pressed: false
};

var KEY_MAP = {
  "ArrowUp": { name: "UP", code: 1, char: "" },
  "ArrowDown": { name: "DOWN", code: 2, char: "" },
  "ArrowLeft": { name: "LEFT", code: 3, char: "" },
  "ArrowRight": { name: "RIGHT", code: 4, char: "" },
  "Enter": { name: "ENTER", code: 5, char: "\n" },
  "Escape": { name: "ESC", code: 6, char: "" },
  "Backspace": { name: "BACK", code: 7, char: "\b" },
  "Tab": { name: "TAB", code: 8, char: "\t" }
};

var hwState = {
  wifiActive: true,
  pinModes: {},
  digitalOutputs: {},
  analogOutputs: {}
};

function rgb565ToCss(color565) {
  var r = ((color565 >> 11) & 0x1F) * 8;
  var g = ((color565 >> 5) & 0x3F) * 4;
  var b = (color565 & 0x1F) * 8;
  return 'rgb(' + r + ',' + g + ',' + b + ')';
}

function getCurrentTargetCtx() {
  var canvas = document.getElementById("tftDisplay");
  var targetCtx = (spriteBinding && activeSprite) ? activeSprite.ctx : (canvas ? canvas.getContext("2d") : null);
  if (targetCtx) {
    targetCtx.imageSmoothingEnabled = false;
  }
  return targetCtx;
}

// --- Objeto Global System ---
var System = {
  setDeviceProfile: function(profileKey) {
    if (!DEVICE_PROFILES[profileKey]) return;
    currentProfileKey = profileKey;
    var prof = DEVICE_PROFILES[profileKey];
    TFT_WIDTH = prof.width;
    TFT_HEIGHT = prof.height;

    var canvas = document.getElementById("tftDisplay");
    if (canvas) {
      canvas.width = TFT_WIDTH;
      canvas.height = TFT_HEIGHT;
    }

    var vKeyEl = document.getElementById("virtualKeyboard");
    if (vKeyEl) {
      vKeyEl.style.display = prof.hasKeyboard ? "flex" : "none";
    }

    System.fillScreen(0x0000);
    System.print("[HARDWARE] Perfil alterado para: " + prof.name);
  },

  getDeviceConfig: function() {
    return DEVICE_PROFILES[currentProfileKey];
  },
  
  screenWidth: function () {
    return TFT_WIDTH;
  },
  screenHeight: function () {
    return TFT_HEIGHT;
  },
  getOSVersion: function () {
    return "1.0.0";
  },
  getAPILevel: function () {
    return 1;
  },
  millis: function () {
    return Date.now() - bootTime;
  },
  micros: function () {
    return (Date.now() - bootTime) * 1000;
  },
  getTemperature: function () {
    return parseFloat((38.0 + Math.sin(Date.now() / 3000) * 1.5).toFixed(1));
  },
  hasTemperatureSensor: function () {
    return true;
  },

  delay: function (ms) {
    return new Promise(function (resolve) {
      setTimeout(resolve, ms);
    });
  },
  delayMicroseconds: function (us) {
    return new Promise(function (resolve) {
      setTimeout(resolve, us / 1000);
    });
  },

  print: function (str) {
    var consoleEl = document.getElementById("consoleOutput");
    if (consoleEl) {
      consoleEl.textContent += str + "\n";
      consoleEl.scrollTop = consoleEl.scrollHeight;
    } else {
      console.log("[System.print]", str);
    }
  },

  connectWiFi: function (ssid, password) {
    hwState.wifiActive = true;
    wifiMemory.connectedSSID = ssid;

    if (wifiMemory.savedNetworks.indexOf(ssid) === -1) {
      wifiMemory.savedNetworks.push(ssid);
    }

    // Persiste no Virtual File System (se disponível)
    if (typeof FS !== "undefined" && FS.writeTextFile) {
      FS.writeTextFile("/local/wifi.json", JSON.stringify(wifiMemory));
    }

    var notificationMsg = "Conectado à rede Wi-Fi: " + ssid;
    System.print("[WIFI] " + notificationMsg);

    var c = getCurrentTargetCtx();
    if (c) {
      c.fillStyle = "rgba(0, 150, 0, 0.85)";
      c.fillRect(10, 10, c.canvas.width - 20, 30);
      c.fillStyle = "#FFFFFF";
      c.font = "12px monospace";
      c.fillText(notificationMsg, 15, 30);
    }

    return true;
  },

  getTouch: function () {
    if (touchState.touched && touchState.x >= 200 && touchState.y <= 40) {
      System.print("[KERNEL] Gatilho de Saída Acionado! App encerrado.");
      if (typeof stopScript === "function") {
        stopScript("App abortado pelo usuário (Touch no canto sup. dir).");
      }
    }
    return { x: touchState.x, y: touchState.y, touched: touchState.touched };
  },

  // --- API do Teclado ---
  triggerKeyEvent: function(keyName, code, charVal, isPress) {
    if (isPress) {
      activeKeyboardState.key = keyName;
      activeKeyboardState.code = code;
      activeKeyboardState.char = charVal;
      activeKeyboardState.pressed = true;

      if (keyName === "ESC") {
        if (typeof stopScript === 'function') stopScript("OS_EXIT: Cancelado via tecla ESC.");
        throw new Error("OS_EXIT");
      }
    } else {
      activeKeyboardState.pressed = false;
      activeKeyboardState.key = "NONE";
      activeKeyboardState.code = 0;
      activeKeyboardState.char = "";
    }

    var hudKey = document.getElementById("hudKey");
    if (hudKey) {
      hudKey.innerText = "Key: " + (activeKeyboardState.pressed ? activeKeyboardState.key : "NONE");
    }
  },

  getKey: function() {
    if (activeKeyboardState.key === "ESC") throw new Error("OS_EXIT");
    return activeKeyboardState.pressed ? activeKeyboardState.key : "NONE";
  },

  isKeyPressed: function(targetKey) {
    if (activeKeyboardState.key === "ESC") throw new Error("OS_EXIT");
    return activeKeyboardState.pressed && (activeKeyboardState.key === targetKey);
  },

  getKeyInput: function() {
    if (activeKeyboardState.key === "ESC") throw new Error("OS_EXIT");
    return {
      key: activeKeyboardState.pressed ? activeKeyboardState.key : "NONE",
      code: activeKeyboardState.pressed ? activeKeyboardState.code : 0,
      pressed: activeKeyboardState.pressed
    };
  },

  getChar: function() {
    if (activeKeyboardState.key === "ESC") throw new Error("OS_EXIT");
    return activeKeyboardState.pressed ? (activeKeyboardState.char || "") : "";
  },

  getTouch: function () {
    if (!DEVICE_PROFILES[currentProfileKey].hasTouch) {
      return { x: 0, y: 0, touched: false };
    }
    if (touchState.touched && touchState.x >= (TFT_WIDTH - 40) && touchState.y <= 40) {
      System.print("[KERNEL] Saída via Touch na Zona EXIT!");
      if (typeof stopScript === 'function') stopScript("App encerrado via touch.");
    }
    return { x: touchState.x, y: touchState.y, touched: touchState.touched };
  },

  getInfo: function () {
    return {
      totalRAM: 327680,
      freeRAM: 92160,
      minFreeRAM: 45000,
      maxAllocRAM: 28000,
      cpuFreqMHz: 240,
      chipModel: "ESP32-D0WDQ6",
      chipCores: 2,
      chipRevision: 1,
      flashSize: 4194304,
      uptimeMs: System.millis(),
    };
  },

  getIPAddress: function () {
    return hwState.wifiActive ? "192.168.1.100" : "0.0.0.0";
  },
  isWiFiActive: function () {
    return hwState.wifiActive;
  },
  getConnectedSSID: function () {
    return wifiMemory.connectedSSID || "Desconectado";
  },
  restart: function () {
    if (typeof rebootSystem === "function") {
      rebootSystem();
    } else {
      location.reload();
    }
  },

  getTime: function () {
    var d = new Date();
    return (
      ("0" + d.getHours()).slice(-2) + ":" + ("0" + d.getMinutes()).slice(-2)
    );
  },
  getSeconds: function () {
    return new Date().getSeconds();
  },
  getDate: function () {
    var d = new Date();
    return (
      ("0" + d.getDate()).slice(-2) +
      "/" +
      ("0" + (d.getMonth() + 1)).slice(-2) +
      "/" +
      d.getFullYear()
    );
  },
  getYear: function () {
    return new Date().getFullYear();
  },
  getMonth: function () {
    return new Date().getMonth() + 1;
  },
  getDay: function () {
    return new Date().getDate();
  },
  getTimezone: function () {
    return "UTC-3";
  },

  prompt: function (promptMsg, initialText) {
    return new Promise(function (resolve) {
      var res = window.prompt(promptMsg, initialText || "");
      resolve(res || "");
    });
  },

  color: function (r, g, b) {
    return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
  },

  fillScreen: function (color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.fillStyle = rgb565ToCss(color);
    c.fillRect(0, 0, c.canvas.width, c.canvas.height);
  },
  drawPixel: function (x, y, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.fillStyle = rgb565ToCss(color);
    c.fillRect(x, y, 1, 1);
  },
  drawLine: function (x1, y1, x2, y2, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.strokeStyle = rgb565ToCss(color);
    c.beginPath();
    c.moveTo(x1, y1);
    c.lineTo(x2, y2);
    c.stroke();
  },
  drawFastHLine: function (x, y, w, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.fillStyle = rgb565ToCss(color);
    c.fillRect(x, y, w, 1);
  },
  drawFastVLine: function (x, y, h, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.fillStyle = rgb565ToCss(color);
    c.fillRect(x, y, 1, h);
  },
  drawRect: function (x, y, w, h, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.strokeStyle = rgb565ToCss(color);
    c.strokeRect(x, y, w, h);
  },
  fillRect: function (x, y, w, h, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.fillStyle = rgb565ToCss(color);
    c.fillRect(x, y, w, h);
  },
  drawRoundRect: function (x, y, w, h, radius, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.strokeStyle = rgb565ToCss(color);
    c.beginPath();
    c.roundRect(x, y, w, h, radius);
    c.stroke();
  },
  fillRoundRect: function (x, y, w, h, radius, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.fillStyle = rgb565ToCss(color);
    c.beginPath();
    c.roundRect(x, y, w, h, radius);
    c.fill();
  },
  drawCircle: function (x, y, r, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.strokeStyle = rgb565ToCss(color);
    c.beginPath();
    c.arc(x, y, r, 0, 2 * Math.PI);
    c.stroke();
  },
  fillCircle: function (x, y, r, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.fillStyle = rgb565ToCss(color);
    c.beginPath();
    c.arc(x, y, r, 0, 2 * Math.PI);
    c.fill();
  },
  drawTriangle: function (x1, y1, x2, y2, x3, y3, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.strokeStyle = rgb565ToCss(color);
    c.beginPath();
    c.moveTo(x1, y1);
    c.lineTo(x2, y2);
    c.lineTo(x3, y3);
    c.closePath();
    c.stroke();
  },
  fillTriangle: function (x1, y1, x2, y2, x3, y3, color) {
    var c = getCurrentTargetCtx();
    if (!c) return;
    c.fillStyle = rgb565ToCss(color);
    c.beginPath();
    c.moveTo(x1, y1);
    c.lineTo(x2, y2);
    c.lineTo(x3, y3);
    c.closePath();
    c.fill();
  },

  createSprite: function (w, h) {
    var spriteCanvas = document.createElement("canvas");
    spriteCanvas.width = w;
    spriteCanvas.height = h;
    activeSprite = {
      canvas: spriteCanvas,
      ctx: spriteCanvas.getContext("2d"),
      w: w,
      h: h,
    };
    return true;
  },
  bindSprite: function (enabled) {
    spriteBinding = !!enabled;
  },
  pushSprite: function (x, y) {
    var canvas = document.getElementById("tftDisplay");
    if (activeSprite && canvas) {
      var mainCtx = canvas.getContext("2d");
      mainCtx.drawImage(activeSprite.canvas, x, y);
    }
  },
  deleteSprite: function () {
    activeSprite = null;
    spriteBinding = false;
  },

  setTextColor: function (fg, bg) {
    textFgColor = fg;
    textBgColor = bg;
  },
  setTextSize: function (s) {
    textSize = Math.max(1, Math.min(5, s));
  },
  drawString: function (str, x, y, font) {
    var c = getCurrentTargetCtx();
    if (!c) return;

    x = Math.floor(x);
    y = Math.floor(y);

    var fontSize = (font === 4 ? 18 : font === 2 ? 14 : 10) * textSize;
    c.font = fontSize + "px 'Courier New', monospace";

    if (textBgColor !== undefined) {
      var metrics = c.measureText(str);
      c.fillStyle = rgb565ToCss(textBgColor);
      c.fillRect(x, y, Math.ceil(metrics.width), fontSize);
    }

    c.fillStyle = rgb565ToCss(textFgColor);
    c.fillText(str, x, y + fontSize - 2);
  },

  drawBMP: function (path, x, y) {
    if (typeof FS === "undefined" || !FS.exists(path) || FS.isDirectory(path)) {
      System.print("[BMP Error] Arquivo nao encontrado: " + path);
      return false;
    }

    if (!path.toLowerCase().endsWith(".bmp")) {
      System.print("[BMP Error] Formato invalido: " + path);
      return false;
    }

    var c = getCurrentTargetCtx();
    if (!c) return false;

    var fileData = FS.readTextFile(path);

    if (fileData && fileData.indexOf("data:image") === 0) {
      var img = new Image();
      img.onload = function () {
        c.drawImage(img, x, y);
      };
      img.src = fileData;
      return true;
    }

    var bmpWidth = 32;
    var bmpHeight = 32;

    c.fillStyle = "#3B82F6";
    c.fillRect(x, y, bmpWidth, bmpHeight);
    c.strokeStyle = "#FFFFFF";
    c.strokeRect(x, y, bmpWidth, bmpHeight);

    c.fillStyle = "#FFFFFF";
    c.font = "8px monospace";
    c.fillText("BMP", x + 6, y + 18);

    System.print("[BMP] Renderizado: " + path + " em (" + x + ", " + y + ")");
    return true;
  },

  gpio: {
    INPUT: 0,
    OUTPUT: 1,
    INPUT_PULLUP: 2,
    HIGH: 1,
    LOW: 0,
    pinMode: function (pin, mode) {
      hwState.pinModes[pin] = mode;
    },
    digitalWrite: function (pin, val) {
      hwState.digitalOutputs[pin] = val;
      if (pin === 2) {
        var led = document.getElementById("ledPin2");
        if (led) led.className = "led-indicator " + (val ? "active" : "");
      }
    },
    digitalRead: function (pin) {
      if (pin === 5) {
        var el = document.getElementById("pin5Digital");
        return el && el.checked ? 1 : 0;
      }
      return 0;
    },
    analogRead: function (pin) {
      if (pin === 4) {
        var el = document.getElementById("pin4Analog");
        return el ? parseInt(el.value) : 0;
      }
      return 0;
    },
    analogWrite: function (pin, pwm) {
      hwState.analogOutputs[pin] = pwm;
    },
    pulseIn: function (pin, state, timeout) {
      timeout = timeout !== undefined ? timeout : 1000000;
      var simulatedPulseMs = Math.floor(Math.random() * 5800) + 115;
      return simulatedPulseMs > timeout ? 0 : simulatedPulseMs;
    },
  },
};


// Captura de Teclas do Teclado do Computador
window.addEventListener("keydown", function(e) {
  if (e.target.tagName === "TEXTAREA" || e.target.tagName === "INPUT" || e.target.classList.contains("CodeMirror-textarea")) return;
  var mapped = KEY_MAP[e.key];
  if (mapped) {
    System.triggerKeyEvent(mapped.name, mapped.code, mapped.char, true);
  } else if (e.key.length === 1) {
    System.triggerKeyEvent(e.key.toUpperCase(), e.key.charCodeAt(0), e.key, true);
  }
});

window.addEventListener("keyup", function(e) {
  if (e.target.tagName === "TEXTAREA" || e.target.tagName === "INPUT" || e.target.classList.contains("CodeMirror-textarea")) return;
  System.triggerKeyEvent("NONE", 0, "", false);
});