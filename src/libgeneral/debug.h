/*
 *      debug.h - libgeneral debugging routines
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

#ifndef LIBGENERAL_DEBUG_H
#define LIBGENERAL_DEBUG_H


/// INCLUDE CONFIG for DEBUG
#ifdef DEBUG

# include <libgeneral/general.h>
# include <libgeneral/args.h>
# include <libgeneral/error.h>

/*
 * Type of dump data
 */
typedef enum {
	STR, INT, HEX, BOL, CHR, PTR
} DUMP_TYPE;

/*
 * Dumping functions
 */

/* Dump string of the form: "prog_name  <dump_data address> (dump_name)   <space>  dump_data" */
extern void dumpstr(DUMP_TYPE type, const char dump_name[], int space, const void *dump_data);
extern void dump_arg_list(ARG_ARRAY l);
extern void dump_error_message(ERROR_MESSAGE *e);

#endif

#endif /* LIBGENERAL_DEBUG_H */
