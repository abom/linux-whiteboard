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


#include "wiicursor.h"


void* wii_thread_func(void* ptr) {
    ASSERT(ptr != 0, "No data has been passed along");
    WiiThreadFuncData& data = *static_cast<WiiThreadFuncData*>(ptr);
    ASSERT(data.this_thread != 0, "This thread has become immortal, omfg!!!1");
                                                                                                                                                               
    WiiCursor wc;
    wc.events() = data.events;
    wc.process(data.wiimotes, data.move_tolerance, data.wait_tolerance, data.thread_running); // The main loop

    for (std::vector<WiimoteAndTransformMatrix>::iterator iter = data.wiimotes.begin(); iter != data.wiimotes.end(); ++iter)
	cwiid_disable(iter->wiimote, CWIID_FLAG_MESG_IFC);
    data.this_thread = 0;

    return 0;
}
void start_wii_thread(WiiThreadFuncData& data) {                                                                                                               
    start_wiimote_related_thread(data, &wii_thread_func);
}
void finish_wii_thread(WiiThreadFuncData& data) {
    finish_wiimote_related_thread(data);
}


void* wiicursor_thread_func(void* ptr) {
    ASSERT(ptr != 0, "No data has been passed along");
    WiiCursorThreadData& data = *static_cast<WiiCursorThreadData*>(ptr);
    ASSERT(data.this_thread != 0, "This thread has become immortal, omfg!!!1");

    // Sets up the timer
    delta_t_t last_time = 0;
    get_delta_t(last_time);

    // Sets up itself
    data.moved = 0;
    data.waited = 0;

    // Readability
    WiiEvents& wii_events = data.events;
    WiiEventData& wii_event_data = data.event_data;
    while ( data.all_running() ) {
	if ( data.right_click() ) {
		wii_events.right_button_down(wii_event_data);
	    break;
	}
	else data.waited += get_delta_t(last_time);

	if ( data.click_and_drag() ) {
		wii_events.begin_click_and_drag(wii_event_data);
	    break;
	}
	else {
	    // WARNING: A locking mechanism is needed to avoid this hack
	    // data.m_ir.x should never be invalid
	    // m_ir must not be read and updated at the same time in this function and in process()
	    if (data.ir.x != INVALID_IR_POS)
		data.moved = squared_distance(data.ir, data.ir_on_mouse_down);
	}
    }

    data.this_thread = 0;

    return 0;
}
void start_wiicursor_thread(WiiCursorThreadData& data) {
    start_wiimote_related_thread(data, &wiicursor_thread_func);                                                                                                
}
void finish_wiicursor_thread(WiiCursorThreadData& data) {
    finish_wiimote_related_thread(data);
}


void WiiCursor::process(
	std::vector<WiimoteAndTransformMatrix>& wiimotes,
        unsigned int move_tolerance, unsigned int wait_tolerance,
        bool const& running)
{
    // Sets up the Wiimotes
    for (std::vector<WiimoteAndTransformMatrix>::iterator iter = wiimotes.begin(); iter != wiimotes.end(); ++iter) {
	cwiid_enable(iter->wiimote, CWIID_FLAG_MESG_IFC);
	cwiid_disable(iter->wiimote, CWIID_FLAG_NONBLOCK);
    }

    // Sets up itself
    m_thread_data.move_tolerance = move_tolerance;
    m_thread_data.wait_tolerance = wait_tolerance;
    m_thread_data.running = &running; // No need to check anything here since running is a reference

    while (running) {
	std::vector<WiiEvent> events;
	get_wiis_event_data(wiimotes, m_thread_data.ir, events);
	for (std::vector<WiiEvent>::const_iterator iter = events.begin(); iter != events.end(); ++iter) {
	    switch (iter->type) {
		case WII_EVENT_TYPE_BUTTON:
		    //running = false; // NOTE: Not handling any key events for now
		    break;
		case WII_EVENT_TYPE_IR:
		    process_ir_events(iter->ir, *iter->transform); // WARNING: Pay attention to the WARNING below
		    break;
		case WII_EVENT_TYPE_NON_EVENT:
		default:
		    break;
	    }
	}
    }
}

