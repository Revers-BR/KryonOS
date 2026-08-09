#ifndef TOUCH_SCREEN_H
#define TOUCH_SCREEN_H

#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3900
#define TOUCH_Y_MIN 150
#define TOUCH_Y_MAX 3900

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

class TouchScreen {
public:
    static void init(TFT_eSPI *tft);
    static bool getTouch(uint16_t *x, uint16_t *y);
    static bool isTouched();

private:
    static bool touched;
    static TFT_eSPI *tftInstance;
};

#endif
