/**
 * @file logx.h
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Main public header for the LogX logging library.
 *
 * Include this single header to access all LogX types, configuration APIs,
 * logging macros, rate-limiting macros, and timer utilities.
 *
 * @version 2.0.0
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 */

#ifndef _LOGX_H
#define _LOGX_H

#include "logx_rotation.h"
#include "logx_time.h"
#include "logx_types.h"
#include "version.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <syslog.h>

/** @brief Maximum byte length for a log file path, including the null terminator. */
#define LOGX_LOG_FILE_PATH_MAX_LEN_BYTES 1024

/** @brief Per-call flag for logx_log_f(): route this message to syslog. Combine with OR. */
#define LOGX_FLAG_SYSLOG (1U << 0)

/**
 * @brief Syslog facility codes.
 *
 * Passed to `logx_cfg_t::syslog_facility` or `logx_set_syslog_facility()`.
 * Values map directly to the POSIX `LOG_*` facility constants.
 */
typedef enum
{
    LOGX_SYSLOG_FACILITY_USER   = LOG_USER,   /**< User-level messages (default). */
    LOGX_SYSLOG_FACILITY_DAEMON = LOG_DAEMON, /**< System daemons. */
    LOGX_SYSLOG_FACILITY_LOCAL0 = LOG_LOCAL0, /**< Local use 0. */
    LOGX_SYSLOG_FACILITY_LOCAL1 = LOG_LOCAL1, /**< Local use 1. */
    LOGX_SYSLOG_FACILITY_LOCAL2 = LOG_LOCAL2, /**< Local use 2. */
    LOGX_SYSLOG_FACILITY_LOCAL3 = LOG_LOCAL3, /**< Local use 3. */
    LOGX_SYSLOG_FACILITY_LOCAL4 = LOG_LOCAL4, /**< Local use 4. */
    LOGX_SYSLOG_FACILITY_LOCAL5 = LOG_LOCAL5, /**< Local use 5. */
    LOGX_SYSLOG_FACILITY_LOCAL6 = LOG_LOCAL6, /**< Local use 6. */
    LOGX_SYSLOG_FACILITY_LOCAL7 = LOG_LOCAL7, /**< Local use 7. */
} logx_syslog_facility_t;

/**
 * @brief Logger configuration structure.
 *
 * Populate and pass a pointer to `logx_create()` to customise logger behaviour.
 * Pass `NULL` to `logx_create()` instead to load configuration from a file or
 * fall back to built-in defaults.
 *
 * String fields (`name`, `file_path`, `banner_pattern`) are copied internally by
 * `logx_create()`; you do not need to keep the originals alive after the call.
 */
struct logx_cfg_t
{
    const char *name;           /**< Logical name shown in every log prefix. */
    const char *file_path;      /**< Path to the log file. Pass NULL to disable file logging. */
    logx_level_t console_level; /**< Minimum level to write to the console. */
    logx_level_t file_level;    /**< Minimum level to write to the log file. */
    int enable_console_logging; /**< 1 = enable console output, 0 = disable. */
    int enable_file_logging;    /**< 1 = enable file output, 0 = disable. */
    int enable_colored_logs;    /**< 1 = use ANSI colour codes on the console. */
    int use_tty_detection;      /**< 1 = auto-disable colours when stdout is not a TTY. */
    logx_rotate_cfg_t rotate;   /**< Log rotation policy (size, date, or none). */
    const char *banner_pattern; /**< Character(s) used to draw the banner border, e.g. `"="`. */
    int print_config;           /**< 1 = print active configuration on `logx_create`. */
    logx_ts_fmt_t ts_format;    /**< Timestamp format written into each log entry. */
    int enable_syslog;          /**< 1 = also route log messages to syslog. */
    logx_syslog_facility_t syslog_facility; /**< syslog facility (default LOGX_SYSLOG_FACILITY_USER). */
    const char *syslog_ident;               /**< syslog identity string (NULL = logger name). */
};

/**
 * @brief Core logger instance.
 *
 * Obtain via `logx_create()`. All fields are managed internally; do not access
 * them directly — use the public API functions instead.
 */
