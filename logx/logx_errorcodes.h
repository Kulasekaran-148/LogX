/**
 * @file logx_errorcodes.h
 * @author Kulasekaran (kulasekaranslrk@gmail.com)
 * @brief Error code utilities for LogX.
 * @version 2.0.0
 * @date 2025-11-10
 *
 * @copyright Copyright (c) 2025
 */

#ifndef LOGX_ERRORCODES_H
#define LOGX_ERRORCODES_H

#include "logx_types.h"

/**
 * @brief Convert a LogX error code to a human-readable string.
 *
 * @param[in] eErr Error code to convert.
 * @return Null-terminated string describing the error, or `"UNKNOWN"` if the
 *         code is not recognised.
 */
const char *logx_errcode_to_str(logx_errorcodes_t eErr);

#endif /* LOGX_ERRORCODES_H */