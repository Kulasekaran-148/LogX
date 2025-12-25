#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include "../../include/logxd/logx_cli.h"
#include "../../include/logx/logx.h"

static int send_ipc(const char *buffer)
{
    int fd;
    struct sockaddr_un addr;
    char buf[256];
    ssize_t n;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, LOGXD_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    write(fd, buffer, strlen(buffer));

    n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }

    close(fd);
    return 0;
}

static int cmd_create(const char *config_path)
{
    pid_t pid = getppid();
    char buffer[512];

    if(config_path)
    {
        snprintf(buffer, sizeof(buffer), "CREATE|%d|%s", pid, config_path);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "CREATE|%d", pid);
    }
    return send_ipc(buffer);
}

static int cmd_destroy(void)
{
    pid_t pid = getppid();
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "DESTROY|%d", pid);

    return send_ipc(buffer);
}

static int cmd_log(const char *level, const char *message)
{
    pid_t pid = getppid();
    char buffer[128 + LOGX_MAX_PAYLOAD_SIZE_BYTES];

    snprintf(buffer, sizeof(buffer), "LOG|%d|%s|%s", pid, level, message);

    return send_ipc(buffer);
}

static int cmd_timer(const char *action, const char *timer_name)
{
    pid_t pid = getppid();
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "TIMER|%d|%s|%s", pid, action, timer_name);

    return send_ipc(buffer);
}

static int cmd_cfg(const char *param, const char *value)
{
    pid_t pid = getppid();
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "CFG|%d|%s|%s", pid, param, value);

    return send_ipc(buffer);
}

static void usage(void)
{
    fprintf(stderr,
        "Usage:\n"
        "  logx <command> [options]\n"
        "\n"
        "Commands:\n"
        "\n"
        "  Create / Destroy:\n"
        "    logx create\n"
        "        Create a logx instance using default configuration\n"
        "\n"
        "    logx create -p, --path <config-file>\n"
        "        Create a logx instance using the specified configuration file\n"
        "\n"
        "    logx destroy\n"
        "        Destroy the logx instance\n"
        "\n"
        "  Logging:\n"
        "    logx trace  <message>    Log a TRACE message\n"
        "    logx debug  <message>    Log a DEBUG message\n"
        "    logx info   <message>    Log an INFO message\n"
        "    logx warn   <message>    Log a WARN message\n"
        "    logx error  <message>    Log an ERROR message\n"
        "    logx fatal  <message>    Log a FATAL message\n"
        "    logx banner <message>    Log a BANNER message\n"
        "\n"
        "  Runtime Configuration:\n"
        "    logx cfg console-logging        <true|false>\n"
        "    logx cfg file-logging           <true|false>\n"
        "    logx cfg console-log-level      <trace|debug|banner|info|warn|error|fatal>\n"
        "    logx cfg file-log-level         <trace|debug|banner|info|warn|error|fatal>\n"
        "    logx cfg colored-logging        <true|false>\n"
        "    logx cfg tty-detection          <true|false>\n"
        "    logx cfg print-config           <true|false>\n"
        "\n"
        "  Log Rotation:\n"
        "    logx rotate-now\n"
        "        Force immediate log rotation\n"
        "\n"
        "    logx cfg rotate-type                <BY_SIZE|BY_DATE|NONE>\n"
        "    logx cfg log-file-size-mb           <size>\n"
        "    logx cfg log-rotation-interval-days <days>\n"
        "    logx cfg max-backups                <number>\n"
        "\n"
        "  Timers:\n"
        "    logx timer start   <timer-name>\n"
        "    logx timer stop    <timer-name>\n"
        "    logx timer pause   <timer-name>\n"
        "    logx timer resume  <timer-name>\n"
        "\n"
        "Examples:\n"
        "  logx create\n"
        "  logx info \"Application started\"\n"
        "  logx cfg console-log-level DEBUG\n"
        "  logx timer start example-timer\n"
        "  logx destroy\n"
        "\n"
    );
}


int main(int argc, char **argv)
{
    /* Debug: print argc and argv */
    fprintf(stderr, "DEBUG: argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        fprintf(stderr, "DEBUG: argv[%d] = \"%s\"\n", i, argv[i]);
    }

    if (argc < 2) {
        usage();
        return -1;
    }

    if(argc == 3)
    {
        if (!strcmp(argv[1], "trace"))
            return cmd_log("TRACE", argv[2]);

        else if (!strcmp(argv[1], "debug"))
            return cmd_log("DEBUG", argv[2]);

        else if (!strcmp(argv[1], "banner"))
            return cmd_log("BANNER", argv[2]);

        else if (!strcmp(argv[1], "info"))
            return cmd_log("INFO", argv[2]);

        else if (!strcmp(argv[1], "warn"))
            return cmd_log("WARN", argv[2]);

        else if (!strcmp(argv[1], "error"))
            return cmd_log("ERROR", argv[2]);

        else if (!strcmp(argv[1], "fatal"))
            return cmd_log("FATAL", argv[2]);
    }

    if (!strcmp(argv[1], "create"))
    {
        if(argc == 2)
            return cmd_create(NULL);
        if(argc == 4 && (!strcmp(argv[2], "-p") || !strcmp(argv[2], "--path")))
            return cmd_create(argv[3]);
    }

    if (!strcmp(argv[1], "destroy"))
        return cmd_destroy();

    if (argc == 4)
    {
        if (!strcmp(argv[1], "timer"))
            return cmd_timer(argv[2], argv[3]);
        if (!strcmp(argv[1], "cfg"))
            return cmd_cfg(argv[2], argv[3]);
    }

    usage();
    return -1;
}
