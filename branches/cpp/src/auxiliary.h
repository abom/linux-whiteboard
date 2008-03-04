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


#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>

#include "matrix.h"
#include "common.h" // ASSERT

// NOTE: Due to glibmm, it must be included after glibmm.h. Try moving to the top.
// Yeah, this is bad.
#include <X11/extensions/XTest.h>


struct OptionStates {
    char mac[18];
    bool will_calibrate;
    bool show_help;

    OptionStates() :
	will_calibrate(false),
	show_help(false)
    { }
};

void show_help();

point_t screen_size();
void screen_corners(point_t p_screen[4]);

matrix_t calculate_transformation_matrix(point_t const p_wii[4]);

/* Returns how long the time has passed since last event in milliseconds
 * I removed the one with static variable, it's a bad practice
 * NOTE: Both must be called ONCE before using for accurate result */
typedef unsigned long long delta_t_t;
delta_t_t get_delta_t(delta_t_t& last_time);
//delta_t_t get_delta_t();

/* This function transforms the provided infrared coordinates to screen's
 * coordinates using the transformation matrix built earlier */
point_t infrared_data(point_t const& ir_pos_new, matrix_t const& transform);


unsigned int sqr(int n);
unsigned int squared_distance(point_t const& p1, point_t const& p2);

std::string config_file_path();
/* Isn't it obvious? :)
 * Returns true on success */
bool load_config(matrix_t& transform);
bool save_config(matrix_t const& transform);

/* Returns true on success */
bool process_options(int argc,char *argv[], OptionStates& opt_states);


#endif /* __AUXILIARY_H__ */
