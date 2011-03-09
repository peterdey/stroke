/*
 *      signals.c - Implementing the signal.h interface
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
#  include <config.h>
#else
#  define RETSIGTYPE void
#endif

#include <stdlib.h>

#include <libgeneral/signals.h>

int _set_jmp_val;

jmp_buf _jump_buffer;

static volatile siginfo_t _signal_info;

static volatile ucontext_t _user_context;

static RETSIGTYPE 
_universal_signal_handler(int signum, siginfo_t *sinfo, void *ucontext) {
	/* Copy signal information */
	_signal_info = *sinfo;
	_user_context = *(ucontext_t*)ucontext;
	longjmp(_jump_buffer, signum);	
}

/** 
 * Sets a signal to be caught by the universal signal handler.
 * This is a function used by the user of this interface.
 */
int SET_SIGNAL(int signum, sigset_t *mask, int flags) {
	struct sigaction sa;
	
	/* Set signal handler */
	sa.sa_sigaction = &_universal_signal_handler;
	
	/* Set signal mask for signal */
	sigemptyset(&sa.sa_mask);
	if( mask ) sa.sa_mask = *mask;
	
	/* Set signal handling flags */
	sa.sa_flags = SA_SIGINFO | SA_RESTART | flags;
	
	return sigaction(signum, &sa, NULL);
}

siginfo_t *GET_SIGINFO() {
	return (siginfo_t*)&_signal_info;
}

ucontext_t *GET_UCONTEXT() {
	return (ucontext_t*)&_user_context;
}



