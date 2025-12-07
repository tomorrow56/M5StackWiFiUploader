#include "SDCardManager.h"

// 静的メンバ変数の初期化
bool SDCardManager::_initialized = false;

// ============================================================================
// 初期化
// ============================================================================

bool SDCardManager::initialize(uint8_t csPin) {
    if (_initialized) {
        return true;
    }

    if (SD.begin(csPin)) {
        _initialized = true;
        Serial.println("[SDCardManager] SD card initialized successfully");
        return true;
    }

    Serial.println("[SDCardManager] Failed to initialize SD card");
    return false;
}

bool SDCardManager::isConnected() {
    return _initialized && SD.cardSize() > 0;
}

// ============================================================================
// ファイル操作
// ============================================================================

bool SDCardManager::fileExists(const char* filepath) {
    if (!_initialized) return false;
    return SD.exists(filepath);
}

bool SDCardManager::deleteFile(const char* filepath) {
    if (!_initialized) return false;
    if (!fileExists(filepath)) return false;
    
    return SD.remove(filepath);
}

uint32_t SDCardManager::getFileSize(const char* filepath) {
    if (!_initialized) return 0;
    
    File file = SD.open(filepath, FILE_READ);
    if (!file) return 0;
    
    uint32_t size = file.size();
    file.close();
    
    return size;
}

uint32_t SDCardManager::readFile(const char* filepath, uint8_t* buffer, uint32_t maxSize) {
    if (!_initialized || !buffer) return 0;
    
    File file = SD.open(filepath, FILE_READ);
    if (!file) return 0;
    
    uint32_t bytesRead = 0;
    while (file.available() && bytesRead < maxSize) {
        buffer[bytesRead++] = file.read();
    }
    
    file.close();
    return bytesRead;
}

bool SDCardManager::writeFile(const char* filepath, const uint8_t* data, uint32_t size, bool append) {
    if (!_initialized || !data) return false;
    
    // ディレクトリが存在することを確認
    const char* lastSlash = strrchr(filepath, '/');
    if (lastSlash && lastSlash != filepath) {
        String dirpath(filepath, lastSlash - filepath);
        if (!dirExists(dirpath.c_str())) {
            if (!createDir(dirpath.c_str())) {
                return false;
            }
        }
    }
    
    File file = SD.open(filepath, append ? FILE_APPEND : FILE_WRITE);
    if (!file) return false;
    
    size_t written = file.write(data, size);
    file.close();
    
    return written == size;
}

bool SDCardManager::writeText(const char* filepath, const char* text, bool append) {
    if (!text) return false;
    return writeFile(filepath, (const uint8_t*)text, strlen(text), append);
}

String SDCardManager::readText(const char* filepath) {
    if (!_initialized) return "";
    
    File file = SD.open(filepath, FILE_READ);
    if (!file) return "";
    
    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }
    
    file.close();
    return content;
}

// ============================================================================
// ディレクトリ操作
// ============================================================================

bool SDCardManager::dirExists(const char* dirpath) {
    if (!_initialized) return false;
    
    File dir = SD.open(dirpath);
    if (!dir) return false;
    
    bool isDir = dir.isDirectory();
    dir.close();
    
    return isDir;
}

bool SDCardManager::createDir(const char* dirpath) {
    if (!_initialized) return false;
    if (dirExists(dirpath)) return true;
    
    return SD.mkdir(dirpath);
}

bool SDCardManager::deleteDir(const char* dirpath) {
    if (!_initialized) return false;
    if (!dirExists(dirpath)) return false;
    
    return SD.rmdir(dirpath);
}

std::vector<String> SDCardManager::listFiles(const char* dirpath, bool includeDir) {
    std::vector<String> files;
    
    if (!_initialized) return files;
    
    File dir = SD.open(dirpath);
    if (!dir || !dir.isDirectory()) {
        return files;
    }
    
    File file = dir.openNextFile();
    while (file) {
        if (includeDir || !file.isDirectory()) {
            files.push_back(file.name());
        }
        file = dir.openNextFile();
    }
    
    dir.close();
    return files;
}

uint32_t SDCardManager::getFileCount(const char* dirpath) {
    if (!_initialized) return 0;
    
    File dir = SD.open(dirpath);
    if (!dir || !dir.isDirectory()) {
        return 0;
    }
    
    uint32_t count = 0;
    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            count++;
        }
        file = dir.openNextFile();
    }
    
    dir.close();
    return count;
}

// ============================================================================
// 容量管理
// ============================================================================

uint32_t SDCardManager::getTotalSpace() {
    if (!_initialized) return 0;
    return SD.totalBytes();
}

