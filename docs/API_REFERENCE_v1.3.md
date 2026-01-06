
# M5Stack WiFi Uploader - APIリファレンス (v1.3.0)

**バージョン**: 1.3.0  
**最終更新日**: 2025年12月9日

---

## 1. 概要

このドキュメントは、`M5StackWiFiUploader` ライブラリの全APIについて詳細に説明します。ライブラリは以下の主要コンポーネントで構成されています。

- **M5StackWiFiUploader**: メインクラス。HTTP/WebSocketサーバー、ファイル操作、コールバック管理など、ライブラリ全体の制御を行います。
- **ErrorHandler**: エラーの記録、管理、通知を行います。
- **RetryManager**: ネットワークエラー発生時の自動再試行を管理します。
- **ProgressTracker**: ファイルアップロードの進捗（速度、残り時間など）を追跡します。
- **WebSocketHandler**: WebSocket通信プロトコルを処理します。
- **SDCardManager**: SDカードへのファイル書き込み、読み取り、削除などの操作を抽象化します。
- **FileValidator**: ファイル名、拡張子、サイズ、マジックナンバーなどの検証を行います。

---

## 2. M5StackWiFiUploader クラス

ライブラリのメインクラスです。すべての機能はこのクラスを通じて利用します。

### 2.1 コンストラクタ

`M5StackWiFiUploader(uint16_t port = 80);`

Uploaderオブジェクトを生成します。

- **`port`**: HTTPサーバーがリッスンするポート番号（デフォルト: 80）。

### 2.2 初期化・制御

`bool begin(uint16_t port = 80, const char* uploadPath = "/uploads");`

ライブラリを初期化し、HTTPサーバーと（有効な場合は）WebSocketサーバーを開始します。

- **`port`**: HTTPサーバーのポート番号。
- **`uploadPath`**: SDカード上のアップロード先ディレクトリ。
- **戻り値**: 初期化に成功した場合は `true`。

`void handleClient();`

クライアントからのリクエストを処理します。Arduinoの `loop()` 関数内で定期的に呼び出す必要があります。

`void end();`

すべてのサーバーを停止し、リソースを解放します。

`bool isRunning() const;`

サーバーが稼働中かどうかを返します。

### 2.3 設定

`void setMaxFileSize(uint32_t maxSize);`

アップロード可能な最大ファイルサイズ（バイト単位）を設定します。

`void setAllowedExtensions(const char** extensions, uint8_t count);`

アップロードを許可する拡張子を設定します。

- **`extensions`**: 拡張子の文字列配列 (例: `{"jpg", "png"}`)
- **`count`**: 配列の要素数

`void setUploadPath(const char* path);`

SDカード上のアップロード先ディレクトリを設定します。

`void setDebugLevel(uint8_t level);`

シリアルモニタに出力するログのレベルを設定します (0:なし, 1:エラー, 2:警告, 3:情報, 4:デバッグ)。

`void enableWebSocket(bool enable = true);`

WebSocketサポートを有効または無効にします。`begin()` の前に呼び出す必要があります。

`void setOverwriteProtection(bool enable = true);`

同名ファイルが存在する場合に上書きを禁止するかどうかを設定します。

### 2.4 コールバック設定

`void onUploadStart(UploadCallback callback);`

ファイルアップロード開始時に呼び出されるコールバック関数を設定します。

`void onUploadProgress(ProgressCallback callback);`

アップロード中に定期的に呼び出されるコールバック関数を設定します。

`void onUploadComplete(CompleteCallback callback);`

アップロード完了時に呼び出されるコールバック関数を設定します。

`void onUploadError(ErrorCallback callback);`

エラー発生時に呼び出されるコールバック関数を設定します。

### 2.5 ステータス取得

`uint32_t getTotalUploaded() const;`

これまでにアップロードされた総バイト数を返します。

`uint8_t getActiveUploads() const;`

現在アクティブなアップロードの数を返します。

`String getServerIP() const;`

サーバーのIPアドレスを返します。

`String getServerURL() const;`

サーバーの完全なURL (`http://...`) を返します。

### 2.6 ユーティリティ

`uint32_t getSDFreeSpace() const;`

SDカードの空き容量（バイト単位）を返します。

`uint32_t getSDTotalSpace() const;`

SDカードの総容量（バイト単位）を返します。

`bool fileExists(const char* filename) const;`

