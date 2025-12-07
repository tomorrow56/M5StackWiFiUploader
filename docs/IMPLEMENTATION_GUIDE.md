# M5Stack WiFi Uploader - 実装ガイド

## 概要

M5Stack WiFi Uploaderライブラリは、M5StackデバイスのSDカードにWiFi経由でファイルをアップロードするための包括的なソリューションを提供します。このドキュメントでは、ライブラリの内部実装、各モジュールの役割、および拡張方法について説明します。

## アーキテクチャ

```
┌─────────────────────────────────────────────────────┐
│          M5Stack WiFi File Uploader Library         │
├─────────────────────────────────────────────────────┤
│  Public API Layer                                   │
│  M5StackWiFiUploader クラス                          │
├─────────────────────────────────────────────────────┤
│  Protocol Layer                                     │
│  HTTPUploadHandler (WebServer)                      │
├─────────────────────────────────────────────────────┤
│  File System Layer                                  │
│  SDCardManager, FileValidator                       │
├─────────────────────────────────────────────────────┤
│  Utility Layer                                      │
│  Logger, BufferManager                              │
└─────────────────────────────────────────────────────┘
```

## モジュール説明

### 1. M5StackWiFiUploader (メインクラス)

**ファイル**: `M5StackWiFiUploader.h`, `M5StackWiFiUploader.cpp`

**責務**:
- HTTPサーバーの初期化と管理
- クライアントリクエストの処理
- ファイルアップロードのオーケストレーション
- コールバック関数の呼び出し
- ステータス情報の提供

**主要メソッド**:
- `begin()`: サーバー初期化
- `handleClient()`: リクエスト処理
- `setMaxFileSize()`: ファイルサイズ制限設定
- `setAllowedExtensions()`: 拡張子設定
- `onUploadStart/Progress/Complete/Error()`: コールバック設定

### 2. SDCardManager (ファイルシステム管理)

**ファイル**: `SDCardManager.h`, `SDCardManager.cpp`

**責務**:
- SDカードの初期化と接続確認
- ファイルの読み書き
- ディレクトリ操作
- 容量管理
- ファイル検証

**主要メソッド**:
- `initialize()`: SDカード初期化
- `readFile()`, `writeFile()`: ファイルI/O
- `listFiles()`: ファイル一覧取得
- `getTotalSpace()`, `getFreeSpace()`: 容量取得
- `copyFile()`, `moveFile()`: ファイル操作

### 3. FileValidator (ファイル検証)

**ファイル**: `FileValidator.h`, `FileValidator.cpp`

**責務**:
- ファイル名の検証
- 拡張子チェック
- MIMEタイプ判定
- マジックナンバー検証
- ファイルサイズ検証

**主要メソッド**:
- `isValidFilename()`: ファイル名検証
- `isAllowedExtension()`: 拡張子チェック
- `getMimeType()`: MIMEタイプ取得
- `validateMagicNumber()`: ファイル形式検証
- `validateFile()`: 総合検証

### 4. Config.h (設定定義)

**ファイル**: `Config.h`

**内容**:
- ライブラリバージョン定義
- デフォルト設定値
- エラーコード定義
- マジックナンバー定義
- ログレベル定義

## ファイルアップロードの流れ

```
1. クライアント: POSTリクエスト送信
   ↓
2. M5StackWiFiUploader: _handleUploadHTTP() 呼び出し
   ↓
3. ファイル名検証 (FileValidator::isValidFilename)
   ↓
4. 拡張子検証 (FileValidator::isAllowedExtension)
   ↓
5. ファイルサイズチェック
   ↓
6. コールバック: onUploadStart() 呼び出し
   ↓
7. ファイル保存 (_saveFile())
   ├─ SDCardManager::writeFile() でSDに書き込み
   ├─ 進捗コールバック: onUploadProgress() 呼び出し
   └─ チャンク単位（4096バイト）で処理
   ↓
8. コールバック: onUploadComplete() 呼び出し
   ↓
9. JSONレスポンス送信
```

## セッション管理

ライブラリは複数のアップロードを同時に処理するため、セッション管理機構を備えています。

```cpp
struct UploadSession {
    String filename;        // ファイル名
    uint32_t filesize;      // ファイルサイズ
    uint32_t uploaded;      // アップロード済みバイト数
    unsigned long startTime;// 開始時刻
    File file;              // SDカードのファイルハンドル
    bool isActive;          // セッション有効フラグ
    uint8_t sessionId;      // セッションID
};
```

セッションはマップで管理され、複数のアップロードを追跡できます。

## エラーハンドリング

ライブラリは以下のエラーコードを定義しています：

| コード | 説明 |
|--------|------|
| 0 | 成功 |
| 1 | ファイルが大きすぎる |
| 2 | 無効な拡張子 |
| 3 | SD書き込み失敗 |
| 4 | 無効なリクエスト |
| 5 | タイムアウト |
| 6 | メモリ不足 |
| 255 | 不明なエラー |

## Webインターフェース

ライブラリは組み込みのWebUIを提供します。このUIは以下の機能を備えています：

- **ドラッグ&ドロップ**: ファイルをドラッグしてアップロード
- **複数ファイル選択**: 複数ファイルの同時アップロード
- **プログレス表示**: アップロード進捗をリアルタイム表示
- **ファイル管理**: アップロード済みファイルの表示と削除

UIはHTMLとJavaScriptで実装され、`_handleRoot()` メソッドで提供されます。

## 拡張方法

### カスタムコールバック追加

```cpp
uploader.onUploadStart([](const char* filename, uint32_t size) {
    // カスタム処理
    Serial.printf("Custom: %s started\n", filename);
});
```

### 新しい拡張子サポート

```cpp
const char* extensions[] = {"jpg", "png", "webp", "custom"};
uploader.setAllowedExtensions(extensions, 4);
```

### ログレベル設定

```cpp
uploader.setDebugLevel(4);  // 詳細ログ
```

## パフォーマンス最適化

### バッファサイズ

デフォルトのバッファサイズは4096バイトです。大容量ファイルの場合は増加させることで性能向上が期待できます。

```cpp
#define DEFAULT_BUFFER_SIZE 8192  // Config.h で変更
```

### 同時アップロード数

デフォルトは3ファイルです。メモリに余裕がある場合は増加させられます。

```cpp
#define MAX_CONCURRENT_UPLOADS 5  // Config.h で変更
```

## セキュリティ考慮事項

1. **ファイル名サニタイズ**: パストラバーサル攻撃を防ぐため、ファイル名から危険な文字を除去します。
2. **拡張子ホワイトリスト**: 許可する拡張子を明示的に指定します。
3. **ファイルサイズ制限**: 大容量ファイルによるメモリ枯渇を防ぎます。
4. **マジックナンバー検証**: ファイル拡張子とコンテンツの一致を確認します。

## トラブルシューティング

### メモリ不足エラー

大容量ファイルをアップロードする場合、メモリ不足が発生する可能性があります。

**対策**:
- バッファサイズを減らす
- 同時アップロード数を減らす
- ファイルサイズ制限を下げる

### SDカード書き込み失敗

SDカードが満杯またはエラー状態の可能性があります。

**対策**:
- SDカードの空き容量を確認
- SDカードをフォーマット
- ハードウェア接続を確認

### WiFi接続不安定

ネットワーク環境が不安定な場合、アップロード失敗が増加します。

**対策**:
- WiFiルーターの再起動
- M5Stackの再起動
- 別のネットワークでテスト

## 参考資料

- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [M5Stack Documentation](https://docs.m5stack.com/)
- [WebServer Library](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer)
