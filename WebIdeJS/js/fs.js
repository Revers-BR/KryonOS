var virtualFS = {
  "/local/app.js":
    "// KryonOS App File\nSystem.print('Iniciando app do FS...');",
  "/local/config.json": '{"theme":"dark","autoStart":true}',
  "/sd/data_log.txt": "2026-08-08 10:00:00 | Sensor: 24.5C",
};

// Diretórios virtuais do sistema
var virtualDirs = ["/", "/local", "/sd"];

var sdMounted = true;

var FS = {
  // Checa se o caminho é um diretório
  isDirectory: function (path) {
    if (!path) return false;
    var cleanPath =
      path.length > 1 && path.endsWith("/") ? path.slice(0, -1) : path;
    if (virtualDirs.indexOf(cleanPath) !== -1) return true;

    var prefix = cleanPath.endsWith("/") ? cleanPath : cleanPath + "/";
    return Object.keys(virtualFS).some(function (filePath) {
      return filePath.indexOf(prefix) === 0;
    });
  },

  // Checa se o caminho é um arquivo regular
  isFile: function (path) {
    return virtualFS.hasOwnProperty(path);
  },

  // Checa se o arquivo ou diretório existe
  exists: function (path) {
    return FS.isFile(path) || FS.isDirectory(path);
  },

  // Retorna estatísticas/metadados do arquivo/diretório
  stat: function (path) {
    if (FS.isFile(path)) {
      return {
        size: virtualFS[path].length,
        isFile: true,
        isDirectory: false,
        mtime: Date.now(),
      };
    } else if (FS.isDirectory(path)) {
      return {
        size: 0,
        isFile: false,
        isDirectory: true,
        mtime: Date.now(),
      };
    }
    return null;
  },

  // Cria um diretório virtual
  mkdir: function (path) {
    if (virtualDirs.indexOf(path) === -1) {
      virtualDirs.push(path);
      if (typeof renderFsTree === "function") renderFsTree();
    }
    return true;
  },

  // Remove um diretório virtual
  rmdir: function (path) {
    var index = virtualDirs.indexOf(path);
    if (index !== -1) {
      virtualDirs.splice(index, 1);
      if (typeof renderFsTree === "function") renderFsTree();
      return true;
    }
    return false;
  },

  readTextFile: function (path) {
    if (path.indexOf("/sd/") === 0 && !sdMounted) return null;
    return virtualFS[path] || null;
  },

  writeTextFile: function (path, content) {
    if (path.indexOf("/sd/") === 0 && !sdMounted) return false;
    virtualFS[path] = content;
    if (typeof renderFsTree === "function") renderFsTree();
    return true;
  },

  appendTextFile: function (path, content) {
    if (path.indexOf("/sd/") === 0 && !sdMounted) return false;
    virtualFS[path] = (virtualFS[path] || "") + content;
    if (typeof renderFsTree === "function") renderFsTree();
    return true;
  },

  deleteFile: function (path) {
    if (path.indexOf("/sd/") === 0 && !sdMounted) return false;
    delete virtualFS[path];
    if (typeof renderFsTree === "function") renderFsTree();
    return true;
  },

  // Renomeia ou move um arquivo
  renameFile: function (pathFrom, pathTo) {
    if (!virtualFS.hasOwnProperty(pathFrom)) return false;
    virtualFS[pathTo] = virtualFS[pathFrom];
    delete virtualFS[pathFrom];
    if (typeof renderFsTree === "function") renderFsTree();
    return true;
  },

  listDir: function (dir) {
    var prefix = dir.endsWith("/") ? dir : dir + "/";
    return Object.keys(virtualFS).filter(function (p) {
      return p.indexOf(prefix) === 0;
    });
  },

  getFileSize: function (path) {
    return virtualFS[path] ? virtualFS[path].length : 0;
  },

  getFileMD5: function (path) {
    return "e10adc3949ba59abbe56e057f20f883e";
  },

  // Métricas de Espaço de Armazenamento
  getTotalSpace: function (drive) {
    if (drive === "/local") return 2097152; // 2MB Internal Flash
    if (drive === "/sd") return 16106127360; // 16GB SD Card
    return 0;
  },

  getUsedSpace: function (drive) {
    var totalUsed = 0;
    var prefix = drive.endsWith("/") ? drive : drive + "/";
    Object.keys(virtualFS).forEach(function (k) {
      if (k.indexOf(prefix) === 0) {
        totalUsed += virtualFS[k].length;
      }
    });
    return totalUsed;
  },

  getFreeSpace: function (drive) {
    return FS.getTotalSpace(drive) - FS.getUsedSpace(drive);
  },

  // Controle de Montagem do SD Card
  mountSD: function () {
    sdMounted = true;
    System.print("[FS] Cartão SD montado com sucesso.");
    if (typeof renderFsTree === "function") renderFsTree();
    return true;
  },

  unmountSD: function () {
    sdMounted = false;
    System.print("[FS] Cartão SD desmontado.");
    if (typeof renderFsTree === "function") renderFsTree();
    return true;
  },
};
