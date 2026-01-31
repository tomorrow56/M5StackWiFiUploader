# ESP32 WiFi File Uploader - Full Featured Demo (OLED Version + OTA Update)

This is a full-featured demo sketch for ESP32 WiFi file uploader with OLED display and OTA update support. It provides HTTP and WebSocket servers, enables file uploads to SD card, real-time progress display, LED status indicators, **0.91-inch OLED display for system status visualization**, and **OTA (Over-The-Air) firmware update functionality**.

## Key Features

- **HTTP File Upload**: Direct file upload from web browser
- **WebSocket Real-time Communication**: Real-time display of upload progress and file list
- **LED Status Indicator**: Visual system status display with 5-color LED
- **OLED Display**: 0.91-inch SSD1306 OLED for system status and progress display
- **OTA Firmware Update**: Separate OTA server for web-based firmware updates
- **AP/STA Mode Support**: Supports both Access Point and Station modes
- **Error Handling**: Detailed error display and recovery processing
- **File Management**: File list display and deletion functionality

## Requirements

### Hardware
- ESP32 development board
- WS2812B LED x1 (NeoPixel, etc.)
- 0.91-inch OLED Display (SSD1306, 128x32 pixels)
- SD card module
- SD card (formatted as FAT32)
- Breadboard and jumper wires

### Software
- Arduino IDE 1.8.13+ or PlatformIO
- Required libraries (described below)

## PIN Configuration

### Sketch PIN Configuration
The PIN configuration in the sketch is as follows. Change as needed.

```cpp
// OLED Display settings (FullFeaturedDemo_ESP32_oled_OTA.ino lines 62-67)
#define OLED_SDA 21    // OLED SDA pin
#define OLED_SCL 22    // OLED SCL pin
#define OLED_ADDR 0x3C // OLED I2C address
#define SCREEN_WIDTH 128   // Screen width
#define SCREEN_HEIGHT 32   // Screen height
#define OLED_RESET -1      // Reset pin (use -1 if not connected)

// SD card SPI settings (FullFeaturedDemo_ESP32_oled_OTA.ino lines 74-77)
#define SD_CS 5     // SD card module CS
#define SD_MOSI 23  // SD card module MOSI  
#define SD_MISO 19  // SD card module MISO
#define SD_SCK 18   // SD card module SCK

// LED settings (FullFeaturedDemo_ESP32_oled_OTA.ino lines 82-83)
#define LED_PIN 27          // WS2812B data pin
#define NUM_LEDS 1          // Number of LEDs
#define LED_BRIGHTNESS 10    // Brightness (0-255) *set in setup()
```

## Library Installation

For Arduino IDE:
1. Open `Tools > Manage Libraries...`
2. Search and install the following libraries:
   - `FastLED` by Daniel Garcia
   - `Adafruit GFX Library` by Adafruit
   - `Adafruit SSD1306` by Adafruit
   - `ESP32FwUploader` by tomorrow56
   - `M5StackWiFiUploader` by tomorrow56

For PlatformIO:
```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    fastled/FastLED@^3.6.0
    adafruit/Adafruit GFX Library@^1.13.0
    adafruit/Adafruit SSD1306@^2.5.7
    https://github.com/tomorrow56/M5StackWiFiUploader
    https://github.com/tomorrow56/ESP32FwUploader
```

## Configuration

### WiFi Mode Switching

**Station Mode (Default):**
Mode to connect to existing WiFi network.

```cpp
// FullFeaturedDemo_ESP32_oled_OTA.ino line 31
// #define APMODE  // Comment out for Station mode

const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
```

**Access Point Mode:**
Mode where ESP32 acts as WiFi access point.

```cpp
// FullFeaturedDemo_ESP32_oled_OTA.ino line 31
#define APMODE  // Uncomment for Access Point mode

const char* AP_SSID = "M5Stack-AP";
const char* AP_PASSWORD = "12345678";
const IPAddress AP_IP(192, 168, 4, 1);
```

### OTA Server Configuration

