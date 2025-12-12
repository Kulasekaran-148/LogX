#define _GNU_SOURCE
#include "../../../include/logx/logx.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define MAXLINE 4096

static int file_exists(const char *path) {
    struct stat st;
    return (path && stat(path, &st) == 0);
}

static char *read_stdin_all(void) {
    size_t cap = 4096, len = 0;
    char  *buf = malloc(cap);
    if (!buf)
        return NULL;
    ssize_t r;
    while ((r = read(STDIN_FILENO, buf + len, cap - len)) > 0) {
        len += (size_t)r;
        if (len + 1 >= cap) {
            cap *= 2;
            char *n = realloc(buf, cap);
            if (!n) {
                free(buf);
                return NULL;
            }
            buf = n;
        }
    }
    if (r < 0) {
        free(buf);
        return NULL;
    }
    buf[len] = '\0';
    return buf;
}

static logx_level_t parse_level_to_enum(const char *level_str) {
    if (strcasecmp(level_str, "TRACE") == 0)
        return LOGX_LEVEL_TRACE;
    if (strcasecmp(level_str, "DEBUG") == 0)
        return LOGX_LEVEL_DEBUG;
    if (strcasecmp(level_str, "INFO") == 0)
        return LOGX_LEVEL_INFO;
    if (strcasecmp(level_str, "WARN") == 0)
        return LOGX_LEVEL_WARN;
    if (strcasecmp(level_str, "ERROR") == 0)
        return LOGX_LEVEL_ERROR;
    if (strcasecmp(level_str, "FATAL") == 0)
        return LOGX_LEVEL_FATAL;
    if (strcasecmp(level_str, "BANNER") == 0)
        return LOGX_LEVEL_BANNER;
    if (strcasecmp(level_str, "OFF") == 0)
        return LOGX_LEVEL_OFF;
    return LOGX_LEVEL_INFO;
}

static void apply_env_overrides(logx_cfg_t *cfg) {
    const char *v;
    if (!cfg)
        return;

    if ((v = getenv("LOGX_NAME")))
        cfg->name = strdup(v);
    if ((v = getenv("LOGX_FILE_PATH")))
        cfg->file_path = strdup(v);

    if ((v = getenv("LOGX_ENABLE_CONSOLE_LOGGING")))
        cfg->enable_console_logging = atoi(v);
    if ((v = getenv("LOGX_CONSOLE_LEVEL"))) {
        cfg->console_level = parse_level_to_enum(v);
    }
    if ((v = getenv("LOGX_ENABLE_FILE_LOGGING")))
        cfg->enable_file_logging = atoi(v);
    if ((v = getenv("LOGX_FILE_LEVEL"))) {
        cfg->file_level = parse_level_to_enum(v);
    }
    if ((v = getenv("LOGX_ENABLE_COLORED_LOGGING")))
        cfg->enable_colored_logs = atoi(v);
    if ((v = getenv("LOGX_USE_TTY_DETECTION")))
        cfg->use_tty_detection = atoi(v);
    if ((v = getenv("LOGX_PRINT_CONFIG")))
        cfg->print_config = atoi(v);

    /* rotation envs */
    if ((v = getenv("LOGX_ROTATE_TYPE"))) {
        cfg->rotate.type = parse_rotate_type_from_str(v);
    }
    if ((v = getenv("LOGX_ROTATE_SIZE_MB"))) {
        cfg->rotate.size_mb = (size_t)atol(v);
    }
    if ((v = getenv("LOGX_ROTATE_MAX_BACKUPS"))) {
        cfg->rotate.max_backups = atoi(v);
    }
    if ((v = getenv("LOGX_ROTATE_INTERVAL_DAYS"))) {
        cfg->rotate.interval_days = atoi(v);
    }

    if ((v = getenv("LOGX_BANNER_PATTERN"))) {
        cfg->banner_pattern = strdup(v);
    }
}

