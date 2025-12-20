#include "M5StackWiFiUploader.h"
#include <WiFi.h>
#include <cstdarg>

// ============================================================================
// コンストラクタ・デストラクタ
// ============================================================================

M5StackWiFiUploader::M5StackWiFiUploader(uint16_t port)
    : _webServer(nullptr),
      _wsHandler(nullptr),
      _port(port),
      _isRunning(false),
      _uploadPath("/uploads"),
      _maxFileSize(50 * 1024 * 1024),  // 50MB
      _debugLevel(1),
      _webSocketEnabled(false),
      _overwriteProtection(false),
      _totalUploaded(0),
      _nextSessionId(0) {
    // デフォルト許可拡張子を設定
    _allowedExtensions = {
        "jpg", "jpeg", "png", "gif", "bmp",
        "bin", "dat", "txt", "csv", "json",
        "zip", "rar", "7z", "tar", "gz"
    };
}

M5StackWiFiUploader::~M5StackWiFiUploader() {
    end();
}

// ============================================================================
// 初期化・制御
// ============================================================================

bool M5StackWiFiUploader::begin(uint16_t port, const char* uploadPath) {
    if (_webSocketEnabled) {
        _wsHandler = new WebSocketHandler(DEFAULT_WS_PORT);
        _wsHandler->begin();
        _wsHandler->onData([this](uint8_t clientId, const uint8_t* data, size_t length) {
            // WebSocketデータ受信処理
        });
    }
    _port = port;
    _uploadPath = uploadPath;

    // WebServerインスタンスを作成
    if (_webServer != nullptr) {
        delete _webServer;
    }
    _webServer = new WebServer(_port);

    if (_webServer == nullptr) {
        _log(1, "Failed to create WebServer instance");
        return false;
    }

    // ルートハンドラーを登録
    _webServer->on("/", HTTP_GET, [this]() { _handleRoot(); });
    _webServer->on("/api/upload", HTTP_POST, 
        [this]() { _handleUploadHTTP(); },
        [this]() { _handleUploadData(); }
    );
    _webServer->on("/api/files", HTTP_GET, [this]() { _handleListFiles(); });
    _webServer->on("/api/files/list", HTTP_GET, [this]() { _handleFileListDetailed(); });
    _webServer->on("/api/download", HTTP_GET, [this]() { _handleFileDownload(); });
    _webServer->on("/api/delete", HTTP_POST, [this]() { _handleDeleteFile(); });
    _webServer->on("/api/status", HTTP_GET, [this]() { _handleStatus(); });
    _webServer->on("/api/debug", HTTP_POST, [this]() { _handleDebugLog(); });

    // アップロードディレクトリを確認・作成
    if (!_ensureUploadDirectory()) {
        _log(1, "Failed to create upload directory: %s", _uploadPath.c_str());
        return false;
    }

    // サーバーを開始
    _webServer->begin();
    _isRunning = true;

    _log(3, "M5StackWiFiUploader started on port %d", _port);
    _log(3, "Upload path: %s", _uploadPath.c_str());
    _log(3, "Server URL: http://%s:%d", WiFi.localIP().toString().c_str(), _port);

    return true;
}

void M5StackWiFiUploader::handleClient() {
    if (!_isRunning) return;
    if (_webServer) _webServer->handleClient();
    if (_wsHandler) _wsHandler->handleClient();
}

void M5StackWiFiUploader::end() {
    if (_wsHandler) {
        _wsHandler->end();
        delete _wsHandler;
        _wsHandler = nullptr;
    }
    if (_webServer != nullptr) {
        _closeAllSessions();
        _webServer->stop();
        delete _webServer;
        _webServer = nullptr;
        _isRunning = false;
        _log(3, "M5StackWiFiUploader stopped");
    }
}

// ============================================================================
// 設定
// ============================================================================

void M5StackWiFiUploader::setMaxFileSize(uint32_t maxSize) {
    _maxFileSize = maxSize;
    _log(3, "Max file size set to %d bytes", maxSize);
}

void M5StackWiFiUploader::setAllowedExtensions(const char** extensions, uint8_t count) {
    _allowedExtensions.clear();
    for (uint8_t i = 0; i < count; i++) {
        _allowedExtensions.push_back(extensions[i]);
    }
    _log(3, "Allowed extensions updated: %d types", count);
}