OTA (Over-The-Air) update functionality configuration. The OTA server runs on a separate port from the file upload server.

```cpp
// OTA settings (FullFeaturedDemo_ESP32_oled_OTA.ino lines 53-57)
static const char* OTA_USERNAME = "admin";        // OTA authentication username
static const char* OTA_PASSWORD = "password123";   // OTA authentication password
static const uint16_t OTA_PORT = 8080;            // OTA server port
WebServer otaServer(OTA_PORT);                    // OTA server instance
```

### OTA Server Customization

**Change Authentication:**
```cpp
static const char* OTA_USERNAME = "your_username";  // Change username
static const char* OTA_PASSWORD = "your_password";   // Change password
```

**Change OTA Port:**
```cpp
static const uint16_t OTA_PORT = 9090;  // Change port to 9090
```

**OTA Behavior Settings:**
```cpp
// OTA settings in setup() (FullFeaturedDemo_ESP32_oled_OTA.ino lines 163-166)
ESP32FwUploader.setDebug(true);      // Enable/disable debug output
ESP32FwUploader.setDarkMode(false);   // Set dark mode
ESP32FwUploader.setAuth(OTA_USERNAME, OTA_PASSWORD);  // Set authentication
ESP32FwUploader.setAutoReboot(true); // Enable/disable auto reboot after update
```

### Differences by Mode

| Item | Station Mode | Access Point Mode |
|------|--------------|-------------------|
| IP Address | DHCP from router | 192.168.4.1 (fixed) |
| Connection Method | Connect to existing WiFi | Direct connection from smartphone/PC |
| WebSocket | Enabled | Disabled |
| File Upload URL | `http://[ESP32 IP]/uploads` | `http://192.168.4.1/uploads` |
| OTA Update URL | `http://[ESP32 IP]:8080/update` | `http://192.168.4.1:8080/update` |

## OLED Display

The OLED display shows the following information in real-time:

- **Header**: "WiFi File Uploader"
- **Status Message**: Current system status ("Initializing...", "Connecting WiFi...", "Ready!", etc.)
- **IP Address**: ESP32's IP address
- **Upload Information**: Uploading filename and progress (percentage display)

### OLED Display Update Timing
- Auto-update every 500ms
- Immediate update on state changes
- Real-time update during upload progress

## LED Status Indicator

| Status | Color (STA Mode) | Color (AP Mode) | Blink/Solid | Description |
|--------|------------------|-----------------|-------------|-------------|
| Initialization | Blue | Blue | Solid | System starting up |
| WiFi Connecting | Yellow | Yellow | 500ms blink | WiFi connection in progress |
| Running | Green | Cyan | Solid | Server running |
| Uploading | Green | Cyan | 200ms blink | File transfer in progress |
| Error | Red | Red | 500ms blink | Error occurred |

## OTA Update Usage

### 1. Prepare for OTA Update
- Ensure ESP32 is connected to WiFi
- Check OTA server URL in serial monitor

### 2. Access OTA Page in Web Browser

**For Station Mode:**
```
http://[ESP32 IP address]:8080/update
```

**For Access Point Mode:**
```
http://192.168.4.1:8080/update
```

### 3. Authentication and Firmware Upload
1. Login with username: `admin`, password: `password123`
2. Select new firmware file (.bin) with "Choose File"
3. Click "Upload" button
4. Progress will be displayed, ESP32 will automatically reboot when complete

### 4. Verify OTA Update
- After reboot, check serial monitor for new version display
- Verify web interface works correctly

### OTA Update Notes
- **Firmware File**: Generate with Arduino IDE "Sketch > Export Compiled Binary"
- **File Size**: Be aware of ESP32 flash size limitations
- **Power Stability**: Do not disconnect power during update
- **Network**: Ensure WiFi connection is not interrupted during update

## Usage

### 1. Compile and Upload
1. Connect ESP32 to PC
2. Compile sketch in Arduino IDE or PlatformIO
3. Upload to ESP32

