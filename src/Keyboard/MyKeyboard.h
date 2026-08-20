#ifndef MY_KEYBOARD_H
#define MY_KEYBOARD_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "boards/Board.h"

class MyKeyboard {
public:
    static String getString(String initialText, String promptMsg, int maxLen = 30);

private:
    static void drawKeyboard(String currentText, String promptMsg, bool caps, int selectedX, int selectedY);
    static void handleTouch(uint16_t x, uint16_t y, String &currentText, bool &caps, bool &done);
};

#endif // MY_KEYBOARD_H
