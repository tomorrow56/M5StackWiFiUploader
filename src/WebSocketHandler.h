#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <Arduino.h>
#include <WebSocketsServer.h>
#include <functional>

// ============================================================================
// WebSocketメッセージタイプ
// ============================================================================
enum WSMessageType {
    WS_MSG_FILE_INFO,      // ファイル情報
    WS_MSG_FILE_DATA,      // ファイルデータ
    WS_MSG_PROGRESS,       // 進捗通知
    WS_MSG_COMPLETE,       // 完了通知
    WS_MSG_ERROR,          // エラー通知
    WS_MSG_CANCEL,         // キャンセル要求
    WS_MSG_PAUSE,          // 一時停止要求
    WS_MSG_RESUME          // 再開要求
};

// ============================================================================
// WebSocketファイル情報
// ============================================================================
struct WSFileInfo {
    String filename;
    uint32_t filesize;
    String mimeType;
    uint32_t chunkSize;
    uint32_t totalChunks;
};

// ============================================================================
// WebSocketコールバック
// ============================================================================
typedef std::function<void(uint8_t clientId, const WSFileInfo& fileInfo)> WSFileInfoCallback;
typedef std::function<void(uint8_t clientId, const uint8_t* data, size_t length)> WSDataCallback;
typedef std::function<void(uint8_t clientId, const char* message)> WSMessageCallback;
typedef std::function<void(uint8_t clientId)> WSClientCallback;

// ============================================================================
// WebSocketHandler クラス
// ============================================================================
class WebSocketHandler {
public:
    WebSocketHandler(uint16_t port = 81);
    ~WebSocketHandler();

    // ========================================================================
    // 初期化・制御
    // ========================================================================

    /**
     * @brief WebSocketサーバーを開始
     * @return 成功時true
     */
    bool begin();

    /**
     * @brief WebSocketサーバーを停止
     */
    void end();

    /**
     * @brief クライアントリクエストを処理
     */
    void handleClient();

    /**
     * @brief サーバーが稼働中かチェック
     * @return 稼働中ならtrue
     */
    bool isRunning() const { return _isRunning; }

    // ========================================================================
    // コールバック設定
    // ========================================================================

    /**
     * @brief ファイル情報受信コールバックを設定
     * @param callback コールバック関数
     */
    void onFileInfo(WSFileInfoCallback callback) { _fileInfoCallback = callback; }

    /**
     * @brief データ受信コールバックを設定
     * @param callback コールバック関数
     */
    void onData(WSDataCallback callback) { _dataCallback = callback; }

    /**
     * @brief テキストメッセージ受信コールバックを設定
     * @param callback コールバック関数
     */
    void onMessage(WSMessageCallback callback) { _messageCallback = callback; }

    /**
     * @brief クライアント接続コールバックを設定
     * @param callback コールバック関数
     */
    void onConnect(WSClientCallback callback) { _connectCallback = callback; }

    /**
     * @brief クライアント切断コールバックを設定
     * @param callback コールバック関数
     */
    void onDisconnect(WSClientCallback callback) { _disconnectCallback = callback; }

    // ========================================================================
    // メッセージ送信
    // ========================================================================

    /**
     * @brief 進捗通知を送信
     * @param clientId クライアントID
     * @param filename ファイル名
     * @param uploaded アップロード済みバイト数
     * @param total 総バイト数
     */
    void sendProgress(uint8_t clientId, const char* filename, 
                     uint32_t uploaded, uint32_t total);

    /**
     * @brief 完了通知を送信
     * @param clientId クライアントID
     * @param filename ファイル名
     * @param success 成功フラグ
     */
    void sendComplete(uint8_t clientId, const char* filename, bool success);

    /**
     * @brief エラー通知を送信
     * @param clientId クライアントID
     * @param errorCode エラーコード
     * @param message エラーメッセージ
     */
    void sendError(uint8_t clientId, uint8_t errorCode, const char* message);

    /**
     * @brief テキストメッセージを送信
     * @param clientId クライアントID
     * @param message メッセージ
     */
    void sendText(uint8_t clientId, const char* message);

    /**
     * @brief バイナリデータを送信
     * @param clientId クライアントID
     * @param data データ
     * @param length データ長
     */
    void sendBinary(uint8_t clientId, const uint8_t* data, size_t length);

    /**
     * @brief 全クライアントにブロードキャスト
     * @param message メッセージ
     */
    void broadcast(const char* message);

    // ========================================================================
    // クライアント管理
    // ========================================================================

    /**
     * @brief 接続中のクライアント数を取得
     * @return クライアント数
     */
    uint8_t getClientCount() const;

    /**
     * @brief クライアントが接続中かチェック
     * @param clientId クライアントID
     * @return 接続中ならtrue
     */
    bool isClientConnected(uint8_t clientId) const;

    /**
     * @brief クライアントを切断
     * @param clientId クライアントID
     */
    void disconnectClient(uint8_t clientId);

    // ========================================================================
    // 設定
    // ========================================================================

    /**
     * @brief 最大チャンクサイズを設定
     * @param size チャンクサイズ（バイト）
     */
    void setMaxChunkSize(uint32_t size) { _maxChunkSize = size; }

    /**
     * @brief タイムアウト時間を設定
     * @param timeoutMs タイムアウト時間（ミリ秒）
     */
    void setTimeout(uint32_t timeoutMs) { _timeoutMs = timeoutMs; }

    /**
     * @brief デバッグレベルを設定
     * @param level ログレベル
     */
    void setDebugLevel(uint8_t level) { _debugLevel = level; }

private:
    WebSocketsServer* _server;
    uint16_t _port;
    bool _isRunning;
    uint32_t _maxChunkSize;
    uint32_t _timeoutMs;
    uint8_t _debugLevel;
    
    WSFileInfoCallback _fileInfoCallback;
    WSDataCallback _dataCallback;
    WSMessageCallback _messageCallback;
    WSClientCallback _connectCallback;
    WSClientCallback _disconnectCallback;

    /**
     * @brief WebSocketイベントハンドラー
     */
    void _handleWebSocketEvent(uint8_t clientId, WStype_t type, 
                               uint8_t* payload, size_t length);

    /**
     * @brief テキストメッセージを処理
     */
    void _handleTextMessage(uint8_t clientId, const char* message);

    /**
     * @brief バイナリメッセージを処理
     */
    void _handleBinaryMessage(uint8_t clientId, const uint8_t* data, size_t length);

    /**
     * @brief JSONメッセージを作成
     */
    String _createJsonMessage(WSMessageType type, const char* data);

    /**
     * @brief ログ出力
     */
    void _log(uint8_t level, const char* format, ...);
};

#endif // WEBSOCKET_HANDLER_H
