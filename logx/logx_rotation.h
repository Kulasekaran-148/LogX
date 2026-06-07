#ifndef LOGX_ROTATION_H
#define LOGX_ROTATION_H

#include "logx_errorcodes.h"
#include "logx_types.h"
#include "stddef.h"

struct logx_rotate_cfg_t
{
    logx_rotate_type_t type;
    size_t size_mb;
    int max_backups;
    int after_days;
};

#ifdef __cplusplus
extern "C"
{
#endif

    /* Function declarations */
    logx_errorcodes_t logx_set_log_rotate_type(logx_t *logger, logx_rotate_type_t type);
    logx_errorcodes_t logx_rotate_now(logx_t *logger);
    logx_errorcodes_t logx_set_log_file_size_mb(logx_t *logger, size_t size_mb);
    logx_errorcodes_t logx_set_num_of_logfile_backups(logx_t *logger, int max_backups);
    logx_errorcodes_t logx_set_rotation_after_days(logx_t *logger, int after_days);

    logx_errorcodes_t check_and_rotate_log(logx_t *ptLogger);

#ifdef __cplusplus
}
#endif

#endif /* LOGX_ROTATION_H */