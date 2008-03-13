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


bool WiiCursorManager::connect() {
    // Trying to connect to as many Wiimotes as possible
    // NOTE: Error checking
    cwiid_wiimote_t* new_wiimote = 0;
    while ( (new_wiimote = wii_connect(0)) ) {
	m_wiis.push_back( WiiThreadFuncData() );
	WiiThreadFuncData& thread_data = m_wiis.back();
	thread_data.wiimote = new_wiimote;

	thread_data.events.left_clicked = sigc::mem_fun(*this, &WiiCursorManager::wii_left_clicked);
	thread_data.events.right_button_down = sigc::mem_fun(*this, &WiiCursorManager::wii_right_button_down);
	thread_data.events.right_button_up = sigc::mem_fun(*this, &WiiCursorManager::wii_right_button_up);
	thread_data.events.begin_click_and_drag = sigc::mem_fun(*this, &WiiCursorManager::wii_begin_click_and_drag);
	thread_data.events.end_click_and_drag = sigc::mem_fun(*this, &WiiCursorManager::wii_end_click_and_drag);
	thread_data.events.mouse_moved = sigc::mem_fun(*this, &WiiCursorManager::wii_mouse_moved);
    }

    return connected();
}
bool WiiCursorManager::disconnect() {
    for (WiiThreadFuncDataIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter) {
	finish_wii_thread(*iter);
	wii_disconnect(iter->wiimote);
    }
    m_wiis.clear();

    // NOTE: Always returns true for now
    return true;
}


bool WiiCursorManager::calibrate() {
    bool ret = true;

    for (WiiThreadFuncDataIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter) {
	CalibrationData cal_data;
	CalibrationWindow cal_window(iter->wiimote, cal_data);
	if ( cal_window.get_calibration_points() ) {
	    ret = false;
	    break;
	}
	else iter->transform = calculate_transformation_matrix(cal_data.p_wii);
    }

    return ret;
}


bool WiiCursorManager::activate() {
    for (WiiThreadFuncDataIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter)
	start_wii_thread(*iter);

    // NOTE: Always returns true for now
    return true;
}
bool WiiCursorManager::deactivate() {
    for (WiiThreadFuncDataIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter)
	finish_wii_thread(*iter);

    // NOTE: Always returns true for now
    return true;
}


void WiiCursorManager::tolerances(unsigned int move_tolerance, delta_t_t wait_tolerance) {
    for (WiiThreadFuncDataIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter) {
	iter->move_tolerance = move_tolerance;
	iter->wait_tolerance = wait_tolerance;
    }
}


// WARNING: These are multi-threaded
void WiiCursorManager::wii_left_clicked(WiiEventData const& data) {
    for (WiiThreadFuncDataIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter) {
    }
}
void WiiCursorManager::wii_right_button_down(WiiEventData const& data) {
}
void WiiCursorManager::wii_right_button_up(WiiEventData const& data) {
}
void WiiCursorManager::wii_begin_click_and_drag(WiiEventData const& data) {
}
void WiiCursorManager::wii_end_click_and_drag(WiiEventData const& data) {
}
void WiiCursorManager::wii_mouse_moved(WiiEventData const& data) {
    // NOTE: There will be an abundance of mouse move events now,
    // but this will be very helpful when I write that physics engine
    // (accumulated data = more accurate).
    m_wii_events.mouse_moved(data);
}
