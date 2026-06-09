#include <logx.h>
#include <stdio.h>

int main()
{
    logx_t *logger = NULL;
    if (logx_create(NULL, &logger) != LOGX_ERR_SUCCESS)
    {
        fprintf(stderr, "Failed to create logger\n");
        return -1;
    }

    LOGX_BANNER(logger, "Binary string example");

    LOGX_DEBUG(logger, "Binary representation of %d is %s", 10, LOGX_BIN_STR(10));
    LOGX_DEBUG(logger, "Binary representation of %u is %s", 255u, LOGX_BIN_STR(255u));
    LOGX_DEBUG(logger, "Binary representation of %d is %s", -128, LOGX_BIN_STR(-128));
    LOGX_DEBUG(logger, "Binary representation of %u is %s", 65535u, LOGX_BIN_STR(65535u));
    LOGX_DEBUG(logger, "Binary representation of %d is %s", -32768, LOGX_BIN_STR(-32768));
    LOGX_DEBUG(logger, "Binary representation of %u is %s", 4294967295u, LOGX_BIN_STR(4294967295u));
    LOGX_DEBUG(logger, "Binary representation of %d is %s", -2147483648, LOGX_BIN_STR(-2147483648));

    logx_destroy(logger);

    return 0;
}