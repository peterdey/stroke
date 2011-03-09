/*
 *      error.c - General error handling routines
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <libgeneral/error.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

#include <libgeneral/general.h>
#include <libgeneral/stack.h>
#include <libgeneral/args.h>

/* 
 * All an application's error messages.
 * An application needs to define a specified a reference to this
 * array when calling the error init function.
 */ 
static ERROR_MESSAGE (*_messages)[];

static void* (*error_malloc)(size_t);

void error_set_malloc( void* (*mal) (size_t size) ) {
	error_malloc = mal;
}

/* For systems that do not have the strerror function */
#ifdef HAVE_STRERROR // FIX
char* strerr(int errno) {
   extern int sys_nerr;
   extern char *sys_errlist[];
   if (errno < 0 || errno > sys_nerr)
           return "Error Unknown";
   else
           return sys_errlist[errno];
}
#endif

#define _ERROR_STRING_FATAL    "FATAL"
#define _ERROR_STRING_ERROR    "ERROR"
#define _ERROR_STRING_WARNING  "WARNING"

/* Indentifying a printf-like parameter specifies */
#define IS_PARAM(P) (*P++ == PARAM_INDICATOR && *p != _PARAM_INDICATOR)

/*
 * Code of the last error that occurred.
 */
int last_error_code;

/*
 * Errors and warnings counted 
 */
int error_cnt;
int warning_cnt;

/* General error printing and formatting function */
static void verror(ERROR_TYPE, int, const char*, const int, char *, ARG_ARRAY, GENERAL_BOOL);

/**
 * An array of error stacks.
 * Upon initialization of the error functionality,
 * the application specifies how many error stacks it needs.
 */
static STACK **error_stack;

/**
 * Creates a fatal error when failing.
 * Fails and exists application on an error.
 */
void libgeneral_init_errors(ERROR_MESSAGE (*message_array)[], int num_error_stacks) {
	int i;
	
	if( !message_array ) {
		errwrn(ERROR_FATAL, 0, FLN, 
		LIBGENERAL_ERROR_PREFIX "Invalid array of error messages given.");
		abort();
	}

	error_malloc = &general_malloc;
	_messages = message_array;

	if( num_error_stacks > 0 ) {
		error_stack = (STACK **)error_malloc((num_error_stacks+1) * sizeof(STACK*));
		for( i = 0; i < num_error_stacks; i++ ) { 
			error_stack[i] = stack_new();
			error_stack[i]->type = ERROR_STACK;
		}
		
		error_stack[num_error_stacks] = NULL;
	}
	
	last_error_code = error_cnt = warning_cnt = 0;
}

/**
 * Cleanup after having used error stacks.
 */
void libgeneral_uninit_errors() {
	STACK **s = error_stack;
	
	/* Cleanup error stacks */
	if( !error_stack ) return;

	if( *error_stack ) {
		while( *s ) {
			stack_destroy(&*s++);
		}
	}
	
	free(error_stack);	
}

/**
 * Is passed an error code and will find and return
 * a pointer to this error message in the global 
 * messages array. Returns NULL if couldn't be found and outputs 
 * an error message.
 */
static ERROR_MESSAGE* find_error(int error_code) {
	ERROR_MESSAGE *m = &(*_messages)[0];
	
	for( ;*m->msg; m++ ) 
		if( m->code == error_code ) 
			return m;
	
	errwrn(ERROR_FATAL, 0, FLN, 
	LIBGENERAL_ERROR_PREFIX "Error code %d not found\n", error_code);
	
	return NULL;
}

/**
 * Determine the type of an error (usually from a set of 3) according
 * to the boundaries given in error.h
 */
static ERROR_TYPE error_type(int err_code) {
	ERROR_TYPE t;
	
	/* Determine type of error */
	if( err_code < FATAL_BOUND ) 
		t = ERROR_FATAL;
	else if( err_code < WARNING_BOUND )
		t = ERROR_WARNING;
	else 
		t = ERROR_ERROR;
	
	return t;
}


/**
 * Output error with error code.
 * For the actual output verror() is used, this function does only
 * do the formatting. Give format arguments as needed by message.
 * NOTE: It is assumed that valid code and the correct number
 * of arguments are passed. No checks will be performed.
 */
void error_out(int code, int errno_err, const char *file, const int line, ...) {
	ERROR_MESSAGE *m;
	ARG_ARRAY args;
	va_list l;
	
	va_start(l, line);
	m = find_error(code);
	
	last_error_code = code;
	
	verror(error_type(code), errno_err, file, line, m->msg,
	       (args = varg_to_argarr(m->msg, l)),
	       libgeneral_check_flag(OPTION_ERROR_CODE_ON_ERROR));
	
	arg_array_destroy(args);
	
	va_end(l);
}

/**
 * Returns true if error is on error stack given by name.
 */
GENERAL_BOOL on_error_stack(int err_stack_num, int code) {
	STACK *s = error_stack[err_stack_num];
	STACK_ELEMENT *e = s->top;
	
	/* Traverse stack of error messages */
	for( ; e; e = e->next )
		if( (*((ERROR_MESSAGE*)e->data)).code == code )
			return TRUE;
			
	return FALSE;
}

/**
 * Stacks an error message onto the error stack, given by its representing number, 
 * onto the error_stack array.
 */
