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
 * @brief Monitor configuration structure
 */
typedef struct {
    const char *serial_device;     /**< Serial device path (e.g., /dev/ttyUSB0) */
    int poll_interval_us;          /**< Polling interval in microseconds */
    time_format_t time_format;     /**< Timestamp format */
    const char *output_file;       /**< Output file path (NULL for stdout) */
    int verbose;                   /**< Verbose mode flag */
} monitor_config_t;

/**
 * @brief Initialize the CTS monitor
 * @param config Pointer to configuration structure
 * @return 0 on success, -1 on failure
 */
int cts_monitor_init(const monitor_config_t *config);

/**
 * @brief Update monitor (check for signal changes)
 * @return 0 on success, -1 on failure
 */
int cts_monitor_update(void);

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

#endif /* CTS_MONITOR_H */