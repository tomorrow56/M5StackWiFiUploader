/**
 * WiFi File Uploader - Full Featured Demo
 * 
 * Features:
 * - HTTP/WebSocket server
 * - Status output via Serial Port
 * - Error handling
 * - Progress display
 * - LED control
 */

#include <WiFi.h>
#include <FastLED.h>
#include "M5StackWiFiUploader.h"
#include "SDCardManager.h"

// Timer interrupt
#include "esp_timer.h"
#include "hal/timer_hal.h"

// AP Mode setting
// #define APMODE

// Enable WebSocket
bool Websocket_Enabled = true;

// ========================================================================
// WiFi Settings
// ========================================================================
#ifdef APMODE
const char* AP_SSID = "M5Stack-AP";           // Access point name	 
const char* AP_PASSWORD = "12345678";         // Password (8+ chars)	 
const IPAddress AP_IP(192, 168, 4, 1);        // AP Mode IP address	 
const IPAddress AP_GATEWAY(192, 168, 4, 1);   // Gateway	 
const IPAddress AP_SUBNET(255, 255, 255, 0);  // Subnet mask
#else
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
#endif

// ========================================================================
// SD Card SPI Settings
// ========================================================================
#define SD_CS 5
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18

// ========================================================================
// LED Settings (WS2812B)
// ========================================================================
#define LED_PIN 27
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// ========================================================================
// Global Variables
// ========================================================================
M5StackWiFiUploader uploader;

// State management
enum AppState {
    STATE_INIT,
    STATE_WIFI_CONNECTING,
    STATE_RUNNING,
    STATE_UPLOADING,  // File transfer in progress
    STATE_ERROR
};

AppState currentState = STATE_INIT;
String statusMessage = "";
uint8_t activeUploads = 0;
uint32_t totalUploaded = 0;
String lastUploadedFile = "";

// LED control timer
volatile bool ledState = false;  // Blink flag (volatile)
volatile bool isUploading = false;  // Transfer in progress flag (volatile)
volatile unsigned long blinkInterval = 500000;  // Blink interval (microseconds)
esp_timer_handle_t ledTimer;

// Display update timer
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 500;

// ========================================================================
// Setup
// ========================================================================
void setup() {
    FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
    FastLED.setBrightness(10);
    
    // Initialize LED timer
    initLEDTimer();
    
    Serial.begin(115200);
    
    displayHeader();
    displayMessage("Initializing SD Card...");
    
    // Initialize SD card
    if (!SDCardManager::initialize(SD_CS)) {
        currentState = STATE_ERROR;
        statusMessage = "SD Card initialization failed";
        displayMessage("ERROR: SD Card failed");
        Serial.println("SD Card initialization failed!");
        setLEDByState(currentState);
        delay(2000);
    } else {
        displayMessage("SD Card Ready!");
        Serial.println("SD Card initialized successfully!");
        delay(1000);
    }
    
   // WiFi connection
    setLEDByState(STATE_WIFI_CONNECTING);
 
 #ifdef APMODE
    displayMessage("Starting AP Mode...");	 
    startAPMode();
#else
    displayMessage("Connecting WiFi...");
    connectWiFi();
#endif
    
    // Uploader configuration (set callbacks before begin())
    configureUploader();
    
    // Start uploader
    if (uploader.begin(80, "/uploads")) {  // File upload on standard port 80
        currentState = STATE_RUNNING;
        statusMessage = "Server started";
        displayMessage("Ready!");
        displayServerInfo();
        setLEDByState(currentState);
    } else {
        currentState = STATE_ERROR;
        statusMessage = "Failed to start server";
        displayMessage("ERROR: Server failed");
        setLEDByState(currentState);
    }
}

// ========================================================================
// Main Loop
// ========================================================================
void loop() {

    // Handle client requests
    if (currentState == STATE_RUNNING || currentState == STATE_UPLOADING) {
        uploader.handleClient();
    }
    
    // Button handling
    handleButtons();
    
    // Display update
    if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }
    
    delay(10);
}

