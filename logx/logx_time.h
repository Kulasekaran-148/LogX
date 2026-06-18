/**
 * @file logx_time.h
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Stopwatch timer and timestamp format APIs for LogX.
 * @version 2.0.0
 * @date 2025-11-22
 *
 * @copyright Copyright (c) 2025
 */

#ifndef LOGX_TIME_H
#define LOGX_TIME_H

#include <logx_types.h>
#include <stdint.h>
#include <sys/time.h>

/** @brief Maximum number of concurrent timers per logger instance. */
#ifndef LOGX_MAX_TIMERS
#define LOGX_MAX_TIMERS 5
#endif

/** @brief Maximum length of a timer name including the null terminator. */
#ifndef LOGX_TIMER_MAX_LEN
#define LOGX_TIMER_MAX_LEN 64
#endif

/**
 * @brief Internal stopwatch timer state.
 */
struct logx_timer_t
{
    void *logger;                  /**< Back-pointer to the owning `logx_t` instance. */
    char name[LOGX_TIMER_MAX_LEN]; /**< Null-terminated timer name. */
    struct timespec start;         /**< Timestamp of the most recent start or resume. */
    uint64_t accumulated_ns;       /**< Total nanoseconds accumulated across pause/resume cycles. */
    int bRunning;                  /**< Non-zero if the timer is currently running. */
};

/**
 * @brief Cleanup callback used by `LOGX_TIMER_AUTO`.
 * @internal
 *
 * Called automatically by the compiler's `__attribute__((cleanup))` extension
 * when the enclosing scope exits.  Do not call directly.
 *
 * @param[in] t Double pointer to the timer to stop.
 */
void logx_timer_auto_cleanup(logx_timer_t **t);

/**
 * @brief Declare a scope-scoped timer that stops automatically on function return.
 *
 * Uses GCC/Clang `__attribute__((cleanup))`. Not supported on MSVC.
 * `__COUNTER__` ensures the internal variable name is unique even if the macro
 * is used multiple times within the same scope.
 *
 * @param logger Pointer to the `logx_t` instance.
 * @param name   String literal or variable holding the timer name.
 */
#define LOGX_TIMER_AUTO(logger, name)                               \
    logx_timer_t *__attribute__((cleanup(logx_timer_auto_cleanup))) \
    __logx_auto_timer_##__COUNTER__ = logx_timer_start(logger, name)

/**
 * @brief Convert a 64-bit unsigned integer to a grouped binary string.
 *
 * Uses thread-local rotating buffers so the result is safe to use directly
 * inside a `printf`-family call without manual memory management.
 * Up to 8 calls per thread can be in-flight simultaneously.
 *
 * Prefer the `LOGX_BIN_STR(v)` macro over calling this function directly.
 *
 * @param[in] value Value to convert.
 * @return Pointer to a null-terminated binary string with nibble grouping,
 *         e.g. `"0001 0100"` for 20.
 */
const char *logx_bin_str64_grouped_tls(uint64_t value);

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Start a new stopwatch timer or resume a paused one.
     *
     * If a timer with the given name already exists and is paused, it is resumed.
     * If it is already running, the call is a no-op.
     * If no timer exists and capacity allows, a new one is created and started.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] name   Null-terminated timer name (max `LOGX_TIMER_MAX_LEN - 1` chars).
     * @return Pointer to the started `logx_timer_t`, or NULL on error.
     */
    logx_timer_t *logx_timer_start(logx_t *logger, const char *name);

    /**
     * @brief Stop a running timer, log the elapsed time, and remove it.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] name   Name of the timer to stop.
     */
    void logx_timer_stop(logx_t *logger, const char *name);

    /**
     * @brief Pause a running timer, accumulating elapsed time so far.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] name   Name of the timer to pause.
     */
    void logx_timer_pause(logx_t *logger, const char *name);

    /**
     * @brief Resume a paused timer.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] name   Name of the timer to resume.
     */
    void logx_timer_resume(logx_t *logger, const char *name);

    /**
     * @brief Set the timestamp format to Unix epoch seconds (`1747384321`).
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_ts_format_to_epoch_s(logx_t *logger);

    /**
     * @brief Set the timestamp format to Unix epoch milliseconds (`1747384321123`).
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_ts_format_to_epoch_ms(logx_t *logger);

    /**
     * @brief Set the timestamp format to Unix epoch microseconds (`1747384321123456`).
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_ts_format_to_epoch_us(logx_t *logger);

    /**
     * @brief Set the timestamp format to local wall-clock time (`2026-05-16 14:32:01.123`).
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_ts_format_to_local(logx_t *logger);

    /**
     * @brief Set the timestamp format to UTC wall-clock time (`2026-05-16 08:32:01.123Z`).
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_ts_format_to_utc(logx_t *logger);

    /**
     * @brief Set the timestamp format to ISO 8601 / RFC 3339 (`2026-05-16T08:32:01.123Z`).
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_ts_format_to_iso8601(logx_t *logger);

    /**
     * @brief Set the timestamp format to RFC 2822 (`Sat, 16 May 2026 08:32:01 +0000`).
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS`, or `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_ts_format_to_rfc2822(logx_t *logger);

#ifdef __cplusplus
}
#endif

/**
 * @brief Internal — format the current time into a buffer according to the given format.
 * @internal
 *
 * @param[out] pszBuffer       Destination buffer.
 * @param[in]  dwBufferLen     Size of the destination buffer in bytes.
 * @param[in]  tv              Time value to format; if NULL, the current time is captured.
 * @param[in]  eTimestampFormat One of the `logx_ts_fmt_t` constants.
 * @return `LOGX_ERR_SUCCESS` on success.
 */
logx_errorcodes_t get_timestamp(char *pszBuffer, size_t dwBufferLen, struct timeval *tv,
                                logx_ts_fmt_t eTimestampFormat);

#endif /* LOGX_TIME_H */