/*
 *      stroke.h - Main header file for stroke
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

#ifndef STROKE_H
#define STROKE_H 1

#ifndef HAVE_CONFIG_H
# define PACKAGE "stroke"
# define VERSION "<test-build>"
# define PACKAGE_BUGREPORT "<maintainer>"
# include <string.h>
#else
# include <config.h>
# ifdef HAVE_UTIMENSAT
#  ifndef _ATFILE_SOURCE
#   define _ATFILE_SOURCE
#  endif
#  ifndef __USE_ATFILE
#   define __USE_ATFILE
#  endif
# endif
#endif

#include <libgeneral/general.h>
#include <sys/types.h>
#include <utime.h>

#ifdef STDC_HEADERS
#  include <string.h>
#else
#  ifndef HAVE_STRCHR
#    define strchr index
#  endif
char *strchr ();
#  ifndef HAVE_STRLEN
size_t strlen(const char *);
#  endif 
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#define PROGRAM PACKAGE

/* Possible program flags */
enum {
	FORCE = _FLAG(0),
	VERBOSE = _FLAG(1),
	SYMLINKS = _FLAG(2),
	QUIET = _FLAG(3),
	INFO = _FLAG(4),
	BATCH = _FLAG(5),
	STAMP = _FLAG(6),
	REFER = _FLAG(7),
	NEXIST = _FLAG(8),
	CHCTIME = _FLAG(9),
	CTPRES = _FLAG(10),
	CTAPPLY = _FLAG(11),
};

/* Time table values */
#define TIME_TBLS 3
#define TIME_VALS 8

/* One specific date or time component */
typedef struct {

	int val;
	const char *name;
	
} FILE_TIME;

/* Time table */
typedef FILE_TIME (*FILE_TIMES)[TIME_VALS];

/* Time value boundaries; as used for validation by validate() */
struct val_bounds {

	int lower;
	int upper;
};

/* Value validation array */
typedef struct val_bounds VALUE_BOUNDS[];

/*
 * Symbolic names for time_vals array indices
 */
enum {
	MTIME = 0, ATIME, CTIME
};

enum {
	YEAR = 0, MON, DAY, HOUR, MIN, SEC, DST, WKD
};

/*
 * Value macros
 */

/* Date conversion and date formatting macros */
#define YEAR_BASE 1900
#define CURR_CENT 20
#define MON_BASE 1
#define DST_BASE 1
#define MODSEPS ",;:"
#define DATE_FORMAT \
	"%02d/%02d/%d-%02d:%02d:%02d %s (%cdst)",\
	D(MON), D(DAY), D(YEAR),\
	D(HOUR), D(MIN), D(SEC),\
	W(D(WKD)), L(D(DST))

/* Macros for times_mod(), translate(), and laccess() */
#define SET_VALUE 0
#define LOOKUP_VALUE 1
#define TO_TM 0
#define TO_FT 1
#define LDANGLING 1

/*
 * Auxilary functional macros
 */

/* Arguments */
#define ARG(SHORT, LONG) (ARH(SHORT) || ARH(LONG))
#define ARH(ARG) (!strcmp(ARG, arg))

/* Flags */
#define SETF(FLAG) flags |= (FLAG)
#define REMF(FLAG) flags &= ~(FLAG)
#define CHKF(FLAG) (flags & (FLAG))

/*FILE_TIMES initializer */
#define WD T(0)
#define T(NAM) {0, NAM}

/* Date format */
#define W(OFF) *(wdays+OFF)
#define D(OFF) (time_vals[t]+OFF)->val
#define L(DST) (DST ? (DST == 1 ? '-' : '+') : '?')

/*
 * External globals
 */

/* Program flags set */
extern _FLAG_TYPE flags;

/* mtime, atime, ctime  */
extern const char *names[];

/*
 * Function declarations
 */
extern char** sep_to_array(const char *seps, const char *str);
extern void free_str_array(char ***strs);
extern void translate(struct tm *tm, FILE_TIMES, int mactime, GENERAL_BOOL to_file_time);
extern int times_mod(FILE_TIMES, const char *name, int *val, GENERAL_BOOL lookup);
extern inline GENERAL_BOOL validate(const char *name, int val);
extern int validate_times(FILE_TIMES);
extern char* tv_to_str(FILE_TIMES, int t);
extern inline GENERAL_BOOL isnum(const char *str);
extern GENERAL_BOOL isnum_zero(const char *str, GENERAL_BOOL with_null);
extern int ft_to_utimbuf(FILE_TIMES, struct utimbuf *);
extern char* skew(char *a);
extern char* read_batch(FILE *st);
extern inline int tstr(const char *);
extern inline int str_array_size(const char **);
extern int lutime_symlink(const char *filename, const struct utimbuf *times);
extern int laccess(const char *pathname, int mode);
extern const char* realname(const char *file);
extern int mod_ctime(FILE_TIMES, const char *file);

/*
 * Debugging
 */
extern inline int verbosity_level();

#ifdef DEBUG
extern inline void dump_tv(FILE_TIME (*)[]);
extern void dump_str_array(const char**);
#endif

#endif /* STROKE_H */
