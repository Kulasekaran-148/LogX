/**
 * @file logx_string_maps.c
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Error code to string mapping, generated from logx_errorcodes.def.
 * @version 2.0.0
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 */

#include "logx_common.h"
#include "logx_errorcodes.h"
#include <stdio.h>

typedef struct
{
    int dwKey;
    const char *value;
} string_map_t;

/**
 * @brief Look up an integer key in a string map and return the associated string.
 *
 * If the key is not found, returns a thread-local buffer containing
 * `"<unknown_prefix> - <key>"`. Returns NULL if `unknown_prefix` is NULL and the
 * key is not found.
 *
 * @param[in] dwKey          Integer key to look up.
 * @param[in] ptMap          Pointer to the string map array.
 * @param[in] map_count      Number of elements in the map.
 * @param[in] unknown_prefix Prefix string used when the key is not found, or NULL.
 * @return Matching value string, a fallback string, or NULL.
 */
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