/*
 *      general.h - General program functionality
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

// Todo: use __attribute__ where possible, like msg() ..
// compatibility: redefining __attribute__ to nothing


#ifndef LIBGENERAL_GENERAL_H
#define LIBGENERAL_GENERAL_H 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

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

#include <libgeneral/args.h>

#define LIBGENERAL_VERSION 0.1

/***********************
 * General definitions *
 ***********************/
typedef enum {
	FALSE = 0, TRUE = 1
} GENERAL_BOOL;


/*******************
 * Library options *
 *******************/
#define _FLAG(N) ((_FLAG_TYPE)(1 << N))
#define _FLAG_TYPE uint32_t

extern _FLAG_TYPE libgeneral_flags;

enum {
	OPTION_QUIET = _FLAG(0),
	OPTION_ERRORS_POINT_TO_SOURCE = _FLAG(1),
	OPTION_VERBOSE_SHOW_LEVEL = _FLAG(2),
	OPTION_ERROR_CODE_ON_ERROR = _FLAG(3),
};

/*********************
 * Auxiliary macros  *
 *********************/
#define GMALLOC(TYPE, SIZE) (TYPE)general_malloc(SIZE);
#define ABS_VALUE(X) ((X>0) ? (X) : (-X))
#define X_OR_ZERO(X) ((X>0) ? (X) : (0))
#define IFF(X, Y) ((X) ? (X) : (Y))
#define IFSTR(COND, STR) ((COND) ? (STR) : (""))
#define STRFY(X) #X
#define TOSTR(X) STRFY(X)


/********************
 * Public variables *
 ********************/

/* Name of program libgeneral is used for */
extern char *prog_name;

/* Libgeneral infomration string; not used as of now  */
extern const char *libgeneral_vinfo;

/* Libgeneral version string */
extern const char *libgeneral_version;

/************************
 * Interface functions  *
 ************************/

/* Library interfacing */
extern void libgeneral_init(const char *, _FLAG_TYPE);
extern void libgeneral_uninit();
extern void libgeneral_set_flag(_FLAG_TYPE flag);
extern void libgeneral_unset_flag(_FLAG_TYPE flag);
extern void libgeneral_init_verbose(int (*verbose_func)(), const char *prefix, int max_level);
extern GENERAL_BOOL libgeneral_check_flag(_FLAG_TYPE flag);

/* Memory managment */
extern void* general_malloc(size_t size);
extern void* general_realloc(void *ptr, size_t size);

/* Misc */
extern int msg(const char *msg, ...);
extern void verbose(int vlevel, const char *msg, ... );
extern void visual_spacing(short space);
extern char* cpy_string(const char *str);
extern char* readline_stream(FILE *s);
extern char* new_str(const char *str);
extern int timer(struct timeval *elapsed);

/*******************************
 * Functions for internal use  *
 *******************************/
extern void _prfx_print_args(FILE *stream, const char *prfx, const char *frm, ARG_ARRAY args);
extern void _prfx_print(FILE *stream, const char *prfx, const char *frm, va_list ap);
extern void _nv_prfx_print(FILE *stream, const char *prfx, const char *frm, ...);

/********************
 * Internal  macros *
 ********************/
#define LIBGENERAL_ERROR_PREFIX "Libgeneral: "
#define _VSPACING(VLIST, MSG, DEFAULT) \
		char vspace[] = {VSPACE};\
		if(*MSG == *vspace) {\
			MSG++;\
			visual_spacing(va_arg(VLIST, int));\
		} else visual_spacing(DEFAULT);
#define VSPACE "\f"


#endif /* LIBGENERAL_GENERAL_H */
