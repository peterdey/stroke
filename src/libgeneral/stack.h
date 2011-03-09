/*
 *      stack.h - a general object stack
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

#ifndef LIBGENERAL_STACK_H
#define LIBGENERAL_STACK_H 1

/*
 * Element of a stack.
 */
typedef struct __STACK_ELEMENT STACK_ELEMENT; 

struct __STACK_ELEMENT {
	void *data;
	STACK_ELEMENT *next;
};

/*
 * Types of stacks
 */
typedef enum {
	NORMAL_STACK,
	ERROR_STACK /* Used by libgeneral's error facility */
} STACK_TYPE;

/*
 * Structure for a general stack.
 */
typedef struct {
	STACK_ELEMENT *top;  /* Top element */
	int count;           /* Current num of elements on stack */
	STACK_TYPE type; 	
} STACK;

/* New/destroy stack */
extern STACK* stack_new();
extern void stack_destroy(STACK **); 

/* Abstract data type operations */
extern int stack_push(STACK *, void *);
extern int stack_pop(STACK *, int destroy);
extern int stack_str_pushed(STACK *s, const char *str); 

#endif /* LIBGENERAL_STACK_H */
