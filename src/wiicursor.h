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
#include <vector>

#include "auxiliary.h"
#include "matrix.h"
#include "common.h"
#include "events.h"
#include "wiicontrol.h"
#include "physics.h"


// Will be passed during events
// NOTE: These data will be not filled with correct info
// all the time. I'm well aware of that.
struct WiiEventData {
    WiiEventData(
        point_t const& ir_pos, point_t const& ir_on_mouse_down, delta_t_t const& waited
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


// More generalized functions for all start* and finish*
// NOTE: Not meant to be used directly since they rely on some common
// member variables of both WiiThreadFuncData and WiiCursorThreadData.
// Bad names, so they stand out more.
template<typename T>
void start_wiimote_related_thread( T& thread_data, void* (*thread_func)(void* ptr) ) {
    // NOTE: Not checking for any return value here
    // NOTE: Remember to figure out how to clear the Wiimote cache
    if (!thread_data.thread_running) {
	thread_data.thread_running = true;
	pthread_create(&thread_data.this_thread, 0, thread_func, &thread_data);
    }
}
template<typename T>
void finish_wiimote_related_thread(T& thread_data) {
    if (thread_data.thread_running) {
	thread_data.thread_running = false;
	// NOTE: We have to check because this_thread also gets cleared at
	// the end of the thread (and it is cleared to notify others, good reason)                                                                             
	if (thread_data.this_thread)
	    pthread_join(thread_data.this_thread, 0);
    }
}


// NOTE: Another badly named structure :-<
struct WiimoteAndTransformMatrix {
    WiimoteAndTransformMatrix(cwiid_wiimote_t* wiimote) :
	wiimote(wiimote),
	transform(TRANSFORM_MATRIX_ROWS, TRANSFORM_MATRIX_COLS)
    { }
    WiimoteAndTransformMatrix(cwiid_wiimote_t* wiimote, matrix_t const& transform) :
	wiimote(wiimote),
	transform(transform)
    { }

    cwiid_wiimote_t* wiimote;
    matrix_t transform;
};


// Helpers, help reduce duplications
// Will be passed to wii_thread_func()
struct WiiThreadFuncData {
    WiiThreadFuncData() :
	move_tolerance(0),
	wait_tolerance(0),
	this_thread(0),
	thread_running(false)
    { }

    std::vector<WiimoteAndTransformMatrix> wiimotes;
    WiiEvents events;

    unsigned int move_tolerance;
    delta_t_t wait_tolerance;

    pthread_t this_thread;
    bool thread_running;
};
// NOTE: More descriptive names would be helpful
// Creates a separate thread so the GUI will not be blocked
void* wii_thread_func(void* ptr);
void start_wii_thread(WiiThreadFuncData& data);
void finish_wii_thread(WiiThreadFuncData& data);


// Used internally by WiiCursor
struct WiiCursorThreadData {
    WiiCursorThreadData() :
	event_data(ir, ir_on_mouse_down, waited)
    { /* NOTE: Not initializing everything since I trust myself (or should I?) */ }

    bool click_and_drag() const { return moved > sqr(move_tolerance); }
    bool right_click() const { return waited > wait_tolerance; }
    bool all_running() const { return *running && thread_running; }                                                                                        

    // Data
    point_t ir;
    point_t ir_on_mouse_down;
    delta_t_t waited;
    unsigned int moved;
    bool thread_running;
    pthread_t this_thread;

    unsigned int move_tolerance;
    delta_t_t wait_tolerance;
    bool const* running;

    WiiEvents events; // Callback functions for events
    WiiEventData event_data; // To be passed to event handlers
};
void* wiicursor_thread_func(void* ptr);
void start_wiicursor_thread(WiiCursorThreadData& data);
void finish_wiicursor_thread(WiiCursorThreadData& data);
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
    WiiEvent(point_t ir, matrix_t const& transform) :
	type(WII_EVENT_TYPE_IR),
	ir(ir),
	transform(&transform) // NOTE: Never needs to check because transform is a reference
    { }
    WiiEventType type;
    uint16_t button;
    point_t ir;
    matrix_t const* transform;
};
void get_wiis_event_data(std::vector<WiimoteAndTransformMatrix>& wiimotes, point_t const& ir_old, std::vector<WiiEvent>& events);

class WiiCursor {
public:
    WiiCursor() :
	m_physics_engine(m_thread_data.ir)
    { }

    void process(
	std::vector<WiimoteAndTransformMatrix>& wiimotes,
	unsigned int move_tolerance, unsigned int wait_tolerance,
	bool const& running);

    // Events
    WiiEvents& events() {
	return m_thread_data.events;
    }
private:
    void process_ir_events(point_t ir_new, matrix_t const& transform);

    WiiCursorThreadData m_thread_data; // To be passed to wiicursor_thread_func()
    PhysicsEngine m_physics_engine;
};


#endif /* __WIICURSOR_H__ */