void get_wiis_event_data(std::vector<WiimoteAndTransformMatrix>& wiimotes, point_t const& ir_old, std::vector<WiiEvent>& events) {
    // These set of events will then be filtered and desirable events
    // will be added to event_data.
    std::vector< std::vector<WiiEvent> > event_batches( wiimotes.size() );

    // Collects all event data, additionally calculates the maximum event counts
    int max_event_counts = 0;
    for (unsigned int i = 0; i != wiimotes.size(); ++i) {
	std::vector<WiiEvent>& batch = event_batches[i]; // Readability
	int msg_count = 0;
	cwiid_mesg* msgs = 0;
	// NOTE: The API's been changed, I don't know what to do with the last argument
#ifdef COMPATIBILITY_GUTSY
	cwiid_get_mesg(wiimotes[i].wiimote, &msg_count, &msgs);
#else   
	timespec tspec;
	cwiid_get_mesg(wiimotes[i].wiimote, &msg_count, &msgs, &tspec);
#endif // COMPATIBILITY_GUTSY

	batch.resize(msg_count);
	for (std::vector<WiiEvent>::iterator iter = batch.begin(); iter != batch.end(); ++iter) {
	    iter->ir = ir_old; // Read wiicontrol.cpp if you're not sure about this
	    iter->button = INVALID_BUTTON_MSG_ID;
	    process_messages(msgs[iter-batch.begin()], &iter->ir, &iter->button);

	    iter->type = (iter->button != INVALID_BUTTON_MSG_ID) ? WII_EVENT_TYPE_BUTTON : WII_EVENT_TYPE_IR;
	}

	if (max_event_counts < msg_count)
	    max_event_counts = msg_count;
    }

    // Re-balances all the event counts to be equal
    // If some Wiimote has less events than the rest, a fresh WiiEvent will be
    // added (which default type is WII_EVENT_TYPE_NON_EVENT).
    // NOTE: That is *not* the real way to do it, these non-events must be
    // equally distributed thorough the shorter events, but whatever.
    for (unsigned int i = 0; i != wiimotes.size(); ++i) {
	std::vector<WiiEvent>& batch = event_batches[i];
	batch.insert( batch.end(), max_event_counts-batch.size(), WiiEvent() );
    }

    // Processes the return value 'events'
    for (int i = 0; i != max_event_counts; ++i) {
	// We allow all button events (hence processing those
	// right away) but only need *one* IR event
	std::vector<point_t> irs;
	std::vector<matrix_t*> transforms;
	for (unsigned int j = 0; j != wiimotes.size(); ++j) {
	    switch (event_batches[j][i].type) {
		case WII_EVENT_TYPE_BUTTON:
		    events.push_back( WiiEvent(event_batches[j][i].button) );
		    break;
		case WII_EVENT_TYPE_IR:
		    irs.push_back(event_batches[j][i].ir);
		    transforms.push_back(&wiimotes[j].transform);
		    break;
		case WII_EVENT_TYPE_NON_EVENT:
		default:
		    break;
	    }
	}

	// If there are multiple valid IR events, take the closest to ir_old
	// If none of them contains valid IR event, returns INVALID_IR_POS
	int closest_ir_index = -1;
	unsigned int closest_distance = static_cast<unsigned int>(-1); // Max uint value, actually sqr(closest)
	ASSERT(irs.size() == transforms.size(), "Someone finds me a lost transformation matrix, omfgwtfbbq!!!1");
	for (std::vector<point_t>::const_iterator iter = irs.begin(); iter != irs.end(); ++iter) 
	    if (iter->x != INVALID_IR_POS) {
		unsigned int const distance = squared_distance(*iter, ir_old);
		if ( closest_distance > distance ) {
		    closest_distance = distance;
		    closest_ir_index = iter - irs.begin();
		    break;
		}
	    }
	if (closest_ir_index != -1)
	    events.push_back( WiiEvent(irs[closest_ir_index], *transforms[closest_ir_index]) );
	else events.push_back(	WiiEvent( point_t(INVALID_IR_POS, 0),
				// WARNING: The matrix WILL fail when accessed, but I left it
				// here because of performance reason (pointer > another copy)
				matrix_t(TRANSFORM_MATRIX_ROWS, TRANSFORM_MATRIX_COLS) ));
    }
}

void WiiCursor::process_ir_events(point_t const& ir_new, matrix_t const& transform) {
    point_t const ir_old = m_thread_data.ir;
    m_thread_data.ir = ir_new;
    // Readability
    WiiEvents& wii_events = m_thread_data.events;                                                                                                      
    WiiEventData& wii_event_data = m_thread_data.event_data;

    if (ir_new.x != INVALID_IR_POS) { // Only updates if the IR data is valid
	wii_event_data.cursor_pos = infrared_data(ir_new, transform);
	// NOTE: The event fires even if the cursor doesn't move,
	// this is my design decision since it makes other things easier.
	wii_events.mouse_moved(wii_event_data);
    }
    if ( (ir_new.x != INVALID_IR_POS) && (ir_old.x == INVALID_IR_POS) ) { // MOUSE_DOWN
	m_thread_data.ir_on_mouse_down = ir_new;
	start_wiicursor_thread(m_thread_data);
	wii_events.mouse_down(wii_event_data);
    }
    if ( (ir_new.x == INVALID_IR_POS) && (ir_old.x != INVALID_IR_POS) ) { // MOUSE_UP
	wii_events.mouse_up(wii_event_data);

	// Finished at this point
	if ( m_thread_data.click_and_drag() ) {
	    wii_events.end_click_and_drag(wii_event_data);
	}
	else {
	    if ( !m_thread_data.right_click() ) { // Left click
		wii_events.left_clicked(wii_event_data);
	    }
	    else wii_events.right_button_up(wii_event_data); // Right click
	}
	finish_wiicursor_thread(m_thread_data);
    }
}
