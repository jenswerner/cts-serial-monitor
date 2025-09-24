# CTS Monitor

A high-precision serial signal monitoring application for Linux that tracks CTS (Clear To Send) and RTS (Request To Send) control signals on serial ports with microsecond-accurate timestamping.

## Features

- **Real-time Signal Monitoring**: Monitor CTS and RTS signal state changes
- **High-Precision Timestamps**: Microsecond-accurate timing using `clock_gettime()`
- **Multiple Time Formats**: Absolute timestamps or relative timing from start
- **Configurable Polling**: Adjustable polling interval from 100 microseconds up
- **Flexible Output**: Console output or log to file
- **Additional Signals**: Optional monitoring of DSR/DTR in verbose mode
- **Cross-platform**: Works with any Linux-supported serial device

## Use Cases

- **Serial Communication Debugging**: Monitor flow control signals during data transmission
- **Hardware Testing**: Verify proper CTS/RTS signaling in custom hardware
- **Protocol Analysis**: Time critical signal changes in serial protocols
- **Embedded Development**: Debug serial communication issues
- **Industrial Automation**: Monitor control signals in industrial serial networks

## Quick Start

### Prerequisites

- Linux system with serial port support
- GCC compiler and Make
- Access to serial device (may require appropriate permissions)

### Building

```bash
git clone <repository-url>
cd cts_monitor
make
```

### Basic Usage

```bash
# Monitor CTS/RTS on USB serial adapter
./cts_monitor /dev/ttyUSB0

# With verbose output and custom polling interval
./cts_monitor -v -i 500 /dev/ttyUSB0

# Log to file with relative timestamps
./cts_monitor -f rel -o signals.log /dev/ttyS0
```

## Command Line Options

```
Usage: cts_monitor [options] <serial_device>

Options:
  -h, --help     Show help message
  -v, --verbose  Enable verbose output (includes DSR/DTR)
  -i INTERVAL    Polling interval in microseconds (default: 1000)
  -f FORMAT      Time format: abs|rel (default: abs)
  -o FILE        Output file (default: stdout)

Serial Device Examples:
  /dev/ttyUSB0   USB serial adapter
  /dev/ttyS0     Built-in serial port
  /dev/ttyACM0   USB CDC device
```

## Output Format

The monitor outputs timestamped signal changes in the following format:

### Absolute Time Format (default)
```
[2025-09-24 14:30:15.123456] === CTS Monitor Started ===
[2025-09-24 14:30:15.123461] Initial state - CTS: LOW, RTS: HIGH
[2025-09-24 14:30:16.456789] CTS: HIGH ↑
[2025-09-24 14:30:17.234567] RTS: LOW ↓
[2025-09-24 14:30:18.345678] CTS: LOW ↓
```

### Relative Time Format (-f rel)
```
[0.000000] === CTS Monitor Started ===
[0.000005] Initial state - CTS: LOW, RTS: HIGH
[1.333333] CTS: HIGH ↑
[2.111111] RTS: LOW ↓
[3.222222] CTS: LOW ↓
```

## Project Structure

```
cts_monitor/
├── src/
│   ├── main.c              # Main application and argument parsing
│   └── cts_monitor.c       # Core signal monitoring logic
├── include/
│   └── cts_monitor.h       # Header with data structures and API
├── build/                  # Build artifacts (created during build)
├── .vscode/               # VS Code configuration
├── Makefile               # Build configuration
└── README.md              # This file
```

## Technical Details

### Signal Detection
- Uses `ioctl(TIOCMGET)` to read serial port modem control lines
- Monitors CTS, RTS, and optionally DSR, DTR signals
- Non-blocking operation to minimize latency

### Timing Precision
- Uses `clock_gettime(CLOCK_REALTIME)` for microsecond-accurate timestamps
- Configurable polling intervals from 100μs to any reasonable value
- Minimal overhead for high-frequency monitoring

### Serial Port Handling
- Opens serial device with minimal configuration
- Only requires access to control signals (no data transmission)
- Supports any Linux-compatible serial device

## Performance Characteristics

