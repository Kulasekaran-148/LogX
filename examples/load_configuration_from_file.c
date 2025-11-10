/**
 * @file fetch_configuration_from_yml.c
 * @author kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This example demonstrates logx fetching is required logger configuration from local
 * .yaml/.json file
 * @version 0.1
 * @date 2025-11-10
 * @copyright Copyright (c) 2025
 *
 */

#include "../include/logx/logx.h"

#include <stdio.h>

int main(void)
{
    logx_t *logger;

    // Pass NULL to logx_create to force it to look for configuration file
    logger = logx_create(NULL);
    if (!logger)
    {
        fprintf(stderr, "Failed to create logger");
        return -1;
    }

    LOGX_BANNER(logger, "LogX - Fetching configuration from yml file example");
    LOGX_TRACE(logger, "This is a trace log");
    LOGX_DEBUG(logger, "This is a debug log");
    LOGX_INFO(logger, "This is an info log");
    LOGX_WARN(logger, "This is a warning log");
    LOGX_ERROR(logger, "This is a error log");
    LOGX_FATAL(logger, "This is a fatal log");

    logx_destroy(logger);

    return 0;
}