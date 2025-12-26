#ifndef _LOGX_CLI_H
#define _LOGX_CLI_H

#include "../logx/logx.h"

#define LOGXD_SOCK_PATH "/run/logxd.sock"

#define LOGX_IPC_MAGIC   0x58474F4C /* "LOGX" */
#define LOGX_IPC_VERSION 1

typedef enum {
    CMD_CREATE = 1,
    CMD_DESTROY,
    CMD_LOG,
    CMD_CFG,
    CMD_ROTATE_NOW,
    CMD_TIMER
} cmd_type_t;

typedef enum {
    LOGX_CFG_CONSOLE_LOGGING = 1,
    LOGX_CFG_FILE_LOGGING,
    LOGX_CFG_CONSOLE_LOG_LEVEL,
    LOGX_CFG_FILE_LOG_LEVEL,
    LOGX_CFG_COLORED_LOGGING,
    LOGX_CFG_TTY_DETECTION,
    LOGX_CFG_PRINT_CONFIG,
    LOGX_CFG_ROTATE_TYPE,
    LOGX_CFG_LOG_FILE_SIZE_MB,
    LOGX_CFG_ROTATION_INTERVAL_DAYS,
    LOGX_CFG_MAX_BACKUPS,
} cfg_keys_t;

typedef enum {
    TIMER_START = 1,
    TIMER_STOP,
    TIMER_PAUSE,
    TIMER_RESUME,
} timer_action_t;

typedef struct {
    uint32_t   magic;
    uint16_t   version;
    cmd_type_t cmd_type;
    uint32_t   pid;
    uint32_t   payload_len;
} ipc_hdr_t;

/* ---- Payloads ---- */

typedef struct {
    logx_level_t level;
    uint32_t     line_num;
    char         file_name[LOGX_MAX_CALLER_FILE_NAME_BYTES];
    char         message[LOGX_MAX_PAYLOAD_SIZE_BYTES];
} ipc_log_t;

typedef struct {
    cfg_keys_t key;
    uint32_t   value;
} ipc_cfg_t;

typedef struct {
    timer_action_t action;
    char           name[LOGX_MAX_TIMER_NAME_LEN_BYTES];
} ipc_timer_t;

typedef struct {
    ipc_hdr_t hdr;

    union {
        char        config_file_path[512];
        ipc_log_t   log;
        ipc_cfg_t   cfg;
        ipc_timer_t timer;
    } u;
} cmd_t;

#endif /* _LOGX_CLI_H */