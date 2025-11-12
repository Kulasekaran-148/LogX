#include "../include/logx/logx.h"

/**
 * @brief Changes the console log level
 *
 * @param[in] logger Pointer to the logx logger instance whose console logging level is to be
 * changed
 * @param[in] level specifies the logging level
 */
void logx_set_console_level(logx_t *logger, logx_level_t level)
{
    if (!logger)
    {
        return;
    }
    
    if(is_valid_logx_level(level) == 0);
    else
    {
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
void logx_set_file_level(logx_t *logger, logx_level_t level)
{
    if (!logger)
    {
        return;
    }
    
    if(is_valid_logx_level(level) == 0);
    else
    {
      fprintf(stderr, "[LogX] %s: Error: Unknown logx_level - %d", __func__, (int)level);
      return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.file_level = level;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Enables/Disables console logging for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the console logging is to be
 * enabled/disabled
 * @param[in] enable value to be set - 1/0
 */
void logx_set_console_logging(logx_t *logger, int enable)
{
    if (!logger)
    {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enable_console_logging = enable ? 1 : 0;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Enables/Disables file logging for a logx logger instance
 *
 * @param logger Pointer to the logx logger instance for which the file logging is to be
 * enabled/disabled
 * @param enable value to be set - 1/0
 */
void logx_set_file_logging(logx_t *logger, int enable)
{
    if (!logger)
    {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    if (enable && !logger->cfg.file_path)
    {
        /* cannot enable without path */
        /* must not use Logger to log the below message since we're already inside the logger */
        fprintf(stderr, "ERROR: cannot enable file logging without "
                        "valid file path\n");
        logger->cfg.enable_file_logging = 0;
    }
    else
    {
        logger->cfg.enable_file_logging = enable ? 1 : 0;
    }
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Enables/Disables colored logs for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the colored logs is to be
 * enabled/disabled
 * @param[in] enable value to be set - 1/0
 */
void logx_set_colored_logs(logx_t *logger, int enable)
{
    if (!logger)
    {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.enabled_colored_logs = enable ? 1 : 0;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Enables/Disables TTY detection for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the TTY detection is to be
 * enabled/disabled
 * @param[in] enable value to be set - 1/0
 */
void logx_set_tty_detection(logx_t *logger, int enable)
{
    if (!logger)
    {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.use_tty_detection = enable ? 1 : 0;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Sets the log rotation type for a logx logger instance
 *
 * @param[in] logger Pointer to the logx logger instance for which the log rotation type is to be
 * chnaged
 * @param[in] enable value to be set
 */
void logx_set_log_rotate_type(logx_t *logger, logx_rotate_type_t type)
{
    if (!logger)
    {
        return;
    }

    pthread_mutex_lock(&logger->lock);
    logger->cfg.use_tty_detection = enable ? 1 : 0;
    pthread_mutex_unlock(&logger->lock);
}

/**
 * @brief Rotates the log file handled by logx logger instance
 *
 * @param[in] logger Pointer to the logger instance whose handled file needs to be rotated
 * @return int 0 on Success, -1 on Failure
 */
int logx_rotate_now(logx_t *logger)
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
            file_lock_ex(logger->fd);
        }

        if (logger->fp)
        {
            fflush(logger->fp);
        }

        r = rotate_files(logger->cfg.file_path, logger->cfg.rotate.max_backups);

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
            file_lock_un(logger->fd);
        }
    }
    pthread_mutex_unlock(&logger->lock);
    return r;
}