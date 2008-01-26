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


#include <SDL.h>
#include <X11/extensions/XTest.h>
#include <sys/time.h>
#include <unistd.h> /* sleep */

#include "matrix.h"
#include "common.h"


extern point_t ir_pos;
extern point_t p_wii[4];

point_t screen_size();
void screen_corners(point_t p_screen[4]);

/* Puts a white pixel */
void pixel(SDL_Surface* surface, int x, int y);
/* Draws the center of a calibration point */
void draw_point(SDL_Surface* surface, point_t p);
/* Draws a square around a calibration point */
void draw_square(SDL_Surface* surface, point_t p);
/* Draws the 4 calibration points, paint the                                                                                                                  
 * active point a blinking square, and paint the
 * calibrated ones static squares */
void draw_calibration_points(SDL_Surface* surface, point_t points[4], int active, int active_light_up);
/* Get 4 calibration points from users
 * Returns 0 on success, -1 on error or user escapes */
int get_calibration_points();

void print_points();

matrix_t *calculate_transformation_matrix();

void fake_move(int x, int y);
void fake_button(int button, int is_press);

/* In microseconds
 * NOTE: Must be called ONCE before using */
unsigned long get_ticks();


#endif   /* __AUXILIARY_H__ */
