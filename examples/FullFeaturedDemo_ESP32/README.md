# ESP32 WiFi File Uploader - Full Featured Demo

ESP32を使用したWiFiファイルアップローダーのフル機能デモスケッチです。HTTPサーバーとWebSocketサーバーを提供し、SDカードへのファイルアップロード、リアルタイム進捗表示、LED状態インジケーターを実装しています。

## 主な機能

- **HTTPファイルアップロード**: Webブラウザから直接ファイルアップロード
- **WebSocketリアルタイム通信**: アップロード進捗とファイルリストのリアルタイム表示
- **LED状態インジケーター**: 5色LEDでシステム状態を視覚的に表示
- **AP/STAモード対応**: アクセスポイントモードとステーションモードの両方に対応
- **エラーハンドリング**: 詳細なエラー表示と回復処理
- **ファイル管理**: ファイル一覧表示、削除機能

## 必要なもの

### ハードウェア
- ESP32開発ボード
- WS2812B LED x1 (NeoPixelなど)
- SDカードモジュール
- SDカード (FAT32でフォーマット済み)
- ブレッドボードとジャンパー線

### ソフトウェア
- Arduino IDE 1.8.13+ または PlatformIO
- 必要なライブラリ（後述）

## PIN設定

### スケッチのPIN設定
スケッチ内のPIN設定は以下のようになっています。必要に応じて変更してください。

```cpp
// SDカードSPI設定 (FullFeaturedDemo_ESP32.ino 44-47行目)
#define SD_CS 5     // SDカードモジュール CS
#define SD_MOSI 23  // SDカードモジュール MOSI  
#define SD_MISO 19  // SDカードモジュール MISO
#define SD_SCK 18   // SDカードモジュール SCK

// LED設定 (FullFeaturedDemo_ESP32.ino 52-53行目)
#define LED_PIN 27          // WS2812Bのデータピン
#define NUM_LEDS 1          // LEDの数
#define LED_BRIGHTNESS 10    // 明るさ (0-255) ※setup()で設定
```

## ライブラリのインストール

Arduino IDEの場合:
1. `ツール > ライブラリを管理...` を開く
2. 以下のライブラリを検索してインストール:
   - `FastLED` by Daniel Garcia
   - `M5StackWiFiUploader` (GitHubから手動インストール)

PlatformIOの場合:
```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    fastled/FastLED@^3.6.0
    https://github.com/tomorrow56/M5StackWiFiUploader
```

## 設定

### WiFiモードの切り替え

**ステーションモード (デフォルト):**
既存のWiFiネットワークに接続するモードです。

```cpp
// FullFeaturedDemo_ESP32.ino 22行目
// #define APMODE  // コメントアウトでステーションモード

const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
```

**アクセスポイントモード:**
ESP32がWiFiアクセスポイントとして動作するモードです。

```cpp
// FullFeaturedDemo_ESP32.ino 22行目
#define APMODE  // アンコメントでアクセスポイントモード

const char* AP_SSID = "M5Stack-AP";
const char* AP_PASSWORD = "12345678";
const IPAddress AP_IP(192, 168, 4, 1);
```

### モードごとの違い

| 項目 | ステーションモード | アクセスポイントモード |
|------|------------------|-------------------|
| IPアドレス | ルーターからDHCPで取得 | 192.168.4.1 (固定) |
| 接続方法 | 既存WiFiに接続 | スマホ/PCで直接接続 |
| WebSocket | 有効 | 無効 |
| URL | `http://[ESP32のIP]/uploads` | `http://192.168.4.1/uploads` |

## LED状態インジケーター

| 状態 | 色 (STAモード) | 色 (APモード) | 点滅/点灯 | 説明 |
|------|----------------|---------------|-----------|------|
| 初期化 | 青 | 青 | 常時点灯 | システム起動中 |
| WiFi接続中 | 黄 | 黄 | 500ms点滅 | WiFi接続処理中 |
| 実行中 | 緑 | シアン | 常時点灯 | サーバー稼働中 |
| アップロード中 | 緑 | シアン | 200ms点滅 | ファイル転送中 |
| エラー | 赤 | 赤 | 500ms点滅 | エラー発生 |

## 使用方法

### 1. コンパイルとアップロード
1. ESP32をPCに接続
2. Arduino IDEまたはPlatformIOでスケッチをコンパイル
3. ESP32にアップロード

### 2. シリアルモニターで確認
- ボーレート: 115200
- IPアドレスとアクセスURLが表示されます

### 3. Webブラウザでアクセス

