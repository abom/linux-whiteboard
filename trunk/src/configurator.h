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

#include "auxiliary.h"
#include "matrix.h"
#include "ConfigFileParser.h"
#include "common.h"


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
typedef std::vector<WiimoteData>::iterator WiimoteDataIterator;

// Configuration key types
#define CONFIG_RIGHT_CLICK_TIME "right_click_time"
#define CONFIG_MATRIX "matrix"
#define CONFIG_SHOW_INSTRUCTIONS_AT_START "show_instructions"

// Handles configurations
// NOTE: We should have a separate Configurator class
// and make this class (GtkConfigurator) inherit from it,
// to handle specific GTK+ stuff.
class Configurator {
public:
    Configurator() :
	m_gtk_right_click_time(0),
	m_gtk_show_instructions(0)
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
    bool const& show_instructions() const {
	return m_config_data.show_instructions;
    }

    // NOTE: Should be Configurator class's members
    bool load_other_config();
    bool load_wiimotes_config();
    bool save_config();
private:
    /* Event handlers */
    void right_click_time_changed() {
	m_config_data.wait_tolerance = m_gtk_right_click_time->get_value_as_int();
	DEBUG_MSG(2, "Right-click-time changed to %d\n", m_config_data.wait_tolerance);
    }
    void show_instructions_toggled() {
	m_config_data.show_instructions = m_gtk_show_instructions->get_active();
	DEBUG_MSG(2, "Show-instructions changed to %d\n", m_config_data.show_instructions);
    }

    /* Helpers */
    std::string construct_matrix_key_name(unsigned int index) const {
	std::ostringstream ret;
	ret << CONFIG_MATRIX << index;
	return ret.str();
    }
    std::string construct_matrix_key_value(matrix_t const& transform) const {
	std::ostringstream ret;
	ret << '\"' << transform << '\"';
	return ret.str();
    }
    matrix_t construct_matrix_from_key_value(std::string const& key_value) const {
	matrix_t ret(TRANSFORM_MATRIX_ROWS, TRANSFORM_MATRIX_COLS);
	std::istringstream stream(key_value);
	stream >> ret;
	return ret;
    }

    /* GUI */
    Gtk::SpinButton* m_gtk_right_click_time;
    Gtk::CheckButton* m_gtk_show_instructions;
    /* Data */
    struct ConfigurationData { // Holds everything that can be saved/loaded
	delta_t_t wait_tolerance;
	std::vector<WiimoteData> wiimotes;
	bool show_instructions;
    } m_config_data;
};

// Gets access to the configurator with this function
Configurator& get_configurator();


#endif /* __CONFIGURATOR_H__ */

