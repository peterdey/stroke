/*
 *      errors.c - stroke error handling data and debugging routines
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

#include "errors.h"

#include "stroke.h"

#include <libgeneral/error.h>
#include <libgeneral/debug.h>

ERROR_MESSAGE error_messages[] = {
	
	/* Fatal errors */
	EM_INIT(ERROR_FATAL_SEGV,  "Segmentation Fault at %p\n"
				   "Please report this bug to <"PACKAGE_BUGREPORT">"),
	/* Warnings */
	EM_INIT(ERROR_WARNING_FORCVAL, "Date validations skipped"),
	EM_INIT(ERROR_WARNING_CTCOPY, "Change time was not copied because root or CAP_SYS_TIME privileges are required"),

	/* Normal errors */
	EM_INIT(ERROR_ERROR_INSUFARGS, "Insufficient command line arguments supplied"),
	EM_INIT(ERROR_ERROR_UKNARG, "Unrecognized option: `%s'"),
	EM_INIT(ERROR_ERROR_MODFIL, "Reference file, time stamp or file argument missing"),
	EM_INIT(ERROR_ERROR_INVMOD, "Invalid modifier(s) encountered"),
	EM_INIT(ERROR_ERROR_INVFIL, "Invalid file argument"),
	EM_INIT(ERROR_ERROR_STAT, "Unable to retrieve file information for:\n\"%s\" %s"),
	EM_INIT(ERROR_ERROR_GMTIM, "Unable to retrieve time information for: \"%s\""),
	EM_INIT(ERROR_ERROR_VALDAT, "Date validation failed: \"%s\""),
	EM_INIT(ERROR_ERROR_TSTMP, "Cannot create time stamp"),
	EM_INIT(ERROR_ERROR_RESOLV, "Failing to resolve right-hand identifier `%s' to\n"
		"time value invalidates modifier expression `%s'"),
	EM_INIT(ERROR_ERROR_SETVAL, "Erroneous partial assignment `%s=%d'\ninvalidates"
		" modifier expression `%s'"),
	EM_INIT(ERROR_ERROR_SETTIM, "Setting modification and access time for\nfile \"%s\" failed"),
	EM_INIT(ERROR_ERROR_INSUFA, "Invalid assignment `%s' encountered"),
	EM_INIT(ERROR_ERROR_FOPEN, "Unable to open file: \"%s\""),
	EM_INIT(ERROR_ERROR_TOOMA, "Too many command line arguments"),
	EM_INIT(ERROR_ERROR_MFIND, "Failing to resolve identifier `%s' for modulation `%s+=%d'\n"
		"invalidates modifier expression `%s'"),
	EM_INIT(ERROR_ERROR_MINVAL, "Erroneous modulator `%s' invalidates\n"
		"modifier expression `%s'"),
	EM_INIT(ERROR_ERROR_GETTD, "Unable to obtain current time"),
	EM_INIT(ERROR_ERROR_BATCHF, "Batch file name argument missing"),
	EM_INIT(ERROR_ERROR_TIMEST, "Invalid time stamp or selector `%s:%s'"),
	EM_INIT(ERROR_ERROR_INVTSP, "Invalid time stamp expression `%s'"),
	EM_INIT(ERROR_ERROR_UTIMSYM, "Cannot change symlink modification or access time: \"%s\""),
	EM_INIT(ERROR_ERROR_CHCTIME, "Altering change time failed:\n\"%s\" %s"),
	EM_INIT(ERROR_ERROR_CTPRES, "`-c' and `-p' must not be given together"),
	EM_INIT(ERROR_ERROR_CTCHPR, "Attempt to alter change time despite `-preserve'"),
	EM_INIT(ERROR_ERROR_INVCOMB, "Invalid option in combination with `-i, -info'"),
	EM_INIT(ERROR_ERROR_INFMODF, "Modifier expression list given together with `-i, -info'"),
	EM_INIT(ERROR_ERROR_FCREATE, "Unable to create file: \"%s\""),
	EM_INIT(ERROR_ERROR_CTPRIV, "Option `%s' requires root or CAP_SYS_TIME privileges"),
	EM_INIT(ERROR_ERROR_SETTIM_PERM, "Insufficient permissions to modify: \"%s\""),
	
	ZERO_SENTINEL
};

#ifdef DEBUG
/*
 * Dump internal time_vals array of time value structures
 */
static void dump_time_vals(FILE_TIME (*time_vals)[TIME_VALS])
{
	int i = TIME_TBLS;
	register int j;
	
	while(--i >= 0) {
		msg("Dumping %s:", names[i]);
		for(j = 0; j < TIME_VALS; j++) {
			dumpstr(STR, "name", 7, IFF(time_vals[i][j].name, "wkd"));
			dumpstr(INT, "val", 7, &time_vals[i][j].val);
		}
	}
}

/*
 * Dump tv if debugging and verbosity allowed.
 */
inline void dump_tv(FILE_TIME (*time_vals)[TIME_VALS])
{
	if(verbosity_level()) dump_time_vals(time_vals);
}

#endif