/* -------------------------
   Simple socket server: accept lines and log via logx instance
   Message protocol for socket: LEVEL|MESSAGE\n
   Control messages:
     CTRL|STOP
     CTRL|STATUS
   ------------------------- */

static int create_unix_socket_server(const char *socket_path) {
    int                srv;
    struct sockaddr_un addr;
    if ((srv = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return -1;

    unlink(socket_path);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(srv);
        return -1;
    }
    if (listen(srv, 5) < 0) {
        close(srv);
        return -1;
    }
    return srv;
}

/* Minimal helper: write a PID file */
static int write_pidfile(const char *pidfile, pid_t pid) {
    FILE *f = fopen(pidfile, "w");
    if (!f)
        return -1;
    fprintf(f, "%d\n", (int)pid);
    fclose(f);
    return 0;
}

/* Remove socket & pid at the end */
static void cleanup_instance(const char *socket_path, const char *pidfile) {
    if (socket_path)
        unlink(socket_path);
    if (pidfile)
        unlink(pidfile);
}

/* Signal handling: request graceful stop */
static volatile sig_atomic_t g_stop_requested = 0;

static void sigint_handler(int s) {
    (void)s;
    g_stop_requested = 1;
}

/* Server loop (simplified) */
static void run_server_loop(int srv, logx_t *logger, const char *pidfile, const char *socket_path) {
    fd_set         set;
    int            maxfd = srv;
    struct timeval tv;
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    while (!g_stop_requested) {
        FD_ZERO(&set);
        FD_SET(srv, &set);
        tv.tv_sec  = 1;
        tv.tv_usec = 0;
        int rv     = select(srv + 1, &set, NULL, NULL, &tv);
        if (rv < 0)
            break;
        if (rv == 0)
            continue; /* timeout */

        if (FD_ISSET(srv, &set)) {
            int client = accept(srv, NULL, NULL);
            if (client >= 0) {
                char    buf[MAXLINE];
                ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
                if (n > 0) {
                    buf[n] = '\0';
                    /* very simple parse: first token is TYPE */
                    if (strncmp(buf, "CTRL|STOP", 9) == 0) {
                        g_stop_requested = 1;
                    } else if (strncmp(buf, "CTRL|STATUS", 11) == 0) {
                        /* In a real app: format status and send back */
                        const char *msg = "OK\n";
                        send(client, msg, strlen(msg), 0);
                    } else {
                        /* Expect LEVEL|MESSAGE */
                        char *p = strchr(buf, '|');
                        if (p) {
                            *p                  = 0;
                            const char *level   = buf;
                            const char *message = p + 1;
                            /* Trim newline */
                            char       *nl      = strchr((char *)message, '\n');
                            if (nl)
                                *nl = '\0';
                            /* Call your library logger function.
                               Example: logx_log(logger, level_enum, "%s", message);
                               Use appropriate mapping from string level to enum. */
                            logx_log_from_string(level, message); /* stub: replace with your call */
                        }
                    }
                }
                close(client);
            }
        }
    }

    /* teardown */
    logx_destroy(logger); /* your library function */
    cleanup_instance(socket_path, pidfile);
}

static int client_send_cmd(const char *socket_path, const char *msg, char *reply,
                           size_t reply_len) {
    int                sock;
    struct sockaddr_un addr;
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return -1;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    ssize_t w = write(sock, msg, strlen(msg));
    (void)w;
    if (reply) {
        ssize_t r = read(sock, reply, reply_len - 1);
        if (r > 0) {
            reply[r] = '\0';
        } else if (r == 0)
            reply[0] = '\0';
        else
            reply[0] = '\0';
    }
    close(sock);
    return 0;
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage:\n");
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        usage(argv[0]);
        return -1;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "create") == 0) {
        logx_t    *logger = NULL;
        logx_cfg_t cfg;

        bool daemonize        = true;
        bool use_default_cfg = false;
        bool use_env_cfg     = false;
        bool use_inline_json_cfg = false;
        bool use_inline_yaml_cfg = false;
        const char *inline_cfg  = NULL;
        const char *file_path   = NULL;
        const char *socket_path = NULL;
        const char *pidfile     = NULL;

        if(argc == 2) {
            use_default_cfg = true;
        }

        else 
        {
            for (int i = 2; i < argc; ++i) {
                if (!strcasecmp(argv[i], "--env") || !strcasecmp(argv[i], "-e")) {
                    use_env_cfg = true;
                } else if (!strcasecmp(argv[i], "--inline-json") && i + 1 < argc) {
                    use_inline_json_cfg = true;
                    inline_cfg = argv[++i];
                } else if (!strcasecmp(argv[i], "--inline-yaml") && i + 1 < argc) {
                    use_inline_yaml_cfg = true;
                    inline_cfg = argv[++i];
                } else if (!strcasecmp(argv[i], "--file" || !strcasecmp(argv[i], "-f")) && i + 1 < argc) {
                    file_path = argv[++i];
                } else {
                    fprintf(stderr, "Unknown arg: %s\n", argv[i]);
                    usage(argv[0]);
                    return -1;
                }
            }
        }

        /* Start with default config */
        if (use_default_cfg) {
            logger = logx_create(NULL);
            if (!logger) {
                fprintf(stderr, "Failed to create logger\n");
                return -1;
            }
        }

        /* Use env overrides for logger configuration */
        else if (use_env_cfg) {
            apply_env_overrides(&cfg);
            logger = logx_create(&cfg);
            if (!logger) {
                fprintf(stderr, "Failed to create logger\n");
                return -1;
            }
        }

        /* Use inline JSON */
        else if (use_inline_json_cfg && inline_cfg) {
            if (logx_parse_json_config_inline(&cfg, inline_cfg) != 0) {
                fprintf(stderr, "Failed to parse inline JSON configuration\n");
                return -1;
            }
            logger = logx_create(&cfg);
            if (!logger) {
                fprintf(stderr, "Failed to create logger\n");
                return -1;
            }
        }

        /* Use inline YAML */
        else if (use_inline_yaml_cfg && inline_cfg) {
            if (logx_parse_yaml_config_inline(&cfg, inline_cfg) != 0) {
                fprintf(stderr, "Failed to parse inline YAML configuration\n");
                return -1;
            }
            logger = logx_create(&cfg);
            if (!logger) {
                fprintf(stderr, "Failed to create logger\n");
                return -1;
            }
        }
        
        /* Use config file */
        else if (file_path) {
            if (file_exists(file_path)) {
                (&cfg, file_path);
            }
            if(logx_parse_config_file(file_path, &cfg) != 0) {
                fprintf(stderr, "Failed to parse configuration file: %s\n", file_path);
                return -1;
            }
            logger = logx_create(&cfg);
            if (!logger) {
                fprintf(stderr, "Failed to create logger\n");
                return -1;
            }
        }

        if (daemonize) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                logx_destroy(logger);
                return -1;
            }
            if (pid > 0) {
                /* parent returns quickly with child pid recorded by wrapper or caller */
                printf("%d\n", (int)pid);
                return 0;
            }
            /* child continues as daemon */
            /* detach from terminal */
            if (setsid() < 0) { /* ignore */
            }
            int srv = create_unix_socket_server(socket_path);
            if (srv < 0) {
                perror("create socket");
                logx_destroy(logger);
                return -1;
            }

            /* write pid file */
            write_pidfile(pidfile, getpid());
            run_server_loop(srv, logger, pidfile, socket_path);
            
            return 0;
        } 
        
        else {
            /* Not daemonized: run server loop in foreground (useful for testing) */
            int srv = create_unix_socket_server(socket_path);
            if (srv < 0) {
                perror("create socket");
                logx_destroy(logger);
                return -1;
            }
            write_pidfile(pidfile, getpid());
            run_server_loop(srv, logger, pidfile, socket_path);
            return 0;
        }
    }

    else  {
        fprintf(stderr, "Unknown command: %s\n", cmd);
        usage(argv[0]);
        return -1;
    }

    return 0;
}
