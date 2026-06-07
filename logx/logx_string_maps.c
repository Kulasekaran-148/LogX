#include "logx_common.h"
#include "logx_errorcodes.h"
#include <stdio.h>

typedef struct
{
    int dwKey;
    const char *value;
} string_map_t;

static const char *get_string_from_map(int dwKey, const string_map_t *ptMap, size_t map_count,
                                       const char *unknown_prefix);

static const string_map_t gtErrorcodesMap[] = {
#define LOGX_ERROR_AUTO(code)          {code, #code},
#define LOGX_ERROR_EXPLICIT(code, val) {code, #code},
#include "logx_errorcodes.def"
#undef LOGX_ERROR_AUTO
#undef LOGX_ERROR_EXPLICIT
};

#define ERRORCODES_MAP_COUNT ARRAY_SIZE(gtErrorcodesMap)

static const char *get_string_from_map(int dwKey, const string_map_t *ptMap, size_t map_count,
                                       const char *unknown_prefix)
{
    static _Thread_local char buffer[64];

    if (!ptMap)
        return "ptMap is NULL";

    for (size_t i = 0; i < map_count; i++)
    {
        if (ptMap[i].dwKey == dwKey)
            return ptMap[i].value;
    }

    if (unknown_prefix == NULL)
        return NULL;

    snprintf(buffer, sizeof(buffer), "%s - %d", unknown_prefix, dwKey);
    return buffer;
}

const char *logx_get_err_string(logx_errorcodes_t eErr)
{
    return get_string_from_map((int)eErr, gtErrorcodesMap, ERRORCODES_MAP_COUNT,
                               "LOGX_ERR_UNKNOWN");
}