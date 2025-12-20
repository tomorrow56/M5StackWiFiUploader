#include "FileValidator.h"

// 静的メンバ変数の初期化
String FileValidator::_lastErrorMessage = "";

// ============================================================================
// ファイル名検証
// ============================================================================

bool FileValidator::isValidFilename(const char* filename) {
    if (!filename || strlen(filename) == 0) {
        _setError("Filename is empty");
        return false;
    }

    if (strlen(filename) > 255) {
        _setError("Filename is too long");
        return false;
    }

    return isSafeFilename(filename);
}

bool FileValidator::isSafeFilename(const char* filename) {
    if (!filename) return false;

    // 危険な文字をチェック
    const char* dangerous = "<>:\"|?*";
    for (const char* p = dangerous; *p; p++) {
        if (strchr(filename, *p)) {
            _setError("Filename contains invalid characters");
            return false;
        }
    }

    // パス区切り文字をチェック
    if (strchr(filename, '/') || strchr(filename, '\\')) {
        _setError("Filename contains path separators");
        return false;
    }

    // 制御文字をチェック
    for (const char* p = filename; *p; p++) {
        if (*p < 32) {
            _setError("Filename contains control characters");
            return false;
        }
    }

    return true;
}

String FileValidator::sanitizeFilename(const char* filename) {
    if (!filename) return "";

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

    // 末尾のスペースを除去
    while (result.endsWith(" ")) {
        result = result.substring(0, result.length() - 1);
    }

    return result;
}

// ============================================================================
// 拡張子検証
// ============================================================================

bool FileValidator::isAllowedExtension(const char* filename, const char** allowedExtensions, uint8_t count) {
    if (!filename || !allowedExtensions || count == 0) {
        return false;
    }

    String ext = getExtension(filename);
    if (ext.length() == 0) {
        _setError("File has no extension");
        return false;
    }

    for (uint8_t i = 0; i < count; i++) {
        if (ext == allowedExtensions[i]) {
            return true;
        }
    }

    _setError("File extension not allowed");
    return false;
}

bool FileValidator::isAllowedExtension(const char* filename, const std::vector<String>& extensionList) {
    if (!filename || extensionList.empty()) {
        return false;
    }

    String ext = getExtension(filename);
    if (ext.length() == 0) {
        _setError("File has no extension");
        return false;
    }

    for (const auto& allowed : extensionList) {
        if (ext == allowed) {
            return true;
        }
    }

    _setError("File extension not allowed");
    return false;
}

String FileValidator::getExtension(const char* filename) {
    if (!filename) return "";

    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return "";
    }

    String ext = String(dot + 1);
    ext.toLowerCase();
    return ext;
}

// ============================================================================
// サイズ検証
// ============================================================================

bool FileValidator::isValidFileSize(uint32_t filesize, uint32_t maxSize, uint32_t minSize) {
    if (filesize < minSize) {
        _setError("File is too small");
        return false;
    }

    if (maxSize > 0 && filesize > maxSize) {
        _setError("File is too large");
        return false;
    }

    return true;
}

String FileValidator::formatFileSize(uint32_t size) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    float fsize = size;
    int unitIndex = 0;

    while (fsize >= 1024.0 && unitIndex < 3) {
        fsize /= 1024.0;
        unitIndex++;
    }

    char buffer[32];
    if (unitIndex == 0) {
        snprintf(buffer, sizeof(buffer), "%lu %s", (unsigned long)fsize, units[unitIndex]);
    } else {
        snprintf(buffer, sizeof(buffer), "%.2f %s", fsize, units[unitIndex]);
    }

    return String(buffer);
}

// ============================================================================
// MIMEタイプ検証
// ============================================================================