struct logx_t
{
    logx_cfg_t cfg;        /**< Active configuration (owned copy). */
    FILE *fp;              /**< Open log-file handle, or NULL. */
    int fd;                /**< File descriptor for flock/fstat. */
    pthread_mutex_t lock;  /**< Mutex that protects all mutable state. */
    char current_date[16]; /**< Last-seen date string `YYYY-MM-DD` for date rotation. */
    logx_timer_t timers[LOGX_MAX_TIMERS]; /**< Pool of stopwatch timers. */
    int timer_count;                      /**< Number of active timers. */
    int syslog_opened; /**< 1 if openlog() has been called for this logger instance. */
};

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Create and initialise a logger instance.
     *
     * When `cfg` is NULL, LogX attempts to load configuration in this order:
     *   1. File at the path defined by the `LOGX_CFG_FILEPATH` macro (if defined).
     *   2. `./logx_cfg.yml`, `./logx_cfg.yaml`, or `./logx_cfg.json` (first found).
     *   3. Built-in defaults from `logx_config.h`.
     *
     * If `file_path` is set in the configuration, LogX will automatically create
     * any intermediate directories needed for the path.
     *
     * @param[in]  cfg        Pointer to a populated `logx_cfg_t`, or NULL.
     * @param[out] out_logger Receives the newly created logger pointer on success.
     *                        Set to NULL on failure.
     * @return `LOGX_ERR_SUCCESS` on success, or an error code on failure.
     */
    logx_errorcodes_t logx_create(const logx_cfg_t *cfg, logx_t **out_logger);

    /**
     * @brief Flush, close, and free a logger instance.
     *
     * Flushes the log file, releases the file descriptor, destroys the mutex, and
     * frees all heap-allocated memory owned by the logger.
     *
     * @param[in] logger Pointer to the logger to destroy.
     * @return `LOGX_ERR_SUCCESS` on success, `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_destroy(logx_t *logger);

    /**
     * @brief Enable console logging at runtime.
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_enable_console_logging(logx_t *logger);

    /**
     * @brief Disable console logging at runtime.
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_disable_console_logging(logx_t *logger);

    /**
     * @brief Enable file logging at runtime.
     *
     * Has no effect if no log file path was provided at creation time; returns
     * `LOGX_ERR_INVALID_LOGFILE_PATH` in that case.
     *
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or an appropriate error code.
     */
    logx_errorcodes_t logx_enable_file_logging(logx_t *logger);

    /**
     * @brief Disable file logging at runtime.
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_disable_file_logging(logx_t *logger);

    /**
     * @brief Set the minimum log level for console output.
     *
     * Only messages at or above `level` will be written to the console.
     * Pass `LOGX_LEVEL_OFF` to silence console output entirely.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] level  Desired minimum level.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` on bad input.
     */
    logx_errorcodes_t logx_set_console_logging_level(logx_t *logger, logx_level_t level);

    /**
     * @brief Set the minimum log level for file output.
     *
     * Only messages at or above `level` will be written to the log file.
     * Pass `LOGX_LEVEL_OFF` to silence file output entirely.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] level  Desired minimum level.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` on bad input.
     */
    logx_errorcodes_t logx_set_file_logging_level(logx_t *logger, logx_level_t level);

    /**
     * @brief Enable ANSI coloured console output.
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_enable_colored_logging(logx_t *logger);

    /**
     * @brief Disable ANSI coloured console output.
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_disable_colored_logging(logx_t *logger);

    /**
     * @brief Enable automatic TTY detection.
     *
     * When enabled, ANSI colours are automatically suppressed when stdout is not
     * connected to a terminal (e.g. when output is piped or redirected).
     *
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_enable_tty_detection(logx_t *logger);

    /**
     * @brief Disable automatic TTY detection.
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_disable_tty_detection(logx_t *logger);

    /**
     * @brief Enable printing of the active configuration on logger creation.
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_enable_print_config(logx_t *logger);

    /**
     * @brief Disable printing of the active configuration on logger creation.
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_disable_print_config(logx_t *logger);

    /**
     * @brief Enable the syslog sink at runtime.
     *
     * Opens the syslog connection (calls `openlog()`) if not already open, using
     * `syslog_ident` from the config (falls back to logger name) and the configured
     * facility.
     *
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_enable_syslog(logx_t *logger);

    /**
     * @brief Disable the syslog sink at runtime.
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_disable_syslog(logx_t *logger);

    /**
     * @brief Change the syslog facility at runtime.
     * @param[in] logger   Pointer to the logger instance.
     * @param[in] facility New facility value.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_syslog_facility(logx_t *logger, logx_syslog_facility_t facility);

    /**
     * @brief Internal log dispatch function called by the `LOGX_*` macros.
     * @internal
     *
     * Do not call this directly. Use the `LOGX_TRACE`, `LOGX_DEBUG`, etc. macros,
     * which automatically supply `file`, `func`, and `line`.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] level  Severity level of the message.
     * @param[in] file   Source file name (stripped of directory prefix).
     * @param[in] func   Calling function name.
     * @param[in] line   Source line number.
     * @param[in] fmt    printf-style format string.
     * @param[in] ...    Format arguments.
     */
    void logx_log(logx_t *logger, logx_level_t level, const char *file, const char *func, int line,
                  const char *fmt, ...);

    /**
     * @brief Extended log dispatch with per-call flags (e.g. `LOGX_FLAG_SYSLOG`).
     * @internal
     *
     * Used by the `LOGX_*_SYSLOG` macros. Prefer those macros over calling this directly.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] level  Severity level of the message.
     * @param[in] flags  Bitfield of `LOGX_FLAG_*` values.
     * @param[in] file   Source file name (stripped of directory prefix).
     * @param[in] func   Calling function name.
     * @param[in] line   Source line number.
     * @param[in] fmt    printf-style format string.
     * @param[in] ...    Format arguments.
     */
    void logx_log_f(logx_t *logger, logx_level_t level, uint32_t flags, const char *file,
                    const char *func, int line, const char *fmt, ...);

    /**
     * @brief Internal rate-limit check used by `LOGX_FREQ` and the `LOGX_*_FREQ` macros.
     * @internal
     *
     * Returns 1 and updates `*last_logged` if at least `sec` seconds have elapsed
     * since the last allowed log. Returns 0 otherwise.
     *
     * @param[in]     sec          Minimum interval in seconds.
     * @param[in,out] last_logged  Pointer to the static timestamp variable at the call site.
     * @return 1 if the message should be logged, 0 if it should be suppressed.
     */
    static inline int logx_freq_check(int sec, time_t *last_logged);

