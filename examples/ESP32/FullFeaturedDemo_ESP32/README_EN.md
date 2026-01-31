# ESP32 WiFi File Uploader - Full Featured Demo

This is a full-featured demo sketch for ESP32 WiFi file uploader. It provides HTTP and WebSocket servers, enables file uploads to SD card, real-time progress display, and LED status indicators.

## Key Features

- **HTTP File Upload**: Direct file upload from web browser
- **WebSocket Real-time Communication**: Real-time display of upload progress and file list
- **LED Status Indicator**: Visual system status display with 5-color LED
- **AP/STA Mode Support**: Supports both Access Point and Station modes
- **Error Handling**: Detailed error display and recovery processing
- **File Management**: File list display and deletion functionality

## Requirements

### Hardware
- ESP32 development board
- WS2812B LED x1 (NeoPixel, etc.)
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
// SD card SPI settings (FullFeaturedDemo_ESP32.ino lines 44-47)
#define SD_CS 5     // SD card module CS
#define SD_MOSI 23  // SD card module MOSI  
#define SD_MISO 19  // SD card module MISO
#define SD_SCK 18   // SD card module SCK

// LED settings (FullFeaturedDemo_ESP32.ino lines 52-53)
#define LED_PIN 27          // WS2812B data pin
#define NUM_LEDS 1          // Number of LEDs
#define LED_BRIGHTNESS 10    // Brightness (0-255) *set in setup()
```

## Library Installation

For Arduino IDE:
1. Open `Tools > Manage Libraries...`
2. Search and install the following libraries:
   - `FastLED` by Daniel Garcia
   - `M5StackWiFiUploader` by tomorrow56

For PlatformIO:
```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    fastled/FastLED@^3.6.0
    https://github.com/tomorrow56/M5StackWiFiUploader
```

## Configuration

### WiFi Mode Switching

**Station Mode (Default):**
Mode to connect to existing WiFi network.

```cpp
// FullFeaturedDemo_ESP32.ino line 22
// #define APMODE  // Comment out for Station mode

const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
```

**Access Point Mode:**
Mode where ESP32 acts as WiFi access point.

```cpp
// FullFeaturedDemo_ESP32.ino line 22
#define APMODE  // Uncomment for Access Point mode

const char* AP_SSID = "M5Stack-AP";
const char* AP_PASSWORD = "12345678";
const IPAddress AP_IP(192, 168, 4, 1);
```

### Differences by Mode

| Item | Station Mode | Access Point Mode |
|------|--------------|-------------------|
| IP Address | DHCP from router | 192.168.4.1 (fixed) |
| Connection Method | Connect to existing WiFi | Direct connection from smartphone/PC |
| WebSocket | Enabled | Disabled |
| URL | `http://[ESP32 IP]/uploads` | `http://192.168.4.1/uploads` |

## LED Status Indicator

| Status | Color (STA Mode) | Color (AP Mode) | Blink/Solid | Description |
|--------|------------------|-----------------|-------------|-------------|
| Initialization | Blue | Blue | Solid | System starting up |
| WiFi Connecting | Yellow | Yellow | 500ms blink | WiFi connection in progress |
| Running | Green | Cyan | Solid | Server running |
| Uploading | Green | Cyan | 200ms blink | File transfer in progress |
| Error | Red | Red | 500ms blink | Error occurred |

## Usage

### 1. Compile and Upload
1. Connect ESP32 to PC
2. Compile sketch in Arduino IDE or PlatformIO
3. Upload to ESP32

### 2. Check with Serial Monitor
- Baud rate: 115200
- IP address and access URL will be displayed

### 3. Access with Web Browser

**For Station Mode:**

- http://[ESP32 IP address]/uploads

**For Access Point Mode:**
1. Connect smartphone/PC to "M5Stack-AP" (password: 12345678)
2. Access `http://192.168.4.1/uploads`

### 4. File Upload
1. Open upload page in web browser
2. Select file with "Choose File"
3. Click "Upload" button
4. LED will blink cyan/green and return to solid when complete

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

**Current Settings (FullFeaturedDemo_ESP32.ino lines 277-284):**
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
Ready!
IP: 192.168.1.100
File Upload URL: http://192.168.1.100/uploads
WebSocket: ws://192.168.1.100
[LED] State changed to: 2
[UPLOAD] Started: test.jpg (1024000 bytes)
[LED] State changed to: 3
[UPLOAD] Complete: test.jpg (1024000 bytes, SUCCESS)
[LED] State changed to: 2
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
    https://github.com/tomorrow56/M5StackWiFiUploader
build_flags = 
    -DCORE_DEBUG_LEVEL=3
```

## License

This sketch is provided under MIT License. You can use, modify, and distribute freely.

## Contributing

Bug reports and feature requests are welcome via GitHub Issues.

## Related Links

- [M5StackWiFiUploader Library](https://github.com/tomorrow56/M5StackWiFiUploader)
- [FastLED Library](https://github.com/FastLED/FastLED)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)

---

**Version**: 1.0.0  
**Last Updated**: January 2026  
**Compatible Boards**: All ESP32 series