- **Latency**: Sub-millisecond signal change detection (depends on polling interval)
- **Precision**: Microsecond timestamp resolution
- **CPU Usage**: <1% CPU at 1ms polling intervals
- **Memory**: <1MB resident memory usage

## Common Serial Devices

| Device Path | Description |
|-------------|-------------|
| `/dev/ttyUSB0` | USB to serial adapter (FTDI, Prolific, etc.) |
| `/dev/ttyS0` | Built-in serial port (COM1 equivalent) |
| `/dev/ttyACM0` | USB CDC ACM device |
| `/dev/ttyAMA0` | ARM serial port (Raspberry Pi) |

## Usage Examples

### Debug Serial Communication
Monitor flow control during file transfer:
```bash
./cts_monitor -v -i 100 /dev/ttyUSB0 > transfer_signals.log &
# Run your serial communication
# Stop monitoring with Ctrl+C
```

### Test Hardware Design
Verify signal timing in custom hardware:
```bash
./cts_monitor -f rel -i 50 /dev/ttyUSB1
```

### Long-term Monitoring
Monitor signals over extended periods:
```bash
./cts_monitor -o daily_signals.log /dev/ttyS0 &
```

## Troubleshooting

### Common Issues

1. **Permission Denied**: Add user to `dialout` group
   ```bash
   sudo usermod -a -G dialout $USER
   # Log out and back in
   ```

2. **Device Busy**: Another application is using the serial port
   ```bash
   lsof /dev/ttyUSB0  # Check what's using the port
   ```

3. **No Signal Changes**: Verify hardware connections and signal levels

### Debugging

Run with verbose output to see additional information:
```bash
./cts_monitor -v /dev/ttyUSB0
```

Check available serial devices:
```bash
ls -la /dev/tty*
```

## Building and Development

### Build Targets
```bash
make          # Debug build
make release  # Optimized build
make clean    # Clean build files
make help     # Show all targets
```

### Development Tools
```bash
make analyze    # Static code analysis
make memcheck   # Memory leak check
make format     # Code formatting
```

## Signal Reference

| Signal | Description | Direction |
|--------|-------------|-----------|
| **CTS** | Clear To Send | From DCE to DTE |
| **RTS** | Request To Send | From DTE to DCE |
| **DSR** | Data Set Ready | From DCE to DTE |
| **DTR** | Data Terminal Ready | From DTE to DCE |

- **DTE**: Data Terminal Equipment (computer)
- **DCE**: Data Communications Equipment (modem)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Follow the existing code style
4. Test with various serial devices
5. Submit a pull request

## License

[Add your license information here]

## Changelog

### Version 2.0.0 (Serial Signal Monitor)
- Complete rewrite for CTS/RTS signal monitoring
- High-precision microsecond timestamps
- Configurable polling intervals
- Multiple output formats
- Real-time signal change detection

---

**Author**: [Your Name]  
**Created**: September 2025  
**Last Updated**: September 2025

## Quick Start

### Prerequisites

- GCC compiler
- Make build system
- Linux operating system
- Standard C library and POSIX headers

### Building

```bash
# Clone and build
git clone <repository-url>
cd cts_monitor
make

# Or build in release mode
make release
```

### Running

```bash
# Basic usage
./cts_monitor

# With verbose output
./cts_monitor -v

# As daemon
./cts_monitor -d

# Custom port and config
./cts_monitor -p 9090 -c config.txt -v
```

### Web Interface

Once running, visit `http://localhost:8080` (or your configured port) to view metrics via HTTP.

## Command Line Options

```
Usage: cts_monitor [options]
Options:
  -h, --help     Show help message
  -v, --verbose  Enable verbose output
  -d, --daemon   Run as daemon
  -c CONFIG      Specify configuration file
  -p PORT        Specify port number (default: 8080)
```

## Project Structure

```
cts_monitor/
├── src/                    # Source files
│   ├── main.c             # Main application entry point
│   ├── cts_monitor.c      # Core monitoring logic
│   ├── system_monitor.c   # System metrics collection
│   └── network_monitor.c  # Network metrics and HTTP server
├── include/               # Header files
│   ├── cts_monitor.h      # Main header
│   ├── system_monitor.h   # System monitoring interface
│   └── network_monitor.h  # Network monitoring interface
├── build/                 # Build artifacts (created during build)
├── tests/                 # Unit tests (future)
├── docs/                  # Documentation
├── Makefile              # Build configuration
└── README.md             # This file
```

