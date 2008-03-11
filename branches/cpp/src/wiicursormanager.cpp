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
	m_wiis.back().wiimote = new_wiimote;
    }

    return m_wiis.size() ? true : false;
}
bool WiiCursorManager::disconnect() {
    for (WiiThreadFuncDataIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter) {
	finish_wii_thread(*iter);
	wii_disconnect(iter->wiimote);
    }

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
