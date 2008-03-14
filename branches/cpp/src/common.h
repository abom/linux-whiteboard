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


template<typename T>
struct Point {
    Point() :
	x( T() ),
	y( T() )
    { }
    Point(T x, T y) :
	x(x),
	y(y)
    { }

    T x, y;                                                                                                                                                 
};
typedef Point<int> point_t; // The default


enum MouseButtonIDs {
    LEFT_BUTTON = 1,
    RIGHT_BUTTON = 3,
};

enum InvalidWiiEventValues {
    INVALID_IR_POS = 1, // To say there's no IR event, or an IR_OFF event
    INVALID_BUTTON_MSG_ID = 1337, // To say there's no button event
};

enum TransformMatrixDimensions {
    TRANSFORM_MATRIX_ROWS = 3,
    TRANSFORM_MATRIX_COLS = 3,
};


#endif   /* __COMMON_H__ */
