#ifndef CX_TIMER_H
#define CX_TIMER_H

#include <stdint.h>
#include <time.h>

typedef struct timespec cx_timer_t;

void cx_timer_reset(cx_timer_t *timer);
int64_t cx_timer_ns(cx_timer_t *timer);

#endif
