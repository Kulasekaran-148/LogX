#define _GNU_SOURCE
#include "reaper.h"
#include "session.h"

#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

/* =========================
 * Configuration
 * ========================= */

#define REAPER_INTERVAL_SEC 5

/* =========================
 * Globals
 * ========================= */

static pthread_t reaper_tid;
static volatile sig_atomic_t reaper_running = 0;

/* =========================
 * Reaper thread
 * ========================= */

static void *reaper_thread(void *arg)
{
    (void)arg;

    while (reaper_running) {
        sleep(REAPER_INTERVAL_SEC);
        session_cleanup_dead();
    }

    return NULL;
}

/* =========================
 * Public API
 * ========================= */

int reaper_start(void)
{
    if (reaper_running)
        return 0;

    reaper_running = 1;

    if (pthread_create(&reaper_tid, NULL, reaper_thread, NULL) != 0) {
        reaper_running = 0;
        perror("reaper pthread_create");
        return -1;
    }

    return 0;
}

void reaper_stop(void)
{
    if (!reaper_running)
        return;

    reaper_running = 0;
    pthread_join(reaper_tid, NULL);
}
