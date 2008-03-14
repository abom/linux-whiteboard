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
    while ( (new_wiimote = wii_connect(0)) )
	m_wiis.push_back( WiimoteAndTransformMatrix(new_wiimote) );

    return connected();
}
bool WiiCursorManager::disconnect() {
    if (m_cal_window)
	m_cal_window->quit();
    deactivate();
    for (WiimoteAndTransformMatrixIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter)
	wii_disconnect(iter->wiimote);
    m_wiis.clear();

    // NOTE: Always returns true for now
    return true;
}


bool WiiCursorManager::calibrate() {
    bool ret = true;

    for (WiimoteAndTransformMatrixIterator iter = m_wiis.begin(); iter != m_wiis.end(); ++iter) {
	std::vector<WiimoteAndTransformMatrix> one_wiimote;
	one_wiimote.push_back( WiimoteAndTransformMatrix(iter->wiimote, iter->transform) );
	CalibrationData cal_data;
	CalibrationWindow cal_window( one_wiimote, cal_data);
	m_cal_window = &cal_window; // NOTE: Not elegant
	if ( cal_window.get_calibration_points() ) {
	    ret = false;
	    m_cal_window = 0; // NOTE: Not elegant :-<
	    break;
	}
	else iter->transform = calculate_transformation_matrix(cal_data.p_wii);
	m_cal_window = 0; // NOTE: Not elegant
    }

    return ret;
}


bool WiiCursorManager::activate() {
    start_wii_thread(m_thread_data);

    // NOTE: Always returns true for now
    return true;
}
bool WiiCursorManager::deactivate() {
    finish_wii_thread(m_thread_data);

    // NOTE: Always returns true for now
    return true;
}
