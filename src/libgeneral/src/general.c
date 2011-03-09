
#include <libgeneral/general.h>

#include <stdio.h>
#include <stdarg.h>

#include <libgeneral/args.h>
#include <libgeneral/error.h>

/*
 * Globals
 */

/* Libgeneral version string */
const char *libgeneral_version = TOSTR(LIBGENERAL_VERSION);

/* Name of the program that uses libgeneral */
char *prog_name;

/* Options bitmask */
_FLAG_TYPE libgeneral_flags;

/* Bool if library is initialized already */
static GENERAL_BOOL initialized;

/*
 * General
 */

void libgeneral_init(const char* progname, _FLAG_TYPE option_flags)
{
	if(initialized) return;
	if( progname && *progname )
		prog_name = cpy_string(progname);
	else prog_name = NULL;
	libgeneral_flags = option_flags;
	++initialized;
}

void libgeneral_uninit()
{
	if(initialized && prog_name) {
		free(prog_name);
		visual_spacing(0);
		new_str(0);
	}
}
/*
 * Libgeneral options
 */

void libgeneral_set_flag(_FLAG_TYPE flag)
{
	libgeneral_flags |= flag;
}

void libgeneral_unset_flag(_FLAG_TYPE flag) {
	libgeneral_flags &= ~flag;
}

inline GENERAL_BOOL libgeneral_check_flag(_FLAG_TYPE flag) {
	return ((libgeneral_flags & flag) == flag);
}

/*
 * Verbosity
 */
static int (*verbose_level)();
static const char *vprefix;
static int max_levels;

/*
 * verbose_func is a function pointer supplied by the program which returns
 * the current verbosity level of the program. max_level is the maximum verbosity
 * level of the program. All levels from 0 to max_level are then valid verbosity
 * levels of the program using libgeneral.
 */
void libgeneral_init_verbose(int (*verbose_func)(), const char *prefix, int max_level) 
{
	if(!initialized) {
		errwrn(ERROR_FATAL, 0, FLN, LIBGENERAL_ERROR_PREFIX "init_verbose(): Library not"
		       "yet initialized");
	}
	
	verbose_level = verbose_func;
	vprefix = prefix;
	max_levels = max_level;
}


/*
 * Memory managment, see malloc().
 */

void* general_malloc(size_t size) {
	void *space;
	if( (space = malloc(size)) == NULL ) {
		errwrn(ERROR_FATAL, 0, FLN, LIBGENERAL_ERROR_PREFIX "Malloc(): Memory exhausted");
	}
	return space; 
}

/*
 * See realloc().
 */
void* general_realloc(void *ptr, size_t size) {
	void *space;
	if( (space = realloc(ptr, size)) == NULL ) {
		errwrn(ERROR_FATAL, 0, FLN, LIBGENERAL_ERROR_PREFIX "Realloc(): Memory exhausted");
	}
	return space;
}

/**
 * For nifty spaced output.
 * This function ought to be called for each output (usually handled
 * in msg(...).
 * "Block" of text will be spaced by 'space' newlines.
 */
void visual_spacing(short space) {
	static int last_space;
	int i = last_space + X_OR_ZERO(space-last_space);
	while(i-- > 0) putchar('\n');
	last_space = space;
}

/**
 * Does pretty much the same as strdup.
 * Returns a copy of string with new memory alloc'd.
 * Needs to be free'd.
 */
char *cpy_string(const char *str) {
	char *strn = general_malloc( strlen(str)+1 );
	strncpy(strn, str, strlen(str));
	strn[strlen(str)]='\0';
	return strn;
}

/**
 * Zero-out string
 */
static inline void zero_out(char *str) {
	for( ; *str != '\n' && *str != '\0'; str++ ) *str = '\0';
	*str = 0;
}

/**
 * Read line from stream whereby an allocated memory space is returned.
 * This function does not have any length restrictions to the line being read.
 * The line will contain the '\n' character at the end.
 * It could happend that not enough memory is available, in this case
 * the line is "silently truncated". This should not happen by default.
 * If the string allocated herein is not used anymore this function has to be called
 * with the stream argument as NULL in the end to perform cleanups.
 */
