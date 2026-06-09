/**
 * @file logx_config.c
 * @author kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This file declares the keys used by yaml and json parsers
 * @version 0.1
 * @date 2025-11-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "logx_config.h"
#include "logx_common.h"
#include <cJSON/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

//clang-format off
const logx_field_desc_t LOGX_FIELD_TABLE[] = {
    {LOGX_KEY_NAME,
     "logx",
     LOGX_FIELD_STRING,
     offsetof(logx_cfg_t, name),
     {.str_default = LOGX_DEFAULT_CFG_NAME}},
    {LOGX_KEY_FILE_PATH,
     "logx",
     LOGX_FIELD_STRING,
     offsetof(logx_cfg_t, file_path),
     {.str_default = LOGX_DEFAULT_CFG_LOGFILE_PATH}},
    {LOGX_KEY_ENABLE_CONSOLE_LOGGING,
     "logx",
     LOGX_FIELD_BOOL,
     offsetof(logx_cfg_t, enable_console_logging),
     {.int_default = LOGX_DEFAULT_CFG_ENABLE_CONSOLE_LOGGING}},
    {LOGX_KEY_ENABLE_FILE_LOGGING,
     "logx",
     LOGX_FIELD_BOOL,
     offsetof(logx_cfg_t, enable_file_logging),
     {.int_default = LOGX_DEFAULT_CFG_ENABLE_FILE_LOGGING}},
    {LOGX_KEY_ENABLE_COLORED_LOGS,
     "logx",
     LOGX_FIELD_BOOL,
     offsetof(logx_cfg_t, enable_colored_logs),
     {.int_default = LOGX_DEFAULT_CFG_ENABLE_COLORED_LOGGING}},
    {LOGX_KEY_USE_TTY_DETECTION,
     "logx",
     LOGX_FIELD_BOOL,
     offsetof(logx_cfg_t, use_tty_detection),
     {.int_default = LOGX_DEFAULT_CFG_ENABLE_TTY_DETECTION}},
    {LOGX_KEY_CONSOLE_LEVEL,
     "logx",
     LOGX_FIELD_LEVEL,
     offsetof(logx_cfg_t, console_level),
     {.int_default = LOGX_DEFAULT_CFG_CONSOLE_LEVEL}},
    {LOGX_KEY_FILE_LEVEL,
     "logx",
     LOGX_FIELD_LEVEL,
     offsetof(logx_cfg_t, file_level),
     {.int_default = LOGX_DEFAULT_CFG_FILE_LEVEL}},
    {LOGX_KEY_ROTATE_TYPE,
     "logx",
     LOGX_FIELD_ROTATE_TYPE,
     offsetof(logx_cfg_t, rotate.type),
     {.int_default = LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE}},
    {LOGX_KEY_MAX_LOGFILE_SIZE_MB,
     "logx",
     LOGX_FIELD_INT,
     offsetof(logx_cfg_t, rotate.size_mb),
     {.int_default = LOGX_DEFAULT_CFG_MAX_LOGFILE_SIZE_MB}},
    {LOGX_KEY_MAX_LOGFILE_BACKUPS,
     "logx",
     LOGX_FIELD_INT,
     offsetof(logx_cfg_t, rotate.max_backups),
     {.int_default = LOGX_DEFAULT_CFG_MAX_LOGFILE_BACKUPS}},
    {LOGX_KEY_ROTATE_AFTER_DAYS,
     "logx",
     LOGX_FIELD_INT,
     offsetof(logx_cfg_t, rotate.after_days),
     {.int_default = LOGX_DEFAULT_CFG_LOG_ROTATE_AFTER_DAYS}},
    {LOGX_KEY_BANNER_PATTERN,
     "logx",
     LOGX_FIELD_STRING,
     offsetof(logx_cfg_t, banner_pattern),
     {.str_default = LOGX_DEFAULT_CFG_BANNER_PATTERN}},
    {LOGX_KEY_PRINT_CONFIG,
     "logx",
     LOGX_FIELD_BOOL,
     offsetof(logx_cfg_t, print_config),
     {.int_default = LOGX_DEFAULT_CFG_PRINT_CONFIG}},
    {LOGX_KEY_TIMESTAMP_FORMAT,
     "logx",
     LOGX_FIELD_TS_FMT,
     offsetof(logx_cfg_t, ts_format),
     {.int_default = LOGX_DEFAULT_CFG_TIMESTAMP_FORMAT}},
    {LOGX_KEY_ENABLE_SYSLOG,
     "logx",
     LOGX_FIELD_BOOL,
     offsetof(logx_cfg_t, enable_syslog),
     {.int_default = LOGX_DEFAULT_CFG_ENABLE_SYSLOG}},
    {LOGX_KEY_SYSLOG_FACILITY,
     "logx",
     LOGX_FIELD_SYSLOG_FACILITY,
     offsetof(logx_cfg_t, syslog_facility),
     {.int_default = LOGX_DEFAULT_CFG_SYSLOG_FACILITY}},
    {LOGX_KEY_SYSLOG_IDENT,
     "logx",
     LOGX_FIELD_STRING,
     offsetof(logx_cfg_t, syslog_ident),
     {.str_default = LOGX_DEFAULT_CFG_SYSLOG_IDENT}},
};
//clang-format on

const size_t LOGX_FIELD_TABLE_COUNT = ARRAY_SIZE(LOGX_FIELD_TABLE);

static const char *logx_syslog_facility_to_string(logx_syslog_facility_t val);

void log_missing_json_keys(cJSON *root)
{
    for (size_t i = 0; i < LOGX_FIELD_TABLE_COUNT; i++)
        if (!cJSON_HasObjectItem(root, LOGX_FIELD_TABLE[i].key))
            fprintf(stderr, "missing: %s\n", LOGX_FIELD_TABLE[i].key);
}

void logx_cfg_print(const logx_cfg_t *cfg)
{
    if (!cfg)
        return;

    printf("\n");
    printf("┌─────────────────────────────────────────────────┐\n");
    printf("│              LogX Active Configuration          │\n");
    printf("└─────────────────────────────────────────────────┘\n");

    for (size_t i = 0; i < LOGX_FIELD_TABLE_COUNT; i++)
    {
        const logx_field_desc_t *desc = &LOGX_FIELD_TABLE[i];
        const void *field_ptr         = (const char *)cfg + desc->offset;

        printf("  %-35s : ", desc->key);

        switch (desc->type)
        {
            case LOGX_FIELD_STRING:
            {
                const char *s = *(const char **)field_ptr;
                printf("%s\n", s ? s : "(null)");
                break;
            }
            case LOGX_FIELD_BOOL:
            {
                printf("%s\n", *(const int *)field_ptr ? "true" : "false");
                break;
            }
            case LOGX_FIELD_INT:
            {
                printf("%d\n", *(const int *)field_ptr);
                break;
            }
            case LOGX_FIELD_LEVEL:
            {
                printf("%s\n", logx_level_to_string(*(const logx_level_t *)field_ptr));
                break;
            }
            case LOGX_FIELD_ROTATE_TYPE:
            {
                printf("%s\n", logx_rotate_type_to_string(*(const logx_rotate_type_t *)field_ptr));
                break;
            }
            case LOGX_FIELD_TS_FMT:
            {
                printf("%s\n", logx_ts_fmt_to_string(*(const logx_ts_fmt_t *)field_ptr));
                break;
            }
            case LOGX_FIELD_SYSLOG_FACILITY:
            {
                printf("%s\n",
                       logx_syslog_facility_to_string(*(const logx_syslog_facility_t *)field_ptr));
                break;
            }
        }
    }

    printf("\n");
}

const char *logx_level_to_string(logx_level_t eLogLevel)
{
    for (size_t i = 0; i < LOGX_LEVEL_MAP_COUNT; i++)
        if (LOGX_LEVEL_MAP[i].val == eLogLevel)
            return LOGX_LEVEL_MAP[i].abbr;
    return "ukwn";
}

const char *logx_rotate_type_to_string(logx_rotate_type_t eRotateType)
{
    for (size_t i = 0; i < LOGX_ROTATE_MAP_COUNT; i++)
        if (LOGX_ROTATE_MAP[i].val == eRotateType)
            return LOGX_ROTATE_MAP[i].disp;
    return "ukwn";
}

const char *logx_ts_fmt_to_string(logx_ts_fmt_t eTsFormat)
{
    for (size_t i = 0; i < LOGX_TS_FMT_MAP_COUNT; i++)
        if (LOGX_TS_FMT_MAP[i].val == eTsFormat)
            return LOGX_TS_FMT_MAP[i].name;
    return "ukwn";
}

static logx_level_t logx_level_from_str(const char *str, logx_level_t fallback)
{
    if (!str)
        return fallback;
    for (size_t i = 0; i < LOGX_LEVEL_MAP_COUNT; i++)
        if (strcasecmp(str, LOGX_LEVEL_MAP[i].name) == 0)
            return LOGX_LEVEL_MAP[i].val;
    fprintf(stderr, "[LogX] Unknown level '%s', using default.\n", str);
    return fallback;
}

static logx_rotate_type_t logx_rotate_type_from_str(const char *str, logx_rotate_type_t fallback)
{
    if (!str)
        return fallback;
    for (size_t i = 0; i < LOGX_ROTATE_MAP_COUNT; i++)
        if (strcasecmp(str, LOGX_ROTATE_MAP[i].name) == 0)
            return LOGX_ROTATE_MAP[i].val;
    fprintf(stderr, "[LogX] Unknown rotate_type '%s', using default.\n", str);
    return fallback;
}

static logx_ts_fmt_t logx_ts_fmt_from_str(const char *str, logx_ts_fmt_t fallback)
{
    if (!str)
        return fallback;
    for (size_t i = 0; i < LOGX_TS_FMT_MAP_COUNT; i++)
        if (strcasecmp(str, LOGX_TS_FMT_MAP[i].name) == 0)
            return LOGX_TS_FMT_MAP[i].val;
    fprintf(stderr, "[LogX] Unknown ts_format '%s', using default.\n", str);
    return fallback;
}

typedef struct
{
    logx_syslog_facility_t val;
    const char *name;
} logx_syslog_facility_entry_t;

static const logx_syslog_facility_entry_t SYSLOG_FACILITY_MAP[] = {
    {LOGX_SYSLOG_FACILITY_USER, "USER"},     {LOGX_SYSLOG_FACILITY_DAEMON, "DAEMON"},
    {LOGX_SYSLOG_FACILITY_LOCAL0, "LOCAL0"}, {LOGX_SYSLOG_FACILITY_LOCAL1, "LOCAL1"},
    {LOGX_SYSLOG_FACILITY_LOCAL2, "LOCAL2"}, {LOGX_SYSLOG_FACILITY_LOCAL3, "LOCAL3"},
    {LOGX_SYSLOG_FACILITY_LOCAL4, "LOCAL4"}, {LOGX_SYSLOG_FACILITY_LOCAL5, "LOCAL5"},
    {LOGX_SYSLOG_FACILITY_LOCAL6, "LOCAL6"}, {LOGX_SYSLOG_FACILITY_LOCAL7, "LOCAL7"},
};
#define SYSLOG_FACILITY_MAP_COUNT ARRAY_SIZE(SYSLOG_FACILITY_MAP)

static const char *logx_syslog_facility_to_string(logx_syslog_facility_t val)
{
    for (size_t i = 0; i < SYSLOG_FACILITY_MAP_COUNT; i++)
        if (SYSLOG_FACILITY_MAP[i].val == val)
            return SYSLOG_FACILITY_MAP[i].name;
    return "ukwn";
}

static logx_syslog_facility_t logx_syslog_facility_from_str(const char *str,
                                                            logx_syslog_facility_t fallback)
{
    if (!str)
        return fallback;
    for (size_t i = 0; i < SYSLOG_FACILITY_MAP_COUNT; i++)
        if (strcasecmp(str, SYSLOG_FACILITY_MAP[i].name) == 0)
            return SYSLOG_FACILITY_MAP[i].val;
    fprintf(stderr, "[LogX] Unknown syslog_facility '%s', using default.\n", str);
    return fallback;
}

void logx_apply_field(logx_cfg_t *cfg, const logx_field_desc_t *desc, const char *str_val,
                      int int_val, bool found)
{
    void *field_ptr = (char *)cfg + desc->offset;

    if (!found)
    {
        /* Apply default */
        if (desc->type == LOGX_FIELD_STRING)
            *(char **)field_ptr = desc->def.str_default ? strdup(desc->def.str_default) : NULL;
        else
            *(int *)field_ptr = desc->def.int_default;
        return;
    }

    switch (desc->type)
    {
        case LOGX_FIELD_STRING:
            *(char **)field_ptr = str_val ? strdup(str_val) : strdup(desc->def.str_default);
            break;
        case LOGX_FIELD_BOOL:
        case LOGX_FIELD_INT:
            *(int *)field_ptr = int_val;
            break;
        case LOGX_FIELD_LEVEL:
            *(logx_level_t *)field_ptr =
                logx_level_from_str(str_val, (logx_level_t)desc->def.int_default);
            break;
        case LOGX_FIELD_ROTATE_TYPE:
            *(logx_rotate_type_t *)field_ptr =
                logx_rotate_type_from_str(str_val, (logx_rotate_type_t)desc->def.int_default);
            break;
        case LOGX_FIELD_TS_FMT:
            *(logx_ts_fmt_t *)field_ptr =
                logx_ts_fmt_from_str(str_val, (logx_ts_fmt_t)desc->def.int_default);
            break;
        case LOGX_FIELD_SYSLOG_FACILITY:
            *(logx_syslog_facility_t *)field_ptr = logx_syslog_facility_from_str(
                str_val, (logx_syslog_facility_t)desc->def.int_default);
            break;
    }
}

void logx_cfg_dup_strings(logx_cfg_t *cfg)
{
    if (!cfg)
        return;
    for (size_t i = 0; i < LOGX_FIELD_TABLE_COUNT; i++)
    {
        if (LOGX_FIELD_TABLE[i].type != LOGX_FIELD_STRING)
            continue;
        char **field = (char **)((char *)cfg + LOGX_FIELD_TABLE[i].offset);
        if (*field)
            *field = strdup(*field);
    }
}

void logx_cfg_free_strings(logx_cfg_t *cfg)
{
    if (!cfg)
        return;
    for (size_t i = 0; i < LOGX_FIELD_TABLE_COUNT; i++)
    {
        if (LOGX_FIELD_TABLE[i].type != LOGX_FIELD_STRING)
            continue;
        char **field = (char **)((char *)cfg + LOGX_FIELD_TABLE[i].offset);
        free(*field);
        *field = NULL;
    }
}