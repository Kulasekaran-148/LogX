#ifndef LOGXD_SESSION_H
#define LOGXD_SESSION_H

#include <time.h>
#include <sys/types.h>
#include "logx.h"

/* Opaque session type */
typedef struct logx_session logx_session_t;

/* Init / shutdown */
int  session_init(void);
void session_shutdown(void);

/* CRUD */
int  session_create(const char *session_id,
                    pid_t owner_pid,
                    const logx_cfg_t *cfg);

logx_t *session_get(const char *session_id);

int  session_destroy(const char *session_id);

/* Maintenance */
void session_touch(const char *session_id);
void session_cleanup_dead(void);

/* Debug */
void session_list(void (*cb)(const char *session_id, void *arg), void *arg);

#endif
