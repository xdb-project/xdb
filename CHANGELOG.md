# Changelog

All notable changes to the **XDB-Project** will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.4.1] - 2026-01-31

### Fixed (Stability & Memory Integrity)
- **Atomic Insert (Deep Copy)**: Resolved Use-After-Free memory corruption by implementing `cJSON_Duplicate` in the `db_insert` function. The database now maintains full memory ownership, isolated from the network layer request lifecycle.
- **Safe Detach-Append Strategy**: Replaced destructive in-place replacement with a Detach & Append strategy for update operations. This ensures internal Linked List pointer integrity during high-concurrency access.
- **Thread-Safe Iteration**: Replaced standard iteration macros with manual pointer navigation to prevent race conditions when the collection structure is modified by concurrent threads.
- **SIGSEGV Prevention**: Eliminated the primary cause of Segmentation Faults in the `get_object_item` function, previously triggered by dangling pointer access.

## [1.4.0] - 2026-01-30

### Added
- **Selective Update (Patch)**: Supported partial updates, allowing users to update specific fields without overwriting the entire document.

### Enforced
- **_id Immutability**: Implemented strict protection to ensure any attempt to modify the unique identifier (`_id`) during an update is ignored, maintaining data integrity.

### Optimized
- **Memory Pointer Synchronization**: Improved synchronization between `g_index` and the main document store to ensure data consistency and faster lookups.

## [1.3.0] - 2026-01-30

### Added
- **Atomic Update**: Implemented `db_update` function for efficient document updates without altering the `_id`.
- **Upsert Logic**: Introduced `db_upsert` function to automatically determine whether to perform an insert or update based on ID existence.
- **Protocol Extension**: Integrated `update` and `upsert` actions into `server.c` for network-layer access.
- **Regression Tests**: Added new test cases in `test_crud.c` to verify data update workflows.

### Fixed
- **Compiler Warnings**: Added `<stdlib.h>` header in unit tests to resolve implicit `free()` declaration issues.
- **Test Compatibility**: Replaced `ASSERT_STR_EQ` macro with standard `strcmp` in `test_crud.c` to ensure compilation stability.

## [1.2.0] - 2026-01-29

### Added
- **Basic Indexing**: Implemented an in-memory Hash Map (O(1) Lookups) for _id fields to replace linear scans during ID-based queries.
- **Index Synchronization**: Automatic index updates on document insertion and deletion to ensure data consistency.

### Improved
- **Search Performance**: Significant latency reduction for large datasets when querying by unique identifier.

### Fixed
- **Deep Copy Indexing**: Resolved empty data return in `db_find` by implementing Deep Copy indexing to ensure data persistence during retrieval.

## [1.1.0] - 2026-01-29

### Added
- **Snapshot System**: Added Auto-Snapshotting every 5 writes and Manual Snapshot action via TCP (`{"action": "snapshot"}`).

### Fixed
- **Empty Request Handling**: Added a filter to ignore whitespace-only or empty newline messages from clients

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
