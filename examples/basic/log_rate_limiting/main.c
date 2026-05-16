#include <logx.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    logx_t *logger = logx_create(NULL);
    if (!logger)
    {
        fprintf(stderr, "Failed to create logger\n");
        return -1;
    }

    while (1)
    {
        LOGX_INFO_FREQ(logger, 5, "This message will be logged at most once every 5 seconds");
        sleep(1);
    }

    return 0;
}