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

#include "gui.h"
#include "matrix.h"
#include "wiicursor.h"
#include "wiicontrol.h"
#include "configurator.h"


struct WiiCursorManagerConnectEvents {
    Glib::Dispatcher start_each_connection;
    Glib::Dispatcher finish_each_connection;
    Glib::Dispatcher done_connecting;
};

// Basically, this class transparently manages all available
// Wiimotes and act as a virtual Wiimote to outsiders.
class WiiCursorManager {
public:
    WiiCursorManager(WiiCursorManagerConnectEvents& events) :
	m_thread_data( get_configurator().wiimotes() ),
	m_wiis(m_thread_data.wiimotes),
	m_cal_window(0),
	m_last_connection_succeeded(false),
	m_connect_events(events)
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

    // Mouse events - To be handled by callers
    WiiEvents& events() {
	return m_thread_data.events;
    }

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
    std::vector<WiimoteData>& m_wiis; // To not have to look at m_thread_data all the time
    // NOTE: Basically a hack, because I don't want to keep a real cal_window around
    // This is needed because if user closes the app, we have to notify it somehow.
    CalibrationWindow* m_cal_window;

    // Used by wiicursormanager_connect_thread()
    bool m_last_connection_succeeded; // Used as a workaround for the lack of cross-thread data-passing technique
    WiiCursorManagerConnectEvents& m_connect_events;
    void wiicursormanager_connect_thread();
};


#endif /* __WIICURSORMANAGER_H__ */
