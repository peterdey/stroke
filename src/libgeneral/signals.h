/*
 *      signals.h - Signal catching routines and macros
 * 	
 * 	       This interface allows a user to have signals that are sent to the application
 *		be handled in the main execution thread of the program. This eliminates the problem
 *		of thread safety concerns when executing an ordinary signal handler.
 *
 *		Warning: A program's state might be undefined when a signal is caught.
 *      
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

#ifndef LIBGENERAL_SIGNALS_H
#define LIBGENERAL_SIGNALS_H 1

#include <setjmp.h>
#include <signal.h>
#include <ucontext.h>

/* The catch signal macro; used in an application as an entry point if
 * the signal which was initialized beforehand, is caught.
 */
#define SIGNAL_CATCHING() \
	_set_jmp_val = setjmp(_jump_buffer)

#define CATCH_SIGNAL(SIGNUM) \
	if( _set_jmp_val == SIGNUM )

extern int _set_jmp_val;
extern jmp_buf _jump_buffer;

/* Prototypes */
extern int SET_SIGNAL(int signum, sigset_t *mask, int flags);
extern siginfo_t *GET_SIGINFO();
extern ucontext_t *GET_UCONTEXT();

#endif /* LIBGENERAL_SIGNALS_H */
