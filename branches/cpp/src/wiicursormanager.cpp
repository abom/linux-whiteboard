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


#include "wiicursormanager.h"


void WiiCursorManager::wiicursormanager_connect_thread() {
    WiiCursorManagerConnectEvents& events = m_connect_events; // Readability

    // Trying to connect to as many Wiimotes as possible
    // NOTE: Error checking
    cwiid_wiimote_t* new_wiimote = 0;
    do {
        events.start_each_connection();
	if ( (new_wiimote = wii_connect(0)) )
	    m_wiis.push_back( WiimoteData(new_wiimote) );
	m_last_connection_succeeded = (new_wiimote != 0) ? true : false;
	events.finish_each_connection();
    }
    while (new_wiimote);

    events.done_connecting();
}


void WiiCursorManager::connect()
{
    // NOTE: We are throwing away the thread ID because we trust
    // the caller to NOT call this repeatedly at the same time.
    // Also not joinable.
    Glib::Thread::create(sigc::mem_fun(*this, &WiiCursorManager::wiicursormanager_connect_thread), false);
}
bool WiiCursorManager::disconnect() {
    if (m_cal_window)
	m_cal_window->quit();
    deactivate();
    for (WiimoteDataIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter)
	wii_disconnect(iter->wiimote);
    m_wiis.clear();

    // NOTE: Always returns true for now
    return true;
}


bool WiiCursorManager::calibrate() {
    bool ret = true;

    for (WiimoteDataIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter) {
	// Constructs a custom user message
	// NOTE: Should be controllable from the main GUI, but not now
	// WARNING: C function. I'd have used std::ostringstream if not for l10n.
	unsigned int const current_wii_index = iter - m_wiis.begin() + 1;
	char message[1024];
	sprintf(message, _("Calibrating Wiimote #%d"), current_wii_index);

	CalibrationWindow cal_window(iter->wiimote, message, *m_thread_data.wait_tolerance);
	m_cal_window = &cal_window;
	WiimoteCalibratedPoints p_wii;
	if ( !cal_window.get_calibration_points(p_wii) ) {
	    ret = false;
	    break;
	}
	else iter->transform = calculate_transformation_matrix(p_wii);
    }
    m_cal_window = 0; // If it gets to this point, the window's been closed

    return ret;
}


bool WiiCursorManager::activate() {
    start_wiicursor_thread(m_thread_data);

    // NOTE: Always returns true for now
    return true;
}
bool WiiCursorManager::deactivate() {
    finish_wiicursor_thread(m_thread_data);

    // NOTE: Always returns true for now
    return true;
}
