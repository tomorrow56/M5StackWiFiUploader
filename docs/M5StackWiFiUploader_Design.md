# M5Stack WiFi File Uploader Library - 設計書

## 1. 概要

M5StackのSDカードにWiFi経由で写真、バイナリ、テキストファイルをアップロードするArduinoライブラリ。HTTPサーバーとWebSocketの両方に対応し、複数ファイルの同時アップロードをサポート。

## 2. アーキテクチャ

```
┌─────────────────────────────────────────────────────┐
│          M5Stack WiFi File Uploader Library         │
├─────────────────────────────────────────────────────┤
│  Public API Layer                                   │
│  - M5StackUploader (メインクラス)                    │
│  - begin(), handleClient(), uploadFile()等          │
├─────────────────────────────────────────────────────┤
│  Protocol Layer                                     │
│  - HTTPServer (WebServer)                           │
│  - WebSocketHandler                                 │
├─────────────────────────────────────────────────────┤
│  File System Layer                                  │
│  - SDCardManager (ファイル操作)                      │
│  - FileValidator (ファイル検証)                      │
├─────────────────────────────────────────────────────┤
│  Utility Layer                                      │
│  - Logger                                           │
│  - BufferManager                                    │
└─────────────────────────────────────────────────────┘
```

## 3. ファイル構成

```
M5StackWiFiUploader/
├── src/
│   ├── M5StackWiFiUploader.h          (メインヘッダ)
│   ├── M5StackWiFiUploader.cpp        (メイン実装)
│   ├── HTTPUploadHandler.h            (HTTP処理)
│   ├── HTTPUploadHandler.cpp
│   ├── WebSocketHandler.h             (WebSocket処理)
│   ├── WebSocketHandler.cpp
│   ├── SDCardManager.h                (SDカード操作)
│   ├── SDCardManager.cpp
│   ├── FileValidator.h                (ファイル検証)
│   ├── FileValidator.cpp
│   └── Config.h                       (設定定義)
├── examples/
│   ├── HTTPUploadExample.ino
│   ├── WebSocketUploadExample.ino
│   └── MultiFileUploadExample.ino
├── library.properties
└── README.md
```

## 4. API仕様

### 4.1 メインクラス: M5StackUploader

```cpp
class M5StackUploader {
public:
    // 初期化
    bool begin(uint16_t port = 80, const char* uploadPath = "/uploads");
    
    // クライアント処理
    void handleClient();
    
    // アップロード設定
    void setMaxFileSize(uint32_t maxSize);
    void setAllowedExtensions(const char** extensions, uint8_t count);
    void setUploadPath(const char* path);
    
    // コールバック設定
    void onUploadStart(UploadCallback callback);
    void onUploadProgress(ProgressCallback callback);
    void onUploadComplete(CompleteCallback callback);
    void onUploadError(ErrorCallback callback);
    
    // ステータス取得
    uint32_t getTotalUploaded();
    uint32_t getActiveUploads();
    bool isRunning();
    
    // WebSocket対応
    void enableWebSocket(bool enable = true);
    
    // ログ設定
    void setDebugLevel(uint8_t level);
};
```

### 4.2 コールバック型定義

```cpp
// アップロード開始
typedef std::function<void(const char* filename, uint32_t filesize)> UploadCallback;

// 進捗通知（バイト数）
typedef std::function<void(const char* filename, uint32_t uploaded, uint32_t total)> ProgressCallback;

// アップロード完了
typedef std::function<void(const char* filename, uint32_t filesize, bool success)> CompleteCallback;

// エラー通知
typedef std::function<void(const char* filename, uint8_t errorCode, const char* message)> ErrorCallback;
```

### 4.3 エラーコード

```cpp
enum UploadError {
    ERR_SUCCESS = 0,
    ERR_FILE_TOO_LARGE = 1,
    ERR_INVALID_EXTENSION = 2,
    ERR_SD_WRITE_FAILED = 3,
    ERR_INVALID_REQUEST = 4,
    ERR_TIMEOUT = 5,
    ERR_OUT_OF_MEMORY = 6,
    ERR_UNKNOWN = 255
};
```

## 5. 通信プロトコル

### 5.1 HTTP マルチパートフォーム

**リクエスト:**
```
POST /api/upload HTTP/1.1
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary

------WebKitFormBoundary
Content-Disposition: form-data; name="file"; filename="photo.jpg"
Content-Type: image/jpeg

[バイナリデータ]
------WebKitFormBoundary--
```

**レスポンス:**
```json
{
    "success": true,
    "filename": "photo.jpg",
    "size": 102400,
    "path": "/uploads/photo.jpg",
    "timestamp": 1234567890
}
```

### 5.2 WebSocket フレーム

**ファイルメタデータ:**
```json
{
    "type": "file_meta",
    "filename": "photo.jpg",
    "filesize": 102400,
    "checksum": "abc123def456"
}
```

**ファイルデータ:**
```
バイナリフレーム: [チャンク1] [チャンク2] ...
各チャンク: 4096バイト（最後は可変）
```

**完了通知:**
```json
{
    "type": "upload_complete",
    "filename": "photo.jpg",
    "success": true,
    "path": "/uploads/photo.jpg"
}
```

## 6. 実装仕様

### 6.1 バッファ管理

- 受信バッファサイズ: 4096バイト（チャンク単位）
- 複数ファイル対応: 最大3同時アップロード
- メモリ効率: ストリーミング処理で大容量ファイル対応

### 6.2 ファイル検証

- 拡張子ホワイトリスト: jpg, jpeg, png, bin, txt, dat等
- ファイルサイズ制限: デフォルト50MB（カスタマイズ可能）
- ファイル名サニタイズ: 危険な文字を除去

### 6.3 SDカード操作

- ディレクトリ自動作成
- ファイル上書き保護（オプション）
- 容量チェック機能
- ファイルシステム互換性: FAT32対応

## 7. 対応M5Stackモデル

- M5Stack Core
- M5Stack Core2
- M5Stack CoreS3

各モデルのWiFi、SDカード機能を活用。

## 8. 依存ライブラリ

- WebServer（ESP32内蔵）
- FS / SD（ESP32内蔵）
- WiFi（ESP32内蔵）
- ArduinoJson（オプション）

