/* Minimal timezone helpers so parse-datetime.c can build without
 * pulling in the full gnulib time module.
 */

#ifndef STROKE_TZ_COMPAT_H
#define STROKE_TZ_COMPAT_H 1

#include <time.h>

typedef int timezone_t;

static inline timezone_t
tzalloc(const char *tzstring)
{
	(void)tzstring;
	return 1;
}

static inline void
tzfree(timezone_t tz)
{
	(void)tz;
}

static inline struct tm *
localtime_rz(timezone_t tz, time_t const *t, struct tm *result)
{
	(void)tz;
	return localtime_r(t, result);
}

static inline time_t
mktime_z(timezone_t tz, struct tm *tm)
{
	(void)tz;
	return mktime(tm);
}

#endif /* STROKE_TZ_COMPAT_H */
