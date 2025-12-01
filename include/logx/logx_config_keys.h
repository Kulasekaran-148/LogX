/**
 * @file logx_config_keys.h
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

#include <cjson/cJSON.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------------
 * LogX JSON Configuration Keys
 * -----------------------------------------------------------------------------
 * These constants represent the JSON keys expected in the LogX configuration
 * file. They are declared as extern so they can be used across multiple source
 * files without causing multiple definition errors.
 *
 * The actual definitions (with assigned string literals) should be placed in
 * logx_config_keys.c
 * --------------------------------------------------------------------------- */

extern const char *LOGX_KEY_NAME;
extern const char *LOGX_KEY_FILE_PATH;
extern const char *LOGX_KEY_ENABLE_CONSOLE_LOGGING;
extern const char *LOGX_KEY_ENABLE_FILE_LOGGING;
extern const char *LOGX_KEY_enable_colored_logs;
extern const char *LOGX_KEY_USE_TTY_DETECTION;
extern const char *LOGX_KEY_BANNER_PATTERN;
extern const char *LOGX_KEY_PRINT_CONFIG;

extern const char *LOGX_KEY_CONSOLE_LEVEL;
extern const char *LOGX_KEY_FILE_LEVEL;

extern const char *LOGX_KEY_ROTATE_TYPE;
extern const char *LOGX_KEY_ROTATE_MAX_MBYTES;
extern const char *LOGX_KEY_ROTATE_MAX_BACKUPS;
extern const char *LOGX_KEY_ROTATE_INTERVAL_DAYS;

extern const size_t LOGX_CONFIG_KEY_COUNT;

typedef struct {
    const char *key;
    const char *description;
} logx_config_key_entry_t;

extern const logx_config_key_entry_t LOGX_CONFIG_KEYS[];

/* Function Declarations */
void log_missing_json_keys(cJSON *root);

#ifdef __cplusplus
}
#endif

#endif /* LOGX_CONFIG_KEYS_H */
