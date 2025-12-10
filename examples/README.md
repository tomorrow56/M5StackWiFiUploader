# M5Stack WiFi Uploader - サンプルコード

このディレクトリには、M5Stack WiFi Uploaderライブラリの使用例が含まれています。

## サンプル一覧

### 1. HTTPUploadExample

**ファイル**: `HTTPUploadExample/HTTPUploadExample.ino`

**説明**: 基本的なHTTPアップロードの使用例です。WiFiルーターに接続し、HTTPサーバーを起動してファイルをアップロードします。

**主な機能**:
- WiFiステーション接続
- HTTPサーバー起動
- 基本的なコールバック設定
- シリアルモニタへのログ出力

**推奨**: 初めて使う方はこのサンプルから始めてください。

---

### 2. MultiFileUploadExample

**ファイル**: `MultiFileUploadExample/MultiFileUploadExample.ino`

**説明**: 複数ファイルの同時アップロードに対応したサンプルです。

**主な機能**:
- 複数ファイルの同時アップロード
- 進捗表示
- ファイル一覧表示
- ファイル削除機能

---

### 3. WebSocketUploadExample

**ファイル**: `WebSocketUploadExample/WebSocketUploadExample.ino`

**説明**: WebSocketプロトコルを使用した高速アップロードのサンプルです。

**主な機能**:
- WebSocketサーバー起動
- リアルタイム進捗通知
- バイナリフレーム処理
- 高速転送

**推奨**: 大容量ファイルや高速転送が必要な場合に使用してください。

---

### 4. APModeExample

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

### 5. FullFeaturedDemo

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

## 使用方法

1. Arduino IDEで各サンプルのフォルダを開きます。
2. WiFi設定（SSID、パスワード）を自分の環境に合わせて変更します。
3. M5Stackにアップロードします。
4. シリアルモニタでIPアドレスを確認します。
5. ブラウザでそのIPアドレスにアクセスします。

## トラブルシューティング

問題が発生した場合は、[トラブルシューティングガイド](../docs/TROUBLESHOOTING.md)を参照してください。
