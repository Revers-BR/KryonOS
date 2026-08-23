#include <WiFi.h>
#include "LauncherUI.h"
#include "../Kernel/Core/HarixKernel.h"
#include "../File System/FileSystem.h"

extern int currentState;

LauncherItem LauncherUI::items[50]; 
String LauncherUI::appPaths[50];
String LauncherUI::appNames[50];
String LauncherUI::appCategories[50];
String LauncherUI::currentCategory;
int LauncherUI::totalItems = 0;
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
    
    const int16_t cy = DISP_VER_RES / 2;
    const int16_t barWidth = (DISP_HOR_RES * 80) / 100;
    const int16_t barHeight = 12;
    const int16_t barX = (DISP_HOR_RES - barWidth) / 2;
    const int16_t barY = cy + 15;

    const char* appDirs[] = { "/local/apps/", "/sd/apps/" };
    
    for (int d = 0; d < 2; d++) {
        if (!FileSystem::exists(appDirs[d])) continue;
        
        FileEntry entries[50];
        int count = FileSystem::listDirectory(appDirs[d], entries, 50);
        
        for (int i = 0; i < count && appCount < 50; i++) {
            if (count > 0) {
                int fillWidth = map(i + 1, 0, count, 0, barWidth - 4);
                tft.fillRect(barX + 2, barY + 2, fillWidth, barHeight - 4, TFT_GREEN);
            }

            if (entries[i].isDir) {
                String appJsonPath = entries[i].path;
                if (!appJsonPath.endsWith("/")) appJsonPath += "/";
                appJsonPath += "app.json";
                
                if (FileSystem::exists(appJsonPath.c_str())) {
                    String jsonContent = FileSystem::readTextFile(appJsonPath.c_str());
                    String name = FileSystem::parseJsonValue(jsonContent, "name");
                    String category = FileSystem::parseJsonValue(jsonContent, "category");
                    
                    if (category.length() == 0) category = "General"; // Categoria padrão

                    if (name.length() > 0) {
                        bool duplicate = false;
                        for (int j = 0; j < appCount; j++) {
                            if (appNames[j] == name) { duplicate = true; break; }
                        }
                        if (duplicate) continue;
                        
                        appPaths[appCount] = entries[i].path;
                        appNames[appCount] = name;
                        appCategories[appCount] = category; // Armazena a categoria
                        appIsFolder[appCount] = true;
                        appCount++;
                    }
                }
            } else {
                String fname = entries[i].name;
                if (fname.endsWith(".js")) {
                    bool duplicate = false;
                    for (int j = 0; j < appCount; j++) {
                        if (appNames[j] == fname) { duplicate = true; break; }
                    }
                    if (duplicate) continue;
                    
                    appPaths[appCount] = entries[i].path;
                    appNames[appCount] = fname;
                    appCategories[appCount] = "Legacy Scripts"; // Categoria para scripts isolados
                    appIsFolder[appCount] = false;
                    appCount++;
                }
            }
        }
    }
}

