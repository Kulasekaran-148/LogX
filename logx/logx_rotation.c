/**
 * @file logx_rotation.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Log file rotation implementation — size-based, date-based, and on-demand rotation.
 * @version 2.0.0
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 */

#include "logx_rotation.h"
#include "fcntl.h"
#include "logx.h"
#include "logx_common.h"
#include "logx_errorcodes.h"
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

/**
 * @brief Compress a file to `<src_path>.gz` using zlib, then delete the original.
 *
 * On any error the partially-written `.gz` file is removed and the original is
 * left intact so that log data is never silently lost.
 *
 * @param[in] src_path Path to the uncompressed source file.
 * @return `LOGX_ERR_SUCCESS` on success, `LOGX_ERR_COMPRESS_FAILED` on zlib error,
 *         or `LOGX_ERR_FILE_OPEN_FAILED` if the source cannot be opened.
 */
static logx_errorcodes_t compress_file(const char *src_path)
{
    char gz_path[LOGX_LOG_FILE_PATH_MAX_LEN_BYTES];
    char buf[65536];
    FILE *src              = NULL;
    gzFile gz              = NULL;
    size_t n               = 0;
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    snprintf(gz_path, sizeof(gz_path), "%s.gz", src_path);

    src = fopen(src_path, "rb");
    if (!src)
    {
        return LOGX_ERR_FILE_OPEN_FAILED;
    }

    gz = gzopen(gz_path, "wb9");
    if (!gz)
    {
        fclose(src);
        return LOGX_ERR_COMPRESS_FAILED;
    }

    while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
    {
        if (gzwrite(gz, buf, (unsigned int)n) != (int)n)
        {
            eErr = LOGX_ERR_COMPRESS_FAILED;
            break;
        }
    }

    gzclose(gz);
    fclose(src);

    if (eErr != LOGX_ERR_SUCCESS)
    {
        unlink(gz_path); /* remove partial output; original is preserved */
        return eErr;
    }

    unlink(src_path); /* delete original only after successful compression */
    return LOGX_ERR_SUCCESS;
}

/**
 * @brief Rotate log backup files, shifting numeric suffixes and optionally compressing.
 *
 * Implements the same strategy as logrotate:
 *  - The oldest backup (`path.max_backups` / `path.max_backups.gz`) is deleted.
 *  - Each existing backup is shifted up by one: `path.N` → `path.(N+1)`,
 *    preserving the `.gz` extension when the file is already compressed.
 *  - The active log is renamed to `path.1`.
 *  - If `compress` is set and `delay_compress` is not, `path.1` is immediately
 *    compressed to `path.1.gz`.
 *  - If both `compress` and `delay_compress` are set, `path.2` (the backup from
 *    the *previous* rotation, still uncompressed) is compressed instead, and
 *    `path.1` is left plain until the next rotation.
 *
 * @param[in] path          Path to the active log file.
 * @param[in] dwMaxBackups  Maximum backup files to retain (0 = truncate, no backup).
 * @param[in] compress      1 = compress backups with gzip.
 * @param[in] delay_compress 1 = defer compression of the newest backup by one rotation.
 *
 * @return `LOGX_ERR_SUCCESS` on success, or an error code on failure.
 */
