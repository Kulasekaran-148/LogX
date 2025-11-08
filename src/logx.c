#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "../include/logx/logx.h"

struct logger_t {
	logger_cfg_t cfg;
	FILE *fp;	       /* opened log file */
	int fd;		       /* file descriptor for locking/stat */
	pthread_mutex_t lock;  /* thread safety */
	char current_date[16]; /* YYYY-MM-DD for date based rotation */
};

/* ANSI color codes */
static const char *COLOR_DEBUG = "\x1b[37m";  /* white */
static const char *COLOR_INFO = "\x1b[32m";   /* green */
static const char *COLOR_WARN = "\x1b[33m";   /* yellow */
static const char *COLOR_ERROR = "\x1b[31m";  /* red */
static const char *COLOR_BANNER = "\x1b[36m"; /* cyan */
static const char *COLOR_FATAL = "\x1b[35m";  /* purple */
static const char *COLOR_RESET = "\x1b[0m";

const char *log_level_to_string(log_level_t level)
{
	switch (level) {
		case LOG_LEVEL_DEBUG:
			return "DBG";
		case LOG_LEVEL_INFO:
			return "INF";
		case LOG_LEVEL_WARN:
			return "WRN";
		case LOG_LEVEL_ERROR:
			return "ERR";
		case LOG_LEVEL_BANNER:
			return "INF";
		case LOG_LEVEL_FATAL:
			return "FTL";
		default:
			return "MSC";
	}
}

/* Internal helper to get millisecond timestamp */
static void now_ts(char *out, size_t out_sz, struct timeval *tv)
{
	if (!tv) {
		struct timeval ttmp;
		gettimeofday(&ttmp, NULL);
		tv = &ttmp;
	}
	struct tm tm;
	localtime_r(&tv->tv_sec, &tm);
	int ms = (int)(tv->tv_usec / 1000);
	snprintf(out, out_sz, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
		 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
		 tm.tm_min, tm.tm_sec, ms);
}

/* file locking helper - uses flock (advisory). Returns 0 on success */
static int file_lock_ex(int fd)
{
	if (fd < 0)
		return -1;
	if (flock(fd, LOCK_EX) == -1)
		return -1;
	return 0;
}

static int file_lock_un(int fd)
{
	if (fd < 0)
		return -1;
	if (flock(fd, LOCK_UN) == -1)
		return -1;
	return 0;
}

/* rotate by renaming files. keeps max_backups; oldest gets removed */
static int rotate_files(const char *path, int max_backups)
{
	char oldname[1024];
	char newname[1024];

	if (!path) {
		return -1;
	}

	if (max_backups <= 0) {
		/* truncate current file */
		int fd = open(path, O_WRONLY | O_TRUNC);
		if (fd >= 0) {
			close(fd);
		}

		return 0;
	}

	snprintf(oldname, sizeof(oldname), "%s.%d", path, max_backups);
	unlink(oldname); /* ignore errors */

	for (int i = max_backups - 1; i >= 0; --i) {
		if (i == 0) {
			snprintf(oldname, sizeof(oldname), "%s", path);
		} else {
			snprintf(oldname, sizeof(oldname), "%s.%d", path, i);
		}
		snprintf(newname, sizeof(newname), "%s.%d", path, i + 1);
		/* rename will fail if oldname doesn't exist - that's fine */
		rename(oldname, newname);
	}
	return 0;
}

