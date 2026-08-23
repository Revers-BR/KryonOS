#ifndef HARIX_KERNEL_H
#define HARIX_KERNEL_H

#include "../../Runtime/duktape.h"
#include "boards/Board.h"
#include <esp_heap_caps.h>

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
    static void executeLua(const char* luaCode);
    static String checkLuaSyntax(const char* luaCode); // Opcional (caso queira validação de sintaxe para Lua)
    static lua_State *L;

private:
    // --- Tratadores de Erro Internos ---
    static void checkJSError(duk_context *ctx, duk_int_t result);
    static void checkLuaError(lua_State *L, int result);
};

#endif // HARIX_KERNEL_H