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


#include "wiicursor.h"


void* wiicursor_thread_func(void* ptr) {
    ASSERT(ptr != 0, "No data has been passed along");
    WiiCursor& data = *static_cast<WiiCursor*>(ptr);
    ASSERT(data.m_this_thread != 0, "This thread has become immortal, omfg!!!1");

    // Sets up the timer
    delta_t_t last_time = 0;
    get_delta_t(last_time);

    while ( data.all_running() ) {
	WiimoteEventData const wii_event_data =
	    {data.m_ir, data.m_ir_on_mouse_down, point_t(), data.m_waited};

	if ( data.right_click() ) {
		data.m_fn_right_button_down(wii_event_data);
	    break;
	}
	else data.m_waited += get_delta_t(last_time);

	if ( data.click_and_drag() ) {
		data.m_fn_begin_click_and_drag(wii_event_data);
	    break;
	}
	else {
	    // WARNING: A locking mechanism is needed to avoid this hack
	    // data.m_ir.x should never be invalid
	    // m_ir must not be read and updated at the same time in this function and in process()
	    if (data.m_ir.x != INVALID_IR_POS)
		data.m_moved = squared_distance(data.m_ir, data.m_ir_on_mouse_down);
	}
    }

    return 0;
}

void WiiCursor::start_thread(point_t const& ir_on_mouse_down) {
    m_ir_on_mouse_down = ir_on_mouse_down;
    set_data_at_thread_start();
    // NOTE: The order of the lines is important, don't mess with it
    // NOTE: Not checking for any return values here
    pthread_create(&m_this_thread, 0, &wiicursor_thread_func, this);
}
void WiiCursor::finish_thread() {
    if (m_thread_running) {
	set_data_at_thread_finish();
	pthread_join(m_this_thread, 0);
    }
}


void WiiCursor::process(
	cwiid_wiimote_t* wiimote, matrix_t transform,
        unsigned int move_tolerance, unsigned int wait_tolerance,
        bool& running)
{
    // Sets up the Wiimote
    cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC);
    cwiid_disable(wiimote, CWIID_FLAG_NONBLOCK);

    // Sets up itself
    m_move_tolerance = move_tolerance;
    m_wait_tolerance = wait_tolerance;
    m_running = &running; // No need to check anything here since running is a reference
    set_data_at_thread_start();
    set_data_at_thread_finish();

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
	    point_t const ir_old = m_ir;
	    point_t& ir_new = m_ir; // Read wii.cpp if you're not sure about this
	    uint16_t button = INVALID_BUTTON_MSG_ID;
	    process_messages(msgs[i], &ir_new, &button);
	    WiimoteEventData wii_event_data = {ir_new, point_t(), point_t(), m_waited};

	    if (button != INVALID_BUTTON_MSG_ID) {
		//running = false; // NOTE: Not handling any key events for now
		break; // Quits early
	    }

	    if (ir_new.x != INVALID_IR_POS) { // Only updates if the IR data is valid
		point_t const cursor = infrared_data(ir_new, transform);
		wii_event_data.cursor_pos = cursor;
		// NOTE: The event fires even if the cursor doesn't move,
		// this is my design decision since it makes other things easier.
		m_fn_mouse_moved(wii_event_data);
	    }
	    if ( (ir_new.x != INVALID_IR_POS) && (ir_old.x == INVALID_IR_POS) ) { // MOUSE_DOWN
		start_thread(ir_new);
		wii_event_data.ir_on_mouse_down = ir_new;
		m_fn_mouse_down(wii_event_data);
	    }
	    if ( (ir_new.x == INVALID_IR_POS) && (ir_old.x != INVALID_IR_POS) ) { // MOUSE_UP
		m_fn_mouse_up(wii_event_data);
		finish_thread();

		// Finished at this point
		if ( click_and_drag() ) {
		    m_fn_end_click_and_drag(wii_event_data);
		}
		else {
		    if ( !right_click() ) { // Left click
			m_fn_left_clicked(wii_event_data);
		    }
		    else m_fn_right_button_up(wii_event_data); // Right click
		}
	    }
	}
    }
}
