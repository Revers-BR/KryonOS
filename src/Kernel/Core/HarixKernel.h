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
    // =====================================================
    // Ponto de entrada único — detecta a engine pela
    // extensão do arquivo e despacha para o executor
    // correspondente, cada um rodando em task dedicada
    // (ver EngineTaskRunner) com stack próprio, isolado
    // da loopTask.
    // =====================================================
    static void runFile(const char* filePath);

    // --- JavaScript (Duktape) ---
    // Mantidos públicos: usados fora do fluxo de execução
    // de apps (ex: REPL, checagem de sintaxe em editor).
    static void executeJS(const char* jsCode);
    static String checkSyntax(const char* jsCode);
    static duk_context *ctx;

    // --- Lua ---
    static lua_State *L;

private:
    // =====================================================
    // Engines suportadas
    // =====================================================
    enum class Engine { Lua, Wren, Duktape, Unknown };

    static Engine detectEngine(const String& filePath);

    // Corpo específico de cada engine — rodam dentro de
    // EngineTaskRunner::run(), chamados só por runFile().
    static void runLuaFile(const char* filePath);
    static void runWrenFile(const char* filePath);
    static void runJsFile(const char* filePath);

    // Stack dedicado por engine (bytes). Ajustar conforme
    // o log de watermark do EngineTaskRunner.
    static constexpr uint32_t kLuaTaskStackSize  = 32768;
    static constexpr uint32_t kWrenTaskStackSize = 65536;
    static constexpr uint32_t kJsTaskStackSize   = 32768;

    // --- Tratadores de Erro Internos ---
    static void checkJSError(duk_context *ctx, duk_int_t result);
    static void checkLuaError(lua_State *L, int result);

};

#endif // HARIX_KERNEL_H