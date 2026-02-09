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
#include <getopt.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libgeneral/general.h>
#include <libgeneral/error.h>
#include <libgeneral/signals.h>

#include "stroke.h"
#include "errors.h"
#include "gnulib/parse-datetime.h"


/***************
 * Information *
 ***************/

const char *usg =
	"Usage: "PROGRAM" [OPTIONS] FILE...\n\n"
	"Without any setters stroke prints every timestamp for each FILE. Provide\n"
	"one or more setters to modify them:\n\n"
	"  -m, --mtime=SPEC      set modification time to SPEC\n"
	"  -a, --atime=SPEC      set access time to SPEC\n"
	"  -c, --ctime=SPEC      set change time to SPEC (requires root)\n"
	"      --copy=FILE       copy all timestamps from FILE\n\n"
	"Options:\n"
	"  -l, --symlinks        operate on symbolic links themselves\n"
	"      --dry-run         validate changes without applying them\n"
	"  -p, --preserve-ctime  preserve change time even when mutating other clocks\n"
	"  -f, --force           skip sanity checks (dangerous)\n"
	"  -q, --quiet           suppress per-file output\n"
	"  -v, --verbose         emit additional diagnostics\n"
	"      --help            show this help text\n"
	"      --version         print program information\n\n"
	"Timestamp SPEC accepts common ISO-8601 forms (e.g. 2024-02-01T13:37) or\n"
	"relative expressions such as \"now -2 hours\" and \"+3days\".\n"
	"\nPlease help by reporting bugs to <"PACKAGE_BUGREPORT">.\n\n";

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


struct timestamp_param {
	GENERAL_BOOL set;
	struct timespec ts;
};

struct stroke_cli {
	struct timestamp_param atime;
	struct timestamp_param mtime;
	struct timestamp_param ctime;
	const char *copy_from;
	GENERAL_BOOL dry_run;
};

static int parse_timestamp_spec(const char *spec, struct timespec *out);
static int assign_timespec(FILE_TIMES ft, int slot, const struct timespec *ts);
static int check_dry_run_permissions(const char *file, GENERAL_BOOL exists,
				     GENERAL_BOOL will_touch_ctime,
				     GENERAL_BOOL create_file);
static GENERAL_BOOL have_ctime_privileges(void);

static void
current_timespec(struct timespec *ts)
{
#ifdef CLOCK_REALTIME
	if(clock_gettime(CLOCK_REALTIME, ts) == 0)
		return;
#endif
	ts->tv_sec = time(NULL);
	ts->tv_nsec = 0;
}

static int
parse_timestamp_spec(const char *spec, struct timespec *out)
{
	struct timespec base;
	current_timespec(&base);
	if(!parse_datetime(out, spec, &base))
		return -1;

	return 0;
}

static int
assign_timespec(FILE_TIMES ft, int slot, const struct timespec *ts)
{
	time_t sec = ts->tv_sec;
	struct tm tm;

	if(!localtime_r(&sec, &tm))
		return -1;

	translate(&tm, ft, slot, TO_FT);
	return 0;
}

static char *
parent_dir(const char *path, char *buf, size_t len)
{
	if(!path || !*path)
		return NULL;

	const char *slash = strrchr(path, '/');
	if(!slash) {
		if(len < 2)
			return NULL;
		strcpy(buf, ".");
		return buf;
	}

	size_t dlen = slash - path;
	if(dlen == 0)
		dlen = 1;
	if(dlen >= len)
		return NULL;
	memcpy(buf, path, dlen);
	buf[dlen] = '\0';
	return buf;
}

static int
ensure_parent_writable(const char *file)
{
	char dirbuf[PATH_MAX];
	const char *dir = parent_dir(file, dirbuf, sizeof(dirbuf));
	if(!dir) dir = ".";

	if(access(dir, W_OK) < 0) {
		error_out(ERROR_ERROR_FCREATE, errno, FLN, file);
		return -1;
	}
	return 0;
}

static int
check_dry_run_permissions(const char *file, GENERAL_BOOL exists,
			  GENERAL_BOOL will_touch_ctime,
			  GENERAL_BOOL create_file)
{
	if(will_touch_ctime && geteuid() != 0) {
		error_out(ERROR_ERROR_CHCTIME, EPERM, FLN, file,
			  "change time modifications require root privileges");
		return -1;
	}

	int rc;
	if(exists) {
		rc = CHKF(SYMLINKS) ? laccess(file, W_OK) : access(file, W_OK);
		if(rc < 0) {
			error_out(ERROR_ERROR_SETTIM, errno, FLN, file);
			return -1;
		}
	} else if(create_file) {
		if(ensure_parent_writable(file) < 0)
			return -1;
	}

	return 0;
}

