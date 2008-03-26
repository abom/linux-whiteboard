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


#ifndef  __CONFIGURATOR_H__
#define  __CONFIGURATOR_H__


#include <gtkmm.h>
#include <libglademm.h>
#include <string>
#include <cstdlib>
#include <cwiid.h>
#include <confuse.h>

#include "auxiliary.h"
#include "matrix.h"


std::string config_file_path();


// All settings specific to a Wiimote
// NOTE: There is not much currently, but in the future
// it may turn out we want to store a lot more.
struct WiimoteData {
    WiimoteData(cwiid_wiimote_t* wiimote) :
	wiimote(wiimote),
	transform(TRANSFORM_MATRIX_ROWS, TRANSFORM_MATRIX_COLS)
    { }
    WiimoteData(cwiid_wiimote_t* wiimote, matrix_t const& transform) :
	wiimote(wiimote),
	transform(transform)
    { }

    cwiid_wiimote_t* wiimote;
    matrix_t transform;
};


// Handles configurations
// NOTE: We should have a separate Configurator class
// and make this class (GtkConfigurator) inherit from it,
// to handle specific GTK+ stuff.
class Configurator {
public:
    Configurator() :
	m_gtk_right_click_time(0)
    { }

    // WARNING: This function must be called *after* 'refXml'
    // has been initialized in caller's code.
    void init(Glib::RefPtr<Gnome::Glade::Xml>& refXml);

    // Getters
    delta_t_t const& wait_tolerance() const {
	return m_config_data.wait_tolerance;
    }
    std::vector<WiimoteData> const& wiimotes() const {
	return m_config_data.wiimotes;
    }
    std::vector<WiimoteData>& wiimotes() {
	return m_config_data.wiimotes;
    }

    // NOTE: Configurator class's members
    bool load_config();
    bool save_config();
private:
    /* GUI */
    Gtk::SpinButton* m_gtk_right_click_time;

    /* Event handlers */
    void right_click_time_changed() {
	m_config_data.wait_tolerance = m_gtk_right_click_time->get_value_as_int();
    }

    /* Data */
    struct ConfigurationData { // Holds everything that can be saved/loaded
	delta_t_t wait_tolerance;
	std::vector<WiimoteData> wiimotes;
    } m_config_data;
};

// Gets access to the configurator with this function
Configurator& get_configurator();


#endif /* __CONFIGURATOR_H__ */

