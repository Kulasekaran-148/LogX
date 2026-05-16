#include "logx_errorcodes.h"

#include <sys/file.h>

/**
 * @brief Acquire an exclusive advisory lock on a file descriptor using flock().
 *
 * @param[in] fd File descriptor to lock.
 * @return int 0 on success, -1 on failure (invalid fd or flock error).
 * @note This uses advisory locking. Other processes must also use flock()
 *       for the lock to be respected.
 */
logx_errorcodes_t exclusive_flock(int fd)
{
    if (fd < 0)
        return LOGX_ERR_INVALID_FD;
    if (flock(fd, LOCK_EX) == -1)
        return LOGX_ERR_FLOCK_FAILED;
    return LOGX_ERR_SUCCESS;
}

/**
 * @brief Release a previously acquired advisory lock on a file descriptor.
 *
 * @param[in] fd File descriptor to unlock.
 * @return logx_errorcodes_t
 * @note Only unlocks descriptors previously locked with flock().
 */
logx_errorcodes_t unlock_flock(int fd)
{
    if (fd < 0)
        return LOGX_ERR_INVALID_FD;
    if (flock(fd, LOCK_UN) == -1)
        return LOGX_ERR_FLOCK_FAILED;
    return LOGX_ERR_SUCCESS;
}