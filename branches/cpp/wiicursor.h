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
	waited(0),
	moved(0),
	move_tolerance(move_tolerance),
	wait_tolerance(wait_tolerance),
	ir(ir),
	program_finished(program_finished)
    {
	set_data_at_thread_end();
    }

    void set_data_at_thread_start() {
	waited = 0;
	moved = 0;
	thread_finished = false;
    }
    void set_data_at_thread_end() {
	thread_finished = true;
	this_thread = 0;
    }
};


void* click_thread(void* ptr);

void ask_thread_to_finish(click_stuff& data); // Yeah, I know it's weird


class WiiCursor {
public:
    WiiCursor(cwiid_wiimote_t* wiimote, matrix_t transform) :
	m_wiimote(wiimote),
	m_transform(transform),
	m_program_finished(true),
	m_click_data( 5, 700, m_ir, m_program_finished )
    { }
    void process();
private:
    cwiid_wiimote_t* const m_wiimote;
    matrix_t const m_transform;

    // NOTE: These cannot be local variables because they don't work
    // very well with references (locals got stored on the stack -
    // well, I know it depends, but I'd better play safe)
    bool m_program_finished;
    point_t m_ir;
    click_stuff m_click_data;
};


#endif /* __WIICURSOR_H__ */
