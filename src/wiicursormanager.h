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


// Basically, this class transparently manages all available
// Wiimotes and act as a virtual Wiimote to outsiders.
class WiiCursorManager {
public:
    WiiCursorManager() :
	m_wii_event_data(m_ir, m_ir_on_mouse_down, m_waited)
    { }

    // Connects all available Wiimotes
    // Returns true if at least 1 Wiimote was connected
    bool connect();
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

    // Setup move and wait tolerances
    void tolerances(unsigned int move_tolerance, delta_t_t wait_tolerance);

    // Events
    WiiEvents& events() {
	return m_wii_events;
    }

    // Loads configurations
    // Returns true on success
    bool load_config();
    // Saves configurations
    // Returns true on success
    bool save_config() const;

    bool connected() const {
	return m_wiis.size() ? true : false;
    }
    bool activated() const {
	return connected() && m_wiis.front().thread_running;
    }
private:
    // Event handlers for all connected Wiimotes
    void wii_left_clicked(WiiEventData const& data);
    void wii_right_button_down(WiiEventData const& data);
    void wii_right_button_up(WiiEventData const& data);
    void wii_begin_click_and_drag(WiiEventData const& data);
    void wii_end_click_and_drag(WiiEventData const& data);
    void wii_mouse_moved(WiiEventData const& data);

    std::vector<WiiThreadFuncData> m_wiis; // All Wiimotes
    typedef std::vector<WiiThreadFuncData>::iterator WiiThreadFuncDataIterator;

    point_t m_ir;
    point_t m_ir_on_mouse_down;
    delta_t_t m_waited;

    WiiEvents m_wii_events; // Events that will be emitted by this class
    WiiEventData m_wii_event_data; // Data that will be sent during events
};


#endif /* __WIICURSORMANAGER_H__ */
