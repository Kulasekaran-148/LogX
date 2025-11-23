/**
 * @file logx.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Core logx source
 * @version 0.1
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 *
 */

#define _POSIX_C_SOURCE 200809L

#include "../include/logx/logx.h"

#include "../include/logx/logx_config_keys.h"
#include "../include/logx/logx_defaults.h"

#include <cjson/cJSON.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <yaml.h>

/* ANSI color codes */

/*
Different consoles render these codes at different values
\x1b[30m	foreground black
\x1b[31m	foreground red
\x1b[32m	foreground green
\x1b[33m	foreground yellow
\x1b[34m	foreground blue
\x1b[35m	foreground magenta
\x1b[36m	foreground cyan
\x1b[37m	foreground white
\x1b[40m	background black
\x1b[41m	background red
\x1b[42m	background green
\x1b[43m	background yellow
\x1b[44m	background blue
\x1b[45m	background magenta
\x1b[46m	background cyan
\x1b[47m	background white
*/

static const char *COLOR_TRACE  = "\x1b[34m"; /* blue */
static const char *COLOR_DEBUG  = "\x1b[37m"; /* white */
static const char *COLOR_INFO   = "\x1b[32m"; /* green */
static const char *COLOR_WARN   = "\x1b[33m"; /* yellow */
static const char *COLOR_ERROR  = "\x1b[31m"; /* red */
static const char *COLOR_BANNER = "\x1b[36m"; /* cyan */
static const char *COLOR_FATAL  = "\x1b[35m"; /* purple */
static const char *COLOR_RESET  = "\x1b[0m";

/**
 * @brief Validates if the logx_rotate_type_t passed is valid or not
 *
 * @param[in] type rotate type to be validated
 * @return int 0 on Success, -1 on Failure
 */
int is_valid_logx_rotate_type(logx_rotate_type_t type)
{
    switch (type)
    {
    case LOGX_ROTATE_NONE:
    case LOGX_ROTATE_BY_SIZE:
    case LOGX_ROTATE_BY_DATE: return 0;
    default: return -1;
    }
    return -1;
}

/**
 * @brief Validates if the logx_level_t passed is valid or not
 *
 * @param[in] level logx level to be validated
 * @return int 0 on Success, -1 on Failure
 */
int is_valid_logx_level(logx_level_t level)
{
    switch (level)
    {
    case LOGX_LEVEL_TRACE:
    case LOGX_LEVEL_DEBUG:
    case LOGX_LEVEL_BANNER:
    case LOGX_LEVEL_INFO:
    case LOGX_LEVEL_WARN:
    case LOGX_LEVEL_ERROR:
    case LOGX_LEVEL_FATAL:
    case LOGX_LEVEL_OFF: return 0;
    default: return -1;
    }
    return -1;
}

/**
 * @brief Converts a log level enum to its corresponding short string representation.
 *
 * This function maps a logx_level_t value to a three-character string used in log messages.
 * It is typically used in the logging system to display the log level in a compact format.
 *
 * @param level The log level to convert.
 *
 * @return const char* Short string representing the log level. Never NULL.
 */
const char *logx_level_to_string(logx_level_t level)
{
    switch (level)
    {
    case LOGX_LEVEL_TRACE: return "TRC";
    case LOGX_LEVEL_DEBUG: return "DBG";
    case LOGX_LEVEL_INFO: return "INF";
    case LOGX_LEVEL_WARN: return "WRN";
    case LOGX_LEVEL_ERROR: return "ERR";
    case LOGX_LEVEL_BANNER: return "BNR";
    case LOGX_LEVEL_FATAL: return "FTL";
    default: return "MSC";
    }
}

/**
 * @brief Converts a log rotate type enum to its corresponding string representation.
 *
 * This function maps a logx_rotate_type_t value to a string used in log messages.
 *
 * @param[in] type The log rotate type to convert.
 *
 * @return const char* string representing the log rotate type. Never NULL.
 */
const char *logx_rotate_type_to_string(logx_rotate_type_t type)
{
    switch (type)
    {
    case LOGX_ROTATE_NONE: return "None";
    case LOGX_ROTATE_BY_SIZE: return "By Size";
    case LOGX_ROTATE_BY_DATE: return "By Date";
    default: return "MSC";
    }
}

/**
 * @brief Helper function that returns a string "Enabled"
 * if the value is 1, else "Disabled"
 *
 * @param[in] val Value to be checked
 * @return const char* "Enabled" if val is 1, "Disabled" if val is 0
 */
static const char *logx_check(int val)
{
    if (val)
        return "Enabled";
    else
        return "Disabled";
}

/**
 * @brief Generates a timestamp string with
 * millisecond precision.
 *
 * This internal helper function formats a struct
 * timeval into a human-readable timestamp string
 * in the format: YYYY-MM-DD HH:MM:SS.mmm If the
 * provided timeval pointer is NULL, the current
 * time is used.
 *
 * @param out Pointer to the output buffer where
 * the timestamp string will be written.
 * @param out_sz Size of the output buffer in
 * bytes.
 * @param tv Pointer to a struct timeval
 * representing the time to format. If NULL, the
 * current system time is used.
 *
 * @note The output buffer must be large enough
 * to hold the timestamp string (at least 24
 * bytes to safely store "YYYY-MM-DD
 * HH:MM:SS.mmm\0"). This function is
 * thread-safe.
 */
