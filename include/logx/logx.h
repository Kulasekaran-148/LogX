#ifndef C_LOGX_H
#define C_LOGX_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define MAJ_VERSION 1
#define MIN_VERSION 0
#define PATCH_VERSION 0

#ifdef __cplusplus
extern "C" {
#endif

/* Log levels */
typedef enum {
	LOG_LEVEL_DEBUG = 0,
	LOG_LEVEL_BANNER,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL,
	LOG_LEVEL_OFF
} log_level_t;

/* Rotation type */
typedef enum {
	LOG_ROTATE_NONE = 0,
	LOG_ROTATE_BY_SIZE,
	LOG_ROTATE_BY_DATE
} log_rotate_type_t;

/* Rotation configuration */
typedef struct {
	log_rotate_type_t type; /* type of rotation */
	size_t max_bytes;	/* used when tyep == LOG_ROTATE_BY_SIZE */
	int max_backups; /* number of backup files to keep (0 = no backups) */
	int daily_interval; /* days between rotations when LOG_ROTATE_BY_DATE (1 = daily) */
} log_rotate_cfg_t;

/* Logger configuration passed to create function */
typedef struct {
	const char *name;	   /* logical name of logger (used in prefix) */
	const char *file_path;	   /* if NULL then file logging disabled */
	log_level_t console_level; /* level threshold for console */
	log_level_t file_level;	   /* level threshold for file */
	int enable_console_logging; /* 0/1 */
	int enable_file_logging;    /* 0/1 */
	int enabled_colored_logs;   /* 0/1 */
	int use_tty_detection; /* if 1, detect isatty and disable colors for non-ttys */
	log_rotate_cfg_t rotate; /* rotation options */
	const char *banner_pattern;
} logger_cfg_t;

/* Opaque logger handle - definition is in the .c file but we expose the struct type for static alloc options */
typedef struct logger_t logger_t;

/* Create and initialize a logger. Returns NULL on failure. */
logger_t *logger_create(const logger_cfg_t *cfg);

/* Close and free resources. Safe to call multiple times. */
void logger_destroy(logger_t *logger);

/* Change levels at runtime. */
void logger_set_console_level(logger_t *logger, log_level_t level);
void logger_set_file_level(logger_t *logger, log_level_t level);

/* Toggle outputs */
void logger_enable_console_logging(logger_t *logger, int enable);
void logger_enable_file_logging(logger_t *logger, int enable);

/* Force an immediate rotation (useful for admin triggers) */
int logger_rotate_now(logger_t *logger);

/* Log a message. file/func/line are helpers provided by macros below. */
void logger_log(logger_t *logger, log_level_t level, const char *file,
		const char *func, int line, const char *fmt, ...);

/* Macros for easy logging (these expand to a call that includes file/func/line) */
#define LOG_DEBUG(logger, fmt, ...)                                            \
	logger_log((logger), LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__,    \
		   (fmt), ##__VA_ARGS__)
#define LOG_INFO(logger, fmt, ...)                                             \
	logger_log((logger), LOG_LEVEL_INFO, __FILE__, __func__, __LINE__,     \
		   (fmt), ##__VA_ARGS__)
#define LOG_WARN(logger, fmt, ...)                                             \
	logger_log((logger), LOG_LEVEL_WARN, __FILE__, __func__, __LINE__,     \
		   (fmt), ##__VA_ARGS__)
#define LOG_ERROR(logger, fmt, ...)                                            \
	logger_log((logger), LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__,    \
		   (fmt), ##__VA_ARGS__)
#define LOG_BANNER(logger, fmt, ...)                                           \
	logger_log((logger), LOG_LEVEL_BANNER, __FILE__, __func__, __LINE__,   \
		   (fmt), ##__VA_ARGS__)
#define LOG_FATAL(logger, fmt, ...)                                            \
	logger_log((logger), LOG_LEVEL_FATAL, __FILE__, __func__, __LINE__,    \
		   (fmt), ##__VA_ARGS__)

/* Helper: convert level to string */
const char *log_level_to_string(log_level_t level);

extern logger_t *logger;
#ifdef __cplusplus
}
#endif

#endif
