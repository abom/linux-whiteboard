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


#include "configurator.h"


std::string config_file_path() {
    // WARNING: Unsafe, one should use something like g_build_filename() and g_get_home_dir()
    std::string const ret( std::string(getenv("HOME")) + "/.whiteboardrc" );

    DEBUG_MSG(2, "Config file path: %s\n", ret.c_str());

    return ret;
}


// Gets access to the configurator with this function
Configurator& get_configurator() {
    // NOTE: Not thread-safe
    static Configurator config;

    return config;
}


void Configurator::init(Glib::RefPtr<Gnome::Glade::Xml>& refXml) {
    DEBUG_MSG(1, "Initializing configurator...\n");

    refXml->get_widget("spin-right-click", m_gtk_right_click_time);

    // Right click time
    right_click_time_changed();
    m_gtk_right_click_time->signal_value_changed().connect( sigc::mem_fun(*this, &Configurator::right_click_time_changed) );

    DEBUG_MSG(1, "Set right-click-time widget to %d\n", m_config_data.wait_tolerance);

    DEBUG_MSG(1, "Configurator initialized\n");
}

bool Configurator::load_other_config() {
    bool ret = true;

    try {
	DEBUG_MSG(1, "Openning config file at %s\n", config_file_path().c_str());
	ConfigFileParser config( config_file_path() );

	// CONFIG_RIGHT_CLICK_TIME
	try {
	    DEBUG_MSG(1, "Reading right-click-time\n");
	    m_gtk_right_click_time->set_value( config.getValue<int>(CONFIG_RIGHT_CLICK_TIME) );
	} catch(ConfigFileParser::KeyNotFound) {
	    DEBUG_MSG(1, "Configuration key '%s' not found.\n", CONFIG_RIGHT_CLICK_TIME);
	    ret = false;
	}
    } catch(ConfigFileParser::FileNotFound) {
	DEBUG_MSG(1, "Configuration file not found.\n");
	ret = false;
    }

    return ret;
}
bool Configurator::load_wiimotes_config() {
    bool ret = true;

    try {
	DEBUG_MSG(1, "Openning config file at %s\n", config_file_path().c_str());
	ConfigFileParser config( config_file_path() );

	// CONFIG_MATRIX
	try {
	    DEBUG_MSG(1, "Getting all transformation matrices...\n");
	    std::vector<WiimoteData>& wiis = m_config_data.wiimotes;
	    for (unsigned int i = 0; i != wiis.size(); ++i) {
		DEBUG_MSG(1, "Getting matrix #%d...", i);
		wiis[i].transform =
		    construct_matrix_from_key_value( config.getValue<std::string>(construct_matrix_key_name(i)) );
		DEBUG_MSG(1, "Got!\n");
	    }
	} catch(ConfigFileParser::KeyNotFound) {
	    DEBUG_MSG(1, "Matrix configuration key not found.\n");
	    ret = false;
	}
    } catch(ConfigFileParser::FileNotFound) {
	DEBUG_MSG(1, "Configuration file not found.\n");
	ret = false;
    }

    return ret;
}
bool Configurator::save_config() {
    // WARNING: Not checking for anything here

    DEBUG_MSG(1, "Saving config file...\n");

    ConfigFileParser config;

    // CONFIG_RIGHT_CLICK_TIME
    DEBUG_MSG(1, "Right-click-time is %d\n", m_config_data.wait_tolerance);
    config.add<unsigned int>(CONFIG_RIGHT_CLICK_TIME, m_config_data.wait_tolerance);
    // CONFIG_MATRIX
    std::vector<WiimoteData> const& wiis = m_config_data.wiimotes;
    for (unsigned int i = 0; i != wiis.size(); ++i) {
	std::string const new_matrix_name( construct_matrix_key_name(i) );
	std::string const new_matrix_transform( construct_matrix_key_value(wiis[i].transform) );

	DEBUG_MSG(1, "%s is %s\n", new_matrix_name.c_str(), new_matrix_transform.c_str());
	config.add<std::string>(new_matrix_name, new_matrix_transform);
    }

    config.saveCfgFile( config_file_path() );

    DEBUG_MSG(1, "Config file saved\n");

    // NOTE: Always returns true for now
    return true;
}
