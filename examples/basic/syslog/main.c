/**
 * @file main.c
 * @brief Demonstrates per-call-site syslog opt-in using LOGX_*_SYSLOG macros.
 *
 * Only the lines using LOGX_*_SYSLOG macros are forwarded to syslog.
 * All lines still appear on the console/file as normal.
 *
 * After running, verify with:
 *   journalctl -t myapp --since "1 minute ago"
 * or:
 *   grep myapp /var/log/syslog
 */

#include <logx.h>
#include <stdio.h>

int main(void)
{
    logx_cfg_t cfg             = {0};
    cfg.name                   = "myapp";
    cfg.enable_console_logging = 1;
    cfg.enable_file_logging    = 0;
    cfg.enable_colored_logs    = 1;
    cfg.use_tty_detection      = 1;
    cfg.console_level          = LOGX_LEVEL_TRACE;
    cfg.print_config           = 1;
    cfg.ts_format              = LOGX_TS_FMT_LOCAL;

    /* Enable syslog sink with LOG_USER facility */
    cfg.enable_syslog   = 1;
    cfg.syslog_facility = LOGX_SYSLOG_FACILITY_USER;
    cfg.syslog_ident    = NULL; /* NULL → uses cfg.name ("myapp") */

    logx_t *logger = NULL;
    if (logx_create(&cfg, &logger) != LOGX_ERR_SUCCESS)
    {
        fprintf(stderr, "Failed to create logger\n");
        return -1;
    }

    /* Normal log — goes to console only, NOT to syslog */
    LOGX_DEBUG(logger, "Routine debug: not forwarded to syslog");
    LOGX_INFO(logger, "Routine info: not forwarded to syslog");

    /* Syslog opt-in — same console/file output PLUS a syslog entry */
    LOGX_WARN_SYSLOG(logger, "Disk usage above 80%% — also in syslog");
    LOGX_ERROR_SYSLOG(logger, "Connection to %s failed (errno=%d) — also in syslog", "db-host",
                      111);

    /* Normal log again — not in syslog */
    LOGX_INFO(logger, "Continuing normally, not in syslog");

    logx_destroy(logger);
    return 0;
}
