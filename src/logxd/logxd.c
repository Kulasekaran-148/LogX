#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <logx/logx.h>
#include <logxd/logx_cli.h>
#include <logxd/logxd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#define LOGXD_BACKLOG         16
#define LOGXD_SESSION_BUCKETS 256 /* Power of two */
#define LOGXD_REAPER_INTERVAL 5   /* seconds */

static int                   server_fd = -1;
static volatile sig_atomic_t running   = 1;

static session_bucket_t session_table[LOGXD_SESSION_BUCKETS];
static pthread_mutex_t  session_list_lock = PTHREAD_MUTEX_INITIALIZER;

static void handle_signal(int sig) {
    (void)sig;
    running = 0;
}

static inline uint32_t pid_hash(pid_t pid) {
    /* Simple & fast */
    return ((uint32_t)pid) & (LOGXD_SESSION_BUCKETS - 1);
}

static int pid_is_alive(pid_t pid) {
    if (pid <= 0)
        return 0;

    /* kill(pid, 0) does not send a signal
     * - returns 0 if process exists
     * - returns -1 with ESRCH if it does not
     */
    if (kill(pid, 0) == 0)
        return 1;

    if (errno == EPERM)
        return 1; /* Exists, but no permission */

    return 0; /* ESRCH -> dead */
}

static void redirect_stdio_to_null(void) {
    int fd = open("/dev/null", O_RDWR);

    if (fd < 0)
        return;

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    if (fd > STDERR_FILENO)
        close(fd);
}

static int daemonize(void) {
    pid_t pid;

    /* First fork:
     *  - Parent exits immediately
     *  - Child continues in background
     *  - Ensures the process is NOT a process group leader
     *    (required for setsid() to succeed)
     */
    pid = fork();
    if (pid < 0)
        return -1;
    if (pid > 0)
        exit(EXIT_SUCCESS); /* Parent exits */

    /* Create a new session:
     *  - Detach from controlling terminal
     *  - Become session leader and process group leader
     *  - Prevent receiving terminal-generated signals (SIGHUP, SIGINT)
     */
    if (setsid() < 0)
        return -1;

    /* Second fork:
     *  - Ensures the daemon is NOT a session leader
     *  - Prevents the daemon from ever acquiring a controlling terminal
     */
    pid = fork();
    if (pid < 0)
        return -1;
    if (pid > 0)
        exit(EXIT_SUCCESS); /* Parent exits */

    /* Reset file mode creation mask:
     *  - Ensures predictable file permissions
     *  - Daemon explicitly controls file access modes
     */
    umask(0);

    /* Change working directory to root:
     *  - Prevents the daemon from blocking filesystem unmounts
     *  - Avoids holding any directory in use
     */
    chdir("/");

    /* Close standard file descriptors:
     *  - Daemon should not read from stdin
     *  - Daemon should not write to stdout/stderr
     *  - Prevents accidental terminal I/O
     *
     * NOTE:
     *  A more robust approach is to redirect these
     *  to /dev/null using dup2().
     */
    redirect_stdio_to_null();

    return 0; /* Daemonization successful */
}

