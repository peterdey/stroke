/*
 *      stroke.c - Main source file
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libgeneral/general.h>
#include <libgeneral/error.h>
#include <libgeneral/signals.h>

#include "stroke.h"
#include "errors.h"


/***************
 * Information *
 ***************/

const char *usg =
	"Usage: "PROGRAM" [OPTION]... ARGUMENT FILE...\n"
	"   or: "PROGRAM" [OPTION]... FILE...\n\n"
	"Meaning of ARGUMENT may differ according to OPTION set.\n\n"
	"      Option:   -s             ARGUMENT = STAMP\n"
	"                -r or -b       ARGUMENT = FILE\n"
	"                otherwise      ARGUMENT = MODIFIERS\n\n"
	"Options:\n"
	"      -s, -stamp         Treat ARGUMENT as a STAMP (see below).\n"
	"      -r, -reference     Treat ARGUMENT as a reference file and set FILE's\n"
	"                          access, modification, (and change) time to that of\n"
	"                          reference.\n"
	"      -l, -symlinks      If FILE or reference file (in case of `-r') refer\n"
	"                          to a symbolic link, read access, modification and\n"
	"                          change time from that link."
#if defined(HAVE_LUTIME) || defined(HAVE_UTIMENSAT)
	" Also, if FILE refers\n"
	"                          to a symbolic link, alter that link's modification,\n"
	"                          access, (and change) time.\n"
#else
	"\n"
#endif
	"      -c, -ctime         Do also apply change time alterations. This might\n"
	"                          require root privileges.\n"
	"      -p, -preserve      When altering modification or access time, prevent\n"
	"                          change time from being implicitly altered too.\n"
	"                          Root privileges might be required.\n"
	"      -i, -info          Output information about a file's access, change,\n"
	"                          and modification time.\n"
	"      -b, -batch         Treat ARGUMENT as a batch file to read modifier\n"
	"                          expressions from.\n"
	"      -f, -force         Do not perform any date or time validations.\n"
	"      -v, -verbose       Generate more verbose output.\n"
	"      -q, -quiet         Do not generate any output, not even for errors;\n"
	"                          only return value indicates exit status.\n"
	"      -h, -help          Print this help text\n"
	"      -version           Print program information\n\n"
	"Modifers:\n"
	"      MODIFIERS is a `,' or `;' separated list of modifier EXPRESSIONs:\n\n"
	"                     EXPRESSION [,|; EXPRESSION]...\n\n"
	"      An EXPRESSION is a simple or chained assignment of the form:\n\n"
	"         IDENTIFIER [= IDENTIFIER]... = VALUE | IDENTIFIER | MODULATOR\n\n"
	"      All IDENTIFIERs of an EXPRESSION are evaluated as if they were\n"
	"       singly assigned the right-most statement.\n\n"
	"      A VALUE is a positive integer value that is appropriate for an\n"
	"       assignment's IDENTIFIER as it is.\n\n"
	"      There are IDENTIFIERs for all components of access, modification,\n"
	"       and change time:\n"
	"                   [mac]Y     Year             [mac]h     Hour\n"
	"                   [mac]M     Month            [mac]m     Minute\n"
	"                   [mac]D     Day              [mac]s     Second\n"
	"                   [mac]l     Dst (0..2)\n\n"
	"      MODULATORs conveniently increment or decrement an IDENTIFIER's\n"
	"      value:\n"
	"                  +n     Increment by n       ++     Increment by 1\n"
	"                  -n     Decrement by n       --     Decrement by 1\n\n"
	"      If MODIFIERS is `-', modifier expressions shall be read from stdin.\n\n"
	"Time stamps:\n"
	"      STAMP must be supplied as ARGUMENT if `-s, -stamp' is given. STAMP\n"
	"      is of the form:\n\n"
	"                  [[CC]YY]MMDDhhmm[.ss][:SELECTOR]\n\n"
	"      SELECTOR specifies whether access time, modification time, change time\n"
	"       or any of those combined should be altered using STAMP.\n"
	"       A SELECTOR is of the form: a, m, c (or any unique combination thereof).\n"
	"       If not given as part of STAMP, SELECTOR defaults to m.\n\n"
	"Examples:\n"
	"       "PROGRAM" mY=aY=2008,mm=+2,am=mm <file>\n"
	"       "PROGRAM" -c cY=mY,cM=mM,cD=mD <file>\n\n"
	"Consult the manual page `stroke (1)' for more detailed information and\n"
	" invocation examples.\n\n"
	"\nPlease help by reporting bugs to <"PACKAGE_BUGREPORT">."
	"\n\n";

