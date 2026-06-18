/**
 * @file logx_types.h
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Core type definitions, enumerations, and forward declarations for LogX.
 * @version 2.0.0
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 */

#ifndef LOGX_TYPES_H
#define LOGX_TYPES_H

#include <stddef.h>
#include <stdint.h>

/** @brief Forward declaration for the logger instance. */
typedef struct logx_t logx_t;

/** @brief Forward declaration for the logger configuration. */
typedef struct logx_cfg_t logx_cfg_t;

/** @brief Forward declaration for log rotation configuration. */
typedef struct logx_rotate_cfg_t logx_rotate_cfg_t;

/** @brief Forward declaration for a stopwatch timer. */
typedef struct logx_timer_t logx_timer_t;

/**
 * @brief Log severity levels, ordered from lowest to highest.
 *
 * Pass `LOGX_LEVEL_OFF` to disable logging entirely on a given output.
 */
typedef enum
{
    LOGX_LEVEL_TRACE = 0, /**< Verbose flow tracing; lowest severity. */
    LOGX_LEVEL_DEBUG,     /**< Developer-only diagnostic information. */
    LOGX_LEVEL_BANNER,    /**< Milestone banners; auto-centered in output. */
    LOGX_LEVEL_INFO,      /**< General operational information. */
    LOGX_LEVEL_WARN,      /**< Non-fatal anomalies that deserve attention. */
    LOGX_LEVEL_ERROR,     /**< Recoverable errors. */
    LOGX_LEVEL_FATAL,     /**< Unrecoverable errors; program should terminate. */
    LOGX_LEVEL_OFF        /**< Disable logging on this output channel. */
} logx_level_t;

/**
 * @brief Internal field type tag used by the config parser dispatch table.
 * @internal
 */
typedef enum
{
    LOGX_FIELD_STRING,          /**< Field holds a `const char *`. */
    LOGX_FIELD_BOOL,            /**< Field holds a boolean-like `int` (0 or 1). */
    LOGX_FIELD_INT,             /**< Field holds a generic integer. */
    LOGX_FIELD_LEVEL,           /**< Field holds a `logx_level_t`. */
    LOGX_FIELD_ROTATE_TYPE,     /**< Field holds a `logx_rotate_type_t`. */
    LOGX_FIELD_TS_FMT,          /**< Field holds a `logx_ts_fmt_t`. */
    LOGX_FIELD_SYSLOG_FACILITY  /**< Field holds a `logx_syslog_facility_t`. */
} logx_field_type_t;

/**
 * @brief Log file rotation strategy.
 */
typedef enum
{
    LOGX_ROTATE_NONE = 0, /**< No automatic rotation. */
    LOGX_ROTATE_BY_SIZE,  /**< Rotate when the log file exceeds a size threshold. */
    LOGX_ROTATE_BY_DATE   /**< Rotate after a configured number of days. */
} logx_rotate_type_t;

/**
 * @brief Timestamp format written to each log entry.
 */
typedef enum
{
    LOGX_TS_FMT_LOCAL,    /**< Local wall-clock time: `2026-05-16 14:32:01.123`       */
    LOGX_TS_FMT_UTC,      /**< UTC wall-clock time:   `2026-05-16 08:32:01.123Z`      */
    LOGX_TS_FMT_EPOCH_S,  /**< Unix epoch seconds:    `1747384321`                    */
    LOGX_TS_FMT_EPOCH_MS, /**< Unix epoch milliseconds: `1747384321123`               */
    LOGX_TS_FMT_EPOCH_US, /**< Unix epoch microseconds: `1747384321123456`            */
    LOGX_TS_FMT_ISO8601,  /**< ISO 8601 / RFC 3339:   `2026-05-16T08:32:01.123Z`     */
    LOGX_TS_FMT_RFC2822,  /**< RFC 2822 email format: `Sat, 16 May 2026 08:32:01 +0000` */
} logx_ts_fmt_t;

/**
 * @brief Return codes used by all public LogX APIs.
 *
 * `LOGX_ERR_SUCCESS` (0) indicates success. All other values indicate failure.
 * `LOGX_ERR_FAILURE` (-1) is a generic failure sentinel.
 * The remaining codes are generated from logx_errorcodes.def.
 */
typedef enum
{
    LOGX_ERR_FAILURE = -1, /**< Generic failure. */

#define LOGX_ERROR_AUTO(code)          code,
#define LOGX_ERROR_EXPLICIT(code, val) code = val,
#include "logx_errorcodes.def"
#undef LOGX_ERROR_AUTO
#undef LOGX_ERROR_EXPLICIT

} logx_errorcodes_t;

#endif /* LOGX_TYPES_H */