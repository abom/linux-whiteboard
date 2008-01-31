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


void* click_thread(void* ptr) {
    printf("Thread has been created.\n");

    ASSERT(ptr != 0, "No data has been passed along");
    click_stuff& data = *static_cast<click_stuff*>(ptr);
    ASSERT(data.this_thread != 0, "This thread has become immortal, omfg!!!1");

    // Sets up our timer
    // NOTE: get_delta_t() uses a static variable, but I guarantee
    // only at most one thread will be running at any given time
    get_delta_t();
    data.set_data_at_thread_start();

    while ( (!data.program_finished) && (!data.thread_finished) ) {
	//printf("WAITED %lld\n", data.waited); fflush(stdout);
	if (data.waited > data.wait_tolerance) { // Right click
	    fake_button(3, true);
	    break;
	}
	else data.waited += get_delta_t();

	if ( data.moved > sqr(data.move_tolerance) ) {
	    fake_button(1, true); // Clicks and drags
	    break;
	}
	else data.moved = squared_distance(data.ir, data.ir_on_mouse_down);
    }
    data.set_data_at_thread_end();

    printf("Thread is finished.\n");

    return 0;
}

void ask_thread_to_finish(click_stuff& data) {
    if (!data.thread_finished) {
	ASSERT(data.this_thread != 0, "Thread ID should NOT be 0 if its data tells us it hasn't finished.");

	data.thread_finished = true; // I know this is not elegant, I'll fix it later
	pthread_join(data.this_thread, 0);
    }
    else ASSERT(data.this_thread == 0, "Thread ID should be 0 if there's none running");
}


void WiiCursor::process() {
    cwiid_disable(m_wiimote, CWIID_FLAG_NONBLOCK);

    m_program_finished = false;
    while (!m_program_finished) {
	int msg_count = 0;
	union cwiid_mesg* msgs = 0;
	cwiid_get_mesg(m_wiimote, &msg_count, &msgs);

	for (int i = 0; i != msg_count; ++i) {
	    point_t ir_new;
	    uint16_t button = INVALID_BUTTON_MSG_ID;
	    process_messages(msgs[i], &ir_new, &button);

	    if (button != INVALID_BUTTON_MSG_ID) {
		m_program_finished = true;
		break; // Quits early
	    }

	    if (ir_new.x != INVALID_IR_POS) { // Only updates if the IR data is valid
		point_t const cursor = infrared_data(ir_new, m_transform);
		fake_move(cursor.x, cursor.y);
	    }
	    if ( (ir_new.x != INVALID_IR_POS) && (m_ir.x == INVALID_IR_POS) ) { // MOUSE_DOWN
		m_click_data.ir_on_mouse_down = ir_new;

		pthread_create( &m_click_data.this_thread, 0, &click_thread, &m_click_data);
	    }
	    if ( (ir_new.x == INVALID_IR_POS) && (m_ir.x != INVALID_IR_POS) ) { // MOUSE_UP
		if (m_click_data.waited <= m_click_data.wait_tolerance) { // Left click
		    fake_button(1, true);
		    fake_button(1, false);
		}
		else fake_button(3, false); // Right click
		if ( m_click_data.moved > sqr(m_click_data.move_tolerance) ) // Click and drag
		    fake_button(1, false);

		ask_thread_to_finish(m_click_data);
	    }
	    m_ir = ir_new;
	}
    }
}
