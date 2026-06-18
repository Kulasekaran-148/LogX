/**
 * @file logx_time.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Stopwatch timer implementation and timestamp formatting for LogX.
 *
 * Provides start/stop/pause/resume timer APIs, an auto-scope timer macro helper,
 * and all timestamp format setter functions.
 *
 * @version 2.0.0
 * @date 2025-11-22
 *
 * @copyright Copyright (c) 2025
 */

#define _POSIX_C_SOURCE 200809L

#include "logx.h"
#include "logx_common.h"
#include "logx_errorcodes.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

logx_errorcodes_t logx_set_ts_format_to_epoch_s(logx_t *logger)
{
    if (logger)
    {
        logger->cfg.ts_format = LOGX_TS_FMT_EPOCH_S;
        return LOGX_ERR_SUCCESS;
    }
    else
        return LOGX_ERR_INVALID_ARG;
}

logx_errorcodes_t logx_set_ts_format_to_epoch_ms(logx_t *logger)
{
    if (logger)
    {
        logger->cfg.ts_format = LOGX_TS_FMT_EPOCH_MS;
        return LOGX_ERR_SUCCESS;
    }
    else
        return LOGX_ERR_INVALID_ARG;
}

logx_errorcodes_t logx_set_ts_format_to_epoch_us(logx_t *logger)
{
    if (logger)
    {
        logger->cfg.ts_format = LOGX_TS_FMT_EPOCH_US;
        return LOGX_ERR_SUCCESS;
    }
    else
        return LOGX_ERR_INVALID_ARG;
}

logx_errorcodes_t logx_set_ts_format_to_local(logx_t *logger)
{
    if (logger)
    {
        logger->cfg.ts_format = LOGX_TS_FMT_LOCAL;
        return LOGX_ERR_SUCCESS;
    }
    else
        return LOGX_ERR_INVALID_ARG;
}

logx_errorcodes_t logx_set_ts_format_to_utc(logx_t *logger)
{
    if (logger)
    {
        logger->cfg.ts_format = LOGX_TS_FMT_UTC;
        return LOGX_ERR_SUCCESS;
    }
    else
        return LOGX_ERR_INVALID_ARG;
}

logx_errorcodes_t logx_set_ts_format_to_iso8601(logx_t *logger)
{
    if (logger)
    {
        logger->cfg.ts_format = LOGX_TS_FMT_ISO8601;
        return LOGX_ERR_SUCCESS;
    }
    else
        return LOGX_ERR_INVALID_ARG;
}

logx_errorcodes_t logx_set_ts_format_to_rfc2822(logx_t *logger)
{
    if (logger)
    {
        logger->cfg.ts_format = LOGX_TS_FMT_RFC2822;
        return LOGX_ERR_SUCCESS;
    }
    else
        return LOGX_ERR_INVALID_ARG;
}

/**
 * @brief Get current timestamp
 *
 * @param[out] pszBuffer - Buffer in which timestamp needs to be filled
 * @param[in] dwBufferLen - Timestamp buffer len
 * @param[out] tv - Timeval object
 * @param[in] eTimestampFormat - Timestamp format
 * @return logx_errorcodes_t LOGX_ERR_SUCCESS on success
 */
logx_errorcodes_t get_timestamp(char *pszBuffer, size_t dwBufferLen, struct timeval *tv,
                                logx_ts_fmt_t eTimestampFormat)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;
    struct timeval ttmp;

    if (!pszBuffer || dwBufferLen == 0)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    /* Capture time if not supplied */
    if (!tv)
    {
        gettimeofday(&ttmp, NULL);
        tv = &ttmp;
    }

    int ms = (int)(tv->tv_usec / 1000);
    int us = (int)(tv->tv_usec);

    switch (eTimestampFormat)
    {

        case LOGX_TS_FMT_EPOCH_S:
        {
            snprintf(pszBuffer, dwBufferLen, "%lld", (long long)tv->tv_sec);
            break;
        }

        case LOGX_TS_FMT_EPOCH_MS:
        {
            snprintf(pszBuffer, dwBufferLen, "%lld", (long long)tv->tv_sec * 1000 + ms);
            break;
        }

        case LOGX_TS_FMT_EPOCH_US:
        {
            snprintf(pszBuffer, dwBufferLen, "%lld", (long long)tv->tv_sec * 1000000 + us);
            break;
        }

        case LOGX_TS_FMT_LOCAL:
        {
            struct tm tm;
            localtime_r(&tv->tv_sec, &tm);
            snprintf(pszBuffer, dwBufferLen, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                     ms);
            break;
        }

        case LOGX_TS_FMT_UTC:
        {
            struct tm tm;
            gmtime_r(&tv->tv_sec, &tm);
            snprintf(pszBuffer, dwBufferLen, "%04d-%02d-%02d %02d:%02d:%02d.%03dZ",
                     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                     ms);
            break;
        }

        case LOGX_TS_FMT_ISO8601:
        {
            struct tm tm;
            gmtime_r(&tv->tv_sec, &tm);
            snprintf(pszBuffer, dwBufferLen, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                     ms);
            break;
        }

        case LOGX_TS_FMT_RFC2822:
        {
            static const char *days[]   = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
            static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
            struct tm tm;
            gmtime_r(&tv->tv_sec, &tm);
            snprintf(pszBuffer, dwBufferLen, "%s, %02d %s %04d %02d:%02d:%02d +0000",
                     days[tm.tm_wday], tm.tm_mday, months[tm.tm_mon], tm.tm_year + 1900, tm.tm_hour,
                     tm.tm_min, tm.tm_sec);
            break;
        }

        default:
        {
            pszBuffer[0] = '\0';
            eErr         = LOGX_ERR_FAILURE;
        }
    }
