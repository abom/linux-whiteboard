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


#ifndef  __WIICURSORMANAGER_H__
#define  __WIICURSORMANAGER_H__


#include <vector>
#include <cstdlib>

#include "gui.h"
#include "matrix.h"
#include "wiicursor.h"


class MainGtkWindow;


void* wiicursormanager_connect_thread(void* ptr);


// Used by wiicursormanager_connect_thread()
struct WiiCursorManagerConnectEvents {
    Glib::Dispatcher start_each_connection;
    Glib::Dispatcher finish_each_connection;
    Glib::Dispatcher done_connecting;
};
struct WiiCursorManagerThreadData {
    WiiCursorManagerConnectEvents& events;
    std::vector<WiimoteAndTransformMatrix>& wiis;
    bool& last_connection_succeeded;

    WiiCursorManagerThreadData(
	WiiCursorManagerConnectEvents& events,
	std::vector<WiimoteAndTransformMatrix>& wiis,
	bool& last_connection_succeeded) :
	events(events),
	wiis(wiis),
	last_connection_succeeded(last_connection_succeeded)
    { }
};

// Basically, this class transparently manages all available
// Wiimotes and act as a virtual Wiimote to outsiders.
class WiiCursorManager {
public:
    WiiCursorManager(WiiCursorManagerConnectEvents& events) :
	m_wiis(m_thread_data.wiimotes),
	m_cal_window(0),
	m_last_connection_succeeded(false),
	m_connect_thread_data(events, m_wiis, m_last_connection_succeeded)
    { }

    // Connects all available Wiimotes, asynchronously
    void connect();
    // Disconnects all Wiimotes
    // Returns true if all successfully disconnected
    bool disconnect();

    // Calibrates all Wiimotes
    // Returns false if anyone of those failed to calibrate
    bool calibrate();

    // Activates all Wiimotes
    // Returns false if anyone of those failed to activate
    bool activate();
    // Deactivates all Wiimotes
    // Returns false if anyone of those failed to deactivate
    bool deactivate();

    // Sets up move and wait tolerances
    void tolerances(delta_t_t const& wait_tolerance) {
	m_thread_data.wait_tolerance = &wait_tolerance;
    }
    // Events
    WiiEvents& events() {
	return m_thread_data.events;
    }

    // Loads configurations
    // Returns true on success
    bool load_config();
    // Saves configurations
    // Returns true on success
    bool save_config() const;

    // General queries
    unsigned int connected() const {
	return m_wiis.size(); // Number of conected Wiimotes
    }
    bool activated() const {
	return connected() && m_thread_data.thread_running;
    }
    bool last_connection_succeeded() const {
	return m_last_connection_succeeded;
    }
private:
    WiiThreadFuncData m_thread_data; // For all Wiimotes
    std::vector<WiimoteAndTransformMatrix>& m_wiis; // To not have to look at m_thread_data all the time
    typedef std::vector<WiimoteAndTransformMatrix>::iterator WiimoteAndTransformMatrixIterator;
    // NOTE: Basically a hack, because I don't want to keep a real cal_window around
    // This is needed because if user closes the app, we have to notify it somehow.
    CalibrationWindow* m_cal_window;
    // Used by wiicursormanager_connect_thread()
    bool m_last_connection_succeeded; // To be used by others as a workaround for the lack of cross-thread data-passing technique
    WiiCursorManagerThreadData m_connect_thread_data;
};


#endif /* __WIICURSORMANAGER_H__ */
