/*
 *      stack.c - A general purpose stack  
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

#include <libgeneral/stack.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <libgeneral/error.h>
#include <libgeneral/general.h>

/* Stack malloc */
void *smalloc(size_t s) {
	return general_malloc(s); 
} 

/**
 * Create and return new stack
 */
STACK* stack_new() {
	STACK *s = (STACK*)smalloc(sizeof(STACK));
	s->top = NULL;
	s->count = 0;
	s->type = NORMAL_STACK;
	return s;
}

/** 
 * Destroy and free stack
 */
void stack_destroy(STACK **s) 
{
	if( !*s ) {
		errwrn(ERROR_FATAL, 0, FLN, LIBGENERAL_ERROR_PREFIX "Destroying stack failed");	
		return;
	} else {
		/* traverse and remove stack elements */
		for( ; (*s)->count > 0; stack_pop(*s, 1) );
		free(*s);
		*s = NULL;
	}
}

/**
 * Returns 0 if successfully pushed onto stack, otherwise -1
 */
int stack_push(STACK *s, void *data) 
{
	STACK_ELEMENT *e;
	assert(s != NULL && data != NULL);
	
	if( !s ) {
		errwrn(ERROR_FATAL, 0, FLN, LIBGENERAL_ERROR_PREFIX "Pushing into stack failed");
		return -1;
	}
	
	e = s->top;
	s->top = smalloc(sizeof(STACK_ELEMENT));
	
	if( !s->top ) return -1;
	
	s->top->next = e;
	s->top->data = data;
	++s->count;

	return 0;
}

/**
 * Pop top element, return 0 on success, -1 on failure.
 * If destroy is > 0 then data on the stack will be 
 * entirely free'd. If destory is not set, they will 
 * remain in memory.
 */
int stack_pop(STACK *s, int destroy) 
{
	STACK_ELEMENT *e;
	
	if( !s ) {
		errwrn(ERROR_FATAL, 0, FLN, LIBGENERAL_ERROR_PREFIX "Popping top off stack failed");	
		return -1;
	}
	
	/* Empty stack needs no popping */
	if( s->count < 1 || !s->top ) return -1;
	
	if( destroy &&  s->top->data ) {
		if( s->type == ERROR_STACK ) {
			clean_error_message(s->top->data);	
		} else if( s->type == NORMAL_STACK ) {
			free(s->top->data);
		}
		s->top->data = NULL;
	}
	
	e = s->top->next; 
	free(s->top);
	s->top = e;
	
	--s->count;
	
	return 0;
}

/**
 * If stack contains strings this function checks whether *str is
 * contained on stack. Return 1 if so, 0 if not, and -1 if an error was
 * encountered.
 */
int stack_str_pushed(STACK *s, const char *str) 
{
	STACK_ELEMENT *e;
	if( !s ) {
		errwrn(ERROR_FATAL, 0, FLN, LIBGENERAL_ERROR_PREFIX "Checking stack for string failed");	
		return -1;
	}
	
	for( e = s->top; e; e = e->next ) {
		if(!e->data) continue;
		if(!strncmp(str, e->data, strlen(str))) return 1;
	}

	return 0;
}
