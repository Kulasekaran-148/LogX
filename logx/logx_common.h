/**
 * @file logx_common.h
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Shared utility macros, ANSI colour codes, and internal helper declarations.
 * @version 2.0.0
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 */

#ifndef LOGX_COMMON_H
#define LOGX_COMMON_H

#include "logx_errorcodes.h"
#include "logx_types.h"

/** @brief Number of elements in a statically-sized array. */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/** @brief Convert megabytes to bytes. */
#define CONVERT_MB_TO_BYTES(x) x * 1024 * 1024

/** @defgroup logx_colors ANSI colour escape codes
 *  @{
 */
#define COLOR_BLUE    "\x1b[34m" /**< ANSI foreground blue. */
#define COLOR_WHITE   "\x1b[37m" /**< ANSI foreground white. */
#define COLOR_GREEN   "\x1b[32m" /**< ANSI foreground green. */
#define COLOR_YELLOW  "\x1b[33m" /**< ANSI foreground yellow. */
#define COLOR_RED     "\x1b[31m" /**< ANSI foreground red. */
#define COLOR_CYAN    "\x1b[36m" /**< ANSI foreground cyan. */
#define COLOR_MAGENTA "\x1b[35m" /**< ANSI foreground magenta. */
#define COLOR_RESET   "\x1b[0m"  /**< Reset all ANSI attributes. */
/** @} */

/** @brief Lookup-table row for a log level — maps enum value to display strings and colour. */
typedef struct
{
    logx_level_t val;  /**< Enum value. */
    const char *abbr;  /**< Short abbreviation used in log prefixes, e.g. `"TRC"`. */
    const char *name;  /**< Full name used in config files, e.g. `"TRACE"`. */
    const char *color; /**< ANSI escape code for this level. */
} logx_level_entry_t;

/** @brief Lookup-table row for a rotation type. */
typedef struct
{
    logx_rotate_type_t val; /**< Enum value. */
    const char *disp;       /**< Human-readable display string, e.g. `"By Size"`. */
    const char *name;       /**< Config-file name, e.g. `"BY_SIZE"`. */
} logx_rotate_entry_t;

/** @brief Lookup-table row for a timestamp format. */
typedef struct
{
    logx_ts_fmt_t val; /**< Enum value. */
    const char *name;  /**< Config-file name, e.g. `"ISO8601"`. */
} logx_ts_fmt_entry_t;

extern const logx_level_entry_t LOGX_LEVEL_MAP[];   /**< Level lookup table. */
extern const logx_rotate_entry_t LOGX_ROTATE_MAP[]; /**< Rotation-type lookup table. */
extern const logx_ts_fmt_entry_t LOGX_TS_FMT_MAP[]; /**< Timestamp-format lookup table. */
extern const size_t LOGX_LEVEL_MAP_COUNT;           /**< Number of rows in LOGX_LEVEL_MAP. */
extern const size_t LOGX_ROTATE_MAP_COUNT;          /**< Number of rows in LOGX_ROTATE_MAP. */
extern const size_t LOGX_TS_FMT_MAP_COUNT;          /**< Number of rows in LOGX_TS_FMT_MAP. */

#ifdef __cplusplus
extern "C"
{
#endif

    const char *logx_bin_str64_grouped_tls(uint64_t value);
    logx_errorcodes_t logx_enable_print_config(logx_t *logger);
    logx_errorcodes_t logx_disable_print_config(logx_t *logger);

#ifdef __cplusplus
}
#endif

/**
 * @brief Return `"Enabled"` or `"Disabled"` based on a boolean integer.
 * @param[in] bEnable Non-zero for enabled.
 * @return `"Enabled"` or `"Disabled"`.
 */
const char *logx_check(int bEnable);

/**
 * @brief Return the abbreviated string for a log level (e.g. `"TRC"`).
 * @param[in] eLogLevel Log level enum value.
 * @return Null-terminated abbreviation string, or `"ukwn"` if not found.
 */
const char *logx_level_to_string(logx_level_t eLogLevel);

/**
 * @brief Return the ANSI colour escape code for a log level.
 * @param[in] eLogLevel Log level enum value.
 * @return ANSI escape string, or `COLOR_RESET` if not found.
 */
const char *logx_level_to_color(logx_level_t eLogLevel);

/**
 * @brief Return the display string for a rotation type (e.g. `"By Size"`).
 * @param[in] eRotateType Rotation type enum value.
 * @return Null-terminated display string, or `"ukwn"`.
 */
const char *logx_rotate_type_to_string(logx_rotate_type_t eRotateType);

/**
 * @brief Return the name string for a timestamp format (e.g. `"ISO8601"`).
 * @param[in] eTsFormat Timestamp format enum value.
 * @return Null-terminated name string, or `"ukwn"`.
 */
const char *logx_ts_fmt_to_string(logx_ts_fmt_t eTsFormat);

/**
 * @brief Validate a rotation type enum value.
 * @param[in] eRotateType Value to validate.
 * @return `LOGX_ERR_SUCCESS` if valid, `LOGX_ERR_FAILURE` otherwise.
 */
logx_errorcodes_t is_valid_logx_rotate_type(logx_rotate_type_t eRotateType);

/**
 * @brief Validate a log level enum value.
 * @param[in] eLogLevel Value to validate.
 * @return `LOGX_ERR_SUCCESS` if valid, `LOGX_ERR_FAILURE` otherwise.
 */
logx_errorcodes_t is_valid_logx_level(logx_level_t eLogLevel);

/**
 * @brief Acquire an exclusive advisory lock on a file descriptor using `flock()`.
 * @param[in] fd Open file descriptor.
 * @return `LOGX_ERR_SUCCESS` on success.
 */
logx_errorcodes_t exclusive_flock(int fd);

/**
 * @brief Release a previously acquired `flock()` advisory lock.
 * @param[in] fd Open file descriptor.
 * @return `LOGX_ERR_SUCCESS` on success.
 */
logx_errorcodes_t unlock_flock(int fd);

/**
 * @brief Ensure all parent directories for a file path exist, creating them if needed.
 * @param[in] path Full file path whose parent directories should be created.
 * @return `LOGX_ERR_SUCCESS` if directories already exist or were created successfully.
 */
logx_errorcodes_t ensure_parent_dir_exists(const char *path);

#endif /* LOGX_COMMON_H */