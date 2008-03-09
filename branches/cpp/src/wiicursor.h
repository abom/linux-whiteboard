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
#include <sigc++/sigc++.h>
#include <memory>

#include "auxiliary.h"
#include "matrix.h"
#include "common.h"
#include "events.h"
#include "wii.h"


// NOTE: These classes have quite a few ways to pass data
// to the others, which means redundancy. Whether or not
// it is good, I'm not sure.


// NOTE: The *only* reason this is not in wiimote_data is
// because pthread is a C library
void* wiicursor_thread_func(void* ptr);


// Will be passed during events
// NOTE: These data will be not filled with correct info
// all the time because it's not necessary. I'm well aware of that.
struct WiimoteEventData {
    WiimoteEventData(
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
typedef sigc::slot<void, WiimoteEventData const&> WiiEventSlotType;

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
    WiiEventSlotType& signal_left_clicked() { return m_fn_left_clicked; }
    WiiEventSlotType& signal_right_button_down() { return m_fn_right_button_down; }
    WiiEventSlotType& signal_right_button_up() { return m_fn_right_button_up; }
    WiiEventSlotType& signal_begin_click_and_drag() { return m_fn_begin_click_and_drag; }
    WiiEventSlotType& signal_end_click_and_drag() { return m_fn_end_click_and_drag; }
    WiiEventSlotType& signal_mouse_down() { return m_fn_mouse_down; }
    WiiEventSlotType& signal_mouse_up() { return m_fn_mouse_up; }
    WiiEventSlotType& signal_mouse_moved() { return m_fn_mouse_moved; }

private:
    friend void* wiicursor_thread_func(void* ptr); // NOTE: Friends can see your private :-<

    void start_thread(point_t const& ir_on_mouse_down);
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
    WiiEventSlotType	m_fn_left_clicked,
			m_fn_right_button_down, m_fn_right_button_up,
			m_fn_begin_click_and_drag, m_fn_end_click_and_drag,
			m_fn_mouse_down, m_fn_mouse_up, m_fn_mouse_moved;
    WiimoteEventData m_wii_event_data; // To pass to event handlers
};


#endif /* __WIICURSOR_H__ */
