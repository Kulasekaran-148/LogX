#ifndef LOGX_TIME_H
#define LOGX_TIME_H

#include <logx_types.h>
#include <stdint.h>
#include <sys/time.h>

#ifndef LOGX_MAX_TIMERS
#define LOGX_MAX_TIMERS 5
#endif

#ifndef LOGX_TIMER_MAX_LEN
#define LOGX_TIMER_MAX_LEN 64
#endif

struct logx_timer_t
{
    void *logger;                  // pointer that will store the parent logger instance
    char name[LOGX_TIMER_MAX_LEN]; // Timer name
    struct timespec start;         // Start time
    uint64_t accumulated_ns;       // nanoseconds accumulated due to pauses
    int bRunning;                  // 1 if currently running
};

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

/* Helper: Converts a 64-bit integer to a binary string with grouped nibbles (4 bits) */
const char *logx_bin_str64_grouped_tls(uint64_t value);

#ifdef __cplusplus
extern "C"
{
#endif

    /* Function declarations */

    /* User Timer APIs */
    logx_timer_t *logx_timer_start(logx_t *logger, const char *name);
    void logx_timer_stop(logx_t *logger, const char *name);
    void logx_timer_pause(logx_t *logger, const char *name);
    void logx_timer_resume(logx_t *logger, const char *name);

    /* User Timestamp APIs */
    logx_errorcodes_t logx_set_ts_format_to_epoch_s(logx_t *logger);
    logx_errorcodes_t logx_set_ts_format_to_epoch_ms(logx_t *logger);
    logx_errorcodes_t logx_set_ts_format_to_epoch_us(logx_t *logger);
    logx_errorcodes_t logx_set_ts_format_to_local(logx_t *logger);
    logx_errorcodes_t logx_set_ts_format_to_utc(logx_t *logger);
    logx_errorcodes_t logx_set_ts_format_to_iso8601(logx_t *logger);
    logx_errorcodes_t logx_set_ts_format_to_rfc2822(logx_t *logger);

#ifdef __cplusplus
}
#endif

logx_errorcodes_t get_timestamp(char *pszBuffer, size_t dwBufferLen, struct timeval *tv,
                                logx_ts_fmt_t eTimestampFormat);

#endif /* LOGX_TIME_H */