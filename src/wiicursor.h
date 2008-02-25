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


// NOTE: The *only* reason this is not in wiimote_data is
// because pthread is a C library
void* thread_func(void* ptr);


class wiimote_data {
public:
    wiimote_data(point_t const& ir, unsigned int const move_tolerance, delta_t_t const wait_tolerance, bool const& running) :
	m_ir(ir),
	m_move_tolerance(move_tolerance),
	m_wait_tolerance(wait_tolerance),
	m_running(running)
    {
	set_data_at_thread_start();
	set_data_at_thread_finish();
    }

    friend void* thread_func(void* ptr); // NOTE: Friends can see your private :-<
    void start_thread(point_t const& ir_on_mouse_down);
    void finish_thread();

    point_t const& ir() const {
	return m_ir;
    }

    bool click_and_drag() const {
	return m_moved > sqr(m_move_tolerance);
    }
    bool right_click() const {
	return m_waited > m_wait_tolerance;
    }
private:
    bool running() const {
	return m_running && m_thread_running;
    }
    void set_data_at_thread_start() {
	m_waited = 0;
	m_moved = 0;
	m_thread_running = true;
    }
    void set_data_at_thread_finish() {
	m_thread_running = false;
	m_this_thread = 0;
    }

    point_t const& m_ir;
    point_t m_ir_on_mouse_down;
    delta_t_t m_waited;
    unsigned int m_moved;
    bool m_thread_running;
    pthread_t m_this_thread;

    unsigned int const m_move_tolerance;
    delta_t_t const m_wait_tolerance;

    bool const& m_running;
};


class WiiCursor {
public:
    WiiCursor(cwiid_wiimote_t* wiimote, matrix_t transform) :
	m_wiimote(wiimote),
	m_transform(transform)
    { }
    void process(bool& running);
private:
    cwiid_wiimote_t* const m_wiimote;
    matrix_t const m_transform;
};


#endif /* __WIICURSOR_H__ */