void M5StackWiFiUploader::setUploadPath(const char* path) {
    _uploadPath = path;
    _ensureUploadDirectory();
    _log(3, "Upload path set to: %s", path);
}

void M5StackWiFiUploader::setDebugLevel(uint8_t level) {
    _debugLevel = level;
}

void M5StackWiFiUploader::enableWebSocket(bool enable) {
    _webSocketEnabled = enable;
    _log(3, "WebSocket %s", enable ? "enabled" : "disabled");
}

void M5StackWiFiUploader::setOverwriteProtection(bool enable) {
    _overwriteProtection = enable;
    _log(3, "Overwrite protection %s", enable ? "enabled" : "disabled");
}

// ============================================================================
// ステータス取得
// ============================================================================

uint8_t M5StackWiFiUploader::getActiveUploads() const {
    uint8_t count = 0;
    for (const auto& session : _activeSessions) {
        if (session.second.isActive) {
            count++;
        }
    }
    return count;
}

String M5StackWiFiUploader::getServerIP() const {
    return WiFi.localIP().toString();
}

String M5StackWiFiUploader::getServerURL() const {
    return "http://" + WiFi.localIP().toString() + ":" + String(_port);
}

uint32_t M5StackWiFiUploader::getSDFreeSpace() const {
    if (!SD.begin()) return 0;
    uint32_t total = SD.totalBytes();
    uint32_t used = SD.usedBytes();
    return (total > used) ? (total - used) : 0;
}

uint32_t M5StackWiFiUploader::getSDTotalSpace() const {
    if (!SD.begin()) return 0;
    return SD.totalBytes();
}

bool M5StackWiFiUploader::fileExists(const char* filename) const {
    String fullPath = _uploadPath + "/" + filename;
    return SD.exists(fullPath.c_str());
}

bool M5StackWiFiUploader::deleteFile(const char* filename) {
    String fullPath = _uploadPath + "/" + filename;
    if (SD.remove(fullPath.c_str())) {
        _log(3, "File deleted: %s", filename);
        return true;
    }
    _log(2, "Failed to delete file: %s", filename);
    return false;
}

std::vector<String> M5StackWiFiUploader::listFiles(const char* path) {
    std::vector<String> files;
    const char* searchPath = path ? path : _uploadPath.c_str();
    
    File dir = SD.open(searchPath);
    if (!dir || !dir.isDirectory()) {
        _log(2, "Failed to open directory: %s", searchPath);
        return files;
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            files.push_back(file.name());
        }
        file = dir.openNextFile();
    }
    dir.close();

    _log(3, "Listed %d files in %s", files.size(), searchPath);
    return files;
}

// ============================================================================
// プライベートメソッド - HTTPハンドラー
// ============================================================================

