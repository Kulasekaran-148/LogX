/**
 * @file main.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This example demonstrates how to integrate LogX library in your C project. In this
 * example, logx_create() is passed a NULL to make LogX to fallback to default configuration.
 * @version 0.1
 * @date 2025-11-12
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <logx/logx.h>
#include <stdio.h>

int main()
{
    logx_t *logger = logx_create(NULL);
    if (!logger)
    {
        fprintf(stderr, "Failed to create logger instance\n");
        return -1;
    }
    LOGX_BANNER(logger, "Welcome to LogX Logging");
    LOGX_TRACE(logger, "This is a trace message");
    LOGX_DEBUG(logger, "This is debug message");
    LOGX_INFO(logger, "This is an info message");
    LOGX_WARN(logger, "This is a warning message");
    LOGX_ERROR(logger, "This is a error message");
    LOGX_FATAL(logger, "This is a fatal message");

    logx_destroy(logger);
    return 0;
}
