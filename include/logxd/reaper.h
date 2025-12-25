#ifndef LOGXD_REAPER_H
#define LOGXD_REAPER_H

/* Start the reaper background thread */
int reaper_start(void);

/* Stop the reaper and wait for it to exit */
void reaper_stop(void);

#endif