static void now_ts(char *out, size_t out_sz, struct timeval *tv)
{
    if (!tv)
    {
        struct timeval ttmp;
        gettimeofday(&ttmp, NULL);
        tv = &ttmp;
    }
    struct tm tm;
    localtime_r(&tv->tv_sec, &tm);
    int ms = (int)(tv->tv_usec / 1000);
    snprintf(out, out_sz, "%04d-%02d-%02d %02d:%02d:%02d.%03d", tm.tm_year + 1900, tm.tm_mon + 1,
             tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, ms);
}

/**
 * @brief Acquire an exclusive advisory lock on a file descriptor using flock().
 *
 * @param fd File descriptor to lock.
 * @return int 0 on success, -1 on failure (invalid fd or flock error).
 *
 * @note This uses advisory locking. Other processes must also use flock()
 *       for the lock to be respected.
 */
int file_lock_ex(int fd)
{
    if (fd < 0)
        return -1;
    if (flock(fd, LOCK_EX) == -1)
        return -1;
    return 0;
}

/**
 * @brief Release a previously acquired advisory lock on a file descriptor.
 *
 * @param fd File descriptor to unlock.
 * @return int 0 on success, -1 on failure (invalid fd or flock error).
 *
 * @note Only unlocks descriptors previously locked with flock().
 */
int file_lock_un(int fd)
{
    if (fd < 0)
        return -1;
    if (flock(fd, LOCK_UN) == -1)
        return -1;
    return 0;
}

/**
 * @brief Rotate log files, maintaining a maximum number of backups.
 *
 * This function implements a simple log rotation mechanism by renaming existing
 * log files with numeric suffixes and truncating the main log file if max_backups <= 0.
 *
 * @param path Path to the main log file.
 * @param max_backups Maximum number of backup files to retain. If <= 0, the main
 *                    file is simply truncated and no rotation occurs.
 *
 * @return int 0 on success, -1 on failure (e.g., invalid path).
 *
 * @details
 * - Existing backup files are renamed: path.1 -> path.2, ..., path.(max_backups-1) ->
 * path.max_backups.
 * - The current log file is renamed to path.1.
 * - The oldest backup (path.max_backups) is deleted if it exists.
 * - If max_backups <= 0, the current log file is truncated instead of rotated.
 */
int rotate_files(const char *path, int max_backups)
{
    char oldname[1024];
    char newname[1024];

    if (!path)
    {
        return -1;
    }

    if (max_backups <= 0)
    {
        /* truncate current file */
        int fd = open(path, O_WRONLY | O_TRUNC);
        if (fd >= 0)
        {
            close(fd);
        }

        return 0;
    }

    snprintf(oldname, sizeof(oldname), "%s.%d", path, max_backups);
    unlink(oldname); /* ignore errors */

    for (int i = max_backups - 1; i >= 0; --i)
    {
        if (i == 0)
        {
            snprintf(oldname, sizeof(oldname), "%s", path);
        }
        else
        {
            snprintf(oldname, sizeof(oldname), "%s.%d", path, i);
        }
        snprintf(newname, sizeof(newname), "%s.%d", path, i + 1);
        /* rename will fail if oldname doesn't exist - that's fine */
        rename(oldname, newname);
    }
    return 0;
}

/* Check rotation conditions and perform rotation if needed. Must be called with mutex held. */
static int check_and_rotate_locked(logx_t *l)
{
    if (!l || !l->cfg.enable_file_logging || !l->cfg.file_path)
        return 0;

    if (l->cfg.rotate.type == LOGX_ROTATE_BY_DATE)
    {
        time_t    t = time(NULL);
        struct tm tm;
        localtime_r(&t, &tm);
        char today[16];
        snprintf(today, sizeof(today), "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1,
                 tm.tm_mday);
        if (strcmp(today, l->current_date) != 0)
        {
            /* rotate */
            if (l->fd >= 0)
            {
                file_lock_ex(l->fd);
            }
            if (l->fp)
                fflush(l->fp);
            rotate_files(l->cfg.file_path, l->cfg.rotate.max_backups);
            /* reopen file */
            if (l->fp)
                fclose(l->fp);
            l->fp = fopen(l->cfg.file_path, "a");
            if (l->fp)
                l->fd = fileno(l->fp);
            if (l->fd >= 0)
                file_lock_un(l->fd);
            strncpy(l->current_date, today, sizeof(l->current_date));
        }
    }
    else if (l->cfg.rotate.type == LOGX_ROTATE_BY_SIZE)
    {
        if (l->fd >= 0)
        {
            struct stat st;
            if (fstat(l->fd, &st) == 0)
            {
                if ((size_t)st.st_size >= l->cfg.rotate.size_mb * (1024 * 1024))
                {
                    file_lock_ex(l->fd);
                    if (l->fp)
                        fflush(l->fp);
                    rotate_files(l->cfg.file_path, l->cfg.rotate.max_backups);
                    if (l->fp)
                        fclose(l->fp);
                    l->fp = fopen(l->cfg.file_path, "a");
                    if (l->fp)
                        l->fd = fileno(l->fp);
                    if (l->fd >= 0)
                        file_lock_un(l->fd);
                }
            }
        }
    }
    return 0;
}

/**
 * @brief Sets the default logx configuration
 *
 * @param[in] cfg Pointer to the logx_cfg_t type configuration object to which default configuration
 * settings will get written to. This object will be used by logx_t type logx logger instance
 *
 * @see logx_t
 * @see logx_cfg_t
 */
