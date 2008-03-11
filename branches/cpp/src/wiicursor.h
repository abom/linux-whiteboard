/* Copyright (C) 2008 Tu Anh Vuong
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
#include <sigc++/sigc++.h>
#include <memory>

#include "auxiliary.h"
#include "matrix.h"
#include "common.h"
#include "events.h"
#include "wiicontrol.h"


// NOTE: These classes have quite a few ways to pass data
// to the others, which means redundancy. Whether or not
// it is good, I'm not sure.


// Will be passed during events
// NOTE: These data will be not filled with correct info
// all the time. I'm well aware of that.
struct WiiEventData {
    WiiEventData(
        point_t const& ir_pos, point_t const& ir_on_mouse_down, point_t cursor_pos, delta_t_t const& waited
    ) : 
        ir_pos(ir_pos),
        ir_on_mouse_down(ir_on_mouse_down),
        waited(waited)
    { }

    point_t const& ir_pos;
    point_t const& ir_on_mouse_down;
    point_t cursor_pos;
    delta_t_t const& waited;
};
typedef sigc::slot<void, WiiEventData const&> WiiEventSlotType;

// Supported events
struct WiiEvents {
    WiiEventSlotType    left_clicked, right_button_down, right_button_up,
                        begin_click_and_drag, end_click_and_drag,
                        mouse_down, mouse_up, mouse_moved;
};


// Helpers, help reduce duplications
// Will be passed to wii_thread_func_helper()
struct WiiThreadFuncData {
    WiiThreadFuncData() :
	wiimote(0),
	transform(TRANSFORM_MATRIX_ROWS, TRANSFORM_MATRIX_COLS),
	move_tolerance(0),
	wait_tolerance(0),
	this_thread(0),
	thread_running(false)
    { }

    cwiid_wiimote_t* wiimote;
    WiiEvents events;
    matrix_t transform;

    unsigned int move_tolerance;
    delta_t_t wait_tolerance;

    pthread_t this_thread;
    bool thread_running;
};
// NOTE: More descriptive names would be helpful
void* wii_thread_func(void* ptr);
void start_wii_thread(WiiThreadFuncData& thread_data);
void finish_wii_thread(WiiThreadFuncData& thread_data);


// Used internally by WiiCursor
void* wiicursor_thread_func(void* ptr);


class WiiCursor {
public:
    WiiCursor() :
	m_wii_event_data(m_ir, m_ir_on_mouse_down, point_t(), m_waited)
    { }

    void process(
	cwiid_wiimote_t* wiimote, matrix_t transform,
	unsigned int move_tolerance, unsigned int wait_tolerance,
	bool& running);

    // Events
    WiiEvents& events() {
	return m_wii_events;
    }
private:
    // NOTE: This needs refactoring like wii_thread_func()
    friend void* wiicursor_thread_func(void* ptr); // NOTE: Friends can see your private :-<

    void start_thread();
    void finish_thread();

    bool click_and_drag() const { return m_moved > sqr(m_move_tolerance); }
    bool right_click() const { return m_waited > m_wait_tolerance; }
    bool all_running() const { return *m_running && m_thread_running; }

    void set_data_at_thread_start() {
        m_waited = 0;
        m_moved = 0;
        m_thread_running = true;
    }
    void set_data_at_thread_finish() {
        m_thread_running = false;
        // NOTE: Not bothering to clear the thread ID here
    }

    // Data
    point_t m_ir;
    point_t m_ir_on_mouse_down;
    delta_t_t m_waited;
    unsigned int m_moved;
    bool m_thread_running;
    pthread_t m_this_thread;

    unsigned int m_move_tolerance;
    delta_t_t m_wait_tolerance;
    bool* m_running;

    // Callback functions for events
    WiiEvents m_wii_events;
    WiiEventData m_wii_event_data; // To pass to event handlers
};


#endif /* __WIICURSOR_H__ */
