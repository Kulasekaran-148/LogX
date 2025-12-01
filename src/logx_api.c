#define _GNU_SOURCE

#include "../include/logx/logx.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Changes the console log level
 *
 * @param[in] logger Pointer to the logx logger instance whose console logging level is to be
 * changed
 * @param[in] level specifies the logging level
 */
void logx_set_console_logging_level(logx_t *logger, logx_level_t level) {
    if (!logger) {
        return;
    }

    if (is_valid_logx_level(level) == 0)
        ;
    else {
        fprintf(stderr, "[LogX] %s: Error: Unknown logx_level - %d", __func__, (int)level);
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.console_level = level;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Changes file log level for a logger instance
 *
 * @param[in] logger Pointer to the logx logger instance whose file logging level is to be changed
 * @param[in] level specifies the logging level
 */
void logx_set_file_logging_level(logx_t *logger, logx_level_t level) {
    if (!logger) {
        return;
    }

    if (is_valid_logx_level(level) == 0)
        ;
    else {
        fprintf(stderr, "[LogX] %s: Error: Unknown logx_level - %d", __func__, (int)level);
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.file_level = level;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Enables console logging for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the console logging is to be
 * enabled
 */
void logx_set_console_logging(logx_t *logger) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_console_logging = 1;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Disables console logging for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the console logging is to be
 * disabled
 */
void logx_disable_console_logging(logx_t *logger) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_console_logging = 0;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Enables file logging for a logx logger instance
 *
 * @param logger Pointer to the logx logger instance for which the file logging is to be
 * enabled
 */
void logx_enable_file_logging(logx_t *logger) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    if (!logger->cfg.file_path) {
        /* cannot enable without path */
        /* must not use Logger to log the below message since we're already inside the logger */
        fprintf(stderr, "ERROR: cannot enable file logging without "
                        "valid file path\n");
        logger->cfg.enable_file_logging = 0;
    } else {
        logger->cfg.enable_file_logging = 1;
    }
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Disables file logging for a logx logger instance
 *
 * @param logger Pointer to the logx logger instance for which the file logging is to be
 * disabled
 */
void logx_disable_file_logging(logx_t *logger) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_file_logging = 0;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Enables colored logs for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the colored logs is to be
 * enabled
 */
void logx_enable_colored_logging(logx_t *logger) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_colored_logs = 1;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Disables colored logs for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the colored logs is to be
 * disabled
 */
void logx_disable_colored_logging(logx_t *logger) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_colored_logs = 0;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Enables TTY detection for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the TTY detection is to be
 * enabled
 */
void logx_enable_tty_detection(logx_t *logger) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.use_tty_detection = 1;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Disables TTY detection for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the TTY detection is to be
 * disabled
 */
void logx_disable_tty_detection(logx_t *logger) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.use_tty_detection = 0;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Sets the log rotation type for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the log rotation type is to be
 * chnaged
 * @param[in] enable value to be set
 */
void logx_set_log_rotate_type(logx_t *logger, logx_rotate_type_t type) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.type = type;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Rotates the log file handled by logx logger instance
 *
 * @param[in] logger Pointer to the logger instance whose handled file needs to be rotated
 * @return int 0 on Success, -1 on Failure
 */
int logx_rotate_now(logx_t *logger) {
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

        r = rotate_files(logger->cfg.file_path, logger->cfg.rotate.max_backups);

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

/**
 * @brief Sets the log file size (in MB) for a logx logger instance
 *
 * @param logger  Pointer to the logx logger instance for which the log file size is to be set
 * @param size_mb Size in MB
 */
void logx_set_log_file_size_mb(logx_t *logger, size_t size_mb) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.size_mb = size_mb;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Sets the number of log file backups to keep for a logx logger instance
 *
 * @param logger Pointer to the logx logger instance for which the number of log file backups is to
 * be set
 * @param max_backups Number of backups to keep
 */
void logx_set_num_of_logfile_backups(logx_t *logger, int max_backups) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.max_backups = max_backups;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Sets the log rotation interval in days for a logx logger instance
 *
 * @param logger Pointer to the logx logger instance for which the log rotation interval is to be
 * set
 * @param interval_days Interval in days
 */
void logx_set_rotation_interval_days(logx_t *logger, int interval_days) {
    if (!logger) {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.rotate.interval_days = interval_days;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Enables print config for a logx logger instance
 *
 * @param logger Pointer to the logx logger instance for which print config is to be enabled
 */
void logx_enable_print_config(logx_t *logger) {
    if (!logger) {
        return;
    }
    pthread_mutex_lock(&logger->lock);
    logger->cfg.print_config = 1;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Disables print config for a logx logger instance
 *
 * @param logger Pointer to the logx logger instance for which print config is to be disabled
 */
void logx_disable_print_config(logx_t *logger) {
    if (!logger) {
        return;
    }
    pthread_mutex_lock(&logger->lock);
    logger->cfg.print_config = 0;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Converts a 64-bit integer to a binary string with grouped nibbles (4 bits) using
 * thread-local
 *
 * @param value 64-bit integer value to convert
 * @return const char* Pointer to the thread-local binary string representation
 */
const char *logx_bin_str64_grouped_tls(uint64_t value) {
    // 8 rotating buffers, each large enough for grouped 64-bit binary + NUL
    enum {
        BUF_COUNT = 8,
        BUF_SIZE  = 128
    };

    static _Thread_local char bufs[BUF_COUNT][BUF_SIZE];
    static _Thread_local int  idx = 0;

    char *out = bufs[idx];
    idx       = (idx + 1) % BUF_COUNT;

    char tmp[64 + 16];
    int  pos = 0;

    // Build full 64-bit binary with nibble spaces
    for (int i = 63; i >= 0; --i) {
        tmp[pos++] = (value & (1ULL << i)) ? '1' : '0';
        if (i % 4 == 0 && i != 0)
            tmp[pos++] = ' ';
    }
    tmp[pos] = '\0';

    // Find first '1'
    int first_one = -1;
    for (int i = 0; tmp[i]; ++i) {
        if (tmp[i] == '1') {
            first_one = i;
            break;
        }
    }

    // All zero → return one nibble
    if (first_one == -1) {
        strcpy(out, "0000");
        return out;
    }

    // Snap to nibble boundary
    while (first_one > 0 && tmp[first_one - 1] != ' ') first_one--;

    if (tmp[first_one] == ' ')
        first_one++;

    strcpy(out, tmp + first_one);
    return out;
}