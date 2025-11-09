#ifndef _LOGX_H
#define _LOGX_H

#include "version.h"

#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* Log levels */
    typedef enum
    {
        LOG_LEVEL_TRACE = 0,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_BANNER,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_FATAL,
        LOG_LEVEL_OFF
    } log_level_t;

    /* Rotation type */
    typedef enum
    {
        LOG_ROTATE_NONE = 0,
        LOG_ROTATE_BY_SIZE,
        LOG_ROTATE_BY_DATE
    } log_rotate_type_t;

    /* Rotation configuration */
    typedef struct
    {
        log_rotate_type_t type;        /* type of rotation */
        size_t            max_bytes;   /* used when tyep == LOG_ROTATE_BY_SIZE */
        int               max_backups; /* number of backup files to keep (0 = no backups) */
        int daily_interval; /* days between rotations when LOG_ROTATE_BY_DATE (1 = daily) */
    } log_rotate_cfg_t;

    /* Logger configuration passed to create function */
    typedef struct
    {
        const char *name;                   /* logical name of logger (used in prefix) */
        const char *file_path;              /* if NULL then file logging disabled */
        log_level_t console_level;          /* level threshold for console */
        log_level_t file_level;             /* level threshold for file */
        int         enable_console_logging; /* 0/1 */
        int         enable_file_logging;    /* 0/1 */
        int         enabled_colored_logs;   /* 0/1 */
        int         use_tty_detection; /* if 1, detect isatty and disable colors for non-ttys */
        log_rotate_cfg_t rotate;       /* rotation options */
        const char      *banner_pattern;
    } logx_cfg_t;

    struct logx_t
    {
        logx_cfg_t      cfg;
        FILE           *fp;               /* opened log file */
        int             fd;               /* file descriptor for locking/stat */
        pthread_mutex_t lock;             /* thread safety */
        char            current_date[16]; /* YYYY-MM-DD for date based rotation */
    };

    /* Opaque logger handle - definition is in the .c file but we expose the struct type for static
     * alloc options */
    typedef struct logx_t logx_t;

    /* Create and initialize a logger. Returns NULL on failure. */
    logx_t *logx_create(const logx_cfg_t *cfg);

    /* Close and free resources. Safe to call multiple times. */
    void logx_destroy(logx_t *logger);

    /* Change levels at runtime. */
    void logx_set_console_level(logx_t *logger, log_level_t level);
    void logx_set_file_level(logx_t *logger, log_level_t level);

    /* Toggle outputs */
    void logx_enable_console_logging(logx_t *logger, int enable);
    void logx_enable_file_logging(logx_t *logger, int enable);

    /* Force an immediate rotation (useful for admin triggers) */
    int logx_rotate_now(logx_t *logger);

    /* Log a message. file/func/line are helpers provided by macros below. */
    void logx_log(logx_t *logger, log_level_t level, const char *file, const char *func, int line,
                  const char *fmt, ...);

    /* Helper: convert level to string */
    const char *log_level_to_string(log_level_t level);

/* Macros for easy logging (these expand to a call that includes file/func/line) */
#define LOGX_TRACE(logger, fmt, ...) \
    logx_log((logger), LOG_LEVEL_TRACE, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_DEBUG(logger, fmt, ...) \
    logx_log((logger), LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_INFO(logger, fmt, ...) \
    logx_log((logger), LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_WARN(logger, fmt, ...) \
    logx_log((logger), LOG_LEVEL_WARN, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_ERROR(logger, fmt, ...) \
    logx_log((logger), LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_BANNER(logger, fmt, ...) \
    logx_log((logger), LOG_LEVEL_BANNER, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_FATAL(logger, fmt, ...) \
    logx_log((logger), LOG_LEVEL_FATAL, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
