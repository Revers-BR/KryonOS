#ifndef HARIX_KERNEL_H
#define HARIX_KERNEL_H

#include "boards/Board.h"
#include <esp_heap_caps.h>
#include <mbedtls/md5.h>
#include "Runtime/duktape.h"

// Estrutura para metadados do bytecode
struct BytecodeMeta {
    String sourceMD5;
    size_t sourceSize;
    time_t sourceTimestamp;
};

struct LuaDumpBuffer {
    uint8_t* data;
    size_t size;
    size_t capacity;
};

// Inclusão dos headers do Lua envolvidos em extern "C" para compatibilidade C++
extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

class HarixKernel {
public:
    // --- JavaScript (Duktape) ---
    static void runFile(const char* filePath);
    static void executeJS(const char* jsCode);
    static String checkSyntax(const char* jsCode);
    static duk_context *ctx;

    // --- Lua ---
    static void runLuaFile(const char* filePath);
    static lua_State *L;

    static void runWrenFile(const char* filePath);

private:
    // --- Tratadores de Erro Internos ---
    static void checkJSError(duk_context *ctx, duk_int_t result);
    static void checkLuaError(lua_State *L, int result);

};

#endif // HARIX_KERNEL_H