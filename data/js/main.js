// =========================================================================
// MAIN.JS - IDE INTERFACE, CODE EDITOR & EXECUTION ENGINE
// =========================================================================

var isRunning = false;
var scriptExecutionId = 0;
var autoRunTimer = null;

var canvas = document.getElementById("tftDisplay");
var ctx = canvas ? canvas.getContext("2d") : null;

if (ctx) {
  ctx.imageSmoothingEnabled = false;
}

var COLOR_CONSTANTS = {
  BLACK: 0x0000, WHITE: 0xffff, RED: 0xf800, GREEN: 0x07e0,
  BLUE: 0x001f, DARKGREY: 0x7bef, LIGHTGREY: 0xc618, YELLOW: 0xffe0,
  CYAN: 0x07ff, MAGENTA: 0xf81f, NAVY: 0x000f, PURPLE: 0x780f, ORANGE: 0xfd20
};

var codeExamples = {
  dashboard: `// Modern UI Dashboard Demo para KryonOS\nvar SW = System.screenWidth();\nvar SH = System.screenHeight();\n\nSystem.fillScreen(BLACK);\nSystem.setTextColor(WHITE, BLACK);\nSystem.drawString("KryonOS Dashboard", 10, 15, 2);\nSystem.drawFastHLine(0, 40, SW, BLUE);\n\nSystem.drawString("Res: " + SW + "x" + SH, 10, 50, 1);\nSystem.drawString("Temp CPU: " + System.getTemperature() + " C", 10, 70, 1);`,
  keyboard: `// Exemplo: KEYPAD & TECLADO API\nSystem.fillScreen(BLACK);\nSystem.setTextColor(WHITE, BLACK);\nSystem.drawString("Aguardando Tecla...", 10, 10, 1);\n\nwhile (true) {\n  var k = System.getKey();\n  if (k !== "NONE") {\n    System.fillRect(10, 30, 200, 40, RED);\n    System.setTextColor(WHITE, RED);\n    System.drawString("Tecla: " + k, 15, 42, 2);\n  }\n  System.delay(50);\n}`,
  graphics: `// Exemplo: GRAPHICS API\nSystem.fillScreen(BLACK);\nSystem.drawRect(10, 10, 100, 50, RED);\nSystem.fillRect(120, 10, 100, 50, BLUE);\nSystem.drawString("Graphics API", 15, 270, 2);`,
  fs: `// Exemplo: FILE SYSTEM API\nvar caminho = "/local/exemplo.txt";\nFS.writeTextFile(caminho, "KryonOS Data Log");\nSystem.print("Lido do FS: " + FS.readTextFile(caminho));`,
  gpio: `// Exemplo: HARDWARE GPIO API\nSystem.gpio.pinMode(2, 1);\nvar estadoLed = 0;\nwhile (true) {\n  estadoLed = estadoLed === 0 ? 1 : 0;\n  System.gpio.digitalWrite(2, estadoLed);\n  System.delay(500);\n}`,
  system: `// Exemplo: SYSTEM API\nSystem.fillScreen(BLACK);\nSystem.drawString("Millis: " + System.millis(), 20, 40, 2);`,
  sprites: `// Exemplo: SPRITES\nSystem.createSprite(80, 80);\nSystem.bindSprite(true);\nSystem.fillScreen(BLUE);\nSystem.bindSprite(false);\nSystem.pushSprite(20, 20);`
};

// --- Inicialização do Editor CodeMirror ---
// =========================================================================
// INICIALIZAÇÃO DO EDITOR CODEMIRROR & POP-OVER DE ASSINATURAS
// =========================================================================

var editor = CodeMirror.fromTextArea(document.getElementById("codeEditor"), {
  mode: "javascript",
  theme: "dracula",
  lineNumbers: true,
  tabSize: 2,
  extraKeys: { "Ctrl-Space": "autocomplete" }
});

