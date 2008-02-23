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


#ifndef  __GUI_H__
#define  __GUI_H__


#include <SDL/SDL.h>
#include <SDL/sge.h>

#include "wii.h"
#include "auxiliary.h"
#include "common.h"


#define SDL_COLOR_WHITE static_cast<unsigned int>(0xffffff)
#define SDL_COLOR_BLACK static_cast<unsigned int>(0x0)
#define SDL_COLOR_RED static_cast<unsigned int>(0xff0000)
#define SDL_COLOR_YELLOW static_cast<unsigned int>(0xffff00)
#define SDL_COLOR_GREEN static_cast<unsigned int>(0x00ff00)


point_t screen_size();
void screen_corners(point_t p_screen[4]);

/* Draws the center of a calibration point */
void draw_point(SDL_Surface* surface, point_t const& p, unsigned int radius);
/* Basic figures
 * NOTE: I just reused point_t with dim, but that's not really desirable since
 * dimensions should be unsigned */
void draw_rectangle(SDL_Surface* surface, point_t const& p, point_t dim, Uint32 color);
void draw_square(SDL_Surface* surface, point_t const& p, unsigned int radius, Uint32 color);
/* Draws the 4 calibration points, paints the
 * active point a blinking square, and paints the
 * calibrated ones static squares */
void draw_calibration_points(SDL_Surface* surface, point_t const points[4], int active, int active_light_up);
/* Get 4 calibration points from users
 * Points are written to p_wii
 * Returns 0 on success, -1 on error or user escapes */
int get_calibration_points(cwiid_wiimote_t* wiimote, point_t p_wii[4]);

void print_points(point_t const p_wii[4]);


#endif /* __GUI_H__ */