int LauncherUI::getLauncherItems(LauncherItem* items, int maxItems) {
    int count = 0;

    // NIVEL 1: Menu Principal (Se não houver categoria selecionada)
    if (currentCategory.length() == 0) {
        // Seção System
        if (count < maxItems) items[count++] = {"[ SYSTEM ]", true, ITEM_HEADER, -1, -1, ""};
        if (count < maxItems) items[count++] = {"App Store", false, ITEM_SYS, 1, -1, ""};
        if (count < maxItems) items[count++] = {"App Installer", false, ITEM_SYS, 2, -1, ""};
        if (count < maxItems) items[count++] = {"Settings", false, ITEM_SYS, 3, -1, ""};
        if (count < maxItems) items[count++] = {"Help Center", false, ITEM_SYS, 4, -1, ""};

        // Seção de Categorias
        if (appCount > 0) {
            if (count < maxItems) items[count++] = {"[ CATEGORIES ]", true, ITEM_HEADER, -1, -1, ""};

            // Agrupa e lista categorias únicas disponíveis
            String uniqueCategories[20];
            int categoryCount = 0;

            for (int i = 0; i < appCount; i++) {
                bool found = false;
                for (int c = 0; c < categoryCount; c++) {
                    if (uniqueCategories[c] == appCategories[i]) {
                        found = true;
                        break;
                    }
                }
                if (!found && categoryCount < 20) {
                    uniqueCategories[categoryCount++] = appCategories[i];
                }
            }

            // Adiciona as pastas de categorias à lista
            for (int c = 0; c < categoryCount && count < maxItems; c++) {
                String catName = "> " + uniqueCategories[c];
                items[count++] = {catName, false, ITEM_CATEGORY, -1, -1, uniqueCategories[c]};
            }
        }
    } 
    // NIVEL 2: Submenu da Categoria Selecionada
    else {
        if (count < maxItems) items[count++] = {"< Voltar", false, ITEM_BACK, -1, -1, ""};
        if (count < maxItems) items[count++] = {"[ " + currentCategory + " ]", true, ITEM_HEADER, -1, -1, ""};

        for (int i = 0; i < appCount && count < maxItems; i++) {
            if (appCategories[i] == currentCategory) {
                items[count++] = {appNames[i], false, ITEM_APP, -1, i, ""};
            }
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

// Desenha o Ícone do Wi-Fi dinamizando a cor e os níveis de sinal conforme o RSSI
void drawIconWiFi(int16_t x, int16_t y) {
    if (WiFi.status() == WL_CONNECTED) {
        int32_t rssi = WiFi.RSSI(); // Sinal em dBm
        
        // 1. Determina a cor ativa com base na força do sinal
        uint16_t activeColor = TFT_GREEN;
        if (rssi <= -75) {
            activeColor = TFT_RED;    // Sinal Fraco / Instável
        } else if (rssi <= -60) {
            activeColor = TFT_YELLOW; // Sinal Médio
        } else {
            activeColor = TFT_GREEN;  // Sinal Forte / Excelente
        }

        uint16_t inactiveColor = TFT_DARKGREY; // Cor das barras desligadas

        // 2. Determina a ativação dos arcos
        uint16_t colorDot  = activeColor; 
        uint16_t colorArc1 = (rssi > -85) ? activeColor : inactiveColor; // Nível 1
        uint16_t colorArc2 = (rssi > -70) ? activeColor : inactiveColor; // Nível 2
        uint16_t colorArc3 = (rssi > -55) ? activeColor : inactiveColor; // Nível 3

        // 3. Renderiza os arcos e o ponto central
        tft.drawCircleHelper(x + 6, y + 9, 6, 1, colorArc3); // Arco Externo
        tft.drawCircleHelper(x + 6, y + 9, 4, 1, colorArc2); // Arco Médio
        tft.drawCircleHelper(x + 6, y + 9, 2, 1, colorArc1); // Arco Interno
        tft.fillCircle(x + 6, y + 9, 1, colorDot);           // Ponto Central
    } else {
        // Desconectado: Exibe apenas um 'X' ou ponto vermelho
        tft.fillCircle(x + 6, y + 9, 1, TFT_RED);
    }
}

// Desenha o Ícone do SD Card com variação de cor por uso:
// Vazio (< 10%): Branco
// Normal (10% a 85%): Verde
// Cheio (> 85%): Vermelho
void drawIconSD(int16_t x, int16_t y) {
    if (isSDMounted()) { 
        uint64_t total = getSDTotalBytes();
        uint64_t used = getSDUsedBytes();
        
        // Define a cor padrão como Branco (Vazio / Sem dados suficientes)
        uint16_t color = TFT_WHITE; 

        if (total > 0) {
            // Calcula a porcentagem de ocupação (0 a 100%)
            int usagePercent = (int)((used * 100) / total);

            if (usagePercent > 85) {
                color = TFT_RED;    // Cheio (> 85%)
            } else if (usagePercent >= 10) {
                color = TFT_GREEN;  // Normal (10% - 85%)
            } else {
                color = TFT_WHITE;  // Vazio (< 10%)
            }
        }

        // Se a tela for pequena (ex: 240x135), usa o modelo básico visível
        if (tft.height() <= 135 || tft.width() <= 135) {
            
            // Modelo Básico (240x135):
            // Corpo preenchido para dar contraste (12x14px)
            tft.fillRect(x, y, 12, 16, color);
            
            // Canto chanfrado superior esquerdo (Corta o canto)
            tft.fillRect(x, y, 3, 5, TFT_BLACK);
            tft.drawLine(x + 3, y, x, y + 2, color);

            // Módulo de pinos (Três detalhes pretos no topo para identificar como SD)
            tft.drawFastVLine(x + 4, y + 2, 6, TFT_BLACK);
            tft.drawFastVLine(x + 7, y + 2, 6, TFT_BLACK);
            tft.drawFastVLine(x + 9, y + 2, 6, TFT_BLACK);

        } else {
            
            // Modelo Original Mantido Intacto (240x320)
            tft.drawRect(x, y, 9, 11, color);
            tft.drawLine(x + 2, y, x, y + 2, color); // Canto chanfrado do SD
            
            // Linhas dos pinos do SD
            tft.drawLine(x + 2, y + 2, x + 2, y + 4, color);
            tft.drawLine(x + 4, y + 2, x + 4, y + 4, color);
            tft.drawLine(x + 6, y + 2, x + 6, y + 4, color);
        }
    }
}

// Desenha o Ícone da Bateria dinâmico de acordo com a porcentagem
void drawIconBattery(int16_t x, int16_t y, uint16_t color) {
    // Corpo da bateria (14x8 pixels)
    tft.drawRect(x, y + 2, 14, 8, color);
    // Polo positivo
    tft.fillRect(x + 14, y + 4, 2, 4, color);

    // Obtém o percentual real da placa
    int percent = getBatteryPercent();

    // Mapeia 0 a 100% para uma largura de preenchimento de 0 a 10 pixels
    int fillWidth = map(percent, 0, 100, 0, 10);

    // Define a cor baseada no nível da bateria
    uint16_t fillColor = TFT_GREEN;
    if (percent <= 20) {
        fillColor = TFT_RED;    // Crítico
    } else if (percent <= 50) {
        fillColor = TFT_YELLOW; // Médio
    }

    // Limpa a área interna antes de desenhar o nível
    tft.fillRect(x + 2, y + 4, 10, 4, TFT_BLACK);

    // Desenha o preenchimento proporcional
    if (fillWidth > 0) {
        tft.fillRect(x + 2, y + 4, fillWidth, 4, fillColor);
    }
}

// Função para desenhar toda a barra de ícones no cabeçalho
void drawHeaderIcons(int16_t startX, int16_t y) {
    int currentX = startX;

    // 1. SD Card (lado esquerdo do bloco de ícones)
    if(isSDMounted()){
        drawIconSD(currentX, y);
        currentX += 14;
    }

    // 2. Wi-Fi
    if (WiFi.status() == WL_CONNECTED) {
        drawIconWiFi(currentX, y);
        currentX += 16;
    }

    // 3. Bateria
    if(hasBattery())drawIconBattery(currentX, y, TFT_WHITE);
}

// Renderização para telas 240x320 (Layout em Grade Vertical)
void LauncherUI::drawTall() {
    // Moldura principal
    tft.drawRoundRect(3, 3, 234, 314, 5, TFT_WHITE);
    
    // Cabeçalho
    tft.fillRoundRect(6, 6, 228, 30, 5, TFT_BLACK);
    tft.drawRoundRect(6, 6, 228, 30, 5, TFT_GREEN);
    
    // Texto do Título alinhado à esquerda para dar espaço aos ícones
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(TL_DATUM); // Top Left
    tft.drawString("KryonOS", 14, 14, 2);

    // Renderiza Ícones de Status no canto superior direito do cabeçalho
    drawHeaderIcons(170, 15);

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

void LauncherUI::drawCompact() {
    // Header Minimalista
    tft.fillRoundRect(2, 2, 236, 18, 3, TFT_DARKGREY);
    
    // Título ajustado à esquerda para caber os ícones
    tft.setTextColor(TFT_GREEN, TFT_DARKGREY);
    tft.setTextDatum(TL_DATUM); // Top Left
    tft.drawString("KryonOS", 6, 4, 2);

    // Desenha ícones no lado direito do cabeçalho
    drawHeaderIcons(175, 5);

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
    bool isLua = false;

    if (isFolder) {
        filePath = path;
        if (!filePath.endsWith("/")) filePath += "/";
        
        // Prioridade: Se houver main.lua, roda Lua; senão, roda main.js (mantém JS compatível)
        if (FileSystem::exists((filePath + "main.lua").c_str())) {
            filePath += "main.lua";
            isLua = true;
        } else {
            filePath += "main.js";
            isLua = false;
        }
    } else {
        filePath = path;
        isLua = filePath.endsWith(".lua");
    }
    
    // Executa na engine correspondente
    if (isLua) {
        HarixKernel::runLuaFile(filePath.c_str()); // Função que gerencia o estado do Lua
    } else {
        HarixKernel::runFile(filePath.c_str());    // Mantém o Duktape original intacto
    }
    
    // Desenha botão de saída padrão
    tft.fillRoundRect(200, 0, 40, 30, 5, TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("X", 220, 15, 2);
}

// Preenche o buffer interno de itens
void LauncherUI::refreshItems() {
    totalItems = getLauncherItems(items, 50);
}

void LauncherUI::executeSelectedItem() {
    extern int currentState;
    refreshItems();

    if (selectedIndex < 0 || selectedIndex >= totalItems) return;

    LauncherItem item = items[selectedIndex];

    switch (item.type) {
        case ITEM_HEADER:
            break;

        case ITEM_BACK:
            currentCategory = "";
            selectedIndex = 0;
            scrollOffset = 0;
            draw();
            break;

        case ITEM_CATEGORY:
            currentCategory = item.categoryTarget;
            selectedIndex = 0;
            scrollOffset = 0;
            draw();
            break;

        case ITEM_SYS:
            if (item.sysId == 1) currentState = 13;      // STATE_APP_STORE
            else if (item.sysId == 2) currentState = 3;  // STATE_INSTALLER
            else if (item.sysId == 3) currentState = 1;  // STATE_SETTINGS
            else if (item.sysId == 4) currentState = 14; // STATE_HELP_CENTER
            break;

        case ITEM_APP:
            if (item.appIndex >= 0 && item.appIndex < appCount) {
                runApp(appPaths[item.appIndex], appIsFolder[item.appIndex]);
            }
            break;
    }
}

void LauncherUI::navigateUp() {
    refreshItems();
    if (totalItems == 0) return;

    int targetIndex = selectedIndex - 1;

    while (targetIndex >= 0 && items[targetIndex].isHeader) {
        targetIndex--;
    }

    if (targetIndex >= 0) {
        selectedIndex = targetIndex;
    }

    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    }
    
    draw();
}

void LauncherUI::navigateDown() {
    refreshItems();
    int maxVisible = 3;

    int targetIndex = selectedIndex + 1;

    while (targetIndex < totalItems && items[targetIndex].isHeader) {
        targetIndex++;
    }

    if (targetIndex < totalItems) {
        selectedIndex = targetIndex;
    }

    if (selectedIndex >= scrollOffset + maxVisible) {
        scrollOffset = selectedIndex - (maxVisible - 1);
    }
    
    draw();
}

void LauncherUI::handleKeyInput(BoardKey key) {
    extern int currentState;

    if (key == BOARD_KEY_UP) {
        navigateUp();
    } 
    else if (key == BOARD_KEY_DOWN) {
        navigateDown();
    } 
    else if (key == BOARD_KEY_ENTER) {
        executeSelectedItem();
    } 
    else if (key == BOARD_KEY_ESC) {
        if (currentCategory.length() > 0) {
            currentCategory = "";
            selectedIndex = 0;
            scrollOffset = 0;
            draw();
        } else {
            currentState = 0;
        }
    }
}

void LauncherUI::handleTouch(uint16_t x, uint16_t y) {
    extern int currentState;
    refreshItems();

    // 1. TOUCH NOS ITENS DA LISTA
    if (y >= 45 && y <= 270) {
        int clickedRelativeIndex = (y - 45) / 30;
        int clickedAbsoluteIndex = scrollOffset + clickedRelativeIndex;

        if (clickedAbsoluteIndex < totalItems && !items[clickedAbsoluteIndex].isHeader) {
            selectedIndex = clickedAbsoluteIndex;
            draw();
            executeSelectedItem();
        }
        return;
    }

    // 2. TOUCH NOS BOTÕES DO RODAPÉ
    if (y >= 285 && y <= 315) {
        if (x < 60) { 
            navigateUp();
        } 
        else if (x > 60 && x < 180) { 
            executeSelectedItem();
        } 
        else if (x > 180) { 
            navigateDown();
        }
        return;
    }

    // 3. TOUCH NO HEADER
    if (y < 40) {
        if (currentCategory.length() > 0) {
            currentCategory = "";
            selectedIndex = 0;
            scrollOffset = 0;
            draw();
        } else {
            currentState = 3;
        }
        return;
    }
}