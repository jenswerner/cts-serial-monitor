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
    printf("  -i INTERVAL    Polling interval in microseconds (default: 1000)\n");
    printf("  -f FORMAT      Time format: abs|rel (default: abs)\n");
    printf("  -o FILE        Output file (default: stdout)\n");
    printf("\n");
    printf("Serial Device Examples:\n");
    printf("  /dev/ttyUSB0   USB serial adapter\n");
    printf("  /dev/ttyS0     Built-in serial port\n");
    printf("  /dev/ttyACM0   USB CDC device\n");
    printf("\n");
    printf("CTS Monitor - Monitor CTS/RTS signals on serial lines\n");
    printf("Outputs timestamped changes in CTS and RTS signal states.\n");
}

int main(int argc, char *argv[]) {
    int verbose = 0;
    int poll_interval_us = 1000;  // 1ms default
    char *serial_device = NULL;
    char *output_file = NULL;
    time_format_t time_format = TIME_FORMAT_ABSOLUTE;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
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
        .verbose = verbose
    };
    
    if (cts_monitor_init(&config) != 0) {
        fprintf(stderr, "Failed to initialize CTS monitor\n");
        return EXIT_FAILURE;
    }
    
    if (verbose) {
        printf("CTS Monitor starting...\n");
        printf("Serial device: %s\n", serial_device);
        printf("Poll interval: %d microseconds\n", poll_interval_us);
        printf("Time format: %s\n", time_format == TIME_FORMAT_ABSOLUTE ? "absolute" : "relative");
        printf("Output: %s\n", output_file ? output_file : "stdout");
        printf("\nMonitoring CTS/RTS signals (Ctrl+C to stop)...\n");
        printf("Format: [timestamp] SIGNAL: state\n\n");
    }
    
    // Main monitoring loop
    while (running) {
        if (cts_monitor_update() != 0) {
            fprintf(stderr, "Monitor update failed\n");
            break;
        }
        
        // Sleep for specified interval
        usleep(poll_interval_us);
    }
    
    // Cleanup
    cts_monitor_cleanup();
    
    if (verbose) {
        printf("\nCTS Monitor shutdown complete\n");
    }
    
    return EXIT_SUCCESS;
}