END:
    return eErr;
}

/**
 * @brief Compute the difference between two timespec values in nanoseconds.
 *
 * Computes (`a - b`) safely and returns the result as a 64-bit unsigned value.
 * Overflow and underflow conditions are checked:
 *
 * - If `a < b`, the function returns 0 (duration cannot be negative).
 * - If computation overflows uint64_t, the function returns UINT64_MAX.
 *
 * @param[in] a Pointer to the end timestamp (later time).
 * @param[in] b Pointer to the start timestamp (earlier time).
 *
 * @return Time difference in nanoseconds. Returns:
 *         - `0` if `a < b`
 *         - `UINT64_MAX` on overflow
 */
static uint64_t diff_ns(const struct timespec *a, const struct timespec *b)
{
    /* Check if a < b */
    if ((a->tv_sec < b->tv_sec) || (a->tv_sec == b->tv_sec && a->tv_nsec < b->tv_nsec))
    {
        return 0;
    }

    uint64_t sec_diff  = (uint64_t)(a->tv_sec - b->tv_sec);
    uint64_t nsec_diff = 0;

    /* Handle nanosecond underflow */
    if (a->tv_nsec >= b->tv_nsec)
        nsec_diff = (uint64_t)(a->tv_nsec - b->tv_nsec);
    else
    {
        /* Borrow 1 second */
        sec_diff -= 1;
        nsec_diff = (uint64_t)(a->tv_nsec + 1000000000ULL - b->tv_nsec);
    }

    /* Overflow check for multiplication */
    if (sec_diff > (UINT64_MAX / 1000000000ULL))
    {
        return UINT64_MAX; /* Overflow */
    }

    uint64_t result = sec_diff * 1000000000ULL;

    /* Final addition overflow check */
    if (result > UINT64_MAX - nsec_diff)
    {
        return UINT64_MAX;
    }

    return result + nsec_diff;
}

/**
 * @brief Convert a duration in nanoseconds to hours, minutes, seconds, and milliseconds.
 *
 * Converts a duration specified in nanoseconds into human-readable
 * components: hours, minutes, seconds, and milliseconds.
 *
 * Example conversion:
 * - 1 hour  = 3,600,000 ms
 * - 1 minute = 60,000 ms
 * - 1 second = 1,000 ms
 *
 * @param[in]  ns  Duration in nanoseconds.
 * @param[out] h   Pointer to store hours component (may be NULL).
 * @param[out] m   Pointer to store minutes component (may be NULL).
 * @param[out] s   Pointer to store seconds component (may be NULL).
 * @param[out] ms  Pointer to store milliseconds component (may be NULL).
 *
 * @note If any output pointer is NULL, that component is ignored.
 */
static void format_time(uint64_t ns, int *h, int *m, int *s, int *ms)
{
    uint64_t ms_total = ns / 1000000ULL;

    *h = ms_total / (1000UL * 60UL * 60UL);
    ms_total %= 1000UL * 60UL * 60UL;

    *m = ms_total / (1000UL * 60UL);
    ms_total %= 1000UL * 60UL;

    *s  = ms_total / 1000UL;
    *ms = ms_total % 1000UL;
}

/**
 * @brief Find an active timer by name in the logger.
 *
 * Searches the logger's internal timer list for a timer whose name
 * matches the given string. The comparison is done using `strcmp()`.
 *
 * @param[in] logger Pointer to the logger instance containing timers.
 * @param[in] name   Null-terminated name of the timer to search for.
 *
 * @return Index of the timer in `logger->timers` if found,
 *         otherwise `-1`.
 *
 * @note The function assumes:
 *       - `logger->timers[i].name` is not NULL.
 */
static int find_timer_index(logx_t *logger, const char *name)
{
    if (!logger || !name)
        return -1;

    for (int i = 0; i < logger->timer_count; i++)
        if (strcmp(logger->timers[i].name, name) == 0)
            return i;

    return -1;
}

