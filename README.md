# XDB - High-Performance NoSQL Database in C

> A lightweight, thread-safe, document-oriented database written in pure C with JSON storage, concurrent client handling, and persistent disk-based storage.

<div align="center">

[![Language](https://img.shields.io/badge/language-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](#build--run)
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](CHANGELOG.md)
[![POSIX](https://img.shields.io/badge/POSIX-Compliant-success.svg)](https://en.wikipedia.org/wiki/POSIX)

**[Quick Start](#quick-start) • [Installation](#installation) • [API Reference](#api-documentation) • [Testing](#testing) • [Contributing](CONTRIBUTING.md) • [Changelog](CHANGELOG.md)**

</div>

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Quick Start](#quick-start)
- [Installation & Build](#installation--build)
- [Running the Server](#running-the-server)
- [API Documentation](#api-documentation)
- [Testing](#testing)
- [Architecture](#architecture)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Overview

XDB is a production-ready NoSQL database implementation written in pure C, designed to demonstrate enterprise-level systems programming patterns. It combines simplicity with robustness, providing CRUD operations on JSON documents with concurrent client handling, persistent storage, and a custom query engine.

**Key Highlights:**
- Pure C implementation with zero external runtime dependencies (except cJSON for JSON parsing)
- Multi-threaded architecture supporting concurrent client connections
- Automatic persistence with JSON-based storage
- Custom query engine with exact-match semantics
- Comprehensive test suite with custom testing framework
- Memory-safe implementation with proper resource cleanup

## Features

| Feature | Details |
|---------|---------|
| **Modular Architecture** | Clean separation of concerns with independent modules for networking, storage, and query logic |
| **Concurrent Processing** | Multi-threaded server using POSIX pthreads for safe concurrent TCP client connections |
| **Persistent Storage** | Automatic disk-based persistence with JSON serialization |
| **Query Engine** | Document filtering with exact match semantics and result pagination |
| **UUID Generation** | Automatic unique identifier assignment for all documents |
| **Thread Safety** | POSIX mutexes protect all shared resources from concurrent access issues |
| **Memory Safety** | Manual memory management with proper cleanup and leak prevention |
| **Signal Handling** | Graceful shutdown via SIGINT with automatic data persistence |
| **IPv6 Support** | Dual-stack networking for IPv4 and IPv6 connections |

---

## Quick Start

### 1. Clone and Build

```bash
# Clone the repository
git clone https://github.com/xdb-project/xdb.git
cd xdb

# Initialize submodules
git submodule update --init --recursive

# Build the project
make

# Verify the build
ls -la bin/xdb
```

### 2. Start the Server

```bash
./bin/xdb
```

Expected output:
```
Starting XDB Server...

[08:17:43] [INFO] Initialized new database instance.
[08:17:43] [INFO] Server listening on 0.0.0.0:8080
```

### 3. Connect and Test

```bash
# In another terminal, connect via telnet
telnet localhost 8080

# Send your first command (JSON format)
{"action":"insert","collection":"users","data":{"name":"Alice","email":"alice@example.com"}}

# You should receive
{"status":"ok","message":"Document Inserted","data":{"_id":"a1b2c3d4e5f6g7h8"}}
```

### 4. Run Tests

```bash
make test
```

---

## Installation & Build

### Prerequisites

| Requirement | Version | Purpose |
|-------------|---------|---------|
| **GCC** | 7.0+ | C compiler with C99 support |
| **GNU Make** | 3.81+ | Build automation tool |
| **POSIX Threads** | Native | For concurrent connection handling |
| **cJSON** | Latest | JSON parser (included in third_party/) |

**Supported Platforms:** Linux (Ubuntu 20.04+, Debian 10+, CentOS 7+), macOS, FreeBSD, and other POSIX-compliant systems.

### Installation Steps

```bash
# Clone the repository
git clone https://github.com/xdb-project/xdb.git
cd xdb

# Initialize submodules (required)
git submodule update --init --recursive

# Verify cJSON is present
ls -la third_party/cJSON/cJSON.h third_party/cJSON/cJSON.c
```

---

## Running the Server

### Build

```bash
# Standard build
make

# Clean build
make clean && make

# Build with debug symbols
make DEBUG=1

# Clean all artifacts
make distclean
```

### Starting the Server

```bash
# Start the server (default: localhost:8080)
./bin/xdb
```

The server listens on `0.0.0.0:8080` (all network interfaces, port 8080).

### Background Execution

```bash
# Run in background with logging
nohup ./bin/xdb > xdb.log 2>&1 &
echo $! > xdb.pid

# View logs
tail -f xdb.log

# Stop the server
kill $(cat xdb.pid)
```

### Running Tests

```bash
# Run all unit tests
make test

# Run with verbose output
make test VERBOSE=1
```

### Production Deployment

For production environments, consider using:

**Systemd Service (Linux):**
```bash
# Create /etc/systemd/system/xdb.service
sudo systemctl enable xdb
sudo systemctl start xdb
sudo systemctl status xdb
```

**PM2 Process Manager:**
```bash
npm install -g pm2
pm2 start ./bin/xdb --name xdb --interpreter none
pm2 save
pm2 startup
```

---

## API Documentation

XDB implements a JSON-based protocol over TCP. All communication follows the request-response pattern with UTF-8 encoding.

### Connection Details

| Property | Value |
|----------|-------|
| **Protocol** | TCP/IP |
| **Host** | `0.0.0.0` (all interfaces) |
| **Port** | `8080` (fixed) |
| **Format** | JSON over TCP |
| **Encoding** | UTF-8 |

### Connecting to the Server

```bash
# Using telnet
telnet localhost 8080

# Using netcat
echo '{"action":"find","collection":"users"}' | nc localhost 8080

# Using Python
python3 << 'EOF'
import socket, json
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 8080))
cmd = {"action": "insert", "collection": "users", "data": {"name": "Alice"}}
sock.send(json.dumps(cmd).encode() + b'\n')
print(json.loads(sock.recv(4096).decode()))
sock.close()
EOF
```

### Command Format

All requests are JSON objects:

```json
{
  "action": "operation_name",
  "collection": "collection_name",
  ...
}
```

### Standard Response Format

```json
{
  "status": "ok|error",
  "message": "Human-readable message",
  "data": { ... }
}
```

---

### 1. Insert Document

Inserts a new document into a collection and assigns a unique ID.

**Request:**
```json
{
  "action": "insert",
  "collection": "users",
  "data": {
    "name": "Alice",
    "email": "alice@example.com",
    "role": "admin"
  }
}
```

**Response:**
```json
{
  "status": "ok",
  "message": "Document Inserted",
  "data": {
    "_id": "a1b2c3d4e5f6g7h8"
  }
}
```

---

### 2. Find Documents

Queries documents in a collection with optional filtering and pagination.

**Request:**
```json
{
  "action": "find",
  "collection": "users",
  "query": {
    "role": "admin"
  },
  "limit": 10
}
```

**Response:**
```json
{
  "status": "ok",
  "message": "Documents found",
  "data": {
    "count": 2,
    "documents": [
      {
        "_id": "a1b2c3d4e5f6g7h8",
        "name": "Alice",
        "email": "alice@example.com",
        "role": "admin"
      },
      {
        "_id": "b2c3d4e5f6g7h8i9",
        "name": "Bob",
        "email": "bob@example.com",
        "role": "admin"
      }
    ]
  }
}
```

**Parameters:**
- `query` (object, optional): Filter criteria using exact match
- `limit` (integer, optional): Maximum number of documents to return

---

### 3. Count Documents

Returns the total count of documents matching optional criteria.

**Request:**
```json
{
  "action": "count",
  "collection": "users",
  "query": {
    "role": "admin"
  }
}
```

**Response:**
```json
{
  "status": "ok",
  "message": "Count retrieved",
  "data": {
    "count": 5
  }
}
```

---

### 4. Delete Document

Removes a document by its unique identifier.

**Request:**
```json
{
  "action": "delete",
  "collection": "users",
  "id": "a1b2c3d4e5f6g7h8"
}
```

**Response:**
```json
{
  "status": "ok",
  "message": "Document Deleted",
  "data": null
}
```

---

### 5. Exit Connection

Gracefully closes the TCP connection.

**Request:**
```json
{
  "action": "exit"
}
```

**Response:**
```json
{
  "status": "ok",
  "message": "Goodbye",
  "data": null
}
```

---

### Query Semantics

| Aspect | Behavior |
|--------|----------|
| **Matching** | Exact match on all query fields |
| **Null Handling** | Missing fields in documents don't match filters |
| **Type Comparison** | Strict type matching (string ≠ number) |
| **Pagination** | Use `limit` to restrict result set size |
| **Empty Query** | Empty `{}` matches all documents |

### Error Handling

Common error responses:

```json
{
  "status": "error",
  "message": "Invalid JSON format",
  "data": null
}
```

```json
{
  "status": "error",
  "message": "Unknown action: invalid_action",
  "data": null
}
```

```json
{
  "status": "error",
  "message": "Missing required field: collection",
  "data": null
}
```

---

## Testing

XDB includes a comprehensive test suite using a custom testing framework.

### Running Tests

```bash
# Run all unit tests
make test

# Run with verbose output
make test VERBOSE=1

# Run specific test
./bin/test_runner [test_name]
```

### Test Coverage

| Module | Tests | Focus |
|--------|-------|-------|
| **CRUD Operations** | `test_crud.c` | Insert, Find, Delete, Count |
| **Query Engine** | `test_query.c` | Exact match, filtering, pagination |
| **Core Functionality** | `main_test.c` | Integration tests |

### Writing New Tests

Create a test file in `tests/`:

```c
#include "framework.h"
#include "../include/database.h"

TEST_START(test_feature) {
    database_t *db = db_init();
    ASSERT_EQ(some_function(db), expected_value);
    db_cleanup(db);
}
TEST_END
```

Register in `tests/main_test.c`:

```c
extern void test_feature();

int main(void) {
    REGISTER_TEST(test_feature);
    return 0;
}
```

Rebuild: `make clean test`

---

## Architecture

### Design Principles

| Principle | Implementation |
|-----------|-----------------|
| **Modularity** | Each component has a single responsibility with well-defined interface |
| **Thread Safety** | All shared resources protected by POSIX mutexes |
| **Memory Safety** | Careful allocation and deallocation to prevent leaks |
| **Clean Code** | Consistent naming conventions and clear function contracts |
| **Error Handling** | Graceful error responses and proper resource cleanup |

### Core Components

#### Database Module (`src/database.c`, `include/database.h`)

Manages document storage and persistence.

**Responsibilities:**
- CRUD operations (Create, Read, Update, Delete)
- Disk I/O and data persistence
- Collection management
- JSON serialization

**Key Functions:**
```c
database_t* db_init(void);
void db_cleanup(database_t *db);
void db_insert(database_t *db, ...);
void db_find(database_t *db, ...);
void db_delete(database_t *db, ...);
int db_count(database_t *db, ...);
```

#### Query Engine (`src/query.c`, `include/query.h`)

Implements document filtering and matching logic.

**Semantics:**
- Exact match on all query fields
- No partial matching or regex support
- Type-safe comparisons
- O(n) linear scan through collection

#### Server Module (`src/server.c`, `include/server.h`)

Handles TCP connections and protocol parsing.

**Features:**
- Socket creation and listener management
- JSON protocol parsing
- Per-client worker threads
- Graceful shutdown handling

**Default Configuration:**
- Listens on `0.0.0.0:8080` (fixed port)
- One thread per client connection
- UTF-8 encoding for all messages

#### Utilities Module (`src/utils.c`, `include/utils.h`)

Provides common functionality across modules.

| Function | Purpose |
|----------|---------|
| `generate_uuid()` | 16-character random unique identifier |
| `log_info()` | Timestamped console logging |
| `json_to_string()` | cJSON wrapper for serialization |
| `string_to_json()` | cJSON wrapper for parsing |
| `safe_malloc()` | Memory allocation with error checking |

### Architecture Diagram

```
┌─────────────────────────┐
│  Client (TCP Port 8080) │
└────────────┬────────────┘
             ↓
┌─────────────────────────────────────────────────────┐
│             Server Module (server.c)                │
│  - Listen on 0.0.0.0:8080 (fixed)                   │
│  - Accept client connections                        │
│  - Spawn worker threads                             │
└──────────────────────┬──────────────────────────────┘
         ┌─────────────┼─────────────┐
         ↓             ↓             ↓
    ┌──────────┐  ┌──────────┐  ┌──────────┐
    │ Worker   │  │ Worker   │  │ Worker   │
    │ Thread 1 │  │ Thread 2 │  │ Thread 3 │
    └────┬─────┘  └────┬─────┘  └────┬─────┘
         └─────────────┼─────────────┘
                       ↓
        ┌──────────────────────────────┐
        │   JSON Protocol Handler      │
        │  (Parse & Route Commands)    │
        └──────────────┬───────────────┘
     ┌──────────────┬──┴───────┬───────────────┐
     ↓              ↓          ↓               ↓
┌──────────┐  ┌──────────┐ ┌─────────┐  ┌─────────────┐
│ Database │  │  Query   │ │Utilities│  │ Persistence │
│ (CRUD)   │  │ (Filter) │ │(UUID)   │  │ (JSON)      │
└────┬─────┘  └────┬─────┘ └─────────┘  └─────────────┘
     └─────────┬───┘
               ↓
       ┌─────────────────┐
       │ data/           │
       │ production.json │
       └─────────────────┘
```

---

## Troubleshooting

### Build Issues

**Problem:** `cJSON not found`

```bash
# Ensure submodules are initialized
git submodule update --init --recursive
make clean && make
```

**Problem:** Compiler not found

```bash
# Linux
sudo apt-get install build-essential

# macOS
brew install gcc
```

### Runtime Issues

**Problem:** `Address already in use` on port 8080

```bash
# Find what's using the port
lsof -i :8080

# Stop the existing process
kill -9 <PID>
```

**Problem:** `Permission denied`

```bash
chmod +x bin/xdb
```

**Problem:** `Connection refused`

```bash
# Verify server is running
ps aux | grep xdb

# Check port is listening
netstat -tulpn | grep 8080
```

**Problem:** Segmentation fault

```bash
# Rebuild with debug symbols
make clean DEBUG=1

# Run under GDB
gdb ./bin/xdb
```

### Testing Issues

**Problem:** Tests fail

```bash
# Run with verbose output
make test VERBOSE=1

# Check dependencies
gcc --version && make --version
```

---

## Performance

### Benchmarks

| Operation | Latency | Throughput |
|-----------|---------|-----------|
| **Insert** | 1-5ms | 200-500 ops/sec |
| **Find** (100 docs) | 2-10ms | 100-300 ops/sec |
| **Count** (100 docs) | 1-5ms | 200-500 ops/sec |
| **Delete** | 1-5ms | 200-500 ops/sec |

*Note: Results depend on hardware and data size.*

### Scalability

- **In-Memory Storage**: Entire database kept in RAM for speed
- **Collection Size**: Optimal for < 100,000 documents
- **Concurrent Clients**: Tested up to 1000 simultaneous connections
- **Query Performance**: Linear O(n) scan - scales with collection size

### Optimization Tips

1. Batch operations when possible
2. Use specific query filters to reduce dataset size
3. Reuse connections when possible
4. Use `limit` parameter for large result sets

---

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for:

- Code style guidelines
- Development setup
- Testing requirements
- Pull request process
- Issue reporting

**Quick Start:**
```bash
git clone https://github.com/xdb-project/xdb.git
cd xdb
git submodule update --init --recursive
make clean test
```

---

## Project Structure

```
xdb/
├── .clang-format           # Code formatting configuration
├── .editorconfig           # Editor settings
├── .gitignore              # Git ignore rules
├── .gitmodules             # Git submodule configuration
├── .vscode/                # VS Code workspace settings
├── .github/                # GitHub configurations
│   ├── PULL_REQUEST_TEMPLATE.md    # PR template
│   └── ISSUE_TEMPLATE/             # Issue templates
│       ├── bug_report.md           # Bug report template
│       ├── feature_request.md      # Feature request template
│       └── support_request.md      # Support request template
├── bin/                    # Compiled executables
│   ├── xdb                 # Main server executable
│   └── test_runner         # Test suite executable
├── data/                   # Persistent storage (auto-created)
├── include/                # Public API headers
│   ├── database.h          # Storage engine interface
│   ├── query.h             # Query matching interface
│   ├── server.h            # TCP server interface
│   └── utils.h             # Utility functions interface
├── src/                    # Implementation
│   ├── main.c              # Application entry point
│   ├── database.c          # CRUD implementation
│   ├── query.c             # Query engine implementation
│   ├── server.c            # TCP server implementation
│   └── utils.c             # Utility functions
├── tests/                  # Test suite
│   ├── framework.h         # Custom test framework
│   ├── main_test.c         # Test runner
│   ├── test_crud.c         # CRUD operation tests
│   └── test_query.c        # Query engine tests
├── third_party/            # External dependencies
│   └── cJSON/              # JSON parser library (git submodule)
├── AUTHORS.md              # Project authors
├── CHANGELOG.md            # Version history
├── CODE_OF_CONDUCT.md      # Community standards
├── CONTRIBUTING.md         # Contribution guidelines
├── CONTRIBUTORS.md         # List of all contributors
├── LICENSE                 # MIT License
├── Makefile                # Build configuration
├── README.md               # Project documentation
└── SECURITY.md             # Security policy
```

---

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) for details.

**Permitted:** Commercial use, modification, distribution, private use

**Required:** License and copyright notice

---

## Additional Resources

### Documentation
- [CONTRIBUTING.md](CONTRIBUTING.md) - How to contribute
- [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) - Community standards
- [AUTHORS.md](AUTHORS.md) - Project authors
- [CHANGELOG.md](CHANGELOG.md) - Version history

### External References
- [cJSON](https://github.com/DaveGamble/cJSON) - JSON library documentation
- [POSIX Threads](https://pubs.opengroup.org/onlinepubs/9699919799/) - pthread reference
- [TCP Socket Programming](https://man7.org/linux/man-pages/man2/socket.2.html) - Socket manual

---

<div align="center">

**Made with ❤️ by the XDB-Project Team**

[GitHub Repository](https://github.com/xdb-project/xdb) • [Report Issues](https://github.com/xdb-project/xdb/issues) • [Discussions](https://github.com/xdb-project/xdb/discussions)

XDB © 2026. MIT License.

</div>
