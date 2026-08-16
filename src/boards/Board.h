#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h>
#include <TFT_eSPI.h>

// Instância global do driver TFT_eSPI
extern TFT_eSPI tft;

// --- Ciclo de Vida da Placa ---
void initHardware(void);
void initDisplay(void);
void initTouch(void);

// --- Interface Direta de Leitura do Touch ---
bool isTouched(void);
bool getTouch(uint16_t *x, uint16_t *y);

// --- Recursos do Hardware CYD ---
void setBacklight(uint8_t brightness);
void setRGBLED(uint8_t red, uint8_t green, uint8_t blue, bool true_color = true);

#endif // BOARD_H