## Build System

The project uses Make with support for debug and release builds:

```bash
# Debug build (default)
make debug

# Release build (optimized)
make release

# Clean build artifacts
make clean

# Install system-wide
sudo make install

# Show all available targets
make help
```

## Configuration

Create a configuration file with key=value pairs:

```ini
# config.txt
port=8080
verbose=1
daemon=0
```

Use with: `./cts_monitor -c config.txt`

## System Metrics

The monitor collects:

- **CPU**: Usage percentage, load averages (1, 5, 15 min)
- **Memory**: Total, used, free RAM with usage percentages
- **Disk**: Total, used, free disk space for root filesystem
- **Network**: RX/TX bytes, packets, current transfer rates
- **System**: Uptime, process count, timestamp

## HTTP API

Simple HTTP interface provides metrics in plain text format:

```bash
curl http://localhost:8080
```

Response format:
```
CTS Monitor Network Stats
RX: 1234567 bytes (5678 packets) [1024 B/s]
TX: 987654 bytes (4321 packets) [512 B/s]
Timestamp: 1640995200
```

## Development

### Code Style

- Follow C99 standard
- Use consistent indentation (4 spaces)
- Document functions with Doxygen-style comments
- Handle errors gracefully

### Building with Development Tools

```bash
# Static analysis
make analyze

# Memory checking
make memcheck

# Code formatting
make format

# Generate documentation
make docs
```

### Adding New Features

1. Add function declarations to appropriate header files
2. Implement functions in corresponding source files
3. Update Makefile if new source files are added
4. Test thoroughly with debug build
5. Update documentation

## Monitoring Architecture

```
┌─────────────────┐
│   Main Loop     │
│   (main.c)      │
└─────────┬───────┘
          │
          v
┌─────────────────┐    ┌─────────────────┐
│ System Monitor  │    │ Network Monitor │
│ (system_monitor)│    │ (network_monitor)│
│                 │    │                 │
│ • CPU Usage     │    │ • Interface Stats│
│ • Memory Info   │    │ • Traffic Rates │
│ • Disk Usage    │    │ • HTTP Server   │
│ • Load Average  │    │ • Connections   │
└─────────────────┘    └─────────────────┘
```

## Dependencies

### Required
- GCC (or compatible C compiler)
- GNU Make
- Linux kernel 2.6+ (for /proc filesystem)
- POSIX-compliant system

### Optional (for development)
- Valgrind (memory checking)
- Cppcheck (static analysis)
- Clang-format (code formatting)
- Doxygen (documentation generation)

## Performance

- **Memory**: ~1-2 MB resident memory
- **CPU**: <1% CPU usage in normal operation
- **Network**: Minimal overhead, non-blocking I/O
- **Updates**: 1 second interval (configurable)

## Security Considerations

- HTTP interface binds to all interfaces (0.0.0.0)
- No authentication on monitoring interface
- Daemon mode drops to background process
- File permissions should be set appropriately for config files

## Troubleshooting

### Common Issues

1. **Port already in use**: Choose different port with `-p` option
2. **Permission denied**: Run with appropriate privileges for system monitoring
3. **Build errors**: Ensure GCC and development headers are installed

### Debug Mode

Run with verbose output for debugging:
```bash
./cts_monitor -v
```

### System Requirements

- Linux 2.6+ (uses /proc filesystem)
- POSIX.1-2008 compatible system
- Network support for HTTP interface

## Contributing

1. Fork the repository
2. Create a feature branch
3. Follow coding standards
4. Add tests for new functionality
5. Submit a pull request

## License

[Add your license information here]

## Changelog

### Version 1.0.0 (Initial Release)
- Basic system monitoring (CPU, memory, disk)
- Network interface monitoring
- HTTP metrics interface
- Daemon mode support
- Configuration file support
- Command-line interface

---

**Author**: [Your Name]  
**Created**: September 2025  
**Last Updated**: September 2025