# M5Stack WiFi Uploader - クイックスタートガイド

このガイドでは、M5Stack WiFi Uploaderライブラリを使い始めるための手順を説明します。

## 1. インストール

### 1.1 ライブラリのダウンロード

1. [GitHubリポジトリ](https://github.com/m5stack/M5StackWiFiUploader)からZIPファイルをダウンロードします。
2. Arduino IDEで `スケッチ` → `ライブラリをインクルード` → `.ZIP形式のライブラリをインストール` を選択します。
3. ダウンロードしたZIPファイルを選択してインストールします。

### 1.2 必要なライブラリ

以下のライブラリがインストールされていることを確認してください。これらはESP32 Arduinoコアに標準で含まれています。

- WiFi
- WebServer
- FS / SD

## 2. 基本的なセットアップ

### 2.1 スケッチの作成

新しいスケッチを作成し、以下のコードを入力します。

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
    delay(100);
    
    // WiFiに接続
    Serial.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi Connected!");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    
    // アップローダーを開始
    if (uploader.begin()) {
        Serial.printf("Server started at: %s\n", uploader.getServerURL().c_str());
    } else {
        Serial.println("Failed to start server");
    }
}

void loop() {
    uploader.handleClient();
    delay(10);
}
```

### 2.2 WiFi認証情報の設定

スケッチ内の以下の部分を、実際のWiFi認証情報に置き換えます。

```cpp
const char* WIFI_SSID = "your_ssid";           // WiFiネットワーク名
const char* WIFI_PASSWORD = "your_password";   // WiFiパスワード
```

### 2.3 M5Stackへのアップロード

1. M5StackをUSBケーブルでPCに接続します。
2. Arduino IDEで `ツール` → `ボード` → `M5Stack-Core-ESP32` を選択します。
3. `ツール` → `ポート` で正しいシリアルポートを選択します。
4. `スケッチ` → `マイコンボードに書き込む` をクリックします。

## 3. サーバーへのアクセス

### 3.1 IPアドレスの確認

M5Stackが起動すると、シリアルモニタに以下のようなメッセージが表示されます。

```
WiFi Connected!
IP Address: 192.168.1.100
Server started at: http://192.168.1.100:80
```

### 3.2 Webインターフェースへのアクセス

1. PCまたはスマートフォンのブラウザを開きます。
2. アドレスバーに上記のIPアドレスを入力します（例: `http://192.168.1.100`）。
3. M5Stack WiFi Uploaderのページが表示されます。

## 4. ファイルのアップロード

### 4.1 単一ファイルのアップロード

1. Webページの「ファイルを選択」ボタンをクリックします。
2. アップロードしたいファイルを選択します。
3. アップロードが自動的に開始されます。
4. 進捗バーでアップロード状況を確認できます。

### 4.2 複数ファイルのアップロード

1. Webページの「ファイルを選択」ボタンをクリックします。
2. Ctrl+Clickで複数のファイルを選択します。
3. すべてのファイルが順番にアップロードされます。

### 4.3 ドラッグ&ドロップでのアップロード

1. PCからファイルをドラッグします。
2. Webページのアップロード領域にドロップします。
3. ファイルが自動的にアップロードされます。

## 5. コールバック関数の設定

アップロードイベントに対応するコールバック関数を設定できます。

```cpp
void setup() {
    // ... 初期化コード ...
    
    // アップロード開始時
    uploader.onUploadStart([](const char* filename, uint32_t size) {
        Serial.printf("Upload started: %s (%.2f MB)\n", filename, size / 1024.0 / 1024.0);
        M5.Lcd.printf("Uploading: %s\n", filename);
    });
    
    // アップロード進捗時
    uploader.onUploadProgress([](const char* filename, uint32_t uploaded, uint32_t total) {
        uint8_t progress = (uploaded * 100) / total;
        Serial.printf("Progress: %d%%\n", progress);
    });
    
    // アップロード完了時
    uploader.onUploadComplete([](const char* filename, uint32_t size, bool success) {
        if (success) {
            Serial.printf("Upload complete: %s\n", filename);
            M5.Lcd.printf("Complete: %s\n", filename);
        }
    });
    
    // エラー発生時
    uploader.onUploadError([](const char* filename, uint8_t code, const char* msg) {
        Serial.printf("Error: %s - %s\n", filename, msg);
    });
}
```

## 6. 設定のカスタマイズ

### 6.1 最大ファイルサイズの設定

```cpp
uploader.setMaxFileSize(100 * 1024 * 1024);  // 100MB
```

### 6.2 許可する拡張子の設定

```cpp
const char* extensions[] = {"jpg", "jpeg", "png", "gif", "bin", "txt"};
uploader.setAllowedExtensions(extensions, 6);
```

### 6.3 アップロードパスの変更

```cpp
uploader.setUploadPath("/photos");
```

### 6.4 デバッグレベルの設定

```cpp
uploader.setDebugLevel(3);  // 0=なし, 1=エラー, 2=警告, 3=情報, 4=詳細
```

## 7. ファイルの確認

### 7.1 Webインターフェースで確認

Webページの下部に「アップロード済みファイル」セクションがあり、アップロード済みのファイル一覧が表示されます。

### 7.2 シリアルモニタで確認

シリアルモニタに以下のような情報が表示されます。

```
[INFO] File uploaded successfully: photo.jpg (2048000 bytes)
[INFO] SD Free Space: 1024.00 MB / 2048.00 MB
```

### 7.3 SDカードで確認

M5StackのSDカードをPCに接続し、`/uploads` ディレクトリを確認することで、アップロードされたファイルを確認できます。

## 8. トラブルシューティング

### Q: Webページが表示されない

**A**: 以下を確認してください。

- M5StackとPCが同じWiFiネットワークに接続されているか
- IPアドレスが正しいか
- ファイアウォール設定でポート80がブロックされていないか

### Q: ファイルがアップロードできない

**A**: 以下を確認してください。

- ファイルサイズが最大ファイルサイズを超えていないか
- ファイルの拡張子が許可されているか
- SDカードに十分な空き容量があるか

### Q: M5Stackが再起動する

**A**: メモリ不足の可能性があります。

- 最大ファイルサイズを減らす
- バッファサイズを減らす
- 同時アップロード数を減らす

## 9. 次のステップ

- [API リファレンス](API_REFERENCE.md) で詳細なAPI仕様を確認
- [実装ガイド](IMPLEMENTATION_GUIDE.md) でライブラリの内部構造を学習
- [examples](../examples/) フォルダのサンプルコードを参照

## 10. サポート

問題が発生した場合は、以下の方法でサポートを受けられます。

- [GitHubのIssues](https://github.com/m5stack/M5StackWiFiUploader/issues)
- [M5Stack Community](https://community.m5stack.com/)
- [M5Stack Documentation](https://docs.m5stack.com/)
