#include "TouchScreen.h"

XPT2046_Touchscreen touchScreen(TOUCHSCREEN_CS_PIN, TOUCHSCREEN_IRQ_PIN);

TFT_eSPI* TouchScreen::tftInstance = nullptr;

bool TouchScreen::touched = false;

void TouchScreen::init(TFT_eSPI *tft){
    SPI.begin(TOUCHSCREEN_SCLK_PIN, TOUCHSCREEN_MISO_PIN, TOUCHSCREEN_MOSI_PIN);
    touchScreen.begin();
    touchScreen.setRotation(0);
    tftInstance = tft;
}

bool TouchScreen::isTouched(){

    touched = touchScreen.touched();

    return touched;
}

bool TouchScreen::getTouch(uint16_t *x, uint16_t *y){
    uint16_t width, height;

    if (isTouched()) {
        bool processNow = false;

        width = tftInstance->width();
        height = tftInstance->height();

        TS_Point p = touchScreen.getPoint();

        int screen_x = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, width - 1);
        int screen_y = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, height - 1);

        screen_x = width - 1 - screen_x;

        *x = constrain(screen_x, 0, width - 1);
        *y = constrain(screen_y, 0, height - 1);

        return true;
    }

    return false;
}