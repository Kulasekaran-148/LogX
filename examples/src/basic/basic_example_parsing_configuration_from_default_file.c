/**
 * @file main.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief This example demonstrates how to integrate LogX in your project. In this example, LogX
 * will internally parse the necessary logger configuration from a default file `./logx_cfg.yaml`
 * file located in the same path as the binary.
 *
 * When NULL is passed to logx_create(), LogX will try to load the configuration for the logging
 * system from one of several possible sources, in the following priority order:
 * 1. If LOGX_CFG_FILEPATH macro is defined, load from that exact file path.
 * 2. Otherwise, try the default configuration files in the working directory:
 *      - logx_cfg.yml
 *      - logx_cfg.yaml
 *      - logx_cfg.json
 *
 * The function will automatically detect which file exists and call the
 * appropriate parser (YAML or JSON).
 * 3. Else, fallback to using the defaults defined in `include/logx/logx_defaults.h`
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
    logx_destroy(logger);
    return 0;
}

#define LOGX_DEFAULT_CFG_YML_FILEPATH  "./logx_cfg.yml"
#define LOGX_DEFAULT_CFG_YAML_FILEPATH "./logx_cfg.yaml"
#define LOGX_DEFAULT_CFG_JSON_FILEPATH "./logx_cfg.json"