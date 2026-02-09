#include <errno.h>
#include <time.h>

long int
gettime_res(void)
{
	return 1000000000L;
}

void
gettime(struct timespec *ts)
{
#ifdef CLOCK_REALTIME
	if(clock_gettime(CLOCK_REALTIME, ts) == 0)
		return;
#endif
	ts->tv_sec = time(NULL);
	ts->tv_nsec = 0;
}

struct timespec
current_timespec(void)
{
	struct timespec ts;
	gettime(&ts);
	return ts;
}

int
settime(struct timespec const *ts)
{
	(void)ts;
	errno = ENOSYS;
	return -1;
}
