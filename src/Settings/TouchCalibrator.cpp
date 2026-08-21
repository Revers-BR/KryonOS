#include "TouchCalibrator.h"
#include "../File System/FileSystem.h"

extern int currentState;

// Instâncias externas declaradas nos arquivos de board
#ifdef TARGET_CYD
#include <XPT2046_Bitbang.h>
extern XPT2046_Bitbang touchscreen;
#elif defined(TARGET_T_HMI)
#include <XPT2046_Touchscreen.h>
extern XPT2046_Touchscreen ts;
#endif

void TouchCalibrator::drawTarget(int16_t x, int16_t y, uint16_t color) {
    tft.drawCircle(x, y, 8, color);
    tft.drawFastHLine(x - 12, y, 25, color);
    tft.drawFastVLine(x, y - 12, 25, color);
}

void TouchCalibrator::waitForTouchRelease() {
    while (isTouched()) {
        delay(10);
    }
    delay(100);
}

bool TouchCalibrator::waitForTouchPress(uint16_t *rawX, uint16_t *rawY, uint32_t timeoutMs) {
    uint32_t start = millis();
    
    while ((millis() - start) < timeoutMs) {
        if (isTouched()) {
            #ifdef TARGET_CYD
                TouchPoint p = touchscreen.getTouch();
                *rawX = p.xRaw;
                *rawY = p.yRaw;
            #elif defined(TARGET_T_HMI)
                TS_Point p = ts.getPoint();
                *rawX = p.x;
                *rawY = p.y;
            #else
                // Fallback genérico para novas boards
                uint16_t dummyX, dummyY;
                getTouch(&dummyX, &dummyY);
                *rawX = dummyX;
                *rawY = dummyY;
            #endif

            waitForTouchRelease();
            return true;
        }
        delay(10);
    }
    return false; // Timeout
}

void TouchCalibrator::runCalibration() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Touch Calibration", DISP_HOR_RES / 2, 30, 2);
    tft.drawString("Toque nos 4 alvos", DISP_HOR_RES / 2, 50, 2);

    // Margem das bordas para os alvos de calibração
    const int offset = 20;
    int16_t targets[4][2] = {
        { offset, offset },                                 // Topo-Esquerda (0)
        { (int16_t)(DISP_HOR_RES - offset), offset },       // Topo-Direita (1)
        { offset, (int16_t)(DISP_VER_RES - offset) },       // Base-Esquerda (2)
        { (int16_t)(DISP_HOR_RES - offset), (int16_t)(DISP_VER_RES - offset) } // Base-Direita (3)
    };

    uint16_t rawX[4], rawY[4];

    waitForTouchRelease();

    for (int i = 0; i < 4; i++) {
        drawTarget(targets[i][0], targets[i][1], TFT_RED);
        
        if (!waitForTouchPress(&rawX[i], &rawY[i])) {
            // Se der timeout, aborta e volta ao launcher
            currentState = 0;
            return;
        }

        drawTarget(targets[i][0], targets[i][1], TFT_BLACK);
    }

    // Calcula os limites médios RAW (X_MIN, X_MAX, Y_MIN, Y_MAX)
    uint16_t calData[4];
    calData[0] = (rawX[0] + rawX[2]) / 2; // X Min
    calData[1] = (rawX[1] + rawX[3]) / 2; // X Max
    calData[2] = (rawY[0] + rawY[1]) / 2; // Y Min
    calData[3] = (rawY[2] + rawY[3]) / 2; // Y Max

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Calibracao Salva!", DISP_HOR_RES / 2, DISP_VER_RES / 2, 2);

    // Salva os 4 parâmetros no FileSystem (SPIFFS/LittleFS/NVS)
    FileSystem::writeCalData(calData);

    delay(1000);
    currentState = 0; // Retorna para o Launcher
}