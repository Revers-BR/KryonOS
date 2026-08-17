#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h>
#include <FS.h>           // <--- Importante para utilizar o ponteiro fs::FS
#include <TFT_eSPI.h>

// Instância global do driver TFT_eSPI
extern TFT_eSPI tft;

// --- Ciclo de Vida da Placa ---
void initHardware(void);
void initDisplay(void);
void initTouch(void);

// --- Armazenamento (SD Card) ---
fs::FS* initSD(void);
void deinitSD(void);
uint64_t getSDTotalBytes(void);
uint64_t getSDUsedBytes(void);

// --- Interface Direta de Leitura do Touch ---
bool isTouched(void);
bool getTouch(uint16_t *x, uint16_t *y);

// --- Recursos do Hardware CYD / Outros ---
void setBacklight(uint8_t brightness);
void setRGBLED(uint8_t red, uint8_t green, uint8_t blue, bool true_color = true);

#endif // BOARD_H