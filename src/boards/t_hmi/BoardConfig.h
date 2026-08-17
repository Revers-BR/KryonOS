#ifndef T_HMI_BOARD_CONFIG_H
#define T_HMI_BOARD_CONFIG_H

#include <Arduino.h>

// --- Resolução da Tela ---
#ifndef DISP_HOR_RES
#define DISP_HOR_RES TFT_WIDTH
#endif

#ifndef DISP_VER_RES
#define DISP_VER_RES TFT_HEIGHT
#endif

// --- Alimentação e Backlight ---
#define PWR_EN_PIN  10
#define PWR_ON_PIN  14
#define TFT_BL      38 // Altere caso o backlight use outro pino no T-HMI

// --- Pinos do Touch Screen (XPT2046) ---
#define TOUCHSCREEN_SCLK_PIN 1
#define TOUCHSCREEN_MISO_PIN 4
#define TOUCHSCREEN_MOSI_PIN 3
#define TOUCHSCREEN_CS_PIN   2
#define TOUCHSCREEN_IRQ_PIN  9

// --- Calibração de Hardware do Touch (Valores RAW ADC) ---
#define TOUCH_X_MIN 800
#define TOUCH_X_MAX 3570
#define TOUCH_Y_MIN 350
#define TOUCH_Y_MAX 3850

// --- Pinos do Cartão SD ---
#define SD_MISO_PIN 13
#define SD_MOSI_PIN 11
#define SD_SCLK_PIN 12

#endif // T_HMI_BOARD_CONFIG_H