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


#ifndef  __AUXILIARY_H__
#define  __AUXILIARY_H__


#include <X11/extensions/XTest.h>
#include <sys/time.h>

#include "matrix.h"
#include "gui.h"
#include "common.h" // ASSERT


matrix_t calculate_transformation_matrix(point_t const p_wii[4]);

/* Returns how long the time has passed since last event in milliseconds
 * The first function doesn't use a static variable, it updates last_time every call
 * NOTE: Both must be called ONCE before using for accurate result */
typedef unsigned long long delta_t_t;
delta_t_t get_delta_t(delta_t_t& last_time);
delta_t_t get_delta_t();

/* This function transforms the provided infrared coordinates to screen's
 * coordinates using the transformation matrix built earlier */
point_t infrared_data(point_t const& ir_pos_new, matrix_t const& transform);


unsigned int sqr(int n);
unsigned int squared_distance(point_t const& p1, point_t const& p2);


#endif /* __AUXILIARY_H__ */
