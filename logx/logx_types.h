#ifndef LOGX_TYPES_H
#define LOGX_TYPES_H

#include <stddef.h>
#include <stdint.h>

/* Forward declarations */
typedef struct logx_t logx_t;
typedef struct logx_cfg_t logx_cfg_t;
typedef struct logx_rotate_cfg_t logx_rotate_cfg_t;
typedef struct logx_timer_t logx_timer_t;

/* enums */
typedef enum
{
    LOGX_LEVEL_TRACE = 0,
    LOGX_LEVEL_DEBUG,
    LOGX_LEVEL_BANNER,
    LOGX_LEVEL_INFO,
    LOGX_LEVEL_WARN,
    LOGX_LEVEL_ERROR,
    LOGX_LEVEL_FATAL,
    LOGX_LEVEL_OFF
} logx_level_t;

typedef enum
{
    LOGX_FIELD_STRING,
    LOGX_FIELD_BOOL,
    LOGX_FIELD_INT,
    LOGX_FIELD_LEVEL,
    LOGX_FIELD_ROTATE_TYPE,
    LOGX_FIELD_TS_FMT,
    LOGX_FIELD_SYSLOG_FACILITY
} logx_field_type_t;

typedef enum
{
    LOGX_ROTATE_NONE = 0,
    LOGX_ROTATE_BY_SIZE,
    LOGX_ROTATE_BY_DATE
} logx_rotate_type_t;

typedef enum
{
    LOGX_TS_FMT_LOCAL,    /* 2026-05-16 14:32:01.123  (local time, your current format) */
    LOGX_TS_FMT_UTC,      /* 2026-05-16 08:32:01.123Z (UTC)                             */
    LOGX_TS_FMT_EPOCH_S,  /* 1747384321                (Unix seconds)                   */
    LOGX_TS_FMT_EPOCH_MS, /* 1747384321123             (Unix milliseconds)              */
    LOGX_TS_FMT_EPOCH_US, /* 1747384321123456          (Unix microseconds)              */
    LOGX_TS_FMT_ISO8601,  /* 2026-05-16T08:32:01.123Z  (ISO 8601 / RFC 3339)           */
    LOGX_TS_FMT_RFC2822,  /* Sat, 16 May 2026 08:32:01 +0000                           */
} logx_ts_fmt_t;

typedef enum
{
    LOGX_ERR_FAILURE = -1, /* manually anchored to -1 */

#define LOGX_ERROR_AUTO(code)          code,
#define LOGX_ERROR_EXPLICIT(code, val) code = val,
#include "logx_errorcodes.def"
#undef LOGX_ERROR_AUTO
#undef LOGX_ERROR_EXPLICIT

} logx_errorcodes_t;

#endif /* LOGX_TYPES_H */