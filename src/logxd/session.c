#define _GNU_SOURCE
#include "session.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

/* =========================
 * Internal structures
 * ========================= */

struct logx_session {
    char            *session_id;
    pid_t            owner_pid;
    time_t           last_seen;
    logx_t          *logger;
    struct logx_session *next;
};

/* =========================
 * Globals
 * ========================= */

static logx_session_t *session_head = NULL;
static pthread_mutex_t session_lock = PTHREAD_MUTEX_INITIALIZER;

/* =========================
 * Helpers
 * ========================= */

static logx_session_t *find_session(const char *session_id)
{
    logx_session_t *cur = session_head;

    while (cur) {
        if (strcmp(cur->session_id, session_id) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

static int is_pid_alive(pid_t pid)
{
    if (pid <= 0)
        return 0;

    if (kill(pid, 0) == 0)
        return 1;

    return (errno == EPERM);
}

/* =========================
 * Public API
 * ========================= */

int session_init(void)
{
    session_head = NULL;
    return 0;
}

void session_shutdown(void)
{
    pthread_mutex_lock(&session_lock);

    logx_session_t *cur = session_head;
    while (cur) {
        logx_session_t *next = cur->next;
        logx_destroy(cur->logger);
        free(cur->session_id);
        free(cur);
        cur = next;
    }

    session_head = NULL;
    pthread_mutex_unlock(&session_lock);
}

int session_create(const char *session_id,
                   pid_t owner_pid,
                   const logx_cfg_t *cfg)
{
    if (!session_id)
        return -1;

    pthread_mutex_lock(&session_lock);

    if (find_session(session_id)) {
        pthread_mutex_unlock(&session_lock);
        return -2; /* already exists */
    }

    logx_t *logger = logx_create(cfg);
    if (!logger) {
        pthread_mutex_unlock(&session_lock);
        return -3;
    }

    logx_session_t *s = calloc(1, sizeof(*s));
    if (!s) {
        logx_destroy(logger);
        pthread_mutex_unlock(&session_lock);
        return -4;
    }

    s->session_id = strdup(session_id);
    s->owner_pid  = owner_pid;
    s->last_seen  = time(NULL);
    s->logger     = logger;

    s->next = session_head;
    session_head = s;

    pthread_mutex_unlock(&session_lock);
    return 0;
}

logx_t *session_get(const char *session_id)
{
    logx_t *logger = NULL;

    pthread_mutex_lock(&session_lock);

    logx_session_t *s = find_session(session_id);
    if (s) {
        s->last_seen = time(NULL);
        logger = s->logger;
    }

    pthread_mutex_unlock(&session_lock);
    return logger;
}

int session_destroy(const char *session_id)
{
    pthread_mutex_lock(&session_lock);

    logx_session_t *prev = NULL;
    logx_session_t *cur  = session_head;

    while (cur) {
        if (strcmp(cur->session_id, session_id) == 0) {
            if (prev)
                prev->next = cur->next;
            else
                session_head = cur->next;

            logx_destroy(cur->logger);
            free(cur->session_id);
            free(cur);

            pthread_mutex_unlock(&session_lock);
            return 0;
        }

        prev = cur;
        cur = cur->next;
    }

    pthread_mutex_unlock(&session_lock);
    return -1;
}

void session_touch(const char *session_id)
{
    pthread_mutex_lock(&session_lock);

    logx_session_t *s = find_session(session_id);
    if (s)
        s->last_seen = time(NULL);

    pthread_mutex_unlock(&session_lock);
}

void session_cleanup_dead(void)
{
    pthread_mutex_lock(&session_lock);

    logx_session_t *prev = NULL;
    logx_session_t *cur  = session_head;

    while (cur) {
        if (!is_pid_alive(cur->owner_pid)) {
            logx_session_t *dead = cur;

            if (prev)
                prev->next = cur->next;
            else
                session_head = cur->next;

            cur = cur->next;

            logx_destroy(dead->logger);
            free(dead->session_id);
            free(dead);
            continue;
        }

        prev = cur;
        cur = cur->next;
    }

    pthread_mutex_unlock(&session_lock);
}

void session_list(void (*cb)(const char *session_id, void *arg), void *arg)
{
    if (!cb)
        return;

    pthread_mutex_lock(&session_lock);

    logx_session_t *cur = session_head;
    while (cur) {
        cb(cur->session_id, arg);
        cur = cur->next;
    }

    pthread_mutex_unlock(&session_lock);
}
