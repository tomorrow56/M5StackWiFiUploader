# M5Stack WiFi File Uploader

[![Version](https://img.shields.io/badge/version-1.4.0-blue.svg)](https://github.com/tomorrow56/M5StackWiFiUploader)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32-lightgrey.svg)](https://www.espressif.com/en/products/socs/esp32)

**M5StackのSDカードにWiFi経由でファイルをアップロードするためのArduinoライブラリです**

このライブラリは、M5Stackデバイス上でHTTP/WebSocketサーバーを起動し、ブラウザ経由で写真、バイナリ、テキストファイルなどをSDカードに直接アップロードする機能を提供します。プログレス表示、自動再試行、エラーハンドリングなど、ファイル転送を実現するために必要な機能を備えています。

[English Version](README_EN.md)

## 主な特徴

- **デュアルプロトコル対応**: 高速なWebSocketと互換性の高いHTTPの両方をサポート
- **エラーハンドリング**: 14種類のエラーコードと自動再試行機能
- **プログレス表示**: 転送速度、残り時間、全体進捗などをリアルタイムで追跡
- **モダンWeb UI**: ドラッグ＆ドロップ対応のレスポンシブなWebインターフェース
- **柔軟な設定**: ファイルサイズ、拡張子、同時アップロード数などを自由に設定可能
- **コールバック**: アップロードの各段階（開始、進捗、完了、エラー）でカスタム処理を実行可能
- **操作の制御**: アップロードの一時停止、再開、キャンセルが可能
- **ファイルダウンロード**: SDカード内のファイルをWeb UI経由でダウンロード可能
- **ファイル一覧表示**: ファイル名、サイズ、更新日時を表形式で表示
- **軽量設計**: ESP32の標準ライブラリを中心に構成され、外部依存を最小限に抑制

## 対応モデル

- M5Stack Core
- M5Stack Core2
- M5Stack CoreS3

## 依存ライブラリ

- **M5Unified** 0.2.11以降
- `WiFi`, `WebServer`, `FS`, `SD` (ESP32 Arduinoライブラリに内蔵)
- `WebSocketsServer`
- `ArduinoJson`

## システム構成

![system diagram](img/system.png)

## クラス構成

![class diagram](img/class.png)

## インストール

1.  リリースページから最新版の `M5StackWiFiUploader.zip` をダウンロードします。
2.  Arduino IDEで `スケッチ` -> `ライブラリをインクルード` -> `.ZIP形式のライブラリをインストール` を選択します。
3.  ダウンロードしたZIPファイルを選択してインストールします。

## 使い方

基本的な使い方は `examples/FullFeaturedDemo/FullFeaturedDemo.ino` を参照してください。

```cpp
#include <M5Unified.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"
#include "SDCardManager.h"

const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASSWORD = "your_password";

M5StackWiFiUploader uploader;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
    
    // SDカード初期化
    if (!SDCardManager::initialize()) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    
    // WiFi接続
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500); 
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    // WebSocketを有効化
    uploader.enableWebSocket(true);

    // アップローダーを開始
    if (uploader.begin(80, "/uploads")) {
        Serial.println("Server started successfully!");
        Serial.printf("Server URL: http://%s\n", WiFi.localIP().toString().c_str());
    }

    // コールバック設定
    uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
        if (success) {
            Serial.printf("Upload complete: %s\n", filename);
        }
    });
}

void loop() {
    M5.update();
    uploader.handleClient();
}
```

## ファイルダウンロード機能

### 主な機能

- **ファイル一覧表示**: SDカード内のファイルを表形式で表示
- **詳細情報表示**: ファイル名、サイズ、更新日時を確認
- **ダウンロード**: ファイルをローカルにダウンロード
- **削除**: 不要なファイルを削除
- **更新**: ファイル一覧を手動で更新

### Web UI

Web UIでは、以下の情報が表示されます：

| 項目 | 説明 |
|------|------|
| ファイル名 | クリックでダウンロード |
| サイズ | 人間が読みやすい形式（KB, MB） |
| 更新日時 | YYYY-MM-DD HH:MM形式 |
| 操作 | ダウンロード・削除ボタン |

### API エンドポイント

| エンドポイント | メソッド | 説明 |
|---------------|---------|------|
| `/api/files/list` | GET | 詳細なファイル一覧取得 |
| `/api/download` | GET | ファイルダウンロード |

#### `/api/files/list` レスポンス例

```json
{
  "files": [
    {
      "name": "photo.jpg",
      "size": 1024000,
      "modified": 1702123456,
      "isDirectory": false,
      "extension": "jpg"
    }
  ],
  "total": 1
}
```

#### `/api/download` パラメータ

- `filename`: ダウンロードするファイル名

例: `/api/download?filename=photo.jpg`

### セキュリティ

- パストラバーサル攻撃を防止（`..`, `/`, `\`を含むファイル名を拒否）
- ファイル存在チェック
- Content-Dispositionヘッダーによる適切なファイル名設定

## 関連ドキュメント

- **[APIリファレンス](docs/API_REFERENCE_v1.3.md)**
- **[FAQ](docs/TROUBLESHOOTING.md)**
- **[Exampleの説明](examples/README.md)**

## 貢献

バグ報告や機能提案は [GitHub Issues](https://github.com/tomorrow56/M5StackWiFiUploader/issues) までお願いします。

## ライセンス

このライブラリは **MITライセンス** の下で公開されています。詳細は `LICENSE` ファイルを参照してください。
