#include "WebServerAppUI.h"
#include "../WebManager/WebManager.h"
#include "../File System/FileSystem.h"

void WebServerAppUI::draw() {
    
    
    tft.fillScreen(TFT_BLACK);
    
    // Draw the main border
    tft.drawRoundRect(3, 3, 234, 314, 5, TFT_WHITE);
    
    // Header Bar
    tft.fillRoundRect(6, 6, 228, 30, 5, TFT_BLACK);
    tft.drawRoundRect(6, 6, 228, 30, 5, TFT_CYAN);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Web Server Manager", 120, 21, 2);

    bool wifiDisabled = FileSystem::exists("/local/nowifi.txt");
    bool serverEnabled = FileSystem::exists("/local/web_on.txt");
    bool isConnected = WebManager::isActive();

    tft.setTextDatum(TL_DATUM);
    
    int y = 45;
    int spacing = 20;

    if (wifiDisabled) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("WiFi is DISABLED", 15, y, 2);
        y += spacing + 5;
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Please turn on WiFi", 15, y, 2);
        y += spacing;
        tft.drawString("to use Web Server.", 15, y, 2);
    } else if (!serverEnabled) {
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);
        tft.drawString("Web Server is OFF", 15, y, 2);
        y += spacing + 5;
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Turn it ON to access", 15, y, 2);
        y += spacing;
        tft.drawString("the file manager.", 15, y, 2);
    } else if (!isConnected) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("CONNECTION FAILED!", 15, y, 2);
        y += spacing + 5;
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Web server is ON but", 15, y, 2);
        y += spacing;
        tft.drawString("WiFi is not connected.", 15, y, 2);
    } else {
        String ip = WebManager::getIPAddress();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("Status:", 15, y, 2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("RUNNING", 80, y, 2);
        y += spacing + 5;

        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("IP:", 15, y, 2);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawString(ip, 80, y, 2);
        y += spacing;

        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("Port:", 15, y, 2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("80", 80, y, 2);
        y += spacing + 10;
        
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);
        tft.drawString("Visit the IP address", 15, y, 2);
        y += spacing;
        tft.drawString("in your web browser", 15, y, 2);
    }

    // Toggle Button (bottom area, above footer)
    tft.setTextDatum(MC_DATUM);
    if (!serverEnabled || wifiDisabled) {
        tft.fillRoundRect(60, 235, 120, 35, 4, TFT_DARKGREY);
        tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
        tft.drawString("Turn ON", 120, 252, 2);
    } else {
        tft.fillRoundRect(60, 235, 120, 35, 4, TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
        tft.drawString("Turn OFF", 120, 252, 2);
    }

    // Touch Footer
    tft.drawRoundRect(5, 285, 230, 30, 5, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("EXIT", 120, 300, 2);
}

void WebServerAppUI::handleTouch(uint16_t x, uint16_t y) {
    extern int currentState;

    // Toggle Button
    if (x >= 60 && x <= 180 && y >= 235 && y <= 270) {
        bool serverEnabled = FileSystem::exists("/local/web_on.txt");
        if (serverEnabled) {
            FileSystem::deleteFile("/local/web_on.txt");
        } else {
            FileSystem::writeTextFile("/local/web_on.txt", "1");
        }
        
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Rebooting to Apply...", 120, 160, 2);
        delay(1000);
        ESP.restart();
    }

    // Bottom Nav: EXIT
    if (y >= 285) {
        if (x > 60 && x < 180) {
            currentState = 0; // STATE_LAUNCHER
        }
    }
}
