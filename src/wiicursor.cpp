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


void* wiicursor_thread_func(void* ptr) {
    ASSERT(ptr != 0, "No data has been passed along");
    WiiThreadFuncData& data = *static_cast<WiiThreadFuncData*>(ptr);
    ASSERT(data.this_thread != 0, "This thread has become immortal, omfg!!!1");
                                                                                                                                                               
    WiiCursor wc;
    wc.events() = data.events;
    wc.process(data.wiimotes, *data.wait_tolerance, data.thread_running); // The main loop

    for (std::vector<WiimoteData>::iterator iter = data.wiimotes.begin(); iter != data.wiimotes.end(); ++iter)
	cwiid_disable(iter->wiimote, CWIID_FLAG_MESG_IFC);

    return 0;
}
void start_wiicursor_thread(WiiThreadFuncData& data) {
    // NOTE: Not checking for any return value here
    if (!data.thread_running) {
	data.thread_running = true;
	pthread_create(&data.this_thread, 0, &wiicursor_thread_func, &data);
    }
}
void finish_wiicursor_thread(WiiThreadFuncData& data) {
    if (data.thread_running) {
	data.thread_running = false;
	pthread_join(data.this_thread, 0);
    }
}


void WiiCursor::right_click_thread_func() {
    WiiCursorThreadData& data = m_thread_data; // Readability

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
}
void WiiCursor::start_right_click_thread() {
    WiiCursorThreadData& data = m_thread_data; // Readability

    // NOTE: Not checking for any return value here
    if (!data.thread_running) {
	data.thread_running = true;
	data.this_thread = Glib::Thread::create(
	    sigc::mem_fun(*this, &WiiCursor::right_click_thread_func), true);
    }
}
void WiiCursor::finish_right_click_thread() {
    WiiCursorThreadData& data = m_thread_data; // Readability

    if (data.thread_running) {
	data.thread_running = false;
	data.this_thread->join();
    }
}


void WiiCursor::process(
	std::vector<WiimoteData>& wiimotes,
        delta_t_t const& wait_tolerance,
        bool const& running)
{
    // Sets up the Wiimotes
    for (std::vector<WiimoteData>::iterator iter = wiimotes.begin(); iter != wiimotes.end(); ++iter) {
	cwiid_enable(iter->wiimote, CWIID_FLAG_MESG_IFC);
	cwiid_disable(iter->wiimote, CWIID_FLAG_NONBLOCK);
    }

    // Sets up itself
    m_thread_data.wait_tolerance = &wait_tolerance;
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
		    process_ir_events(iter->ir, iter->transform);
		    break;
		case WII_EVENT_TYPE_NON_EVENT:
		default:
		    break;
	    }
	}
    }
}

void get_wiis_event_data_collect_all_events(
    std::vector<WiimoteData>& wiimotes,
    point_t const& ir_old,
    std::vector< std::vector<WiiEvent> >& event_batches,
    unsigned int& max_event_counts)
{
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
	for (int i = 0; i != msg_count; ++i) {
	    WiiEvent& cur_event = batch[i]; // Readability
	    cur_event.ir = ir_old; // Read wiicontrol.cpp if you're not sure about this
	    cur_event.button = INVALID_BUTTON_MSG_ID;
	    process_messages(msgs[i], &cur_event.ir, &cur_event.button);

	    cur_event.type = (cur_event.button != INVALID_BUTTON_MSG_ID) ? WII_EVENT_TYPE_BUTTON : WII_EVENT_TYPE_IR;
	}

	if ( max_event_counts < static_cast<unsigned int>(msg_count) )
	    max_event_counts = static_cast<unsigned int>(msg_count);
    }
}
void get_wiis_event_data_rebalance_events(unsigned int max_event_counts, std::vector< std::vector<WiiEvent> >& event_batches) {
    // Re-balances all the event counts to be equal
    // If some Wiimote has less events than the rest, a fresh WiiEvent will be
    // added (which default type is WII_EVENT_TYPE_NON_EVENT).
    // NOTE: This is *not* the real way to do it, these non-events must be
    // equally distributed throughout the shorter event batches, but whatever.
    for (std::vector< std::vector<WiiEvent> >::iterator iter = event_batches.begin(); iter != event_batches.end(); ++iter)
	iter->insert( iter->end(), max_event_counts-iter->size(), WiiEvent() );
}
void get_wiis_event_data_process_events(
    std::vector< std::vector<WiiEvent> > const& event_batches,
    unsigned int current_event_index,
    std::vector<WiimoteData> const& wiimotes,
    point_t const& ir_old,
    std::vector<point_t>& irs,
    std::vector<matrix_t const*>& transforms,
    std::vector<WiiEvent>& events)
{
    // We allow all button events (hence processing those
    // right away) but only need *one* IR event
    for (unsigned int i = 0; i != event_batches.size(); ++i) {
	switch (event_batches[i][current_event_index].type) {
	    case WII_EVENT_TYPE_BUTTON:
		events.push_back( WiiEvent(event_batches[i][current_event_index].button) );
		break;
	    case WII_EVENT_TYPE_IR:
		irs.push_back(event_batches[i][current_event_index].ir);
		transforms.push_back(&wiimotes[i].transform);
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
	events.push_back( WiiEvent(irs[closest_ir_index], transforms[closest_ir_index]) );                                                                 
    else events.push_back( WiiEvent( point_t(INVALID_IR_POS, 0), 0) );
}

void get_wiis_event_data(std::vector<WiimoteData>& wiimotes, point_t const& ir_old, std::vector<WiiEvent>& events) {
    // This set of events will then be filtered and desirable events
    // will be added to event_data.
    std::vector< std::vector<WiiEvent> > event_batches( wiimotes.size() );

    unsigned int max_event_counts = 0;
    get_wiis_event_data_collect_all_events(wiimotes, ir_old, event_batches, max_event_counts);
    get_wiis_event_data_rebalance_events(max_event_counts, event_batches);

    // Processes the return value 'events'
    for (unsigned int i = 0; i != max_event_counts; ++i) {
	std::vector<point_t> irs;
	std::vector<matrix_t const*> transforms;
	get_wiis_event_data_process_events(event_batches, i, wiimotes, ir_old, irs, transforms, events);
    }
}

void WiiCursor::process_ir_events(point_t ir_new, matrix_t const* transform) {
    point_t const ir_old = m_thread_data.ir;
    // We don't want the raw ir_new, let's put it
    // through the IR signal filter first.
    ir_new = m_ir_filter.process(ir_new);
    m_thread_data.ir = ir_new;
    // Readability
    WiiEvents& wii_events = m_thread_data.events;                                                                                                      
    WiiEventData& wii_event_data = m_thread_data.event_data;

    if (ir_new.x != INVALID_IR_POS) { // MOUSE_MOVE - Only updates if the IR data is valid
	// NOTE: Can happen (ir_new is valid but transform is null)
	// if m_ir_filter is tolerating the disappearing IR
	if (transform)
	    wii_event_data.cursor_pos = infrared_data(ir_new, *transform);
	// NOTE: The event fires even if the cursor doesn't move,
	// this is my design decision since it makes other things easier.
	wii_events.mouse_moved(wii_event_data);
    }
    if ( (ir_new.x != INVALID_IR_POS) && (ir_old.x == INVALID_IR_POS) ) { // MOUSE_DOWN
	m_thread_data.ir_on_mouse_down = ir_new;
	start_right_click_thread();
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
	finish_right_click_thread();
    }
}
