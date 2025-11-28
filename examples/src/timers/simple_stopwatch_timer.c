/**
 * @file simple_stopwatch_timer.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This example demonstrates how to use logx timer with start/stop
 * @version 0.1
 * @date 2025-11-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <logx/logx.h>

int main(void)
{
    logx_t *logger;
    
    logx_cfg_t cfg = {0};

    /* Logger Configuration */
    cfg.name                   = "LogX";
    cfg.enable_console_logging = 1;
    cfg.enable_file_logging    = 1;
    cfg.file_path              = "./simple_timer.log";
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
        fprintf(stderr, "[LogX] Failed to create logx logger instance\n");
        return -1;
    }

    LOGX_BANNER(logger, "Simple Timer example");

    // start the timer
    logx_timer_start(logger, "timer name");

    // do some work for 1s
    sleep(1);

    // stop the timer
    logx_timer_stop(logger, "timer name");

    return 0;
}
