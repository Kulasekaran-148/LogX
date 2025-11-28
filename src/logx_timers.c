/**
 * @file logx_timers.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Contains function related to LogX stopwatch timers
 * @version 0.1
 * @date 2025-11-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#define _POSIX_C_SOURCE 200809L

#include "../include/logx/logx.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

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
    if ( (a->tv_sec < b->tv_sec) ||
         (a->tv_sec == b->tv_sec && a->tv_nsec < b->tv_nsec) )
    {
        return 0;
    }

    uint64_t sec_diff  = (uint64_t)(a->tv_sec - b->tv_sec);
    uint64_t nsec_diff = 0;

    /* Handle nanosecond underflow */
    if (a->tv_nsec >= b->tv_nsec)
        nsec_diff = (uint64_t)(a->tv_nsec - b->tv_nsec);
    else {
        /* Borrow 1 second */
        sec_diff -= 1;
        nsec_diff = (uint64_t)(a->tv_nsec + 1000000000ULL - b->tv_nsec);
    }

    /* Overflow check for multiplication */
    if (sec_diff > (UINT64_MAX / 1000000000ULL)) {
        return UINT64_MAX; /* Overflow */
    }

    uint64_t result = sec_diff * 1000000000ULL;

    /* Final addition overflow check */
    if (result > UINT64_MAX - nsec_diff) {
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
    if(!logger || !name) return -1;

    for (int i = 0; i < logger->timer_count; i++)
        if (strcmp(logger->timers[i].name, name) == 0)
            return i;

    return -1;
}

/**
 * @brief Start a new stopwatch timer or resume an existing one.
 *
 * If a timer with the given name already exists:
 *   - If it is paused, the timer is resumed.
 *   - If it is already running, no action is taken.
 *
 * If no timer exists and capacity permits, a new timer is created and started.
 *
 * @param[in] logger Pointer to the logx instance.
 * @param[in] name   Null-terminated timer name.
 *
 * @note Thread-safe: acquires `logger->lock`.
 * @note If the maximum number of timers (`LOGX_MAX_TIMERS`) has been reached,
 *       the function prints a warning and returns.
 * @note If `logger` or `name` is NULL, the function returns immediately.
 */
void logx_timer_start(logx_t *logger, const char *name)
{
    if (!logger || !name)
        return;

    pthread_mutex_lock(&logger->lock);

    int idx = find_timer_index(logger, name);

    // If timer already exists
    if (idx >= 0)
    {
        fprintf(stderr, "[LogX] Timer[%s] already exists !\n", logger->timers[idx].name);
        logx_timer_t *t = &logger->timers[idx];

        if (t->running)
        {
            // Already running
            pthread_mutex_unlock(&logger->lock);
            return;
        }

        // Resuming a paused timer
        clock_gettime(CLOCK_MONOTONIC, &t->start);
        t->running = true;

        pthread_mutex_unlock(&logger->lock);
        return;
    }

    // Check if Max timer capacity reached
    if (logger->timer_count >= LOGX_MAX_TIMERS)
    {
        // No space – fail silently or log warning
        fprintf(stderr, "[LogX] Max timer capacity reached. Can't create new timer\n");
        pthread_mutex_unlock(&logger->lock);
        return;
    }

    logx_timer_t *t = &logger->timers[logger->timer_count++];
    
    strncpy(t->name, name, LOGX_TIMER_MAX_LEN - 1);
    t->name[LOGX_TIMER_MAX_LEN - 1] = '\0'; // Ensure null-termination

    clock_gettime(CLOCK_MONOTONIC, &t->start);
    t->accumulated_ns = 0;
    t->running        = 1;

    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Pause a running stopwatch timer.
 *
 * If the timer is found and currently running, its elapsed
 * duration since the last start/resume is added to the accumulated
 * total and the timer is marked as paused.
 *
 * If the timer does not exist or is already paused, no action is taken.
 *
 * @param[in] logger Pointer to the logx instance.
 * @param[in] name   Null-terminated timer name to pause.
 *
 * @note Thread-safe: acquires `logger->lock`.
 * @note If `logger` or `name` is NULL, the function returns immediately.
 */
void logx_timer_pause(logx_t *logger, const char *name)
{
    if (!logger || !name) return;

    pthread_mutex_lock(&logger->lock);

    int idx = find_timer_index(logger, name);
    if (idx < 0)
    {
        pthread_mutex_unlock(&logger->lock);
        return;
    }

    logx_timer_t *t = &logger->timers[idx];

    if (!t->running)
    {
        pthread_mutex_unlock(&logger->lock);
        return; // Already paused
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    t->accumulated_ns += diff_ns(&now, &t->start);
    t->running = 0;

    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Resume a paused stopwatch timer.
 *
 * If the timer exists and is currently paused, this function
 * records a new `start` timestamp and marks it as running again.
 *
 * If the timer does not exist or is already running, no action is taken.
 *
 * @param[in] logger Pointer to the logx instance.
 * @param[in] name   Null-terminated timer name to resume.
 *
 * @note Thread-safe: acquires `logger->lock`.
 * @note If `logger` or `name` is NULL, the function returns immediately.
 */
void logx_timer_resume(logx_t *logger, const char *name)
{
    if (!logger || !name) return;

    pthread_mutex_lock(&logger->lock);

    int idx = find_timer_index(logger, name);
    if (idx < 0)
    {
        pthread_mutex_unlock(&logger->lock);
        return;
    }

    logx_timer_t *t = &logger->timers[idx];

    if (t->running)
    {
        fprintf(stderr, "[LogX] Timer[%s] is already running\n", logger->timers[idx].name);
        pthread_mutex_unlock(&logger->lock);
        return; // Already running
    }

    clock_gettime(CLOCK_MONOTONIC, &t->start);
    t->running = 1;

    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Stop a stopwatch timer, log the elapsed time, and remove it.
 *
 * If the timer is running, the time since its last start/resume is added
 * to the accumulated elapsed time before formatting and logging.
 *
 * The formatted output is logged to:
 *   - Console, if console logging is enabled.
 *   - Log file, if file logging is enabled.
 *
 * After logging, the timer is removed from the logger's timer list.
 *
 * @param[in] logger Pointer to the logx instance.
 * @param[in] name   Null-terminated timer name to stop and remove.
 *
 * @note Thread-safe: acquires `logger->lock`.
 * @note Timer entries are removed by shifting the remaining array elements.
 *       On systems with high timer churn, consider a linked list instead.
 * @note If `logger` or `name` is NULL, or timer not found, the function returns immediately.
 */
void logx_timer_stop(logx_t *logger, const char *name)
{
    if (!logger || !name) return;

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
    if (t->running)
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
            file_lock_ex(logger->fd);

        if (logger->fp)
        {
            fprintf(logger->fp, "[LogX] Timer[%s] took %dh:%dm:%ds:%dms\n",
               t->name, h, m, s, ms);
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

static inline void logx_timer_auto_cleanup(logx_timer_t **t)
{
    if (*t)
        logx_timer_stop((*t)->logger, (*t)->name);
}


logx_timer_t *logx_timer_start_ptr(logx_t *logger, const char *name)
{
    logx_timer_start(logger, name);
    return find_timer(logger, name); // You already have find_timer_index()
}
