# M5Stack WiFi File Uploader

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/m5stack/M5StackWiFiUploader)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32-lightgrey.svg)](https://www.espressif.com/en/products/socs/esp32)

**M5StackのSDカードにWiFi経由でファイルをアップロードするためのArduinoライブラリ**

このライブラリは、M5Stackデバイス上でHTTPサーバーを起動し、ブラウザ経由で写真、バイナリ、テキストファイルなどをSDカードに直接アップロードする機能を提供します。プログレス表示や複数ファイルの同時アップロードにも対応しており、M5Stack Core, Core2, CoreS3で動作します。

![Web UI](https://user-images.githubusercontent.com/xxxx/web-ui-screenshot.png)  
*（注: 上記はUIのイメージです）*

## 1. 特徴

- **HTTPサーバー**: M5Stack上で軽量なHTTPサーバーを起動し、ファイルアップロードを受け付けます。
- **Web UI**: PCやスマートフォンのブラウザから簡単にファイルをアップロードできるWebインターフェースを提供します。
- **複数ファイル対応**: 複数のファイルを同時に選択してアップロードできます。
- **プログレス表示**: アップロードの進捗状況をリアルタイムで確認できます。
- **コールバック**: アップロードの開始、進捗、完了、エラーの各イベントをフックできます。
- **SDカード操作**: ファイルの保存、一覧表示、削除などの基本的なファイル操作をサポートします。
- **設定可能**: 最大ファイルサイズ、許可する拡張子などを自由に設定できます。
- **軽量設計**: ESP32の標準ライブラリを中心に構成されており、外部依存を最小限に抑えています。

## 2. 対応モデル

- **M5Stack Core**
- **M5Stack Core2**
- **M5Stack CoreS3**

## 3. 依存ライブラリ

- `WiFi` (ESP32内蔵)
- `WebServer` (ESP32内蔵)
- `FS` / `SD` (ESP32内蔵)

これらのライブラリはESP32 Arduinoコアに標準で含まれているため、追加のインストールは不要です。

## 4. インストール

1.  このリポジトリをZIPファイルとしてダウンロードします。
2.  Arduino IDEで `スケッチ` -> `ライブラリをインクルード` -> `.ZIP形式のライブラリをインストール` を選択します。
3.  ダウンロードしたZIPファイルを選択してインストールします。

## 5. 使い方

### 5.1 基本的な流れ

1.  `M5StackWiFiUploader` オブジェクトを作成します。
2.  WiFiに接続します。
3.  `uploader.begin()` でサーバーを開始します。
4.  必要に応じてコールバック関数 (`onUploadStart`, `onUploadProgress` など) を設定します。
5.  `loop()` 内で `uploader.handleClient()` を呼び出してリクエストを処理します。

### 5.2 サンプルコード

基本的な使い方は `examples/HTTPUploadExample.ino` を参照してください。

```cpp
#include <M5Stack.h>
#include <WiFi.h>
#include "M5StackWiFiUploader.h"

// WiFi設定
const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASSWORD = "your_password";

// Uploaderオブジェクト
M5StackWiFiUploader uploader;

void setup() {
    M5.begin();
    Serial.begin(115200);

    // WiFi接続
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());

    // アップローダーを開始
    if (uploader.begin()) {
        Serial.printf("Server started at: %s\n", uploader.getServerURL().c_str());
    }

    // コールバック設定
    uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
        if (success) {
            Serial.printf("Upload complete: %s (%d bytes)\n", filename, size);
        }
    });
}

void loop() {
    uploader.handleClient();
    delay(10);
}
```

### 5.3 APIリファレンス

#### `bool begin(uint16_t port = 80, const char* uploadPath = "/uploads")`

HTTPサーバーを初期化して開始します。

- `port`: サーバーのポート番号
- `uploadPath`: SDカード上のアップロード先ディレクトリ

#### `void handleClient()`

クライアントからのリクエストを処理します。`loop()` 内で定期的に呼び出す必要があります。

#### `void setMaxFileSize(uint32_t maxSize)`

アップロード可能な最大ファイルサイズ（バイト単位）を設定します。

#### `void setAllowedExtensions(const char** extensions, uint8_t count)`

アップロードを許可する拡張子を設定します。

```cpp
const char* extensions[] = {"jpg", "png", "bin"};
uploader.setAllowedExtensions(extensions, 3);
```

#### コールバック関数

- `onUploadStart(callback)`: アップロード開始時に呼ばれます。
- `onUploadProgress(callback)`: アップロード中に定期的に呼ばれます。
- `onUploadComplete(callback)`: アップロード完了時に呼ばれます。
- `onUploadError(callback)`: エラー発生時に呼ばれます。

## 6. Webインターフェース

1.  M5Stackを起動し、シリアルモニタに表示されるIPアドレスまたはURLにアクセスします。
    - 例: `http://192.168.1.10`
2.  表示されたWebページで「ファイルを選択」ボタンをクリックするか、ファイルをドラッグ＆ドロップします。
3.  アップロードが自動的に開始されます。
4.  ページ下部には、アップロード済みのファイル一覧が表示され、削除も可能です。

## 7. トラブルシューティング

- **SDカードが認識されない**: 
  - SDカードが正しく挿入されているか確認してください。
  - `SD.begin()` が成功しているか確認してください。
- **ファイルがアップロードできない**: 
  - WiFiに正しく接続されているか確認してください。
  - ファイルサイズが `maxFileSize` を超えていないか確認してください。
  - 拡張子が許可されているか確認してください。
- **Webページが表示されない**: 
  - M5StackとPC/スマートフォンが同じネットワークに接続されているか確認してください。
  - IPアドレスが正しいか確認してください。

## 8. ライセンス

このライブラリは **MITライセンス** の下で公開されています。詳細は `LICENSE` ファイルを参照してください。