static logx_errorcodes_t rotate_files(const char *path, int dwMaxBackups, int compress,
                                      int delay_compress)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;
    int fd                 = -1;
    char oldname[LOGX_LOG_FILE_PATH_MAX_LEN_BYTES];
    char newname[LOGX_LOG_FILE_PATH_MAX_LEN_BYTES];

    if (!path)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    if (dwMaxBackups <= 0)
    {
        /* truncate current file */
        fd = open(path, O_WRONLY | O_TRUNC);
        if (fd < 0)
        {
            eErr = LOGX_ERR_FD_OPEN_FAILED;
        }
        goto END;
    }

    /* delete the oldest backup (try compressed form first, then plain) */
    snprintf(oldname, sizeof(oldname), "%s.%d.gz", path, dwMaxBackups);
    unlink(oldname);
    snprintf(oldname, sizeof(oldname), "%s.%d", path, dwMaxBackups);
    unlink(oldname);

    /* shift backups: path.(N-1) → path.N, preserving .gz extension */
    for (int i = dwMaxBackups - 1; i >= 1; --i)
    {
        snprintf(oldname, sizeof(oldname), "%s.%d.gz", path, i);
        if (access(oldname, F_OK) == 0)
        {
            snprintf(newname, sizeof(newname), "%s.%d.gz", path, i + 1);
        }
        else
        {
            snprintf(oldname, sizeof(oldname), "%s.%d", path, i);
            snprintf(newname, sizeof(newname), "%s.%d", path, i + 1);
        }
        /* rename will fail silently if oldname doesn't exist — that's fine */
        rename(oldname, newname);
    }

    /* rotate the active log to path.1 (always uncompressed at this point) */
    snprintf(newname, sizeof(newname), "%s.1", path);
    rename(path, newname);

    /* compress after rotation, mirroring logrotate behaviour */
    if (compress)
    {
        if (!delay_compress)
        {
            /* compress the freshly rotated backup immediately */
            snprintf(oldname, sizeof(oldname), "%s.1", path);
            compress_file(oldname); /* failure is non-fatal: file stays uncompressed */
        }
        else
        {
            /* delaycompress: compress path.2 (from the previous rotation) if it is
             * still plain; path.1 will be compressed on the next rotation */
            snprintf(oldname, sizeof(oldname), "%s.2", path);
            if (access(oldname, F_OK) == 0)
            {
                compress_file(oldname);
            }
        }
    }

END:
    if (fd > 0)
    {
        close(fd);
    }
    return eErr;
}

/**
 * @brief Helper function that performs log rotation on the active log file.
 *
 * Flushes and closes the current log file, rotates the backup files via
 * rotate_files(), then reopens the log file for appending. The file is
 * exclusively locked for the duration of the operation to prevent concurrent
 * writes during rotation.
 *
 * @param[in,out] ptLogger Pointer to the logger instance. Must not be NULL.
 *                         On successful rotation, ptLogger->fp and ptLogger->fd
 *                         are updated to the newly opened file.
 *
 * @return LOGX_ERR_SUCCESS        Rotation completed and file reopened successfully.
 * @return LOGX_ERR_INVALID_ARG    ptLogger is NULL.
 * @return LOGX_ERR_FILE_OPEN_FAILED File could not be reopened after rotation;
 *                                  file logging is automatically disabled on
 *                                  the logger instance.
 *
 * @note If the file cannot be reopened after rotation, file logging is
 *       disabled (enable_file_logging = 0) to prevent further failed writes.
 */
