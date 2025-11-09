#ifndef LOGX_DEFAULTS_H
#define LOGX_DEFAUTLS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Default LogX Configuration MACROS */
#define LOGX_DEFAULT_CFG_NAME                       "LogX_Default"
#define LOGX_DEFAULT_CFG_PATH                       "./logx.log"
#define LOGX_DEFAULT_CFG_CONSOLE_LEVEL              LOG_LEVEL_INFO
#define LOGX_DEFAULT_CFG_FILE_LEVEL                 LOG_LEVEL_DEBUG
#define LOGX_DEFAULT_CFG_ENABLE_CONSOLE_LOGGING     true
#define LOGX_DEFAULT_CFG_ENABLE_FILE_LOGGING        true
#define LOGX_DEFAULT_CFG_ENABLE_COLORED_LOGGING     true
#define LOGX_DEFAULT_CFG_ENABLE_TTY_DETECTION       true
#define LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE            LOG_ROTATE_BY_SIZE
#define LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_SIZE_BYTES  10 * 1024 * 1024 // 10mb
#define LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_NUM_BACKUPS 3
#define LOGX_DEFAULT_CFG_LOG_ROTATE_DAILY_INTERVAL  1
#define LOGX_DEFAULT_CFG_BANNER_PATTERN             "=-"

/* Default LogX Configuration file paths */
#define LOGX_DEFAULT_CFG_YML_FILEPATH  "logx_cfg.yml"
#define LOGX_DEFAULT_CFG_YAML_FILEPATH "logx_cfg.yaml"
#define LOGX_DEFAULT_CFG_JSON_FILEPATH "logx_cfg.json"

#ifdef __cplusplus
}
#endif

#endif /* LOGX_DEFAULTS_H */