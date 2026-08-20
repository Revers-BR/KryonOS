#include "LauncherUI.h"
#include "../Kernel/Core/HarixKernel.h"
#include "../File System/FileSystem.h"

extern int currentState;

String LauncherUI::appPaths[50];
String LauncherUI::appNames[50];
bool   LauncherUI::appIsFolder[50];
int LauncherUI::appCount = 0;
int LauncherUI::selectedIndex = 1;
int LauncherUI::scrollOffset = 0;
bool LauncherUI::needsRescan = true;

int launcherSubMenu = 0;

void LauncherUI::requestRescan() {
    needsRescan = true;
}

void LauncherUI::scanLocalApps() {
    appCount = 0;
    
    // Scan both /local/apps/ and /sd/apps/ for folder-based apps and legacy .js files
    const char* appDirs[] = { "/local/apps/", "/sd/apps/" };
    
    for (int d = 0; d < 2; d++) {
        if (!FileSystem::exists(appDirs[d])) continue;
        
        FileEntry entries[50];
        int count = FileSystem::listDirectory(appDirs[d], entries, 50);
        
        for (int i = 0; i < count && appCount < 50; i++) {
            // Draw loading bar
            tft.fillRect(20, 200, (i * 200) / count, 10, TFT_GREEN);
            
            if (entries[i].isDir) {
                // Check if it's an app package (has app.json)
                String appJsonPath = entries[i].path;
                if (!appJsonPath.endsWith("/")) appJsonPath += "/";
                appJsonPath += "app.json";
                
                if (FileSystem::exists(appJsonPath.c_str())) {
                    // Read app name from app.json
                    String jsonContent = FileSystem::readTextFile(appJsonPath.c_str());
                    String name = FileSystem::parseJsonValue(jsonContent, "name");
                    
                    if (name.length() > 0) {
                        // Check if this app is already in our list (avoid duplicates from SD + local)
                        bool duplicate = false;
                        for (int j = 0; j < appCount; j++) {
                            if (appNames[j] == name) { duplicate = true; break; }
                        }
                        if (duplicate) continue;
                        
                        appPaths[appCount] = entries[i].path;
                        appNames[appCount] = name;
                        appIsFolder[appCount] = true;
                        appCount++;
                    }
                }
            } else {
                // Legacy .js file support
                String fname = entries[i].name;
                if (fname.endsWith(".js")) {
                    // Check if this legacy app is already in our list
                    bool duplicate = false;
                    for (int j = 0; j < appCount; j++) {
                        if (appNames[j] == fname) { duplicate = true; break; }
                    }
                    if (duplicate) continue;
                    
                    appPaths[appCount] = entries[i].path;
                    appNames[appCount] = fname;
                    appIsFolder[appCount] = false;
                    appCount++;
                }
            }
        }
    }
}

// Auxiliar: Gera a lista sequencial de itens ignorando a paginação linear
int LauncherUI::getLauncherItems(LauncherItem* items, int maxItems) {
    int count = 0;

    // Seção System
    if (count < maxItems) items[count++] = {"[ SYSTEM ]", true, -1, -1};
    if (count < maxItems) items[count++] = {"App Store", false, 1, -1};
    if (count < maxItems) items[count++] = {"App Installer", false, 2, -1};
    if (count < maxItems) items[count++] = {"Settings", false, 3, -1};
    if (count < maxItems) items[count++] = {"Help Center", false, 4, -1};

    // Seção Apps
    if (appCount > 0) {
        if (count < maxItems) items[count++] = {"[ APPS ]", true, -1, -1};
        for (int i = 0; i < appCount && count < maxItems; i++) {
            items[count++] = {appNames[i], false, -1, i};
        }
    }

    return count;
}

void LauncherUI::draw() {
    tft.fillScreen(TFT_BLACK);

    if (needsRescan) {
        scanLocalApps();
        needsRescan = false;
        tft.fillScreen(TFT_BLACK);
    }

    if (tft.height() >= 240) {
        drawTall();
    } else {
        drawCompact();
    }
}