static GENERAL_BOOL
have_ctime_privileges(void)
{
	return geteuid() == 0;
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
		if(errno == EPERM || errno == EACCES)
			error_out(ERROR_ERROR_SETTIM_PERM, errno, FLN, file);
		else
			error_out(ERROR_ERROR_SETTIM, errno, FLN, file);
		return -1;
	}
	
	/* ctime */
	if(CHKF(CTAPPLY) || CHKF(CTPRES)) {
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
	sigset_t segv_mask;
	int fd;
	struct stroke_cli cli = {0};
	GENERAL_BOOL preserve_ctime_requested = FALSE;
	GENERAL_BOOL have_setters = FALSE;
	FILE_TIME copy_template[TIME_TBLS][TIME_VALS];
	GENERAL_BOOL have_copy_template = FALSE;

	static const struct option long_opts[] = {
		{"mtime",   required_argument, NULL, 'm'},
		{"atime",   required_argument, NULL, 'a'},
		{"ctime",   required_argument, NULL, 'c'},
		{"reference", required_argument, NULL, 'r'},
		{"copy",    required_argument, NULL, 'r'},
		{"symlinks",no_argument,       NULL, 'l'},
		{"preserve-ctime", no_argument,NULL, 'p'},
		{"force",   no_argument,       NULL, 'f'},
		{"quiet",   no_argument,       NULL, 'q'},
		{"verbose", no_argument,       NULL, 'v'},
		{"dry-run", no_argument,       NULL, 1000},
		{"help",    no_argument,       NULL, 'h'},
		{"version", no_argument,       NULL, 1001},
		{0,0,0,0}
	};

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

	int opt;
	while((opt = getopt_long(argc, argv, "m:a:c:r:lpqvfh", long_opts, NULL)) != -1) {
		switch(opt) {
		case 'm':
			if(parse_timestamp_spec(optarg, &cli.mtime.ts) < 0) {
				error_out(ERROR_ERROR_INVTSP, 0, FLN, optarg);
				return last_error_code;
			}
			cli.mtime.set = TRUE;
			have_setters = TRUE;
			break;
		case 'a':
			if(parse_timestamp_spec(optarg, &cli.atime.ts) < 0) {
				error_out(ERROR_ERROR_INVTSP, 0, FLN, optarg);
				return last_error_code;
			}
			cli.atime.set = TRUE;
			have_setters = TRUE;
			break;
		case 'c':
			if(parse_timestamp_spec(optarg, &cli.ctime.ts) < 0) {
				error_out(ERROR_ERROR_INVTSP, 0, FLN, optarg);
				return last_error_code;
			}
			cli.ctime.set = TRUE;
			have_setters = TRUE;
			break;
		case 'r':
			cli.copy_from = optarg;
			have_setters = TRUE;
			break;
		case 'l':
			SETF(SYMLINKS);
			break;
		case 'p':
			preserve_ctime_requested = TRUE;
			break;
		case 'f':
			SETF(FORCE);
			break;
		case 'q':
			SETF(QUIET);
			libgeneral_set_flag(OPTION_QUIET);
			break;
		case 'v':
			SETF(VERBOSE);
			libgeneral_set_flag(OPTION_ERROR_CODE_ON_ERROR);
#ifdef DEBUG
			libgeneral_set_flag(OPTION_ERRORS_POINT_TO_SOURCE);
#endif
			break;
		case 'h':
			usage(0);
			break;
		case 1000: /* --dry-run */
			cli.dry_run = TRUE;
			break;
		case 1001: /* --version */
			info();
			break;
		default:
			error_out(ERROR_ERROR_UKNARG, 0, FLN, argv[optind-1]);
			return last_error_code;
		}
	}

	(void)tzset();

	if(verbosity_level() && CHKF(FORCE))
		error_out(ERROR_WARNING_FORCVAL, 0, FLN);

	if(optind >= argc) {
		error_out(ERROR_ERROR_INSUFARGS, 0, FLN);
		usage(1);
	}

	if(preserve_ctime_requested && (cli.copy_from || cli.ctime.set)) {
		error_out(ERROR_ERROR_CTPRES, 0, FLN);
		return last_error_code;
	}

	GENERAL_BOOL have_ctime_priv = have_ctime_privileges();
	GENERAL_BOOL copy_ctime_skipped = FALSE;

	if(cli.ctime.set && !have_ctime_priv) {
		error_out(ERROR_ERROR_CTPRIV, 0, FLN, "--ctime");
		return last_error_code;
	}

	if(preserve_ctime_requested && !have_ctime_priv) {
		error_out(ERROR_ERROR_CTPRIV, 0, FLN, "--preserve-ctime");
		return last_error_code;
	}

	if(cli.copy_from) {
		if(scan(cli.copy_from) < 0)
			return last_error_code;
		memcpy(copy_template, time_vals, sizeof(copy_template));
		have_copy_template = TRUE;
		if(!have_ctime_priv)
			copy_ctime_skipped = TRUE;
	}

	for(int idx = optind; idx < argc; ++idx) {
		const char *file = argv[idx];
		GENERAL_BOOL exists;

		if(CHKF(SYMLINKS)) {
			if(laccess(file, F_OK) < 0 && access(file, F_OK) < 0)
				exists = FALSE;
			else
				exists = TRUE;
		} else {
			exists = (access(file, F_OK) == 0);
		}

		if(exists)
			REMF(NEXIST);
		else
			SETF(NEXIST);

		REMF(CTAPPLY);

		if(!have_setters) {
			if(exists && scan(file) < 0)
				return last_error_code;
			if(!CHKF(QUIET))
				times_info(file);
			continue;
		}

		if(have_copy_template) {
			memcpy(time_vals, copy_template, sizeof(copy_template));
			if(have_ctime_priv)
				SETF(CTAPPLY);
			else
				REMF(CTAPPLY);
		} else if(!exists) {
			if(scan(NULL) < 0)
				return last_error_code;
		} else {
			if(scan(file) < 0)
				return last_error_code;
		}

		if(cli.mtime.set) {
			if(assign_timespec(time_vals, MTIME, &cli.mtime.ts) < 0) {
				error_out(ERROR_ERROR_GMTIM, errno, FLN, file);
				return last_error_code;
			}
		}

		if(cli.atime.set) {
			if(assign_timespec(time_vals, ATIME, &cli.atime.ts) < 0) {
				error_out(ERROR_ERROR_GMTIM, errno, FLN, file);
				return last_error_code;
			}
		}

		if(cli.ctime.set) {
			if(assign_timespec(time_vals, CTIME, &cli.ctime.ts) < 0) {
				error_out(ERROR_ERROR_GMTIM, errno, FLN, file);
				return last_error_code;
			}
			SETF(CTAPPLY);
		}

		GENERAL_BOOL run_preserve =
			preserve_ctime_requested &&
			(cli.mtime.set || cli.atime.set);

		if(run_preserve)
			SETF(CTPRES);
		else
			REMF(CTPRES);

		if(validate_times(time_vals) < 0)
			return last_error_code;

		GENERAL_BOOL need_ctime = CHKF(CTAPPLY) || CHKF(CTPRES);

		if(cli.dry_run) {
			if(check_dry_run_permissions(file, exists, need_ctime, CHKF(NEXIST)) < 0)
				return last_error_code;
		} else {
			if(CHKF(NEXIST)) {
				if((fd = open(file, O_CREAT | O_WRONLY | O_TRUNC,
					      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
					error_out(ERROR_ERROR_FCREATE, errno, FLN, file);
					return last_error_code;
				}
				close(fd);
				REMF(NEXIST);
				verbose(1, "File created: \"%s\"", IFF(realname(file), "-"));
				exists = TRUE;
			}

			if(apply(file) < 0)
				return last_error_code;

			if(scan(file) < 0)
				return last_error_code;
		}

		if(!cli.dry_run && CHKF(QUIET))
			continue;
		if(cli.dry_run && CHKF(QUIET))
			continue;

		GENERAL_BOOL cleared = FALSE;
		if(cli.dry_run && CHKF(NEXIST)) {
			REMF(NEXIST);
			cleared = TRUE;
		}

		times_info(file);

	if(cleared)
		SETF(NEXIST);
}

	if(copy_ctime_skipped)
		error_out(ERROR_WARNING_CTCOPY, 0, FLN);

	return 0;
}
