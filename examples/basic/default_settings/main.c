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

#include <logx.h>
#include <stdio.h>

int main()
{
    // Initialize logger
    logx_t *logger = logx_create(NULL); // passing NULL to use default configuration
    if (!logger)
    {
        fprintf(stderr, "Failed to create logx logger instance\n");
        return -1;
    }

    LOGX_DEBUG(logger, "This is a debug message");

    // Destroy logger to clean up resources
    if (logger)
    {
        logx_destroy(logger);
    }

    return 0;
}
