/**
 * @file logx_string_maps.h
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief String-map helpers that convert LogX error codes to display strings.
 * @version 2.0.0
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 */

#ifndef LOGX_STRING_MAPS_H
#define LOGX_STRING_MAPS_H

#include "logx_errorcodes.h"

/**
 * @brief Return a human-readable description of a LogX error code.
 *
 * @param[in] eErr Error code to look up.
 * @return Null-terminated constant string, e.g. `"LOGX_ERR_SUCCESS"`.
 *         Returns `"UNKNOWN"` for unrecognised values.
 */
const char *logx_get_err_string(logx_errorcodes_t eErr);

#endif /* LOGX_STRING_MAPS_H */