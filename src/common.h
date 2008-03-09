/* Copyright (C) 2008 Pere Negre
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or                                                                                         
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* This file contains the commonly used macros and data structures */

#ifndef  __COMMON_H__
#define  __COMMON_H__


#include <assert.h> // assert

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "gettext.h"
#define _(String) gettext (String)


#define ASSERT(exp, msg) { \
    int const p = exp; \
    if (!p) \
    { \
	printf ("\nAssertion '%s' FAILED at line '%d' of function '%s' in file '%s': '%s'.\n", #exp, __LINE__, __FUNCTION__, __FILE__, msg); \
	assert (1); \
    } \
}


struct Point {
    Point() :
	x(0),
	y(0)
    { }
    Point(int x, int y) :
	x(x),
	y(y)
    { }

    int x, y;                                                                                                                                                 
};
typedef Point point_t;


// Mouse button IDs
#define LEFT_BUTTON static_cast<int>(1)
#define RIGHT_BUTTON static_cast<int>(3)


// To say there's no IR event, or an IR_OFF event
#define INVALID_IR_POS static_cast<int>(-1)
// To say there's no button event
#define INVALID_BUTTON_MSG_ID static_cast<unsigned int>(1337)


#endif   /* __COMMON_H__ */