/* Check rotation conditions and perform rotation if needed. Must be called with mutex held. */
static int check_and_rotate_locked(logger_t *l)
{
	if (!l || !l->cfg.enable_file_logging || !l->cfg.file_path)
		return 0;

	if (l->cfg.rotate.type == LOG_ROTATE_BY_DATE) {
		time_t t = time(NULL);
		struct tm tm;
		localtime_r(&t, &tm);
		char today[16];
		snprintf(today, sizeof(today), "%04d-%02d-%02d",
			 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
		if (strcmp(today, l->current_date) != 0) {
			/* rotate */
			if (l->fd >= 0) {
				file_lock_ex(l->fd);
			}
			if (l->fp)
				fflush(l->fp);
			rotate_files(l->cfg.file_path,
				     l->cfg.rotate.max_backups);
			/* reopen file */
			if (l->fp)
				fclose(l->fp);
			l->fp = fopen(l->cfg.file_path, "a");
			if (l->fp)
				l->fd = fileno(l->fp);
			if (l->fd >= 0)
				file_lock_un(l->fd);
			strncpy(l->current_date, today,
				sizeof(l->current_date));
		}
	} else if (l->cfg.rotate.type == LOG_ROTATE_BY_SIZE) {
		if (l->fd >= 0) {
			struct stat st;
			if (fstat(l->fd, &st) == 0) {
				if ((size_t)st.st_size >=
				    l->cfg.rotate.max_bytes) {
					file_lock_ex(l->fd);
					if (l->fp)
						fflush(l->fp);
					rotate_files(l->cfg.file_path,
						     l->cfg.rotate.max_backups);
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

logger_t *logger_create(const logger_cfg_t *cfg)
{
	if (!cfg) {
		return NULL;
	}

	/* Using calloc() to explicitly initialize values to zero, so that they don't contain garbage */
	logger_t *l = calloc(1, sizeof(*l));

	if (!l) {
		return NULL;
	}

	/* copy cfg shallowly; copy strings by pointer - caller must keep them alive or they can be duplicated by caller */
	memcpy(&l->cfg, cfg, sizeof(l->cfg));

	pthread_mutex_init(&l->lock, NULL);

	l->fp = NULL;
	l->fd = -1;
	l->current_date[0] = '\0';

	if (l->cfg.enable_file_logging && l->cfg.file_path) {
		l->fp = fopen(l->cfg.file_path, "a");
		if (!l->fp) {
			/* cannot open file - instead disable file logging */
			l->cfg.enable_file_logging = 0;
		} else {
			l->fd = fileno(l->fp);

			/* initialize date tracking */
			time_t t = time(NULL);
			struct tm tm;
			localtime_r(&t, &tm);
			snprintf(l->current_date, sizeof(l->current_date),
				 "%04d-%02d-%02d", tm.tm_year + 1900,
				 tm.tm_mon + 1, tm.tm_mday);
		}
	}

	return l;
}

void logger_destroy(logger_t *logger)
{
	if (!logger) {
		return;
	}

	pthread_mutex_lock(&logger->lock);

	if (logger->fp) {
		fflush(logger->fp);
		fclose(logger->fp);
		logger->fp = NULL;
		logger->fd = -1;
	}

	pthread_mutex_unlock(&logger->lock);
	pthread_mutex_destroy(&logger->lock);

	free(logger);
}

void logger_set_console_level(logger_t *logger, log_level_t level)
{
	if (!logger) {
		return;
	}

	pthread_mutex_lock(&logger->lock);
	logger->cfg.console_level = level;
	pthread_mutex_unlock(&logger->lock);
}

void logger_set_file_level(logger_t *logger, log_level_t level)
{
	if (!logger) {
		return;
	}

	pthread_mutex_lock(&logger->lock);
	logger->cfg.file_level = level;
	pthread_mutex_unlock(&logger->lock);
}

void logger_enable_console_logging(logger_t *logger, int enable)
{
	if (!logger) {
		return;
	}

	pthread_mutex_lock(&logger->lock);
	logger->cfg.enable_console_logging = enable ? 1 : 0;
	pthread_mutex_unlock(&logger->lock);
}

void logger_enable_file_logging(logger_t *logger, int enable)
{
	if (!logger) {
		return;
	}

	pthread_mutex_lock(&logger->lock);
	if (enable && !logger->cfg.file_path) {
		/* cannot enable without path */
		/* must not use Logger to log the below message since we're already inside the logger */
		fprintf(stderr, "ERROR: cannot enable file logging without "
				"valid file path\n");
		logger->cfg.enable_file_logging = 0;
	} else {
		logger->cfg.enable_file_logging = enable ? 1 : 0;
	}
	pthread_mutex_unlock(&logger->lock);
}

int logger_rotate_now(logger_t *logger)
{
	int r = 0;

	if (!logger) {
		return -1;
	}

	pthread_mutex_lock(&logger->lock);
	if (logger->cfg.enable_file_logging && logger->cfg.file_path) {
		if (logger->fd >= 0) {
			file_lock_ex(logger->fd);
		}

		if (logger->fp) {
			fflush(logger->fp);
		}

		r = rotate_files(logger->cfg.file_path,
				 logger->cfg.rotate.max_backups);

		if (logger->fp) {
			fclose(logger->fp);
		}

		logger->fp = fopen(logger->cfg.file_path, "a");

		if (logger->fp) {
			logger->fd = fileno(logger->fp);
		}

		if (logger->fd >= 0) {
			file_lock_un(logger->fd);
		}
	}
	pthread_mutex_unlock(&logger->lock);
	return r;
}

void logger_log(logger_t *logger, log_level_t level, const char *file,
		const char *func, int line, const char *fmt, ...)
{
	if (!logger)
		return;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	pthread_mutex_lock(&logger->lock);

	/* Check thresholds */
	int write_console = logger->cfg.enable_console_logging &&
			    level >= logger->cfg.console_level;
	int write_file = logger->cfg.enable_file_logging &&
			 level >= logger->cfg.file_level && logger->fp;

	if (!write_console && !write_file) {
		pthread_mutex_unlock(&logger->lock);
		return;
	}

	/* rotation check */
	check_and_rotate_locked(logger);

	char ts[64];
	now_ts(ts, sizeof(ts), &tv);

	/* prepare message payload */
	char payload[4096];
	char linebuf[4096];
	char border[4096 + 10]; // payload max + margins
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(payload, sizeof(payload), fmt, ap);
	va_end(ap);

	int use_color = logger->cfg.enabled_colored_logs;
	const char *c = COLOR_RESET;

	if (use_color) {
		switch (level) {
			case LOG_LEVEL_DEBUG:
				c = COLOR_DEBUG;
				break;
			case LOG_LEVEL_INFO:
				c = COLOR_INFO;
				break;
			case LOG_LEVEL_WARN:
				c = COLOR_WARN;
				break;
			case LOG_LEVEL_ERROR:
				c = COLOR_ERROR;
				break;
			case LOG_LEVEL_BANNER:
				c = COLOR_BANNER;
				break;
			case LOG_LEVEL_FATAL:
				c = COLOR_FATAL;
				break;
			default:
				c = COLOR_RESET;
				break;
		}
	}
	/* If it's a banner log, build the banner */
	if (level == LOG_LEVEL_BANNER) {
		const char *pattern = (logger->cfg.banner_pattern &&
				       *logger->cfg.banner_pattern)
					      ? logger->cfg.banner_pattern
					      : "=";

		size_t msg_len = strlen(payload);
		size_t pattern_len = strlen(pattern);

		/* --- border generation --- */
		size_t border_len = (msg_len < sizeof(border) - 11)
					    ? msg_len
					    : sizeof(border) - 11;

		// Add padding on both sides (5 chars each)
		size_t padded_len = border_len + 10;
		if (padded_len > sizeof(border) - 1)
			padded_len = sizeof(border) - 1;

		for (size_t i = 0; i < padded_len; ++i)
			border[i] = pattern[i % pattern_len];

		border[padded_len] = '\0';
	}

	snprintf(linebuf, sizeof(linebuf), "[%s] [%s] (%s:%s:%d): ", ts,
		 log_level_to_string(level), file ? file : "?",
		 func ? func : "?", line);

	int gap_len = strlen(linebuf);

	/* Console write */
	if (write_console) {
		FILE *out = (level >= LOG_LEVEL_WARN) ? stderr : stdout;
		if (logger->cfg.use_tty_detection)
			use_color = use_color && isatty(fileno(out));

		if (use_color) {
			if (level == LOG_LEVEL_BANNER) {
				fprintf(out, "%s%s%s", c, linebuf, COLOR_RESET);
				fprintf(out, "%s%s%s\n", c, border,
					COLOR_RESET);
				fprintf(out, "%*s", gap_len, "");
				fprintf(out, "%s%*s%s%s\n", c, 5, "", payload,
					COLOR_RESET);
				fprintf(out, "%*s", gap_len, "");
				fprintf(out, "%s%s%s\n", c, border,
					COLOR_RESET);
			} else {
				fprintf(out, "%s%s%s", c, linebuf, COLOR_RESET);
				fprintf(out, "%s%s%s\n", c, payload,
					COLOR_RESET);
			}
		} else {
			if (level == LOG_LEVEL_BANNER) {
				fprintf(out, "%s", linebuf);
				fprintf(out, "%s\n", border);
				fprintf(out, "%*s", gap_len, "");
				fprintf(out, "%*s%s\n", 5, "", payload);
				fprintf(out, "%*s", gap_len, "");
				fprintf(out, "%s\n", border);
			} else {
				fprintf(out, "%s", linebuf);
				fprintf(out, "%s\n", payload);
			}
		}

		fflush(out);
	}

	/* File write */
	if (write_file) {
		if (logger->fd >= 0)
			file_lock_ex(logger->fd);

		if (logger->fp) {
			if (level == LOG_LEVEL_BANNER) {
				fprintf(logger->fp, "%s", linebuf);
				fprintf(logger->fp, "%s\n", border);
				fprintf(logger->fp, "%*s", gap_len, "");
				fprintf(logger->fp, "%*s%s\n", 5, "", payload);
				fprintf(logger->fp, "%*s", gap_len, "");
				fprintf(logger->fp, "%s\n", border);
			} else {
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