### 2. Check with Serial Monitor
- Baud rate: 115200
- IP address and access URLs will be displayed
- OLED display will also show status

### 3. Access with Web Browser

**For Station Mode:**

- File Upload: http://[ESP32 IP address]/uploads
- OTA Update: http://[ESP32 IP address]:8080/update

**For Access Point Mode:**
1. Connect smartphone/PC to "M5Stack-AP" (password: 12345678)
2. File Upload: `http://192.168.4.1/uploads`
3. OTA Update: `http://192.168.4.1:8080/update`

### 4. File Upload
1. Open upload page in web browser
2. Select file with "Choose File"
3. Click "Upload" button
4. LED will blink cyan/green and OLED will show progress
5. When complete, LED returns to solid and OLED shows "Ready!"

### 5. WebSocket Features (when enabled)
- Real-time upload progress display
- Automatic file list updates
- Error notifications

## Supported File Formats

### Default File Formats
- **Images**: jpg, jpeg, png, gif, bmp
- **Data**: bin, dat, txt, csv, json
- **Compressed**: zip, rar, 7z, tar, gz
- **Others**: pdf, mp4, apk

### How to Change Allowed File Formats

Allowed file formats are set in the `configureUploader()` function. Follow these steps to change.

**Current Settings (FullFeaturedDemo_ESP32_oled_OTA.ino lines 277-284):**
```cpp
// Allowed extensions
const char* extensions[] = {
    "jpg", "jpeg", "png", "gif", "bmp",
    "bin", "dat", "txt", "csv", "json",
    "zip", "rar", "7z", "tar", "gz",
    "pdf", "mp4", "apk"
};
uploader.setAllowedExtensions(extensions, 18);  // 18 extensions
```

**Example 1: Add audio files**
```cpp
const char* extensions[] = {
    "jpg", "jpeg", "png", "gif", "bmp",
    "bin", "dat", "txt", "csv", "json",
    "zip", "rar", "7z", "tar", "gz",
    "pdf", "mp4", "apk",
    "mp3", "wav", "flac", "aac"  // Add audio files
};
uploader.setAllowedExtensions(extensions, 22);  // Change to 22
```

**Example 2: Allow all formats**
```cpp
const char* extensions[] = {
    "*"  // Wildcard to allow all formats
};
uploader.setAllowedExtensions(extensions, 1);
```

### Notes
- Write extensions in lowercase
- Do not include periods (.)
- Array element count must match second argument of `setAllowedExtensions()`
- For security, recommend allowing only necessary file formats

## Customization

### Enable/Disable WebSocket
```cpp
bool Websocket_Enabled = true;  // true to enable, false to disable
```

### Change Maximum File Size
```cpp
uploader.setMaxFileSize(100 * 1024 * 1024);  // Change to 100MB
```

### Change LED Blink Interval
```cpp
const unsigned long LED_BLINK_INTERVAL = 500;      // Normal blink (ms)
const unsigned long UPLOAD_BLINK_INTERVAL = 200;   // Upload blink (ms)
```

### OLED Display Customization
```cpp
// Change display update interval
const unsigned long DISPLAY_UPDATE_INTERVAL = 500;  // 500ms -> 1000ms change possible

// Customize OLED display content by editing these functions
void displayHeader();      // Header display
void displayMessage();     // Message display
void renderOLED();         // Overall rendering
```

### OTA Callback Customization
```cpp
// OTA callback settings in setup() (FullFeaturedDemo_ESP32_oled_OTA.ino lines 168-190)
ESP32FwUploader.onStart([]() {
    // OTA start processing
    Serial.println("[OTA] Started");
});

ESP32FwUploader.onProgress([](size_t current, size_t total) {
    // OTA progress display
    float progress = (float)current / (float)total * 100.0f;
    Serial.printf("[OTA] Progress: %.1f%%\n", progress);
});

ESP32FwUploader.onEnd([](bool success) {
    // OTA completion processing
    Serial.printf("[OTA] End: %s\n", success ? "SUCCESS" : "FAILED");
});

ESP32FwUploader.onError([](ESP32Fw_Error error, const String& message) {
    // OTA error processing
    Serial.printf("[OTA] Error %d: %s\n", error, message.c_str());
});
```

