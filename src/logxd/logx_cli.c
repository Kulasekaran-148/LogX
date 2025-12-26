#include <logx/logx.h>
#include <logxd/logx_cli.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define LOGXD_SOCK_PATH "/run/logxd.sock"

static int open_socket(void) {
    int                fd;
    struct sockaddr_un addr;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, LOGXD_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

int main(int argc, char **argv) {
    cmd_t cmd;
    int   fd;

    if (argc < 2) {
        fprintf(stderr, "[LOGX-CLI]: insufficient arguments\n");
        return EXIT_FAILURE;
    }

    memset(&cmd, 0, sizeof(cmd));

    /* ---- Common header ---- */
    cmd.hdr.magic    = LOGX_IPC_MAGIC;
    cmd.hdr.version  = LOGX_IPC_VERSION;
    cmd.hdr.cmd_type = (uint16_t)atoi(argv[1]);
    cmd.hdr.pid      = (uint32_t)atoi(argv[2]);

    /* ---- Dispatch based on command ---- */
    switch (cmd.hdr.cmd_type) {
    case CMD_LOG:
        /* argv:
         * CMD_LOG PID LEVEL FILE LINE MESSAGE
         */
        if (argc < 7)
            return EXIT_FAILURE;

        cmd.u.log.level = (logx_level_t)atoi(argv[3]);
        strncpy(cmd.u.log.file_name, argv[4], sizeof(cmd.u.log.file_name) - 1);
        cmd.u.log.line_num = (uint32_t)atoi(argv[5]);
        strncpy(cmd.u.log.message, argv[6], sizeof(cmd.u.log.message) - 1);
        cmd.hdr.payload_len = sizeof(cmd.u.log);
        break;

    case CMD_CFG:
        /* argv: CMD_CFG KEY VALUE */
        if (argc < 4)
            return EXIT_FAILURE;

        cmd.u.cfg.key       = (uint8_t)atoi(argv[2]);
        cmd.u.cfg.value     = (uint32_t)atoi(argv[3]);
        cmd.hdr.payload_len = sizeof(cmd.u.cfg);
        break;

    case CMD_ROTATE_NOW: cmd.hdr.payload_len = 0; break;

    case CMD_TIMER:
        /* argv: CMD_TIMER ACTION NAME */
        if (argc < 4)
            return EXIT_FAILURE;

        cmd.u.timer.action = (uint8_t)atoi(argv[2]);
        strncpy(cmd.u.timer.name, argv[3], sizeof(cmd.u.timer.name) - 1);
        cmd.hdr.payload_len = sizeof(cmd.u.timer);
        break;

    case CMD_CREATE:
        /* argv: CMD_CREATE PID [CONFIG_PATH] */
        if (argc == 4) {
            strncpy(cmd.u.config_file_path, argv[3], sizeof(cmd.u.config_file_path) - 1);
            cmd.hdr.payload_len = sizeof(cmd.u.config_file_path);
        } else {
            cmd.u.config_file_path[0] = '\0';
        }
        break;

    case CMD_DESTROY:
        /* argv: CMD_DESTROY PID */
        break;

    default: fprintf(stderr, "[LOGX-CLI]: unknown command\n"); return EXIT_FAILURE;
    }

    fd = open_socket();
    if (fd < 0) {
        perror("connect");
        return EXIT_FAILURE;
    }

    if (write(fd, &cmd, sizeof(cmd.hdr) + cmd.hdr.payload_len) < 0) {
        perror("write");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);
    return EXIT_SUCCESS;
}
