/*
 *      error.h - General error handling routines
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

#ifndef LIBGENERAL_ERROR_H
#define LIBGENERAL_ERROR_H 1

#include <stdlib.h>
#include <stdarg.h>

#include <libgeneral/general.h>
#include <libgeneral/args.h>

/* Max number of optional arguments that are passed to an error message */
#define MAX_ERR_ARGS 20

/* Shorten the file/line combination passed to error_out and error_store */
#define FLN __FILE__, __LINE__

/**
 * Types of errors
 */
typedef enum {
	/* Normal errors and warnings will simply be output whereas fatal errors
	 * will cause the application to be abnormally aborted immediately.
	 */
	ERROR_FATAL,
	ERROR_ERROR,
	ERROR_WARNING
} ERROR_TYPE;

/* Numeric boundaries (ranges) for types of error messags 
 */
#define FATAL_BOUND 100
#define WARNING_BOUND 200

#define _MAX_ERRORMSG_LEN 1024

typedef struct {
	
	/* Error code; see enum above */
	int code;

	/* Error message */
	char msg[_MAX_ERRORMSG_LEN];
	
	/* Only used for errors that are stored on an error stack in order to remember error
	 * location as well as arguments passed to the error message.
	 */
	struct error_info {
		/* File where error occurred */
		char *file;
		/* Line where error occurred */
		int line;
		/* Error number (may be 0 for none) that was returned */
		int err_no;
		/* Arguments passed to error */
		ARG_ARRAY args;
	} *err_info; 
	
	/* Error handler function called when error is output */
	void (*error_handler)(void *);
	
} ERROR_MESSAGE;

/* Initialization macros for messages[] array of type ERROR_MESSAGE */
#define ERR_INFO_INIT NULL 
#define ZERO_SENTINEL {0, "\0", ERR_INFO_INIT, NULL} 
#define EM_INIT(ERRCODE, MSG) {ERRCODE, MSG, ERR_INFO_INIT, NULL}

/*
 * Code of the last error that occurred.
 */
extern int last_error_code;

/*
 * Number of error and warnings.
 */
extern int error_cnt;
extern int warning_cnt;


/* Prototypes */
extern void error_set_malloc(void *(*mal)(size_t size));
extern void libgeneral_init_errors(ERROR_MESSAGE  (*message_array)[], int num_error_stacks);
extern void libgeneral_uninit_errors();
extern char* store_string(const char *str);
extern void clean_error_message(ERROR_MESSAGE *e);
extern void error_out(int code, int errno_err, const char *file, const int line, ...);
extern void errors_out(int error_stack_num);
extern void error_store(int error_stack_num, int code, int errno_err, const char *file, const int line, ...);
extern void errwrn(ERROR_TYPE type, int errno_err, const char *file, const int line, char *err, ...);

#endif /* LIBGENERAL_ERROR_H */