## Serial Output Example

```
WiFi File Uploader
[CONFIG] Upload callbacks configured
[CONFIG] Uploader configuration complete
Connecting WiFi...
WiFi Connected!
IP: 192.168.1.100
[INFO] WebSocket enabled
[INFO] M5StackWiFiUploader started on port 80
[INFO] OTA server started on port 8080
Ready!
IP: 192.168.1.100
File Upload URL: http://192.168.1.100/uploads
OTA URL: http://192.168.1.100:8080/update
OTA Username: admin
OTA Password: password123
WebSocket: ws://192.168.1.100
[LED] State changed to: 2
[UPLOAD] Started: test.jpg (1024000 bytes)
[LED] State changed to: 3
[UPLOAD] Complete: test.jpg (1024000 bytes, SUCCESS)
[LED] State changed to: 2
[OTA] Started
[OTA] Progress: 25.0% (256000/1024000 bytes)
[OTA] Progress: 50.0% (512000/1024000 bytes)
[OTA] Progress: 75.0% (768000/1024000 bytes)
[OTA] Progress: 100.0% (1024000/1024000 bytes)
[OTA] End: SUCCESS
```

## OLED Display Example

```
┌──────────────────────────────┐
│WiFi File Uploader            │
│Ready!                        │
│IP: 192.168.1.100             │
│Upload: test.jpg 75%          │
└──────────────────────────────┘
```

## Usage with PlatformIO

`platformio.ini`:
```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    fastled/FastLED@^3.6.0
    adafruit/Adafruit GFX Library@^1.13.0
    adafruit/Adafruit SSD1306@^2.5.7
    https://github.com/tomorrow56/M5StackWiFiUploader
    https://github.com/tomorrow56/ESP32FwUploader
build_flags = 
    -DCORE_DEBUG_LEVEL=3
```

## Troubleshooting

### OLED Not Displaying
1. Check I2C connection (SDA: GPIO21, SCL: GPIO22)
2. Verify OLED I2C address (usually 0x3C)
3. Check power supply (3.3V or 5V)
4. Use I2C scanner to verify address

### SD Card Not Recognized
1. Check SPI connection
2. Verify SD card is formatted as FAT32
3. Check SD card capacity (max 32GB recommended)

### LED Not Lighting
1. Check LED data pin connection (GPIO27)
2. Verify LED power and GND connection
3. Check brightness setting (FastLED.setBrightness())

### OTA Update Fails
1. Check network connection
2. Verify authentication credentials are correct
3. Ensure firmware file is not corrupted
4. Check ESP32 available space
5. Verify power supply is stable

### Cannot Access OTA Page
1. Ensure OTA_PORT doesn't conflict with other services
2. Check firewall settings
3. Verify ESP32 is running properly via serial monitor

## Security Considerations

### OTA Update Security Recommendations
- **Strong Passwords**: Change default passwords
- **Network Isolation**: Perform OTA updates on trusted networks
- **Firmware Verification**: Verify firmware integrity before update
- **Rollback Capability**: Consider rollback mechanism for failed updates

### Network Security
- Use WPA2/WPA3 encrypted WiFi
- Do not expose OTA port unnecessarily to external networks
- Change passwords regularly

## License

This sketch is provided under MIT License. You can use, modify, and distribute freely.

## Contributing

Bug reports and feature requests are welcome via GitHub Issues.

## Related Links

- [M5StackWiFiUploader Library](https://github.com/tomorrow56/M5StackWiFiUploader)
- [ESP32FwUploader Library](https://github.com/tomorrow56/ESP32FwUploader)
- [FastLED Library](https://github.com/FastLED/FastLED)
- [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)

---

**Version**: 1.0.0  
**Last Updated**: January 2026  
**Compatible Boards**: All ESP32 series  
**Special Features**: 0.91-inch OLED display support + OTA update functionality
