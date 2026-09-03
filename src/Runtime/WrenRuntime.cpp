#include "Runtime/WrenRuntime.h"
#include "Runtime/WrenBindings.h"

WrenVM* WrenRuntime::vm = nullptr;
int WrenRuntime::apiLineCount = 0;

// =====================================================
// Output
// =====================================================

void WrenRuntime::writeFn(
    WrenVM* vm,
    const char* text
)
{
    (void)vm;

    Serial.print(text);
}


// =====================================================
// Error
// =====================================================

void WrenRuntime::errorFn(
    WrenVM* vm,
    WrenErrorType type,
    const char* module,
    int line,
    const char* msg
)
{
    (void)vm;

    const char* typeName = "Unknown Error";

    switch (type)
    {
        case WREN_ERROR_COMPILE:      typeName = "Compile Error"; break;
        case WREN_ERROR_STACK_TRACE:  typeName = "Stack Trace";   break;
        case WREN_ERROR_RUNTIME:      typeName = "Runtime Error"; break;
        default: break;
    }

    int realLine = line;
    if (type == WREN_ERROR_STACK_TRACE &&
        WrenRuntime::apiLineCount > 0 &&
        line > WrenRuntime::apiLineCount)
    {
        realLine = line - WrenRuntime::apiLineCount;
    }

    // Serial (sem mudanças)
    Serial.print("[Wren] ");
    Serial.print(typeName);
    if (module)
    {
        Serial.print(" ");
        Serial.print(module);
        if (realLine > 0) { Serial.print(":"); Serial.print(realLine); }
    }
    Serial.print(" - ");
    Serial.println(msg ? msg : "no message");

    // TFT — continua chamando showError, só que agora ele acumula
    String errorText;
    if (module)
    {
        errorText += module;
        if (realLine > 0) { errorText += ":"; errorText += String(realLine); }
        errorText += "\n";
    }
    if (msg) errorText += msg;

    WrenBindings::showError(typeName, errorText.c_str());
}

// =====================================================
// Module Loader
// =====================================================

WrenLoadModuleResult WrenRuntime::loadModule(
    WrenVM* vm,
    const char* name
)
{
    (void)vm;
    (void)name;

    WrenLoadModuleResult result{};

    result.source = nullptr;
    result.onComplete = nullptr;
    result.userData = nullptr;

    return result;
}


// =====================================================
// Foreign Methods
// =====================================================

WrenForeignMethodFn WrenRuntime::bindForeignMethod(
    WrenVM* vm,
    const char* module,
    const char* className,
    bool isStatic,
    const char* signature
)
{
    return WrenBindings::bindForeignMethod(
        vm,
        module,
        className,
        isStatic,
        signature
    );
}


// =====================================================
// Foreign Classes
// =====================================================

WrenForeignClassMethods WrenRuntime::bindForeignClass(
    WrenVM* vm,
    const char* module,
    const char* className
)
{
    return WrenBindings::bindForeignClass(
        vm,
        module,
        className
    );
}


// =====================================================
// Begin
// =====================================================

bool WrenRuntime::begin()
{
    // Não criar duas VMs
    if (vm)
    {
        shutdown();
    }


    WrenConfiguration config;

    wrenInitConfiguration(&config);


    // =====================================================
    // Callbacks
    // =====================================================

    config.writeFn = writeFn;
    config.errorFn = errorFn;
    config.loadModuleFn = loadModule;

    config.bindForeignMethodFn =
        bindForeignMethod;

    config.bindForeignClassFn =
        bindForeignClass;


    // =====================================================
    // Create VM
    // =====================================================

    vm = wrenNewVM(&config);

    if (!vm)
    {
        Serial.println("[Wren] Failed to create VM");
        return false;
    }


    // =====================================================
    // Initialize bindings
    // =====================================================

    WrenBindings::init(vm);


    Serial.println("[Wren] VM initialized");

    return true;
}


// =====================================================
// Execute
// =====================================================

bool WrenRuntime::execute(
    const char* source,
    int lineOffset
)
{
    if (!vm)
    {
        Serial.println("[Wren] VM is not initialized");
        return false;
    }

    if (!source)
    {
        Serial.println("[Wren] Source is null");
        return false;
    }

    apiLineCount = lineOffset;

    WrenInterpretResult result =
        wrenInterpret(
            vm,
            "main",
            source
        );

    switch (result)
    {
        case WREN_RESULT_SUCCESS:
            Serial.println("[Wren] Script finished.");
            return true;

        case WREN_RESULT_COMPILE_ERROR:
            Serial.println("[Wren] Compilation failed.");
            return false;

        case WREN_RESULT_RUNTIME_ERROR:
            Serial.println("[Wren] Runtime failed.");
            return false;
    }

    return false;
}

// =====================================================
// Shutdown
// =====================================================

void WrenRuntime::shutdown()
{
    if (!vm)
        return;

    wrenFreeVM(vm);

    vm = nullptr;

    Serial.println(
        "[Wren] VM destroyed"
    );
}


// =====================================================
// Update
// =====================================================

void WrenRuntime::update()
{
    // Futuramente:
    //
    // processamento de eventos
    // timers
    // callbacks
    // etc.
}