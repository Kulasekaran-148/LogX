#define _POSIX_C_SOURCE 200809L

#include "logx.h"
#include "logx_common.h"
#include "logx_config.h"
#include "logx_errorcodes.h"
#include "logx_rotation.h"
#include "logx_string_maps.h"
#include "logx_time.h"

#include <cJSON/cJSON.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <yaml.h>

const char *logx_bin_str64_grouped_tls(uint64_t value)
{
    // 8 rotating buffers, each large enough for grouped 64-bit binary + NUL
    enum
    {
        BUF_COUNT = 8,
        BUF_SIZE  = 128
    };

    static _Thread_local char bufs[BUF_COUNT][BUF_SIZE];
    static _Thread_local int idx = 0;

    char *out = bufs[idx];
    idx       = (idx + 1) % BUF_COUNT;

    char tmp[64 + 16];
    int pos = 0;

    // Build full 64-bit binary with nibble spaces
    for (int i = 63; i >= 0; --i)
    {
        tmp[pos++] = (value & (1ULL << i)) ? '1' : '0';
        if (i % 4 == 0 && i != 0)
            tmp[pos++] = ' ';
    }
    tmp[pos] = '\0';

    // Find first '1'
    int first_one = -1;
    for (int i = 0; tmp[i]; ++i)
    {
        if (tmp[i] == '1')
        {
            first_one = i;
            break;
        }
    }

    // All zero → return one nibble
    if (first_one == -1)
    {
        strcpy(out, "0000");
        return out;
    }

    // Snap to nibble boundary
    while (first_one > 0 && tmp[first_one - 1] != ' ')
        first_one--;

    if (tmp[first_one] == ' ')
        first_one++;

    strcpy(out, tmp + first_one);
    return out;
}

