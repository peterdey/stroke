/*
 *      aux.c - Auxiliary functionality for stroke
 * 		
 *      Copyright 2008 Sören Wellhöfer <soeren.wellhoefer@gmx.net>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include "stroke.h"

#include "errors.h"

#include <libgeneral/error.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

/***********
 * Globals *
 ***********/

static const char *wdays[] =
	{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

/* Ranges for time value validation */
static VALUE_BOUNDS bounds = {
	/* year, month, day, hour, minute, second, dst, week day */
	{1899, 2101}, {0, 13}, {0, 32}, {-1, 24}, {-1, 60}, {-1, 60}, {-1, 3}, {-1, 7}
};

/*******************************
 * General auxiliary functions *
 *******************************/

/*
 * Current verbosity level function.
 * Used as reference by libgeneral.
 */
inline int
verbosity_level()
{
	return CHKF(VERBOSE);
}

/* legacy helper functions removed */

/*
 * Checks if string is a natural number.
 * If with_null is TRUE a leading 0 is accepted
 * as a number.
 */
GENERAL_BOOL isnum_zero(const char *str, GENERAL_BOOL with_null)
{
	if(!str || !*str)
		return FALSE;
	if(!with_null && (*str == '0' && *(str+1)))
		return FALSE;
	while(*str)
		if(!isdigit(*str++))
			return FALSE;
	return TRUE;
}

/*
 * Checks if string is a natural number.
 */
inline GENERAL_BOOL isnum(const char *str)
{
	return isnum_zero(str, FALSE);
}

/*
 * Returned current cwd using new_str().
 * Return NULL on failure, a pointer on success.
 */
static char* scwd()
{
#ifdef HAVE_GETCWD
	char buf[PATH_MAX];
	if(!getcwd(buf, sizeof buf))
		return NULL;
	return new_str(buf);
#else
	char *pwd;
	if(!(pwd = getenv("PWD")))
		return NULL;
	return new_str(pwd);
#endif
}
/*
 * Set the modification and access time of a symbolic link.
 * Linux does not have the lutime() system call by default
 * which is a why a hack using utimensat() is used here.
 * However, utimens() is only present on more recent
 * versions of Linux.
 * This function returns the exit status lutime(),
 * utimensat(), or utime() respectively or -1 if
 * a general error occurred.
 */
int
lutime_symlink(const char *filename, const struct utimbuf *times)
{
	verbose(1, "Altering symbolic link if one");
	
#if defined(HAVE_LUTIME)
	return lutime(filename, times);
#elif defined(HAVE_UTIMENSAT)
	
	char *cwd = NULL;
	int retv;
	struct timespec ts[2] = {
		{times->actime, 0},
		{times->modtime, 0}
	};
	
	if(!(cwd = scwd(cwd)) ||
	   chdir(dirname(new_str(filename))) < 0)
		goto error;
	
	retv = utimensat(AT_FDCWD, basename(new_str(filename)), ts, AT_SYMLINK_NOFOLLOW);

	if(chdir(cwd) < 0)
		goto error;

	return retv;

 error:
	error_out(ERROR_ERROR_UTIMSYM, errno, FLN, filename);
	return -1;
	
#else
# warning lutime() or utimensat() missing; will not be able to set atime or mtime of symlink 
	return utime(filename, times);
#endif
}

/*
 * Similiar to access(). Only checks whether pathname
 * is a symlink exclusively. If so, 0 is returned
 * otherwise -1.
 * If link is a dangling symlink return LDANGLING.
 */
int
laccess(const char *pathname, int mode)
{
	/* Fixme:
	 * This function is intrinsically faulty, as pathname might
	 * still exist, however, lstat() fails as read permissions
	 * are denied.
	 * On the other hand, I know of no nice way to
	 * check whether a symlink exists as it is or not.
	 * Note that the mode field of this function is ignored
	 * altogether; F_OK may be given simply for readability.
	 */
	struct stat buf;
	
	if((lstat(pathname, &buf) < 0 ||
	    !S_ISLNK(buf.st_mode)))
		return -1;

	return !access(pathname, F_OK) ? 0 : LDANGLING;
}

/*
 * If file refers to a regular file that name
 * is returned. In case file refers to a
 * symbolic link, the name of the file pointed
 * to is returned. String returned is new_str()'d
 * in case of a symlink, otherwise it's just file.
 * NULL is returned on error.
 */
const char*
realname(const char *file)
{
	char lnk[PATH_MAX];
	struct stat st;
	ssize_t lbyt;

	if(lstat(file, &st) < 0)
		return NULL;

	if(S_ISLNK(st.st_mode)) {
		if((lbyt = readlink(file, lnk, sizeof lnk)) < 0)
			return NULL;
		lnk[lbyt] = 0;
		return (const char*)new_str(lnk);
	} 

	return file;
}


/*********************************
 * Specific auxiliariy functions *
 *********************************/

/*
 * Convert FILE_TIME table to fill `struct utimbuf' as used by utime()
 * and lutime().
 * Note that utimbuf does not contain a field for ctime.
 * Returns 0 on success, -1 on failure.
 */
int
ft_to_utimbuf(FILE_TIMES time_vals, struct utimbuf *ut)
{
	struct tm tm;

	memset(&tm, 0, sizeof(struct tm));
	
	translate(&tm, time_vals, MTIME, TO_TM);
	if((ut->modtime = mktime(&tm)) < 0) goto error;

	translate(&tm, time_vals, ATIME, TO_TM);
	if((ut->actime = mktime(&tm)) < 0) goto error;

	return 0;
 error:
	error_out(ERROR_ERROR_TSTMP, 0, FLN);
	return -1;
}

/*
 * Validate a time_vals structure for what is logically feasable as date.
 * Returns 0 upon successful validation, -1 otherwise.
 */
int
validate_times(FILE_TIMES time_vals)
{
	register int j;
	int t;

	if(CHKF(FORCE))
		return 0;

	for(t = 0; t < TIME_TBLS; t++) {
		for(j = 0; j < TIME_VALS-1; j++) {
			if(!validate(time_vals[t][j].name, time_vals[t][j].val))
				goto error;
		}

		struct tm tm = {0};
		translate(&tm, time_vals, t, TO_TM);
		struct tm original = tm;
		if(mktime(&tm) == (time_t)-1)
			goto error;
		if(original.tm_year != tm.tm_year ||
		   original.tm_mon  != tm.tm_mon  ||
		   original.tm_mday != tm.tm_mday)
			goto error;
	}

	return 0;

error:
	error_out(ERROR_ERROR_VALDAT, 0, FLN, tv_to_str(time_vals, t));
	return -1;
}

/*
 * Change a file's ctime according to the values set in
 * time_vals array passed.
 * As ctime cannot be directly modified a trick is used:
 * the system clock is reset to the desired ctime,
 * then a chmod() or fopen(..., "w") call is performed
 * which alteres the ctime as desired.
 * Note that CAP_SYS_TIME capability is required under
 * linux, which, by default, is only masked to root.
 * I guess other systems handle this similarily.
 * Returns 0 on success, -1 on failure.
 */
int
mod_ctime(FILE_TIME (*time_vals)[], const char *file)
{
	int (*statf)(const char*, struct stat*);
	int (*chmodf)(const char *, mode_t);
	struct timeval current, elapsed = {0, 0}, ctime = {0, 0};
	struct stat st;
	struct tm tm;
	
	verbose(1, "Attempting to %s change time",
		CHKF(CTPRES) ? "preserve" : "modify");

	memset(&tm, 0, sizeof tm);
	translate(&tm, time_vals, CTIME, TO_TM);

	if(gettimeofday(&current, NULL) < 0)
		goto error;

	if((ctime.tv_sec = mktime(&tm)) < 0)
		goto error;

	statf = CHKF(SYMLINKS) ? &lstat : &stat;

	if((*statf)(file, &st) < 0)
		goto error;
		
#ifdef HAVE_LCHMOD
	chmodf = CHKF(SYMLINKS) ? &lchmod : &chmod;
#else
#warning Will not be able to alter ctime of symbolic links
	chmodf = &chmod;
#endif
	
	/*
	 * Note: Beware clock skews; hence use of timer()
	 */
	
	/* set to ctime, change file, set back to current time */
	if(settimeofday(&ctime, NULL))
		goto error;

	(void)timer(NULL);

	if((*chmodf)(file, st.st_mode))
		goto error;

	(void)timer(&elapsed);

	/* Fixme: Adding seconds too slow/unnecessary? */
	current.tv_usec += elapsed.tv_usec;
	current.tv_sec += elapsed.tv_sec;
	
	if(settimeofday(&current, NULL))
		goto error;
	
	return 0;

 error:
	error_out(ERROR_ERROR_CHCTIME, errno, FLN, file,
		  IFSTR(geteuid(), "Root privileges might be required."));
	return -1;
}

/*
 * Maps the string representation of time_values to
 * enum constants that represent indices within the time_vals array.
 */
static inline int
value_mapping(const char *name)
{
	switch(*++name) {
		case 'Y': return YEAR; break;
		case 'M': return MON; break;
		case 'D': return DAY; break;
		case 'h': return HOUR; break;
		case 'm': return MIN; break;
		case 's': return SEC; break;
		case 'l': return DST; break;
	}
	return -1;
}


/*
 * Returns TRUE if all time values within the time_vals struct could successfully be
 * validated - syntactically and logically that is. Returns FALSE otherwise.
 */
GENERAL_BOOL
validate(const char *name, int val)
{
	int time_map;

	if(CHKF(FORCE))
		return TRUE;
	if(!name || strlen(name) != 2)
		return FALSE;
	if((time_map = value_mapping(name)) < 0)
		return FALSE;

	return (bounds[time_map].lower < val && bounds[time_map].upper > val);
}

/*
 * Translate between standard time structue `struct tm' and FILE_TIME array as used by stroke.
 */
void
translate(struct tm *tm, FILE_TIMES ft, int mactime, GENERAL_BOOL to_file_time) 
{
	if(to_file_time) {
		ft[mactime][YEAR].val = tm->tm_year + YEAR_BASE;
		ft[mactime][MON].val = tm->tm_mon + MON_BASE;
		ft[mactime][DAY].val = tm->tm_mday;
		ft[mactime][HOUR].val = tm->tm_hour;
		ft[mactime][MIN].val = tm->tm_min;
		ft[mactime][SEC].val = tm->tm_sec;
		ft[mactime][DST].val = tm->tm_isdst + DST_BASE;
		ft[mactime][WKD].val = tm->tm_wday;
	} else {
		tm->tm_year = ft[mactime][YEAR].val - YEAR_BASE;
		tm->tm_mon = ft[mactime][MON].val - MON_BASE;
		tm->tm_mday = ft[mactime][DAY].val;
		tm->tm_hour = ft[mactime][HOUR].val;
		tm->tm_min = ft[mactime][MIN].val;
		tm->tm_sec = ft[mactime][SEC].val;
		tm->tm_isdst = ft[mactime][DST].val - DST_BASE;
	}
}

/*
 * Make string representation of time_vals date entry.
 */
char*
tv_to_str(FILE_TIMES time_vals, int t)
{
	char dbuf[128];
	sprintf(dbuf, DATE_FORMAT);
	return new_str(dbuf);
}


/*
 * Look at first character of time value identifier name and return
 * symbolic representation of array index.
 */
inline int
tstr(const char *tname)
{
	switch(*tname) {
		case 'm': return MTIME; break;
		case 'a': return ATIME; break;	
		case 'c': return CTIME; break;
	}	
	return -1;
}

/*
 * Attempts to find entry in time_vals table given by its name.
 * If lookup is TRUE the value found will be written to *val,
 * if lookup is FALSE the value pointed to by val will be written
 * to the time_vals table.
 * Returns 0 on success, -1 on failure (equivalent to identifier
 * not found). Also, may return -2 if ctime preservation was opted
 * for (`-preserve'), but ctime changes were attempted herein.
 */
int
times_mod(FILE_TIMES time_vals, const char *name, int *val, GENERAL_BOOL lookup)
{
	int times_tbl, i;
	FILE_TIME *tbl;

	if(*name == 'c') {
		if(CHKF(CTPRES)) {
			error_out(ERROR_ERROR_CTCHPR, 0, FLN);
			return -2;
		} else SETF(CTAPPLY);
	}
	
	if((times_tbl = tstr(name)) < 0) return -1;

	tbl = *(time_vals + times_tbl);

	for(i = 0; i < TIME_VALS-1; i++) {
		if(!strcmp(tbl[i].name, name)) {
			if(lookup) 
				*val = tbl[i].val;
			else
				tbl[i].val = *val;
			return 0;
		}
	}
	
	return -1;
}
