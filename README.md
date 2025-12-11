# M5Stack WiFi File Uploader

[![Version](https://img.shields.io/badge/version-1.3.0-blue.svg)](https://github.com/tomorrow56/M5StackWiFiUploader)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32-lightgrey.svg)](https://www.espressif.com/en/products/socs/esp32)

**M5StackのSDカードにWiFi経由でファイルをアップロードするための高機能Arduinoライブラリ**

このライブラリは、M5Stackデバイス上でHTTP/WebSocketサーバーを起動し、ブラウザ経由で写真、バイナリ、テキストファイルなどをSDカードに直接アップロードする機能を提供します。詳細なプログレス表示、自動再試行、エラーハンドリングなど、堅牢なファイル転送を実現するための高度な機能を備えています。

[English Version](README_EN.md)

## ✨ 主な特徴

- **デュアルプロトコル対応**: 高速なWebSocketと互換性の高いHTTPの両方をサポート。
- **堅牢なエラーハンドリング**: 14種類のエラーコードと自動再試行機能（指数バックオフ対応）。
- **詳細なプログレス表示**: 転送速度、残り時間、全体進捗などをリアルタイムで追跡。
- **モダンなWeb UI**: ドラッグ＆ドロップ対応のレスポンシブなWebインターフェース。
- **柔軟な設定**: ファイルサイズ、拡張子、同時アップロード数などを自由に設定可能。
- **豊富なコールバック**: アップロードの各段階（開始、進捗、完了、エラー）でカスタム処理を実行可能。
- **操作の制御**: アップロードの一時停止、再開、キャンセルが可能。
- **ファイルダウンロード**: SDカード内のファイルをWeb UI経由でダウンロード可能。
- **ファイル一覧表示**: ファイル名、サイズ、更新日時を表形式で表示。
- **軽量設計**: ESP32の標準ライブラリを中心に構成され、外部依存を最小限に抑制。

## 🛠️ 対応モデル

- M5Stack Core
- M5Stack Core2
- M5Stack CoreS3

## 📚 依存ライブラリ

- `WiFi`, `WebServer`, `FS`, `SD` (ESP32コアに内蔵)
- `WebSocketsServer` (本ライブラリに同梱)
- `ArduinoJson` (本ライブラリに同梱)

## 🚀 インストール

1.  リリースページから最新版の `M5StackWiFiUploader.zip` をダウンロードします。
2.  Arduino IDEで `スケッチ` -> `ライブラリをインクルード` -> `.ZIP形式のライブラリをインストール` を選択します。
3.  ダウンロードしたZIPファイルを選択してインストールします。

## 💡 使い方

基本的な使い方は `examples/FullFeaturedDemo/FullFeaturedDemo.ino` を参照してください。

```cpp
#include <M5Stack.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"

const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASSWORD = "your_password";

M5StackWiFiUploader uploader;

void setup() {
    M5.begin();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }

    // WebSocketを有効化
    uploader.enableWebSocket(true);

    // アップローダーを開始
    uploader.begin();

    // コールバック設定
    uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
        if (success) {
            Serial.printf("Upload complete: %s\n", filename);
        }
    });
}

void loop() {
    uploader.handleClient();
}
```

## 📖 ドキュメント

- **[APIリファレンス](docs/API_REFERENCE_v1.1.md)**: 全クラス・メソッドの詳細な仕様。
- **[トラブルシューティングガイド](docs/TROUBLESHOOTING.md)**: よくある問題と解決策。

## 🤝 貢献

バグ報告や機能提案は [GitHub Issues](https://github.com/tomorrow56/M5StackWiFiUploader/issues) までお願いします。プルリクエストも歓迎します。

## 📜 ライセンス

このライブラリは **MITライセンス** の下で公開されています。詳細は `LICENSE` ファイルを参照してください。


## 📥 ファイルダウンロード機能

v1.3.0から、SDカード内のファイルをWeb UI経由でダウンロードできるようになりました。

### 主な機能

- **ファイル一覧表示**: SDカード内のファイルを表形式で表示
- **詳細情報表示**: ファイル名、サイズ、更新日時を確認
- **ダウンロード**: ファイルをローカルにダウンロード
- **削除**: 不要なファイルを削除
- **更新**: ファイル一覧を手動で更新

### Web UI

新しいWeb UIでは、以下の情報が表示されます：

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