/**
 * @brief Convert any integer value to a grouped binary string for display.
 *
 * Wraps `logx_bin_str64_grouped_tls()`. Safe to use inside printf-family calls.
 * Example: `LOGX_DEBUG(logger, "val = %s", LOGX_BIN_STR(0xAB))` → `"1010 1011"`.
 */
#define LOGX_BIN_STR(v) logx_bin_str64_grouped_tls((uint64_t)(v))

/** @brief Strip the directory prefix from `__FILE__` at compile time — zero runtime cost. */
#define LOGX_FILENAME(path) \
    (__builtin_strrchr(path, '/') ? __builtin_strrchr(path, '/') + 1 : (path))

/** @defgroup logx_macros Logging macros
 *  Convenience macros that capture source location automatically.
 *  @{
 */

/** @brief Log a TRACE-level message. */
#define LOGX_TRACE(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_TRACE, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
/** @brief Log a DEBUG-level message. */
#define LOGX_DEBUG(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_DEBUG, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
/** @brief Log an INFO-level message. */
#define LOGX_INFO(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_INFO, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
/** @brief Log a WARN-level message. */
#define LOGX_WARN(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_WARN, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
/** @brief Log an ERROR-level message. */
#define LOGX_ERROR(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_ERROR, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
/** @brief Log a BANNER-level message (auto-centered inside a border). */
#define LOGX_BANNER(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_BANNER, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
/** @brief Log a FATAL-level message. */
#define LOGX_FATAL(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_FATAL, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)

