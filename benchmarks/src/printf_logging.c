#include <stdio.h>
#include <stdlib.h>
#include <logx/logx.h>

void log_messages(logx_t *logger, int limit) {
    LOGX_TIMER_AUTO(logger, "Console logging timer");
    for (int i = 0; i < limit; i++) {
        LOGX_INFO(logger, "This is log message number: %d", i + 1);
    }
}

void printf_messages(logx_t *logger, int limit) {
    LOGX_TIMER_AUTO(logger, "Printf logging timer");
    for (int i = 0; i < limit; i++) {
        printf("This is printf message number: %d\n", i + 1);
    }
}

int main(int argc, char *argv[])
{
    int limit = 10000;  // default limit

    if (argc >= 2) {
        limit = atoi(argv[1]);
        if (limit <= 0) {
            fprintf(stderr, "Invalid limit provided: %s\n", argv[1]);
            fprintf(stderr, "Usage: %s <limit>\n", argv[0]);
            return -1;
        }
    }

    logx_t *logger = logx_create(NULL);
    if (!logger) {
        fprintf(stderr, "Failed to create logger instance\n");
        return -1;
    }

    logx_disable_file_logging(logger);

    LOGX_BANNER(logger, "Measuring time taken to print %d logs", limit);

    // Directly using printf to log messages
    printf_messages(logger, limit);

    logx_destroy(logger);
    return 0;
}