static int open_socket(void) {
    struct sockaddr_un addr;

    unlink(LOGXD_SOCK_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, LOGXD_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    /* Allow non-root clients if needed */
    chmod(LOGXD_SOCK_PATH, 0666);

    if (listen(server_fd, LOGXD_BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return 0;
}

static void process_log_command(logx_t *logger, cmd_t *cmd) {
    logx_log(logger, cmd->u.log.level, cmd->u.log.file_name, NULL, cmd->u.log.line_num, "%s",
             cmd->u.log.message);
}

static void process_cfg_command(logx_t *logger, cmd_t *cmd) {
    switch (cmd->u.cfg.key) {
    case LOGX_CFG_CONSOLE_LOGGING:
        cmd->u.cfg.value ? logx_enable_console_logging(logger)
                         : logx_disable_console_logging(logger);
        break;
    case LOGX_CFG_FILE_LOGGING:
        cmd->u.cfg.value ? logx_enable_file_logging(logger) : logx_disable_file_logging(logger);
        break;
    case LOGX_CFG_CONSOLE_LOG_LEVEL:
        logx_set_console_logging_level(logger, (logx_level_t)cmd->u.cfg.value);
        break;
    case LOGX_CFG_FILE_LOG_LEVEL:
        logx_set_file_logging_level(logger, (logx_level_t)cmd->u.cfg.value);
        break;
    case LOGX_CFG_COLORED_LOGGING:
        cmd->u.cfg.value ? logx_enable_colored_logging(logger)
                         : logx_disable_colored_logging(logger);
        break;
    case LOGX_CFG_TTY_DETECTION:
        cmd->u.cfg.value ? logx_enable_tty_detection(logger) : logx_disable_tty_detection(logger);
        break;
    case LOGX_CFG_PRINT_CONFIG:
        cmd->u.cfg.value ? logx_enable_print_config(logger) : logx_disable_print_config(logger);
        break;
    case LOGX_CFG_ROTATE_TYPE:
        logx_set_log_rotate_type(logger, (logx_rotate_type_t)cmd->u.cfg.value);
        break;
    case LOGX_CFG_LOG_FILE_SIZE_MB:
        logx_set_log_file_size_mb(logger, (size_t)cmd->u.cfg.value);
        break;
    case LOGX_CFG_ROTATION_INTERVAL_DAYS:
        logx_set_rotation_interval_days(logger, (int)cmd->u.cfg.value);
        break;
    case LOGX_CFG_MAX_BACKUPS:
        logx_set_num_of_logfile_backups(logger, (int)cmd->u.cfg.value);
        break;
    default: break;
    }
}

static void process_timer_command(logx_t *logger, cmd_t *cmd) {
    switch (cmd->u.timer.action) {
    case TIMER_START: logx_timer_start(logger, cmd->u.timer.name); break;
    case TIMER_STOP: logx_timer_stop(logger, cmd->u.timer.name); break;
    case TIMER_PAUSE: logx_timer_pause(logger, cmd->u.timer.name); break;
    case TIMER_RESUME: logx_timer_resume(logger, cmd->u.timer.name); break;
    default: break;
    }
}

static int is_valid_hdr(ipc_hdr_t *hdr) {
    if (hdr->magic != LOGX_IPC_MAGIC)
        return 0;
    if (hdr->version != LOGX_IPC_VERSION)
        return 0;
    if (hdr->payload_len <= 0)
        return 0;
    return 1;
}

static logx_t *find_logger_session(pid_t pid) {
    uint32_t        idx  = pid_hash(pid);
    logx_session_t *node = atomic_load_explicit(&session_table[idx].head, memory_order_acquire);

    while (node) {
        if (node->pid == pid)
            return node->logger;
        node = node->next;
    }

    return NULL;
}

static void create_logger_session(const cmd_t *cmd) {
    uint32_t idx = pid_hash(cmd->hdr.pid);

    logx_session_t *node = calloc(1, sizeof(*node));
    if (!node)
        return;

    node->pid    = cmd->hdr.pid;
    node->logger = logxd_create(cmd->u.config_file_path);

    pthread_mutex_lock(&session_list_lock);

    node->next = atomic_load(&session_table[idx].head);
    atomic_store(&session_table[idx].head, node);

    pthread_mutex_unlock(&session_list_lock);
}

static void destroy_logger_session(pid_t pid) {
    uint32_t idx = pid_hash(pid);

    pthread_mutex_lock(&session_list_lock);

    logx_session_t *prev = NULL;
    logx_session_t *curr = atomic_load(&session_table[idx].head);

    while (curr) {
        if (curr->pid == pid) {
            if (prev)
                prev->next = curr->next;
            else
                atomic_store(&session_table[idx].head, curr->next);

            logx_destroy(curr->logger);
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    pthread_mutex_unlock(&session_list_lock);
}

static void *client_thread(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    cmd_t   cmd;
    logx_t *logger = NULL;
    ssize_t n;

    memset(&cmd, 0, sizeof(cmd));

    n = read(client_fd, &cmd.hdr, sizeof(cmd.hdr));
    if (n <= 0) {
        close(client_fd);
        return NULL;
    }

    if (is_valid_hdr(&cmd.hdr)) {
        n = read(client_fd, &cmd.u, cmd.hdr.payload_len);
        if (n <= 0) {
            close(client_fd);
            return NULL;
        }
    }

    switch (cmd.hdr.cmd_type) {
    case CMD_LOG:
        if ((logger = find_logger_session(cmd.hdr.pid)))
            process_log_command(logger, &cmd);
        break;
    case CMD_CFG:
        if ((logger = find_logger_session(cmd.hdr.pid)))
            process_cfg_command(logger, &cmd);
        break;
    case CMD_TIMER:
        if ((logger = find_logger_session(cmd.hdr.pid)))
            process_timer_command(logger, &cmd);
        break;
    case CMD_CREATE:
        if (!find_logger_session(cmd.hdr.pid))
            create_logger_session(&cmd);
        break;
    case CMD_DESTROY:
        if (find_logger_session(cmd.hdr.pid))
            destroy_logger_session(cmd.hdr.pid);
        break;
    case CMD_ROTATE_NOW:
        if ((logger = find_logger_session(cmd.hdr.pid)))
            logx_rotate_now(logger);
        break;
    default: break;
    }

    /* Close client socket */
    close(client_fd);

    return NULL;
}

static void server_loop(void) {
    while (running) {
        int      *client_fd;
        pthread_t tid;

        client_fd = malloc(sizeof(int));
        if (!client_fd)
            continue;

        *client_fd = accept(server_fd, NULL, NULL);
        if (*client_fd < 0) {
            free(client_fd);
            if (errno == EINTR)
                continue;
            perror("accept");
            break;
        }

        if (pthread_create(&tid, NULL, client_thread, client_fd) == 0) {
            pthread_detach(tid);
        } else {
            fprintf(stderr, "%s(%d): Failed to create client thread\n", __FILE__, __LINE__);
            close(*client_fd);
            free(client_fd);
        }
    }
}

static void *reaper_thread(void *arg) {
    (void)arg;

    while (running) {
        pthread_mutex_lock(&session_list_lock);

        for (int i = 0; i < LOGXD_SESSION_BUCKETS; i++) {
            logx_session_t *prev = NULL;
            logx_session_t *curr = atomic_load(&session_table[i].head);

            while (curr) {
                if (!pid_is_alive(curr->pid)) {
                    logx_session_t *dead = curr;

                    if (prev)
                        prev->next = curr->next;
                    else
                        atomic_store(&session_table[i].head, curr->next);

                    curr = curr->next;

                    logx_destroy(dead->logger);
                    free(dead);
                    continue;
                }

                prev = curr;
                curr = curr->next;
            }
        }

        pthread_mutex_unlock(&session_list_lock);

        /* Reap interval */
        sleep(LOGXD_REAPER_INTERVAL); /* Tunable */
    }

    return NULL;
}

int main(void) {
    if (daemonize() < 0) {
        perror("daemonize");
        exit(EXIT_FAILURE);
    }

    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);

    if (open_socket() < 0)
        exit(EXIT_FAILURE);

    pthread_t reaper_tid;
    if (pthread_create(&reaper_tid, NULL, reaper_thread, NULL) != 0) {
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    server_loop();

    return 0;
}
