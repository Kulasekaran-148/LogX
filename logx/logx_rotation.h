/**
 * @file logx_rotation.h
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Log file rotation configuration and public rotation APIs.
 * @version 2.0.0
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 */

#ifndef LOGX_ROTATION_H
#define LOGX_ROTATION_H

#include "logx_errorcodes.h"
#include "logx_types.h"
#include "stddef.h"

/**
 * @brief Log rotation configuration embedded inside `logx_cfg_t`.
 */
struct logx_rotate_cfg_t
{
    logx_rotate_type_t type; /**< Rotation strategy: none, by size, or by date. */
    size_t size_mb;          /**< Max log file size in MB before size-based rotation. */
    int max_backups;         /**< Number of backup files to keep (0 = truncate, no backup). */
    int after_days;          /**< Days between rotations for date-based rotation. */
    int compress;            /**< 1 = gzip-compress rotated backup files (appends `.gz`). */
    int delay_compress;      /**< 1 = skip compressing the most-recently rotated backup; compress it on the next rotation instead (like logrotate's `delaycompress`). */
};

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Change the log rotation strategy at runtime.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] type   One of `LOGX_ROTATE_NONE`, `LOGX_ROTATE_BY_SIZE`, `LOGX_ROTATE_BY_DATE`.
     * @return `LOGX_ERR_SUCCESS` on success, `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_log_rotate_type(logx_t *logger, logx_rotate_type_t type);

    /**
     * @brief Force an immediate log rotation regardless of size or date thresholds.
     *
     * @param[in] logger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS` on success.
     */
    logx_errorcodes_t logx_rotate_now(logx_t *logger);

    /**
     * @brief Set the maximum log file size in MB for size-based rotation.
     *
     * @param[in] logger  Pointer to the logger instance.
     * @param[in] size_mb Maximum file size in megabytes.
     * @return `LOGX_ERR_SUCCESS` on success, `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_log_file_size_mb(logx_t *logger, size_t size_mb);

    /**
     * @brief Set the number of backup log files to keep during rotation.
     *
     * @param[in] logger      Pointer to the logger instance.
     * @param[in] max_backups Number of backups to retain. Pass 0 to truncate without backup.
     * @return `LOGX_ERR_SUCCESS` on success, `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_num_of_logfile_backups(logx_t *logger, int max_backups);

    /**
     * @brief Set the rotation interval in days for date-based rotation.
     *
     * @param[in] logger     Pointer to the logger instance.
     * @param[in] after_days Number of days between rotations (e.g. 1 = daily).
     * @return `LOGX_ERR_SUCCESS` on success, `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_rotation_after_days(logx_t *logger, int after_days);

    /**
     * @brief Enable or disable gzip compression of rotated backup files.
     *
     * When enabled, each backup file is compressed to `<path>.N.gz` immediately
     * after rotation (unless `delay_compress` is also enabled).
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] enable 1 to enable compression, 0 to disable.
     * @return `LOGX_ERR_SUCCESS` on success, `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_compress(logx_t *logger, int enable);

    /**
     * @brief Enable or disable delayed compression of the most-recently rotated backup.
     *
     * When both `compress` and `delay_compress` are enabled, the file rotated to
     * `<path>.1` is left uncompressed until the *next* rotation, at which point it
     * is compressed before being shifted to `<path>.2.gz`. This mirrors logrotate's
     * `delaycompress` directive and is useful when another process may still hold
     * the recently rotated file open.
     *
     * Has no effect unless `compress` is also enabled.
     *
     * @param[in] logger Pointer to the logger instance.
     * @param[in] enable 1 to enable delayed compression, 0 to disable.
     * @return `LOGX_ERR_SUCCESS` on success, `LOGX_ERR_INVALID_ARG` if logger is NULL.
     */
    logx_errorcodes_t logx_set_delay_compress(logx_t *logger, int enable);

    /**
     * @brief Internal — check rotation criteria and rotate if needed.
     * @internal
     *
     * Called automatically inside `logx_log` before each file write. Not intended for direct
     * use by application code; use `logx_rotate_now` to force immediate rotation instead.
     *
     * @param[in,out] ptLogger Pointer to the logger instance.
     * @return `LOGX_ERR_SUCCESS` if no rotation was needed or rotation succeeded.
     */
    logx_errorcodes_t check_and_rotate_log(logx_t *ptLogger);

#ifdef __cplusplus
}
#endif

#endif /* LOGX_ROTATION_H */