#ifdef APMODE
// ========================================================================
// Start AP Mode
// =========================================================================
void startAPMode() {
    Serial.println("Starting Access Point Mode...");
    
    // Configure AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    
    // Start AP
    bool success = WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    if (success) {
        Serial.println("AP Mode Started!");
        Serial.printf("SSID: %s\n", AP_SSID);
        Serial.printf("Password: %s\n", AP_PASSWORD);
        Serial.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println("AP Mode Failed!");
        while (1) delay(1000);
    }
    
    delay(1000);
}
#else

// ========================================================================
// WiFi Connection
// =========================================================================
void connectWiFi() {
    currentState = STATE_WIFI_CONNECTING;
    displayMessage("Connecting to WiFi...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        displayMessage("WiFi Connected!");
        delay(1000);
    } else {
        currentState = STATE_ERROR;
        displayMessage("WiFi Connection Failed");
        setLEDByState(currentState);
        while (1) delay(1000);
    }
}
#endif

// ========================================================================
// Upload Callback Functions
// ========================================================================
void onUploadStart(const char* filename, uint32_t filesize) {
    Serial.printf("[UPLOAD] Started: %s (%d bytes)\n", filename, filesize);
    displayMessage("Uploading: " + String(filename));
    setLEDByState(STATE_UPLOADING);
    isUploading = true;
}

void onUploadComplete(const char* filename, uint32_t filesize, bool success) {
    Serial.printf("[UPLOAD] Complete: %s (%d bytes, %s)\n", 
                 filename, filesize, success ? "SUCCESS" : "FAILED");
    
    if (success) {
        displayMessage("Upload Complete!");
    } else {
        displayMessage("Upload Failed!");
        setLEDByState(STATE_ERROR);
        isUploading = false;
        return;
    }
    
    setLEDByState(STATE_RUNNING);
    isUploading = false;
}

// ========================================================================
// Uploader Configuration
// =========================================================================
void configureUploader() {
    // Configure upload callbacks
    uploader.onUploadStart(onUploadStart);
    uploader.onUploadComplete(onUploadComplete);
    
    Serial.println("[CONFIG] Upload callbacks configured");
    
    // Set debug level
    uploader.setDebugLevel(3);  // Verbose logging
    
    // Set maximum file size (50MB)
    uploader.setMaxFileSize(50 * 1024 * 1024);
    
#ifdef APMODE
    // Disable WebSocket in AP mode
    Websocket_Enabled = false;
    uploader.enableWebSocket(false);
#else
    // Enable WebSocket
    Websocket_Enabled = true;
    uploader.enableWebSocket(true);
#endif
    
    // Allowed extensions
    const char* extensions[] = {
        "jpg", "jpeg", "png", "gif", "bmp",
        "bin", "dat", "txt", "csv", "json",
        "zip", "rar", "7z", "tar", "gz",
        "pdf", "mp4", "apk"
    };
    uploader.setAllowedExtensions(extensions, 18);
    
    Serial.println("[CONFIG] Uploader configuration complete");
}

// ========================================================================
// Display Functions
// =========================================================================
void displayHeader() {
#ifdef APMODE
    Serial.println("WiFi File Uploader (AP Mode)");
#else
    Serial.println("WiFi File Uploader");
#endif
}

void displayMessage(String msg) {
    Serial.println(msg);
}

// ========================================================================
// Button Handling
// ========================================================================
void handleButtons() {
    // Add as needed
}

// ========================================================================
// Display Functions
// ========================================================================
void displayServerInfo() {
#ifdef APMODE
    Serial.printf("SSID: %s\n", AP_SSID);
    Serial.println();
    Serial.printf("Password: %s\n", AP_PASSWORD);
    Serial.println();
    Serial.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println();
    Serial.printf("URL: http://%s\n", WiFi.softAPIP().toString().c_str());
    Serial.println();
#else
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.println();
    Serial.printf("File Upload URL: http://%s/uploads", WiFi.localIP().toString().c_str());
    Serial.println();
#endif
    if (Websocket_Enabled) {
        Serial.printf("WebSocket: ws://%s", WiFi.localIP().toString().c_str());
        Serial.println();
    } else {
        Serial.println("WebSocket: Disabled");
    }
}


