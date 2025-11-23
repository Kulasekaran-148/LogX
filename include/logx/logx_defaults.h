/**
 * @file logx_defaults.h
 * @author kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This file defines the default logx logger configuration macros
 * @version 0.1
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef LOGX_DEFAULTS_H
#define LOGX_DEFAUTLS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Default LogX Configuration MACROS */

#ifndef LOGX_DEFAULT_CFG_NAME
#define LOGX_DEFAULT_CFG_NAME  "LogX_Default"
#endif

#ifndef LOGX_DEFAULT_CFG_LOGFILE_PATH
#define LOGX_DEFAULT_CFG_LOGFILE_PATH  "./logx.log"
#endif

#ifndef LOGX_DEFAULT_CFG_CONSOLE_LEVEL
#define LOGX_DEFAULT_CFG_CONSOLE_LEVEL  LOGX_LEVEL_TRACE
#endif

#ifndef LOGX_DEFAULT_CFG_FILE_LEVEL
#define LOGX_DEFAULT_CFG_FILE_LEVEL     LOGX_LEVEL_TRACE
#endif

#ifndef LOGX_DEFAULT_CFG_ENABLE_CONSOLE_LOGGING
#define LOGX_DEFAULT_CFG_ENABLE_CONSOLE_LOGGING  true
#endif

#ifndef LOGX_DEFAULT_CFG_ENABLE_FILE_LOGGING
#define LOGX_DEFAULT_CFG_ENABLE_FILE_LOGGING     true
#endif

#ifndef LOGX_DEFAULT_CFG_ENABLE_COLORED_LOGGING
#define LOGX_DEFAULT_CFG_ENABLE_COLORED_LOGGING  true
#endif

#ifndef LOGX_DEFAULT_CFG_ENABLE_TTY_DETECTION
#define LOGX_DEFAULT_CFG_ENABLE_TTY_DETECTION    true
#endif

#ifndef LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE
#define LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE         LOGX_ROTATE_BY_SIZE
#endif

#ifndef LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_SIZE_BYTES
#define LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_SIZE_BYTES  (10 * 1024 * 1024)
#endif

#ifndef LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_NUM_BACKUPS
#define LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_NUM_BACKUPS  3
#endif

#ifndef LOGX_DEFAULT_CFG_LOG_ROTATE_DAILY_INTERVAL
#define LOGX_DEFAULT_CFG_LOG_ROTATE_DAILY_INTERVAL   1
#endif

#ifndef LOGX_DEFAULT_CFG_BANNER_PATTERN
#define LOGX_DEFAULT_CFG_BANNER_PATTERN              "="
#endif

#ifndef LOGX_DEFAULT_CFG_PRINT_CONFIG
#define LOGX_DEFAULT_CFG_PRINT_CONFIG                true
#endif


/* Default LogX Configuration file paths */
#define LOGX_DEFAULT_CFG_YML_FILEPATH  "./logx_cfg.yml"
#define LOGX_DEFAULT_CFG_YAML_FILEPATH "./logx_cfg.yaml"
#define LOGX_DEFAULT_CFG_JSON_FILEPATH "./logx_cfg.json"

#ifdef __cplusplus
}
#endif

#endif /* LOGX_DEFAULTS_H */