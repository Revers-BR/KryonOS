#include "Board.h"
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#ifdef TARGET_T_HMI

// --- Definição dos Pinos de Touch (T-HMI) ---
#ifndef TOUCHSCREEN_SCLK_PIN
#define TOUCHSCREEN_SCLK_PIN 12
#endif

#ifndef TOUCHSCREEN_MISO_PIN
#define TOUCHSCREEN_MISO_PIN 13
#endif

#ifndef TOUCHSCREEN_MOSI_PIN
#define TOUCHSCREEN_MOSI_PIN 11
#endif

#ifndef TOUCHSCREEN_CS_PIN
#define TOUCHSCREEN_CS_PIN 14
#endif

#ifndef TOUCHSCREEN_IRQ_PIN
#define TOUCHSCREEN_IRQ_PIN 15
#endif

// Instância do Display (TFT_eSPI) acessível globalmente
TFT_eSPI tft = TFT_eSPI();

// Instância do Touch e Barramento SPI (FSPI no ESP32-S3)
static SPIClass touchSPI(FSPI);
static XPT2046_Touchscreen ts(TOUCHSCREEN_CS_PIN, TOUCHSCREEN_IRQ_PIN);

// --- Métodos Globais de Leitura Direta do Touch ---

bool isTouched(void) {
    return ts.touched();
}

bool getTouch(uint16_t *x, uint16_t *y) {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();

        // Mapeamento dos valores do ADC para as dimensões da tela
        int16_t mapped_x = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, DISP_HOR_RES, 0);
        int16_t mapped_y = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, DISP_VER_RES);

        *x = (uint16_t)constrain(mapped_x, 0, DISP_HOR_RES - 1);
        *y = (uint16_t)constrain(mapped_y, 0, DISP_VER_RES - 1);

        return true;
    }
    return false;
}

// --- Métodos de Inicialização da Placa T-HMI ---

void initHardware(void) {
    Serial.println("[Board T-HMI] Inicializando Pinos e Backlight...");

#if defined(PWR_ON_PIN)
    pinMode(PWR_ON_PIN, OUTPUT);
    digitalWrite(PWR_ON_PIN, HIGH);
#endif
#if defined(PWR_EN_PIN)
    pinMode(PWR_EN_PIN, OUTPUT);
    digitalWrite(PWR_EN_PIN, HIGH);
#endif
    delay(100);

    // Garante CS do Touch em nível alto
    pinMode(TOUCHSCREEN_CS_PIN, OUTPUT);
    digitalWrite(TOUCHSCREEN_CS_PIN, HIGH);

    // Liga o Backlight do ST7789
#if defined(TFT_BL)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
#endif

    Serial.println("[Board] Hardware T-HMI Inicializado.");
}

void initDisplay(void) {
    Serial.println("[Board T-HMI] Inicializando Display ST7789 (TFT_eSPI)...");

    tft.init();
    tft.setRotation(0); // 0 = Portrait, 1 = Landscape
    tft.setSwapBytes(true);
    tft.fillScreen(TFT_BLACK);
}

void initTouch(void) {
    Serial.println("[Board T-HMI] Inicializando Touch XPT2046 (FSPI)...");

    touchSPI.begin(TOUCHSCREEN_SCLK_PIN, TOUCHSCREEN_MISO_PIN, TOUCHSCREEN_MOSI_PIN, -1);
    ts.begin(touchSPI);
    ts.setRotation(0);

    Serial.println("[Board T-HMI] Touch Pronto!");
}

#endif