static void logx_set_default_cfg(logx_cfg_t *cfg)
{
    if (!cfg)
        return;

    memset(cfg, 0, sizeof(*cfg)); // ensure no garbage

    cfg->name                   = LOGX_DEFAULT_CFG_NAME;
    cfg->file_path              = LOGX_DEFAULT_CFG_LOGFILE_PATH;
    cfg->console_level          = LOGX_DEFAULT_CFG_CONSOLE_LEVEL;
    cfg->file_level             = LOGX_DEFAULT_CFG_FILE_LEVEL;
    cfg->enable_console_logging = LOGX_DEFAULT_CFG_ENABLE_CONSOLE_LOGGING;
    cfg->enable_file_logging    = LOGX_DEFAULT_CFG_ENABLE_FILE_LOGGING;
    cfg->enabled_colored_logs   = LOGX_DEFAULT_CFG_ENABLE_COLORED_LOGGING;
    cfg->use_tty_detection      = LOGX_DEFAULT_CFG_ENABLE_TTY_DETECTION;
    cfg->rotate.type            = LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE;
    cfg->rotate.size_mb         = LOGX_DEFAULT_CFG_LOG_ROTATE_SIZE_MB;
    cfg->rotate.max_backups     = LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_NUM_BACKUPS;
    cfg->rotate.daily_interval  = LOGX_DEFAULT_CFG_LOG_ROTATE_DAILY_INTERVAL;
    cfg->banner_pattern         = LOGX_DEFAULT_CFG_BANNER_PATTERN;
    cfg->print_config           = LOGX_DEFAULT_CFG_PRINT_CONFIG;
}

/**
 * @brief Parse a LogX configuration from a JSON file.
 *
 * This function parses all known LogX configuration keys from a JSON file.
 * If a key is missing or invalid, a warning is logged and a default value is used.
 *
 * @param filepath Path to the JSON config file.
 * @param cfg Pointer to a logx_cfg_t structure that will be filled.
 * @return 0 on success, -1 on error
 */
