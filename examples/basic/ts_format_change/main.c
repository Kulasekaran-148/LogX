#include <logx.h>
#include <stdio.h>

int main()
{
    logx_t *logger = logx_create(NULL);
    if (!logger)
    {
        fprintf(stderr, "Failed to create logger\n");
        return -1;
    }

    logx_set_ts_format_to_local(logger);
    LOGX_DEBUG(logger, "This message contains timestamp in LOGX_TS_FMT_LOCAL");

    logx_set_ts_format_to_utc(logger);
    LOGX_DEBUG(logger, "This message contains timestamp in LOGX_TS_FMT_UTC");

    logx_set_ts_format_to_epoch_s(logger);
    LOGX_DEBUG(logger, "This message contains timestamp in LOGX_TS_FMT_EPOCH_S");

    logx_set_ts_format_to_epoch_ms(logger);
    LOGX_DEBUG(logger, "This message contains timestamp in LOGX_TS_FMT_EPOCH_MS");

    logx_set_ts_format_to_epoch_us(logger);
    LOGX_DEBUG(logger, "This message contains timestamp in LOGX_TS_FMT_EPOCH_US");

    logx_set_ts_format_to_iso8601(logger);
    LOGX_DEBUG(logger, "This message contains timestamp in LOGX_TS_FMT_ISO8601");

    logx_set_ts_format_to_rfc2822(logger);
    LOGX_DEBUG(logger, "This message contains timestamp in LOGX_TS_FMT_RFC8222");

    if (logger)
    {
        logx_destroy(logger);
    }

    return 0;
}