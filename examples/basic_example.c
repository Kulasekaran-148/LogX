#include "../include/logx/logx.h"

#include <stdio.h>

int main(void) {
  logx_t    *logger;
  logx_cfg_t cfg = {0};

  /* Logger Configuration */
  cfg.name                   = "LogX Basic Example";
  cfg.enable_console_logging = 1;
  cfg.enable_file_logging    = 1;
  cfg.file_path              = "./LogX_Basic_Example.log";
  cfg.enabled_colored_logs   = 1;
  cfg.use_tty_detection      = 1;
  cfg.console_level          = LOG_LEVEL_DEBUG;
  cfg.file_level             = LOG_LEVEL_DEBUG;
  cfg.rotate.type            = LOG_ROTATE_BY_SIZE;
  cfg.rotate.max_bytes       = 1024 * 1024 * 1; /* 1 MB */
  cfg.rotate.max_backups     = 3;

  logger = logx_create(&cfg);
  if (!logger) {
    fprintf(stderr, "Failed to create logger");
    return -1;
  }

  LOGX_BANNER(logger, "LogX Basic Example");
  LOGX_DEBUG(logger, "This is a debug log");
  LOGX_INFO(logger, "This is an info log");
  LOGX_WARN(logger, "This is a warning log");
  LOGX_ERROR(logger, "This is a error log");
  LOGX_FATAL(logger, "This is a fatal log");

  logx_destroy(logger);

  return 0;
}