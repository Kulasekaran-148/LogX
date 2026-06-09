#include <logx.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    logx_t *logger = NULL;
    if (logx_create(NULL, &logger) != LOGX_ERR_SUCCESS)
    {
        fprintf(stderr, "Failed to create logger\n");
        return -1;
    }

    while (1)
    {
        LOGX_INFO_FREQ(logger, 5, "This message will be logged at most once every 5 seconds");
        sleep(1);
    }

    logx_destroy(logger);

    return 0;
}