// Renderização para telas 240x320 (Layout em Grade Vertical)
void LauncherUI::drawTall() {
    // Moldura principal
    tft.drawRoundRect(3, 3, 234, 314, 5, TFT_WHITE);
    
    // Cabeçalho
    tft.fillRoundRect(6, 6, 228, 30, 5, TFT_BLACK);
    tft.drawRoundRect(6, 6, 228, 30, 5, TFT_GREEN);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("KryonOS Home", 120, 21, 2);

    LauncherItem items[100];
    int totalItems = getLauncherItems(items, 100);

    int itemsPerPage = 6;
    int startY = 42;
    int cardH = 36;
    int gapY = 4;

    for (int i = 0; i < itemsPerPage; i++) {
        int listIndex = scrollOffset + i;
        if (listIndex >= totalItems) break;

        LauncherItem item = items[listIndex];
        int yPos = startY + (i * (cardH + gapY));

        if (item.isHeader) {
            tft.fillRoundRect(8, yPos, 224, cardH, 4, TFT_DARKGREY);
            tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(item.label.c_str(), 120, yPos + (cardH / 2), 2);
        } else {
            bool isSelected = (listIndex == selectedIndex);
            uint16_t bg = isSelected ? TFT_WHITE : TFT_BLACK;
            uint16_t border = isSelected ? TFT_WHITE : TFT_DARKGREY;
            uint16_t textCol = isSelected ? TFT_BLACK : TFT_WHITE;

            tft.fillRoundRect(8, yPos, 224, cardH, 4, bg);
            tft.drawRoundRect(8, yPos, 224, cardH, 4, border);

            tft.setTextColor(textCol, bg);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(item.label.c_str(), 120, yPos + (cardH / 2), 2);
        }
    }

    // Touch Footer
    tft.drawRoundRect(5, 285, 230, 26, 4, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("UP   |   SEL   |   DN", 120, 298, 2);
}

// Renderização para telas 240x135 (Compact / Cardputer - Grade 2x2)
void LauncherUI::drawCompact() {
    // Header Minimalista
    tft.fillRoundRect(2, 2, 236, 18, 3, TFT_DARKGREY);
    tft.setTextColor(TFT_GREEN, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("KryonOS Home", 120, 10, 2);

    LauncherItem items[100];
    int totalItems = getLauncherItems(items, 100);

    int cols = 2;
    int rows = 2;
    int itemsPerPage = cols * rows; // 4 itens visíveis por página
    int cardW = 112;
    int cardH = 34;
    int startY = 24;

    for (int i = 0; i < itemsPerPage; i++) {
        int listIndex = scrollOffset + i;
        if (listIndex >= totalItems) break;

        LauncherItem item = items[listIndex];

        int r = i / cols;
        int c = i % cols;
        int xPos = 5 + c * (cardW + 6);
        int yPos = startY + r * (cardH + 4);

        if (item.isHeader) {
            tft.fillRoundRect(xPos, yPos, cardW, cardH, 4, TFT_DARKGREY);
            tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(item.label.c_str(), xPos + (cardW / 2), yPos + (cardH / 2), 1);
        } else {
            bool isSelected = (listIndex == selectedIndex);
            uint16_t bg = isSelected ? TFT_WHITE : TFT_BLACK;
            uint16_t border = isSelected ? TFT_WHITE : TFT_DARKGREY;
            uint16_t textCol = isSelected ? TFT_BLACK : TFT_WHITE;

            tft.fillRoundRect(xPos, yPos, cardW, cardH, 4, bg);
            tft.drawRoundRect(xPos, yPos, cardW, cardH, 4, border);

            tft.setTextColor(textCol, bg);
            tft.setTextDatum(MC_DATUM);
            
            // Trunca nome do app se for muito grande
            String displayName = item.label;
            if (displayName.length() > 12) {
                displayName = displayName.substring(0, 10) + "..";
            }
            tft.drawString(displayName.c_str(), xPos + (cardW / 2), yPos + (cardH / 2), 2);
        }
    }

    // Rodapé minimalista com indicação de navegação
    tft.fillRoundRect(5, 104, 230, 26, 4, TFT_NAVY);
    tft.drawRoundRect(5, 104, 230, 26, 4, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Use Arrows / Enter to Open", 120, 117, 2);
}

static void runApp(const String& path, bool isFolder) {
    extern int currentState;
    currentState = 2; // STATE_RUN_APP
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    
    String filePath;
    if (isFolder) {
        filePath = path;
        if (!filePath.endsWith("/")) filePath += "/";
        filePath += "main.js";
    } else {
        filePath = path;
    }
    
    HarixKernel::runFile(filePath.c_str());
    
    // Draw exit button
    tft.fillRoundRect(200, 0, 40, 30, 5, TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("X", 220, 15, 2);
}

// Executa a opção selecionada no momento
void LauncherUI::executeSelectedItem() {
    extern int currentState;
    
    Serial.printf("[UI] Executando item selecionado ID: %d\n", selectedIndex);

    if (selectedIndex == 0 || selectedIndex == 1) {
        currentState = 13; // STATE_APP_STORE
    } else if (selectedIndex == 2) {
        currentState = 3;  // STATE_INSTALLER
    } else if (selectedIndex == 3) {
        currentState = 1;  // STATE_SETTINGS
    } else if (selectedIndex == 4) {
        currentState = 14; // STATE_HELP_CENTER
    } else if (selectedIndex > 5) {
        int appIndex = selectedIndex - 6;
        if (appIndex >= 0 && appIndex < appCount) {
            runApp(appPaths[appIndex], appIsFolder[appIndex]);
        }
    }
}

// Navega para CIMA na lista
void LauncherUI::navigateUp() {
    if (selectedIndex > 0) {
        selectedIndex--;
        if (selectedIndex == 5) selectedIndex--; // Pula o header de Apps (Item 5)
        
        if (selectedIndex < scrollOffset) {
            scrollOffset = selectedIndex;
        }
    } else {
        // Se já está no topo, força a rolagem para o início
        selectedIndex = 0;
        scrollOffset = 0;
    }
    
    draw(); // Redesenha obrigatoriamente
}

// Navega para BAIXO na lista
void LauncherUI::navigateDown(int totalItems) {
    int maxVisible = 3; // No Cardputer (240x135) cabem cerca de 3 a 4 itens visíveis

    if (selectedIndex < totalItems - 1) {
        selectedIndex++;
        if (selectedIndex == 5) selectedIndex++; // Pula o header de Apps (Item 5)
        
        if (selectedIndex >= scrollOffset + maxVisible) {
            scrollOffset = selectedIndex - (maxVisible - 1);
        }
    }
    
    draw(); // Redesenha obrigatoriamente
}

void LauncherUI::handleKeyInput(BoardKey key) {
    extern int currentState;
    int totalItems = appCount + 6;

    if (key == BOARD_KEY_UP) {
        Serial.println("[UI] Mover CIMA");
        navigateUp();
    } 
    else if (key == BOARD_KEY_DOWN) {
        Serial.println("[UI] Mover BAIXO");
        navigateDown(totalItems);
    } 
    else if (key == BOARD_KEY_ENTER) {
        Serial.println("[UI] Executar Item");
        executeSelectedItem();
    } 
    else if (key == BOARD_KEY_ESC) {
        Serial.println("[UI] Voltar para Installer");
        currentState = 0;
    }
}

void LauncherUI::handleTouch(uint16_t x, uint16_t y) {
    extern int currentState;
    int totalItems = appCount + 6;

    // 1. TOUCH NOS ITENS DA LISTA (y entre 45 e 270)
    if (y >= 45 && y <= 270) {
        int clickedRelativeIndex = (y - 45) / 30;
        int clickedAbsoluteIndex = scrollOffset + clickedRelativeIndex;

        // Evita clicar em headers desativados (0 e 5)
        if (clickedAbsoluteIndex < totalItems && clickedAbsoluteIndex != 0 && clickedAbsoluteIndex != 5) {
            selectedIndex = clickedAbsoluteIndex;
            draw(); // Destaca o item selecionado na tela
            
            executeSelectedItem(); // REUTILIZADO: Executa a ação do item
        }
        return;
    }

    // 2. TOUCH NOS BOTÕES DO RODAPÉ (y entre 285 e 315)
    if (y >= 285 && y <= 315) {
        if (x < 60) { 
            // Botão UP
            navigateUp(); // REUTILIZADO: Sobe na lista e redesenha
        } 
        else if (x > 60 && x < 180) { 
            // Botão SEL
            executeSelectedItem(); // REUTILIZADO: Executa o item selecionado
        } 
        else if (x > 180) { 
            // Botão DN
            navigateDown(totalItems); // REUTILIZADO: Desce na lista e redesenha
        }
        return;
    }

    // 3. TOUCH NO HEADER (Toque rápido no topo abre o Installer)
    if (y < 40) {
        currentState = 3; // STATE_INSTALLER
        return;
    }
}