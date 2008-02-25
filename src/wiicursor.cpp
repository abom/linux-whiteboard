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


void* thread_func(void* ptr) {
    //printf("Thread has been created.\n");

    ASSERT(ptr != 0, "No data has been passed along");
    wiimote_data& data = *static_cast<wiimote_data*>(ptr);
    ASSERT(data.m_this_thread != 0, "This thread has become immortal, omfg!!!1");

    // Sets up the timer
    delta_t_t last_time = 0;
    get_delta_t(last_time);

    while ( data.running() ) {
	if ( data.right_click() ) {
	    fake_button(3, true);
	    break;
	}
	else data.m_waited += get_delta_t(last_time);

	if ( data.click_and_drag() ) {
	    fake_button(1, true);
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

    //printf("Thread is finished.\n");

    return 0;
}

void wiimote_data::start_thread(point_t const& ir_on_mouse_down) {
    m_ir_on_mouse_down = ir_on_mouse_down;
    set_data_at_thread_start();
    // NOTE: The order of the lines is important, don't mess with it
    // NOTE: Not checking for any return values here
    pthread_create(&m_this_thread, 0, &thread_func, this);
}
void wiimote_data::finish_thread() {
    if (m_thread_running) {
	ASSERT(m_this_thread != 0, "Thread ID should NOT be 0 if its data tells us it hasn't finished");

	pthread_t const this_thread = m_this_thread; // Backs up the thread ID before it gets cleared, not elegant
	set_data_at_thread_finish();
	pthread_join(this_thread, 0);
    }
    else ASSERT(m_this_thread == 0, "Thread ID should be 0 if there's none running");
}


void WiiCursor::process(bool& running) {
    // Sets up the Wiimote
    cwiid_disable(m_wiimote, CWIID_FLAG_NONBLOCK);

    unsigned int const MOVE_TOLERANCE = 5;
    unsigned int const WAIT_TOLERANCE = 700;
    point_t ir(INVALID_IR_POS, 0); // To avoid having to expose wiimote_data::ir, and to make it obvious that we're sharing
    wiimote_data wii_data(ir, MOVE_TOLERANCE, WAIT_TOLERANCE, running);

    while (running) {
	int msg_count = 0;
	union cwiid_mesg* msgs = 0;
	// NOTE: The API's been changed, I don't know what to do with the last argument
	timespec tspec;
	cwiid_get_mesg(m_wiimote, &msg_count, &msgs);

	for (int i = 0; i != msg_count; ++i) {
	    point_t const ir_old = wii_data.ir();
	    uint16_t button = INVALID_BUTTON_MSG_ID;
	    process_messages(msgs[i], &ir, &button);
	    point_t const& ir_new = ir; // Readability

	    if (button != INVALID_BUTTON_MSG_ID) {
		running = false;
		break; // Quits early
	    }

	    if (ir_new.x != INVALID_IR_POS) { // Only updates if the IR data is valid
		point_t const cursor = infrared_data(ir_new, m_transform);
		fake_move(cursor.x, cursor.y);
	    }
	    if ( (ir_new.x != INVALID_IR_POS) && (ir_old.x == INVALID_IR_POS) ) { // MOUSE_DOWN
		wii_data.start_thread(ir_new);
	    }
	    if ( (ir_new.x == INVALID_IR_POS) && (ir_old.x != INVALID_IR_POS) ) { // MOUSE_UP
		wii_data.finish_thread();

		// Finished at this point
		if ( wii_data.click_and_drag() ) {
		    fake_button(1, false);
		}
		else {
		    if ( !wii_data.right_click() ) { // Left click
			fake_button(1, true);
			fake_button(1, false);
		    }
		    else fake_button(3, false); // Right click
		}
	    }
	}
    }
}
