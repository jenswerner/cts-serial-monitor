#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include "cts_monitor.h"

static int initialized = 0;
static int serial_fd = -1;
static FILE *output_fp = NULL;
static monitor_config_t current_config;
static signal_state_t last_state;
static struct timespec start_time;
static int irq_mode_active = 0;

// Get high-precision timestamp
static void get_timestamp(char *buffer, size_t size) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    if (current_config.time_format == TIME_FORMAT_ABSOLUTE) {
        // Absolute time with microsecond precision
        struct tm *tm_info = localtime(&ts.tv_sec);
        size_t len = strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
        snprintf(buffer + len, size - len, ".%06ld", ts.tv_nsec / 1000);
    } else {
        // Relative time from start in microseconds
        struct timespec diff;
        diff.tv_sec = ts.tv_sec - start_time.tv_sec;
        diff.tv_nsec = ts.tv_nsec - start_time.tv_nsec;
        
        if (diff.tv_nsec < 0) {
            diff.tv_sec--;
            diff.tv_nsec += 1000000000L;
        }
        
        long long total_us = diff.tv_sec * 1000000LL + diff.tv_nsec / 1000;
        snprintf(buffer, size, "%lld.%06lld", total_us / 1000000, total_us % 1000000);
    }
}

// Read current CTS/RTS state
static int read_signal_state(signal_state_t *state) {
    int status;
    
    if (ioctl(serial_fd, TIOCMGET, &status) < 0) {
        if (current_config.verbose) {
            fprintf(stderr, "Error reading serial port status: %s\n", strerror(errno));
        }
        return -1;
    }
    
    state->cts = (status & TIOCM_CTS) ? 1 : 0;
    state->rts = (status & TIOCM_RTS) ? 1 : 0;
    state->dsr = (status & TIOCM_DSR) ? 1 : 0;  // Also read DSR for completeness
    state->dtr = (status & TIOCM_DTR) ? 1 : 0;  // Also read DTR for completeness
    
    return 0;
}

// Log signal change
static void log_signal_change(const char *signal_name, int old_state, int new_state) {
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    
    const char *state_str = new_state ? "HIGH" : "LOW";
    const char *transition = (old_state < new_state) ? "↑" : "↓";
    
    fprintf(output_fp, "[%s] %s: %s %s\n", timestamp, signal_name, state_str, transition);
    fflush(output_fp);
    
    if (current_config.verbose && output_fp != stdout) {
        printf("[%s] %s: %s %s\n", timestamp, signal_name, state_str, transition);
    }
}

// Setup high-frequency polling for IRQ mode (more reliable than SIGIO)
static int setup_signal_io(void) {
    // Note: True hardware interrupt-driven CTS/RTS detection is not reliably
    // supported across all Linux serial drivers and hardware combinations.
    // Instead, we use very high-frequency polling (10μs intervals) to achieve
    // near-IRQ performance while maintaining hardware compatibility.
    
    if (current_config.verbose) {
        printf("IRQ-mode: Using high-frequency polling (10μs) for reliable CTS/RTS detection\n");
        printf("Note: True hardware interrupts for modem signals are not universally supported\n");
    }
    
    irq_mode_active = 1;
    return 0;
}

// Cleanup signal-driven I/O
static void cleanup_signal_io(void) {
    irq_mode_active = 0;
    
    if (current_config.verbose) {
        printf("High-frequency polling disabled\n");
    }
}

int cts_monitor_init(const monitor_config_t *config) {
    if (initialized) {
        if (config->verbose) printf("Monitor already initialized\n");
        return 0;
    }
    
    // Copy configuration
    current_config = *config;
    
    // Record start time for relative timestamps
    clock_gettime(CLOCK_REALTIME, &start_time);
    
    if (config->verbose) {
        printf("Initializing CTS Monitor...\n");
        printf("Opening serial device: %s\n", config->serial_device);
    }
    
    // Open serial device
    serial_fd = open(config->serial_device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_fd < 0) {
        fprintf(stderr, "Error opening serial device %s: %s\n", 
                config->serial_device, strerror(errno));
        return -1;
    }
    
    // Configure serial port (minimal configuration, just for control signals)
    struct termios tty;
    if (tcgetattr(serial_fd, &tty) < 0) {
        fprintf(stderr, "Error getting serial port attributes: %s\n", strerror(errno));
        close(serial_fd);
        return -1;
    }
    
    // Set minimal configuration - we only care about control signals
    cfmakeraw(&tty);
    tty.c_cflag |= CLOCAL;  // Ignore modem control lines
    tty.c_cflag &= ~CRTSCTS;  // Disable hardware flow control initially
    
    if (tcsetattr(serial_fd, TCSANOW, &tty) < 0) {
        fprintf(stderr, "Error setting serial port attributes: %s\n", strerror(errno));
        close(serial_fd);
        return -1;
    }
    
    // Open output file if specified
    if (config->output_file) {
        output_fp = fopen(config->output_file, "w");
        if (!output_fp) {
            fprintf(stderr, "Error opening output file %s: %s\n", 
                    config->output_file, strerror(errno));
            close(serial_fd);
            return -1;
        }
    } else {
        output_fp = stdout;
    }
    
    // Read initial state
    if (read_signal_state(&last_state) < 0) {
        fprintf(stderr, "Failed to read initial signal state\n");
        if (output_fp != stdout) fclose(output_fp);
        close(serial_fd);
        return -1;
    }
    
    // Log initial state
    if (config->verbose) {
        char timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(output_fp, "[%s] === CTS Monitor Started ===\n", timestamp);
        fprintf(output_fp, "[%s] Initial state - CTS: %s, RTS: %s\n", 
                timestamp,
                last_state.cts ? "HIGH" : "LOW",
                last_state.rts ? "HIGH" : "LOW");
        fflush(output_fp);
    }
    
    initialized = 1;
    
    if (config->verbose) {
        printf("CTS Monitor initialized successfully\n");
        printf("Initial CTS: %s, RTS: %s\n", 
               last_state.cts ? "HIGH" : "LOW", 
               last_state.rts ? "HIGH" : "LOW");
    }
    
    return 0;
}

