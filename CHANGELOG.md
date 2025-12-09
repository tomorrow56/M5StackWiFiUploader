# Changelog

All notable changes to this project will be documented in this file.

## [1.2.0] - 2025-12-09

### Added
- Detailed API Reference (`docs/API_REFERENCE_v1.1.md`)
- Troubleshooting Guide (`docs/TROUBLESHOOTING.md`)
- Unit and integration tests (`tests/`)
- Full-featured demo application (`examples/FullFeaturedDemo/`)
- English README (`README_EN.md`)
- `CHANGELOG.md` and `CONTRIBUTING.md`

### Changed
- Updated `README.md` with v1.2.0 features.
- Updated `library.properties` to version 1.2.0.

## [1.1.0] - 2025-12-08

### Added
- `ErrorHandler` for detailed error tracking.
- `RetryManager` with exponential backoff.
- `ProgressTracker` with speed and time estimation.
- `WebSocketHandler` for real-time file transfers.
- Pause, resume, and cancel functionality.
- `WebSocketUploadExample`.

### Changed
- `Config.h` with WebSocket and retry settings.
- Integrated new features into `M5StackWiFiUploader`.
- Updated `library.properties` to version 1.1.0.

## [1.0.0] - 2025-12-07

### Added
- Initial release.
- HTTP server for file uploads.
- Web UI with drag-and-drop support.
- Multi-file upload capability.
- Basic progress and completion callbacks.
- `SDCardManager` for file system operations.
- `FileValidator` for file validation.
- `HTTPUploadExample` and `MultiFileUploadExample`.