const char *pinf[] =
	{"Dynamically altering modification, access, and change time components",
	 "GPL", "2009, 2010, 2011", "Soeren Wellhoefer (soeren.wellhoefer@gmx.net)"};

const char *infostr =
	"\nGNU "PROGRAM" "VERSION" (libgeneral version %s)\n\n"
	"Copyright (C) %s Free Software Foundation, Inc.\n"
	"This is free software; see the source for copying conditions.  There is NO\n"
	"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n"
	"Written by %s\n\n";

const char *bugs =
	"Please help by reporting bugs to <" PACKAGE_BUGREPORT ">.\n";


/***************
 *   Globals   *
 ***************/

FILE_TIME time_vals[][TIME_VALS] =
{	
	/* mtime: year, month, day, hour, minute, second, daylight saving time, weekday */
	{ T("mY"), T("mM"), T("mD"), T("mh"), T("mm"), T("ms"), T("ml"), WD },
	/* atime */
	{ T("aY"), T("aM"), T("aD"), T("ah"), T("am"), T("as"), T("al"), WD },
	/* ctime */
	{ T("cY"), T("cM"), T("cD"), T("ch"), T("cm"), T("cs"), T("cl"), WD }
};

const char *names[] = {"mtime", "atime", "ctime"};

/* Flags set through cmd line args */
_FLAG_TYPE flags;


/***************
 *  Functions  *
 ***************/	

/*
 * Reads time information for file and write them to global
 * time_vals array. If file is NULL the current time is taken instead.
 * Returns 0 on success, -1 on failure.
 */
static int
scan(const char *file)
{
	int (*statf)(const char*, struct stat*);
	struct timeval tv;
	struct stat st;
	struct tm *ts;

	if(!file) {
		if(gettimeofday(&tv, NULL) < 0) {
			error_out(ERROR_ERROR_GETTD, errno, FLN);
			return -1;
		}
		st.st_mtime = st.st_atime = st.st_ctime = tv.tv_sec;
	} else {
		statf = CHKF(SYMLINKS) ? &lstat : &stat;
		if((*statf)(file, &st) < 0) {
			error_out(ERROR_ERROR_STAT, errno, FLN, file,
				  IFSTR(!laccess(file, F_OK),
					"Dangling symbolic link? Try `-l'."));
			return -1;
		}
	}

	/* mtime */
	if(!(ts = localtime(&st.st_mtime))) goto gmfail;
	translate(ts, time_vals, MTIME, TO_FT);

	/* atime */
	if(!(ts = localtime(&st.st_atime))) goto gmfail;
	translate(ts, time_vals, ATIME, TO_FT);

	/* ctime */
	if(!(ts = localtime(&st.st_ctime))) goto gmfail;
	translate(ts, time_vals, CTIME, TO_FT);

	return 0;

 gmfail:
	error_out(ERROR_ERROR_GMTIM, errno, FLN, file);
	return -1;
}


/*
 * Evaluate modifier list and set values in time_vals table.
 * Return 0 on success, -1 on failure; issues error messages.
 */
static int
eval(const char **modifier_list)
{
	const char *modifier, *idf, *prem, **mtokens;
	int value, modulator;
	register int t;

	while((modifier = *modifier_list++)) {

		verbose(1, VSPACE "Evaluating modifier expression `%s'",  0, modifier);
		     
		mtokens = (const char**)sep_to_array("=", modifier);
		for(t = 0; mtokens[t]; t++);
		modulator = 0;
		
		if(t-- < 2) {
			error_out(ERROR_ERROR_INSUFA, 0, FLN, modifier);
			goto error;
		}

		/* Right-hand expression: value, modulator, or identifier */
		if(isnum(mtokens[t])) {			
			value = atoi(mtokens[t]);
#ifdef DEBUG			
			verbose(1, VSPACE "Right-hand value is %d", 0, value);
#endif			
		}
		else if((prem = strchr("+-", *mtokens[t]))) {
			if(isnum(mtokens[t]+1))
				modulator = atoi(mtokens[t]);
			else if(*(mtokens[t]+1) == *prem) {
				if(*prem == '+')
					modulator = 1;
				else
					modulator = -1;
			}
			else {
				error_out(ERROR_ERROR_MINVAL, 0, FLN, mtokens[t], modifier);
				goto error;
			}

#ifdef DEBUG			
			verbose(1, VSPACE "Right-hand token is value modulator `%s'", 0,
				mtokens[t]);
#endif			
		}
		else {
			if(times_mod(time_vals, mtokens[t], &value, LOOKUP_VALUE) < 0) {
				error_out(ERROR_ERROR_RESOLV, 0, FLN, mtokens[t], modifier);
				goto error;
			}
			verbose(1, VSPACE "Right-hand identifier `%s' resolved to %d",
				0, mtokens[t], value);
		}

		/* Remaining modifier expressions are partial assignments */
		while(--t >= 0) {
			
			idf = mtokens[t];

			/* When modulating read original value first */
			if(modulator) {
				if(times_mod(time_vals, idf, &value, LOOKUP_VALUE) < 0) {
					error_out(ERROR_ERROR_MFIND, 0, FLN, idf, idf, modulator,
						  modifier);
					goto error;
				}

				verbose(1, VSPACE "Applying modulator `%d' to `%s=%d'", 0,
					modulator, idf, value);
				
				value += modulator;
			}

			if(!validate(idf, value) ||
			   times_mod(time_vals, idf, &value, SET_VALUE) < 0) {
				
				error_out(ERROR_ERROR_SETVAL, 0, FLN, idf, value, modifier);
				goto error;
			}

			verbose(1, VSPACE "Partial assignment `%s=%d' made", 0, idf, value);
		}

		free_str_array((char ***)&mtokens);
	}
	
	return 0;
	
 error:
	free_str_array((char ***)&mtokens);
	return -1;
}

