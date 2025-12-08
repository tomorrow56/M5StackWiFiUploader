#include "WebSocketHandler.h"
#include <ArduinoJson.h>

// ============================================================================
// コンストラクタ・デストラクタ
// ============================================================================

WebSocketHandler::WebSocketHandler(uint16_t port)
    : _port(port),
      _isRunning(false),
      _maxChunkSize(4096),
      _timeoutMs(30000),
      _debugLevel(2),
      _fileInfoCallback(nullptr),
      _dataCallback(nullptr),
      _messageCallback(nullptr),
      _connectCallback(nullptr),
      _disconnectCallback(nullptr) {
    _server = new WebSocketsServer(_port);
}

WebSocketHandler::~WebSocketHandler() {
    end();
    if (_server) {
        delete _server;
        _server = nullptr;
    }
}

// ============================================================================
// 初期化・制御
// ============================================================================

bool WebSocketHandler::begin() {
    if (_isRunning) {
        return true;
    }

    if (!_server) {
        _log(1, "[WS] Server object is null");
        return false;
    }

    // WebSocketイベントハンドラーを設定
    _server->onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        this->_handleWebSocketEvent(num, type, payload, length);
    });

    _server->begin();
    _isRunning = true;

    _log(2, "[WS] WebSocket server started on port %d", _port);
    return true;
}

void WebSocketHandler::end() {
    if (!_isRunning) {
        return;
    }

    if (_server) {
        _server->close();
    }

    _isRunning = false;
    _log(2, "[WS] WebSocket server stopped");
}

void WebSocketHandler::handleClient() {
    if (_isRunning && _server) {
        _server->loop();
    }
}

// ============================================================================
// メッセージ送信
// ============================================================================

void WebSocketHandler::sendProgress(uint8_t clientId, const char* filename,
                                    uint32_t uploaded, uint32_t total) {
    if (!_isRunning || !_server) return;

    StaticJsonDocument<256> doc;
    doc["type"] = "progress";
    doc["filename"] = filename;
    doc["uploaded"] = uploaded;
    doc["total"] = total;
    doc["percentage"] = (uploaded * 100) / total;

    String json;
    serializeJson(doc, json);
    _server->sendTXT(clientId, json);
}

void WebSocketHandler::sendComplete(uint8_t clientId, const char* filename, bool success) {
    if (!_isRunning || !_server) return;

    StaticJsonDocument<256> doc;
    doc["type"] = "complete";
    doc["filename"] = filename;
    doc["success"] = success;

    String json;
    serializeJson(doc, json);
    _server->sendTXT(clientId, json);
}

void WebSocketHandler::sendError(uint8_t clientId, uint8_t errorCode, const char* message) {
    if (!_isRunning || !_server) return;

    StaticJsonDocument<256> doc;
    doc["type"] = "error";
    doc["code"] = errorCode;
    doc["message"] = message;

    String json;
    serializeJson(doc, json);
    _server->sendTXT(clientId, json);
}

void WebSocketHandler::sendText(uint8_t clientId, const char* message) {
    if (!_isRunning || !_server) return;
    _server->sendTXT(clientId, message);
}

void WebSocketHandler::sendBinary(uint8_t clientId, const uint8_t* data, size_t length) {
    if (!_isRunning || !_server) return;
    _server->sendBIN(clientId, data, length);
}

void WebSocketHandler::broadcast(const char* message) {
    if (!_isRunning || !_server) return;
    _server->broadcastTXT(message);
}

// ============================================================================
// クライアント管理
// ============================================================================

uint8_t WebSocketHandler::getClientCount() const {
    if (!_server) return 0;
    return _server->connectedClients();
}

bool WebSocketHandler::isClientConnected(uint8_t clientId) const {
    // WebSocketsServerには直接的なクライアント接続チェックメソッドがないため
    // 簡易実装
    return _isRunning && _server && getClientCount() > 0;
}

