#ifndef LAUNCHER_UI_H
#define LAUNCHER_UI_H

#include <TFT_eSPI.h>
#include <Arduino.h>
#include "boards/Board.h"

// Estrutura de item do Launcher para simplificar a grade
struct LauncherItem {
    String label;
    bool isHeader;
    int systemActionId; // -1 se for app da lista
    int appIndex;       // Índice no array appNames
};

class LauncherUI {
public:
    static void draw();
    static void handleTouch(uint16_t x, uint16_t y);
    static void handleKeyInput(BoardKey key);
    static void executeSelectedItem();
    static void navigateUp();
    static void navigateDown(int totalItems);
    static void requestRescan();
    static void scanLocalApps();
    static bool needsRescan;

private:
    static TFT_eSPI *tftInstance;

    static void drawCompact();
    static void drawTall();
    static int getLauncherItems(LauncherItem* items, int maxItems);
    
    static String appPaths[50];   // Path to app folder or .js file
    static String appNames[50];   // Display name (from app.json or filename)
    static bool   appIsFolder[50]; // true = folder app, false = legacy .js
    static int appCount;
    static int selectedIndex;
    static int scrollOffset;
};

#endif // LAUNCHER_UI_H
