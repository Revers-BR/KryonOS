#include "HarixKernel.h"
#include "../../Runtime/JSBindings.h"
#include "../../Runtime/LuaBindings.h"
#include "../../File System/FileSystem.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

lua_State *HarixKernel::L = nullptr;

duk_context *HarixKernel::ctx = nullptr;

// Função auxiliar de alocação com fallback para a memória interna
static void* psram_or_internal_malloc(size_t size) {
    if (size == 0) return nullptr;
    
    void* p = nullptr;
    if (psramFound()) {
        p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    if (!p) { // Se a PSRAM não existir ou estiver cheia, usa a RAM interna
        p = malloc(size);
    }
    return p;
}

// Função auxiliar de realocação com fallback para a memória interna
static void* psram_or_internal_realloc(void* ptr, size_t size) {
    if (size == 0) {
        free(ptr);
        return nullptr;
    }
    if (!ptr) {
        return psram_or_internal_malloc(size);
    }

    void* p = nullptr;
    if (psramFound()) {
        p = heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    if (!p) { // Se a PSRAM falhar, tenta realocar na RAM interna
        p = realloc(ptr, size);
    }
    return p;
}

// 1. Alocador Lua
static void *my_lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud;
    (void)osize;
    if (nsize == 0) {
        free(ptr);
        return nullptr;
    }
    
    void *p = psram_or_internal_realloc(ptr, nsize);
    if (!p) {
        Serial.println("[Lua] Out of memory!");
    }
    return p;
}

// 2. Alocador Duktape (malloc)
static void *my_alloc(void *udata, duk_size_t size) {
    (void)udata;
    if (size == 0) return nullptr;

    void *p = psram_or_internal_malloc(size);
    if (!p) {
        Serial.println("[Duktape] Out of memory!");
    }
    return p;
}

// 3. Alocador Duktape (realloc)
static void *my_realloc(void *udata, void *ptr, duk_size_t size) {
    (void)udata;
    if (size == 0) {
        free(ptr);
        return nullptr;
    }

    void *p = psram_or_internal_realloc(ptr, size);
    if (!p) {
        Serial.println("[Duktape] Out of memory!");
    }
    return p;
}

// 2. Manipulador de pânico do Lua (equivalente ao my_fatal do Duktape)
static int my_lua_panic(lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    Serial.print("Lua fatal panic: ");
    Serial.println(msg ? msg : "no message");

    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawString("Out Of Ram Error", 10, 20, 4);
    tft.drawString("Please turn off WiFi in", 10, 60, 2);
    tft.drawString("setting to free the ram", 10, 80, 2);
    tft.drawString("and make this app running", 10, 100, 2);
    
    // Desenha o botão 'X' para fechar/reiniciar
    tft.fillRoundRect(200, 0, 40, 30, 5, TFT_WHITE);
    tft.setTextColor(TFT_RED, TFT_WHITE);
    tft.drawString("X", 215, 8, 2);
    
    uint16_t tx, ty;
    while(true) {
        if (getTouch(&tx, &ty)) {
            if (tx >= 200 && ty <= 40) break;
        }
        delay(50);
    }
    
    ESP.restart(); // Reinicia o ESP32 ao fechar
    return 0;
}

// 3. Verificador de erros do Lua com suporte a tela TFT e Touch/Teclado
void HarixKernel::checkLuaError(lua_State *L, int result) {
    if (result != LUA_OK) {
        const char *msg = lua_tostring(L, -1);
        String errorMsg = msg ? msg : "Unknown Lua error";
        
        // Intercepta sinal oculto de saída do OS ("OS_EXIT")
        if (errorMsg.indexOf("OS_EXIT") != -1) {
            lua_pop(L, 1); // Remove o erro da pilha
            return;        // Sai limpo sem exibir tela vermelha
        }
        
        // Intercepta sinais de Out Of Memory (OOM)
        if (errorMsg.indexOf("alloc") != -1 || errorMsg.indexOf("out of memory") != -1) {
            tft.fillScreen(TFT_RED);
            tft.setTextColor(TFT_WHITE, TFT_RED);
            tft.drawString("Out Of Ram Error", 10, 20, 4);
            tft.drawString("Please turn off WiFi in", 10, 60, 2);
            tft.drawString("setting to free the ram", 10, 80, 2);
            tft.drawString("and make this app running", 10, 100, 2);
            
            tft.fillRoundRect(200, 0, 40, 30, 5, TFT_WHITE);
            tft.setTextColor(TFT_RED, TFT_WHITE);
            tft.drawString("X", 215, 8, 2);
            
            uint16_t tx, ty;
            while(true) {
                if (getTouch(&tx, &ty)) {
                    if (tx >= 200 && ty <= 40) break;
                }
                BoardKey key = getKeyInput();
                if (key != BOARD_KEY_NONE) break;
                delay(50);
            }
            lua_pop(L, 1);
            return;
        }
        
        Serial.print("Lua Execution Error: ");
        Serial.println(errorMsg);
        
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("LUA EXCEPTION!", 10, 10, 4);
        
        // Configuração de quebra de linha automática
        tft.setTextWrap(true, true);
        tft.setTextFont(2);
        tft.setCursor(10, 45);
        tft.print(errorMsg);
        
        // Botão 'X' para fechar
        tft.fillRoundRect(200, 0, 40, 30, 5, TFT_WHITE);
        tft.setTextColor(TFT_RED, TFT_WHITE);
        tft.drawString("X", 215, 8, 2);
        
        uint16_t tx, ty;
        while(true) {
            if (getTouch(&tx, &ty)) {
                if (tx >= 200 && ty <= 40) break;
            }
            BoardKey key = getKeyInput();
            if (key != BOARD_KEY_NONE) break;
            delay(50);
        }
    }
    lua_pop(L, 1); // Remove o resultado ou mensagem de erro da pilha do Lua
}

static void my_free(void *udata, void *ptr) {
    free(ptr);
}

// Dummy fatal error handler if duktape aborts
static void my_fatal(void *udata, const char *msg) {
    Serial.print("Duktape fatal error: ");
    Serial.println(msg ? msg : "no message");

    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawString("Out Of Ram Error", 10, 20, 4);
    tft.drawString("Please turn off WiFi in", 10, 60, 2);
    tft.drawString("setting to free the ram", 10, 80, 2);
    tft.drawString("and make this app running", 10, 100, 2);
    
    // Draw an 'X' to close/reboot
    tft.fillRoundRect(200, 0, 40, 30, 5, TFT_WHITE);
    tft.setTextColor(TFT_RED, TFT_WHITE);
    tft.drawString("X", 215, 8, 2);
    
    // Wait for user to touch the X before rebooting!
    uint16_t tx, ty;
    while(true) {
        if (getTouch(&tx, &ty)) {
            if (tx >= 200 && ty <= 40) break;
        }
        delay(50);
    }
    
    if (msg && strstr(msg, "alloc")) {
        Serial.println("out of memory");
    }
    ESP.restart(); // Reboot when they close it
}

void HarixKernel::checkJSError(duk_context *ctx, duk_int_t result) {
    if (result != 0) {
        String errorMsg = "";
        if (duk_is_error(ctx, -1)) {
            duk_get_prop_string(ctx, -1, "stack");
            errorMsg = duk_safe_to_string(ctx, -1);
            duk_pop(ctx);
        } else {
            errorMsg = duk_safe_to_string(ctx, -1);
        }
        
        // Intercept hidden OS Exit signal
        if (errorMsg.indexOf("OS_EXIT") != -1) {
            duk_pop(ctx); // pop the error
            return; // Cleanly exit execution without printing red screen
        }
        
        // Intercept OOM signals
        if (errorMsg.indexOf("alloc") != -1 || errorMsg.indexOf("out of memory") != -1) {
            tft.fillScreen(TFT_RED);
            tft.setTextColor(TFT_WHITE, TFT_RED);
            tft.drawString("Out Of Ram Error", 10, 20, 4);
            tft.drawString("Please turn off WiFi in", 10, 60, 2);
            tft.drawString("setting to free the ram", 10, 80, 2);
            tft.drawString("and make this app running", 10, 100, 2);
            
            // Draw an 'X' to close
            tft.fillRoundRect(200, 0, 40, 30, 5, TFT_WHITE);
            tft.setTextColor(TFT_RED, TFT_WHITE);
            tft.drawString("X", 215, 8, 2);
            
            uint16_t tx, ty;
            while(true) {
                if (getTouch(&tx, &ty)) {
                    if (tx >= 200 && ty <= 40) break;
                }
                
                BoardKey key = getKeyInput();
                if (key != BOARD_KEY_NONE) {
                    break; 
                }

                delay(50);
            }
            duk_pop(ctx);
            return;
        }
        
        Serial.print("JS Execution Error: ");
        Serial.println(errorMsg);
        
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("JS EXCEPTION!", 10, 10, 4);
        
        // --- CONFIGURAÇÃO DA QUEBRA DE LINHA AUTOMÁTICA ---
        tft.setTextWrap(true, true); // Ativa wrap nos eixos X e Y
        tft.setTextFont(2);          // Fonte padrão tamanho 2
        tft.setCursor(10, 45);       // Define a posição inicial do cursor

        // Escreve a mensagem de erro inteira; o print cuida do wrap de borda e dos \n
        tft.print(errorMsg);
        
        // Draw an 'X' to close
        tft.fillRoundRect(200, 0, 40, 30, 5, TFT_WHITE);
        tft.setTextColor(TFT_RED, TFT_WHITE);
        tft.drawString("X", 215, 8, 2);
        
        uint16_t tx, ty;
        while(true) {
            if (getTouch(&tx, &ty)) {
                if (tx >= 200 && ty <= 40) break;
            }

            BoardKey key = getKeyInput();
            if (key != BOARD_KEY_NONE) {
                break;
            }

            delay(50);
        }
    }
    duk_pop(ctx); // pop result or error
}

void HarixKernel::executeJS(const char* jsCode) {
    if (!ctx) return;
    
    duk_int_t rc = duk_peval_string(ctx, jsCode);
    checkJSError(ctx, rc);
}

// Struct to pass data to the syntax check task
struct SyntaxCheckParams {
    const char* jsCode;
    String result;
    bool done;
};

static void syntaxCheckTask(void* param) {
    SyntaxCheckParams* p = (SyntaxCheckParams*)param;
    
    duk_context *tempCtx = duk_create_heap(my_alloc, my_realloc, my_free, nullptr, nullptr);
    if (!tempCtx) {
        p->result = "Out of Memory allocating JS heap";
        p->done = true;
        vTaskDelete(NULL);
        return;
    }
    
    duk_int_t rc = duk_pcompile_string(tempCtx, 0, p->jsCode);
    if (rc != 0) {
        p->result = duk_safe_to_string(tempCtx, -1);
        Serial.printf("Syntax Error: %s\n", p->result.c_str());
    } else {
        p->result = "";
    }
    duk_pop(tempCtx);
    duk_destroy_heap(tempCtx);
    
    p->done = true;
    vTaskDelete(NULL);
}

String HarixKernel::checkSyntax(const char* jsCode) {
    SyntaxCheckParams params;
    params.jsCode = jsCode;
    params.result = "";
    params.done = false;
    
    // Run in a dedicated task with 16KB stack to avoid overflowing loopTask
    BaseType_t created = xTaskCreatePinnedToCore(
        syntaxCheckTask,
        "syntaxChk",
        16384,        // 16KB stack just for this task
        &params,
        1,            // Low priority
        NULL,
        1             // Run on Core 1
    );
    
    if (created != pdPASS) {
        return "Failed to create syntax check task";
    }
    
    // Block until the task finishes
    while (!params.done) {
        delay(10);
    }
    
    return params.result;
}

void HarixKernel::runFile(const char* filePath) {
    if (ctx) {
        duk_destroy_heap(ctx);
        ctx = nullptr;
    }

    ctx = duk_create_heap(my_alloc, my_realloc, my_free, nullptr, my_fatal);
    if (!ctx) {
        Serial.println("Failed to create Duktape heap for app.");
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
        tft.drawString("Out Of Ram Error", 10, 20, 4);
        tft.drawString("Please turn off WiFi in", 10, 60, 2);
        tft.drawString("setting to free the ram", 10, 80, 2);
        tft.drawString("and make this app running", 10, 100, 2);
        
        // Draw an 'X' to close
        tft.fillRoundRect(200, 0, 40, 30, 5, TFT_WHITE);
        tft.setTextColor(TFT_RED, TFT_WHITE);
        tft.drawString("X", 215, 8, 2);
        
        uint16_t tx, ty;
        while(true) {
            // 1. Checa por Touch no botão 'X'
            if (getTouch(&tx, &ty)) {
                if (tx >= 200 && ty <= 40) break;
            }

            // 2. Checa por Tecla Pressionada (Entrada do Cardputer)
            BoardKey key = getKeyInput();
            if (key == BOARD_KEY_ESC) {
                break;
            }

            delay(50);
        }
        return; // Soft exit back to OS
    }

    JSBindings::init(ctx);
    
    {
        String content = FileSystem::readTextFile(filePath);
        if (content.length() == 0) {
            Serial.print("Failed to read JS file: ");
            Serial.println(filePath);
            duk_destroy_heap(ctx);
            ctx = nullptr;
            return;
        }

        duk_push_string(ctx, filePath);
        duk_int_t rc = duk_pcompile_string_filename(ctx, 0, content.c_str());
        if (rc != 0) {
            checkJSError(ctx, rc);
            duk_destroy_heap(ctx);
            ctx = nullptr;
            return;
        }
    } // `content` String is destroyed here, freeing 50KB+ of RAM before the app runs

    duk_int_t rc = duk_pcall(ctx, 0);
    checkJSError(ctx, rc);
    
    // Destroy heap after app exits to free RAM
    duk_destroy_heap(ctx);
    ctx = nullptr;
}

// Writer callback para lua_dump
int luaDumpWriter(lua_State* L, const void* p, size_t sz, void* ud) {
    LuaDumpBuffer* buf = (LuaDumpBuffer*)ud;
    
    if (buf->size + sz > buf->capacity) {
        size_t newCapacity = (buf->capacity == 0) ? 1024 : buf->capacity * 2;
        while (newCapacity < buf->size + sz) {
            newCapacity *= 2;
        }
        
        uint8_t* newData = (uint8_t*)realloc(buf->data, newCapacity);
        if (!newData) {
            return 1; // Erro de alocação
        }
        
        buf->data = newData;
        buf->capacity = newCapacity;
    }
    
    memcpy(buf->data + buf->size, p, sz);
    buf->size += sz;
    
    return 0;
}

// Salva metadados
bool saveBytecodeMeta(const char* luacPath, const String& sourceMD5, size_t sourceSize) {
    String metaPath = String(luacPath) + ".meta";
    String content = sourceMD5 + "|" + String(sourceSize) + "|" + String(millis());
    return FileSystem::writeTextFile(metaPath.c_str(), content.c_str());
}

// Carrega metadados
BytecodeMeta loadBytecodeMeta(const char* luacPath) {
    BytecodeMeta meta = {"", 0, 0};
    
    String metaPath = String(luacPath) + ".meta";
    if (!FileSystem::exists(metaPath.c_str())) {
        return meta;
    }
    
    String content = FileSystem::readTextFile(metaPath.c_str());
    if (content.length() == 0) {
        return meta;
    }
    
    int sep1 = content.indexOf('|');
    int sep2 = (sep1 >= 0) ? content.indexOf('|', sep1 + 1) : -1;
    
    if (sep1 > 0) {
        meta.sourceMD5 = content.substring(0, sep1);
    }
    
    if (sep1 >= 0 && sep2 > sep1) {
        meta.sourceSize = content.substring(sep1 + 1, sep2).toInt();
    }
    
    if (sep2 >= 0) {
        meta.sourceTimestamp = content.substring(sep2 + 1).toInt();
    }
    
    return meta;
}

// Verifica se precisa recompilar
bool needsRecompile(const char* luaPath, const char* luacPath) {
    // Se não tem .luac, precisa compilar
    if (!FileSystem::exists(luacPath)) {
        return true;
    }
    
    // Se não tem .lua, usa .luac (não precisa recompilar)
    if (!FileSystem::exists(luaPath)) {
        return false;
    }
    
    // Carrega metadados
    BytecodeMeta meta = loadBytecodeMeta(luacPath);
    
    // Sem metadados, recompila
    if (meta.sourceMD5.length() == 0) {
        return true;
    }
    
    // Compara MD5
    String currentMD5 = FileSystem::getFileMD5(luaPath);
    if (currentMD5 != meta.sourceMD5) {
        return true; // Fonte mudou
    }
    
    // Compara tamanho
    size_t currentSize = FileSystem::getFileSize(luaPath);
    if (currentSize != meta.sourceSize) {
        return true; // Tamanho diferente
    }
    
    return false; // Bytecode está atualizado
}

// Compila e salva bytecode BINÁRIO
bool compileAndSaveBytecode(lua_State* L, const char* luaPath, const char* luacPath) {
    // Lê código fonte
    String source = FileSystem::readTextFile(luaPath);
    if (source.length() == 0) {
        Serial.println("Failed to read source for compilation");
        return false;
    }
    
    // Compila o código
    int rc = luaL_loadbuffer(L, source.c_str(), source.length(), luaPath);
    if (rc != LUA_OK) {
        Serial.print("Compilation error: ");
        Serial.println(lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    
    // Dump bytecode para buffer
    LuaDumpBuffer buf = {nullptr, 0, 0};
    rc = lua_dump(L, luaDumpWriter, &buf);
    
    if (rc != 0) {
        Serial.println("Failed to dump bytecode");
        lua_pop(L, 1);
        if (buf.data) free(buf.data);
        return false;
    }
    
    // Remove o chunk da pilha
    lua_pop(L, 1);
    
    // Salva bytecode BINÁRIO
    bool saved = FileSystem::writeBinaryFile(luacPath, buf.data, buf.size);
    
    Serial.print("Bytecode saved: ");
    Serial.print(buf.size);
    Serial.println(" bytes");
    
    free(buf.data);
    
    if (!saved) {
        Serial.println("Failed to save bytecode");
        return false;
    }
    
    // Salva metadados
    String md5 = FileSystem::getFileMD5(luaPath);
    size_t size = FileSystem::getFileSize(luaPath);
    saveBytecodeMeta(luacPath, md5, size);
    
    return true;
}

// Executa bytecode BINÁRIO
bool executeBytecode(lua_State* L, const char* luacPath) {
    // Obtém tamanho do arquivo
    size_t fileSize = FileSystem::getFileSize(luacPath);
    if (fileSize == 0) {
        Serial.println("Bytecode file is empty");
        return false;
    }
    
    // Verifica se é muito grande (limite de segurança)
    if (fileSize > 100000) { // 100KB máximo
        Serial.println("Bytecode file too large");
        return false;
    }
    
    // Aloca buffer para o bytecode
    uint8_t* buffer = (uint8_t*)malloc(fileSize);
    if (!buffer) {
        Serial.println("Failed to allocate memory for bytecode");
        return false;
    }
    
    // Lê o bytecode binário
    size_t bytesRead = FileSystem::readBinaryFile(luacPath, buffer, fileSize);
    if (bytesRead != fileSize) {
        Serial.println("Failed to read complete bytecode");
        free(buffer);
        return false;
    }
    
    // Verifica assinatura Lua (\x1bLua)
    if (bytesRead < 4 || 
        buffer[0] != 0x1B || 
        buffer[1] != 'L' || 
        buffer[2] != 'u' || 
        buffer[3] != 'a') {
        Serial.println("Invalid bytecode signature");
        free(buffer);
        return false;
    }
    
    // Carrega o bytecode
    int rc = luaL_loadbuffer(L, (const char*)buffer, bytesRead, luacPath);
    free(buffer);
    
    if (rc != LUA_OK) {
        Serial.print("Failed to load bytecode: ");
        Serial.println(lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}

// Função principal
void HarixKernel::runLuaFile(const char* filePath) {
    if (L) {
        lua_close(L);
        L = nullptr;
    }

    L = lua_newstate(my_lua_alloc, nullptr);
    if (!L) {
        Serial.println("Failed to create Lua state for app.");
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_RED);
        tft.drawString("Out Of Ram Error", 10, 20, 4);
        tft.drawString("Please turn off WiFi in", 10, 60, 2);
        tft.drawString("setting to free the ram", 10, 80, 2);
        tft.drawString("and make this app running", 10, 100, 2);
        
        tft.fillRoundRect(200, 0, 40, 30, 5, TFT_WHITE);
        tft.setTextColor(TFT_RED, TFT_WHITE);
        tft.drawString("X", 215, 8, 2);
        
        uint16_t tx, ty;
        while(true) {
            if (getTouch(&tx, &ty)) {
                if (tx >= 200 && ty <= 40) break;
            }
            BoardKey key = getKeyInput();
            if (key == BOARD_KEY_ESC) {
                break;
            }
            delay(50);
        }
        return;
    }

    luaL_openlibs(L);
    lua_atpanic(L, my_lua_panic);
    LuaBindings::init(L);
    
    // Determina caminhos
    String luaPath = String(filePath);
    String luacPath;
    
    // Converte entre .lua e .luac
    if (luaPath.endsWith(".luac")) {
        luacPath = luaPath;
        luaPath = luacPath.substring(0, luacPath.length() - 5) + ".lua";
    } else if (luaPath.endsWith(".lua")) {
        int lastDotLua = luaPath.lastIndexOf(".lua");
        luacPath = luaPath.substring(0, lastDotLua) + ".luac";
    } else {
        Serial.println("Invalid file extension");
        lua_close(L);
        L = nullptr;
        return;
    }
    
    Serial.print("Lua source: ");
    Serial.println(luaPath);
    Serial.print("Lua bytecode: ");
    Serial.println(luacPath);
    
    // Verifica o que existe
    bool hasLua = FileSystem::exists(luaPath.c_str());
    bool hasLuac = FileSystem::exists(luacPath.c_str());
    
    Serial.print("Has .lua: ");
    Serial.println(hasLua ? "YES" : "NO");
    Serial.print("Has .luac: ");
    Serial.println(hasLuac ? "YES" : "NO");
    
    if (!hasLua && !hasLuac) {
        Serial.println("No Lua files found!");
        lua_close(L);
        L = nullptr;
        return;
    }
    
    int rc = LUA_OK;
    bool loaded = false;
    
    if (hasLuac && !hasLua) {
        // CASO 1: Só tem bytecode
        Serial.println("Loading bytecode directly...");
        loaded = executeBytecode(L, luacPath.c_str());
        
    } else if (hasLua && !hasLuac) {
        // CASO 2: Só tem fonte
        Serial.println("Compiling source...");
        if (compileAndSaveBytecode(L, luaPath.c_str(), luacPath.c_str())) {
            loaded = executeBytecode(L, luacPath.c_str());
        } else {
            Serial.println("Falling back to source execution");
            String content = FileSystem::readTextFile(luaPath.c_str());
            if (content.length() > 0) {
                rc = luaL_loadbuffer(L, content.c_str(), content.length(), luaPath.c_str());
                loaded = (rc == LUA_OK);
            }
        }
        
    } else {
        // CASO 3: Ambos existem
        if (needsRecompile(luaPath.c_str(), luacPath.c_str())) {
            Serial.println("Source changed, recompiling...");
            if (compileAndSaveBytecode(L, luaPath.c_str(), luacPath.c_str())) {
                loaded = executeBytecode(L, luacPath.c_str());
            } else {
                Serial.println("Falling back to source execution");
                String content = FileSystem::readTextFile(luaPath.c_str());
                if (content.length() > 0) {
                    rc = luaL_loadbuffer(L, content.c_str(), content.length(), luaPath.c_str());
                    loaded = (rc == LUA_OK);
                }
            }
        } else {
            Serial.println("Using cached bytecode");
            loaded = executeBytecode(L, luacPath.c_str());
        }
    }
    
    if (!loaded) {
        if (rc != LUA_OK) {
            checkLuaError(L, rc);
        } else {
            Serial.println("Failed to load script");
        }
        lua_close(L);
        L = nullptr;
        return;
    }
    
    // Executa o chunk carregado
    rc = lua_pcall(L, 0, LUA_MULTRET, 0);
    checkLuaError(L, rc);
    
    // Destrói o estado após o app fechar
    lua_close(L);
    L = nullptr;
}