アップロードディレクトリ内に指定されたファイルが存在するかどうかを返します。

`bool deleteFile(const char* filename);`

指定されたファイルを削除します。

`std::vector<String> listFiles(const char* path = nullptr);`

指定されたディレクトリ内のファイル一覧を返します。`path` が `nullptr` の場合はデフォルトのアップロードパスが使用されます。


---

## 3. ErrorHandler クラス

エラーの詳細な記録と管理を行います。`M5StackWiFiUploader` クラスに内蔵されており、直接操作する必要は通常ありません。

### 3.1 メソッド

`void logError(uint8_t code, const char* message, const char* filename = nullptr, uint32_t filesize = 0);`

エラーを記録します。

`ErrorInfo getLastError() const;`

最後に発生したエラーの詳細情報を返します。

`const std::vector<ErrorInfo>& getErrorHistory() const;`

これまでに記録されたエラーの履歴（最大50件）を返します。

`void clearHistory();`

エラー履歴を消去します。

`static bool isRecoverable(uint8_t errorCode);`

指定されたエラーコードが回復可能（再試行可能）かどうかを返します。

### 3.2 ErrorInfo 構造体

```cpp
struct ErrorInfo {
    uint32_t timestamp;      // タイムスタンプ
    uint8_t code;            // エラーコード
    String message;          // エラーメッセージ
    String filename;         // ファイル名
    uint32_t filesize;       // ファイルサイズ
    bool recoverable;        // 回復可能フラグ
};
```

---

## 4. RetryManager クラス

回復可能なエラーが発生した際の自動再試行を管理します。

### 4.1 メソッド

`void setMaxRetries(uint8_t attempts);`

最大再試行回数を設定します。

`void setStrategy(RetryStrategy strategy, uint32_t initialDelay = 1000, float backoffMultiplier = 2.0);`

再試行戦略を設定します。

- **`strategy`**: `RETRY_FIXED`, `RETRY_LINEAR`, `RETRY_EXPONENTIAL`
- **`initialDelay`**: 初期遅延時間（ミリ秒）
- **`backoffMultiplier`**: バックオフ乗数（指数バックオフ時に使用）

`bool executeWithRetry(std::function<bool()> operation, uint8_t errorCode);`

指定された操作を再試行ロジック付きで実行します。


---

## 5. ProgressTracker クラス

ファイルアップロードの進捗を詳細に追跡します。

### 5.1 メソッド

`uint8_t startUpload(const char* filename, uint32_t totalBytes);`

新しいアップロードの追跡を開始します。

`void updateProgress(uint8_t sessionId, uint32_t uploadedBytes);`

進捗を更新します。

`void completeUpload(uint8_t sessionId, bool success = true);`

アップロードを完了としてマークします。

`ProgressInfo getProgress(uint8_t sessionId) const;`

指定されたセッションの進捗情報を取得します。

`OverallProgress getOverallProgress() const;`

複数ファイルアップロード時の全体進捗を取得します。

`static String formatBytes(uint32_t bytes);`

バイト数を人間が読みやすい形式（KB, MB, GB）に変換します。

`static String formatSpeed(float bytesPerSecond);`

転送速度を人間が読みやすい形式（KB/s, MB/s）に変換します。

`static String formatTime(uint32_t seconds);`

秒数を人間が読みやすい形式（分, 時間）に変換します。

### 5.2 ProgressInfo 構造体

```cpp
struct ProgressInfo {
    String filename;         // ファイル名
    uint32_t totalBytes;     // 総バイト数
    uint32_t uploadedBytes;  // アップロード済みバイト数
    uint8_t percentage;      // 進捗率
    float transferSpeed;     // 転送速度 (B/s)
    uint32_t remainingTime;  // 残り時間 (秒)
};
```

---

## 6. WebSocketHandler クラス

WebSocket通信を処理します。

### 6.1 メソッド

`bool begin();`

WebSocketサーバーを開始します。

`void handleClient();`

クライアントリクエストを処理します。

`void sendProgress(uint8_t clientId, ...);`

クライアントに進捗を通知します。

`void sendComplete(uint8_t clientId, ...);`

クライアントに完了を通知します。

`void sendError(uint8_t clientId, ...);`

クライアントにエラーを通知します。

`void broadcast(const char* message);`

接続されているすべてのクライアントにメッセージを送信します。