/*
 * Evaluating time stamp of the form [[CC]YY]MMDDhhmm[.ss].
 * stp is an array of strings of size one or
 * two whose second element, if present, is a string containing 'm'
 * or 'a' which denote mtime or atime to be changed
 * respectively. Having no specifying element two in stp will
 * default to changing the mtime within the time_vals array.
 * Returns 0 on success, -1 on failure.
 */
static int
time_stamp(const char **stp)
{
	const char *default_tv = "m", *tvs, *s;
	int parse[TIME_VALS-2], tvidx;
	register int i, j;
	char buf[3];

	verbose(1, "Evaluating time stamp expression `%s'", *stp);

	if(*(s = *stp++) == '.')
	   goto error;

	/* Selector */
	if(*stp && **stp) {
		if(strlen((tvs = *stp)) > 3)
			goto error;
	} else
		tvs = default_tv;
	
	memset(&parse, 0, sizeof parse);
	memset(buf, '\0', sizeof buf);
			
	/* Optional seconds */
	for( ; *s; s++) {
		if(*s == '.') {
			if(strlen(++s) != 2 || !isnum_zero(s, TRUE))
				goto error;
			parse[SEC] = atoi(s--);
			break;
		}
	}

	/* Parse from minute backwards */
	for(i = MIN; s > stp[-1]; i--) {
		
		if((s -= 2) < stp[-1]) goto error;
		
		strncpy(buf, s, 2);
		if(!isnum_zero(buf, TRUE)) goto error;

		/* Century */
		if(i < 0) {
			parse[YEAR] += atoi(buf) * 100;
		} else
			parse[i] = atoi(buf);
	}

	if(i > YEAR) goto error;

	/* Supplement century */
	if(i == -1) parse[YEAR] += CURR_CENT * 100;
		    
	/* Copy values */
	for( ; *tvs; tvs++) {
		if((tvidx = tstr(tvs)) < 0)
			goto error;

		if(tvidx == CTIME) SETF(CTAPPLY);
		
		for(j = YEAR; j <= SEC; j++) {
			if(!j && i >= 0) continue; /* Year not set */
			time_vals[tvidx][j].val = parse[j];
			
			verbose(1, VSPACE "Assignment `%s=%d' made", 0,
				time_vals[tvidx][j].name, parse[j]);
		}
	}
	
	return 0;
	
 error:
	error_out(ERROR_ERROR_TIMEST, 0, FLN, stp[-1], IFF(*stp, "-"));
	return -1;
}

/*
 * Apply time stamps in time_vals array to file.
 * Returns 0 on success, -1 on failure.
 */
static int
apply(const char *file)
{
	struct utimbuf ut;
	int (*utimef)(const char *, const struct utimbuf *) =
		CHKF(SYMLINKS) ? &lutime_symlink : &utime;

	verbose(1, "Applying date and time alterations: \"%s\"", file);
	
	/* mtime, atime */
	if(ft_to_utimbuf(time_vals, &ut) < 0 || (*utimef)(file, &ut) < 0) {
		error_out(ERROR_ERROR_SETTIM, errno, FLN, file);
		return -1;
	}
	
	/* ctime */
	if(CHKF(CHCTIME) || CHKF(CTAPPLY) || CHKF(CTPRES)) {
		if(mod_ctime(time_vals, file) < 0)
			return -1;
	}
	
	return 0;
}

