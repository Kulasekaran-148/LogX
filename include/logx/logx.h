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

#include "version.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOGX_MAX_TIMERS
#define LOGX_MAX_TIMERS 5
#endif

#ifndef LOGX_TIMER_MAX_LEN
#define LOGX_TIMER_MAX_LEN 64
#endif

/* Log levels */
typedef enum {
    LOGX_LEVEL_TRACE = 0,
    LOGX_LEVEL_DEBUG,
    LOGX_LEVEL_BANNER,
    LOGX_LEVEL_INFO,
    LOGX_LEVEL_WARN,
    LOGX_LEVEL_ERROR,
    LOGX_LEVEL_FATAL,
    LOGX_LEVEL_OFF
} logx_level_t;

/* Rotation type */
typedef enum {
    LOGX_ROTATE_NONE = 0,
    LOGX_ROTATE_BY_SIZE,
    LOGX_ROTATE_BY_DATE
} logx_rotate_type_t;

/* Rotation configuration */
/* When you add new members here, make sure to update the list in logx_config_key.h */
typedef struct {
    logx_rotate_type_t type;        /* type of rotation */
    size_t             size_mb;     /* used when tyep == LOGX_ROTATE_BY_SIZE */
    int                max_backups; /* number of backup files to keep (0 = no backups) */
    int daily_interval; /* days between rotations when LOGX_ROTATE_BY_DATE (1 = daily) */
} logx_rotate_cfg_t;

/* Logger configuration passed to create function */
/* When you add new members here, make sure to update the list in logx_config_key.h */
typedef struct {
    const char       *name;                   /* logical name of logger (used in prefix) */
    const char       *file_path;              /* if NULL then file logging disabled */
    logx_level_t      console_level;          /* level threshold for console */
    logx_level_t      file_level;             /* level threshold for file */
    int               enable_console_logging; /* 0/1 */
    int               enable_file_logging;    /* 0/1 */
    int               enable_colored_logs;    /* 0/1 */
    int               use_tty_detection; /* if 1, detect isatty and disable colors for non-ttys */
    logx_rotate_cfg_t rotate;            /* rotation options */
    const char       *banner_pattern;    /* Banner pattern */
    int               print_config;      /* 0/1 */
} logx_cfg_t;

/* Timer object */
typedef struct {
    void           *logger;                   // pointer that will store the parent logger instance
    char            name[LOGX_TIMER_MAX_LEN]; // Timer name
    struct timespec start;                    // Start time
    uint64_t        accumulated_ns;           // nanoseconds accumulated due to pauses
    bool            running;                  // 1 if currently running
} logx_timer_t;

/* LogX core structure */
typedef struct {
    logx_cfg_t      cfg;
    FILE           *fp;                      /* opened log file */
    int             fd;                      /* file descriptor for locking/stat */
    pthread_mutex_t lock;                    /* thread safety */
    char            current_date[16];        /* YYYY-MM-DD for date based rotation */
    logx_timer_t    timers[LOGX_MAX_TIMERS]; /* stopwatch timers */
    int             timer_count;
} logx_t;

/* Utility functions - Users can call */
/* Create and initialize a logger. Returns NULL on failure. */
logx_t *logx_create(const logx_cfg_t *cfg);

/* Close and free resources. Safe to call multiple times. */
void logx_destroy(logx_t *logger);

/* Enable/Disable console logging */
void logx_enable_console_logging(logx_t *logger);
void logx_disable_console_logging(logx_t *logger);

/* Enable/Disable file logging */
void logx_enable_file_logging(logx_t *logger);
void logx_disable_file_logging(logx_t *logger);

/* Set log levels */
void logx_set_console_logging_level(logx_t *logger, logx_level_t level);
void logx_set_file_logging_level(logx_t *logger, logx_level_t level);

/* Enable/Disable colored logging */
void logx_enable_colored_logging(logx_t *logger);
void logx_disable_colored_logging(logx_t *logger);

/* Enable/Disable TTY detection */
void logx_enable_tty_detection(logx_t *logger);
void logx_disable_tty_detection(logx_t *logger);

/* Enable/Disable print config */
void logx_enable_print_config(logx_t *logger);
void logx_disable_print_config(logx_t *logger);

/* ===== Log rotation User APIs ===== */

/* Set log rotation type */
void logx_set_log_rotate_type(logx_t *logger, logx_rotate_type_t type);

/* Force an immediate rotation (useful for admin triggers) */
int logx_rotate_now(logx_t *logger);

/* Set log file max size */
void logx_set_log_file_size_mb(logx_t *logger, size_t size_mb);

/* Set log file max number of backups */
void logx_set_num_of_logfile_backups(logx_t *logger, int max_backups);

/* ===== Timer User APIs ===== */

/* Creates or starts a logx timer */
logx_timer_t *logx_timer_start(logx_t *logger, const char *name);

/* Stops a logx timer */
void logx_timer_stop(logx_t *logger, const char *name);

/* Pauses a logx timer */
void logx_timer_pause(logx_t *logger, const char *name);

/* Resumes a logx timer */
void logx_timer_resume(logx_t *logger, const char *name);

/* ===== Helper functions ===== */

/* Helper functions - LogX will internally call */
/* Log a message. file/func/line are helpers provided by macros below. */
void logx_log(logx_t *logger, logx_level_t level, const char *file, const char *func, int line,
              const char *fmt, ...);

/* Helper: convert level to string */
const char *logx_level_to_string(logx_level_t level);

/* Helper: convert rotate type to string */
const char *logx_rotate_type_to_string(logx_rotate_type_t type);

/* Helper: validates whether level is a valid logx_level_t value */
int is_valid_logx_level(logx_level_t level);

/* Helper: validates whether type is a valid logx_rotate_type_t value */
int is_valid_logx_rotate_type(logx_rotate_type_t type);

/* Helper: Locks the fd using flock */
int file_lock_ex(int fd);

/* Helper: Unlocks the fd */
int file_lock_un(int fd);

/* Helper: Rotates log files */
int rotate_files(const char *path, int max_backups);

/* Helper: LOGX_TIMER_AUTO clean up function */
void logx_timer_auto_cleanup(logx_timer_t **t);

/**
 * @brief Automatically starts-stops a timer when the function returns
 * @note __attribute__((cleanup)) is a compiler extension that is available only in GCC & Clang.
 * This will not work with MSVC compiler.
 * @note __COUNTER__ is to make sure the variable is named unique in-case the MACRO gets called
 * multiple times within the same scope
 */
#define LOGX_TIMER_AUTO(logger, name)                               \
    logx_timer_t *__attribute__((cleanup(logx_timer_auto_cleanup))) \
    __logx_auto_timer_##__COUNTER__ = logx_timer_start(logger, name)

/* Macros for easy logging (these expand to a call that includes file/func/line) */
#define LOGX_TRACE(logger, fmt, ...) \
    logx_log((logger), LOGX_LEVEL_TRACE, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_DEBUG(logger, fmt, ...) \
    logx_log((logger), LOGX_LEVEL_DEBUG, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_INFO(logger, fmt, ...) \
    logx_log((logger), LOGX_LEVEL_INFO, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_WARN(logger, fmt, ...) \
    logx_log((logger), LOGX_LEVEL_WARN, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_ERROR(logger, fmt, ...) \
    logx_log((logger), LOGX_LEVEL_ERROR, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_BANNER(logger, fmt, ...) \
    logx_log((logger), LOGX_LEVEL_BANNER, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGX_FATAL(logger, fmt, ...) \
    logx_log((logger), LOGX_LEVEL_FATAL, __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
