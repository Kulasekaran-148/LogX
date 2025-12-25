#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <pthread.h>

#include "../../include/logxd/logx_cli.h"
#include "../../include/logxd/reaper.h"
#include "../../include/logxd/session.h"

#define LOGXD_BACKLOG   16

static int server_fd = -1;
static volatile sig_atomic_t running = 1;

static void handle_signal(int sig)
{
    (void)sig;
    running = 0;
}

static int daemonize(void)
{
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
        exit(EXIT_SUCCESS);  /* Parent exits */

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
        exit(EXIT_SUCCESS);  /* Parent exits */

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
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    return 0;  /* Daemonization successful */
}


static int setup_socket(void)
{
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

static void *client_thread(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char req_buf[128 + LOGX_MAX_PAYLOAD_SIZE_BYTES];
    char resp_buf[1024];
    ssize_t n;

    /* Zero buffers */
    memset(req_buf, 0, sizeof(req_buf));
    memset(resp_buf, 0, sizeof(resp_buf));

    /* Read request from client */
    n = read(client_fd, req_buf, sizeof(req_buf) - 1);
    if (n <= 0) {
        /* Client closed connection or error */
        fprintf(stderr, "%s(%d): Failed to read from client socket\n", __FILE__, __LINE__);
        close(client_fd);
        return NULL;
    }

    /* Ensure NUL termination */
    req_buf[n] = '\0';

    /* Dispatch request
     * Expected to fill resp_buf with response text
     */
    if (dispatch_request(req_buf, resp_buf, sizeof(resp_buf)) < 0) {
        snprintf(resp_buf, sizeof(resp_buf),
                 "ERR|dispatch_failed\n");
    }

    /* Send response back to client */
    write(client_fd, resp_buf, strlen(resp_buf));

    /* Close client socket */
    close(client_fd);

    return NULL;
}

static void server_loop(void)
{
    while (running) {
        int *client_fd;
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

int main(int argc, char *argv[])
{
    if (daemonize() < 0) {
        perror("daemonize");
        exit(EXIT_FAILURE);
    }

    signal(SIGTERM, handle_signal);
    signal(SIGINT,  handle_signal);
    signal(SIGQUIT, handle_signal);

    if (setup_socket() < 0)
        exit(EXIT_FAILURE);

    server_loop();

    return 0;
}
