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
#include <vector>
#include <glibmm.h>

#include "configurator.h"
#include "auxiliary.h"
#include "common.h"
#include "events.h"
#include "wiicontrol.h"
#include "irfilter.h"


// Will be passed during events
// NOTE: These data will be not filled with correct info
// all the time. I'm well aware of that.
struct WiiEventData {
    WiiEventData(
        point_t const& ir_pos, point_t const& ir_on_mouse_down,
	delta_t_t const& waited, unsigned int const& move_tolerance
    ) :
        ir_pos(ir_pos),
        ir_on_mouse_down(ir_on_mouse_down),
        waited(waited),
	move_tolerance(move_tolerance)
    { }

    point_t const& ir_pos;
    point_t const& ir_on_mouse_down;
    point_t cursor_pos;
    delta_t_t const& waited;
    unsigned int const& move_tolerance;
};
// Supported events
// WARNING: Do *not* attempt to update any GTK+ elements
// during those events.
struct WiiEvents {
    typedef sigc::slot<void, WiiEventData const&> WiiEventSlotType;
    WiiEventSlotType    left_clicked, right_button_down, right_button_up,
                        begin_click_and_drag, end_click_and_drag,
                        mouse_down, mouse_up, mouse_moved;
};


// Will be passed to wiicursor_thread_func()
struct WiiThreadFuncData {
    WiiThreadFuncData(std::vector<WiimoteData>& wiimotes) :
	wiimotes(wiimotes),
	this_thread(0),
	thread_running(false)
    { }

    std::vector<WiimoteData>& wiimotes;
    WiiEvents events;

    pthread_t this_thread;
    bool thread_running;
};
// Creates a separate thread so the GUI will not be blocked
void* wiicursor_thread_func(void* ptr);
void start_wiicursor_thread(WiiThreadFuncData& data);
void finish_wiicursor_thread(WiiThreadFuncData& data);


// Used internally by WiiCursor
enum WiiEventType {
    WII_EVENT_TYPE_BUTTON,
    WII_EVENT_TYPE_IR,
    WII_EVENT_TYPE_NON_EVENT, // NOTE: Not returned by wiicontrol.cpp, but used by get_wiis_event_data()
};
struct WiiEvent {
    WiiEvent() :
	type(WII_EVENT_TYPE_NON_EVENT)
    { }
    WiiEvent(uint16_t button) :
	type(WII_EVENT_TYPE_BUTTON),
	button(button)
    { }
    WiiEvent(point_t ir, matrix_t const* transform) :
	type(WII_EVENT_TYPE_IR),
	ir(ir),
	transform(transform)
    { }
    WiiEventType type;
    uint16_t button;
    point_t ir;
    matrix_t const* transform;
};
void get_wiis_event_data(std::vector<WiimoteData>& wiimotes, point_t const& ir_old, std::vector<WiiEvent>& events);
// Used by get_wiis_event_data()
void get_wiis_event_data_collect_all_events(
    std::vector<WiimoteData>& wiimotes,
    point_t const& ir_old,
    std::vector< std::vector<WiiEvent> >& event_batches,
    unsigned int& max_event_counts);
void get_wiis_event_data_rebalance_events(unsigned int max_event_counts, std::vector< std::vector<WiiEvent> >& event_batches);
void get_wiis_event_data_process_events(
    std::vector< std::vector<WiiEvent> > const& event_batches,
    unsigned int current_event_index,
    std::vector<WiimoteData> const& wiimotes,
    point_t const& ir_old,
    std::vector<point_t>& irs,
    std::vector<matrix_t const*>& transforms,
    std::vector<WiiEvent>& events);

class WiiCursor {
public:
    WiiCursor() :
	m_ir_filter(m_thread_data.ir, m_thread_data.move_tolerance)
    { }

    void process(std::vector<WiimoteData>& wiimotes, bool const& running);

    // Events
    WiiEvents& events() {
	return m_thread_data.events;
    }
private:
    void process_ir_events(point_t ir_new, matrix_t const* transform);

    // Thread stuff
    struct WiiCursorThreadData {
	WiiCursorThreadData() :
	    ir(INVALID_IR_POS, 0),
	    ir_on_mouse_down(INVALID_IR_POS, 0),
	    waited(0),
	    moved(0),
	    thread_running(false),
	    this_thread(0),
	    move_tolerance(0),
	    wait_tolerance( get_configurator().wait_tolerance() ),
	    running(0),
	    event_data(ir, ir_on_mouse_down, waited, move_tolerance)
	{ }

	bool click_and_drag() const { return moved > sqr(move_tolerance); }
	bool right_click() const { return waited > wait_tolerance; }
	bool all_running() const { return *running && thread_running; }

	// Data
	point_t ir;
	point_t ir_on_mouse_down;
	delta_t_t waited;
	unsigned int moved;
	bool thread_running;
	Glib::Thread* this_thread;

	unsigned int move_tolerance;
	delta_t_t const& wait_tolerance;
	bool const* running;

	WiiEvents events; // Callback functions for events
	WiiEventData event_data; // To be passed to event handlers
    } m_thread_data;
    void right_click_thread_func();
    void start_right_click_thread();
    void finish_right_click_thread();

    // Other data
    IrFilter m_ir_filter;
};


#endif /* __WIICURSOR_H__ */
