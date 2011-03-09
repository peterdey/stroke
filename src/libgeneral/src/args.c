/*
 *      args.c - Functions to handle variadic argument list as an alternative to 
 *               using va_list. This functionality has been mainly implemented due 
 *               to some general insatisfactions with the va_copy function.     
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

#include <libgeneral/general.h>
#include <libgeneral/args.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef STDC_HEADERS
#  if !HAVE_MEMCPY
#    define memcpy(d, s, n) bcopy ((s), (d), (n))
#  endif
#endif

/*
 * Settings and Adaptability Section:
 * Certain settings; what kind of functions to be used to
 * malloc and the like.
 */

/* Error prefix */
#define ARGS_ERROR_PREFIX "Internal error: "

/* Argument types. Character type_descr is similiar to a printf-like
 * argument specifier.
 */	
#define ARGS_ALLOWED "dscfph"
#define FRMT_ALLOWED "1234567890*$.#+-'Ilou"
struct {
	char type_descr;
	size_t tsize;
} types[] = {
	/* Invalid type */
	{0, 0},
	/* String */
	{'s', -1},
	/* Integer */
	{'d', sizeof(int)},
	/* Character */
	{'c', sizeof(char)},
	/* Floating point (Double) */
	{'f', sizeof(double)},
	/* Hex */
	{'h', sizeof(unsigned int)},
	/* Pointer */
	{'p', sizeof(void*)}
};

/* Argument indicator for format string. Usually '%'. */
const char argument = '%';

#define ARGS_MALLOC(FUNC)  void* (*my_malloc)(size_t) = FUNC
#define ARGS_REALLOC(FUNC) void* (*my_realloc)(void*, size_t) = FUNC
#define ARGS_PRINTF(FUNC)  int (*my_printf)(const char *, ...) = FUNC
	
/* Malloc and realloc functions to be used */
ARGS_MALLOC(&general_malloc);
ARGS_REALLOC(&general_realloc);
/* Output function */
ARGS_PRINTF(&msg);

/*
 * Code Section:
 * Implementations
 */

/**
 * Given a printf-like type specifier, this 
 * function returns the appropriate index within the types 
 * array (see declaration above).
 * If type given by character 'type' couldn't be found 0 
 * is returned, indexing the first (zeroed)-element of 
 * the types array.
 */
static int find_arg_type(char type) {
	int i = 1;
	for( ; i < ( (sizeof types) / (sizeof *types) ); i++ ) 
		if( type == types[i].type_descr ) return i;
	return 0;
}

/**
 * Copies a "type" from src to dest whereby
 * dest is newly allocated according to the size of the type 
 * that is implicitally given by the type-inidcating character
 * type. The size is defined in the types array (see declaration above).
 * NULL is returned if something went wrong, otherwise
 * a pointer to dest will be returned.
 * Notice that strings are handled specially.
 */
static void* type_copy(void *dest, void *src, char type) {
	size_t type_size;
	
	if ( !src ) {
		my_printf(ARGS_ERROR_PREFIX "Invalid source pointer to copy from\n");
		return NULL;
	}
	
	type_size = types[ find_arg_type(type) ].tsize;
	
	/* Strings are special as we don't indent to allocate heap memory
	   for a pointer to a string but for the string itself. */
	if ( type == 's' ) {
		type_size = strlen(*(char**)src) + 1;
		src = *(char**)src;
	} else if ( type_size < 1 ) return NULL;
	
	/* Allocate memory for argument */
	if( (dest = my_malloc(type_size)) == NULL ) return NULL;
	
	/* Copy memory */
	memcpy(dest, src, type_size);
	
	return dest;
} 

/**
 * Cleanup (free) an argument array
 */
void arg_array_destroy(ARG_ARRAY args) {
	struct arg **a = args;
	if( !args ) return;
	while( *a ) {
		free((*a)->argument);
		free(*a++);
	}
	free(args);
	args = NULL;
}


/**
 * Advances string pointer *str until any of the characters in strin chrs is
 * encountered in string pointed to by *str. 
 * While advancing all of the characters given in allow
 * are allowed to be encountered in str. NULL is returned if this rule
 * is violated or if *str points to the end of the string str. In these cases
 * *str remains ("gets stuck") at its position in str. Because
 * we advance the pointer *str itself a pointer to this pointer is passed
 * as the first argument of this function. 
 */
char* strchrstrallow(char **str, const char* chrs, const char* allow) {
#define PSET(NAME, TYPE) NAME##_p = (TYPE*)NAME
#define TRAV(STR1, STR2, ACTION) while( *STR1 ) if( *STR1++ == *STR2 ) ACTION
	
	char *chrs_p, *allow_p;
	
	if( !*str || !chrs || !allow ) return NULL;
	
	for( ; **str; (*str)++ ) {
			PSET(chrs, char), PSET(allow, char);
			TRAV(chrs_p, *str, return *str);
			TRAV(allow_p, *str, goto contin);
			break;
			contin: continue;
	}
	
	return NULL;
}

/**
 * Returns a variadic argument array from the format string (frm)
 * and the variable argument list vl passed.
 * For the types supported as arguments in the format string see
 * types array declararion a the top of this file.
 * The arg array returned may be NULL if none could be created due 
 * to the lack of arguments or any other failure.
 */
ARG_ARRAY varg_to_argarr(const char *frm, va_list vl) {
	ARG_ARRAY args = NULL;
	char *p = (char*)frm;
	size_t arr_size = 0;
	/* Variables needed to extract from va_list */
	void *next_arg = NULL;
	int int_arg;
	double float_arg;
	char *str_arg;
	
	while( *p ) {
		if( *p++ == argument && *p != argument ) {
			
		/* Skip formatting and advance till argument specifier */
		if( !strchrstrallow(&p, ARGS_ALLOWED, FRMT_ALLOWED) ) {
			my_printf(ARGS_ERROR_PREFIX "Undefined argument given in format string: '%c'\n", *p);
			continue;
		}

		/* Enlarge argument array */
		if( (args = (ARG_ARRAY)my_realloc((void*)args, (arr_size+2) * sizeof(struct arg*))) == NULL ) {
			my_printf(ARGS_ERROR_PREFIX "Not enough memory for argument array.\n");
			return NULL;
		}	
		
		/* Allocate memory for argument itself */
		if( (args[arr_size] = my_malloc(sizeof(struct arg))) == NULL ) {
			my_printf(ARGS_ERROR_PREFIX "Insufficient memory for argument: '%c'\n", *p);
			args[arr_size] = NULL;
			return args;
		}		
		
		/* Set argument type */
		args[arr_size]->argument_type = *p;
		
		/* Handle argument type properly; extracting from va_list can be somewhat cumbersome */
		switch( *p ) {
			/* Fallthrough due to standard promotion of types */
			case 'd':
			case 'h':
			case 'c':
				int_arg = va_arg(vl, int);
				next_arg = &int_arg;
			break;
			case 'f':
				float_arg = va_arg(vl, double);
				next_arg = &float_arg;
			break;
			case 's':
			case 'p':
				str_arg = va_arg(vl, char*);
				next_arg = &str_arg;
			break;
		}
		
		/* Copy argument itself */
		if( (args[arr_size]->argument = type_copy(args[arr_size]->argument, next_arg, *p))
		     == NULL ) {
			my_printf(ARGS_ERROR_PREFIX "Unable to copy argument: '%c'\n", *p);
			free(args[arr_size]);
			args[arr_size] = NULL;
			return args;
		} 
	
		/* NULL-terminate arguments array */
		args[++arr_size] = NULL; 	
	}
	}
	
	return args;
}
