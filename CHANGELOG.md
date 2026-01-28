# Changelog

All notable changes to the **XDB-Project** will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2026-01-29

### Added
- **Snapshot System**: Added Auto-Snapshotting every 5 writes and Manual Snapshot action via TCP (`{"action": "snapshot"}`).

### Fixed
- **Empty Request Handling** Added a filter to ignore whitespace-only or empty newline messages from clients

## [1.0.0] - 2026-01-27

### Added
- **Core Engine**: Initial implementation of the JSON-based database engine.
- **CRUD Operations**: Full support for Create, Read, Update, and Delete operations.
- **Dependency Management**: Integrated `cJSON` library via Git Submodules for high-performance JSON parsing.
- **CI/CD Pipeline**: 
    - Automated code formatting with `clang-format`.
    - Static analysis using `cppcheck` with a custom suppression list.
    - Multi-compiler build verification (GCC and Clang).
    - Memory safety validation using `Valgrind` for Pull Requests.
- **Build System**: Optimized Makefile for clean compilation and automated unit testing.
- **Project Structure**: Standardized C project layout (`src`, `include`, `third_party`).
- **Documentation**: Comprehensive README and initial project configuration files.