void displayUploadStatus(const char* filename, uint32_t uploaded, uint32_t total) {
    Serial.printf("Uploading: %s", filename);
    Serial.println();
    // プログレス
    uint8_t progress = (total > 0) ? (uploaded * 100) / total : 0;
    Serial.printf("Progress: %d%%", progress);
    Serial.println();
    
    int barWidth = (progress * 280) / 100;
    Serial.printf("%d / %d bytes", uploaded, total);
    Serial.println();
}

void displayFileList() {
    Serial.println("Files on SD Card:");
    
    auto files = uploader.listFiles();
    int y = 65;
    for (int i = 0; i < min((int)files.size(), 8); i++) {
        Serial.printf("%d. %s", i + 1, files[i].c_str());
        Serial.println();
    }
    
    if (files.size() == 0) {
        Serial.println("No files");
    }
}

void displayStatus() {
    Serial.println("System Status:");
    
    Serial.printf("Active uploads: %d", uploader.getActiveUploads());
    Serial.println();
    
    Serial.printf("Total uploaded: %d bytes", uploader.getTotalUploaded());
    Serial.println();
    
    Serial.printf("SD Free: %d MB", uploader.getSDFreeSpace() / 1024 / 1024);
    Serial.println();
    
    Serial.printf("SD Total: %d MB", uploader.getSDTotalSpace() / 1024 / 1024);
    Serial.println();
    
    Serial.printf("Last file: %s", lastUploadedFile.c_str());
    Serial.println();
}

void updateDisplay() {
    // Status bar update
    Serial.printf("Active: %d | Total: %d KB", 
                 uploader.getActiveUploads(),
                 uploader.getTotalUploaded() / 1024);
    Serial.println();
}

// ========================================================================
// LED Control
// =========================================================================

// Timer callback function
void ledTimerCallback(void* arg) {
    ledState = !ledState;
    
    // LED control
    switch (currentState) {
        case STATE_INIT:
            leds[0] = CRGB::Blue;  // Solid
            break;
        case STATE_WIFI_CONNECTING:
            leds[0] = ledState ? CRGB::Yellow : CRGB::Black;
            break;
        case STATE_UPLOADING:
#ifdef APMODE
            leds[0] = ledState ? CRGB::Cyan : CRGB::Black;  // Blink during transfer
#else
            leds[0] = ledState ? CRGB::Green : CRGB::Black;  // Blink during transfer
#endif
            break;
        case STATE_ERROR:
            leds[0] = ledState ? CRGB::Red : CRGB::Black;
            break;
        case STATE_RUNNING:
#ifdef APMODE
            leds[0] = CRGB::Cyan;  // Solid
#else
            leds[0] = CRGB::Green;  // Solid
#endif
            break;
    }
    
    FastLED.show();
}

// Initialize LED timer
void initLEDTimer() {
    esp_timer_create_args_t timerArgs = {
        .callback = &ledTimerCallback,
        .name = "led_timer"
    };
    
    esp_timer_create(&timerArgs, &ledTimer);
    esp_timer_start_periodic(ledTimer, 500000);  // Initial 500ms interval
}

void setLEDByState(AppState state) {
    currentState = state;
    ledState = false;
    
    // Set blink interval
    switch (state) {
        case STATE_UPLOADING:
            blinkInterval = 200000;  // 200ms (transfer)
            break;
        default:
            blinkInterval = 500000;  // 500ms (normal)
            break;
    }
    
    // Reset timer
    esp_timer_stop(ledTimer);
    esp_timer_start_periodic(ledTimer, blinkInterval);
    
    // Debug output
    Serial.printf("[LED] State changed to: %d\n", state);
    
}