void error_store(int error_stack_num, int code, int errno_err, const char *file, const int line, ...) {
	ERROR_MESSAGE *e;
	va_list l;
	/* Check if already stored */
	if( on_error_stack(error_stack_num, code) ) return;
	
	va_start(l, line);
	
	e = find_error(code);
	
	e->err_info = error_malloc(sizeof(struct error_info));  // FREE() //
	
	/* Initializations */
	e->err_info->args = NULL;
	
	/* Fill err_info structure in order to record were error hailed from and what arguments 
	 * were passed to it.
	 */
	
	/* Set file, line, errno, and arglist from where error occurred */
	if( file ) e->err_info->file = cpy_string(file);
	e->err_info->line = line;
	e->err_info->err_no = errno_err;
	
	/* Fill args structure to preserve arguments passed as well as their type */
	e->err_info->args = varg_to_argarr(e->msg, l);
	
	/* Push error onto error stack specified */
	stack_push(error_stack[error_stack_num], (void*)e);	
	
	//if(verbosity_check(VERBOSE_HIGH)) dump_stack(error_stack[error_stack_num], 0);
	
	va_end(l);
}


/**
 * Cleans up an error message back to initialization state.
 * This means that everything that was allocated for this 
 * error message (like stuff in the err_info structure or error_info itself) 
 * will get free'd.
 */
void clean_error_message(ERROR_MESSAGE *e) {
	/* Cleanup error information structure */
	if( e->err_info ) {
		arg_array_destroy(e->err_info->args);
		if( e->err_info->file )
			free(e->err_info->file); 
		free(e->err_info);
	}
	e = NULL;
}


/**
 * Outputs errors stored on an error stack.
 * Error stack will be invalid afterwards.
 * All malloc'd resources will have been free'd too.
 */
void errors_out(int error_stack_num) {
	STACK *estack = error_stack[error_stack_num];
	ERROR_MESSAGE *m;
	
	/* Retrieve stacked error messages, poperrors_out, and output */
	for( ; estack->count > 0; stack_pop(estack, 1) ) {
		m = (ERROR_MESSAGE*)estack->top->data;
		/* Sanity check */
		if( !m->msg || !*m->msg ) {
			errwrn(ERROR_FATAL, 0, FLN, 
			LIBGENERAL_ERROR_PREFIX "Unable to print message with error code %d\n", m->code);
			return;
		}
	
		last_error_code = m->code;
	
		/* Call error printing function for stacked error */
		putchar('\n');
		verror(error_type(m->code), m->err_info->err_no, m->err_info->file, 
		m->err_info->line, m->msg, m->err_info->args, TRUE);
	}
	
}

/**
 * Print an error message to stderr.
 */
static void verror(
	ERROR_TYPE type, 
	int errno_err, 
	const char *file, 
	const int line, 
	char *err, 
	ARG_ARRAY args,
	GENERAL_BOOL error_code) 
{
	char prfx[127], msgtype[64], errm[1024], *strerr;
	
	/* If option quiet set do not output anything */
	if( libgeneral_check_flag(OPTION_QUIET) ) return;
	
	*msgtype = 0;
	
	switch(type) {
		case ERROR_FATAL:
			strcpy(msgtype, _ERROR_STRING_FATAL);
		break;
		case ERROR_ERROR:
			strcpy(msgtype, _ERROR_STRING_ERROR);
			++error_cnt;
		break;
		case ERROR_WARNING:
			strcpy(msgtype, _ERROR_STRING_WARNING);
			++warning_cnt;
		break;
	}
	
	strcpy(errm, err);
	sprintf(prfx, "%s: ** %s", prog_name, msgtype);
	
	if(libgeneral_check_flag(OPTION_ERRORS_POINT_TO_SOURCE)) { 
		if(file && line)
			sprintf(prfx, "%s (%s:%d)", prfx, basename((char*)file), line);
	}
	
	if(errno_err != 0) {
		strerr = strerror(errno_err);
		sprintf(errm, "%s\n(%s)", errm, strerr);
		errno = 0;
	} 
	
#define ECODE_STRING(TYPE) ((TYPE == ERROR_ERROR || TYPE == ERROR_FATAL) ? ("Error code") : ("Code"))
	
	visual_spacing(1);
	_prfx_print_args(stderr, prfx, errm, args);
	if(error_code) {
		putchar('\n');
		_nv_prfx_print(stderr, prfx, "[%s: %d]", ECODE_STRING(type), last_error_code);
	}
	putchar('\n');

	/* Exit abruptly if a fatal error occurred */
	if(type == ERROR_FATAL) {
		arg_array_destroy(args);
		error_cnt = -1;
		fflush(stdout);
		fflush(stderr);
#ifndef DEBUG		
		exit(last_error_code);
#else
		abort(); /* Backtrace */
#endif		
	}
	
	/* Reset errno */
	if(errno_err) errno = 0;
}

/**
 * Error routine to output errors not registeres in the array
 * of error messages. Think of this as a "raw error" function.
 * It is used by libgeneral to report internal errors.
 */
void errwrn(
	ERROR_TYPE type, 
	int errno_err,
	const char *file,
	const int line,
	char *err, ...) 
{
	va_list ap;
	ARG_ARRAY arg_ar = NULL;
	va_start(ap, err);
	verror(type, errno_err, file, line, err, (arg_ar = varg_to_argarr(err, ap)), 0);
	arg_array_destroy(arg_ar);
	va_end(ap);
}
	
