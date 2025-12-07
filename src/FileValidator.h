#ifndef FILE_VALIDATOR_H
#define FILE_VALIDATOR_H

#include <Arduino.h>
#include <vector>

/**
 * @brief ファイルの検証を行うクラス
 * 
 * ファイル名、拡張子、サイズ、MIMEタイプなどの検証機能を提供します。
 */
class FileValidator {
public:
    // ========================================================================
    // ファイル名検証
    // ========================================================================

    /**
     * @brief ファイル名が有効かチェック
     * @param filename ファイル名
     * @return 有効ならtrue
     */
    static bool isValidFilename(const char* filename);

    /**
     * @brief ファイル名に危険な文字が含まれていないかチェック
     * @param filename ファイル名
     * @return 安全ならtrue
     */
    static bool isSafeFilename(const char* filename);

    /**
     * @brief ファイル名をサニタイズ
     * @param filename ファイル名
     * @return サニタイズ済みファイル名
     */
    static String sanitizeFilename(const char* filename);

    // ========================================================================
    // 拡張子検証
    // ========================================================================

    /**
     * @brief 拡張子が許可されているかチェック
     * @param filename ファイル名
     * @param allowedExtensions 許可する拡張子配列
     * @param count 拡張子の個数
     * @return 許可されていればtrue
     */
    static bool isAllowedExtension(const char* filename, const char** allowedExtensions, uint8_t count);

    /**
     * @brief 拡張子が許可されているかチェック（可変長引数版）
     * @param filename ファイル名
     * @param extensionList 許可する拡張子のベクタ
     * @return 許可されていればtrue
     */
    static bool isAllowedExtension(const char* filename, const std::vector<String>& extensionList);

    /**
     * @brief ファイルの拡張子を取得
     * @param filename ファイル名
     * @return 拡張子（ドットなし、小文字）
     */
    static String getExtension(const char* filename);

    // ========================================================================
    // サイズ検証
    // ========================================================================

    /**
     * @brief ファイルサイズが許可範囲内かチェック
     * @param filesize ファイルサイズ
     * @param maxSize 最大サイズ
     * @param minSize 最小サイズ（デフォルト0）
     * @return 許可範囲内ならtrue
     */
    static bool isValidFileSize(uint32_t filesize, uint32_t maxSize, uint32_t minSize = 0);

    /**
     * @brief ファイルサイズを人間が読める形式に変換
     * @param size サイズ（バイト）
     * @return フォーマット済み文字列（例: "1.5 MB"）
     */
    static String formatFileSize(uint32_t size);

    // ========================================================================
    // MIMEタイプ検証
    // ========================================================================

    /**
     * @brief ファイル名からMIMEタイプを推定
     * @param filename ファイル名
     * @return MIMEタイプ
     */
    static String getMimeType(const char* filename);

    /**
     * @brief MIMEタイプが画像形式かチェック
     * @param mimeType MIMEタイプ
     * @return 画像形式ならtrue
     */
    static bool isImageMimeType(const char* mimeType);

    /**
     * @brief MIMEタイプがテキスト形式かチェック
     * @param mimeType MIMEタイプ
     * @return テキスト形式ならtrue
     */
    static bool isTextMimeType(const char* mimeType);

    /**
     * @brief MIMEタイプがバイナリ形式かチェック
     * @param mimeType MIMEタイプ
     * @return バイナリ形式ならtrue
     */
    static bool isBinaryMimeType(const char* mimeType);

    // ========================================================================
    // ファイルコンテンツ検証
    // ========================================================================

    /**
     * @brief ファイルのマジックナンバーを検証（簡易版）
     * @param data ファイルデータ
     * @param size データサイズ
     * @param expectedType 期待されるファイルタイプ（"jpg", "png", "gif"等）
     * @return 一致ならtrue
     */
    static bool validateMagicNumber(const uint8_t* data, uint32_t size, const char* expectedType);

    /**
     * @brief JPEGファイルのマジックナンバーをチェック
     * @param data ファイルデータ
     * @param size データサイズ
     * @return JPEGなら true
     */
    static bool isJPEG(const uint8_t* data, uint32_t size);

    /**
     * @brief PNGファイルのマジックナンバーをチェック
     * @param data ファイルデータ
     * @param size データサイズ
     * @return PNGなら true
     */
    static bool isPNG(const uint8_t* data, uint32_t size);

    /**
     * @brief GIFファイルのマジックナンバーをチェック
     * @param data ファイルデータ
     * @param size データサイズ
     * @return GIFなら true
     */
    static bool isGIF(const uint8_t* data, uint32_t size);

    /**
     * @brief BMPファイルのマジックナンバーをチェック
     * @param data ファイルデータ
     * @param size データサイズ
     * @return BMPなら true
     */
    static bool isBMP(const uint8_t* data, uint32_t size);

    // ========================================================================
    // 総合検証
    // ========================================================================

    /**
     * @brief ファイルの総合検証
     * @param filename ファイル名
     * @param filesize ファイルサイズ
     * @param data ファイルデータ（オプション）
     * @param dataSize データサイズ（オプション）
     * @param allowedExtensions 許可する拡張子配列
     * @param extensionCount 拡張子の個数
     * @param maxFileSize 最大ファイルサイズ
     * @return 検証成功ならtrue
     */
    static bool validateFile(
        const char* filename,
        uint32_t filesize,
        const uint8_t* data = nullptr,
        uint32_t dataSize = 0,
        const char** allowedExtensions = nullptr,
        uint8_t extensionCount = 0,
        uint32_t maxFileSize = 0
    );

    /**
     * @brief 検証エラーメッセージを取得
     * @return エラーメッセージ
     */
    static String getLastErrorMessage() { return _lastErrorMessage; }

private:
    static String _lastErrorMessage;

    /**
     * @brief エラーメッセージを設定
     */
    static void _setError(const char* message);
};

#endif // FILE_VALIDATOR_H