String FileValidator::getMimeType(const char* filename) {
    if (!filename) return "application/octet-stream";

    String ext = getExtension(filename);

    // 画像形式
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "gif") return "image/gif";
    if (ext == "bmp") return "image/bmp";
    if (ext == "webp") return "image/webp";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "ico") return "image/x-icon";

    // テキスト形式
    if (ext == "txt") return "text/plain";
    if (ext == "csv") return "text/csv";
    if (ext == "json") return "application/json";
    if (ext == "xml") return "application/xml";
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";

    // バイナリ形式
    if (ext == "bin" || ext == "dat") return "application/octet-stream";
    if (ext == "pdf") return "application/pdf";
    if (ext == "zip") return "application/zip";
    if (ext == "rar") return "application/x-rar-compressed";
    if (ext == "7z") return "application/x-7z-compressed";
    if (ext == "tar") return "application/x-tar";
    if (ext == "gz") return "application/gzip";

    // オーディオ形式
    if (ext == "mp3") return "audio/mpeg";
    if (ext == "wav") return "audio/wav";
    if (ext == "m4a") return "audio/mp4";
    if (ext == "flac") return "audio/flac";

    // ビデオ形式
    if (ext == "mp4") return "video/mp4";
    if (ext == "avi") return "video/x-msvideo";
    if (ext == "mov") return "video/quicktime";
    if (ext == "mkv") return "video/x-matroska";

    return "application/octet-stream";
}

bool FileValidator::isImageMimeType(const char* mimeType) {
    if (!mimeType) return false;
    return strncmp(mimeType, "image/", 6) == 0;
}

bool FileValidator::isTextMimeType(const char* mimeType) {
    if (!mimeType) return false;
    return strncmp(mimeType, "text/", 5) == 0;
}

bool FileValidator::isBinaryMimeType(const char* mimeType) {
    if (!mimeType) return false;
    return strncmp(mimeType, "application/", 12) == 0;
}

// ============================================================================
// ファイルコンテンツ検証
// ============================================================================

bool FileValidator::validateMagicNumber(const uint8_t* data, uint32_t size, const char* expectedType) {
    if (!data || size < 4 || !expectedType) {
        return false;
    }

    String type = String(expectedType);
    type.toLowerCase();

    if (type == "jpg" || type == "jpeg") {
        return isJPEG(data, size);
    }
    if (type == "png") {
        return isPNG(data, size);
    }
    if (type == "gif") {
        return isGIF(data, size);
    }
    if (type == "bmp") {
        return isBMP(data, size);
    }

    return false;
}

bool FileValidator::isJPEG(const uint8_t* data, uint32_t size) {
    // JPEGマジックナンバー: FF D8 FF
    if (size < 3) return false;
    return (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF);
}

bool FileValidator::isPNG(const uint8_t* data, uint32_t size) {
    // PNGマジックナンバー: 89 50 4E 47
    if (size < 4) return false;
    return (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47);
}

bool FileValidator::isGIF(const uint8_t* data, uint32_t size) {
    // GIFマジックナンバー: 47 49 46 ("GIF")
    if (size < 3) return false;
    return (data[0] == 0x47 && data[1] == 0x49 && data[2] == 0x46);
}

bool FileValidator::isBMP(const uint8_t* data, uint32_t size) {
    // BMPマジックナンバー: 42 4D ("BM")
    if (size < 2) return false;
    return (data[0] == 0x42 && data[1] == 0x4D);
}

// ============================================================================
// 総合検証
// ============================================================================

bool FileValidator::validateFile(
    const char* filename,
    uint32_t filesize,
    const uint8_t* data,
    uint32_t dataSize,
    const char** allowedExtensions,
    uint8_t extensionCount,
    uint32_t maxFileSize) {

    // ファイル名検証
    if (!isValidFilename(filename)) {
        return false;
    }

    // ファイルサイズ検証
    if (!isValidFileSize(filesize, maxFileSize > 0 ? maxFileSize : 0xFFFFFFFF)) {
        return false;
    }

    // 拡張子検証
    if (allowedExtensions && extensionCount > 0) {
        if (!isAllowedExtension(filename, allowedExtensions, extensionCount)) {
            return false;
        }
    }

    // マジックナンバー検証（データが提供されている場合）
    if (data && dataSize > 0) {
        String ext = getExtension(filename);
        if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" || ext == "bmp") {
            if (!validateMagicNumber(data, dataSize, ext.c_str())) {
                _setError("File content does not match extension");
                return false;
            }
        }
    }

    return true;
}

// ============================================================================
// プライベートメソッド
// ============================================================================

void FileValidator::_setError(const char* message) {
    _lastErrorMessage = message;
}
