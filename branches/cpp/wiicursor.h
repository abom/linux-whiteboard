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


#ifndef  __WIICURSOR_H__
#define  __WIICURSOR_H__


#include <cwiid.h>
#include <pthread.h>

#include "auxiliary.h"
#include "matrix.h"
#include "common.h"
#include "events.h"
#include "wii.h"


// NOTE: Return values and error notification are still lacking, feel free to improve it


struct click_stuff {
    point_t ir_on_mouse_down;
    delta_t_t waited;
    unsigned int moved;
    bool thread_finished;
    pthread_t this_thread;

    unsigned int const move_tolerance;
    delta_t_t const wait_tolerance;

    point_t const& ir;
    bool const& program_finished;

    click_stuff(unsigned int const move_tolerance, delta_t_t const wait_tolerance, point_t const& ir, bool const& program_finished) :
	move_tolerance(move_tolerance),
	wait_tolerance(wait_tolerance),
	ir(ir),
	program_finished(program_finished)
    {
	set_data_at_thread_start();
	set_data_at_thread_finish();
    }

    void set_data_at_thread_start() {
	waited = 0;
	moved = 0;
	thread_finished = false;
    }
    void set_data_at_thread_finish() {
	thread_finished = true;
	this_thread = 0;
    }
};


void* click_thread(void* ptr);

void start_click_thread(click_stuff& data, point_t const& ir_on_mouse_down);
void finish_click_thread(click_stuff& data);


class WiiCursor {
public:
    WiiCursor(cwiid_wiimote_t* wiimote, matrix_t transform) :
	m_wiimote(wiimote),
	m_transform(transform)
    { }
    void process();
private:
    cwiid_wiimote_t* const m_wiimote;
    matrix_t const m_transform;
};


#endif /* __WIICURSOR_H__ */