/*
 * Print mtime, atime, ctime information of current time_vals table.
 */
static void
times_info(const char *file)
{
	char lnk[PATH_MAX];
	int i, slnk;

	msg("%s:", file);
	
	if((slnk = laccess(file, F_OK)) >= 0 &&
	   (i = readlink(file, lnk, sizeof lnk)) > 0) {
		lnk[i] = 0;
		msg(" Symbolic link: \"%s\" -> \"%s\" %s",
		    file, lnk, IFSTR(slnk == LDANGLING, "(dangling)"));
		msg(" %s shown:", CHKF(SYMLINKS) ? "Symbolic link" : "Actual file");
	}

	if(CHKF(NEXIST)) {
		msg(" File does not exist. %s",
		    IFSTR(laccess(file, F_OK) == LDANGLING,
			  "Dangling symbolic link? Try `-l'."));
		return;
	}
	
	for(i = 0; i < TIME_TBLS; i++)
		msg(" %s: %s", names[i], tv_to_str(time_vals,i));
}

/*
 * Prints usage; will exit program
 */
static void
usage(GENERAL_BOOL error)
{
	if(!error) printf(PROGRAM" - %s\n\n", *pinf);
	else putchar('\n');
	printf("%s", usg);
	exit(error ? last_error_code : 0);
}

/*
 * Prints program info; will exit program
 */
static void info()
{
	printf(infostr, libgeneral_version, pinf[2], pinf[3]);
	exit(0);
}

/*
 * Perform cleanups; called atexit()
 */
void cleanups()
{
	libgeneral_uninit_errors();
	libgeneral_uninit();
}

