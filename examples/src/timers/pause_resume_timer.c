/**
 * @file pause_resume_timer.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This example demonstrates the use of logx timers with pause/resume
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
    cfg.file_path              = "./pause_resumetimer.log";
    cfg.enabled_colored_logs   = 1;
    cfg.use_tty_detection      = 1;
    cfg.console_level          = LOGX_LEVEL_TRACE;
    cfg.file_level             = LOGX_LEVEL_TRACE;
    cfg.rotate.type            = LOGX_ROTATE_BY_SIZE;
    cfg.rotate.max_bytes       = 1024 * 1024 * 1; /* 1 MB */
    cfg.rotate.max_backups     = 3;
    cfg.print_config           = 1;

    logger = logx_create(&cfg);

    if (!logger)
    {
        fprintf(stderr, "[LogX] Failed to create logx instance\n");
        return -1;
    }

    LOGX_BANNER(logger, "Pause - Resume - Timer example");
    
    LOGX_DEBUG(logger, "Starting timer: task\n");
    logx_timer_start(logger, "task");

    // Phase 1
    sleep(1);

    LOGX_DEBUG(logger, "Pausing timer: task\n");
    logx_timer_pause(logger, "task");

    // Simulate idle / wait
    sleep(2);

    LOGX_DEBUG(logger, "Resuming timer: task\n");
    logx_timer_resume(logger, "task");

    // Phase 2
    sleep(1);

    LOGX_DEBUG(logger, "Stopping timer: task\n");
    logx_timer_stop(logger, "task");

    logx_destroy(logger);
    return 0;
}
