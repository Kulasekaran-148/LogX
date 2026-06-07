/**
 * @file logx.h
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Core logx header file
 * @version 0.1
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 *
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

#define LOGX_LOG_FILE_PATH_MAX_LEN_BYTES 1024

/* Logger configuration passed to create function */
struct logx_cfg_t
{
    const char *name;           /* logical name of logger (used in prefix) */
    const char *file_path;      /* if NULL then file logging disabled */
    logx_level_t console_level; /* level threshold for console */
    logx_level_t file_level;    /* level threshold for file */
    int enable_console_logging; /* 0/1 */
    int enable_file_logging;    /* 0/1 */
    int enable_colored_logs;    /* 0/1 */
    int use_tty_detection;      /* if 1, detect isatty and disable colors for non-ttys */
    logx_rotate_cfg_t rotate;   /* rotation options */
    const char *banner_pattern; /* Banner pattern */
    int print_config;           /* 0/1 */
    logx_ts_fmt_t ts_format;    /* Timestamp format */
};

/* LogX core structure */
struct logx_t
{
    logx_cfg_t cfg;
    FILE *fp;                             /* opened log file */
    int fd;                               /* file descriptor for locking/stat */
    pthread_mutex_t lock;                 /* thread safety */
    char current_date[16];                /* YYYY-MM-DD for date based rotation */
    logx_timer_t timers[LOGX_MAX_TIMERS]; /* stopwatch timers */
    int timer_count;
};

#ifdef __cplusplus
extern "C"
{
#endif

    logx_t *logx_create(const logx_cfg_t *cfg);

    /* Close and free resources. Safe to call multiple times. */
    void logx_destroy(logx_t *logger);

    /* Enable/Disable console logging */
    logx_errorcodes_t logx_enable_console_logging(logx_t *logger);
    logx_errorcodes_t logx_disable_console_logging(logx_t *logger);

    /* Enable/Disable file logging */
    logx_errorcodes_t logx_enable_file_logging(logx_t *logger);
    logx_errorcodes_t logx_disable_file_logging(logx_t *logger);

    /* Set log levels */
    logx_errorcodes_t logx_set_console_logging_level(logx_t *logger, logx_level_t level);
    logx_errorcodes_t logx_set_file_logging_level(logx_t *logger, logx_level_t level);

    /* Enable/Disable colored logging */
    logx_errorcodes_t logx_enable_colored_logging(logx_t *logger);
    logx_errorcodes_t logx_disable_colored_logging(logx_t *logger);

    /* Enable/Disable TTY detection */
    logx_errorcodes_t logx_enable_tty_detection(logx_t *logger);
    logx_errorcodes_t logx_disable_tty_detection(logx_t *logger);

    /* Enable/Disable print config */
    logx_errorcodes_t logx_enable_print_config(logx_t *logger);
    logx_errorcodes_t logx_disable_print_config(logx_t *logger);

    /* Helper functions - LogX will internally call */
    /* Log a message. file/func/line are helpers provided by macros below. */
    void logx_log(logx_t *logger, logx_level_t level, const char *file, const char *func, int line,
                  const char *fmt, ...);

    /* Helper: Checks if enough time has passed since the last logged message */
    static inline int logx_freq_check(int sec, time_t *last_logged);

/* Helper: Binary string conversion macros */
#define LOGX_BIN_STR(v) logx_bin_str64_grouped_tls((uint64_t)(v))

/* Strip directory prefix from __FILE__ at compile time — zero runtime cost */
#define LOGX_FILENAME(path) \
    (__builtin_strrchr(path, '/') ? __builtin_strrchr(path, '/') + 1 : (path))

/* Macros for easy logging (these expand to a call that includes file/func/line) */
#define LOGX_TRACE(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_TRACE, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
#define LOGX_DEBUG(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_DEBUG, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
#define LOGX_INFO(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_INFO, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
#define LOGX_WARN(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_WARN, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
#define LOGX_ERROR(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_ERROR, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
#define LOGX_BANNER(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_BANNER, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)
#define LOGX_FATAL(logger, fmt, ...)                                                         \
    logx_log((logger), LOGX_LEVEL_FATAL, LOGX_FILENAME(__FILE__), __func__, __LINE__, (fmt), \
             ##__VA_ARGS__)

#define LOGX_FREQ(sec, last_logged) logx_freq_check((sec), &(last_logged))

#define LOGX_TRACE_FREQ(logger, sec, fmt, ...)          \
    do                                                  \
    {                                                   \
        static time_t _last = 0;                        \
        if (LOGX_FREQ((sec), _last))                    \
            LOGX_TRACE((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

#define LOGX_DEBUG_FREQ(logger, sec, fmt, ...)          \
    do                                                  \
    {                                                   \
        static time_t _last = 0;                        \
        if (LOGX_FREQ((sec), _last))                    \
            LOGX_DEBUG((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

#define LOGX_INFO_FREQ(logger, sec, fmt, ...)          \
    do                                                 \
    {                                                  \
        static time_t _last = 0;                       \
        if (LOGX_FREQ((sec), _last))                   \
            LOGX_INFO((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

#define LOGX_WARN_FREQ(logger, sec, fmt, ...)          \
    do                                                 \
    {                                                  \
        static time_t _last = 0;                       \
        if (LOGX_FREQ((sec), _last))                   \
            LOGX_WARN((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

#define LOGX_ERROR_FREQ(logger, sec, fmt, ...)          \
    do                                                  \
    {                                                   \
        static time_t _last = 0;                        \
        if (LOGX_FREQ((sec), _last))                    \
            LOGX_ERROR((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

#define LOGX_FATAL_FREQ(logger, sec, fmt, ...)          \
    do                                                  \
    {                                                   \
        static time_t _last = 0;                        \
        if (LOGX_FREQ((sec), _last))                    \
            LOGX_FATAL((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

#define LOGX_BANNER_FREQ(logger, sec, fmt, ...)          \
    do                                                   \
    {                                                    \
        static time_t _last = 0;                         \
        if (LOGX_FREQ((sec), _last))                     \
            LOGX_BANNER((logger), (fmt), ##__VA_ARGS__); \
    } while (0)

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
