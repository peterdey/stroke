/*
 *      args.h - Dealing with variable argument list as an alternative to va_list
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

#ifndef LIBGENERAL_ARGS_H
#define LIBGENERAL_ARGS_H 1

#include <stdarg.h>

/*
 * Type for variable argument list. See struct specification below.
 * And ARG_ARRAY thus, is just an array of pointers to arg structure instances.
 */
typedef struct arg** ARG_ARRAY;

/* An error of pointers, where each of the pointers in the
 * array points to the following structure is regarded as
 * a variable argument list.
 *
 * So for instance `struct arg **a` would be variable argument list.
 * The last pointer of this array points to NULL. 
 */
struct arg {
	/* Represents a printf like conversion
	 specification character like 'd', 's', or 'c' */
	char argument_type;
	void *argument;
};

/* Prototypes */
extern ARG_ARRAY varg_to_argarr(const char *frm, va_list vl);
extern void arg_array_destroy(ARG_ARRAY args);

#endif /* LIBGENERAL_ARGS_H */