void WebSocketHandler::disconnectClient(uint8_t clientId) {
    if (!_isRunning || !_server) return;
    _server->disconnect(clientId);
}

// ============================================================================
// プライベートメソッド
// ============================================================================

void WebSocketHandler::_handleWebSocketEvent(uint8_t clientId, WStype_t type,
                                             uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            _log(2, "[WS] Client %d disconnected", clientId);
            if (_disconnectCallback) {
                _disconnectCallback(clientId);
            }
            break;

        case WStype_CONNECTED:
            {
                IPAddress ip = _server->remoteIP(clientId);
                _log(2, "[WS] Client %d connected from %s", clientId, ip.toString().c_str());
                if (_connectCallback) {
                    _connectCallback(clientId);
                }
            }
            break;

        case WStype_TEXT:
            _log(3, "[WS] Text message from client %d: %s", clientId, (char*)payload);
            _handleTextMessage(clientId, (char*)payload);
            break;

        case WStype_BIN:
            _log(3, "[WS] Binary message from client %d, length: %d", clientId, length);
            _handleBinaryMessage(clientId, payload, length);
            break;

        case WStype_ERROR:
            _log(1, "[WS] Error on client %d", clientId);
            break;

        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            _log(3, "[WS] Fragment message from client %d", clientId);
            break;

        default:
            break;
    }
}

void WebSocketHandler::_handleTextMessage(uint8_t clientId, const char* message) {
    // JSONメッセージをパース
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        _log(1, "[WS] JSON parse error: %s", error.c_str());
        sendError(clientId, 4, "Invalid JSON format");
        return;
    }

    const char* type = doc["type"];
    if (!type) {
        _log(1, "[WS] Missing message type");
        sendError(clientId, 4, "Missing message type");
        return;
    }

    // メッセージタイプに応じて処理
    if (strcmp(type, "file_info") == 0) {
        // ファイル情報
        WSFileInfo fileInfo;
        fileInfo.filename = doc["filename"].as<String>();
        fileInfo.filesize = doc["filesize"];
        fileInfo.mimeType = doc["mimeType"].as<String>();
        fileInfo.chunkSize = doc["chunkSize"] | 4096;
        fileInfo.totalChunks = doc["totalChunks"];

        _log(2, "[WS] File info: %s (%u bytes)", fileInfo.filename.c_str(), fileInfo.filesize);

        if (_fileInfoCallback) {
            _fileInfoCallback(clientId, fileInfo);
        }
    }
    else if (strcmp(type, "cancel") == 0) {
        _log(2, "[WS] Upload cancel request from client %d", clientId);
        // キャンセル処理
    }
    else if (strcmp(type, "pause") == 0) {
        _log(2, "[WS] Upload pause request from client %d", clientId);
        // 一時停止処理
    }
    else if (strcmp(type, "resume") == 0) {
        _log(2, "[WS] Upload resume request from client %d", clientId);
        // 再開処理
    }
    else {
        // その他のメッセージ
        if (_messageCallback) {
            _messageCallback(clientId, message);
        }
    }
}

void WebSocketHandler::_handleBinaryMessage(uint8_t clientId, const uint8_t* data, size_t length) {
    // バイナリデータ（ファイルチャンク）を処理
    if (_dataCallback) {
        _dataCallback(clientId, data, length);
    }
}

String WebSocketHandler::_createJsonMessage(WSMessageType type, const char* data) {
    StaticJsonDocument<256> doc;
    
    switch (type) {
        case WS_MSG_PROGRESS:
            doc["type"] = "progress";
            break;
        case WS_MSG_COMPLETE:
            doc["type"] = "complete";
            break;
        case WS_MSG_ERROR:
            doc["type"] = "error";
            break;
        default:
            doc["type"] = "message";
            break;
    }
    
    if (data) {
        doc["data"] = data;
    }
    
    String json;
    serializeJson(doc, json);
    return json;
}

void WebSocketHandler::_log(uint8_t level, const char* format, ...) {
    if (level > _debugLevel) return;

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.println(buffer);
}
