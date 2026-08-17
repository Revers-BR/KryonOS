#include "Board.h"
#include "BoardConfig.h" // Importa as constantes locais
#include <SPI.h>
#include <SD_MMC.h>
#include <XPT2046_Touchscreen.h>

#ifdef TARGET_T_HMI

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

        // Mapeamento mantendo a inversão do eixo X (Maior RAW = Esquerda, Menor RAW = Direita)
        int16_t mapped_x = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, DISP_HOR_RES - 1, 0);
        int16_t mapped_y = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, DISP_VER_RES - 1);

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

    // Liga o Backlight
#if defined(TFT_BL)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif

    Serial.println("[Board] Hardware T-HMI Inicializado.");
}

void initDisplay(void) {
    Serial.println("[Board T-HMI] Inicializando Display ST7789 (TFT_eSPI)...");

    tft.init();
    tft.setRotation(0); // 0 = Portrait
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

fs::FS* initSD(void) {
    if (SD_MMC.cardType() != CARD_NONE) {
        return &SD_MMC;
    }

    Serial.println("[Board T-HMI] Inicializando Cartão SD (SD_MMC 1-bit)...");

    SD_MMC.setPins(SD_SCLK_PIN, SD_MOSI_PIN, SD_MISO_PIN);

    // Tenta montar o cartão no modo 1-bit (segundo parâmetro = true)
    if (!SD_MMC.begin("/sd", true)) {
        Serial.println("[Board T-HMI] Falha ao montar o SD. Verifique os pinos e a formatação (FAT32).");
        return nullptr;
    }

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[Board T-HMI] Nenhum cartão SD detectado.");
        return nullptr;
    }

    Serial.printf("[Board T-HMI] SD Montado com Sucesso. Tamanho: %llu MB\n", SD_MMC.cardSize() / (1024 * 1024));
    return &SD_MMC;
}

void deinitSD(void) {
    if (SD_MMC.cardType() != CARD_NONE) {
        SD_MMC.end();
        Serial.println("[Board T-HMI] Cartão SD desmontado com sucesso.");
    }
}

uint64_t getSDTotalBytes(void) {
    if (SD_MMC.cardType() == CARD_NONE) return 0;
    return SD_MMC.totalBytes();
}

uint64_t getSDUsedBytes(void) {
    if (SD_MMC.cardType() == CARD_NONE) return 0;
    return SD_MMC.usedBytes();
}

#endif