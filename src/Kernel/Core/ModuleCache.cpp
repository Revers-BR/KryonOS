// ModuleCache.cpp
#include "ModuleCache.h"
#include <mbedtls/md5.h>
#include "HarixKernel.h"

std::vector<ModuleCache::CacheEntry> ModuleCache::_cache;
String ModuleCache::_appDirectory = "";

String ModuleCache::getLuacPath(const String& luaPath) {
    if (luaPath.endsWith(".luac")) {
        return luaPath;
    }
    
    if (luaPath.endsWith(".lua")) {
        return luaPath.substring(0, luaPath.length() - 4) + ".luac";
    }
    
    return luaPath + ".luac";
}

String ModuleCache::resolveModulePath(lua_State* L, const char* moduleName) {
    if (moduleName == nullptr) return "";
    
    String modulePath = String(moduleName);
    modulePath.replace(".", "/");
    
    if (modulePath.indexOf("..") >= 0) {
        Serial.println("[ModuleCache] ERRO: Path traversal detectado!");
        return "";
    }
    
    if (_appDirectory.length() == 0) {
        Serial.println("[ModuleCache] ERRO: Diretório do app não configurado!");
        return "";
    }
    
    String candidates[4];
    
    candidates[0] = _appDirectory + "/" + modulePath + ".luac";
    
    candidates[1] = _appDirectory + "/" + modulePath + ".lua";
    
    candidates[2] = _appDirectory + "/" + modulePath + "/init.luac";
    
    candidates[3] = _appDirectory + "/" + modulePath + "/init.lua";
    
    for (int i = 0; i < 4; i++) {
        if (FileSystem::exists(candidates[i].c_str()) && 
            FileSystem::isFile(candidates[i].c_str())) {
            
            Serial.printf("[ModuleCache] '%s' -> %s\n", moduleName, candidates[i].c_str());
            return candidates[i];
        }
    }
    
    Serial.printf("[ModuleCache] Módulo '%s' não encontrado\n", moduleName);
    return "";
}

bool ModuleCache::needsRecompile(const String& luaPath, const String& luacPath) {
    if (!FileSystem::exists(luacPath.c_str())) {
        Serial.printf("[ModuleCache] %s não existe\n", luacPath.c_str());
        return true;
    }
    
    if (!FileSystem::exists(luaPath.c_str())) {
        return false;
    }
    
    time_t luaTime = FileSystem::getLastModified(luaPath.c_str());
    time_t luacTime = FileSystem::getLastModified(luacPath.c_str());
    
    if (luaTime > luacTime) {
        Serial.printf("[ModuleCache] %s é mais recente\n", luaPath.c_str());
        return true;
    }
    
    String luaMD5 = FileSystem::getFileMD5(luaPath.c_str());
    
    String metaPath = luacPath + ".meta";
    if (FileSystem::exists(metaPath.c_str())) {
        String metaContent = FileSystem::readTextFile(metaPath.c_str());
        if (metaContent.length() > 0) {
            String savedMD5 = metaContent.substring(0, 32);
            if (savedMD5 != luaMD5) {
                Serial.printf("[ModuleCache] MD5 diferente para %s\n", luaPath.c_str());
                return true;
            }
            return false;
        }
    }
    
    return true; // Sem metadados, recompila
}

static int moduleDumpWriter(lua_State* L, const void* p, size_t sz, void* ud) {
    LuaDumpBuffer* buf = (LuaDumpBuffer*)ud;
    
    if (buf->size + sz > buf->capacity) {
        size_t newCapacity = (buf->capacity == 0) ? 1024 : buf->capacity * 2;
        while (newCapacity < buf->size + sz) {
            newCapacity *= 2;
        }
        
        uint8_t* newData = (uint8_t*)realloc(buf->data, newCapacity);
        if (!newData) {
            return 1;
        }
        
        buf->data = newData;
        buf->capacity = newCapacity;
    }
    
    memcpy(buf->data + buf->size, p, sz);
    buf->size += sz;
    return 0;
}