static logx_errorcodes_t process_log_rotation(logx_t *ptLogger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    if (!ptLogger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    /* lock file */
    if (ptLogger->fd >= 0)
    {
        exclusive_flock(ptLogger->fd);
    }

    /* flush contents */
    if (ptLogger->fp)
    {
        fflush(ptLogger->fp);
    }

    /* perform log rotation */
    if ((eErr = rotate_files(ptLogger->cfg.file_path, ptLogger->cfg.rotate.max_backups,
                             ptLogger->cfg.rotate.compress,
                             ptLogger->cfg.rotate.delay_compress)) != LOGX_ERR_SUCCESS)
    {
        goto END;
    }

    /* close file */
    if (ptLogger->fp)
    {
        fclose(ptLogger->fp);
    }

    /* reopen file */
    ptLogger->fp = fopen(ptLogger->cfg.file_path, "a");
    if (ptLogger->fp)
    {
        ptLogger->fd = fileno(ptLogger->fp);
    }
    else
    {
        ptLogger->cfg.enable_file_logging = 0; /* disable file logging if we can't open file */
        unlock_flock(ptLogger->fd);
        ptLogger->fd = -1;
        eErr         = LOGX_ERR_FILE_OPEN_FAILED;
        goto END;
    }

    /* unlock file */
    if (ptLogger->fd >= 0)
    {
        unlock_flock(ptLogger->fd);
    }

END:
    return eErr;
}

/**
 * @brief Function that checks if log rotation is needed and performs log rotation
 *
 * @param[in] ptLogger - Pointer to the logger instance
 * @return logx_errorcodes_t
 */
logx_errorcodes_t check_and_rotate_log(logx_t *ptLogger)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;
    time_t t               = time(NULL);
    struct tm tm;
    char today[16];
    struct stat st;

    if (!ptLogger || !ptLogger->cfg.enable_file_logging || !ptLogger->cfg.file_path)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    if (ptLogger->cfg.rotate.type == LOGX_ROTATE_BY_DATE)
    {
        localtime_r(&t, &tm);
        snprintf(today, sizeof(today), "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1,
                 tm.tm_mday);
        if (strcmp(today, ptLogger->current_date) != 0)
        {
            if ((eErr = process_log_rotation(ptLogger)) != LOGX_ERR_SUCCESS)
            {
                goto END;
            }

            /* update current date */
            strncpy(ptLogger->current_date, today, sizeof(ptLogger->current_date));
        }
    }
    else if (ptLogger->cfg.rotate.type == LOGX_ROTATE_BY_SIZE)
    {
        if (ptLogger->fd >= 0)
        {
            if (fstat(ptLogger->fd, &st) == 0)
            {
                if ((size_t)st.st_size >=
                    (long unsigned int)CONVERT_MB_TO_BYTES(ptLogger->cfg.rotate.size_mb))
                {
                    if ((eErr = process_log_rotation(ptLogger)) != LOGX_ERR_SUCCESS)
                    {
                        goto END;
                    }
                }
            }
            else
            {
                eErr = LOGX_ERR_FSTAT_FAILED;
                goto END;
            }
        }
        else
        {
            eErr = LOGX_ERR_INVALID_FD;
            goto END;
        }
    }

    else if (ptLogger->cfg.rotate.type == LOGX_ROTATE_NONE)
    {
        ;
    }

    else
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

END:
    return eErr;
}

logx_errorcodes_t logx_set_log_rotate_type(logx_t *logger, logx_rotate_type_t type)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.type = type;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_set_log_file_size_mb(logx_t *logger, size_t size_mb)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.size_mb = size_mb;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_set_num_of_logfile_backups(logx_t *logger, int max_backups)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.max_backups = max_backups;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_set_rotation_after_days(logx_t *logger, int after_days)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    /* Sanity check */
    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.after_days = after_days;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_set_compress(logx_t *logger, int enable)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.compress = enable;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_set_delay_compress(logx_t *logger, int enable)
{
    logx_errorcodes_t eErr = LOGX_ERR_SUCCESS;

    if (!logger)
    {
        eErr = LOGX_ERR_INVALID_ARG;
        goto END;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.delay_compress = enable;
    pthread_mutex_unlock(&logger->lock);

END:
    return eErr;
}

logx_errorcodes_t logx_rotate_now(logx_t *logger)
{
    int r = 0;

    if (!logger)
    {
        return -1;
    }

    pthread_mutex_lock(&logger->lock);
    if (logger->cfg.enable_file_logging && logger->cfg.file_path)
    {
        if (logger->fd >= 0)
        {
            exclusive_flock(logger->fd);
        }

        if (logger->fp)
        {
            fflush(logger->fp);
        }

        r = rotate_files(logger->cfg.file_path, logger->cfg.rotate.max_backups,
                         logger->cfg.rotate.compress, logger->cfg.rotate.delay_compress);

        if (logger->fp)
        {
            fclose(logger->fp);
        }

        logger->fp = fopen(logger->cfg.file_path, "a");

        if (logger->fp)
        {
            logger->fd = fileno(logger->fp);
        }

        if (logger->fd >= 0)
        {
            unlock_flock(logger->fd);
        }
    }
    pthread_mutex_unlock(&logger->lock);
    return r;
}