#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "cts_monitor.h"

static volatile int running = 1;

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\nReceived signal %d, shutting down gracefully...\n", sig);
        running = 0;
    }
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options] <serial_device>\n", program_name);
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --verbose  Enable verbose output\n");
    printf("  -m MODE        Monitoring mode: poll|irq (default: poll)\n");
    printf("  -i INTERVAL    Polling interval in microseconds (default: 1000, poll mode only)\n");
    printf("  -f FORMAT      Time format: abs|rel (default: abs)\n");
    printf("  -o FILE        Output file (default: stdout)\n");
    printf("\n");
    printf("Monitoring Modes:\n");
    printf("  poll           Polling-based monitoring (configurable interval)\n");
    printf("  irq            Event-driven monitoring using select() system call\n");
    printf("\n");
    printf("Serial Device Examples:\n");
    printf("  /dev/ttyUSB0   USB serial adapter (FTDI auto-detected)\n");
    printf("  /dev/ttyS0     Built-in serial port\n");
    printf("  /dev/ttyACM0   USB CDC device\n");
    printf("\n");
#ifdef HAVE_LIBFTDI1
    printf("CTS Monitor v1.2.0 - Monitor CTS/RTS signals on serial lines\n");
    printf("Built with libftdi1 support for enhanced FTDI device monitoring.\n");
#else
    printf("CTS Monitor v1.2.0 - Monitor CTS/RTS signals on serial lines\n");
    printf("Build with libftdi1-dev for enhanced FTDI device support.\n");
#endif
    printf("Outputs timestamped changes in CTS and RTS signal states.\n");
}

int main(int argc, char *argv[]) {
    int verbose = 0;
    int poll_interval_us = 1000;  // 1ms default
    char *serial_device = NULL;
    char *output_file = NULL;
    time_format_t time_format = TIME_FORMAT_ABSOLUTE;
    monitor_mode_t monitor_mode = MONITOR_MODE_POLLING;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        }
        else if (strcmp(argv[i], "-m") == 0) {
            if (i + 1 < argc) {
                char *mode = argv[++i];
                if (strcmp(mode, "poll") == 0) {
                    monitor_mode = MONITOR_MODE_POLLING;
                } else if (strcmp(mode, "irq") == 0) {
                    monitor_mode = MONITOR_MODE_IRQ;
                } else {
                    fprintf(stderr, "Error: Invalid monitor mode %s (use 'poll' or 'irq')\n", mode);
                    return EXIT_FAILURE;
                }
            } else {
                fprintf(stderr, "Error: -m option requires a mode (poll|irq)\n");
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 < argc) {
                poll_interval_us = atoi(argv[++i]);
                if (poll_interval_us < 100) {
                    fprintf(stderr, "Error: Minimum polling interval is 100 microseconds\n");
                    return EXIT_FAILURE;
                }
            } else {
                fprintf(stderr, "Error: -i option requires an interval in microseconds\n");
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) {
                char *format = argv[++i];
                if (strcmp(format, "abs") == 0) {
                    time_format = TIME_FORMAT_ABSOLUTE;
                } else if (strcmp(format, "rel") == 0) {
                    time_format = TIME_FORMAT_RELATIVE;
                } else {
                    fprintf(stderr, "Error: Invalid time format %s (use 'abs' or 'rel')\n", format);
                    return EXIT_FAILURE;
                }
            } else {
                fprintf(stderr, "Error: -f option requires a format (abs|rel)\n");
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                fprintf(stderr, "Error: -o option requires an output file\n");
                return EXIT_FAILURE;
            }
        }
        else if (argv[i][0] != '-') {
            // This should be the serial device
            if (serial_device == NULL) {
                serial_device = argv[i];
            } else {
                fprintf(stderr, "Error: Multiple serial devices specified\n");
                return EXIT_FAILURE;
            }
        }
        else {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    
    // Check if serial device was provided
    if (serial_device == NULL) {
        fprintf(stderr, "Error: Serial device must be specified\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize monitor
    monitor_config_t config = {
        .serial_device = serial_device,
        .poll_interval_us = poll_interval_us,
        .time_format = time_format,
        .output_file = output_file,
        .verbose = verbose,
        .mode = monitor_mode,
        .device_type = DEVICE_TYPE_STANDARD  // Auto-detected during init
    };
    
    if (cts_monitor_init(&config) != 0) {
        fprintf(stderr, "Failed to initialize CTS monitor\n");
        return EXIT_FAILURE;
    }
    
    if (verbose) {
        printf("CTS Monitor v1.2.0 starting...\n");
        printf("Serial device: %s\n", serial_device);
        printf("Monitor mode: %s\n", monitor_mode == MONITOR_MODE_IRQ ? "Event-driven (select)" : "Standard polling");
        if (monitor_mode == MONITOR_MODE_POLLING) {
            printf("Poll interval: %d microseconds\n", poll_interval_us);
        }
        printf("Time format: %s\n", time_format == TIME_FORMAT_ABSOLUTE ? "absolute" : "relative");
        printf("Output: %s\n", output_file ? output_file : "stdout");
#ifdef HAVE_LIBFTDI1
        printf("FTDI support: Available\n");
#endif
        printf("\nMonitoring CTS/RTS signals (Ctrl+C to stop)...\n");
        printf("Format: [timestamp] SIGNAL: state\n\n");
    }
    
    // Start IRQ mode if configured
    if (monitor_mode == MONITOR_MODE_IRQ) {
        if (cts_monitor_start_irq() != 0) {
            fprintf(stderr, "Failed to start IRQ-driven monitoring\n");
            cts_monitor_cleanup();
            return EXIT_FAILURE;
        }
    }
    
    // Main monitoring loop
    while (running) {
        if (monitor_mode == MONITOR_MODE_POLLING) {
            // Polling mode: regular updates
            if (cts_monitor_update() != 0) {
                fprintf(stderr, "Monitor update failed\n");
                break;
            }
            // Sleep for specified interval
            usleep(poll_interval_us);
        } else {
            // IRQ mode: event-driven monitoring with select()
            int events = cts_monitor_process_irq_events();
            if (events < 0) {
                fprintf(stderr, "IRQ event processing failed\n");
                break;
            }
            // No sleep needed - select() provides the blocking/timing
        }
    }
    
    // Cleanup
    cts_monitor_cleanup();
    
    if (verbose) {
        printf("\nCTS Monitor shutdown complete\n");
    }
    
    return EXIT_SUCCESS;
}