uint32_t SDCardManager::getUsedSpace() {
    if (!_initialized) return 0;
    return SD.usedBytes();
}

uint32_t SDCardManager::getFreeSpace() {
    if (!_initialized) return 0;
    uint32_t total = SD.totalBytes();
    uint32_t used = SD.usedBytes();
    return (total > used) ? (total - used) : 0;
}

uint8_t SDCardManager::getUsagePercent() {
    if (!_initialized) return 0;
    uint32_t total = SD.totalBytes();
    if (total == 0) return 0;
    uint32_t used = SD.usedBytes();
    return (uint8_t)((used * 100) / total);
}

// ============================================================================
// ファイル検証
// ============================================================================

bool SDCardManager::isValidFilename(const char* filename) {
    if (!filename || strlen(filename) == 0) {
        return false;
    }
    
    // 危険な文字をチェック
    const char* dangerous = "<>:\"|?*";
    for (const char* p = dangerous; *p; p++) {
        if (strchr(filename, *p)) {
            return false;
        }
    }
    
    // パス区切り文字をチェック
    if (strchr(filename, '/') || strchr(filename, '\\')) {
        return false;
    }
    
    return true;
}

String SDCardManager::sanitizeFilename(const char* filename) {
    String result = filename;
    
    // 危険な文字を置換
    result.replace("<", "_");
    result.replace(">", "_");
    result.replace(":", "_");
    result.replace("\"", "_");
    result.replace("|", "_");
    result.replace("?", "_");
    result.replace("*", "_");
    result.replace("/", "_");
    result.replace("\\", "_");
    
    // パストラバーサル対策
    result.replace("..", "");
    
    // 先頭のドットを除去（隠しファイル対策）
    while (result.startsWith(".")) {
        result = result.substring(1);
    }
    
    return result;
}

String SDCardManager::getFileExtension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return "";
    }
    
    String ext = String(dot + 1);
    ext.toLowerCase();
    return ext;
}

String SDCardManager::getBaseName(const char* filename) {
    const char* slash = strrchr(filename, '/');
    const char* start = slash ? slash + 1 : filename;
    
    const char* dot = strrchr(start, '.');
    if (!dot) {
        return String(start);
    }
    
    return String(start, dot - start);
}

// ============================================================================
// ユーティリティ
// ============================================================================

bool SDCardManager::isAbsolutePath(const char* path) {
    return path && path[0] == '/';
}

String SDCardManager::normalizePath(const char* path) {
    if (!path) return "";
    
    String normalized = path;
    
    // 末尾のスラッシュを除去
    while (normalized.endsWith("/") && normalized.length() > 1) {
        normalized = normalized.substring(0, normalized.length() - 1);
    }
    
    // 重複スラッシュを除去
    while (normalized.indexOf("//") != -1) {
        normalized.replace("//", "/");
    }
    
    // パストラバーサル対策
    while (normalized.indexOf("/../") != -1) {
        int pos = normalized.indexOf("/../");
        int prevSlash = normalized.lastIndexOf("/", pos - 1);
        if (prevSlash >= 0) {
            normalized = normalized.substring(0, prevSlash) + normalized.substring(pos + 3);
        } else {
            break;
        }
    }
    
    return normalized;
}

bool SDCardManager::copyFile(const char* srcPath, const char* dstPath) {
    if (!_initialized || !fileExists(srcPath)) return false;
    
    File srcFile = SD.open(srcPath, FILE_READ);
    if (!srcFile) return false;
    
    File dstFile = SD.open(dstPath, FILE_WRITE);
    if (!dstFile) {
        srcFile.close();
        return false;
    }
    
    uint8_t buffer[512];
    size_t bytesRead;
    bool success = true;
    
    while ((bytesRead = srcFile.read(buffer, sizeof(buffer))) > 0) {
        if (dstFile.write(buffer, bytesRead) != bytesRead) {
            success = false;
            break;
        }
    }
    
    srcFile.close();
    dstFile.close();
    
    return success;
}

bool SDCardManager::moveFile(const char* srcPath, const char* dstPath) {
    if (!_initialized || !fileExists(srcPath)) return false;
    
    // コピーして元ファイルを削除
    if (copyFile(srcPath, dstPath)) {
        return deleteFile(srcPath);
    }
    
    return false;
}

uint32_t SDCardManager::getLastModified(const char* filepath) {
    if (!_initialized) return 0;
    
    File file = SD.open(filepath, FILE_READ);
    if (!file) return 0;
    
    // ESP32のSDライブラリではgetLastWriteメソッドが使用可能
    // ただし、実装によっては0を返す場合があります
    uint32_t lastWrite = file.getLastWrite();
    file.close();
    
    return lastWrite;
}
