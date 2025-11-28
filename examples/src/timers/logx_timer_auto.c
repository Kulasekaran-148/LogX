/**
 * @file logx_timer_auto.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This example demonstrates the usage of the macro LOGX_TIMER_AUTO
 * @version 0.1
 * @date 2025-11-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <logx/logx.h>

void auto_timer(logx_t *logger, int wait_time)
{
    /* It automatically starts the timer as the function begins
    and calls the logx_timer_stop when the function returns */
    LOGX_TIME_AUTO(logger, "auto timer")
    {
        switch(wait_time)
        {
            case 1:
                sleep(1);
                return;

            case 2:
                sleep(2);
                return;

            case 3:
                sleep(3);
                return;

            default:
                sleep(1);
                return;
        }
    }
}

int main(void)
{
    logx_t *logger;
    logx_cfg_t cfg = {0};

    /* Logger Configuration */
    cfg.name                   = "LogX";
    cfg.enable_console_logging = 1;
    cfg.enable_file_logging    = 1;
    cfg.file_path              = "./logx_timer_auto.log";
    cfg.enabled_colored_logs   = 1;
    cfg.use_tty_detection      = 1;
    cfg.console_level          = LOGX_LEVEL_TRACE;
    cfg.file_level             = LOGX_LEVEL_TRACE;
    cfg.rotate.type            = LOGX_ROTATE_BY_SIZE;
    cfg.rotate.size_mb       = 1024 * 1024 * 1; /* 1 MB */
    cfg.rotate.max_backups     = 3;
    cfg.print_config           = 1;

    logger = logx_create(&cfg);

    if (!logger)
    {
        fprintf(stderr, "[LogX] Failed to create logx instance\n");
        return -1;
    }

    LOGX_BANNER(logger, "LOGX_TIMER_AUTO example");
    auto_timer(logger, 2);

    logx_destroy(logger);
    return 0;
}
