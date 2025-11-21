/**
 * @file logx_timers.h
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-11-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef _LOGX_TIMERS_H
#define _LOGX_TIMERS_H

#include <logx.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef LOGX_MAX_TIMERS
#define LOGX_MAX_TIMERS 5
#endif

#define LOGX_TIME_SCOPE(logger, name) \
    for (int _i = (logx_timer_start(logger, name), 0); !_i; \
         logx_timer_stop(logger, name), _i++)

/* Timer object */
typedef struct
{
    const char     *name;
    struct timespec start;
    uint64_t        accumulated_ns; // nanoseconds accumulated due to pauses
    bool            running;        // 1 if currently running
} logx_timer_t;

void logx_timer_start(logx_t *logger, const char *name);

void logx_timer_stop(logx_t *logger, const char *name);

void logx_timer_pause(logx_t *logger, const char *name);

void logx_timer_pause(logx_t *logger, const char *name);

#endif