int logx_parse_json_config(const char *filepath, logx_cfg_t *cfg)
{
    FILE *f = fopen(filepath, "r");
    if (!f)
    {
        fprintf(stderr, "[LogX] Could not open JSON config file: %s\n", filepath);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(len + 1);
    if (!data)
    {
        fprintf(stderr, "[LogX] Memory allocation failed while loading: %s\n", filepath);
        fclose(f);
        return -1;
    }

    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(data);
    free(data);

    if (!root)
    {
        fprintf(stderr, "[LogX] JSON parse error in %s\n", filepath);
        return -1;
    }

/* Define helper macros */
#define get_str(root, key) \
    (cJSON_GetObjectItem(root, key) ? cJSON_GetObjectItem(root, key)->valuestring : NULL)

#define get_int(root, key) \
    (cJSON_GetObjectItem(root, key) ? cJSON_GetObjectItem(root, key)->valueint : -1)

#define get_bool(root, key) \
    (cJSON_HasObjectItem(root, key) ? cJSON_IsTrue(cJSON_GetObjectItem(root, key)) : -1)

    /* Log missing keys for visibility */
    log_missing_json_keys(root);

    /* Basic fields */
    const char *val;
    int         ival;

    val       = get_str(root, LOGX_KEY_NAME);
    cfg->name = val ? strdup(val) : LOGX_DEFAULT_CFG_NAME;

    val            = get_str(root, LOGX_KEY_FILE_PATH);
    cfg->file_path = val ? strdup(val) : LOGX_DEFAULT_CFG_LOGFILE_PATH;

    ival                        = get_bool(root, LOGX_KEY_ENABLE_CONSOLE_LOGGING);
    cfg->enable_console_logging = (ival != -1) ? ival : LOGX_DEFAULT_CFG_ENABLE_CONSOLE_LOGGING;

    ival                     = get_bool(root, LOGX_KEY_ENABLE_FILE_LOGGING);
    cfg->enable_file_logging = (ival != -1) ? ival : LOGX_DEFAULT_CFG_ENABLE_FILE_LOGGING;

    ival                      = get_bool(root, LOGX_KEY_ENABLED_COLORED_LOGS);
    cfg->enabled_colored_logs = (ival != -1) ? ival : LOGX_DEFAULT_CFG_ENABLE_COLORED_LOGGING;

    ival                   = get_bool(root, LOGX_KEY_USE_TTY_DETECTION);
    cfg->use_tty_detection = (ival != -1) ? ival : LOGX_DEFAULT_CFG_ENABLE_TTY_DETECTION;

    ival              = get_bool(root, LOGX_KEY_PRINT_CONFIG);
    cfg->print_config = (ival != -1) ? ival : LOGX_DEFAULT_CFG_PRINT_CONFIG;

    val                 = get_str(root, LOGX_KEY_BANNER_PATTERN);
    cfg->banner_pattern = val ? strdup(val) : LOGX_DEFAULT_CFG_BANNER_PATTERN;

    /* Console log level */
    val = get_str(root, LOGX_KEY_CONSOLE_LEVEL);
    if (!val)
    {
        cfg->console_level = LOGX_DEFAULT_CFG_CONSOLE_LEVEL;
    }
    else if (strcasecmp(val, "TRACE") == 0)
        cfg->console_level = LOGX_LEVEL_TRACE;
    else if (strcasecmp(val, "DEBUG") == 0)
        cfg->console_level = LOGX_LEVEL_DEBUG;
    else if (strcasecmp(val, "INFO") == 0)
        cfg->console_level = LOGX_LEVEL_INFO;
    else if (strcasecmp(val, "WARN") == 0)
        cfg->console_level = LOGX_LEVEL_WARN;
    else if (strcasecmp(val, "ERROR") == 0)
        cfg->console_level = LOGX_LEVEL_ERROR;
    else if (strcasecmp(val, "FATAL") == 0)
        cfg->file_level = LOGX_LEVEL_FATAL;
    else
    {
        fprintf(stderr, "[LogX] Invalid console_level: %s → Using default.\n", val);
        cfg->console_level = LOGX_DEFAULT_CFG_CONSOLE_LEVEL;
    }

    /* File log level */
    val = get_str(root, LOGX_KEY_FILE_LEVEL);
    if (!val)
    {
        cfg->file_level = LOGX_DEFAULT_CFG_FILE_LEVEL;
    }
    else if (strcasecmp(val, "TRACE") == 0)
        cfg->file_level = LOGX_LEVEL_TRACE;
    else if (strcasecmp(val, "DEBUG") == 0)
        cfg->file_level = LOGX_LEVEL_DEBUG;
    else if (strcasecmp(val, "INFO") == 0)
        cfg->file_level = LOGX_LEVEL_INFO;
    else if (strcasecmp(val, "WARN") == 0)
        cfg->file_level = LOGX_LEVEL_WARN;
    else if (strcasecmp(val, "ERROR") == 0)
        cfg->file_level = LOGX_LEVEL_ERROR;
    else if (strcasecmp(val, "FATAL") == 0)
        cfg->file_level = LOGX_LEVEL_FATAL;
    else
    {
        fprintf(stderr, "[LogX] Invalid file_level: %s → Using default.\n", val);
        cfg->file_level = LOGX_DEFAULT_CFG_FILE_LEVEL;
    }

    /* Rotation options */
    val = get_str(root, LOGX_KEY_ROTATE_TYPE);
    if (!val)
        cfg->rotate.type = LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE;
    else if (strcasecmp(val, "BY_SIZE") == 0)
        cfg->rotate.type = LOGX_ROTATE_BY_SIZE;
    else if (strcasecmp(val, "BY_DATE") == 0)
        cfg->rotate.type = LOGX_ROTATE_BY_DATE;
    else
    {
        fprintf(stderr, "[LogX] Invalid rotate_type: %s → Using default.\n", val);
        cfg->rotate.type = LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE;
    }

    ival = get_int(root, LOGX_KEY_ROTATE_MAX_MBYTES);
    cfg->rotate.size_mb =
        (ival > 0) ? (size_t)ival * 1024 * 1024 : LOGX_DEFAULT_CFG_LOG_ROTATE_SIZE_MB;

    ival                    = get_int(root, LOGX_KEY_ROTATE_MAX_BACKUPS);
    cfg->rotate.max_backups = (ival >= 0) ? ival : LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_NUM_BACKUPS;

    ival                       = get_int(root, LOGX_KEY_ROTATE_DAILY_INTERVAL);
    cfg->rotate.daily_interval = (ival > 0) ? ival : LOGX_DEFAULT_CFG_LOG_ROTATE_DAILY_INTERVAL;

    cJSON_Delete(root);

#undef get_str
#undef get_int
#undef get_bool

    return 0;
}

/**
 * @brief Parse a LogX configuration from a YAML file.
 *
 * This function parses the LogX configuration YAML file using libyaml.
 * If any key is missing or invalid, a warning is printed and default
 * values from LOGX_DEFAULT_CFG_* macros are applied.
 *
 * @param filepath Path to YAML configuration file.
 * @param cfg Pointer to logx_cfg_t structure.
 * @return 0 on success, -1 on failure.
 */
int logx_parse_yaml_config(const char *filepath, logx_cfg_t *cfg)
{
    FILE *fh = fopen(filepath, "r");
    if (!fh)
    {
        fprintf(stderr, "[LogX] Could not open YAML config file: %s\n", filepath);
        return -1;
    }

    yaml_parser_t parser;
    yaml_token_t  token;
    char          key[128] = {0};

    if (!yaml_parser_initialize(&parser))
    {
        fprintf(stderr, "[LogX] Failed to initialize YAML parser.\n");
        fclose(fh);
        return -1;
    }

    yaml_parser_set_input_file(&parser, fh);

    while (1)
    {
        yaml_parser_scan(&parser, &token);
        if (token.type == YAML_STREAM_END_TOKEN)
        {
            yaml_token_delete(&token);
            break;
        }

        if (token.type == YAML_KEY_TOKEN)
        {
            yaml_token_delete(&token);
            yaml_parser_scan(&parser, &token);
            if (token.type == YAML_SCALAR_TOKEN)
                strncpy(key, (char *)token.data.scalar.value, sizeof(key) - 1);
        }
        else if (token.type == YAML_VALUE_TOKEN)
        {
            yaml_token_delete(&token);
            yaml_parser_scan(&parser, &token);
            if (token.type == YAML_SCALAR_TOKEN)
            {
                const char *val = (const char *)token.data.scalar.value;

                /* ---- Begin parsing ---- */
                if (strcmp(key, LOGX_KEY_NAME) == 0)
                    cfg->name = strdup(val);
                else if (strcmp(key, LOGX_KEY_FILE_PATH) == 0)
                    cfg->file_path = strdup(val);
                else if (strcmp(key, LOGX_KEY_CONSOLE_LEVEL) == 0)
                {
                    if (strcasecmp(val, "TRACE") == 0)
                        cfg->console_level = LOGX_LEVEL_TRACE;
                    else if (strcasecmp(val, "DEBUG") == 0)
                        cfg->console_level = LOGX_LEVEL_DEBUG;
                    else if (strcasecmp(val, "INFO") == 0)
                        cfg->console_level = LOGX_LEVEL_INFO;
                    else if (strcasecmp(val, "WARN") == 0)
                        cfg->console_level = LOGX_LEVEL_WARN;
                    else if (strcasecmp(val, "ERROR") == 0)
                        cfg->console_level = LOGX_LEVEL_ERROR;
                    else if (strcasecmp(val, "FATAL") == 0)
                        cfg->file_level = LOGX_LEVEL_FATAL;
                    else
                    {
                        fprintf(stderr, "[LogX] Invalid console_level '%s' → Using default.\n",
                                val);
                        cfg->console_level = LOGX_DEFAULT_CFG_CONSOLE_LEVEL;
                    }
                }
                else if (strcmp(key, LOGX_KEY_FILE_LEVEL) == 0)
                {
                    if (strcasecmp(val, "TRACE") == 0)
                        cfg->file_level = LOGX_LEVEL_TRACE;
                    else if (strcasecmp(val, "DEBUG") == 0)
                        cfg->file_level = LOGX_LEVEL_DEBUG;
                    else if (strcasecmp(val, "INFO") == 0)
                        cfg->file_level = LOGX_LEVEL_INFO;
                    else if (strcasecmp(val, "WARN") == 0)
                        cfg->file_level = LOGX_LEVEL_WARN;
                    else if (strcasecmp(val, "ERROR") == 0)
                        cfg->file_level = LOGX_LEVEL_ERROR;
                    else if (strcasecmp(val, "FATAL") == 0)
                        cfg->file_level = LOGX_LEVEL_FATAL;
                    else
                    {
                        fprintf(stderr, "[LogX] Invalid file_level '%s' → Using default.\n", val);
                        cfg->file_level = LOGX_DEFAULT_CFG_FILE_LEVEL;
                    }
                }
                else if (strcmp(key, LOGX_KEY_ENABLE_CONSOLE_LOGGING) == 0)
                    cfg->enable_console_logging =
                        (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
                else if (strcmp(key, LOGX_KEY_ENABLE_FILE_LOGGING) == 0)
                    cfg->enable_file_logging =
                        (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
                else if (strcmp(key, LOGX_KEY_ENABLED_COLORED_LOGS) == 0)
                    cfg->enabled_colored_logs =
                        (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
                else if (strcmp(key, LOGX_KEY_USE_TTY_DETECTION) == 0)
                    cfg->use_tty_detection =
                        (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
                else if (strcmp(key, LOGX_KEY_ROTATE_TYPE) == 0)
                {
                    if (strcasecmp(val, "BY_SIZE") == 0)
                        cfg->rotate.type = LOGX_ROTATE_BY_SIZE;
                    else if (strcasecmp(val, "BY_DATE") == 0)
                        cfg->rotate.type = LOGX_ROTATE_BY_DATE;
                    else
                    {
                        fprintf(stderr, "[LogX] Invalid rotate_type '%s' → Using default.\n", val);
                        cfg->rotate.type = LOGX_DEFAULT_CFG_LOG_ROTATE_TYPE;
                    }
                }
                else if (strcmp(key, LOGX_KEY_ROTATE_MAX_MBYTES) == 0)
                {
                    int mbytes = atoi(val);
                    if (mbytes > 0)
                        cfg->rotate.size_mb = (size_t)mbytes * 1024 * 1024;
                    else
                    {
                        fprintf(stderr, "[LogX] Invalid rotate_max_Mbytes '%s' → Using default.\n",
                                val);
                        cfg->rotate.size_mb = LOGX_DEFAULT_CFG_LOG_ROTATE_SIZE_MB;
                    }
                }
                else if (strcmp(key, LOGX_KEY_ROTATE_MAX_BACKUPS) == 0)
                {
                    int backups = atoi(val);
                    cfg->rotate.max_backups =
                        backups > 0 ? backups : LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_NUM_BACKUPS;
                }
                else if (strcmp(key, LOGX_KEY_ROTATE_DAILY_INTERVAL) == 0)
                {
                    int interval = atoi(val);
                    cfg->rotate.daily_interval =
                        interval > 0 ? interval : LOGX_DEFAULT_CFG_LOG_ROTATE_DAILY_INTERVAL;
                }
                else if (strcmp(key, LOGX_KEY_BANNER_PATTERN) == 0)
                    cfg->banner_pattern = strdup(val);
                else if (strcmp(key, LOGX_KEY_PRINT_CONFIG) == 0)
                    cfg->print_config = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
                else
                {
                    fprintf(stderr, "[LogX] Unknown YAML key: %s (ignored)\n", key);
                }
            }
        }

        yaml_token_delete(&token);
    }

    yaml_parser_delete(&parser);
    fclose(fh);

    /* ---- Fallback defaults ---- */
    if (!cfg->name)
        cfg->name = LOGX_DEFAULT_CFG_NAME;
    if (!cfg->file_path)
        cfg->file_path = LOGX_DEFAULT_CFG_LOGFILE_PATH;
    if (!cfg->banner_pattern)
        cfg->banner_pattern = LOGX_DEFAULT_CFG_BANNER_PATTERN;
    if (!cfg->rotate.size_mb)
        cfg->rotate.size_mb = LOGX_DEFAULT_CFG_LOG_ROTATE_SIZE_MB;
    if (!cfg->rotate.max_backups)
        cfg->rotate.max_backups = LOGX_DEFAULT_CFG_LOG_ROTATE_MAX_NUM_BACKUPS;
    if (!cfg->rotate.daily_interval)
        cfg->rotate.daily_interval = LOGX_DEFAULT_CFG_LOG_ROTATE_DAILY_INTERVAL;
    return 0;
}

/**
 * @brief Parse a LOGX configuration file.
 *
 * This function detects the file type based on its extension and calls the
 * corresponding parser for YAML or JSON configuration formats.
 *
 * @param[in] filepath Absolute or relative path to the configuration file.
 * @param[out] cfg Pointer to the logx configuration structure to populate.
 * @return int 0 on success, -1 on failure.
 */
static int logx_parse_config_file(const char *filepath, logx_cfg_t *cfg)
{
    if (!filepath || !cfg)
        return -1;

    const char *ext = strrchr(filepath, '.');
    if (!ext)
        return -1;

    if (strcmp(ext, ".yml") == 0 || strcmp(ext, ".yaml") == 0)
    {
        return logx_parse_yaml_config(filepath, cfg);
    }
    else if (strcmp(ext, ".json") == 0)
    {
        return logx_parse_json_config(filepath, cfg);
    }

    return -1;
}

/**
 * @brief Load the LOGX configuration file from a predefined path.
 *
 * This function attempts to load the configuration for the logging system
 * from one of several possible sources, in the following priority order:
 * 1. If LOGX_CFG_FILEPATH macro is defined, load from that exact file path.
 * 2. Otherwise, try the default configuration files in the working directory:
 *      - logx_cfg.yml
 *      - logx_cfg.yaml
 *      - logx_cfg.json
 *
 * The function will automatically detect which file exists and call the
 * appropriate parser (YAML or JSON).
 *
 * @param[out] cfg Pointer to the configuration structure to populate.
 * @return int 0 on success, -1 if no valid configuration file was found or parsing failed.
 */
static int logx_load_cfg_from_file(logx_cfg_t *cfg)
{
    if (!cfg)
        return -1;

#ifdef LOGX_CFG_FILEPATH
    /* Try the explicitly defined config path first */
    if (access(LOGX_CFG_FILEPATH, F_OK) == 0)
    {
        printf("[LogX] Found logger configuration file: %s. Trying to parse and set configuration "
               "...\n",
               LOGX_CFG_FILEPATH);
        return logx_parse_config_file(LOGX_CFG_FILEPATH, cfg);
    }
#endif

    /* Try default filenames if no explicit path is set or accessible */
#ifdef LOGX_DEFAULT_CFG_YML_FILEPATH
    if (access(LOGX_DEFAULT_CFG_YML_FILEPATH, F_OK) == 0)
    {
        printf("[LogX] Found logger configuration file: %s. Trying to parse and set configuration "
               "...\n",
               LOGX_DEFAULT_CFG_YML_FILEPATH);
        return logx_parse_config_file(LOGX_DEFAULT_CFG_YML_FILEPATH, cfg);
    }
#endif

#ifdef LOGX_DEFAULT_CFG_YAML_FILEPATH
    if (access(LOGX_DEFAULT_CFG_YAML_FILEPATH, F_OK) == 0)
    {
        printf("[LogX] Found logger configuration file: %s. Trying to parse and set configuration "
               "...\n",
               LOGX_DEFAULT_CFG_YAML_FILEPATH);
        return logx_parse_config_file(LOGX_DEFAULT_CFG_YAML_FILEPATH, cfg);
    }
#endif

#ifdef LOGX_DEFAULT_CFG_JSON_FILEPATH
    if (access(LOGX_DEFAULT_CFG_JSON_FILEPATH, F_OK) == 0)
    {
        printf("[LogX] Found logger configuration file: %s. Trying to parse and set configuration "
               "...\n",
               LOGX_DEFAULT_CFG_JSON_FILEPATH);
        return logx_parse_config_file(LOGX_DEFAULT_CFG_JSON_FILEPATH, cfg);
    }
#endif

    printf("[LogX] Couldn't find any logx configuration files\n");
    return -1;
}

/**
 * @brief Prints out currently configured logger configuration
 *
 * @param[in] l Pointer to the LogX logger instance whose configuration is to be printed out.
 */
static void logx_print_config(logx_t *l)
{
    if (!l)
        return;

    fprintf(stderr, "[LogX] ==========================================\n");
    fprintf(stderr, "[LogX] Logger configuration details\n");
    fprintf(stderr, "[LogX] Name                        : %s\n", l->cfg.name);
    fprintf(stderr, "[LogX] File Path                   : %s\n", l->cfg.file_path);
    fprintf(stderr, "[LogX] Console Log Level           : %s\n",
            logx_level_to_string(l->cfg.console_level));
    fprintf(stderr, "[LogX] File Log Level              : %s\n",
            logx_level_to_string(l->cfg.file_level));
    fprintf(stderr, "[LogX] Console Logging             : %s\n",
            logx_check(l->cfg.enable_console_logging));
    fprintf(stderr, "[LogX] File Logging                : %s\n",
            logx_check(l->cfg.enable_file_logging));
    fprintf(stderr, "[LogX] Colored Logs                : %s\n",
            logx_check(l->cfg.enabled_colored_logs));
    fprintf(stderr, "[LogX] TTY Detection               : %s\n",
            logx_check(l->cfg.use_tty_detection));
    fprintf(stderr, "[LogX] Log Rotate Type             : %s\n",
            logx_rotate_type_to_string(l->cfg.rotate.type));
    fprintf(stderr, "[LogX] Max Log Size                : %ld MB\n",
            l->cfg.rotate.size_mb / (1024 * 1024));
    fprintf(stderr, "[LogX] Max Backups                 : %d\n", l->cfg.rotate.max_backups);
    fprintf(stderr, "[LogX] Rotation Interval (Days)    : %d\n", l->cfg.rotate.daily_interval);
    fprintf(stderr, "[LogX] Print Config                : %s\n", logx_check(l->cfg.print_config));
    fprintf(stderr, "[LogX] ==========================================\n");
}

/**
 * @brief Create and initialize a LogX logger instance.
 *
 * This function creates a new logger instance and configures it based on one of the following:
 * 1. A user-provided configuration (`cfg`).
 * 2. A configuration file (`logx.yml`, `logx.yaml`, or `logx.json`).
 * 3. A built-in default configuration (used as a fallback when no configuration is provided or
 * found).
 *
 * The created logger can then be used with the logging macros such as:
 * @code
 *   LOGX_INFO(logger, "This is an info message");
 *   LOGX_ERROR(logger, "An error occurred: %d", err);
 * @endcode
 *
 * @param[in] cfg   Optional pointer to a user-provided configuration structure.
 *                  If NULL, the function attempts to load from a configuration file,
 *                  and if no file is found, falls back to default settings.
 *
 * @return Pointer to a newly created logger instance (`logx_t*`) on success,
 *         or NULL if allocation or initialization fails.
 *
 * @note The caller is responsible for destroying the logger using `logx_destroy()`
 *       to release allocated resources.
 *
 * @see logx_cfg_t
 * @see logx_destroy()
 */
logx_t *logx_create(const logx_cfg_t *cfg)
{
    logx_cfg_t internal_cfg;

    if (cfg)
    {
        internal_cfg = *cfg; // shallow copy
    }
    else
    {
        fprintf(stderr,
                "[LogX] No configuration provided. Trying to load configuration from file...\n");
        if (logx_load_cfg_from_file(&internal_cfg) < 0)
        {
            fprintf(stderr, "[LogX] Setting default configuration...\n");
            logx_set_default_cfg(&internal_cfg);
        }
    }

    // Allocate logger
    logx_t *l = calloc(1, sizeof(*l));
    if (!l)
        return NULL;

    memcpy(&l->cfg, &internal_cfg, sizeof(l->cfg));
    pthread_mutex_init(&l->lock, NULL);

    l->fp              = NULL;
    l->fd              = -1;
    l->current_date[0] = '\0';

    if (l->cfg.enable_file_logging && l->cfg.file_path)
    {
        l->fp = fopen(l->cfg.file_path, "a");
        if (!l->fp)
        {
            fprintf(stderr, "[LogX] Opening %s failed. Disabling file logging...\n",
                    l->cfg.file_path);
            l->cfg.enable_file_logging = 0; // disable if cannot open
        }
        else
        {
            l->fd       = fileno(l->fp);
            // initialize date tracking
            time_t    t = time(NULL);
            struct tm tm;
            localtime_r(&t, &tm);
            snprintf(l->current_date, sizeof(l->current_date), "%04d-%02d-%02d", tm.tm_year + 1900,
                     tm.tm_mon + 1, tm.tm_mday);
        }
    }

    if (l->cfg.print_config)
    {
        logx_print_config(l);
    }

    return l;
}

/**
 * @brief Destroys a LogX logger instance and frees associated resources.
 *
 * This function performs the following actions:
 *   - Flushes and closes the log file (if file logging was enabled).
 *   - Releases any allocated resources associated with the logger.
 *   - Destroys the internal mutex used for thread safety.
 *   - Frees the logger structure itself.
 *
 * After calling this function, the logger pointer should not be used.
 *
 * @param[in] logger Pointer to the logx_t instance to destroy. If NULL, the function does nothing.
 */
void logx_destroy(logx_t *logger)
{
    if (!logger)
    {
        return;
    }

    pthread_mutex_lock(&logger->lock);

    if (logger->fp)
    {
        fflush(logger->fp);
        fclose(logger->fp);
        logger->fp = NULL;
        logger->fd = -1;
    }

    pthread_mutex_unlock(&logger->lock);
    pthread_mutex_destroy(&logger->lock);

    free(logger);
}

/**
 * @brief Logs a message to console and/or file.
 *
 * This function formats and writes a log message according to the logger configuration.
 * It supports different log levels, colored console output, file logging with optional
 * file locking, and a special banner log format. If both console and file logging are
 * disabled for the given level, the function returns immediately.
 *
 * @param logger Pointer to a valid logx_t logger instance. If NULL, the function does nothing.
 * @param level The log level for this message (e.g., LOGX_LEVEL_TRACE, LOGX_LEVEL_INFO,
 * LOGX_LEVEL_BANNER).
 * @param file The source filename from which the log call originates (usually __FILE__).
 *             If NULL, a "?" placeholder is used.
 * @param func The function name from which the log call originates (usually __func__).
 *             If NULL, a "?" placeholder is used.
 * @param line The line number from which the log call originates (usually __LINE__).
 * @param fmt printf-style format string for the log message.
 * @param ... Variable arguments corresponding to the format string.
 *
 * @details
 * - The function acquires the logger mutex to ensure thread safety.
 * - It checks if the log level meets the thresholds configured for console and file logging.
 * - If file logging is enabled, it performs a log rotation check before writing.
 * - Timestamp is generated with microsecond precision using gettimeofday().
 * - Console output can be colored depending on configuration and TTY detection.
 * - Banner logs (LOGX_LEVEL_BANNER) are surrounded by a customizable border pattern and aligned
 * nicely.
 * - Normal logs include a timestamp, log level, source file, function name, and line number prefix.
 * - File writes honor file locks if a file descriptor is provided.
 *
 * @note
 * - Payloads are truncated if they exceed 4096 bytes.
 * - Colored output will be disabled if TTY detection is enabled and the output is not a terminal.
 * - The function is safe to call from multiple threads.
 */

void logx_log(logx_t *logger, logx_level_t level, const char *file, const char *func, int line,
              const char *fmt, ...)
{
    if (!logger)
        return;

    struct timeval tv;
    gettimeofday(&tv, NULL);

    pthread_mutex_lock(&logger->lock);

    /* Check thresholds */
    int write_console = logger->cfg.enable_console_logging && level >= logger->cfg.console_level;
    int write_file =
        logger->cfg.enable_file_logging && level >= logger->cfg.file_level && logger->fp;

    if (!write_console && !write_file)
    {
        pthread_mutex_unlock(&logger->lock);
        return;
    }

    /* rotation check */
    check_and_rotate_locked(logger);

    char ts[64];
    now_ts(ts, sizeof(ts), &tv);

    /* prepare message payload */
    char    payload[4096];
    char    linebuf[4096];
    char    border[4096 + 10]; // payload max + margins
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(payload, sizeof(payload), fmt, ap);
    va_end(ap);

    int         use_color = logger->cfg.enabled_colored_logs;
    const char *c         = COLOR_RESET;

    if (use_color)
    {
        switch (level)
        {
        case LOGX_LEVEL_TRACE: c = COLOR_TRACE; break;
        case LOGX_LEVEL_DEBUG: c = COLOR_DEBUG; break;
        case LOGX_LEVEL_INFO: c = COLOR_INFO; break;
        case LOGX_LEVEL_WARN: c = COLOR_WARN; break;
        case LOGX_LEVEL_ERROR: c = COLOR_ERROR; break;
        case LOGX_LEVEL_BANNER: c = COLOR_BANNER; break;
        case LOGX_LEVEL_FATAL: c = COLOR_FATAL; break;
        default: c = COLOR_RESET; break;
        }
    }
    /* If it's a banner log, build the banner */
    if (level == LOGX_LEVEL_BANNER)
    {
        const char *pattern = (logger->cfg.banner_pattern && *logger->cfg.banner_pattern)
                                  ? logger->cfg.banner_pattern
                                  : "=";

        size_t msg_len     = strlen(payload);
        size_t pattern_len = strlen(pattern);

        /* --- border generation --- */
        size_t border_len = (msg_len < sizeof(border) - 11) ? msg_len : sizeof(border) - 11;

        // Add padding on both sides (5 chars each)
        size_t padded_len = border_len + 10;
        if (padded_len > sizeof(border) - 1)
            padded_len = sizeof(border) - 1;

        for (size_t i = 0; i < padded_len; ++i) border[i] = pattern[i % pattern_len];

        border[padded_len] = '\0';
    }

    snprintf(linebuf, sizeof(linebuf), "[%s] [%s] (%s:%s:%d): ", ts, logx_level_to_string(level),
             file ? file : "?", func ? func : "?", line);

    int gap_len = strlen(linebuf);

    /* Console write */
    if (write_console)
    {
        FILE *out = (level >= LOGX_LEVEL_WARN) ? stderr : stdout;
        if (logger->cfg.use_tty_detection)
            use_color = use_color && isatty(fileno(out));

        if (use_color)
        {
            if (level == LOGX_LEVEL_BANNER)
            {
                fprintf(out, "%s%s%s", c, linebuf, COLOR_RESET);
                fprintf(out, "%s%s%s\n", c, border, COLOR_RESET);
                fprintf(out, "%*s", gap_len, "");
                fprintf(out, "%s%*s%s%s\n", c, 5, "", payload, COLOR_RESET);
                fprintf(out, "%*s", gap_len, "");
                fprintf(out, "%s%s%s\n", c, border, COLOR_RESET);
            }
            else
            {
                fprintf(out, "%s%s%s", c, linebuf, COLOR_RESET);
                fprintf(out, "%s%s%s\n", c, payload, COLOR_RESET);
            }
        }
        else
        {
            if (level == LOGX_LEVEL_BANNER)
            {
                fprintf(out, "%s", linebuf);
                fprintf(out, "%s\n", border);
                fprintf(out, "%*s", gap_len, "");
                fprintf(out, "%*s%s\n", 5, "", payload);
                fprintf(out, "%*s", gap_len, "");
                fprintf(out, "%s\n", border);
            }
            else
            {
                fprintf(out, "%s", linebuf);
                fprintf(out, "%s\n", payload);
            }
        }

        fflush(out);
    }

    /* File write */
    if (write_file)
    {
        if (logger->fd >= 0)
            file_lock_ex(logger->fd);

        if (logger->fp)
        {
            if (level == LOGX_LEVEL_BANNER)
            {
                fprintf(logger->fp, "%s", linebuf);
                fprintf(logger->fp, "%s\n", border);
                fprintf(logger->fp, "%*s", gap_len, "");
                fprintf(logger->fp, "%*s%s\n", 5, "", payload);
                fprintf(logger->fp, "%*s", gap_len, "");
                fprintf(logger->fp, "%s\n", border);
            }
            else
            {
                fprintf(logger->fp, "%s", linebuf);
                fprintf(logger->fp, "%s\n", payload);
            }
            fflush(logger->fp);
        }

        if (logger->fd >= 0)
            file_lock_un(logger->fd);
    }

    pthread_mutex_unlock(&logger->lock);
}
