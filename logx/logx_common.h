#ifndef LOGX_COMMON_H
#define LOGX_COMMON_H

#include "logx_errorcodes.h"
#include "logx_types.h"

#define ARRAY_SIZE(arr)        (sizeof(arr) / sizeof((arr)[0]))
#define CONVERT_MB_TO_BYTES(x) x * 1024 * 1024

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

#define COLOR_BLUE    "\x1b[34m"
#define COLOR_WHITE   "\x1b[37m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"

/* Enum lookup tables — one row per enum value; shared by to_string, from_str, is_valid */
typedef struct
{
    logx_level_t val;
    const char *abbr;
    const char *name;
    const char *color;
} logx_level_entry_t;

typedef struct
{
    logx_rotate_type_t val;
    const char *disp;
    const char *name;
} logx_rotate_entry_t;

typedef struct
{
    logx_ts_fmt_t val;
    const char *name;
} logx_ts_fmt_entry_t;

extern const logx_level_entry_t LOGX_LEVEL_MAP[];
extern const logx_rotate_entry_t LOGX_ROTATE_MAP[];
extern const logx_ts_fmt_entry_t LOGX_TS_FMT_MAP[];
extern const size_t LOGX_LEVEL_MAP_COUNT;
extern const size_t LOGX_ROTATE_MAP_COUNT;
extern const size_t LOGX_TS_FMT_MAP_COUNT;

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

const char *logx_check(int bEnable);
const char *logx_level_to_string(logx_level_t eLogLevel);
const char *logx_level_to_color(logx_level_t eLogLevel);
const char *logx_rotate_type_to_string(logx_rotate_type_t eRotateType);
const char *logx_ts_fmt_to_string(logx_ts_fmt_t eTsFormat);
logx_errorcodes_t is_valid_logx_rotate_type(logx_rotate_type_t eRotateType);
logx_errorcodes_t is_valid_logx_level(logx_level_t eLogLevel);
logx_errorcodes_t exclusive_flock(int fd);
logx_errorcodes_t unlock_flock(int fd);
logx_errorcodes_t ensure_parent_dir_exists(const char *path);

#endif /* LOGX_COMMON_H */