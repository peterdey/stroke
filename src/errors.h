/*
 *      errors.h - stroke error codes
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

#ifndef STROKE_ERRORS_H
#define STROKE_ERRORS_H 1

#include <libgeneral/error.h>

enum {
	/* Fatal errors 1 to 99 */
	ERROR_FATAL_SEGV = 2,
	
	/* Warnings 100 to 199 */
	ERROR_WARNING_FORCVAL = 101,
	ERROR_WARNING_CTCOPY = 102,
	
	/* Normal errors 200 and beyond */
	ERROR_ERROR_INSUFARGS = 201,
	ERROR_ERROR_UKNARG = 202,
	ERROR_ERROR_MODFIL = 203,
	ERROR_ERROR_INVMOD = 204,
	ERROR_ERROR_INVFIL = 205,
	ERROR_ERROR_STAT = 207,
	ERROR_ERROR_GMTIM = 208,
	ERROR_ERROR_VALDAT = 209,
	ERROR_ERROR_TSTMP = 210,
	ERROR_ERROR_RESOLV = 211,
	ERROR_ERROR_SETVAL = 212,
	ERROR_ERROR_SETTIM = 213,
	ERROR_ERROR_INSUFA = 214,
	ERROR_ERROR_FOPEN = 215,
	ERROR_ERROR_TOOMA = 216,
	ERROR_ERROR_MFIND = 217,
	ERROR_ERROR_MINVAL = 218,
	ERROR_ERROR_GETTD = 219,
	ERROR_ERROR_BATCHF = 220,
	ERROR_ERROR_TIMEST = 221,
	ERROR_ERROR_INVTSP = 222,
	ERROR_ERROR_UTIMSYM = 223,
	ERROR_ERROR_CHCTIME = 224,
	ERROR_ERROR_CTPRES = 225,
	ERROR_ERROR_CTCHPR = 226,
	ERROR_ERROR_INVCOMB = 227,
	ERROR_ERROR_INFMODF = 228,
	ERROR_ERROR_FCREATE = 229,
	ERROR_ERROR_CTPRIV = 230,
	ERROR_ERROR_SETTIM_PERM = 231,
};

/* Array of error messages; used by libgeneral; initialized in errors.c */
extern ERROR_MESSAGE error_messages[];

#endif /* STROKE_ERRORS_H */
