/**
 * @file logx_common.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Shared utility functions — enum-to-string maps, file locking, and directory helpers.
 * @version 2.0.0
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 */

#include "logx_common.h"
#include "logx.h"
#include "logx_errorcodes.h"
#include "logx_rotation.h"
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Enum lookup tables — add/remove a row here to add/remove an enum value */
const logx_level_entry_t LOGX_LEVEL_MAP[] = {
    {LOGX_LEVEL_TRACE, "TRC", "TRACE", COLOR_BLUE},
    {LOGX_LEVEL_DEBUG, "DBG", "DEBUG", COLOR_WHITE},
    {LOGX_LEVEL_BANNER, "BNR", "BANNER", COLOR_CYAN},
    {LOGX_LEVEL_INFO, "INF", "INFO", COLOR_GREEN},
    {LOGX_LEVEL_WARN, "WRN", "WARN", COLOR_YELLOW},
    {LOGX_LEVEL_ERROR, "ERR", "ERROR", COLOR_RED},
    {LOGX_LEVEL_FATAL, "FTL", "FATAL", COLOR_MAGENTA},
    {LOGX_LEVEL_OFF, "OFF", "OFF", COLOR_RESET},
};

const size_t LOGX_LEVEL_MAP_COUNT = ARRAY_SIZE(LOGX_LEVEL_MAP);

const logx_rotate_entry_t LOGX_ROTATE_MAP[] = {
    {LOGX_ROTATE_NONE, "None", "NONE"},
    {LOGX_ROTATE_BY_SIZE, "By Size", "BY_SIZE"},
    {LOGX_ROTATE_BY_DATE, "By Date", "BY_DATE"},
};
const size_t LOGX_ROTATE_MAP_COUNT = ARRAY_SIZE(LOGX_ROTATE_MAP);

const logx_ts_fmt_entry_t LOGX_TS_FMT_MAP[] = {
    {LOGX_TS_FMT_LOCAL, "LOCAL"},       {LOGX_TS_FMT_UTC, "UTC"},
    {LOGX_TS_FMT_EPOCH_S, "EPOCH_S"},   {LOGX_TS_FMT_EPOCH_MS, "EPOCH_MS"},
    {LOGX_TS_FMT_EPOCH_US, "EPOCH_US"}, {LOGX_TS_FMT_ISO8601, "ISO8601"},
    {LOGX_TS_FMT_RFC2822, "RFC2822"},
};
const size_t LOGX_TS_FMT_MAP_COUNT = ARRAY_SIZE(LOGX_TS_FMT_MAP);

/* ========================= Internal Helper functions ========================= */

logx_errorcodes_t exclusive_flock(int fd)
{
    if (fd < 0)
        return LOGX_ERR_INVALID_FD;
    if (flock(fd, LOCK_EX) == -1)
        return LOGX_ERR_FLOCK_FAILED;
    return LOGX_ERR_SUCCESS;
}

logx_errorcodes_t unlock_flock(int fd)
{
    if (fd < 0)
        return LOGX_ERR_INVALID_FD;
    if (flock(fd, LOCK_UN) == -1)
        return LOGX_ERR_FLOCK_FAILED;
    return LOGX_ERR_SUCCESS;
}

logx_errorcodes_t is_valid_logx_rotate_type(logx_rotate_type_t eRotateType)
{
    for (size_t i = 0; i < LOGX_ROTATE_MAP_COUNT; i++)
        if (LOGX_ROTATE_MAP[i].val == eRotateType)
            return LOGX_ERR_SUCCESS;
    return LOGX_ERR_FAILURE;
}

logx_errorcodes_t is_valid_logx_level(logx_level_t eLogLevel)
{
    for (size_t i = 0; i < LOGX_LEVEL_MAP_COUNT; i++)
        if (LOGX_LEVEL_MAP[i].val == eLogLevel)
            return LOGX_ERR_SUCCESS;
    return LOGX_ERR_FAILURE;
}

const char *logx_level_to_color(logx_level_t eLogLevel)
{
    for (size_t i = 0; i < LOGX_LEVEL_MAP_COUNT; i++)
        if (LOGX_LEVEL_MAP[i].val == eLogLevel)
            return LOGX_LEVEL_MAP[i].color;
    return COLOR_RESET;
}

const char *logx_check(int bEnable)
{
    if (bEnable)
        return "Enabled";
    else
        return "Disabled";
}

/**
 * @brief Recursively create a directory path, like `mkdir -p`.
 *
 * Creates each component of `path` in turn, ignoring `EEXIST` at each step.
 * The path must fit within 255 bytes.
 *
 * @param[in] path Null-terminated directory path to create.
 * @return `LOGX_ERR_SUCCESS` if the path exists or was created,
 *         `LOGX_ERR_DIRECTORY_CREATION_FAILED` on any other error.
 */
static logx_errorcodes_t mkdir_p(const char *path)
{
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s", path);

    for (char *p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            if (mkdir(tmp, 0755) && errno != EEXIST)
                return LOGX_ERR_DIRECTORY_CREATION_FAILED;
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) && errno != EEXIST)
        return LOGX_ERR_DIRECTORY_CREATION_FAILED;

    return 0;
}

logx_errorcodes_t ensure_parent_dir_exists(const char *filepath)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;
    char path[256];
    snprintf(path, sizeof(path), "%s", filepath);

    char *last_slash = strrchr(path, '/');
    if (last_slash)
    {
        *last_slash = '\0';

        if ((eErr = mkdir_p(path)) != LOGX_ERR_SUCCESS)
        {
            fprintf(stderr, "[LogX] Failed to create directory %s (%s)\n", path, strerror(errno));
        }
    }
    return eErr;
}