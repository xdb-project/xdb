# XDB

### *High-Performance Document-Oriented NoSQL Database*

XDB is a lightweight, thread-safe database engine written in **pure C99**. Designed for efficiency and reliability, it features high-concurrency client handling, robust JSON-based document storage, and persistent disk-based recovery.

<div align="center">

![C99](https://img.shields.io/badge/Language-C99-blue.svg?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-green.svg?style=flat-square)
![Build](https://img.shields.io/badge/Build-Passing-brightgreen.svg?style=flat-square)
![Version](https://img.shields.io/badge/Version-1.2.0-blue.svg?style=flat-square)
![POSIX](https://img.shields.io/badge/POSIX-Compliant-orange.svg?style=flat-square)

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
- Database Snapshotting Built-in mechanism for capturing point-in-time state snapshots to support secure backups and recover
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
| **Database Snapshotting** | Mechanism for capturing point-in-time state snapshots to support secure backups and recover |

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
```ini
# Create `/etc/systemd/system/xdb.service`:

[Unit]
Description=XDB High-Performance NoSQL Database Server
After=network.target

[Service]
Type=simple
User=xdb
WorkingDirectory=/opt/xdb
ExecStart=/opt/xdb/bin/xdb
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Set up a secure environment and permissions for the XDB service:

```bash
# Create a dedicated system user for XDB (no login shell for security)
sudo useradd -r -s /usr/bin/nologin xdb

# Set ownership to the xdb user and adjust directory permissions
sudo chown -R xdb:xdb /opt/xdb
sudo chmod -R 755 /opt/xdb

# Grant execution permissions to the XDB binary
sudo chmod +x /opt/xdb/bin/xdb
```

Then manage with:

```bash
# Reload systemd manager configuration
sudo systemctl daemon-reload

# Enable service to start on boot and start it now
sudo systemctl enable --now xdb

# Check current service status
sudo systemctl status xdb

# Monitor real-time logs
sudo journalctl -u xdb -f
```

**PM2 Process Manager**
```javascript
// Create `ecosystem.config.js` in your XDB root directory:

module.exports = {
  apps: [
    {
      name: 'xdb',
      script: './bin/xdb',
      instances: 1,
      exec_mode: 'fork',
      interpreter: 'none',
      
      env: {
        NODE_ENV: 'development',
        XDB_PORT: 8080,
        XDB_HOST: '0.0.0.0'
      },
      
      env_production: {
        NODE_ENV: 'production',
        XDB_PORT: 8080
      },
      
      restart_delay: 4000,
      max_memory_restart: '1G',
      error_file: './logs/pm2-error.log',
      out_file: './logs/pm2-out.log',
      kill_timeout: 5000,
      wait_ready: true,
      listen_timeout: 3000
    }
  ]
};
```

Install PM2 globally

```bash
npm install -g pm2

# or
pnpm add -g pm2
```

Start and manage XDB:

```bash
# Start XDB with PM2
pm2 start ecosystem.config.js

# Save configuration
pm2 save

# Enable auto-startup on system reboot
pm2 startup
pm2 save

# View running processes
pm2 list

# Monitor in real-time
pm2 monit

# View logs
pm2 logs xdb
pm2 logs xdb -f          # Real-time log streaming
pm2 logs xdb --lines 100 # Last 100 lines

# Manage processes
pm2 restart xdb  # Force restart
pm2 reload xdb   # Graceful restart (zero-downtime)
pm2 stop xdb     # Stop process
pm2 delete xdb   # Remove from PM2

# Check detailed information
pm2 info xdb
pm2 env xdb

# Disable auto-startup (if needed)
pm2 unstartup systemd
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

### 5. Manual Snapshot (Backup)

Triggers an immediate backup of the current database state into the data/ directory. This creates a "restore point" by copying the entire database into a new JSON file with a precise timestamp.

**Request:**

```json
{
  "action": "snapshot"
}
```
**Response:**

```json
{
  "status": "ok",
  "message": "Snapshot created successfully",
  "data": null
```

---

### 6. Exit Connection

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
├── .clang-format-ignore    # Files/folders to be skipped by clang-format
├── .cppcheck-suppress      # Static analysis warnings to be ignored by cppcheck
├── .editorconfig           # Editor settings (indentation, charset, etc.)
├── .gitignore              # Git ignore rules for build artifacts and binaries
├── .gitmodules             # Git submodule configuration
├── .github/                # GitHub-specific configurations
│   ├── ISSUE_TEMPLATE/             # Templates for reporting issues
│   │   ├── bug_report.md           # Bug report template
│   │   ├── feature_request.md      # Feature request template
│   │   └── support_request.md      # Support request template
│   ├── workflows/                  # Automation directory (GitHub Actions)
│   │   └── ci.yml                  # Continuous Integration pipeline
│   └── PULL_REQUEST_TEMPLATE.md    # Template for new pull requests
├── .vscode/                # VS Code workspace settings (git-ignored)
├── bin/                    # Compiled executables (git-ignored, generated by make)
│   ├── test_runner         # Test suite executable
│   └── xdb                 # Main server executable
├── data/                   # Database storage directory
│   ├── .gitkeep            # Ensures directory tracking even if empty
│   ├── production.json     # Main production database file
│   └── test_db.json        # Database file for testing purposes
├── include/                # Public API headers
│   ├── database.h          # Storage engine interface
│   ├── query.h             # Query matching interface
│   ├── server.h            # TCP server interface
│   └── utils.h             # Utility functions interface
├── src/                    # Implementation source files
│   ├── main.c              # Application entry point
│   ├── database.c          # CRUD operations implementation
│   ├── query.c             # Query engine implementation
│   ├── server.c            # TCP server implementation
│   └── utils.c             # Shared utility functions
├── tests/                  # Unit and integration test suite
│   ├── framework.h         # Custom lightweight test framework
│   ├── main_test.c         # Test runner entry point
│   ├── test_crud.c         # CRUD operation unit tests
│   └── test_query.c        # Query engine unit tests
├── third_party/            # External dependencies
│   └── cJSON/              # JSON parser library (managed via git submodule)
├── AUTHORS.md              # Project creators and maintainers
├── CHANGELOG.md            # Detailed version history and updates
├── CODE_OF_CONDUCT.md      # Community standards
├── CONTRIBUTING.md         # Guidelines for contributing to the project
├── CONTRIBUTORS.md         # Acknowledgment of all code contributors
├── LICENSE                 # Project license (MIT)
├── Makefile                # Build system configuration
├── README.md               # Project documentation
└── SECURITY.md             # Security policy and reporting instructions
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
