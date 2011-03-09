/*
 *      debug.c - libgeneral debugging routines
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
# include <config.h>
#else
# include <string.h>
#endif

#ifdef DEBUG

# include <libgeneral/debug.h>

# include <libgeneral/general.h>
# include <stdio.h>
# include <assert.h>

/*
 * Auxiliary macros
 */
#define ARG_DUMP(A) dumpstr(A, "*a->argument", 25, (*al)->argument);

	
/** 
 * General dump string.
 * Dump name is the string up front, space ist the space between the dump_name and the
 * data being dumped.
 */
void dumpstr(DUMP_TYPE type, const char dump_name[], int space, const void *dump_data)
{
	int i;
	
	assert(space > strlen(dump_name));
	
	printf("%s:     %p: ", prog_name, (void*)dump_data); 
	printf("(%s)", dump_name);
	
	for(i = 0; i < space-strlen(dump_name); i++)
		putchar(' ');

	switch(type) {
		case STR:
			printf(": \"%s\"\n", (char*)dump_data);
		break;
		case INT:
			printf(": %d\n", *((int*)dump_data));
		break;
		case HEX:
			printf(": %#08x\n", (unsigned int)dump_data);
		break;
		case BOL:
			printf(": %s\n", *((int*)dump_data) ? "true" : "false");
		break;
		case CHR:
			printf(": %c\n", *(char*)dump_data);
		break;
		case PTR:
			printf(": %p\n", (void*)dump_data);
		break;
	}
}


/**
 * Dumping a variable argument strucutre.
 * See args.h for mor information.
 */
void dump_arg_list(ARG_ARRAY l)
{
	struct arg **al = l;
	
	if( !al ) {
		msg(" Dump not possible, argument list == NULL\v");
		return;	
	}
	
	msg(" ** arg_list_dump DUMP: (0x%h)\v",  al);

	for( ; *al; al++ ) {

		msg(" next argument DUMP:\v");
		dumpstr(CHR, "*a->argument_type", 25, &(*al)->argument_type);

		switch( (*al)->argument_type ) {
			case 's':
				ARG_DUMP(STR);
			break;
			case 'd':
				ARG_DUMP(INT);
			break;
			case 'c':
				ARG_DUMP(CHR);
			break;
		}
	}
	
	msg(" ** end arg_list dump\v");
}

/**
 * Dumping an error message structure.
 * See error/error.h for more information.
 */
void dump_error_message(ERROR_MESSAGE *e)
{
	struct error_info *ei;
	
	assert(e != NULL);
	
	/* Dumping struct ERROR_MESSAGE */
	msg("  error_message dump (%p)\v", e);
	dumpstr(INT, "e->code", 18, &e->code);
	dumpstr(STR, "e->msg",  18, e->msg);
	
	/* Dumping struct error info */
	if( (ei = e->err_info) == NULL ) {
		msg("  -- no error information: (null)\v");
		return;
	}
	msg("    error_info: (0x%h)\v", e->err_info);
	dumpstr(STR, "ei->file",   18, ei->file);
	dumpstr(INT, "ei->line",   18, &ei->line);
	dumpstr(INT, "ei->err_no", 18, &ei->err_no);
	
	/* Dumping struct args */
	if( ei->args == NULL ) {
		msg("  -- no arguments for message: (null)\v");
		return;
	}
	
	dump_arg_list(ei->args);
}

#endif /* DEBUG */
