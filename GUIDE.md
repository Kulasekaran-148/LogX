# LogX User Guide

This guide contains easy-to-understand examples explaining how to use each and every features of LogX

## Table of Contents

1. [LogX Integration](#logx-integration)
    - [Understanding LogX configuration](#understanding-logx-configuration)
    - [Default configuration](#logx-integration---default-configuration)
    - [Passing configuration](#logx-integration---passing-configuration)
    - [Parsing configuration from default file](#logx-integration---parsing-configuration-from-custom-file)
    - [Parsing configuration from custom file](#logx-integration---parsing-configuration-from-default-file)

2. [LogX - Log Levels](#logx---log-levels)
    - [Trace](#logx---trace)
    - [Debug](#logx---debug)
    - [Info](#logx---info)
    - [Warn](#logx---warn)
    - [Error](#logx---error)
    - [Fatal](#logx---fatal)
    - [Banner](#logx---banner)

3. [LogX - Log Rotation](#logx---log-rotation)
    - [Rotation based on Size](#logx---rotation-based-on-size)
    - [Rotation based on Date](#logx---rotation-based-on-date)
    - [No Rotation](#logx---no-rotation)

4. [LogX - Timers](#logx---timers)
    - [Simple Timer](#simple-timer)
    - [Pause & Resume](#pause-&-resume)
    - [Auto scope timer](#auto-scope-timer)


## Logx Integration

### Understanding LogX Configuration

- LogX uses the following configuration structure the let's you fine-tune the logging needs for your application.

```c
typedef struct
{
    const char       *name;                   /* Name for the logger instance - just for identification purpose */
    const char       *file_path;              /* Logfile path */
    logx_level_t      console_level;          /* level threshold for console logging */
    logx_level_t      file_level;             /* level threshold for file logging */
    int               enable_console_logging; /* Enable / Disable console logging */
    int               enable_file_logging;    /* Enable / Disable file logging */
    int               enabled_colored_logs;   /* Enable / Disable ANSI colored logs (Only visible in console logs) */
    int               use_tty_detection;      /* Auto Enable / Disable colored logs based on TTY detection */
    logx_rotate_cfg_t rotate;                 /* Control Log Rotation */
    const char       *banner_pattern;         /* Configure Banner Pattern (Used in LOGX_BANNER() */
    int               print_config;           /* Enable / Disable verbose print of LogX configuration that's chosen */
} logx_cfg_t;
```
---

### LogX Integration - Default Configuration

- When `logx_create()` is called with NULL, LogX will try to look for configuration information in the following order until success:
    1. If `LOGX_CFG_FILE_PATH` macro is declared, tries to fetch configuration settings from that file.
    2. Looks for `./logx_cfg.yml`, `./logx_cfg.yaml` or `./logx_cfg.json` (in the same order)
    3. Sets Default configuration.

- The default configurations are defined in `./include/logx/logx_defaults.h`. Click [here](https://github.com/Kulasekaran-148/LogX/blob/main/include/logx/logx_defaults.h) to view the default MACROS.
- In case, you want to override these defaults, you can simply re-declare the macro that you need to change in your project source (without having to modify the library source). Please keep in mind that you need to declare the macro before the `#include <logx/logx.h>` statement in your code.

```C
#include <stdio.h>
#include <logx/logx.h>

int main() {
    // Initialize logger
    logx_t *logger = logx_create(NULL); // passing NULL to use default configuration
    if(!logger)
    {
      fprintf(stderr, "Failed to create logx logger instance\n");
      return -1;
    }

    LOGX_DEBUG(logger, "This is a debug message");
    
    // Destroy logger to clean up resources
    logx_destroy(logger);
    return 0;
}
```

### Output:

![basic_example_default_settings](./assets/images/basic_example_default_settings.png)

---

### LogX Integration - Passing configuration

```c
#include <logx/logx.h>
#include <stdio.h>

int main()
{
    logx_t    *logger;
    logx_cfg_t cfg = {0};

    /* Logger Configuration */
    cfg.name                   = "LogX";
    cfg.enable_console_logging = 1;
    cfg.enable_file_logging    = 1;
    cfg.file_path              = "./basic_example_passing_configuration.log";
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
        fprintf(stderr, "Failed to create logger instance\n");
        return -1;
    }
    LOGX_BANNER(logger, "Welcome to LogX Logging");
    logx_destroy(logger);
    return 0;
}
```

### Output:

![basic example passing configuration](./assets/images/basic_example_passing_configuration.png)

---

### LogX Integration - Parsing configuration from default file

- When `logx_create()` is called with NULL, LogX will try to look for configuration information in the following order until success:
    1. If `LOGX_CFG_FILE_PATH` macro is declared, tries to fetch information from that file.
    2. Looks for `./logx_cfg.yml`, `./logx_cfg.yaml` or `./logx_cfg.json` (in the same order)
    3. Sets Default configuration.

```c
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
```

### Output:

![basic example parsing configuration from default file](./assets/images/basic_example_parsing_configuration_from_default_file.png)

---

### LogX Integration - Parsing configuration from custom file

- When `logx_create()` is called with NULL, LogX will try to look for configuration information in the following order until success:
    1. If `LOGX_CFG_FILE_PATH` macro is declared, tries to fetch information from that file.
    2. Looks for `./logx_cfg.yml`, `./logx_cfg.yaml` or `./logx_cfg.json` (in the same order)
    3. Sets Default configuration.

```c
#include <logx/logx.h>
#include <stdio.h>

#define LOGX_CFG_FILE_PATH "./some_file_path" // file must be a valid YAML/JSON

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
```

---

## LogX - Log Levels

- LogX provides the following log levels
    - `TRACE` - Useful when debugging the flow of the program
    - `DEBUG` - Useful for logging information that is for developers-only
    - `INFO` - Useful for logging information that is necessary for non-developers, perhaps during testing
    - `WARN` - Useful for logging failures that do not affect functionality, but still need to be aware of
    - `ERROR` - Useful for logging errors
    - `FATAL` - Useful for logging critical failures in the code, due to which the program will need to terminate
    - `BANNER` - Useful for logging specific milestones during runtime. (E.g. "Starting Firmware update...")

```c
/* Log levels */
typedef enum
{
    LOGX_LEVEL_TRACE = 0,
    LOGX_LEVEL_DEBUG,
    LOGX_LEVEL_BANNER,
    LOGX_LEVEL_INFO,
    LOGX_LEVEL_WARN,
    LOGX_LEVEL_ERROR,
    LOGX_LEVEL_FATAL,
    LOGX_LEVEL_OFF
} logx_level_t;
```

### LogX - Trace

```c
LOGX_TRACE(logger, "This is a trace message and BLUE in color");
```

![trace_msg](./assets/images/trace_msg.png)

### LogX - Debug

```c
LOGX_DEBUG(logger, "This is a debug message and WHITE in color");
```

![debug_msg](./assets/images/debug_msg.png)

### LogX - Info

```c
LOGX_INFO(logger, "This is a info message and GREEN in color");
```

![info_msg](./assets/images/info_msg.png)

### LogX - Warn

```c
LOGX_WARN(logger, "This is a warn message and YELLOW in color");
```

![warn_msg](./assets/images/warn_msg.png)

### LogX - Error

```c
LOGX_ERROR(logger, "This is a error message and RED in color");
```

![error_msg](./assets/images/error_msg.png)

### LogX - Fatal

```c
LOGX_FATAL(logger, "This is a fatal message and MAGENTA in color");
```

![fatal_msg](./assets/images/fatal_msg.png)

### LogX - Banner

```c
LOGX_BANNER(logger, "This is a banner message and CYAN in color");
```

- If you notice, there is one thing specially crafted about `LOGX_BANNER`, can you guess what it is 👀 ?

![banner_msg](./assets/images/banner_msg.png)

- Banner messages are **auto-centered** inside the banner, making it look aesthetically more beautiful ✨

---

## LogX - Log Rotation

- LogX comes in handy with log rotation feature, so that you don't need to worry about your log file filling up the space
- LogX let's you control the rotation of log files based on `SIZE` or `DATE`
- LogX also let's you control the number of backups to be maintained.

```c
typedef struct
{
    logx_rotate_type_t type;            /* type of rotation */
    size_t             size_mb;       /* used when tyep == LOGX_ROTATE_BY_SIZE */
    int                max_backups;     /* number of backup files to keep (0 = no backups) */
    int                daily_interval;  /* days between rotations when type = LOGX_ROTATE_BY_DATE (1 = daily) */
} logx_rotate_cfg_t;
```

**Log Rotation Types:**

```c
typedef enum
{
    LOGX_ROTATE_NONE = 0,
    LOGX_ROTATE_BY_SIZE,
    LOGX_ROTATE_BY_DATE
} logx_rotate_type_t;
```

---

### LogX - Rotation based on size

During configuration, you can use `cfg.rotate.type` to set it based on size and specify the size in mb using `cfg.rotate.size_mb`

```c
cfg.rotate.type = LOGX_ROTATE_BY_SIZE;
cfg.rotate.size_mb = 10;    // Log rotation happens if file size exceeds 10mb
cfg.rotate.max_backups = 5; // Number of backups to maintain
```

- For example, consider your log file is `example.log`.
- When logfile exceeds the size, it gets renamed as `example.log` --> `example.log.1`
- Again when size exceeds, `example.log.1` --> `example.log.2`
- This continues until there are `cfg.rotate.max_backups` number of log files are present. Now, when size exceeds again, the oldest of the log file gets deleted.

---

### LogX - Rotation based on date

During `logx_create`, LogX will save the current date in its configuration. Using this information, the LOGX_ROTATE_BY_DATE works.

- You can use `cfg.rotate.daily_interval` to specify how many number of days once the log file needs to be rotated.

```c
cfg.rotate.type = LOGX_ROTATE_BY_DATE;
cfg.rotate.daily_interval = 1 // Rotate the log files daily
cfg.rotate.max_backups = 5    // Number of backups to maintain
```

- With rotation by date, number of backups mean, the number of days after which the oldest log files will start getting deleted.

---

### LogX - No rotation

- If you don't want LogX to take care of rotation at all, just specify `cfg.rotate.type = LOGX_ROTATE_NONE`
- When the above is set, none of the rotation configuration matters

---

## LogX - Timers

### Simple Timer

```c
LOGX_DEBUG(logger, "Starting timer...");
logx_timer_start(logger, "simple");

// Simulated work
sleep(1);

LOGX_DEBUG(logger, "Stopping timer...");
logx_timer_stop(logger, "simple");
```

---

### Pause & Resume

```c
// start the timer
logx_timer_start(logger, "task");

// do some work for 1s
sleep(1);

logx_timer_pause(logger, "task");

// Simulate idle / wait
sleep(2);

LOGX_DEBUG(logger, "Resuming timer: task\n");
logx_timer_resume(logger, "task");

// Phase 2
sleep(1);

LOGX_DEBUG(logger, "Stopping timer: task\n");
logx_timer_stop(logger, "task");
```
---

### Auto Scope timer

---