#include "boards/Board.h"
#include "BoardConfig.h"

#ifdef TARGET_ES3C28P

#include <Arduino.h>
#include <SD_MMC.h>
#include "touch.h"


// ============================================================
// DISPLAY
// ============================================================

TFT_eSPI tft = TFT_eSPI();

// ============================================================
// CAPACIDADES DA PLACA
// ============================================================

bool hasTouch(void) {
    return true;
}

bool hasKeyboard(void) {
    return false;
}

bool hasBattery(void) {
    return true;
}


// ============================================================
// TOUCH
// ============================================================

void initTouch(void) {
    Serial.println(
        "[Board ES3C28P] Inicializando Touch FT6336G..."
    );

    touch_init(DISP_HOR_RES, DISP_VER_RES, 0);

    Serial.println(
        "[Board ES3C28P] Touch Pronto!"
    );
}

bool isTouched(void) {
    return touch_touched();
}

bool getTouch(uint16_t *x, uint16_t *y) {
    static uint32_t circleTime = 0;
    static int16_t circleX = -1;
    static int16_t circleY = -1;
    static bool circleVisible = false;

    if (circleVisible && (millis() - circleTime >= 150)) {
        tft.fillCircle(circleX, circleY, 6, TFT_BLACK);
        circleVisible = false;
    }

    if (!touch_touched()) {
        return false;
    }

    *x = constrain((uint16_t)touch_last_x, 0, DISP_HOR_RES - 1);
    *y = constrain((uint16_t)touch_last_y, 0, DISP_VER_RES - 1);

    if (circleX != *x || circleY != *y) {
        tft.fillCircle(*x, *y, 5, TFT_RED);
        circleX = *x;
        circleY = *y;
        circleTime = millis();
        circleVisible = true;
    }

    Serial.printf("[TOUCH] X=%u Y=%u\n", *x, *y);

    return true;
}

// ============================================================
// HARDWARE
// ============================================================

void initHardware(void) {

    Serial.println("[Board ES3C28P] Inicializando Hardware...");

    // --------------------------------------------------------
    // Bateria
    // --------------------------------------------------------

    analogReadResolution(12);
    pinMode(BAT_ADC_PIN, INPUT);

    delay(100);

    // --------------------------------------------------------
    // Backlight
    // --------------------------------------------------------

#if defined(TFT_BL)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif

    Serial.println("[Board ES3C28P] Hardware Inicializado.");
}


// ============================================================
// BATERIA
// ============================================================

float getBatteryVoltage(void) {

    uint32_t raw_mv =
        analogReadMilliVolts(BAT_ADC_PIN);

    // Divisor resistivo da placa (ajuste o fator se necessário)
    float voltage =
        (raw_mv * 2.2f) / 1000.0f;

    return voltage;
}


int getBatteryPercent(void) {

    float v = getBatteryVoltage();

    if (v >= 4.2f)
        return 100;

    if (v <= 3.3f)
        return 0;

    int percent =
        (int)(
            (v - 3.3f) /
            (4.2f - 3.3f) *
            100.0f
        );

    return constrain(percent, 0, 100);
}


// ============================================================
// DISPLAY
// ============================================================

void initDisplay(void) {

    Serial.println(
        "[Board ES3C28P] Inicializando Display ILI9341V..."
    );

    tft.init();

    tft.setRotation(0);

    tft.fillScreen(TFT_BLACK);
}

// ============================================================
// SD
// ============================================================

fs::FS* initSD(void) {

    if (SD_MMC.cardType() != CARD_NONE) {
        return &SD_MMC;
    }

    Serial.println(
        "[Board ES3C28P] Inicializando SD_MMC 4-bit..."
    );

    SD_MMC.setPins(
        SD_CLK_PIN,
        SD_CMD_PIN,
        SD_D0_PIN,
        SD_D1_PIN,
        SD_D2_PIN,
        SD_D3_PIN
    );

    if (!SD_MMC.begin("/sd", false)) {

        Serial.println(
            "[Board ES3C28P] Falha ao montar o SD."
        );

        return nullptr;
    }

    uint8_t cardType =
        SD_MMC.cardType();

    if (cardType == CARD_NONE) {

        Serial.println(
            "[Board ES3C28P] Nenhum SD detectado."
        );

        return nullptr;
    }

    Serial.printf(
        "[Board ES3C28P] SD Montado. "
        "Tamanho: %llu MB\n",
        SD_MMC.cardSize() /
        (1024 * 1024)
    );

    return &SD_MMC;
}


void deinitSD(void) {

    if (SD_MMC.cardType() != CARD_NONE) {

        SD_MMC.end();

        Serial.println(
            "[Board ES3C28P] SD desmontado."
        );
    }
}


uint64_t getSDTotalBytes(void) {

    if (SD_MMC.cardType() == CARD_NONE)
        return 0;

    return SD_MMC.totalBytes();
}


uint64_t getSDUsedBytes(void) {

    if (SD_MMC.cardType() == CARD_NONE)
        return 0;

    return SD_MMC.usedBytes();
}


bool isSDMounted() {

    return SD_MMC.cardType() != CARD_NONE;
}


// ============================================================
// TECLADO
// ============================================================

BoardKey getKeyInput(void) {
    return BOARD_KEY_NONE;
}


void updateModifiers(BoardKey) {
}


void clearModifiers() {
}


char keyToChar(BoardKey) {
    return '\0';
}


bool isShiftActive() {
    return false;
}


bool isFnActive() {
    return false;
}


#endif