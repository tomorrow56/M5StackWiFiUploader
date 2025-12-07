# M5Stack WiFi Uploader - API リファレンス

## 目次

1. [M5StackWiFiUploader クラス](#m5stackwifiuploader-クラス)
2. [SDCardManager クラス](#sdcardmanager-クラス)
3. [FileValidator クラス](#filevalidator-クラス)
4. [エラーコード](#エラーコード)
5. [コールバック型](#コールバック型)

---

## M5StackWiFiUploader クラス

メインのアップローダークラスです。HTTPサーバーの管理とファイルアップロードの処理を行います。

### コンストラクタ

```cpp
M5StackWiFiUploader(uint16_t port = 80)
```

指定されたポートでHTTPサーバーを作成します。

**パラメータ**:
- `port`: HTTPサーバーのポート番号（デフォルト: 80）

**例**:
```cpp
M5StackWiFiUploader uploader(80);
```

### 初期化・制御

#### `bool begin(uint16_t port = 80, const char* uploadPath = "/uploads")`

HTTPサーバーを初期化して開始します。

**パラメータ**:
- `port`: HTTPサーバーのポート番号
- `uploadPath`: SDカード上のアップロード先ディレクトリ

**戻り値**: 初期化成功時`true`、失敗時`false`

**例**:
```cpp
if (uploader.begin(80, "/uploads")) {
    Serial.println("Server started");
}
```

#### `void handleClient()`

クライアントからのHTTPリクエストを処理します。`loop()` 内で定期的に呼び出す必要があります。

**例**:
```cpp
void loop() {
    uploader.handleClient();
    delay(10);
}
```

#### `void end()`

サーバーを停止します。

**例**:
```cpp
uploader.end();
```

#### `bool isRunning() const`

サーバーが稼働中かチェックします。

**戻り値**: 稼働中なら`true`

### 設定

#### `void setMaxFileSize(uint32_t maxSize)`

アップロード可能な最大ファイルサイズを設定します（バイト単位）。

**パラメータ**:
- `maxSize`: 最大ファイルサイズ（バイト）

**例**:
```cpp
uploader.setMaxFileSize(50 * 1024 * 1024);  // 50MB
```

#### `void setAllowedExtensions(const char** extensions, uint8_t count)`

アップロードを許可する拡張子を設定します。

**パラメータ**:
- `extensions`: 拡張子配列（例: `{"jpg", "png", "bin"}`）
- `count`: 拡張子の個数

**例**:
```cpp
const char* exts[] = {"jpg", "jpeg", "png", "gif", "bin"};
uploader.setAllowedExtensions(exts, 5);
```

#### `void setUploadPath(const char* path)`

アップロードファイルの保存先パスを設定します。

**パラメータ**:
- `path`: SDカード上のパス

**例**:
```cpp
uploader.setUploadPath("/photos");
```

#### `void setDebugLevel(uint8_t level)`

デバッグログレベルを設定します。

**パラメータ**:
- `level`: ログレベル（0=なし, 1=エラー, 2=警告, 3=情報, 4=詳細）

**例**:
```cpp
uploader.setDebugLevel(3);  // 情報レベル
```

#### `void enableWebSocket(bool enable = true)`

WebSocketサポートを有効化します（将来の拡張用）。

**パラメータ**:
- `enable`: `true`=有効, `false`=無効

#### `void setOverwriteProtection(bool enable = true)`

ファイル上書き保護を有効化します。有効時は、既存ファイルへのアップロードが拒否されます。

**パラメータ**:
- `enable`: `true`=有効, `false`=無効

### コールバック設定

#### `void onUploadStart(UploadCallback callback)`

アップロード開始時のコールバックを設定します。

**コールバック型**: `void(const char* filename, uint32_t filesize)`

**例**:
```cpp
uploader.onUploadStart([](const char* filename, uint32_t size) {
    Serial.printf("Upload started: %s (%.2f MB)\n", filename, size / 1024.0 / 1024.0);
});
```

#### `void onUploadProgress(ProgressCallback callback)`

アップロード進捗時のコールバックを設定します。

**コールバック型**: `void(const char* filename, uint32_t uploaded, uint32_t total)`

**例**:
```cpp
uploader.onUploadProgress([](const char* filename, uint32_t uploaded, uint32_t total) {
    uint8_t progress = (uploaded * 100) / total;
    Serial.printf("Progress: %d%%\n", progress);
});
```

#### `void onUploadComplete(CompleteCallback callback)`

アップロード完了時のコールバックを設定します。

**コールバック型**: `void(const char* filename, uint32_t filesize, bool success)`

**例**:
```cpp
uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
    if (success) {
        Serial.printf("Upload complete: %s\n", filename);
    } else {
        Serial.printf("Upload failed: %s\n", filename);
    }
});
```

#### `void onUploadError(ErrorCallback callback)`

エラー発生時のコールバックを設定します。

**コールバック型**: `void(const char* filename, uint8_t errorCode, const char* message)`

**例**:
```cpp
uploader.onUploadError([](const char* filename, uint8_t code, const char* msg) {
    Serial.printf("Error [%d]: %s - %s\n", code, filename, msg);
});
```

### ステータス取得

#### `uint32_t getTotalUploaded() const`

合計アップロードバイト数を取得します。

**戻り値**: アップロード済みバイト数

#### `uint8_t getActiveUploads() const`

現在進行中のアップロード数を取得します。

**戻り値**: アクティブなアップロード数

#### `String getServerIP() const`

サーバーのIPアドレスを取得します。

**戻り値**: IPアドレス文字列

#### `String getServerURL() const`

サーバーのURLを取得します。

**戻り値**: URL文字列（例: `http://192.168.1.10:80`）

### ユーティリティ

#### `uint32_t getSDFreeSpace() const`

SDカードの空き容量を取得します。

**戻り値**: 空き容量（バイト）

#### `uint32_t getSDTotalSpace() const`

SDカードの総容量を取得します。

**戻り値**: 総容量（バイト）

#### `bool fileExists(const char* filename) const`

ファイルが存在するかチェックします。

**パラメータ**:
- `filename`: ファイル名

**戻り値**: 存在すれば`true`

#### `bool deleteFile(const char* filename)`

ファイルを削除します。

**パラメータ**:
- `filename`: ファイル名

**戻り値**: 削除成功時`true`

#### `std::vector<String> listFiles(const char* path = nullptr)`

ディレクトリ内のファイル一覧を取得します。

**パラメータ**:
- `path`: ディレクトリパス（省略時はアップロードパス）

**戻り値**: ファイル名のベクタ

---

## SDCardManager クラス

SDカードのファイル操作を管理するユーティリティクラスです。すべてのメソッドは静的メソッドです。

### 初期化

#### `static bool initialize(uint8_t csPin = 4)`

SDカードを初期化します。

**パラメータ**:
- `csPin`: チップセレクトピン（デフォルト: 4）

**戻り値**: 初期化成功時`true`

#### `static bool isConnected()`

SDカードが接続されているかチェックします。

**戻り値**: 接続されていれば`true`

### ファイル操作

#### `static bool fileExists(const char* filepath)`

ファイルが存在するかチェックします。

#### `static bool deleteFile(const char* filepath)`

ファイルを削除します。

#### `static uint32_t getFileSize(const char* filepath)`

ファイルサイズを取得します。

#### `static uint32_t readFile(const char* filepath, uint8_t* buffer, uint32_t maxSize)`

ファイルを読み込みます。

#### `static bool writeFile(const char* filepath, const uint8_t* data, uint32_t size, bool append = false)`

ファイルに書き込みます。

**パラメータ**:
- `filepath`: ファイルパス
- `data`: 書き込みデータ
- `size`: データサイズ
- `append`: `true`=追記, `false`=上書き

#### `static bool writeText(const char* filepath, const char* text, bool append = false)`

テキストをファイルに書き込みます。

#### `static String readText(const char* filepath)`

ファイルを読み込んでテキストを取得します。

### ディレクトリ操作

#### `static bool dirExists(const char* dirpath)`

ディレクトリが存在するかチェックします。

#### `static bool createDir(const char* dirpath)`

ディレクトリを作成します。

#### `static bool deleteDir(const char* dirpath)`

ディレクトリを削除します。

#### `static std::vector<String> listFiles(const char* dirpath, bool includeDir = false)`

ディレクトリ内のファイル一覧を取得します。

#### `static uint32_t getFileCount(const char* dirpath)`

ディレクトリ内のファイル数を取得します。

### 容量管理

#### `static uint32_t getTotalSpace()`

SDカードの総容量を取得します。

#### `static uint32_t getUsedSpace()`

SDカードの使用容量を取得します。

#### `static uint32_t getFreeSpace()`

SDカードの空き容量を取得します。

#### `static uint8_t getUsagePercent()`

SDカードの使用率を取得します（0-100）。

---

## FileValidator クラス

ファイルの検証を行うユーティリティクラスです。すべてのメソッドは静的メソッドです。

### ファイル名検証

#### `static bool isValidFilename(const char* filename)`

ファイル名が有効かチェックします。

#### `static bool isSafeFilename(const char* filename)`

ファイル名に危険な文字が含まれていないかチェックします。

#### `static String sanitizeFilename(const char* filename)`

ファイル名をサニタイズします（危険な文字を除去）。

### 拡張子検証

#### `static bool isAllowedExtension(const char* filename, const char** allowedExtensions, uint8_t count)`

拡張子が許可されているかチェックします。

#### `static String getExtension(const char* filename)`

ファイルの拡張子を取得します。

### サイズ検証

#### `static bool isValidFileSize(uint32_t filesize, uint32_t maxSize, uint32_t minSize = 0)`

ファイルサイズが許可範囲内かチェックします。

#### `static String formatFileSize(uint32_t size)`

ファイルサイズを人間が読める形式に変換します。

**例**:
```cpp
String formatted = FileValidator::formatFileSize(1048576);  // "1.00 MB"
```

### MIMEタイプ検証

#### `static String getMimeType(const char* filename)`

ファイル名からMIMEタイプを推定します。

#### `static bool isImageMimeType(const char* mimeType)`

MIMEタイプが画像形式かチェックします。

#### `static bool isTextMimeType(const char* mimeType)`

MIMEタイプがテキスト形式かチェックします。

### ファイルコンテンツ検証

#### `static bool validateMagicNumber(const uint8_t* data, uint32_t size, const char* expectedType)`

ファイルのマジックナンバーを検証します。

#### `static bool isJPEG(const uint8_t* data, uint32_t size)`

JPEGファイルのマジックナンバーをチェックします。

#### `static bool isPNG(const uint8_t* data, uint32_t size)`

PNGファイルのマジックナンバーをチェックします。

#### `static bool isGIF(const uint8_t* data, uint32_t size)`

GIFファイルのマジックナンバーをチェックします。

#### `static bool isBMP(const uint8_t* data, uint32_t size)`

BMPファイルのマジックナンバーをチェックします。

### 総合検証

#### `static bool validateFile(...)`

ファイルの総合検証を行います。

**パラメータ**:
- `filename`: ファイル名
- `filesize`: ファイルサイズ
- `data`: ファイルデータ（オプション）
- `dataSize`: データサイズ（オプション）
- `allowedExtensions`: 許可する拡張子配列
- `extensionCount`: 拡張子の個数
- `maxFileSize`: 最大ファイルサイズ

---

## エラーコード

| コード | 定数 | 説明 |
|--------|------|------|
| 0 | `ERR_SUCCESS` | 成功 |
| 1 | `ERR_FILE_TOO_LARGE` | ファイルが大きすぎる |
| 2 | `ERR_INVALID_EXTENSION` | 無効な拡張子 |
| 3 | `ERR_SD_WRITE_FAILED` | SD書き込み失敗 |
| 4 | `ERR_INVALID_REQUEST` | 無効なリクエスト |
| 5 | `ERR_TIMEOUT` | タイムアウト |
| 6 | `ERR_OUT_OF_MEMORY` | メモリ不足 |
| 255 | `ERR_UNKNOWN` | 不明なエラー |

---

## コールバック型

```cpp
// アップロード開始
typedef std::function<void(const char* filename, uint32_t filesize)> UploadCallback;

// 進捗通知
typedef std::function<void(const char* filename, uint32_t uploaded, uint32_t total)> ProgressCallback;

// アップロード完了
typedef std::function<void(const char* filename, uint32_t filesize, bool success)> CompleteCallback;

// エラー通知
typedef std::function<void(const char* filename, uint8_t errorCode, const char* message)> ErrorCallback;
```
