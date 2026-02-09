# Autoconf input file for stroke 
#
# Copyright 2009 Sören Wellhöfer <soeren.wellhoefer@gmx.net>
#      
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.


AC_PREREQ(2.61)
AC_INIT([stroke],[0.1.4], [soeren.wellhoefer@gmx.net])
AC_CONFIG_SRCDIR([src/stroke.h])

#
# Configure options
#
EXTRA_FLAGS= 

# Debugging mode option
debugging=no

AC_ARG_ENABLE([debug],
[  --enable-debug          enable debugging mode],
[debugging=${enableval}],  [debugging=no])

if test "x${debugging}" = xyes; then
   AC_DEFINE([DEBUG], 1, [Add usbsync source debug features])
   EXTRA_FLAGS+="-Wall --debug -g -O0"
   AC_MSG_NOTICE([Debugging mode enabled])
fi

AC_SUBST([EXTRA_FLAGS])

#
# Check for programs
#
AC_PROG_CC
gl_EARLY
AC_PROG_INSTALL
AC_PROG_RANLIB

#
# Gnulib
#
gl_INIT

#
# Automake stuff
#
AM_PROG_CC_C_O
AM_INIT_AUTOMAKE([foreign])

#
# Checks for header files
#
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h libgen.h limits.h stdlib.h stdarg.h string.h time.h sys/time.h unistd.h utime.h],,[
AC_MSG_ERROR([
*********************************
Unable to locate mandatory header
file: "$ac_header".
Make sure header is installed
and present in your include
path.
********************************
])])

#
# Checks for typedefs, structures, and compiler characteristics
#
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_TYPE_SIGNAL
AC_STRUCT_TM

#
# Checks for library functions
#
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK

AC_CHECK_FUNCS([memset strchr strlen utime],, [
AC_MSG_ERROR([
******************************
ERROR: Mandatory function call
not present: "$ac_func"
*****************************
])])

AC_CHECK_FUNCS(utimensat, [], [
AC_MSG_WARN([
************************************
WARNING: utimensat() not available.
Program will not be able to set a
symbolic link's modification, access,
or change time.
***********************************
])])

AC_CHECK_FUNCS(lchmod, [], [
AC_MSG_WARN([
************************************
WARNING: lchmod() not available.
Program will not be able to set a
symbolic link's change time.
***********************************
])])

# Functions with replacements/alternatives
AC_CHECK_FUNCS([lutime getcwd])

#
# Finish up
#
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/libgeneral/Makefile
	lib/Makefile
	doc/Makefile])
AC_OUTPUT
AC_MSG_RESULT([
**************************************
$PACKAGE_STRING properly configured.
Debugging mode: $debugging
**************************************
You may now compile and install $PACKAGE_STRING
by entering:
> \`make'
> \`sudo make install'
])
