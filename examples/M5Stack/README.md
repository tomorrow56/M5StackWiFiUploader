# M5Stack WiFi Uploader - サンプルコード

このディレクトリには、M5Stack WiFi Uploaderライブラリの使用例とテストコードが含まれています。

## サンプル一覧

### 基本サンプル

#### 1. HTTPUploadExample

<img src="../img/HTTPUploadExample.jpg" alt="HTTPUploadExample" width="300px">

**ファイル**: `HTTPUploadExample/HTTPUploadExample.ino`

**説明**: 基本的なHTTPアップロードの使用例です。WiFiルーターに接続し、HTTPサーバーを起動してファイルをアップロードします。

**主な機能**:
- WiFiステーション接続
- HTTPサーバー起動
- 基本的なコールバック設定
- シリアルモニタへのログ出力

**推奨**: 初めて使う方はこのサンプルから始めてください。

---

#### 2. MultiFileUploadExample

<img src="../img/MultiFileUploadExample.jpg" alt="MultiFileUploadExample" width="300px">

**ファイル**: `MultiFileUploadExample/MultiFileUploadExample.ino`

**説明**: 複数ファイルの同時アップロードに対応したサンプルです。

**主な機能**:
- 複数ファイルの同時アップロード
- 進捗表示
- ファイル一覧表示
- ファイル削除機能

---

#### 3. WebSocketUploadExample

**ファイル**: `WebSocketUploadExample/WebSocketUploadExample.ino`

**説明**: WebSocketプロトコルを使用した高速アップロードのサンプルです。

**主な機能**:
- WebSocketサーバー起動
- リアルタイム進捗通知
- バイナリフレーム処理
- 高速転送

**推奨**: 大容量ファイルや高速転送が必要な場合に使用してください。

---

#### 4. APModeExample

**ファイル**: `APModeExample/APModeExample.ino`

**説明**: M5StackをWiFiアクセスポイント（AP）として動作させるサンプルです。外部のWiFiルーターに接続せずにファイルをアップロードできます。

**主な機能**:
- WiFi APモード
- 固定IPアドレス（192.168.4.1）
- 接続クライアント数表示
- M5Stack画面への状態表示

**使い方**:
1. M5Stackを起動
2. PCやスマートフォンのWiFi設定で "M5Stack-AP" に接続
3. パスワード: "12345678"
4. ブラウザで http://192.168.4.1 にアクセス

**推奨**: WiFiルーターがない環境や、M5Stackを独立したアクセスポイントとして使用したい場合に最適です。

---

#### 5. FullFeaturedDemo

**ファイル**: `FullFeaturedDemo/FullFeaturedDemo.ino`

**説明**: ライブラリのすべての機能を統合した完全なデモアプリケーションです。

**主な機能**:
- M5Stack画面への完全な状態表示
- ボタン操作（ファイル一覧、ステータス、クリア）
- リアルタイムプログレスバー
- エラー表示
- 統計情報表示

**推奨**: ライブラリの全機能を確認したい場合や、実際のアプリケーション開発の参考にしてください。

---

#### 6. FileDownloadExample

**ファイル**: `FileDownloadExample/FileDownloadExample.ino`

**説明**: SDカード内のファイルをWeb UI経由でダウンロードする機能のサンプルです。

**主な機能**:
- ファイル一覧表示
- ファイルダウンロード
- ファイル削除

---

### テストコード

テストコードは `tests/` フォルダに格納されています。ライブラリの各コンポーネントの動作確認に使用できます。

#### 1. test_error_handler

**ファイル**: `tests/test_error_handler/test_error_handler.ino`

**説明**: ErrorHandlerクラスの機能テストです。

**テスト内容**:
- エラーログ記録
- エラー履歴取得
- 回復可能エラーの判定
- エラー統計
- エラーコールバック

---

#### 2. test_progress_tracker

**ファイル**: `tests/test_progress_tracker/test_progress_tracker.ino`

**説明**: ProgressTrackerクラスの機能テストです。

**テスト内容**:
- 進捗追跡
- 転送速度計算
- 残り時間計算
- 複数ファイルの進捗管理
- フォーマット関数
- 一時停止/再開

---

#### 3. test_integration

**ファイル**: `tests/test_integration/test_integration.ino`

**説明**: M5StackWiFiUploaderライブラリの統合テストです。

**テスト内容**:
- 初期化テスト
- 設定テスト
- コールバックテスト
- SDカード操作テスト
- ステータス取得テスト

---

## 使用方法

1. Arduino IDEで各サンプルのフォルダを開きます。
2. WiFi設定（SSID、パスワード）を自分の環境に合わせて変更します。
3. M5Stackにアップロードします。
4. シリアルモニタでIPアドレスを確認します。
5. ブラウザでそのIPアドレスにアクセスします。

## 許可拡張子（転送可能ファイル）の変更方法

このライブラリは、アップロード時にファイル拡張子を検証します。
許可拡張子はスケッチ側から `setAllowedExtensions()` で上書きできます。

### 変更手順（推奨）

1. 許可したい拡張子（ドットなし）を配列で用意します。
2. `begin()` の後に `setAllowedExtensions()` を呼び出します。

```cpp
const char* allowedExtensions[] = {
  "bin",
  "txt",
  "json",
};

uploader.setAllowedExtensions(
  allowedExtensions,
  sizeof(allowedExtensions) / sizeof(allowedExtensions[0])
);
```

### 注意点

- **拡張子はドットなし**で指定します（例: `"bin"`）。
- **小文字での指定を推奨**します（内部で小文字化して比較されます）。
- 許可リストが空（`count == 0`）の場合、拡張子チェックが常に失敗し、アップロードできません。
- 現状、**ダウンロード機能は拡張子で制限していません**（パストラバーサル等のチェックのみ行います）。

### デフォルトの許可拡張子

何も設定しない場合、デフォルトで以下が許可されています。

- `jpg`, `jpeg`, `png`, `gif`, `bmp`
- `bin`, `dat`, `txt`, `csv`, `json`
- `zip`, `rar`, `7z`, `tar`, `gz`

## トラブルシューティング

問題が発生した場合は、[トラブルシューティングガイド](../docs/TROUBLESHOOTING.md)を参照してください。
