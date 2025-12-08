#ifndef M5STACK_WIFI_UPLOADER_CONFIG_H
#define M5STACK_WIFI_UPLOADER_CONFIG_H

// ============================================================================
// ライブラリバージョン
// ============================================================================
#define M5STACK_WIFI_UPLOADER_VERSION "1.0.0"
#define M5STACK_WIFI_UPLOADER_VERSION_MAJOR 1
#define M5STACK_WIFI_UPLOADER_VERSION_MINOR 0
#define M5STACK_WIFI_UPLOADER_VERSION_PATCH 0

// ============================================================================
// デフォルト設定
// ============================================================================

// HTTPサーバーのデフォルトポート
#define DEFAULT_HTTP_PORT 80

// デフォルトアップロードパス
#define DEFAULT_UPLOAD_PATH "/uploads"

// デフォルト最大ファイルサイズ（50MB）
#define DEFAULT_MAX_FILE_SIZE (50 * 1024 * 1024)

// デフォルト受信バッファサイズ
#define DEFAULT_BUFFER_SIZE 4096

// デフォルトタイムアウト（ミリ秒）
#define DEFAULT_TIMEOUT 30000

// ============================================================================
// WebSocket設定
// ============================================================================
#define DEFAULT_WS_PORT 81
#define DEFAULT_WS_MAX_CHUNK_SIZE 4096
#define DEFAULT_WS_TIMEOUT 30000

// ============================================================================
// 再試行設定
// ============================================================================
#define DEFAULT_RETRY_MAX_ATTEMPTS 3
#define DEFAULT_RETRY_INITIAL_DELAY 1000
#define DEFAULT_RETRY_MAX_DELAY 10000
#define DEFAULT_RETRY_BACKOFF_MULTIPLIER 2.0f

// ============================================================================
// 機能設定
// ============================================================================

// WebSocketサポートを有効化
#define ENABLE_WEBSOCKET 1

// ファイル上書き保護を有効化
#define ENABLE_OVERWRITE_PROTECTION 0

// デバッグログを有効化
#define ENABLE_DEBUG_LOG 1

// ============================================================================
// パフォーマンス設定
// ============================================================================

// 同時アップロード数の上限
#define MAX_CONCURRENT_UPLOADS 3

// セッションIDの最大値
#define MAX_SESSION_ID 255

// ファイルリスト取得時の最大ファイル数
#define MAX_FILE_LIST_SIZE 1000

// ============================================================================
// セキュリティ設定
// ============================================================================

// ファイル名の最大長
#define MAX_FILENAME_LENGTH 255

// パスの最大深さ
#define MAX_PATH_DEPTH 10

// 許可する拡張子（デフォルト）
#define DEFAULT_ALLOWED_EXTENSIONS \
    "jpg", "jpeg", "png", "gif", "bmp", \
    "bin", "dat", "txt", "csv", "json"

#define DEFAULT_ALLOWED_EXTENSIONS_COUNT 10

// ============================================================================
// M5Stackモデル別設定
// ============================================================================

// M5Stack Core / Core2 / CoreS3 の共通設定
#define M5STACK_SD_CS_PIN 4

// ============================================================================
// ログレベル定義
// ============================================================================
#define LOG_LEVEL_NONE    0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

// デフォルトログレベル
#define DEFAULT_LOG_LEVEL LOG_LEVEL_INFO

// ============================================================================
// エラーコード定義
// ============================================================================
#define ERR_SUCCESS             0
#define ERR_FILE_TOO_LARGE      1
#define ERR_INVALID_EXTENSION   2
#define ERR_SD_WRITE_FAILED     3
#define ERR_INVALID_REQUEST     4
#define ERR_TIMEOUT             5
#define ERR_OUT_OF_MEMORY       6
#define ERR_UNKNOWN             255

// ============================================================================
// HTTP ステータスコード
// ============================================================================
#define HTTP_OK                 200
#define HTTP_BAD_REQUEST        400
#define HTTP_NOT_FOUND          404
#define HTTP_METHOD_NOT_ALLOWED 405
#define HTTP_INTERNAL_ERROR     500

// ============================================================================
// マジックナンバー定義
// ============================================================================

// JPEG
#define JPEG_MAGIC_0 0xFF
#define JPEG_MAGIC_1 0xD8
#define JPEG_MAGIC_2 0xFF

// PNG
#define PNG_MAGIC_0 0x89
#define PNG_MAGIC_1 0x50
#define PNG_MAGIC_2 0x4E
#define PNG_MAGIC_3 0x47

// GIF
#define GIF_MAGIC_0 0x47
#define GIF_MAGIC_1 0x49
#define GIF_MAGIC_2 0x46

// BMP
#define BMP_MAGIC_0 0x42
#define BMP_MAGIC_1 0x4D

#endif // M5STACK_WIFI_UPLOADER_CONFIG_H
