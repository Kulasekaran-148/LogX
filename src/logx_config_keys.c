/**
 * @file logx_config_keys.c
 * @author kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This file declares the keys used by yaml and json parsers
 * @version 0.1
 * @date 2025-11-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "../include/logx/logx_config_keys.h"

#include <cjson/cJSON.h>
#include <stdio.h>

const char *LOGX_KEY_NAME                   = "name";
const char *LOGX_KEY_FILE_PATH              = "file_path";
const char *LOGX_KEY_ENABLE_CONSOLE_LOGGING = "enable_console_logging";
const char *LOGX_KEY_ENABLE_FILE_LOGGING    = "enable_file_logging";
const char *LOGX_KEY_ENABLED_COLORED_LOGS   = "enabled_colored_logs";
const char *LOGX_KEY_USE_TTY_DETECTION      = "use_tty_detection";
const char *LOGX_KEY_BANNER_PATTERN         = "banner_pattern";

const char *LOGX_KEY_CONSOLE_LEVEL = "console_level";
const char *LOGX_KEY_FILE_LEVEL    = "file_level";

const char *LOGX_KEY_ROTATE_TYPE           = "rotate_type";
const char *LOGX_KEY_ROTATE_MAX_MBYTES     = "rotate_max_Mbytes";
const char *LOGX_KEY_ROTATE_MAX_BACKUPS    = "rotate_max_backups";
const char *LOGX_KEY_ROTATE_DAILY_INTERVAL = "rotate_daily_interval";

/* Array of known keys for validation and debugging */
const logx_config_key_entry_t LOGX_CONFIG_KEYS[] = {
    {                  "name",                          "Logger name"},
    {             "file_path",                 "Log file output path"},
    {"enable_console_logging",     "Enable or disable console output"},
    {   "enable_file_logging",        "Enable or disable file output"},
    {  "enabled_colored_logs",       "Enable or disable colored logs"},
    {     "use_tty_detection",     "Enable TTY detection for console"},
    {        "banner_pattern",  "Banner header pattern for log start"},
    {         "console_level",                    "Console log level"},
    {            "file_level",                       "File log level"},
    {           "rotate_type",   "Rotation type (BY_SIZE or BY_DATE)"},
    {     "rotate_max_Mbytes",    "Maximum file size before rotation"},
    {    "rotate_max_backups",      "Maximum number of rotated files"},
    { "rotate_daily_interval", "Interval (in days) for date rotation"},
};

/* Expose count for iteration */
const size_t LOGX_CONFIG_KEY_COUNT = sizeof(LOGX_CONFIG_KEYS) / sizeof(LOGX_CONFIG_KEYS[0]);

/**
 * @brief Logs any missing json keys in the configuration file
 *
 * @param[in] root cJSON type object which is filled with parsed information. If at this point, any
 * of the information is not present, then it means it must've hit some error while parsing or the
 * key must be invlaid in the provided configuration file
 */
void log_missing_json_keys(cJSON *root)
{
    for (size_t i = 0; i < LOGX_CONFIG_KEY_COUNT; ++i)
    {
        const char *key = LOGX_CONFIG_KEYS[i].key;
        if (!cJSON_GetObjectItem(root, key))
        {
            fprintf(stderr, "[LogX] Missing key: %-25s (%s)\n", key,
                    LOGX_CONFIG_KEYS[i].description);
        }
    }
}