char *readline_stream(FILE *s)
{
#define LINE_END(C) (C == '\n' || C == '\r' || C == '\0') 

	static char *line_buf;
	size_t i = 0, sz = 0x4;
	
	if(!s) {
		if(line_buf) {
			free(line_buf);
			line_buf = NULL;
		}
		return NULL;
	}
	
	if(line_buf) zero_out(line_buf);
	
	do {
		sz <<= 1;
		if((line_buf = realloc(line_buf, sz * sizeof(char))) == NULL) {
			/* First time must yield memory */
			if(!i) { 
				/* Will abort */
				errwrn(ERROR_FATAL, 0, FLN, LIBGENERAL_ERROR_PREFIX "Realloc failed");	
			}
			line_buf[i] = '\n';
		}
		
		do {
			if(ferror(s)) {
				errwrn(ERROR_FATAL, 0, FLN, LIBGENERAL_ERROR_PREFIX "File stream invalid");
				return NULL;
			}
			if((line_buf[i++] = fgetc(s)) == EOF) 
				return NULL;
		} while(!LINE_END(line_buf[i-1]) && i+1 < sz); 
		
	} while(!LINE_END(line_buf[i-1]));
	
	line_buf[i] = 0;
	
	return line_buf;
}

/** 
 * This function supplies "stored strings".
 * They can be used by any function in order to have a reference to a dynamically
 * allocated string in order to escape the limiting dynamic stack.
 * A function may obtain such a reference by calling new_str( str ). Then str is 
 * copied into the newly allocated heap memory.
 * To clean up heap memory, new_str(NULL) should be called at frequent intervals.
 * Will return NULL if an error occured or if string
 * supplied was somehow invalid.
 */
char* new_str(const char *str)
{
	static char **str_strg;
	static int num_strs;
	char *ret = (char*)0;
	
	/* str was passed; store string on heap */
	if( str ) {
		str_strg = (char**)general_realloc(str_strg, (num_strs + 1) * sizeof(char*));
		str_strg[num_strs] = (char*)general_malloc(strlen(str) + 1);
		strcpy(str_strg[num_strs], str);
		ret = *(str_strg + num_strs);
		++num_strs;
	} 
	/* NULL was passed; do cleanups */
	else if( num_strs > 0 ) {
		while( num_strs > 0 ) {
			free(str_strg[--num_strs]);	
		}
		free(str_strg);
		str_strg = (char**)0;
	}
	return ret;
}

/**
 * This function is like fprintfs only that it additionally 
 * takes a prefix string to put in fron of every line and that it
 * takes an arguments array instead of a variable argument list (See arg/args.c).
 * Morever, the newline character '\n' always yields a newline with leading 
 * prefix. The vertical tab character '\v' is used for definite new line.
 */