int cts_monitor_update() {
    if (!initialized) {
        fprintf(stderr, "Monitor not initialized\n");
        return -1;
    }
    
    signal_state_t current_state;
    
    // Read current signal state
    if (read_signal_state(&current_state) < 0) {
        return -1;
    }
    
    // Check for changes and log them
    if (current_state.cts != last_state.cts) {
        log_signal_change("CTS", last_state.cts, current_state.cts);
    }
    
    if (current_state.rts != last_state.rts) {
        log_signal_change("RTS", last_state.rts, current_state.rts);
    }
    
    // Also monitor DSR/DTR if verbose mode (optional)
    if (current_config.verbose) {
        if (current_state.dsr != last_state.dsr) {
            log_signal_change("DSR", last_state.dsr, current_state.dsr);
        }
        
        if (current_state.dtr != last_state.dtr) {
            log_signal_change("DTR", last_state.dtr, current_state.dtr);
        }
    }
    
    // Update last known state
    last_state = current_state;
    
    return 0;
}

void cts_monitor_cleanup() {
    if (!initialized) {
        return;
    }
    
    // Stop IRQ mode if active
    if (irq_mode_active) {
        cts_monitor_stop_irq();
    }
    
    if (current_config.verbose) {
        char timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(output_fp, "[%s] === CTS Monitor Stopped ===\n", timestamp);
        printf("Cleaning up CTS Monitor...\n");
    }
    
    if (serial_fd >= 0) {
        close(serial_fd);
        serial_fd = -1;
    }
    
    if (output_fp && output_fp != stdout) {
        fclose(output_fp);
        output_fp = NULL;
    }
    
    initialized = 0;
    
    if (current_config.verbose) {
        printf("CTS Monitor cleanup complete\n");
    }
}

// Utility function to get current signal state
int cts_monitor_get_state(signal_state_t *state) {
    if (!initialized) {
        return -1;
    }
    
    return read_signal_state(state);
}

// Start IRQ-driven monitoring
int cts_monitor_start_irq(void) {
    if (!initialized) {
        fprintf(stderr, "Monitor not initialized\n");
        return -1;
    }
    
    if (current_config.mode != MONITOR_MODE_IRQ) {
        if (current_config.verbose) {
            printf("Not configured for IRQ mode\n");
        }
        return -1;
    }
    
    if (irq_mode_active) {
        if (current_config.verbose) {
            printf("IRQ mode already active\n");
        }
        return 0;
    }
    
    if (setup_signal_io() < 0) {
        return -1;
    }
    
    if (current_config.verbose) {
        printf("High-frequency polling mode started (10μs intervals)\n");
    }
    
    return 0;
}

// Stop IRQ-driven monitoring
int cts_monitor_stop_irq(void) {
    if (!irq_mode_active) {
        return 0;
    }
    
    cleanup_signal_io();
    
    if (current_config.verbose) {
        printf("High-frequency polling mode stopped\n");
    }
    
    return 0;
}

// Process pending IRQ events (now uses continuous high-frequency polling)
int cts_monitor_process_irq_events(void) {
    if (!initialized || !irq_mode_active) {
        return -1;
    }
    
    // In IRQ mode, we continuously check for changes at high frequency
    // This provides much better performance than the default polling mode
    signal_state_t current_state;
    if (read_signal_state(&current_state) < 0) {
        return -1;
    }
    
    int events_processed = 0;
    
    // Check for changes and log them
    if (current_state.cts != last_state.cts) {
        log_signal_change("CTS", last_state.cts, current_state.cts);
        events_processed++;
    }
    
    if (current_state.rts != last_state.rts) {
        log_signal_change("RTS", last_state.rts, current_state.rts);
        events_processed++;
    }
    
    // Also monitor DSR/DTR if verbose mode (optional)
    if (current_config.verbose) {
        if (current_state.dsr != last_state.dsr) {
            log_signal_change("DSR", last_state.dsr, current_state.dsr);
            events_processed++;
        }
        
        if (current_state.dtr != last_state.dtr) {
            log_signal_change("DTR", last_state.dtr, current_state.dtr);
            events_processed++;
        }
    }
    
    // Update last known state
    last_state = current_state;
    
    return events_processed;
}