bool ModuleCache::compileToLuac(lua_State* L, const String& luaPath, const String& luacPath) {
    Serial.printf("[ModuleCache] Compilando %s\n", luaPath.c_str());
    
    // Lê código fonte
    String source = FileSystem::readTextFile(luaPath.c_str());
    if (source.length() == 0) {
        Serial.printf("[ModuleCache] ERRO: %s vazio\n", luaPath.c_str());
        return false;
    }
    
    // Compila
    int rc = luaL_loadbuffer(L, source.c_str(), source.length(), luaPath.c_str());
    if (rc != LUA_OK) {
        Serial.printf("[ModuleCache] ERRO de compilação: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    
    // Dump bytecode
    LuaDumpBuffer buf = {nullptr, 0, 0};
    rc = lua_dump(L, moduleDumpWriter, &buf);
    
    if (rc != 0) {
        Serial.println("[ModuleCache] ERRO ao gerar bytecode");
        lua_pop(L, 1);
        if (buf.data) free(buf.data);
        return false;
    }
    
    lua_pop(L, 1); // Remove chunk
    
    // Salva bytecode
    bool saved = FileSystem::writeBinaryFile(luacPath.c_str(), buf.data, buf.size);
    
    Serial.printf("[ModuleCache] Bytecode salvo: %d bytes\n", buf.size);
    
    free(buf.data);
    
    if (!saved) {
        Serial.println("[ModuleCache] ERRO ao salvar bytecode");
        return false;
    }
    
    // Salva metadados
    String md5 = FileSystem::getFileMD5(luaPath.c_str());
    String metaPath = luacPath + ".meta";
    String metaContent = md5 + "|" + String(FileSystem::getFileSize(luaPath.c_str()));
    FileSystem::writeTextFile(metaPath.c_str(), metaContent.c_str());
    
    return true;
}

bool ModuleCache::loadLuac(lua_State* L, const String& luacPath) {
    // Lê o bytecode
    size_t fileSize = FileSystem::getFileSize(luacPath.c_str());
    if (fileSize == 0) {
        Serial.println("[ModuleCache] Bytecode vazio");
        return false;
    }
    
    // Aloca buffer
    uint8_t* buffer = (uint8_t*)malloc(fileSize);
    if (!buffer) {
        Serial.println("[ModuleCache] ERRO de alocação");
        return false;
    }
    
    // Lê o arquivo
    size_t bytesRead = FileSystem::readBinaryFile(luacPath.c_str(), buffer, fileSize);
    if (bytesRead != fileSize) {
        Serial.println("[ModuleCache] ERRO ao ler bytecode");
        free(buffer);
        return false;
    }
    
    // Verifica assinatura
    if (bytesRead < 4 || 
        buffer[0] != 0x1B || 
        buffer[1] != 'L' || 
        buffer[2] != 'u' || 
        buffer[3] != 'a') {
        Serial.println("[ModuleCache] Assinatura inválida");
        free(buffer);
        return false;
    }
    
    // Carrega
    int rc = luaL_loadbuffer(L, (const char*)buffer, bytesRead, luacPath.c_str());
    free(buffer);
    
    if (rc != LUA_OK) {
        Serial.printf("[ModuleCache] ERRO ao carregar: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}

// ============================================
// PRÉ-COMPILAÇÃO
// ============================================

void ModuleCache::precompileDirectory(lua_State* L, const String& dirPath) {
    Serial.printf("[ModuleCache] Pré-compilando %s\n", dirPath.c_str());
    
    // Lista arquivos
    String* files = new String[200];
    int count = FileSystem::listDir(dirPath.c_str(), files, 200);
    
    for (int i = 0; i < count; i++) {
        String file = files[i];
        
        if (file.endsWith(".lua")) {
            String luacPath = getLuacPath(file);
            
            if (needsRecompile(file, luacPath)) {
                compileToLuac(L, file.c_str(), luacPath.c_str());
            }
        }
    }
    
    delete[] files;
}