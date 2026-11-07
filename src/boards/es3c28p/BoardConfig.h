#ifndef ES3C28P_BOARD_CONFIG_H
#define ES3C28P_BOARD_CONFIG_H

#include <Arduino.h>

// --- Resolução da Tela ---
#ifndef DISP_HOR_RES
#define DISP_HOR_RES TFT_WIDTH   // 240
#endif

#ifndef DISP_VER_RES
#define DISP_VER_RES TFT_HEIGHT  // 320
#endif

// --- Alimentação e Backlight ---
#define BAT_ADC_PIN 9

// --- Pinos do Touch Capacitivo (FT6336G, I2C) ---
#define TOUCHSCREEN_SDA_PIN 16
#define TOUCHSCREEN_SCL_PIN 15
#define TOUCHSCREEN_RST_PIN 18
#define TOUCHSCREEN_INT_PIN 17

// --- Pinos do Cartão SD (SDIO 4 bits) ---
#define SD_CLK_PIN  38
#define SD_CMD_PIN  40
#define SD_D0_PIN   39
#define SD_D1_PIN   41
#define SD_D2_PIN   48
#define SD_D3_PIN   47

// --- Outros periféricos ---
#define PIN_RGB_LED 42

#endif // ES3C28P_BOARD_CONFIG_H