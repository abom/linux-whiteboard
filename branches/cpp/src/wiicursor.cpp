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
    wc.process(data.wiimote, data.transform, data.move_tolerance, data.wait_tolerance, data.thread_running); // The main loop

    cwiid_disable(data.wiimote, CWIID_FLAG_MESG_IFC);
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
	cwiid_wiimote_t* wiimote, matrix_t transform,
        unsigned int move_tolerance, unsigned int wait_tolerance,
        bool const& running)
{
    // Sets up the Wiimote
    cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC);
    cwiid_disable(wiimote, CWIID_FLAG_NONBLOCK);

    // Sets up itself
    m_thread_data.move_tolerance = move_tolerance;
    m_thread_data.wait_tolerance = wait_tolerance;
    m_thread_data.running = &running; // No need to check anything here since running is a reference

    while (running) {
	int msg_count = 0;
	union cwiid_mesg* msgs = 0;
	// NOTE: The API's been changed, I don't know what to do with the last argument
#ifdef COMPATIBILITY_GUTSY
	cwiid_get_mesg(wiimote, &msg_count, &msgs);
#else
	timespec tspec;
	cwiid_get_mesg(wiimote, &msg_count, &msgs, &tspec);
#endif // COMPATIBILITY_GUTSY

	for (int i = 0; i != msg_count; ++i) {
	    point_t const ir_old = m_thread_data.ir;
	    point_t& ir_new = m_thread_data.ir; // Read wiicontrol.cpp if you're not sure about this
	    uint16_t button = INVALID_BUTTON_MSG_ID;
	    process_messages(msgs[i], &ir_new, &button);
	    // Readability
	    WiiEvents& wii_events = m_thread_data.events;
	    WiiEventData& wii_event_data = m_thread_data.event_data;

	    if (button != INVALID_BUTTON_MSG_ID) {
		//running = false; // NOTE: Not handling any key events for now
		break; // Quits early
	    }

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
    }
}
