#define _GNU_SOURCE
#include "dispatcher.h"

#include "session.h"
#include "logx.h"

#include <string.h>
#include <stdio.h>

/* =========================
 * Helpers
 * ========================= */

static void set_ok(dispatch_result_t *res)
{
    res->ok = 1;
    res->error_code[0] = '\0';
    res->error_msg[0]  = '\0';
}

static void set_error(dispatch_result_t *res,
                      const char *code,
                      const char *msg)
{
    res->ok = 0;
    strncpy(res->error_code, code, sizeof(res->error_code) - 1);
    strncpy(res->error_msg, msg, sizeof(res->error_msg) - 1);
}

/* =========================
 * Command handlers
 * ========================= */

static void handle_create(const ipc_request_t *req,
                          dispatch_result_t *res)
{
    int rc = session_create(req->session,
                            req->pid,
                            (const logx_cfg_t *)req->config);

    if (rc == 0)
        set_ok(res);
    else if (rc == -2)
        set_error(res, "LOGGER_EXISTS", "Logger already exists");
    else
        set_error(res, "CREATE_FAILED", "Failed to create logger");
}

static void handle_destroy(const ipc_request_t *req,
                           dispatch_result_t *res)
{
    if (session_destroy(req->session) == 0)
        set_ok(res);
    else
        set_error(res, "LOGGER_NOT_FOUND", "Logger not found");
}

static void handle_log(const ipc_request_t *req,
                       dispatch_result_t *res)
{
    logx_t *logger = session_get(req->session);
    if (!logger) {
        set_error(res, "LOGGER_NOT_FOUND", "Logger not created");
        return;
    }

    logx_level_t level = LOGX_LEVEL_INFO;

    if (strcmp(req->level, "TRACE") == 0) level = LOGX_LEVEL_TRACE;
    else if (strcmp(req->level, "DEBUG") == 0) level = LOGX_LEVEL_DEBUG;
    else if (strcmp(req->level, "INFO")  == 0) level = LOGX_LEVEL_INFO;
    else if (strcmp(req->level, "WARN")  == 0) level = LOGX_LEVEL_WARN;
    else if (strcmp(req->level, "ERROR") == 0) level = LOGX_LEVEL_ERROR;
    else if (strcmp(req->level, "FATAL") == 0) level = LOGX_LEVEL_FATAL;
    else {
        set_error(res, "INVALID_LEVEL", "Invalid log level");
        return;
    }

    logx_log(logger,
             level,
             req->file[0] ? req->file : "shell",
             req->func[0] ? req->func : "shell",
             req->line,
             "%s",
             req->message);

    set_ok(res);
}

static void handle_cfg_set(const ipc_request_t *req,
                           dispatch_result_t *res)
{
    logx_t *logger = session_get(req->session);
    if (!logger) {
        set_error(res, "LOGGER_NOT_FOUND", "Logger not found");
        return;
    }

    if (strcmp(req->key, "console.enabled") == 0) {
        if (strcmp(req->value, "true") == 0)
            logx_enable_console_logging(logger);
        else
            logx_disable_console_logging(logger);
    }
    else if (strcmp(req->key, "file.enabled") == 0) {
        if (strcmp(req->value, "true") == 0)
            logx_enable_file_logging(logger);
        else
            logx_disable_file_logging(logger);
    }
    else if (strcmp(req->key, "console.level") == 0) {
        logx_level_t lvl = logx_level_from_string(req->value);
        if (!is_valid_logx_level(lvl)) {
            set_error(res, "INVALID_LEVEL", "Invalid console level");
            return;
        }
        logx_set_console_logging_level(logger, lvl);
    }
    else {
        set_error(res, "INVALID_CONFIG", "Unknown config key");
        return;
    }

    set_ok(res);
}

static void handle_timer(const ipc_request_t *req,
                         dispatch_result_t *res)
{
    logx_t *logger = session_get(req->session);
    if (!logger) {
        set_error(res, "LOGGER_NOT_FOUND", "Logger not found");
        return;
    }

    if (strcmp(req->cmd, "timer_start") == 0)
        logx_timer_start(logger, req->timer_name);
    else if (strcmp(req->cmd, "timer_stop") == 0)
        logx_timer_stop(logger, req->timer_name);
    else if (strcmp(req->cmd, "timer_pause") == 0)
        logx_timer_pause(logger, req->timer_name);
    else if (strcmp(req->cmd, "timer_resume") == 0)
        logx_timer_resume(logger, req->timer_name);
    else {
        set_error(res, "INVALID_CMD", "Unknown timer command");
        return;
    }

    set_ok(res);
}

/* =========================
 * Dispatcher entry
 * ========================= */

void dispatch_request(const ipc_request_t *req,
                      dispatch_result_t *res)
{
    if (!req || !res) {
        return;
    }

    if (req->version != 1) {
        set_error(res, "INVALID_VERSION", "Unsupported IPC version");
        return;
    }

    if (strcmp(req->cmd, "create") == 0)
        handle_create(req, res);
    else if (strcmp(req->cmd, "destroy") == 0)
        handle_destroy(req, res);
    else if (strcmp(req->cmd, "log") == 0)
        handle_log(req, res);
    else if (strcmp(req->cmd, "cfg_set") == 0)
        handle_cfg_set(req, res);
    else if (strncmp(req->cmd, "timer_", 6) == 0)
        handle_timer(req, res);
    else {
        set_error(res, "INVALID_CMD", "Unknown command");
    }
}