void M5StackWiFiUploader::_handleRoot() {
    String html = R"RAWHTML(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>M5Stack WiFi Uploader</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 1000px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; }
        h2 { color: #555; margin-top: 30px; }
        .upload-area { border: 2px dashed #ccc; padding: 20px; text-align: center; margin: 20px 0; border-radius: 4px; cursor: pointer; }
        .upload-area:hover { background: #f9f9f9; }
        .upload-area.dragover { background: #e3f2fd; border-color: #2196F3; }
        input[type="file"] { display: none; }
        button { background: #2196F3; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; font-size: 14px; margin: 2px; }
        button:hover { background: #1976D2; }
        button.danger { background: #f44336; }
        button.danger:hover { background: #d32f2f; }
        button.success { background: #4CAF50; }
        button.success:hover { background: #388E3C; }
        .file-list { margin-top: 20px; }
        .file-table { width: 100%; border-collapse: collapse; }
        .file-table th, .file-table td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }
        .file-table th { background: #f5f5f5; font-weight: bold; }
        .file-table tr:hover { background: #f9f9f9; }
        .file-name { font-weight: 500; color: #2196F3; cursor: pointer; }
        .file-name:hover { text-decoration: underline; }
        .file-size { color: #666; }
        .file-date { color: #999; font-size: 0.9em; }
        .progress { width: 100%; height: 20px; background: #e0e0e0; border-radius: 4px; margin: 10px 0; overflow: hidden; }
        .progress-bar { height: 100%; background: #4CAF50; width: 0%; transition: width 0.3s; }
        .status { padding: 10px; margin: 10px 0; border-radius: 4px; }
        .status.info { background: #e3f2fd; color: #1976D2; }
        .status.success { background: #e8f5e9; color: #388E3C; }
        .status.error { background: #ffebee; color: #C62828; }
        .actions { display: flex; gap: 5px; }
        .no-files { text-align: center; padding: 20px; color: #999; }
        .loading { text-align: center; padding: 20px; }
        
        /* モバイル対応 */
        @media (max-width: 768px) {
            body { margin: 10px; }
            .container { padding: 15px; }
            .file-table { 
                display: block; 
                overflow-x: auto; 
                -webkit-overflow-scrolling: touch; 
                font-size: 12px; 
            }
            .file-table th, .file-table td { 
                padding: 8px 4px; 
                font-size: 12px; 
            }
            .file-name { 
                word-break: break-all; 
                max-width: 150px; 
            }
            .actions { 
                flex-direction: column; 
                gap: 2px; 
            }
            button { 
                padding: 6px 8px; 
                font-size: 11px; 
                margin: 1px 0; 
            }
            .upload-area { padding: 15px; }
            h1 { font-size: 24px; }
            h2 { font-size: 20px; }
        }
        
        @media (max-width: 480px) {
            .file-table th, .file-table td { 
                padding: 6px 2px; 
                font-size: 11px; 
            }
            .file-name { 
                max-width: 120px; 
            }
            button { 
                padding: 4px 6px; 
                font-size: 10px; 
            }
            .container { padding: 10px; }
            .upload-area { padding: 10px; }
        }
        
            </style>
</head>
<body>
    <div class="container">
        <h1>M5Stack WiFi File Uploader</h1>
        <p>ファイルをドラッグ&ドロップするか、下のボタンをクリックしてアップロードしてください。</p>
        
        <div class="upload-area" id="uploadArea">
            <p>ここにファイルをドラッグ&ドロップ</p>
            <input type="file" id="fileInput" multiple>
            <button onclick="document.getElementById('fileInput').click()">ファイルを選択</button>
        </div>

        <div id="status"></div>
        <div id="uploadProgress"></div>

        <div class="file-list">
            <h2>SDカード内のファイル</h2>
            <button onclick="loadFilesList()" class="success">更新</button>
            <div id="filesList" class="loading">読み込み中...</div>
        </div>
    </div>

    <script>
        const uploadArea = document.getElementById('uploadArea');
        const fileInput = document.getElementById('fileInput');
        const statusDiv = document.getElementById('status');
        const progressDiv = document.getElementById('uploadProgress');
        const filesListDiv = document.getElementById('filesList');

        uploadArea.addEventListener('dragover', (e) => {
            e.preventDefault();
            uploadArea.classList.add('dragover');
        });

        uploadArea.addEventListener('dragleave', () => {
            uploadArea.classList.remove('dragover');
        });

        uploadArea.addEventListener('drop', (e) => {
            e.preventDefault();
            uploadArea.classList.remove('dragover');
            handleFiles(e.dataTransfer.files);
        });

        fileInput.addEventListener('change', (e) => {
            handleFiles(e.target.files);
        });

        function handleFiles(files) {
            for (let file of files) {
                uploadFile(file);
            }
        }

        function uploadFile(file) {
            const formData = new FormData();
            formData.append('file', file);

            const progressId = 'progress-' + Date.now();
            const progressHTML = `
                <div id="${progressId}">
                    <p>${file.name} (${formatFileSize(file.size)})</p>
                    <div class="progress">
                        <div class="progress-bar" id="${progressId}-bar"></div>
                    </div>
                </div>
            `;
            progressDiv.innerHTML += progressHTML;

            const xhr = new XMLHttpRequest();
            
            xhr.upload.addEventListener('progress', (e) => {
                if (e.lengthComputable) {
                    const percentComplete = (e.loaded / e.total) * 100;
                    document.getElementById(progressId + '-bar').style.width = percentComplete + '%';
                }
            });

            xhr.addEventListener('load', () => {
                if (xhr.status === 200) {
                    const response = JSON.parse(xhr.responseText);
                    showStatus('success', `${file.name} アップロード完了`);
                    document.getElementById(progressId).remove();
                    loadFilesList();
                } else {
                    showStatus('error', `${file.name} アップロード失敗: ${xhr.status}`);
                }
            });

            xhr.addEventListener('error', () => {
                showStatus('error', `${file.name} アップロードエラー`);
            });

            xhr.open('POST', '/api/upload');
            xhr.send(formData);
        }

        function loadFilesList() {
            filesListDiv.innerHTML = '<div class="loading">読み込み中...</div>';
            
            fetch('/api/files/list')
                .then(response => response.json())
                .then(data => {
                    filesListDiv.innerHTML = '';
                    if (data.files && data.files.length > 0) {
                        const table = document.createElement('table');
                        table.className = 'file-table';
                        table.innerHTML = `
                            <thead>
                                <tr>
                                    <th>ファイル名</th>
                                    <th>サイズ</th>
                                    <th>更新日時</th>
                                    <th>操作</th>
                                </tr>
                            </thead>
                            <tbody id="filesTableBody"></tbody>
                        `;
                        filesListDiv.appendChild(table);
                        
                        const tbody = document.getElementById('filesTableBody');
                        data.files.forEach(file => {
                            const row = document.createElement('tr');
                            row.innerHTML = `
                                <td><span class="file-name" onclick="downloadFile('${file.name}')">${file.name}</span></td>
                                <td class="file-size">${formatFileSize(file.size)}</td>
                                <td class="file-date">${formatDate(file.modified)}</td>
                                <td class="actions">
                                    <button onclick="downloadFile('${file.name}')" class="success">ダウンロード</button>
                                    <button onclick="deleteFile('${file.name}')" class="danger">削除</button>
                                </td>
                            `;
                            tbody.appendChild(row);
                        });
                    } else {
                        filesListDiv.innerHTML = '<div class="no-files">ファイルがありません</div>';
                    }
                })
                .catch(error => {
                    filesListDiv.innerHTML = '<div class="no-files">ファイル一覧の取得に失敗しました</div>';
                    showStatus('error', 'ファイル一覧の取得に失敗しました');
                });
        }

        function downloadFile(filename) {
            window.location.href = `/api/download?filename=${encodeURIComponent(filename)}`;
            showStatus('info', `${filename} をダウンロード中...`);
        }

        function deleteFile(filename) {
            if (confirm(`${filename} を削除しますか？`)) {
                fetch('/api/delete', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ filename: filename })
                })
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        showStatus('success', `${filename} を削除しました`);
                        loadFilesList();
                    } else {
                        showStatus('error', `削除に失敗しました: ${data.message}`);
                    }
                });
            }
        }

        function formatFileSize(bytes) {
            if (bytes === 0) return '0 B';
            const k = 1024;
            const sizes = ['B', 'KB', 'MB', 'GB'];
            const i = Math.floor(Math.log(bytes) / Math.log(k));
            return (bytes / Math.pow(k, i)).toFixed(2) + ' ' + sizes[i];
        }

        function formatDate(timestamp) {
            if (!timestamp || timestamp === 0) return '-';
            const date = new Date(timestamp * 1000);
            const year = date.getFullYear();
            const month = String(date.getMonth() + 1).padStart(2, '0');
            const day = String(date.getDate()).padStart(2, '0');
            const hours = String(date.getHours()).padStart(2, '0');
            const minutes = String(date.getMinutes()).padStart(2, '0');
            return `${year}-${month}-${day} ${hours}:${minutes}`;
        }

        function showStatus(type, message) {
            const status = document.createElement('div');
            status.className = 'status ' + type;
            status.textContent = message;
            statusDiv.insertBefore(status, statusDiv.firstChild);
            setTimeout(() => status.remove(), 5000);
        }

        loadFilesList();
    </script>
</body>
</html>
    )RAWHTML";
    
    _webServer->send(200, "text/html; charset=utf-8", html);
}

void M5StackWiFiUploader::_handleUploadHTTP() {
    // マルチパートアップロードが完了した後に呼ばれる
    Serial.println("[DEBUG] _handleUploadHTTP called");
    _sendJSONResponse(true, "File uploaded successfully");
}

void M5StackWiFiUploader::_handleUploadData() {
    Serial.println("[DEBUG] _handleUploadData called");
    HTTPUpload& upload = _webServer->upload();
    static File uploadFile;
    static String currentFilename;
    static uint32_t currentFilesize;
    
    Serial.printf("[DEBUG] upload.status = %d\n", upload.status);
    
    if (upload.status == UPLOAD_FILE_START) {
        currentFilename = upload.filename;
        currentFilesize = 0;
        
        _log(3, "Upload Start: %s", currentFilename.c_str());
        
        // ファイル名を検証
        if (!_isValidFilename(currentFilename.c_str())) {
            _log(2, "Invalid filename: %s", currentFilename.c_str());
            return;
        }
        
        currentFilename = _sanitizeFilename(currentFilename.c_str());
        
        // 拡張子を検証
        if (!_isValidExtension(currentFilename.c_str())) {
            _log(2, "Invalid file extension: %s", currentFilename.c_str());
            return;
        }
        
        // ファイルパスを作成
        String fullPath = _uploadPath + "/" + currentFilename;
        
        // 上書き保護をチェック
        if (_overwriteProtection && SD.exists(fullPath.c_str())) {
            _log(2, "File already exists (overwrite protection): %s", currentFilename.c_str());
            return;
        }
        
        // ファイルを開く
        uploadFile = SD.open(fullPath.c_str(), FILE_WRITE);
        if (!uploadFile) {
            _log(1, "Failed to open file for writing: %s", fullPath.c_str());
            return;
        }
        
        // コールバック: アップロード開始
        if (_onUploadStart) {
            _onUploadStart(currentFilename.c_str(), upload.totalSize);
        }
        
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
            // ファイルサイズをチェック
            currentFilesize += upload.currentSize;
            if (currentFilesize > _maxFileSize) {
                _log(2, "File too large: %d bytes (max: %d)", currentFilesize, _maxFileSize);
                uploadFile.close();
                String fullPath = _uploadPath + "/" + currentFilename;
                SD.remove(fullPath.c_str());
                return;
            }
            
            // データを書き込み
            size_t written = uploadFile.write(upload.buf, upload.currentSize);
            if (written != upload.currentSize) {
                _log(1, "Write error: expected %d, wrote %d", upload.currentSize, written);
            }
            
            // コールバック: 進捗
            if (_onUploadProgress) {
                _onUploadProgress(currentFilename.c_str(), currentFilesize, upload.totalSize);
            }
        }
        
    } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            uploadFile.close();
            _totalUploaded += currentFilesize;
            _log(3, "Upload Complete: %s (%d bytes)", currentFilename.c_str(), currentFilesize);
            
            // コールバック: アップロード完了
            if (_onUploadComplete) {
                _onUploadComplete(currentFilename.c_str(), currentFilesize, true);
            }
        }
        
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (uploadFile) {
            uploadFile.close();
            String fullPath = _uploadPath + "/" + currentFilename;
            SD.remove(fullPath.c_str());
            _log(2, "Upload Aborted: %s", currentFilename.c_str());
            
            // コールバック: エラー
            if (_onUploadError) {
                _onUploadError(currentFilename.c_str(), ERR_UNKNOWN, "Upload aborted");
            }
        }
    }
}

void M5StackWiFiUploader::_handleListFiles() {
    std::vector<String> files = listFiles();
    
    String json = "{\"success\": true, \"files\": [";
    for (size_t i = 0; i < files.size(); i++) {
        json += "\"" + files[i] + "\"";
        if (i < files.size() - 1) {
            json += ", ";
        }
    }
    json += "]}";

    _webServer->send(200, "application/json", json);
}

void M5StackWiFiUploader::_handleDeleteFile() {
    if (_webServer->method() != HTTP_POST) {
        _sendJSONResponse(false, "Method not allowed");
        return;
    }

    String body = _webServer->arg("plain");
    // 簡易的なJSON解析（ArduinoJsonを使わない）
    int filenamePos = body.indexOf("\"filename\"");
    if (filenamePos == -1) {
        _sendJSONResponse(false, "No filename provided");
        return;
    }

    int startPos = body.indexOf("\"", filenamePos + 11) + 1;
    int endPos = body.indexOf("\"", startPos);
    String filename = body.substring(startPos, endPos);

    if (deleteFile(filename.c_str())) {
        _sendJSONResponse(true, "File deleted successfully", filename.c_str());
    } else {
        _sendJSONResponse(false, "Failed to delete file");
    }
}

void M5StackWiFiUploader::_handleStatus() {
    String json = "{";
    json += "\"running\": " + String(_isRunning ? "true" : "false") + ", ";
    json += "\"activeUploads\": " + String(getActiveUploads()) + ", ";
    json += "\"totalUploaded\": " + String(_totalUploaded) + ", ";
    json += "\"sdFreeSpace\": " + String(getSDFreeSpace()) + ", ";
    json += "\"sdTotalSpace\": " + String(getSDTotalSpace()) + ", ";
    json += "\"serverIP\": \"" + getServerIP() + "\", ";
    json += "\"serverPort\": " + String(_port);
    json += "}";

    _webServer->send(200, "application/json", json);
}

void M5StackWiFiUploader::_handleFileListDetailed() {
    _log(3, "Handling detailed file list request");
    
    std::vector<FileInfo> files = SDCardManager::listFilesWithInfo(_uploadPath.c_str(), false);
    
    String json = "{";
    json += "\"files\": [";
    
    for (size_t i = 0; i < files.size(); i++) {
        if (i > 0) json += ", ";
        json += "{";
        json += "\"name\": \"" + files[i].name + "\", ";
        json += "\"size\": " + String(files[i].size) + ", ";
        json += "\"modified\": " + String(files[i].modified) + ", ";
        json += "\"isDirectory\": " + String(files[i].isDirectory ? "true" : "false") + ", ";
        json += "\"extension\": \"" + files[i].extension + "\"";
        json += "}";
    }
    
    json += "], ";
    json += "\"total\": " + String(files.size());
    json += "}";
    
    _webServer->send(200, "application/json", json);
}

void M5StackWiFiUploader::_handleFileDownload() {
    if (!_webServer->hasArg("filename")) {
        _sendJSONResponse(false, "Missing filename parameter", nullptr);
        return;
    }
    
    String filename = _webServer->arg("filename");
    String fullPath = _uploadPath + "/" + filename;
    
    _log(3, "Download request for: %s", filename.c_str());
    
    // パストラバーサル攻撃を防止
    if (filename.indexOf("..") >= 0 || filename.indexOf("/") >= 0 || filename.indexOf("\\") >= 0) {
        _log(1, "Invalid filename (path traversal attempt): %s", filename.c_str());
        _sendJSONResponse(false, "Invalid filename", filename.c_str());
        return;
    }
    
    if (!SDCardManager::fileExists(fullPath.c_str())) {
        _log(1, "File not found: %s", fullPath.c_str());
        _sendJSONResponse(false, "File not found", filename.c_str());
        return;
    }
    
    File file = SD.open(fullPath.c_str(), FILE_READ);
    if (!file) {
        _log(1, "Failed to open file: %s", fullPath.c_str());
        _sendJSONResponse(false, "Failed to open file", filename.c_str());
        return;
    }
    
    String contentType = _getContentType(filename.c_str());
    size_t fileSize = file.size();
    
    _webServer->sendHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
    _webServer->streamFile(file, contentType);
    
    file.close();
    _log(3, "File download completed: %s (%d bytes)", filename.c_str(), fileSize);
}

// ============================================================================
// プライベートメソッド - ファイル操作
// ============================================================================

bool M5StackWiFiUploader::_saveFile(const char* filename, uint8_t* data, uint32_t size) {
    String fullPath = _uploadPath + "/" + filename;

    File file = SD.open(fullPath.c_str(), FILE_WRITE);
    if (!file) {
        _log(1, "Failed to open file for writing: %s", fullPath.c_str());
        return false;
    }

    uint32_t written = 0;
    uint32_t chunkSize = 4096;

    while (written < size) {
        uint32_t toWrite = (size - written) < chunkSize ? (size - written) : chunkSize;
        size_t result = file.write(data + written, toWrite);

        if (result != toWrite) {
            _log(1, "Failed to write data to file: %s", filename);
            file.close();
            return false;
        }

        written += toWrite;

        // コールバック: 進捗通知
        if (_onUploadProgress) {
            _onUploadProgress(filename, written, size);
        }
    }

    file.close();
    _log(3, "File saved successfully: %s (%d bytes)", fullPath.c_str(), size);
    return true;
}

bool M5StackWiFiUploader::_isValidExtension(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext || ext == filename) {
        return false;
    }

    ext++;  // ドットをスキップ
    String extStr = String(ext);
    extStr.toLowerCase();

    for (const auto& allowed : _allowedExtensions) {
        if (extStr == allowed) {
            return true;
        }
    }

    return false;
}

bool M5StackWiFiUploader::_isValidFilename(const char* filename) {
    if (!filename || strlen(filename) == 0) {
        return false;
    }

    // 危険な文字をチェック
    const char* dangerous = "<>:\"|?*";
    for (const char* p = dangerous; *p; p++) {
        if (strchr(filename, *p)) {
            return false;
        }
    }

    return true;
}

String M5StackWiFiUploader::_sanitizeFilename(const char* filename) {
    String result = filename;
    
    // パス区切り文字を除去
    result.replace("/", "_");
    result.replace("\\", "_");
    result.replace("..", "");
    
    // 先頭のドットを除去（隠しファイル対策）
    while (result.startsWith(".")) {
        result = result.substring(1);
    }

    return result;
}

bool M5StackWiFiUploader::_ensureUploadDirectory() {
    if (!SD.exists(_uploadPath.c_str())) {
        if (!SD.mkdir(_uploadPath.c_str())) {
            _log(1, "Failed to create upload directory: %s", _uploadPath.c_str());
            return false;
        }
        _log(3, "Upload directory created: %s", _uploadPath.c_str());
    }
    return true;
}

// ============================================================================
// プライベートメソッド - ユーティリティ
// ============================================================================

void M5StackWiFiUploader::_log(uint8_t level, const char* format, ...) {
    if (level > _debugLevel) {
        return;
    }

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    const char* levelStr = "";
    switch (level) {
        case 1: levelStr = "[ERROR]"; break;
        case 2: levelStr = "[WARN]"; break;
        case 3: levelStr = "[INFO]"; break;
        case 4: levelStr = "[DEBUG]"; break;
        default: levelStr = "[LOG]"; break;
    }

    Serial.printf("%s %s\n", levelStr, buffer);
}

void M5StackWiFiUploader::_sendJSONResponse(bool success, const char* message, const char* filename) {
    String json = "{";
    json += "\"success\": " + String(success ? "true" : "false") + ", ";
    json += "\"message\": \"" + String(message) + "\"";
    
    if (filename) {
        json += ", \"filename\": \"" + String(filename) + "\"";
    }
    
    json += "}";

    _webServer->send(success ? 200 : 400, "application/json", json);
}

String M5StackWiFiUploader::_getContentType(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";

    ext++;
    String extStr = String(ext);
    extStr.toLowerCase();

    if (extStr == "jpg" || extStr == "jpeg") return "image/jpeg";
    if (extStr == "png") return "image/png";
    if (extStr == "gif") return "image/gif";
    if (extStr == "txt") return "text/plain";
    if (extStr == "json") return "application/json";
    if (extStr == "csv") return "text/csv";

    return "application/octet-stream";
}

// ============================================================================
// セッション管理（将来の拡張用）
// ============================================================================

uint8_t M5StackWiFiUploader::_createSession(const char* filename, uint32_t filesize) {
    uint8_t sessionId = _nextSessionId++;
    UploadSession session;
    session.filename = filename;
    session.filesize = filesize;
    session.uploaded = 0;
    session.startTime = millis();
    session.isActive = true;
    session.sessionId = sessionId;

    _activeSessions[sessionId] = session;
    return sessionId;
}

UploadSession* M5StackWiFiUploader::_getSession(uint8_t sessionId) {
    auto it = _activeSessions.find(sessionId);
    if (it != _activeSessions.end()) {
        return &it->second;
    }
    return nullptr;
}

void M5StackWiFiUploader::_closeSession(uint8_t sessionId) {
    auto it = _activeSessions.find(sessionId);
    if (it != _activeSessions.end()) {
        if (it->second.file) {
            it->second.file.close();
        }
        _activeSessions.erase(it);
    }
}

void M5StackWiFiUploader::_closeAllSessions() {
    for (auto& session : _activeSessions) {
        if (session.second.file) {
            session.second.file.close();
        }
    }
    _activeSessions.clear();
}

void M5StackWiFiUploader::_handleDebugLog() {
    String message = _webServer->arg("message");
    Serial.printf("[WEB_DEBUG] %s\n", message.c_str());
    _webServer->send(200, "text/plain", "OK");
}