int
main(int argc, char **argv)	
{
	char *modifier_list, **modifiers;
	char *arg, *file, *modifs;
	sigset_t segv_mask;
	FILE *batch;
	int fd;
	
	/*
	 * Initialization
	 */

	modifier_list = NULL;
	modifiers = NULL;
	modifs = NULL;
	
	libgeneral_init(PROGRAM, 0);
	
	libgeneral_init_errors(&error_messages, 0);
	libgeneral_init_verbose(&verbosity_level, "verbose", 1);
	
	atexit(&cleanups);

	sigfillset(&segv_mask);
	SET_SIGNAL(SIGSEGV, &segv_mask, 0);

	SIGNAL_CATCHING();
		
	CATCH_SIGNAL(SIGSEGV) {
		error_out(ERROR_FATAL_SEGV, 0, FLN, GET_SIGINFO()->si_addr);
	}

	/*
	 * Argument parsing
	 */
	
	if(argc < 2) usage(0);
	
	/* Handle options (arg used by ARG()) */
	while(--argc && *(arg = *++argv) == '-') {
		
		if(!*++arg) break; /* stdin - */
		
		if(ARG("h", "help"))
			usage(0);
		else if(ARG("f", "force"))
			SETF(FORCE);
		else if(ARG("l", "symlinks"))
			SETF(SYMLINKS);
		else if(ARG("i", "info"))
			SETF(INFO);
		else if(ARG("b", "batch"))
			SETF(BATCH);
		else if(ARG("s", "stamp"))
			SETF(STAMP);
		else if(ARG("r", "reference"))
			SETF(REFER);
		else if(ARG("c", "ctime"))
			SETF(CHCTIME);
		else if(ARG("p", "preserve"))
			SETF(CTPRES);
		else if(ARG("q", "quiet")) {
			SETF(QUIET);
			libgeneral_set_flag(OPTION_QUIET);
		}
		else if(ARH("version"))
			info();
		else if(ARG("v", "verbose")) {	
			SETF(VERBOSE);
			libgeneral_set_flag(OPTION_ERROR_CODE_ON_ERROR);
#ifdef DEBUG			
			libgeneral_set_flag(OPTION_ERRORS_POINT_TO_SOURCE);
#endif
		}
		else {
			error_out(ERROR_ERROR_UKNARG, 0, FLN, --arg);
			usage(1);
		}
	}

	/* `-p' and `-c' */ 
	if(CHKF(CTPRES) && CHKF(CHCTIME)) {
		error_out(ERROR_ERROR_CTPRES, 0, FLN);
		usage(1);
	}

	/* `i' and `-r' or `-s' or `-b' */
	if(CHKF(INFO) && (CHKF(REFER) ||  CHKF(STAMP) || CHKF(BATCH))) {
		error_out(ERROR_ERROR_INVCOMB, 0, FLN);
		usage(1);
	}

	/* Number of args validations */
	if(argc < 1) {
		error_out(ERROR_ERROR_INSUFARGS, 0, FLN);
		usage(1);
	} else if(argc < 2 && CHKF(BATCH | STAMP | REFER)) {
		if(CHKF(BATCH))
			error_out(ERROR_ERROR_BATCHF, 0, FLN);
		else
			error_out(ERROR_ERROR_MODFIL, 0, FLN);
		usage(1);
	}

	/* Fixme: Filename containing '='? */
	modifs = strchr(*argv, '=');
	
	if(modifs && CHKF(INFO)) {
		error_out(ERROR_ERROR_INFMODF, 0, FLN);
		goto error;
	}

	if(verbosity_level() && CHKF(FORCE))
		error_out(ERROR_WARNING_FORCVAL, 0, FLN);

	/*
	 * Obtain modifier expressions, time stamps, or reference file
	 */
	if(modifs || CHKF(STAMP) || CHKF(REFER) || CHKF(BATCH) || **argv == '-') {
		/* batch, stdin, normal */
		if(CHKF(BATCH)) {
			verbose(1, "Opening batch file: \"%s\"", *argv);
			if(!(batch = fopen(*argv, "r"))) {
				error_out(ERROR_ERROR_FOPEN, errno, FLN, *argv);
				goto error;
			}
			modifier_list = read_batch(batch);
			fclose(batch);
		} else if (**argv == '-') {
			verbose(1, "Reading modifier list from stdin");
			modifier_list = read_batch(stdin);
		}
		else
			modifier_list = *argv;
	
		modifiers = sep_to_array(MODSEPS, skew(modifier_list));

		(void)read_batch(NULL);

		if(!modifiers || !*modifiers || !**modifiers) {
			error_out(ERROR_ERROR_INVMOD, 0, FLN);
			goto error;
		}

		file = *++argv, --argc;
	} else
		file = *argv;

	(void)tzset();

	/*
	 * Scanning reference file or current time
	 */
	if(CHKF(REFER)) {
		verbose(1, "Scanning reference file \"%s\"", *modifiers);
		if(scan(*modifiers) < 0)
			goto error;
	} else if(!CHKF(INFO)) {
		verbose(1, "Retrieving current time");
		if(scan(NULL) < 0)
			goto error;
	}

	/*
	 * Handle file(s)
	 */
	for( ; argc-- > 0; file = *++argv, REMF(NEXIST)) {
		
		if(!file || !*file) {
			error_out(ERROR_ERROR_INVFIL, 0, FLN);
			usage(1);
		}

		/* If `-l', file exists as link even if dangling */
		if(CHKF(SYMLINKS)) {
			if(laccess(file, F_OK) < 0 && access(file, F_OK) < 0)
				SETF(NEXIST);
		} else {
			if(access(file, F_OK) < 0)
				SETF(NEXIST);
		}

		/*
		 * Scanning file
		 */
		
		if(CHKF(REFER)) {
			goto apply;
		} else if(!CHKF(NEXIST) && (modifiers || CHKF(INFO))) {
			verbose(1, "Scanning file \"%s\"", file);
			if(scan(file) < 0 || validate_times(time_vals) < 0)
				goto error;
		}

		if(CHKF(INFO)) {
			times_info(file);
			continue;
		}
	
		/*
		 * Evaluating
		 */
	
		/* Time stamp and modifier expression list evaluation */
		if(CHKF(STAMP)) {
			if(str_array_size((const char**)modifiers) > 2) {
				error_out(ERROR_ERROR_INVTSP, 0, FLN, modifier_list);
				goto error;
			}
			if(time_stamp((const char**)modifiers) < 0)
				goto error;
		} else if(modifiers) {
			if(eval((const char**)modifiers) < 0)
				goto error;
		}
	
		/*
		 * Applying
		 */
	apply:
	
		if(validate_times(time_vals) < 0)
			goto error;
	
		/* Create file if necessary */
		if(CHKF(NEXIST) && (!modifiers || CHKF(STAMP | REFER))) {
			if((fd = open(file, O_CREAT | O_WRONLY | O_TRUNC,
				      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
				error_out(ERROR_ERROR_FCREATE, errno, FLN, file);
				goto error;
			}
			close(fd);
			verbose(1, "File created: \"%s\"", IFF(realname(file), "-"));
		}
		
		if(apply(file) < 0)
			goto error;
	}
	
	free_str_array(&modifiers);
	return 0;

	/*
	 * Error handling
	 */
 error:
	
#ifdef DEBUG
	dump_tv(time_vals);
	dump_str_array((const char**)modifiers);
#endif
	
	free_str_array(&modifiers);
	return last_error_code;
}

