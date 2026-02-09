#
# Makefile.am for stroke
# (C) 2009 by Soeren Wellhoefer <soeren.wellhoefer@gmx.net> under GPL
#

# Binary
bin_PROGRAMS = stroke

# Source files
stroke_headers = stroke.h errors.h
stroke_sources = aux.c errors.c stroke.c gnulib/parse-datetime.c gnulib/timespec-extra.c
stroke_SOURCES = $(stroke_headers) $(stroke_sources)

# Libraries
SUBDIRS = libgeneral
stroke_LDADD = libgeneral/libgeneral.a $(top_builddir)/lib/libgnu.a

# Preprocessor flags
AM_CPPFLAGS = $(EXTRA_FLAGS) -Ilibgeneral -I$(top_srcdir)/lib -I$(srcdir)/gnulib

# Ensure we can rely on C11 features
AM_CFLAGS = -std=gnu11
