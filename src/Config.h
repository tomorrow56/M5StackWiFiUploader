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
// フラッシュ削減モード設定
// ============================================================================
// LITE_MODE を有効にすると、フラッシュ使用量を大幅に削減（50-80KB削減）
// ESP32 4MBフラッシュなどリソースが限られた環境で使用
#ifndef LITE_MODE
#define LITE_MODE 0  // 1=有効（軽量モード）, 0=無効（フル機能）
#endif

// ============================================================================
// 機能設定（個別制御可能）
// ============================================================================
// スケッチ(.ino)で個別に設定する場合は、#include前に定義してください
// 例: #define ENABLE_WEBSOCKET 0

// WebSocketサポートを有効化（削減効果: 30-50KB）
#ifndef ENABLE_WEBSOCKET
  #if LITE_MODE
    #define ENABLE_WEBSOCKET 0
  #else
    #define ENABLE_WEBSOCKET 1
  #endif
#endif

// ファイル上書き保護を有効化
#ifndef ENABLE_OVERWRITE_PROTECTION
#define ENABLE_OVERWRITE_PROTECTION 0
#endif

// デバッグログを有効化（削減効果: 2-5KB）
#ifndef ENABLE_DEBUG_LOG
  #if LITE_MODE
    #define ENABLE_DEBUG_LOG 0
  #else
    #define ENABLE_DEBUG_LOG 1
  #endif
#endif

// 詳細な進捗追跡機能（転送速度、残り時間など）（削減効果: 10-15KB）
#ifndef ENABLE_PROGRESS_TRACKER_FULL
  #if LITE_MODE
    #define ENABLE_PROGRESS_TRACKER_FULL 0
  #else
    #define ENABLE_PROGRESS_TRACKER_FULL 1
  #endif
#endif

// 高度な再試行機能（指数バックオフなど）（削減効果: 5-10KB）
#ifndef ENABLE_RETRY_MANAGER_FULL
  #if LITE_MODE
    #define ENABLE_RETRY_MANAGER_FULL 0
  #else
    #define ENABLE_RETRY_MANAGER_FULL 1
  #endif
#endif

// 高度なHTTPエンドポイント（詳細リスト、ダウンロード、デバッグ）（削減効果: 3-5KB）
#ifndef ENABLE_ADVANCED_ENDPOINTS
  #if LITE_MODE
    #define ENABLE_ADVANCED_ENDPOINTS 0
  #else
    #define ENABLE_ADVANCED_ENDPOINTS 1
  #endif
#endif

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