logx_errorcodes_t logx_enable_print_config(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.print_config = 1;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_disable_print_config(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.print_config = 0;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_set_console_logging_level(logx_t *logger, logx_level_t level)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger || !is_valid_logx_level(level))
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.console_level = level;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_set_file_logging_level(logx_t *logger, logx_level_t level)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger || !is_valid_logx_level(level))
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.file_level = level;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_set_console_logging(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_console_logging = 1;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_disable_console_logging(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_console_logging = 0;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_enable_file_logging(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    if (!logger->cfg.file_path)
    {
        logger->cfg.enable_file_logging = 0;
        eErr                            = LOGX_ERR_INVALID_LOGFILE_PATH;
        pthread_mutex_unlock(&logger->lock);
        goto END;
    }
    else
    {
        logger->cfg.enable_file_logging = 1;
    }
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_disable_file_logging(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_file_logging = 0;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_enable_colored_logging(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_colored_logs = 1;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_disable_colored_logging(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_colored_logs = 0;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_enable_tty_detection(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.use_tty_detection = 1;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_disable_tty_detection(logx_t *logger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.use_tty_detection = 0;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

static logx_errorcodes_t logx_set_default_cfg(logx_cfg_t *cfg)
{
    if (!cfg)
        return LOGX_ERR_INVALID_ARG;

    memset(cfg, 0, sizeof(*cfg));

    for (size_t i = 0; i < LOGX_FIELD_TABLE_COUNT; i++)
    {
        const logx_field_desc_t *desc = &LOGX_FIELD_TABLE[i];
        void *field_ptr               = (char *)cfg + desc->offset;
        if (desc->type == LOGX_FIELD_STRING)
            *(const char **)field_ptr = desc->def.str_default;
        else
            *(int *)field_ptr = desc->def.int_default;
    }

    return LOGX_ERR_SUCCESS;
}

/**
 * @brief Parse a LogX configuration from a JSON file.
 *
 * This function parses all known LogX configuration keys from a JSON file.
 * If a key is missing or invalid, a warning is logged and a default value is used.
 *
 * @param[in] filepath Path to the JSON config file.
 * @param[in] cfg Pointer to a logx_cfg_t structure that will be filled.
 * @return LOGX_ERR_SUCCESS on success, error otherwise
 */
static logx_errorcodes_t logx_parse_json_config(const char *filepath, logx_cfg_t *cfg)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;
    FILE *fp               = NULL;
    char *data             = NULL;
    cJSON *root            = NULL;

    fp = fopen(filepath, "r");
    if (!fp)
    {
        fprintf(stderr, "[LogX] Could not open JSON config file: %s\n", filepath);
        eErr = LOGX_ERR_FILE_OPEN_FAILED;
        goto END;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    data = malloc(len + 1);
    if (!data)
    {
        fprintf(stderr, "[LogX] Memory allocation failed while loading: %s\n", filepath);
        eErr = LOGX_ERR_NO_MEM;
        goto END;
    }

    fread(data, 1, len, fp);
    data[len] = '\0';

    root = cJSON_Parse(data);

    if (!root)
    {
        fprintf(stderr, "[LogX] JSON parse error in: %s\n", filepath);
        eErr = LOGX_ERR_CJSON_PARSE_FAILED;
        goto END;
    }

    /* Warn about unrecognized or missing keys */
    log_missing_json_keys(root);

    for (size_t i = 0; i < LOGX_FIELD_TABLE_COUNT; i++)
    {
        const logx_field_desc_t *desc = &LOGX_FIELD_TABLE[i];
        cJSON *item                   = cJSON_GetObjectItem(root, desc->key);

        bool found    = (item != NULL);
        const char *s = (found && cJSON_IsString(item)) ? item->valuestring : NULL;
        int iv        = (found && cJSON_IsNumber(item)) ? item->valueint
                        : (found && cJSON_IsBool(item)) ? cJSON_IsTrue(item)
                                                        : 0;

        logx_apply_field(cfg, desc, s, iv, found);
    }

END:
    if (fp)
        fclose(fp);
    if (data)
        free(data);
    if (root)
        cJSON_Delete(root);
    if (eErr != LOGX_ERR_SUCCESS)
        logx_cfg_free_strings(cfg);
    return eErr;
}

/**
 * @brief Parse a LogX configuration from a YAML file.
 *
 * This function parses the LogX configuration YAML file using libyaml.
 * If any key is missing or invalid, a warning is printed and default
 * value gets applied
 *
 * @param[in] filepath Path to YAML configuration file.
 * @param[out] cfg Pointer to logx_cfg_t structure.
 * @return 0 on success, -1 on failure.
 */
static logx_errorcodes_t logx_parse_yaml_config(const char *filepath, logx_cfg_t *cfg)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;
    FILE *fh               = NULL;
    yaml_parser_t parser;
    yaml_token_t token;
    bool parser_initialized = false;

#define YAML_MAX_KEYS 32
    char keys[YAML_MAX_KEYS][128] = {{0}};
    char vals[YAML_MAX_KEYS][256] = {{0}};
    int pair_count                = 0;

    fh = fopen(filepath, "r");
    if (!fh)
    {
        fprintf(stderr, "[LogX] Could not open YAML config file: %s\n", filepath);
        eErr = LOGX_ERR_FILE_OPEN_FAILED;
        goto END;
    }

    if (!yaml_parser_initialize(&parser))
    {
        fprintf(stderr, "[LogX] Failed to initialize YAML parser.\n");
        eErr = LOGX_ERR_FAILURE;
        goto END;
    }
    parser_initialized = true;

    yaml_parser_set_input_file(&parser, fh);

    /* ── Phase 1: stream tokens into flat key-value pairs ────────────────── */
    char current_key[128] = {0};

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
                strncpy(current_key, (char *)token.data.scalar.value, sizeof(current_key) - 1);
        }
        else if (token.type == YAML_VALUE_TOKEN)
        {
            yaml_token_delete(&token);
            yaml_parser_scan(&parser, &token);
            if (token.type == YAML_SCALAR_TOKEN && pair_count < YAML_MAX_KEYS)
            {
                strncpy(keys[pair_count], current_key, sizeof(keys[0]) - 1);
                strncpy(vals[pair_count], (char *)token.data.scalar.value, sizeof(vals[0]) - 1);
                pair_count++;
            }
        }

        yaml_token_delete(&token);
    }

    /* ── Phase 2: walk field table, look up each key in collected pairs ───── */
    for (size_t i = 0; i < LOGX_FIELD_TABLE_COUNT; i++)
    {
        const logx_field_desc_t *desc = &LOGX_FIELD_TABLE[i];
        bool found                    = false;
        const char *val               = NULL;

        for (int j = 0; j < pair_count; j++)
        {
            if (strcmp(keys[j], desc->key) == 0)
            {
                found = true;
                val   = vals[j];
                break;
            }
        }

        /* YAML is text-only — everything comes as a string.
         * Convert to int for BOOL and INT fields before applying. */
        int iv = 0;
        if (found && val)
        {
            switch (desc->type)
            {
                case LOGX_FIELD_BOOL:
                    iv = (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0) ? 1 : 0;
                    break;
                case LOGX_FIELD_INT:
                    iv = atoi(val);
                    break;
                default:
                    break;
            }
        }

        logx_apply_field(cfg, desc, val, iv, found);
    }

END:
    if (parser_initialized)
        yaml_parser_delete(&parser);
    if (fh)
        fclose(fh);
    if (eErr != LOGX_ERR_SUCCESS)
        logx_cfg_free_strings(cfg);
    return eErr;

#undef YAML_MAX_KEYS
}

static logx_errorcodes_t logx_parse_config_file(const char *filepath, logx_cfg_t *cfg)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    if (!filepath)
    {
        eErr = LOGX_ERR_INVALID_LOGFILE_PATH;
        goto END;
    }

    if (!cfg)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    const char *ext = strrchr(filepath, '.');
    if (!ext)
    {
        eErr = LOGX_ERR_INVALID_LOGFILE_PATH;
        goto END;
    }

    if (strcmp(ext, ".yml") == 0 || strcmp(ext, ".yaml") == 0)
    {
        eErr = logx_parse_yaml_config(filepath, cfg);
    }
    else if (strcmp(ext, ".json") == 0)
    {
        eErr = logx_parse_json_config(filepath, cfg);
    }

END:
    return eErr;
}

static logx_errorcodes_t logx_load_cfg_from_file(logx_cfg_t *cfg)
{
    if (!cfg)
        return LOGX_ERR_INVALID_ARG;

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

logx_t *logx_create(const logx_cfg_t *cfg)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;
    logx_cfg_t internal_cfg;

    if (cfg)
    {
        internal_cfg = *cfg;
        logx_cfg_dup_strings(&internal_cfg); /* take ownership of caller's strings */
    }
    else
    {
        fprintf(stderr,
                "[LogX] No configuration provided. Trying to load configuration from file...\n");
        if (logx_load_cfg_from_file(&internal_cfg) < 0)
        {
            fprintf(stderr, "[LogX] Setting default configuration...\n");
            logx_set_default_cfg(&internal_cfg);
            logx_cfg_dup_strings(&internal_cfg); /* literals → heap */
        }
        /* parser path: strings already heap-owned via logx_apply_field */
    }

    // Allocate logger
    logx_t *l = calloc(1, sizeof(*l));
    if (!l)
    {
        logx_cfg_free_strings(&internal_cfg);
        return NULL;
    }

    memcpy(&l->cfg, &internal_cfg, sizeof(l->cfg));
    pthread_mutex_init(&l->lock, NULL);

    l->fp              = NULL;
    l->fd              = -1;
    l->current_date[0] = '\0';

    if (l->cfg.enable_file_logging && l->cfg.file_path)
    {

        /* Ensure directory exists */
        if ((eErr = ensure_parent_dir_exists(l->cfg.file_path)) != LOGX_ERR_SUCCESS)
        {
            fprintf(stderr, "[LogX] Failed to create path for logfile: %s",
                    logx_get_err_string(eErr));
        }

        l->fp = fopen(l->cfg.file_path, "a");
        if (!l->fp)
        {
            fprintf(stderr, "[LogX] Opening %s failed. Disabling file logging...\n",
                    l->cfg.file_path);
            l->cfg.enable_file_logging = 0; // disable if cannot open
        }
        else
        {
            l->fd = fileno(l->fp);
            // initialize date tracking
            time_t t = time(NULL);
            struct tm tm;
            localtime_r(&t, &tm);
            snprintf(l->current_date, sizeof(l->current_date), "%04d-%02d-%02d", tm.tm_year + 1900,
                     tm.tm_mon + 1, tm.tm_mday);
        }
    }

    /* TTY detection */
    if (l->cfg.use_tty_detection)
    {
        if (!isatty(fileno(stdout)))
        {
            l->cfg.enable_colored_logs = 0;
        }
    }

    /* Print configuration if enabled */
    if (l->cfg.print_config)
    {
        logx_cfg_print(l);
    }

    return l;
}

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

    logx_cfg_free_strings(&logger->cfg);
    free(logger);
}

void logx_log(logx_t *logger, logx_level_t level, const char *file, const char *func, int line,
              const char *fmt, ...)
{
    if (!logger || level == LOGX_LEVEL_OFF)
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
    if (write_file)
        check_and_rotate_log(logger);

    char ts[64];
    get_timestamp(ts, sizeof(ts), &tv, logger->cfg.ts_format);

    /* prepare message payload */
    char payload[4096];
    char linebuf[4096];
    char border[4096 + 10]; // payload max + margins
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(payload, sizeof(payload), fmt, ap);
    va_end(ap);

    int use_color = logger->cfg.enable_colored_logs;
    const char *color;

    /* Determine color code */
    if (use_color)
    {
        color = logx_level_to_color(level);
    }
    else
    {
        color = COLOR_RESET;
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

        for (size_t i = 0; i < padded_len; ++i)
            border[i] = pattern[i % pattern_len];

        border[padded_len] = '\0';
    }

    /* Prepare prefix */
    int gap_len    = snprintf(linebuf, sizeof(linebuf), "[%s] [%s] [%s] (%s:%s:%d): ", ts,
                              logx_level_to_string(level), logger->cfg.name, file ? file : "?",
                           func ? func : "?", line);
    int prefix_len = 5;

    /* Console write */
    if (write_console)
    {
        FILE *out = (level >= LOGX_LEVEL_WARN) ? stderr : stdout;

        if (use_color)
        {
            if (level == LOGX_LEVEL_BANNER)
            {
                fprintf(out, "%s%s%s", color, linebuf, COLOR_RESET);
                fprintf(out, "%s%s%s\n", color, border, COLOR_RESET);
                fprintf(out, "%*s", gap_len, "");
                fprintf(out, "%s%*s%s%s\n", color, prefix_len, "", payload, COLOR_RESET);
                fprintf(out, "%*s", gap_len, "");
                fprintf(out, "%s%s%s\n", color, border, COLOR_RESET);
            }
            else
            {
                fprintf(out, "%s%s%s", color, linebuf, COLOR_RESET);
                fprintf(out, "%s%s%s\n", color, payload, COLOR_RESET);
            }
        }
        else
        {
            if (level == LOGX_LEVEL_BANNER)
            {
                fprintf(out, "%s", linebuf);
                fprintf(out, "%s\n", border);
                fprintf(out, "%*s", gap_len, "");
                fprintf(out, "%*s%s\n", prefix_len, "", payload);
                fprintf(out, "%*s", gap_len, "");
                fprintf(out, "%s\n", border);
            }
            else
            {
                fprintf(out, "%s", linebuf);
                fprintf(out, "%s\n", payload);
            }
        }

        // fflush(out);
    }

    /* File write */
    if (write_file)
    {
        if (logger->fd >= 0)
            exclusive_flock(logger->fd);

        if (logger->fp)
        {
            if (level == LOGX_LEVEL_BANNER)
            {
                fprintf(logger->fp, "%s", linebuf);
                fprintf(logger->fp, "%s\n", border);
                fprintf(logger->fp, "%*s", gap_len, "");
                fprintf(logger->fp, "%*s%s\n", prefix_len, "", payload);
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
            unlock_flock(logger->fd);
    }

    pthread_mutex_unlock(&logger->lock);
}