// 1. Provedor de Hint / Autocomplete
CodeMirror.registerHelper("hint", "javascript", function (cm) {
  var cur = cm.getCursor();
  var token = cm.getTokenAt(cur);
  var word = token.string.trim();

  // Lista unificada de métodos do System, FS e Constantes de Cor
  var suggestions = [
    "System.screenWidth()", "System.screenHeight()", "System.getOSVersion()",
    "System.getDeviceConfig()", "System.millis()", "System.delay(", "System.print(",
    "System.getTouch()", "System.getKey()", "System.isKeyPressed(", "System.getKeyInput()",
    "System.getChar()", "System.fillScreen(", "System.drawPixel(", "System.drawLine(",
    "System.drawRect(", "System.fillRect(", "System.drawCircle(", "System.fillCircle(",
    "System.drawString(", "System.setTextColor(", "System.setTextSize(",
    "System.gpio.pinMode(", "System.gpio.digitalWrite(", "System.gpio.digitalRead(", "System.gpio.analogRead(",
    "FS.exists(", "FS.readTextFile(", "FS.writeTextFile(", "FS.deleteFile(", "FS.listDir(",
    "BLACK", "WHITE", "RED", "GREEN", "BLUE", "YELLOW", "CYAN", "MAGENTA", "DARKGREY", "ORANGE"
  ];

  var list = suggestions.filter(function (item) {
    return item.toLowerCase().indexOf(word.toLowerCase()) === 0;
  });

  return {
    list: list,
    from: CodeMirror.Pos(cur.line, token.start),
    to: CodeMirror.Pos(cur.line, cur.ch)
  };
});

// Dispara Autocomplete ao digitar
editor.on("inputRead", function (cm, change) {
  if (change.origin !== "+delete" && change.text[0] && /^[\w\.$]$/.test(change.text[0])) {
    CodeMirror.commands.autocomplete(cm, null, { completeSingle: false });
  }
});

// 2. Evento para Exibir o Popover de Documentação (methodSignatures)
var popover = document.getElementById("methodPopover");

editor.on("cursorActivity", function (cm) {
  if (!popover || typeof methodSignatures === "undefined") return;

  var cur = cm.getCursor();
  var lineText = cm.getLine(cur.line);
  var token = cm.getTokenAt(cur);

  // Procura por chamadas de métodos na linha (ex: System.getKey ou FS.writeTextFile)
  var matchedKey = null;
  Object.keys(methodSignatures).forEach(function (key) {
    if (lineText.indexOf(key) !== -1) {
      matchedKey = key;
    }
  });

  if (matchedKey) {
    var coords = cm.cursorCoords(true, "page");
    popover.innerText = methodSignatures[matchedKey];
    popover.style.display = "block";
    popover.style.left = coords.left + "px";
    popover.style.top = (coords.top - 30) + "px";
  } else {
    popover.style.display = "none";
  }
});

editor.on("inputRead", function (cm, change) {
  if (change.origin !== "+delete" && change.text[0] && /^[\w\.$]$/.test(change.text[0])) {
    CodeMirror.commands.autocomplete(cm, null, { completeSingle: false });
  }
});

function changeDeviceProfile() {
  var profKey = document.getElementById("deviceSelector").value;
  System.setDeviceProfile(profKey);
}

function validateES5Syntax(code) {
  var warnings = [];
  if (/\blet\s+/.test(code)) warnings.push("Uso de 'let' detectado! Duktape requer 'var'.");
  if (/\bconst\s+/.test(code)) warnings.push("Uso de 'const' detectado! Duktape requer 'var'.");
  if (/=>/.test(code)) warnings.push("Arrow Function (()=>) detectada! Use 'function() {}'.");

  var box = document.getElementById("linterBox");
  if (box) {
    if (warnings.length > 0) {
      box.style.display = "block";
      box.innerHTML = "⚠️ <b>Avisos ES5/Duktape:</b><br>" + warnings.join("<br>");
    } else {
      box.style.display = "none";
    }
  }
}

