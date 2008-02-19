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


#include "auxiliary.h"


matrix_t calculate_transformation_matrix(point_t const p_wii[4]) {
    printf("Calculating coefficients... ");

    point_t p_screen[4];
    screen_corners(p_screen);
    matrix_t m(8, 8), n(1, 8);

    for (int i = 0; i != 4; ++i)
    {
	n[0][i*2] = p_screen[i].x;
	n[0][i*2+1] = p_screen[i].y;
    }

    for (int i = 0; i != 4; ++i)
    {
	m[0][i*2] = p_wii[i].x;
	m[1][i*2] = p_wii[i].y;
	m[2][i*2] = 1;
	for (int j = 3; j != 6; ++j)
	    m[j][i*2] = 0;
	m[6][i*2] = -p_screen[i].x * p_wii[i].x;
	m[7][i*2] = -p_screen[i].x * p_wii[i].y;

	for (int j = 0; j != 3; ++j)
	    m[j][i*2+1] = 0;
	m[3][i*2+1] = p_wii[i].x;
	m[4][i*2+1] = p_wii[i].y;
	m[5][i*2+1] = 1;
	m[6][i*2+1] = -p_screen[i].y * p_wii[i].x;
	m[7][i*2+1] = -p_screen[i].y * p_wii[i].y;
    }

    matrix_t out(3, 3);
    matrix_t const r = m.invert() * n;
    memcpy(out.elems(), r.elems(), 8*sizeof(matrix_elem_t));

    printf("Done!\n");

    return out;
}


// WARNING: Pay attention to the limit of these data types
delta_t_t get_delta_t(delta_t_t& last_time) {
    timeval current = {0, 0};
    gettimeofday(&current, 0);
    delta_t_t const current_time =
	static_cast<delta_t_t>( static_cast<double>(current.tv_sec)*1000.0 + static_cast<double>(current.tv_usec)/1000.0 );

    ASSERT (current_time >= last_time, "Time is going backwards. omgwtf!!!1");
    delta_t_t const ret = current_time - last_time;
    last_time = current_time;

    return ret;
}
/*delta_t_t get_delta_t() {
    static delta_t_t last_time = 0;

    return get_delta_t(last_time);
}*/


point_t infrared_data(point_t const& ir_pos_new, matrix_t const& transform) {
    point_t ir_pos;

    matrix_t const& t = transform; // For readability
    ir_pos.x = static_cast<int> (
	    (t[0][0]*ir_pos_new.x + t[0][1]*ir_pos_new.y + t[0][2]) /
	    (t[2][0]*ir_pos_new.x + t[2][1]*ir_pos_new.y + 1.0) );
    ir_pos.y = static_cast<int> (
	    (t[1][0]*ir_pos_new.x + t[1][1]*ir_pos_new.y + t[1][2]) /
	    (t[2][0]*ir_pos_new.x + t[2][1]*ir_pos_new.y + 1.0) );

    point_t const scr_size = screen_size();
    if (ir_pos.x<0)       ir_pos.x = 0; 
    if (ir_pos.x>=scr_size.x)  ir_pos.x = scr_size.x-1;
    if (ir_pos.y<0)       ir_pos.y = 0;
    if (ir_pos.y>=scr_size.y)  ir_pos.y = scr_size.y-1;

    return ir_pos;
}


unsigned int sqr(int n) {
    return static_cast<unsigned int>(n*n);
}
unsigned int squared_distance(point_t const& p1, point_t const& p2) {
    return sqr(p1.x - p2.x) + sqr(p1.y - p2.y);
}


/* NOTE: Not necessary because load_config is going to fail if 'false' anyway */
/* bool config_file_exists() {
    // WARNING: Actually I only check whether the file can be opened for reading,
    // it doesn't return the correct error message if it was caused by something
    // else (permissions, I/O errors, etc)
    std::ifstream in(".whiteboardrc"); // WARNING: Hard-coding the file for now
    return in.is_open();
} */
bool load_config(matrix_t& transform) {
    std::ifstream in(".whiteboardrc"); // WARNING: Hard-coding the file for now
    if ( in.is_open() ) {
	in >> transform;

	std::cout << "Finished loading configurations" << std::endl;
	return true;
    }

    std::cout << "Loading configuration file failed" << std::endl;
    return false;
}
bool save_config(matrix_t const& transform) {
    std::ofstream out(".whiteboardrc"); // WARNING: Hard-coding the file for now
    if ( out.is_open() ) {
	out << transform;

	std::cout << "Finished saving configurations" << std::endl;
	return true;
    }

    std::cout << "Saving configuration file failed" << std::endl;
    return false;
}