/** @brief Log a TRACE message and also route it to syslog. */
#define LOGX_TRACE_SYSLOG(logger, fmt, ...)                                                     \
    logx_log_f((logger), LOGX_LEVEL_TRACE, LOGX_FLAG_SYSLOG, LOGX_FILENAME(__FILE__), __func__, \
               __LINE__, (fmt), ##__VA_ARGS__)
/** @brief Log a DEBUG message and also route it to syslog. */
#define LOGX_DEBUG_SYSLOG(logger, fmt, ...)                                                     \
    logx_log_f((logger), LOGX_LEVEL_DEBUG, LOGX_FLAG_SYSLOG, LOGX_FILENAME(__FILE__), __func__, \
               __LINE__, (fmt), ##__VA_ARGS__)
/** @brief Log an INFO message and also route it to syslog. */
#define LOGX_INFO_SYSLOG(logger, fmt, ...)                                                     \
    logx_log_f((logger), LOGX_LEVEL_INFO, LOGX_FLAG_SYSLOG, LOGX_FILENAME(__FILE__), __func__, \
               __LINE__, (fmt), ##__VA_ARGS__)
/** @brief Log a WARN message and also route it to syslog. */
#define LOGX_WARN_SYSLOG(logger, fmt, ...)                                                     \
    logx_log_f((logger), LOGX_LEVEL_WARN, LOGX_FLAG_SYSLOG, LOGX_FILENAME(__FILE__), __func__, \
               __LINE__, (fmt), ##__VA_ARGS__)
/** @brief Log an ERROR message and also route it to syslog. */
#define LOGX_ERROR_SYSLOG(logger, fmt, ...)                                                     \
    logx_log_f((logger), LOGX_LEVEL_ERROR, LOGX_FLAG_SYSLOG, LOGX_FILENAME(__FILE__), __func__, \
               __LINE__, (fmt), ##__VA_ARGS__)
/** @brief Log a BANNER message and also route it to syslog. */
#define LOGX_BANNER_SYSLOG(logger, fmt, ...)                                                     \
    logx_log_f((logger), LOGX_LEVEL_BANNER, LOGX_FLAG_SYSLOG, LOGX_FILENAME(__FILE__), __func__, \
               __LINE__, (fmt), ##__VA_ARGS__)
/** @brief Log a FATAL message and also route it to syslog. */
#define LOGX_FATAL_SYSLOG(logger, fmt, ...)                                                     \
    logx_log_f((logger), LOGX_LEVEL_FATAL, LOGX_FLAG_SYSLOG, LOGX_FILENAME(__FILE__), __func__, \
               __LINE__, (fmt), ##__VA_ARGS__)

/** @} */ /* logx_macros */

/**
 * @brief Raw rate-limit gate — evaluates to 1 if `sec` seconds have elapsed since last pass.
 *
 * `last_logged` must be an lvalue of type `time_t`.
 * Prefer the `LOGX_*_FREQ` macros, which manage the state variable automatically.
 */
#define LOGX_FREQ(sec, last_logged) logx_freq_check((sec), &(last_logged))

/** @defgroup logx_freq_macros Rate-limited logging macros
 *
 *  Each macro suppresses the log call if fewer than `sec` seconds have elapsed
 *  since the same call site last produced output.  Each call site maintains its
 *  own independent `static time_t` counter.
 *
 *  @{
 */

/** @brief Log a TRACE message at most once every `sec` seconds. */
#define LOGX_TRACE_FREQ(logger, sec, fmt, ...)          \
    do                                                  \
    {                                                   \
        static time_t _last = 0;                        \
        if (LOGX_FREQ((sec), _last))                    \
            LOGX_TRACE((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

/** @brief Log a DEBUG message at most once every `sec` seconds. */
#define LOGX_DEBUG_FREQ(logger, sec, fmt, ...)          \
    do                                                  \
    {                                                   \
        static time_t _last = 0;                        \
        if (LOGX_FREQ((sec), _last))                    \
            LOGX_DEBUG((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

/** @brief Log an INFO message at most once every `sec` seconds. */
#define LOGX_INFO_FREQ(logger, sec, fmt, ...)          \
    do                                                 \
    {                                                  \
        static time_t _last = 0;                       \
        if (LOGX_FREQ((sec), _last))                   \
            LOGX_INFO((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

/** @brief Log a WARN message at most once every `sec` seconds. */
#define LOGX_WARN_FREQ(logger, sec, fmt, ...)          \
    do                                                 \
    {                                                  \
        static time_t _last = 0;                       \
        if (LOGX_FREQ((sec), _last))                   \
            LOGX_WARN((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

/** @brief Log an ERROR message at most once every `sec` seconds. */
#define LOGX_ERROR_FREQ(logger, sec, fmt, ...)          \
    do                                                  \
    {                                                   \
        static time_t _last = 0;                        \
        if (LOGX_FREQ((sec), _last))                    \
            LOGX_ERROR((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

/** @brief Log a FATAL message at most once every `sec` seconds. */
#define LOGX_FATAL_FREQ(logger, sec, fmt, ...)          \
    do                                                  \
    {                                                   \
        static time_t _last = 0;                        \
        if (LOGX_FREQ((sec), _last))                    \
            LOGX_FATAL((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

/** @brief Log a BANNER message at most once every `sec` seconds. */
#define LOGX_BANNER_FREQ(logger, sec, fmt, ...)          \
    do                                                   \
    {                                                    \
        static time_t _last = 0;                         \
        if (LOGX_FREQ((sec), _last))                     \
            LOGX_BANNER((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

    /** @} */ /* logx_freq_macros */

#ifdef __cplusplus
}
#endif

static inline int logx_freq_check(int sec, time_t *last_logged)
{
    time_t _now = time(NULL);
    if ((_now - *last_logged) >= sec)
    {
        *last_logged = _now;
        return 1;
    }
    return 0;
}

#endif