function transformAsyncCode(code) {
  var transformed = code.replace(/\bfunction\s+/g, "async function ");
  transformed = transformed.replace(/\bSystem\.delay\s*\(/g, "await System.delay(");
  return transformed;
}

async function runScript() {
  stopScript("Iniciando novo script...");
  var rawCode = editor.getValue();
  validateES5Syntax(rawCode);

  var asyncCode = transformAsyncCode(rawCode);

  isRunning = true;
  scriptExecutionId++;
  var currentId = scriptExecutionId;

  var statusEl = document.getElementById("scriptStatus");
  if (statusEl) {
    statusEl.innerText = "Rodando";
    statusEl.style.color = "var(--accent-color)";
  }

  try {
    var colorNames = Object.keys(COLOR_CONSTANTS);
    var colorValues = colorNames.map(function (k) { return COLOR_CONSTANTS[k]; });

    var fnArgs = ["System", "FS"].concat(colorNames).concat([
      "async function _exec() {\n" + asyncCode + "\n}\nreturn _exec();"
    ]);
    var asyncFn = Function.apply(null, fnArgs);

    await asyncFn.apply(null, [System, FS].concat(colorValues));

    if (currentId === scriptExecutionId && statusEl) {
      statusEl.innerText = "Concluído";
    }
  } catch (err) {
    if (currentId === scriptExecutionId) {
      if (err.message !== "OS_EXIT") {
        System.print("[ERRO JS] " + err.message);
      }
      if (statusEl) {
        statusEl.innerText = err.message === "OS_EXIT" ? "Encerrado" : "Erro";
        statusEl.style.color = "var(--accent-danger)";
      }
    }
  }
}

function stopScript(reason) {
  isRunning = false;
  scriptExecutionId++;
  var statusEl = document.getElementById("scriptStatus");
  if (statusEl) {
    statusEl.innerText = "Parado";
    statusEl.style.color = "var(--text-muted)";
  }
  if (reason && typeof System !== 'undefined') System.print("[SYSTEM] " + reason);
}

function rebootSystem() {
  stopScript("Reboot executado.");
  System.fillScreen(0x0000);
  System.drawString("KryonOS Booting...", 20, 50, 2);
  setTimeout(function () {
    System.fillScreen(0x0000);
  }, 400);
}

// Touch Handlers
function handlePointerStart(e) {
  if (!canvas) return;
  var rect = canvas.getBoundingClientRect();
  var clientX = e.touches ? e.touches[0].clientX : e.clientX;
  var clientY = e.touches ? e.touches[0].clientY : e.clientY;

  var conf = System.getDeviceConfig();
  touchState.x = Math.floor((clientX - rect.left) * (conf.width / rect.width));
  touchState.y = Math.floor((clientY - rect.top) * (conf.height / rect.height));
  touchState.touched = true;

  var touchHud = document.getElementById("hudTouch");
  if (touchHud) touchHud.innerText = "Touch: " + touchState.x + "," + touchState.y;
}

function handlePointerEnd() {
  touchState.touched = false;
  var touchHud = document.getElementById("hudTouch");
  if (touchHud) touchHud.innerText = "Touch: OFF";
}

if (canvas) {
  canvas.addEventListener("mousedown", handlePointerStart);
  canvas.addEventListener("mouseup", handlePointerEnd);
  canvas.addEventListener("touchstart", function (e) { e.preventDefault(); handlePointerStart(e); }, { passive: false });
  canvas.addEventListener("touchend", function (e) { e.preventDefault(); handlePointerEnd(); }, { passive: false });
}

function updateHwState() {
  var valPin4 = document.getElementById("valPin4");
  var pin4Analog = document.getElementById("pin4Analog");
  if (valPin4 && pin4Analog) valPin4.innerText = pin4Analog.value;

  var wifiToggle = document.getElementById("wifiToggle");
  if (wifiToggle && typeof hwState !== 'undefined') {
    hwState.wifiActive = wifiToggle.checked;
    var ledWifi = document.getElementById("ledWifi");
    if (ledWifi) ledWifi.className = "led-indicator " + (hwState.wifiActive ? "active" : "");
  }
}

function renderFsTree() {
  var treeEl = document.getElementById("fsTree");
  if (!treeEl || typeof virtualFS === 'undefined') return;
  treeEl.innerHTML = "";
  Object.keys(virtualFS).forEach(function (path) {
    var item = document.createElement("div");
    item.className = "fs-item";
    item.innerText = "📄 " + path + " (" + virtualFS[path].length + "b)";
    treeEl.appendChild(item);
  });
}

function loadSelectedExample() {
  var sel = document.getElementById("exampleSelector").value;
  editor.setValue(codeExamples[sel] || "");
}

setInterval(function () {
  var tempEl = document.getElementById("cpuTempReadout");
  if (tempEl && typeof System !== 'undefined') {
    tempEl.innerText = System.getTemperature() + " °C";
  }
}, 2000);

window.onload = function () {
  loadSelectedExample();
  changeDeviceProfile();
  renderFsTree();
  rebootSystem();
};