#include <logx/logx.h>
#include <stdatomic.h>
#include <stddef.h>
#include <sys/types.h>

typedef struct logx_session {
    pid_t                pid;           /* user script PID */
    char                 file_name[64]; /* script name (optional) */
    logx_t              *logger;        /* owned logger */
    time_t               last_seen;     /* for cleanup */
    struct logx_session *next;
} logx_session_t;

typedef struct {
    _Atomic(logx_session_t *) head;
} session_bucket_t;