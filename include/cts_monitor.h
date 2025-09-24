#ifndef CTS_MONITOR_H
#define CTS_MONITOR_H

/**
 * @file cts_monitor.h
 * @brief CTS/RTS Serial Signal Monitor
 * 
 * This application monitors CTS (Clear To Send) and RTS (Request To Send)
 * control signals on serial ports and outputs timestamped changes with
 * high precision timing.
 */

#include <time.h>

#ifdef HAVE_LIBFTDI1
#include <ftdi.h>
#endif

/**
 * @brief Device type enumeration
 */
typedef enum {
    DEVICE_TYPE_STANDARD,   /**< Standard serial device */
    DEVICE_TYPE_FTDI        /**< FTDI USB-to-serial device */
} device_type_t;

/**
 * @brief Time format options
 */
typedef enum {
    TIME_FORMAT_ABSOLUTE,   /**< Absolute timestamp (YYYY-MM-DD HH:MM:SS.uuuuuu) */
    TIME_FORMAT_RELATIVE    /**< Relative timestamp from start (seconds.microseconds) */
} time_format_t;

/**
 * @brief Signal state structure
 */
typedef struct {
    int cts;    /**< CTS (Clear To Send) state: 1 = HIGH, 0 = LOW */
    int rts;    /**< RTS (Request To Send) state: 1 = HIGH, 0 = LOW */
    int dsr;    /**< DSR (Data Set Ready) state: 1 = HIGH, 0 = LOW */
    int dtr;    /**< DTR (Data Terminal Ready) state: 1 = HIGH, 0 = LOW */
} signal_state_t;

/**
 * @brief Monitor mode options
 */
typedef enum {
    MONITOR_MODE_POLLING,   /**< Polling-based monitoring (configurable interval) */
    MONITOR_MODE_IRQ        /**< Event-driven monitoring using select() system call */
} monitor_mode_t;

/**
 * @brief Monitor configuration structure
 */
typedef struct {
    const char *serial_device;     /**< Serial device path (e.g., /dev/ttyUSB0) */
    int poll_interval_us;          /**< Polling interval in microseconds (polling mode only) */
    time_format_t time_format;     /**< Timestamp format */
    const char *output_file;       /**< Output file path (NULL for stdout) */
    int verbose;                   /**< Verbose mode flag */
    monitor_mode_t mode;           /**< Monitoring mode: polling or IRQ-driven */
    device_type_t device_type;     /**< Device type: standard or FTDI */
} monitor_config_t;

/**
 * @brief Initialize the CTS monitor
 * @param config Pointer to configuration structure
 * @return 0 on success, -1 on failure
 */
int cts_monitor_init(const monitor_config_t *config);

/**
 * @brief Start IRQ-driven monitoring (non-blocking)
 * @return 0 on success, -1 on failure
 */
int cts_monitor_start_irq(void);

/**
 * @brief Stop IRQ-driven monitoring
 * @return 0 on success, -1 on failure
 */
int cts_monitor_stop_irq(void);

/**
 * @brief Update monitor (check for signal changes)
 * @return 0 on success, -1 on failure
 */
int cts_monitor_update(void);

/**
 * @brief Process pending IRQ events
 * @return Number of events processed, -1 on failure
 */
int cts_monitor_process_irq_events(void);

/**
 * @brief Clean up and shutdown monitor
 */
void cts_monitor_cleanup(void);

/**
 * @brief Get current signal state
 * @param state Pointer to signal_state_t structure to fill
 * @return 0 on success, -1 on failure
 */
int cts_monitor_get_state(signal_state_t *state);

#ifdef HAVE_LIBFTDI1
/**
 * @brief Detect if device is an FTDI device
 * @param device_path Path to the serial device
 * @return 1 if FTDI device, 0 if not, -1 on error
 */
int cts_monitor_is_ftdi_device(const char *device_path);

/**
 * @brief Initialize FTDI-specific monitoring
 * @return 0 on success, -1 on failure
 */
int cts_monitor_init_ftdi(void);

/**
 * @brief Update FTDI device monitoring
 * @return 0 on success, -1 on failure
 */
int cts_monitor_update_ftdi(void);

/**
 * @brief Cleanup FTDI resources
 */
void cts_monitor_cleanup_ftdi(void);
#endif

#endif /* CTS_MONITOR_H */