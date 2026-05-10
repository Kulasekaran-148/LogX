#include "logx_errorcodes.h"
#include <stdio.h>

typedef struct {
    int dwKey;
    const char *value;
} string_map_t;

static const char *get_string_from_map(int dwKey, const string_map_t *ptMap, size_t map_count, const char *unknown_prefix);

static const string_map_t g_tErrorcodesMap[] = {
    /* Generic */
    { LOGX_ERR_FAILURE,                "LOGX_ERR_FAILURE" },
    { LOGX_ERR_SUCCESS,                "LOGX_ERR_SUCCESS" },
    { LOGX_ERR_INVALID_VALUE,          "LOGX_ERR_INVALID_VALUE" },
    { LOGX_ERR_OUT_OF_RANGE,           "LOGX_ERR_OUT_OF_RANGE" },
    { LOGX_ERR_NO_MEM,                 "LOGX_ERR_NO_MEM" },
    { LOGX_ERR_NULL_PTR,               "LOGX_ERR_NULL_PTR" },
    { LOGX_ERR_INVALID_ARG,            "LOGX_ERR_INVALID_ARG" },

    /* Thread */
    { LOGX_ERR_THREAD_CREATION_FAILED, "LOGX_ERR_THREAD_CREATION_FAILED" },
    { LOGX_ERR_THREAD_NAMING_FAILED,   "LOGX_ERR_THREAD_NAMING_FAILED" },
    { LOGX_ERR_THREAD_JOIN_FAILED,     "LOGX_ERR_THREAD_JOIN_FAILED" },
    { LOGX_ERR_THREAD_CANCEL_FAILED,   "LOGX_ERR_THREAD_CANCEL_FAILED" },

    /* File */
    { LOGX_ERR_FILE_NOT_FOUND,         "LOGX_ERR_FILE_NOT_FOUND" },
    { LOGX_ERR_FILE_OPEN_FAILED,       "LOGX_ERR_FILE_OPEN_FAILED" },
    { LOGX_ERR_FD_OPEN_FAILED,         "LOGX_ERR_FD_OPEN_FAILED" },
    { LOGX_ERR_FILE_READ_FAILED,       "LOGX_ERR_FILE_READ_FAILED" },
    { LOGX_ERR_FILE_WRITE_FAILED,      "LOGX_ERR_FILE_WRITE_FAILED" },
    { LOGX_ERR_FILE_RENAME_FAILED,     "LOGX_ERR_FILE_RENAME_FAILED" },
    { LOGX_ERR_FLOCK_FAILED,           "LOGX_ERR_FLOCK_FAILED" },
    { LOGX_ERR_FUNLOCK_FAILED,         "LOGX_ERR_FUNLOCK_FAILED" },

    /* Directory */
    { ERR_DIRECTORY_CREATION_FAILED,   "ERR_DIRECTORY_CREATION_FAILED" },
};