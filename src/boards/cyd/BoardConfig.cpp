#include "Board.h"
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#ifdef TARGET_CYD

// --- Pinos do Touch (VSPI) ---
#ifndef TOUCHSCREEN_SCLK_PIN
#define TOUCHSCREEN_SCLK_PIN 25
#endif

#ifndef TOUCHSCREEN_MISO_PIN
#define TOUCHSCREEN_MISO_PIN 39
#endif

#ifndef TOUCHSCREEN_MOSI_PIN
#define TOUCHSCREEN_MOSI_PIN 32
#endif

#ifndef TOUCHSCREEN_CS_PIN
#define TOUCHSCREEN_CS_PIN 33
#endif

#ifndef TOUCHSCREEN_IRQ_PIN
#define TOUCHSCREEN_IRQ_PIN 36
#endif

// --- Constantes de Calibração Raw (ADC) do Touch ---
#define TS_MIN_X 300
#define TS_MAX_X 3800
#define TS_MIN_Y 300
#define TS_MAX_Y 3800

// Instância do Display (TFT_eSPI) acessível globalmente
TFT_eSPI tft = TFT_eSPI();

// Instância do Barramento SPI e do Touch Controler
static SPIClass touchSPI(VSPI);
static XPT2046_Touchscreen ts(TOUCHSCREEN_CS_PIN, TOUCHSCREEN_IRQ_PIN);

// --- Interface de Leitura Direta do Touch ---

bool isTouched(void) {
    return ts.touched();
}

bool getTouch(uint16_t *x, uint16_t *y) {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();

        // Mapeia os valores RAW lidos do sensor para as dimensões do display
        int16_t mapped_x = map(p.x, TS_MIN_X, TS_MAX_X, 0, DISP_HOR_RES);
        int16_t mapped_y = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, DISP_VER_RES);

        // Aplica limites de segurança para evitar estouros de tela
        *x = (uint16_t)constrain(mapped_x, 0, DISP_HOR_RES - 1);
        *y = (uint16_t)constrain(mapped_y, 0, DISP_VER_RES - 1);

        return true;
    }
    return false;
}

// --- Métodos do Ciclo de Vida da Board ---

void initHardware(void) {
    Serial.println("[Board CYD] Inicializando Pinos e Hardware...");

    // Garante que o Pino CS do Display comece desativado (HIGH)
#if defined(TFT_CS)
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
#endif

    // Liga o Backlight do Display
#if defined(TFT_BL)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
#endif

    Serial.println("[Board CYD] Hardware Inicializado.");
}

void initDisplay(void) {
    Serial.println("[Board CYD] Inicializando Display via TFT_eSPI...");

    tft.init();
    tft.setRotation(0); // 0 = Portrait, 1 = Landscape
    tft.fillScreen(TFT_BLACK);
}

void initTouch(void) {
    Serial.println("[Board CYD] Inicializando Touch XPT2046 (VSPI)...");

    // Inicializa o barramento VSPI exclusivo do Touch
    touchSPI.begin(TOUCHSCREEN_SCLK_PIN, TOUCHSCREEN_MISO_PIN, TOUCHSCREEN_MOSI_PIN, -1);
    
    // Inicializa o controlador Touch
    ts.begin(touchSPI);
    ts.setRotation(0); // Sincroniza rotação com o display

    Serial.println("[Board CYD] Touch XPT2046 Pronto!");
}

#endif