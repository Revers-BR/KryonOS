#pragma once

#include <Arduino.h>
#include "Board.h"

class WebServerAppUI {
private:

public:
    static void init(TFT_eSPI *tft);
    static void draw();
    static void handleTouch(uint16_t x, uint16_t y);
};