logx_timer_t *logx_timer_start(logx_t *logger, const char *name)
{
    if (!logger || !name)
        return NULL;

    pthread_mutex_lock(&logger->lock);

    int idx = find_timer_index(logger, name);

    // If timer already exists
    if (idx >= 0)
    {
        fprintf(stderr, "[LogX] Timer[%s] already exists !\n", logger->timers[idx].name);
        logx_timer_t *t = &logger->timers[idx];

        if (t->bRunning)
        {
            // Already running
            pthread_mutex_unlock(&logger->lock);
            return t;
        }

        // Resuming a paused timer
        clock_gettime(CLOCK_MONOTONIC, &t->start);
        t->bRunning = true;

        pthread_mutex_unlock(&logger->lock);
        return t;
    }

    // Check if Max timer capacity reached
    if (logger->timer_count >= LOGX_MAX_TIMERS)
    {
        // No space – fail silently or log warning
        fprintf(stderr, "[LogX] Max timer capacity reached. Can't create new timer\n");
        pthread_mutex_unlock(&logger->lock);
        return NULL;
    }

    logx_timer_t *t = &logger->timers[logger->timer_count++];

    strncpy(t->name, name, LOGX_TIMER_MAX_LEN - 1);
    t->name[LOGX_TIMER_MAX_LEN - 1] = '\0'; // Ensure null-termination

    clock_gettime(CLOCK_MONOTONIC, &t->start);
    t->accumulated_ns = 0;
    t->bRunning       = 1;

    t->logger = logger;

    pthread_mutex_unlock(&logger->lock);

    return t;
}

void logx_timer_pause(logx_t *logger, const char *name)
{
    if (!logger || !name)
        return;

    pthread_mutex_lock(&logger->lock);

    int idx = find_timer_index(logger, name);
    if (idx < 0)
    {
        pthread_mutex_unlock(&logger->lock);
        return;
    }

    logx_timer_t *t = &logger->timers[idx];

    if (!t->bRunning)
    {
        pthread_mutex_unlock(&logger->lock);
        return; // Already paused
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    t->accumulated_ns += diff_ns(&now, &t->start);
    t->bRunning = 0;

    pthread_mutex_unlock(&logger->lock);
}

void logx_timer_resume(logx_t *logger, const char *name)
{
    if (!logger || !name)
        return;

    pthread_mutex_lock(&logger->lock);

    int idx = find_timer_index(logger, name);
    if (idx < 0)
    {
        pthread_mutex_unlock(&logger->lock);
        return;
    }

    logx_timer_t *t = &logger->timers[idx];

    if (t->bRunning)
    {
        fprintf(stderr, "[LogX] Timer[%s] is already running\n", logger->timers[idx].name);
        pthread_mutex_unlock(&logger->lock);
        return; // Already running
    }

    clock_gettime(CLOCK_MONOTONIC, &t->start);
    t->bRunning = 1;

    pthread_mutex_unlock(&logger->lock);
}

void logx_timer_stop(logx_t *logger, const char *name)
{
    if (!logger || !name)
        return;

    pthread_mutex_lock(&logger->lock);

    int idx = find_timer_index(logger, name);
    if (idx < 0)
    {
        pthread_mutex_unlock(&logger->lock);
        return;
    }

    logx_timer_t *t = &logger->timers[idx];
    struct timespec now;

    // If running, add the final duration
    if (t->bRunning)
    {
        clock_gettime(CLOCK_MONOTONIC, &now);
        t->accumulated_ns += diff_ns(&now, &t->start);
    }

    // Format the elapsed time
    int h, m, s, ms;
    format_time(t->accumulated_ns, &h, &m, &s, &ms);

    // Log the time (adjust to your log function)
    if (logger->cfg.enable_console_logging)
    {
        fprintf(stderr, "[LogX] Timer[%s] took %dh:%dm:%ds:%dms\n", t->name, h, m, s, ms);
    }

    if (logger->cfg.enable_file_logging)
    {
        if (logger->fd >= 0)
            exclusive_flock(logger->fd);

        if (logger->fp)
        {
            fprintf(logger->fp, "[LogX] Timer[%s] took %dh:%dm:%ds:%dms\n", t->name, h, m, s, ms);
            fflush(logger->fp);
        }
    }

    // Remove timer by shifting array left
    /* FIX ME - is array shifting inefficient ? */
    for (int i = idx; i < logger->timer_count - 1; i++)
        logger->timers[i] = logger->timers[i + 1];

    logger->timer_count--;

    pthread_mutex_unlock(&logger->lock);
}

void logx_timer_auto_cleanup(logx_timer_t **t)
{
    if (t && *t && (*t)->logger)
    {
        logx_timer_stop((*t)->logger, (*t)->name);
    }
}
