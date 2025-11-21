/**
 * @file logx_timers.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-11-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "../include/logx/logx.h"
#include "../include/logx/logx_timers.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

static uint64_t diff_ns(const struct timespec *a, const struct timespec *b)
{
    return (a->tv_sec - b->tv_sec) * 1000000000ULL + (a->tv_nsec - b->tv_nsec);
}

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

static int find_timer_index(logx_t *logger, const char *name)
{
    for (int i = 0; i < logger->timer_count; i++)
        if (strcmp(logger->timers[i].name, name) == 0)
            return i;

    return -1;
}

void logx_timer_start(logx_t *logger, const char *name)
{
    if (!logger || !name)
        return;

    pthread_mutex_lock(&logger->lock);

    int idx = find_timer_index(logger, name);

    // If timer already exists
    if (idx >= 0)
    {
        fprintf(stderr, "[LogX]: Timer with same name already exists !\n");
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
        fprintf(stderr, "[LogX]: Max timer capacity reached. Can't create new timer\n");
        pthread_mutex_unlock(&logger->lock);
        return;
    }

    logx_timer_t *t = &logger->timers[logger->timer_count++];
    t->name         = name;
    clock_gettime(CLOCK_MONOTONIC, &t->start);
    t->accumulated_ns = 0;
    t->running        = 1;

    pthread_mutex_unlock(&logger->lock);
}

void logx_timer_pause(logx_t *log, const char *name)
{
    if (!log || !name) return;

    pthread_mutex_lock(&log->lock);

    int idx = find_timer_index(log, name);
    if (idx < 0)
    {
        pthread_mutex_unlock(&log->lock);
        return;
    }

    logx_timer_t *t = &log->timers[idx];

    if (!t->running)
    {
        pthread_mutex_unlock(&log->lock);
        return; // Already paused
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    t->accumulated_ns += diff_ns(&now, &t->start);
    t->running = 0;

    pthread_mutex_unlock(&log->lock);
}

void logx_timer_resume(logx_t *log, const char *name)
{
    if (!log || !name) return;

    pthread_mutex_lock(&log->lock);

    int idx = find_timer_index(log, name);
    if (idx < 0)
    {
        pthread_mutex_unlock(&log->lock);
        return;
    }

    logx_timer_t *t = &log->timers[idx];

    if (t->running)
    {
        pthread_mutex_unlock(&log->lock);
        return; // Already running
    }

    clock_gettime(CLOCK_MONOTONIC, &t->start);
    t->running = 1;

    pthread_mutex_unlock(&log->lock);
}

void logx_timer_stop(logx_t *log, const char *name)
{
    if (!log || !name) return;

    pthread_mutex_lock(&log->lock);

    int idx = find_timer_index(log, name);
    if (idx < 0)
    {
        pthread_mutex_unlock(&log->lock);
        return;
    }

    logx_timer_t *t = &log->timers[idx];
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
    if (log->cfg.enable_console_logging)
    {
        printf("[TIMER] %s took %dH:%dM:%dS:%dMS\n", 
               t->name, h, m, s, ms);
    }

    // Remove timer by shifting array left
    for (int i = idx; i < log->timer_count - 1; i++)
        log->timers[i] = log->timers[i + 1];

    log->timer_count--;

    pthread_mutex_unlock(&log->lock);
}