**ステーションモードの場合:**

- http://[ESP32のIPアドレス]/uploads

**アクセスポイントモードの場合:**
1. スマホ/PCで "M5Stack-AP" に接続 (パスワード: 12345678)
2. `http://192.168.4.1/uploads` にアクセス

### 4. ファイルアップロード
1. Webブラウザでアップロードページを開く
2. 「ファイルを選択」でファイルを選択
3. 「アップロード」ボタンをクリック
4. LEDがシアン/緑色で点滅し、完了すると常時点灯に戻ります

### 5. WebSocket機能 (有効時)
- リアルタイムでアップロード進捗を表示
- ファイルリストの自動更新
- エラー通知

## サポートされるファイル形式

### デフォルトのファイル形式
- **画像**: jpg, jpeg, png, gif, bmp
- **データ**: bin, dat, txt, csv, json
- **圧縮**: zip, rar, 7z, tar, gz
- **その他**: pdf, mp4, apk

### 許可ファイル形式の変更方法

許可ファイル形式は `configureUploader()` 関数内で設定されています。変更する場合は以下の手順で行ってください。

**現在の設定 (FullFeaturedDemo_ESP32.ino 277-284行目):**
```cpp
// 許可拡張子
const char* extensions[] = {
    "jpg", "jpeg", "png", "gif", "bmp",
    "bin", "dat", "txt", "csv", "json",
    "zip", "rar", "7z", "tar", "gz",
    "pdf", "mp4", "apk"
};
uploader.setAllowedExtensions(extensions, 18);  // 18個の拡張子
```

**変更例　1: 音声ファイルを追加する場合**
```cpp
const char* extensions[] = {
    "jpg", "jpeg", "png", "gif", "bmp",
    "bin", "dat", "txt", "csv", "json",
    "zip", "rar", "7z", "tar", "gz",
    "pdf", "mp4", "apk",
    "mp3", "wav", "flac", "aac"  // 音声ファイルを追加
};
uploader.setAllowedExtensions(extensions, 22);  // 22個に変更
```

**変更例2: すべての形式を許可する場合**
```cpp
const char* extensions[] = {
    "*"  // ワイルドカードですべての形式を許可
};
uploader.setAllowedExtensions(extensions, 1);
```

### 注意事項
- 拡張子は小文字で記述してください
- ピリオド (.) は含めないでください
- 配列の要素数と `setAllowedExtensions()` の第2引数は一致させる必要があります
- セキュリティ上、必要なファイル形式のみを許可することをお勧めします

## カスタマイズ

### WebSocketの有効/無効
```cpp
bool Websocket_Enabled = true;  // trueで有効、falseで無効
```

### 最大ファイルサイズの変更
```cpp
uploader.setMaxFileSize(100 * 1024 * 1024);  // 100MBに変更
```

### LED点滅周期の変更
```cpp
const unsigned long LED_BLINK_INTERVAL = 500;      // 通常点滅 (ms)
const unsigned long UPLOAD_BLINK_INTERVAL = 200;   // アップロード時点滅 (ms)
```

## シリアル出力例

```
WiFi File Uploader
[CONFIG] Upload callbacks configured
[CONFIG] Uploader configuration complete
Connecting WiFi...
WiFi Connected!
IP: 192.168.1.100
[INFO] WebSocket enabled
[INFO] M5StackWiFiUploader started on port 80
Ready!
IP: 192.168.1.100
File Upload URL: http://192.168.1.100/uploads
WebSocket: ws://192.168.1.100
[LED] State changed to: 2
[UPLOAD] Started: test.jpg (1024000 bytes)
[LED] State changed to: 3
[UPLOAD] Complete: test.jpg (1024000 bytes, SUCCESS)
[LED] State changed to: 2
```

## PlatformIOでの使用

`platformio.ini`:
```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    fastled/FastLED@^3.6.0
    https://github.com/tomorrow56/M5StackWiFiUploader
build_flags = 
    -DCORE_DEBUG_LEVEL=3
```

## ライセンス

このスケッチはMITライセンスの下で提供されています。自由に使用、変更、配布できます。

## 貢献

バグ報告や機能リクエストはGitHubのIssuesでお知らせください。

## 関連リンク

- [M5StackWiFiUploaderライブラリ](https://github.com/tomorrow56/M5StackWiFiUploader)
- [FastLEDライブラリ](https://github.com/FastLED/FastLED)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)

---

**バージョン**: 1.0.0  
**最終更新**: 2026年1月  
**対応ボード**: ESP32シリーズ全般
