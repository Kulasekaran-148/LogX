#!/bin/sh
#
# logxd.sh - Shell helper for logx daemon
#
# Usage:
#   source /usr/lib/logx/logxd.sh in your script
#

###############################################################################
# ENVIRONMENT
###############################################################################

LOGX_SHELL_PID="${LOGX_SHELL_PID:-$$}"

export LOGX_SHELL_PID

###############################################################################
# IPC ENUM DEFINITIONS (MUST MATCH C HEADERS)
###############################################################################

# ---- Commands ----
# Below values must match cmd_type_t in logx_cli.h
LOGX_CMD_CREATE=1
LOGX_CMD_DESTROY=2
LOGX_CMD_LOG=3
LOGX_CMD_CFG=4
LOGX_CMD_ROTATE_NOW=5
LOGX_CMD_TIMER=6

# ---- Log Levels ----
# Below values must match logx_level_t in logx.h
LOGX_LEVEL_TRACE=0
LOGX_LEVEL_DEBUG=1
LOGX_LEVEL_BANNER=2
LOGX_LEVEL_INFO=3
LOGX_LEVEL_WARN=4
LOGX_LEVEL_ERROR=5
LOGX_LEVEL_FATAL=6

# ---- Config Keys ----
# Below values must match cfg_keys_t in logx_cli.h
LOGX_CFG_CONSOLE_LOGGING=1
LOGX_CFG_FILE_LOGGING=2
LOGX_CFG_CONSOLE_LOG_LEVEL=3
LOGX_CFG_FILE_LOG_LEVEL=4
LOGX_CFG_COLORED_LOGGING=5
LOGX_CFG_TTY_DETECTION=6
LOGX_CFG_PRINT_CONFIG=7
LOGX_CFG_ROTATE_TYPE=8
LOGX_CFG_LOG_FILE_SIZE_MB=9
LOGX_CFG_ROTATION_INTERVAL_DAYS=10
LOGX_CFG_MAX_BACKUPS=11

# ---- Rotate Types ----
# Below values must match logx_rotate_type_t in logx.h
LOGX_ROTATE_NONE=0
LOGX_ROTATE_BY_SIZE=1
LOGX_ROTATE_BY_DATE=2

# ---- Timer Actions ----
# Below valus must match timer_action_t in logx_cli.h
LOGX_TIMER_START=1
LOGX_TIMER_STOP=2
LOGX_TIMER_PAUSE=3
LOGX_TIMER_RESUME=4

###############################################################################
# INTERNAL HELPERS
###############################################################################

_logx_cli() {
    # Centralized CLI invocation
    # All arguments must be numeric except message/name/path
    logx-cli "$@"
}

_logx_bool_to_int() {
    case "$1" in
        1|true|TRUE|yes|YES) echo 1 ;;
        0|false|FALSE|no|NO) echo 0 ;;
        *) echo "logx: invalid boolean '$1'" >&2; return 1 ;;
    esac
}

###############################################################################
# LIFECYCLE
###############################################################################

logx_create() {
    # Optional config path
    if [ $# -eq 1 ]; then
        _logx_cli "$LOGX_CMD_CREATE" "$LOGX_SHELL_PID" "$1"
    else
        _logx_cli "$LOGX_CMD_CREATE" "$LOGX_SHELL_PID"
    fi
}

logx_destroy() {
    _logx_cli "$LOGX_CMD_DESTROY" "$LOGX_SHELL_PID"
}

###############################################################################
# LOGGING
###############################################################################

_logx_log() {
    level="$1"
    shift

    _logx_cli \
        "$LOGX_CMD_LOG" \
        "$LOGX_SHELL_PID" \
        \
        "$level" \
        "$0" \
        "$LINENO" \
        "$*"
}

logx_trace()  { _logx_log "$LOGX_LEVEL_TRACE"  "$@"; }
logx_debug()  { _logx_log "$LOGX_LEVEL_DEBUG"  "$@"; }
logx_banner() { _logx_log "$LOGX_LEVEL_BANNER" "$@"; }
logx_info()   { _logx_log "$LOGX_LEVEL_INFO"   "$@"; }
logx_warn()   { _logx_log "$LOGX_LEVEL_WARN"   "$@"; }
logx_error()  { _logx_log "$LOGX_LEVEL_ERROR"  "$@"; }
logx_fatal()  { _logx_log "$LOGX_LEVEL_FATAL"  "$@"; }

###############################################################################
# CONFIGURATION
###############################################################################

logx_cfg_console_logging() {
    val=$(_logx_bool_to_int "$1") || return 1
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_CONSOLE_LOGGING" "$val"
}

logx_cfg_file_logging() {
    val=$(_logx_bool_to_int "$1") || return 1
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_FILE_LOGGING" "$val"
}

logx_cfg_console_log_level() {
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_CONSOLE_LOG_LEVEL" "$1"
}

logx_cfg_file_log_level() {
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_FILE_LOG_LEVEL" "$1"
}

logx_cfg_colored_logging() {
    val=$(_logx_bool_to_int "$1") || return 1
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_COLORED_LOGGING" "$val"
}

logx_cfg_tty_detection() {
    val=$(_logx_bool_to_int "$1") || return 1
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_TTY_DETECTION" "$val"
}

logx_cfg_print_config() {
    val=$(_logx_bool_to_int "$1") || return 1
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_PRINT_CONFIG" "$val"
}

###############################################################################
# ROTATION
###############################################################################

logx_rotate_now() {
    _logx_cli "$LOGX_CMD_ROTATE_NOW" "$LOGX_SHELL_PID"
}

logx_cfg_rotate_type() {
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_ROTATE_TYPE" "$1"
}

logx_cfg_log_file_size_mb() {
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_LOG_FILE_SIZE_MB" "$1"
}

logx_cfg_rotation_interval_days() {
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_ROTATION_INTERVAL_DAYS" "$1"
}

logx_cfg_max_backups() {
    _logx_cli "$LOGX_CMD_CFG" "$LOGX_SHELL_PID" "$LOGX_CFG_MAX_BACKUPS" "$1"
}

###############################################################################
# TIMERS
###############################################################################

logx_timer_start() {
    _logx_cli "$LOGX_CMD_TIMER" "$LOGX_SHELL_PID" "$LOGX_TIMER_START" "$1"
}

logx_timer_stop() {
    _logx_cli "$LOGX_CMD_TIMER" "$LOGX_SHELL_PID" "$LOGX_TIMER_STOP" "$1"
}

logx_timer_pause() {
    _logx_cli "$LOGX_CMD_TIMER" "$LOGX_SHELL_PID" "$LOGX_TIMER_PAUSE" "$1"
}

logx_timer_resume() {
    _logx_cli "$LOGX_CMD_TIMER" "$LOGX_SHELL_PID" "$LOGX_TIMER_RESUME" "$1"
}
