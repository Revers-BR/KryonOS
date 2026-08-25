// =========================================================================
// SIGNATURES.JS - POP-OVER HELP & INTELISENSE METHOD SIGNATURES
// =========================================================================

var methodSignatures = {
  // --- SYSTEM API ---
  "System.screenWidth": "System.screenWidth() : Number - Retorna a largura do display (240px).",
  "System.screenHeight": "System.screenHeight() : Number - Retorna a altura do display (320px).",
  "System.getOSVersion": "System.getOSVersion() : String - Retorna a versão atual do KryonOS.",
  "System.getAPILevel": "System.getAPILevel() : Number - Retorna o nível de compatibilidade da API.",
  "System.millis": "System.millis() : Number - Retorna o tempo em milissegundos desde o boot.",
  "System.micros": "System.micros() : Number - Retorna o tempo em microssegundos desde o boot.",
  "System.getTemperature": "System.getTemperature() : Number - Retorna a temperatura atual do CPU em °C.",
  "System.delay": "System.delay(ms) : Promise - Pausa a execução de forma assíncrona pelo tempo especificado.",
  "System.print": "System.print(msg) : Void - Envia uma mensagem de log para o Monitor Serial.",
  "System.getTouch": "System.getTouch() : Object - Retorna { x, y, touched } indicando o estado do toque.",
  "System.getInfo": "System.getInfo() : Object - Retorna { heap, freeRAM, cpuFreq, temp }.",
  "System.getIPAddress": "System.getIPAddress() : String - Retorna o IP atual do dispositivo.",
  "System.isWiFiActive": "System.isWiFiActive() : Boolean - Retorna verdadeiro se o Wi-Fi estiver conectado.",
  "System.restart": "System.restart() : Void - Reinicia o emulador / hardware.",
  "System.getTime": "System.getTime() : String - Retorna o horário atual formatado (HH:MM:SS).",
  "System.getDate": "System.getDate() : String - Retorna a data atual formatada (DD/MM/YYYY).",

  // --- GRAPHICS API ---
  "System.color": "System.color(r, g, b) : Number - Converte RGB (0-255) em formato RGB565 (16-bit).",
  "System.fillScreen": "System.fillScreen(color) : Void - Preenche a tela inteira com uma cor RGB565.",
  "System.drawPixel": "System.drawPixel(x, y, color) : Void - Desenha um ponto isolado.",
  "System.drawLine": "System.drawLine(x0, y0, x1, y1, color) : Void - Desenha uma linha reta entre dois pontos.",
  "System.drawFastHLine": "System.drawFastHLine(x, y, w, color) : Void - Desenha uma linha horizontal rápida.",
  "System.drawFastVLine": "System.drawFastVLine(x, y, h, color) : Void - Desenha uma linha vertical rápida.",
  "System.drawRect": "System.drawRect(x, y, w, h, color) : Void - Desenha a borda de um retângulo.",
  "System.fillRect": "System.fillRect(x, y, w, h, color) : Void - Preenche uma área retangular com cor.",
  "System.drawRoundRect": "System.drawRoundRect(x, y, w, h, r, color) : Void - Desenha retângulo com cantos arredondados.",
  "System.fillRoundRect": "System.fillRoundRect(x, y, w, h, r, color) : Void - Preenche retângulo com cantos arredondados.",
  "System.drawCircle": "System.drawCircle(x, y, r, color) : Void - Desenha a borda de um círculo.",
  "System.fillCircle": "System.fillCircle(x, y, r, color) : Void - Preenche um círculo.",
  "System.drawTriangle": "System.drawTriangle(x0, y0, x1, y1, x2, y2, color) : Void - Desenha um triângulo.",
  "System.fillTriangle": "System.fillTriangle(x0, y0, x1, y1, x2, y2, color) : Void - Preenche um triângulo.",

  // --- SPRITE API ---
  "System.createSprite": "System.createSprite(w, h) : Void - Aloca um buffer de memória off-screen (Sprite).",
  "System.bindSprite": "System.bindSprite(enable) : Void - Direciona todos os desenhos para o Sprite (true) ou Canvas (false).",
  "System.pushSprite": "System.pushSprite(x, y) : Void - Renderiza o Sprite criado na tela principal.",
  "System.deleteSprite": "System.deleteSprite() : Void - Libera o buffer do Sprite da memória.",

  // --- KEYBOARD API (Duktape / Native Bindings) ---
  "System.getKey": "System.getKey() : String - Retorna a tecla ativa ('UP', 'DOWN', 'ENTER', 'ESC', 'NONE'). Lança 'OS_EXIT' no ESC.",
  "System.isKeyPressed": "System.isKeyPressed(keyName) : Boolean - Checa se uma tecla ('UP', 'A', etc) está pressionada.",
  "System.getKeyInput": "System.getKeyInput() : Object - Retorna { key, code, pressed } da entrada ativa.",
  "System.getChar": "System.getChar() : String - Retorna o caractere digitado (suporta '\\n', '\\t', '\\b').",

  // --- TEXT API ---
  "System.setTextColor": "System.setTextColor(fgColor, [bgColor]) : Void - Define as cores de texto principal e fundo.",
  "System.setTextSize": "System.setTextSize(size) : Void - Ajusta a escala da fonte (1, 2, 3, etc.).",
  "System.drawString": "System.drawString(text, x, y, [size]) : Void - Renderiza uma string na posição informada.",

  // --- GPIO API ---
  "System.gpio.pinMode": "System.gpio.pinMode(pin, mode) : Void - Configura pino como 0 (INPUT) ou 1 (OUTPUT).",
  "System.gpio.digitalWrite": "System.gpio.digitalWrite(pin, val) : Void - Define saída em ALTO (1) ou BAIXO (0).",
  "System.gpio.digitalRead": "System.gpio.digitalRead(pin) : Number - Lê valor digital do pino (0 ou 1).",
  "System.gpio.analogRead": "System.gpio.analogRead(pin) : Number - Lê entrada analógica do ADC (0 a 4095).",
  "System.gpio.analogWrite": "System.gpio.analogWrite(pin, val) : Void - Gera sinal PWM (0 a 255).",

  // --- FILE SYSTEM (FS) API ---
  "FS.exists": "FS.exists(path) : Boolean - Verifica se um arquivo existe na memória SPIFFS/FAT.",
  "FS.readTextFile": "FS.readTextFile(path) : String - Lê o conteúdo em texto plano de um arquivo.",
  "FS.writeTextFile": "FS.writeTextFile(path, content) : Boolean - Grava ou sobrescreve um arquivo de texto.",
  "FS.appendTextFile": "FS.appendTextFile(path, content) : Boolean - Anexa texto ao final de um arquivo existente.",
  "FS.deleteFile": "FS.deleteFile(path) : Boolean - Apaga permanentemente um arquivo.",
  "FS.listDir": "FS.listDir(path) : Array - Retorna lista com os caminhos dos arquivos gravados.",
  "FS.getFileSize": "FS.getFileSize(path) : Number - Retorna o tamanho do arquivo em bytes.",
  "FS.getFileMD5": "FS.getFileMD5(path) : String - Retorna a hash MD5 do arquivo para verificação de integridade."
};