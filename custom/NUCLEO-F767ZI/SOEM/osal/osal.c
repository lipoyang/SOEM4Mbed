/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <stdlib.h>
#include "osal.h"

uint32_t us_ticker_read(void);
void wait_us(int us);

ec_timet osal_current_time (void)
{
   uint32 ret = us_ticker_read();
   return ret;
}

void osal_time_diff(ec_timet *start, ec_timet *end, ec_timet *diff)
{
   *diff = *end - *start;
}

void osal_timer_start (osal_timert *self, uint32 timeout_usec)
{
    self->start_time = us_ticker_read();
    self->timeout = timeout_usec;
}

boolean osal_timer_is_expired (osal_timert *self)
{
    uint32 now = us_ticker_read();
    uint32 elapsed = now - self->start_time;
    
    return (elapsed >= self->timeout);
}

int osal_usleep (uint32 usec)
{
    wait_us(usec);
    return 0;
}

void *osal_malloc(size_t size)
{
   return malloc(size);
}

void osal_free(void *ptr)
{
   free(ptr);
}

int osal_thread_create(void *thandle, int stacksize, void *func, void *param)
{
    // TODO not supported
    return 1;
}

int osal_thread_create_rt(void *thandle, int stacksize, void *func, void *param)
{
    // TODO not supported
    return 1;
}