void _prfx_print_args(FILE *stream, const char *prfx, const char *frm, ARG_ARRAY args)
{
       char *str, *format_p, format[16], format_buf[16];
       GENERAL_BOOL p = FALSE;
       void *next_arg;
       
       format_p = format_buf;
       str = (char *)frm;
       
       memset(format_buf, '\0', sizeof(format_buf));
       
       if(prfx && *prfx) fprintf(stream, "%s: ", prfx);
       
       for( ; *str; str++ ) {
       		if( *str == '\n' ) {
       			if( prfx && *prfx ) fprintf(stream, "\n%s: ", prfx);
       			else fputc('\n', stream);
       		}
       		else if(*str == '\v') { 
       			/* Use vertical tab '\v' as our new line */
				fputc('\n', stream);
			}
       		else if(*str == '%') {
       			p = TRUE;
       		} else if(p) {
       				/* Check if next argument is available */
       				if( !*args ) continue;
       				if( strchr("sdfhc", *str) != NULL ) {
       					*++format_p='\0';
       					sprintf(format, "%%%s%c", format_buf, *str);
       				}
       				/* Retrieve next argument */
       				if( (next_arg = (*args)->argument) == NULL ) continue;
       			switch(*str) {
       				/* Advance argument array */
#define ARG_OUT p = FALSE, ++args
       				case 'f':
       					fprintf(stream, format, *(double*)next_arg);
       					ARG_OUT;
       				break;
       				case 's':
       					fprintf(stream, format, (char*)next_arg);
       					ARG_OUT;
       				break;
       				case 'd':
       					fprintf(stream, format, *(int*)next_arg);
       					ARG_OUT;
       				break;
       				case 'h':
       					fprintf(stream,"%x", *(unsigned int*)next_arg);
       					ARG_OUT;;
       				break;
       				case 'c':
       					fprintf(stream, "%c", *(char*)next_arg);
       					ARG_OUT;
       				break;
       				case 'p':
       					fprintf(stream, "%p", (void*)next_arg);
       					ARG_OUT;
       				break;
       				default:
       					*format_p++ = *str;
       				break;
       			}
       			if( !p ) {
       				format_p = format_buf;
				}
			} else {
				fputc(*str, stream);
			}  	
	   } 
}

/**
 * Printf like function that takes a prefix.
 * See function above.
 */
void _prfx_print(FILE *stream, const char *prfx, const char *frm, va_list ap) {
	ARG_ARRAY args;
	
	args = varg_to_argarr(frm, ap);
	_prfx_print_args(stdout, prfx, frm, args);
	arg_array_destroy(args);
	new_str(0);
}

/**
 * Like prfx_print. See above.
 */
void _nv_prfx_print(FILE *stream, const char *prfx, const char *frm, ...) {
	va_list v;
	va_start(v, frm);
	_prfx_print(stream, prfx, frm, v);
	va_end(v);
}

/*
 * An optional virtual_spacing (block spacing) may be opted for
 * by using this function as follows. The same applies for verbose().
 *
 * msg(VSPACE "Some random %s", 2, "string");
 *
 */
int msg(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	if( !msg || !*msg || strlen(msg) < 1 ) return 0;
	if( !libgeneral_check_flag(OPTION_QUIET) ) {
		_VSPACING(ap, msg, 0);
		_prfx_print(stdout, prog_name, msg, ap);
		putc('\n', stdout);
	}
	va_end(ap);
	return 0;
}


void verbose(int vlevel, const char *msg, ... )
{
	char prfx[127], level[10];
	int verbosity;
	va_list ap;

	verbosity = verbose_level();
	va_start(ap, msg);

	if( !libgeneral_check_flag(OPTION_QUIET) ) {
		/* Verbosity level check */
		if(vlevel <= verbosity && verbosity > 0) {
			_VSPACING(ap, msg, 1);
			*level = 0;
			if(libgeneral_check_flag(OPTION_VERBOSE_SHOW_LEVEL))
				sprintf(level, "[%d]", vlevel);
			sprintf(prfx, "%s: %s%s", prog_name, vprefix, level);
			_prfx_print(stdout, prfx, msg, ap);
			putc('\n', stdout);
		}
	}
	va_end(ap);
}

/*
 * Time (elapsed) measuring.
 * Call this function with NULL argument once
 * to initialize time measuring.
 * Call this function twice with a valid
 * reference to a `struct timeval' object.
 * Time time elapsed will be written
 * to that structure.
 * Returns -1 on failure, 0 on success is returned.
 */

int timer(struct timeval *elapsed)
{
	static struct timeval start;
	struct timeval end;

	if(!elapsed) {
		if(gettimeofday(&start, NULL) < 0)
			return -1;
	} else {
		if(!start.tv_sec || gettimeofday(&end, NULL) < 0)
			return -1;
		elapsed->tv_sec = end.tv_sec - start.tv_sec;		
		elapsed->tv_usec = end.tv_usec - start.tv_usec;
	}
		
	return 0;
}

