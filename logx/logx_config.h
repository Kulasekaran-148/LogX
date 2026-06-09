/**
 * @file logx_config.h
 * @author kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This file defines the keys used in configuration files for logx logger
 * @version 0.1
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef LOGX_CONFIG_KEYS_H
#define LOGX_CONFIG_KEYS_H

#include "cJSON/cJSON.h"
#include "logx.h"
#include <stddef.h>

#ifndef LOGX_DEFAULT_CFG_NAME
#define LOGX_DEFAULT_CFG_NAME "LogX_Default"
#endif

#ifndef LOGX_DEFAULT_CFG_LOGFILE_PATH
#define LOGX_DEFAULT_CFG_LOGFILE_PATH "./logx.log"
#endif

#ifndef LOGX_DEFAULT_CFG_CONSOLE_LEVEL
#define LOGX_DEFAULT_CFG_CONSOLE_LEVEL LOGX_LEVEL_TRACE
#endif

#ifndef LOGX_DEFAULT_CFG_FILE_LEVEL
#define LOGX_DEFAULT_CFG_FILE_LEVEL LOGX_LEVEL_TRACE
#endif

#ifndef LOGX_DEFAULT_CFG_ENABLE_CONSOLE_LOGGING
#define LOGX_DEFAULT_CFG_ENABLE_CONSOLE_LOGGING 1
#endif

#ifndef LOGX_DEFAULT_CFG_ENABLE_FILE_LOGGING
#define LOGX_DEFAULT_CFG_ENABLE_FILE_LOGGING 1
#endif

#ifndef LOGX_DEFAULT_CFG_ENABLE_COLORED_LOGGING
#define LOGX_DEFAULT_CFG_ENABLE_COLORED_LOGGING 1
#endif

#ifndef LOGX_DEFAULT_CFG_ENABLE_TTY_DETECTION
#define LOGX_DEFAULT_CFG_ENABLE_TTY_DETECTION 1
#endif

#ifndef LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE
#define LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE LOGX_ROTATE_BY_SIZE
#endif

#ifndef LOGX_DEFAULT_CFG_MAX_LOGFILE_SIZE_MB
#define LOGX_DEFAULT_CFG_MAX_LOGFILE_SIZE_MB 10
#endif

#ifndef LOGX_DEFAULT_CFG_MAX_LOGFILE_BACKUPS
#define LOGX_DEFAULT_CFG_MAX_LOGFILE_BACKUPS 3
#endif

#ifndef LOGX_DEFAULT_CFG_LOG_ROTATE_AFTER_DAYS
#define LOGX_DEFAULT_CFG_LOG_ROTATE_AFTER_DAYS 1
#endif

#ifndef LOGX_DEFAULT_CFG_BANNER_PATTERN
#define LOGX_DEFAULT_CFG_BANNER_PATTERN "="
#endif

#ifndef LOGX_DEFAULT_CFG_PRINT_CONFIG
#define LOGX_DEFAULT_CFG_PRINT_CONFIG 1
#endif

#ifndef LOGX_DEFAULT_CFG_TIMESTAMP_FORMAT
#define LOGX_DEFAULT_CFG_TIMESTAMP_FORMAT LOGX_TS_FMT_LOCAL
#endif

#ifndef LOGX_DEFAULT_CFG_ENABLE_SYSLOG
#define LOGX_DEFAULT_CFG_ENABLE_SYSLOG 0
#endif

#ifndef LOGX_DEFAULT_CFG_SYSLOG_FACILITY
#define LOGX_DEFAULT_CFG_SYSLOG_FACILITY LOGX_SYSLOG_FACILITY_USER
#endif

#ifndef LOGX_DEFAULT_CFG_SYSLOG_IDENT
#define LOGX_DEFAULT_CFG_SYSLOG_IDENT NULL
#endif

/* Default LogX Configuration file paths */
#define LOGX_DEFAULT_CFG_YML_FILEPATH  "./logx_cfg.yml"
#define LOGX_DEFAULT_CFG_YAML_FILEPATH "./logx_cfg.yaml"
#define LOGX_DEFAULT_CFG_JSON_FILEPATH "./logx_cfg.json"

#define LOGX_KEY_NAME                   "name"
#define LOGX_KEY_FILE_PATH              "logfile_path"
#define LOGX_KEY_ENABLE_CONSOLE_LOGGING "enable_console_logging"
#define LOGX_KEY_ENABLE_FILE_LOGGING    "enable_file_logging"
#define LOGX_KEY_ENABLE_COLORED_LOGS    "enable_colored_logs"
#define LOGX_KEY_USE_TTY_DETECTION      "use_tty_detection"
#define LOGX_KEY_CONSOLE_LEVEL          "console_level"
#define LOGX_KEY_FILE_LEVEL             "file_level"
#define LOGX_KEY_ROTATE_TYPE            "rotate_type"
#define LOGX_KEY_MAX_LOGFILE_SIZE_MB    "max_logfile_size_mb"
#define LOGX_KEY_MAX_LOGFILE_BACKUPS    "max_logfile_backups"
#define LOGX_KEY_ROTATE_AFTER_DAYS      "rotate_after_days"
#define LOGX_KEY_BANNER_PATTERN         "banner_pattern"
#define LOGX_KEY_PRINT_CONFIG           "print_config"
#define LOGX_KEY_TIMESTAMP_FORMAT       "timestamp_format"
#define LOGX_KEY_ENABLE_SYSLOG          "enable_syslog"
#define LOGX_KEY_SYSLOG_FACILITY        "syslog_facility"
#define LOGX_KEY_SYSLOG_IDENT           "syslog_ident"

typedef struct
{
    const char *key;     /* JSON/YAML/INI key name */
    const char *section; /* Used by INI parser only */
    logx_field_type_t type;
    size_t offset; /* offsetof(logx_cfg_t, field) */
    union
    {
        const char *str_default;
        int int_default;
    } def;
} logx_field_desc_t;

/* Declared here, defined once in logx_config.c */
extern const logx_field_desc_t LOGX_FIELD_TABLE[];
extern const size_t LOGX_FIELD_TABLE_COUNT;

/* Functions */
void log_missing_json_keys(cJSON *root);
void logx_cfg_print(const logx_cfg_t *cfg);
void logx_apply_field(logx_cfg_t *cfg, const logx_field_desc_t *desc, const char *str_val,
                      int int_val, bool found);
void logx_cfg_dup_strings(logx_cfg_t *cfg);
void logx_cfg_free_strings(logx_cfg_t *cfg);
const char *logx_level_to_string(logx_level_t eLogLevel);
const char *logx_rotate_type_to_string(logx_rotate_type_t eRotateType);
const char *logx_ts_fmt_to_string(logx_ts_fmt_t eTsFormat);

#endif /* LOGX_CONFIG_KEYS_H */
