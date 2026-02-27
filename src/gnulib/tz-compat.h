/* Minimal timezone helpers so parse-datetime.c can build without
 * pulling in the full gnulib time module.
 */

#ifndef STROKE_TZ_COMPAT_H
#define STROKE_TZ_COMPAT_H 1

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct stroke_timezone {
	bool use_utc;
} *timezone_t;

static inline bool
stroke_tzstring_is_utc(const char *tzstring)
{
	if(!tzstring || !*tzstring)
		return false;

	if(strcmp(tzstring, "UTC") == 0)
		return true;
	if(strcmp(tzstring, "UTC0") == 0)
		return true;
	if(strcmp(tzstring, "UTC+0") == 0 || strcmp(tzstring, "UTC-0") == 0)
		return true;
	if(strcmp(tzstring, "GMT") == 0)
		return true;
	if(strcmp(tzstring, "GMT0") == 0)
		return true;

	return false;
}

static inline timezone_t
tzalloc(const char *tzstring)
{
	timezone_t tz = malloc(sizeof(*tz));
	if(!tz)
		return NULL;

	tz->use_utc = stroke_tzstring_is_utc(tzstring);
	return tz;
}

static inline void
tzfree(timezone_t tz)
{
	free(tz);
}

static inline struct tm *
localtime_rz(timezone_t tz, time_t const *t, struct tm *result)
{
	if(tz && tz->use_utc)
		return gmtime_r(t, result);

	return localtime_r(t, result);
}

static inline time_t
mktime_z(timezone_t tz, struct tm *tm)
{
	if(tz && tz->use_utc)
		return timegm(tm);

	return mktime(tm);
}

#endif /* STROKE_TZ_COMPAT_H */
