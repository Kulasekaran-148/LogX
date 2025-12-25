#ifndef LOGXD_DISPATCHER_H
#define LOGXD_DISPATCHER_H

#include <sys/types.h>

/* IPC command result */
typedef struct {
    int  ok;                 /* 1 = success, 0 = error */
    char error_code[32];     /* e.g. LOGGER_NOT_FOUND */
    char error_msg[128];     /* human-readable */
} dispatch_result_t;

/* Parsed IPC message (from JSON layer) */
typedef struct {
    int     version;
    char    cmd[32];
    char    session[64];
    pid_t   pid;

    /* optional fields */
    char    level[16];
    char    message[512];
    char    file[128];
    char    func[64];
    int     line;

    char    key[64];
    char    value[64];

    char    timer_name[64];

    void   *config; /* pointer to parsed logx_cfg_t (or NULL) */
} ipc_request_t;

/* Entry point */
void dispatch_request(const ipc_request_t *req,
                      dispatch_result_t *res);

#endif
