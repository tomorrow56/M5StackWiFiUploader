#ifndef SDCARD_MANAGER_H
#define SDCARD_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <vector>

/**
 * @brief ファイル情報構造体
 */
struct FileInfo {
    String name;          // ファイル名
    uint32_t size;        // ファイルサイズ（バイト）
    uint32_t modified;    // 最終更新時刻（UNIXタイムスタンプ）
    bool isDirectory;     // ディレクトリフラグ
    String extension;     // 拡張子
};

/**
 * @brief SDカード操作を管理するクラス
 * 
 * ファイルの読み書き、ディレクトリ管理、容量チェック等の
 * SDカード操作を統一的に提供します。
 */
class SDCardManager {
public:
    /**
     * @brief SDカードを初期化
     * @param csPin チップセレクトピン（デフォルト4）
     * @return 初期化成功時true
     */
    static bool initialize(uint8_t csPin = 4);

    /**
     * @brief SDカードが接続されているかチェック
     * @return 接続されていればtrue
     */
    static bool isConnected();

    // ========================================================================
    // ファイル操作
    // ========================================================================

    /**
     * @brief ファイルが存在するかチェック
     * @param filepath ファイルパス
     * @return 存在すればtrue
     */
    static bool fileExists(const char* filepath);

    /**
     * @brief ファイルを削除
     * @param filepath ファイルパス
     * @return 削除成功時true
     */
    static bool deleteFile(const char* filepath);

    /**
     * @brief ファイルサイズを取得
     * @param filepath ファイルパス
     * @return ファイルサイズ（バイト）、存在しない場合は0
     */
    static uint32_t getFileSize(const char* filepath);

    /**
     * @brief ファイルを読み込む
     * @param filepath ファイルパス
     * @param buffer 読み込みバッファ
     * @param maxSize バッファサイズ
     * @return 読み込んだバイト数
     */
    static uint32_t readFile(const char* filepath, uint8_t* buffer, uint32_t maxSize);

    /**
     * @brief ファイルに書き込む
     * @param filepath ファイルパス
     * @param data 書き込みデータ
     * @param size データサイズ
     * @param append true=追記、false=上書き
     * @return 書き込み成功時true
     */
    static bool writeFile(const char* filepath, const uint8_t* data, uint32_t size, bool append = false);

    /**
     * @brief テキストをファイルに書き込む
     * @param filepath ファイルパス
     * @param text テキスト
     * @param append true=追記、false=上書き
     * @return 書き込み成功時true
     */
    static bool writeText(const char* filepath, const char* text, bool append = false);

    /**
     * @brief ファイルを読み込んでテキストを取得
     * @param filepath ファイルパス
     * @return ファイル内容（文字列）
     */
    static String readText(const char* filepath);

    // ========================================================================
    // ディレクトリ操作
    // ========================================================================

    /**
     * @brief ディレクトリが存在するかチェック
     * @param dirpath ディレクトリパス
     * @return 存在すればtrue
     */
    static bool dirExists(const char* dirpath);

    /**
     * @brief ディレクトリを作成
     * @param dirpath ディレクトリパス
     * @return 作成成功時true
     */
    static bool createDir(const char* dirpath);

    /**
     * @brief ディレクトリを削除
     * @param dirpath ディレクトリパス
     * @return 削除成功時true
     */
    static bool deleteDir(const char* dirpath);

    /**
     * @brief ディレクトリ内のファイル一覧を取得
     * @param dirpath ディレクトリパス
     * @param includeDir true=ディレクトリも含める
     * @return ファイル名のベクタ
     */
    static std::vector<String> listFiles(const char* dirpath, bool includeDir = false);

    /**
     * @brief ディレクトリ内のファイル詳細情報一覧を取得
     * @param dirpath ディレクトリパス
     * @param includeDir true=ディレクトリも含める
     * @return ファイル情報のベクタ
     */
    static std::vector<FileInfo> listFilesWithInfo(const char* dirpath, bool includeDir = false);

    /**
     * @brief ファイルの詳細情報を取得
     * @param filepath ファイルパス
     * @return ファイル情報
     */
    static FileInfo getFileInfo(const char* filepath);

    /**
     * @brief ディレクトリ内のファイル数を取得
     * @param dirpath ディレクトリパス
     * @return ファイル数
     */
    static uint32_t getFileCount(const char* dirpath);

    // ========================================================================
    // 容量管理
    // ========================================================================

    /**
     * @brief SDカードの総容量を取得
     * @return 総容量（バイト）
     */
    static uint32_t getTotalSpace();

    /**
     * @brief SDカードの使用容量を取得
     * @return 使用容量（バイト）
     */
    static uint32_t getUsedSpace();

    /**
     * @brief SDカードの空き容量を取得
     * @return 空き容量（バイト）
     */
    static uint32_t getFreeSpace();

    /**
     * @brief SDカードの使用率を取得
     * @return 使用率（0-100）
     */
    static uint8_t getUsagePercent();

    // ========================================================================
    // ファイル検証
    // ========================================================================

    /**
     * @brief ファイル名が有効かチェック
     * @param filename ファイル名
     * @return 有効ならtrue
     */
    static bool isValidFilename(const char* filename);

    /**
     * @brief ファイル名をサニタイズ（危険な文字を除去）
     * @param filename ファイル名
     * @return サニタイズ済みファイル名
     */
    static String sanitizeFilename(const char* filename);

    /**
     * @brief ファイルの拡張子を取得
     * @param filename ファイル名
     * @return 拡張子（ドットなし）
     */
    static String getFileExtension(const char* filename);

    /**
     * @brief ファイルのベース名を取得（拡張子なし）
     * @param filename ファイル名
     * @return ベース名
     */
    static String getBaseName(const char* filename);

    // ========================================================================
    // ユーティリティ
    // ========================================================================

    /**
     * @brief パスが絶対パスかチェック
     * @param path パス
     * @return 絶対パスならtrue
     */
    static bool isAbsolutePath(const char* path);

    /**
     * @brief パスを正規化（重複スラッシュを除去等）
     * @param path パス
     * @return 正規化済みパス
     */
    static String normalizePath(const char* path);

    /**
     * @brief ファイルをコピー
     * @param srcPath ソースファイルパス
     * @param dstPath 宛先ファイルパス
     * @return コピー成功時true
     */
    static bool copyFile(const char* srcPath, const char* dstPath);

    /**
     * @brief ファイルを移動
     * @param srcPath ソースファイルパス
     * @param dstPath 宛先ファイルパス
     * @return 移動成功時true
     */
    static bool moveFile(const char* srcPath, const char* dstPath);

    /**
     * @brief ファイルの最終更新時刻を取得
     * @param filepath ファイルパス
     * @return UNIXタイムスタンプ
     */
    static uint32_t getLastModified(const char* filepath);

private:
    static bool _initialized;

    /**
     * @brief パスを正規化（内部用）
     */
    static String _normalizePath(const String& path);
};

#endif // SDCARD_MANAGER_H
