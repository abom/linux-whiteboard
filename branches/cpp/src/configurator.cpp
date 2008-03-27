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
    return std::string(getenv("HOME")) + "/.whiteboardrc";
}


// Gets access to the configurator with this function
Configurator& get_configurator() {
    // NOTE: Not thread-safe
    static Configurator config;

    return config;
}


void Configurator::init(Glib::RefPtr<Gnome::Glade::Xml>& refXml) {
    refXml->get_widget("spin-right-click", m_gtk_right_click_time);

    // Right click time
    right_click_time_changed();
    m_gtk_right_click_time->signal_value_changed().connect( sigc::mem_fun(*this, &Configurator::right_click_time_changed) );
}

bool Configurator::load_other_config() {
    bool ret = true;

    try {
	ConfigFileParser config( config_file_path() );

	// CONFIG_RIGHT_CLICK_TIME
	try {
	    m_gtk_right_click_time->set_value( config.getValue<int>(CONFIG_RIGHT_CLICK_TIME) );
	} catch(ConfigFileParser::KeyNotFound) {
	    printf("Configuration key '%s' not found.\n", CONFIG_RIGHT_CLICK_TIME);
	    ret = false;
	}
    } catch(ConfigFileParser::FileNotFound) {
	printf("Configuration file not found.\n");
	ret = false;
    }

    return ret;
}
bool Configurator::load_wiimotes_config() {
    bool ret = true;

    try {
	ConfigFileParser config( config_file_path() );

	// CONFIG_MATRIX
	try {
	    std::vector<WiimoteData>& wiis = m_config_data.wiimotes;
	    for (unsigned int i = 0; i != wiis.size(); ++i)
		wiis[i].transform =
		    construct_matrix_from_key_value( config.getValue<std::string>(construct_matrix_key_name(i)) );
	} catch(ConfigFileParser::KeyNotFound) {
	    printf("Matrix configuration key not found.\n");
	    ret = false;
	}
    } catch(ConfigFileParser::FileNotFound) {
	printf("Configuration file not found.\n");
	ret = false;
    }

    return ret;
}
bool Configurator::save_config() {
    // WARNING: Not checking for anything here

    ConfigFileParser config;

    // CONFIG_RIGHT_CLICK_TIME
    config.add<unsigned int>(CONFIG_RIGHT_CLICK_TIME, m_config_data.wait_tolerance);
    // CONFIG_MATRIX
    std::vector<WiimoteData> const& wiis = m_config_data.wiimotes;
    for (unsigned int i = 0; i != wiis.size(); ++i)
	config.add<std::string>( construct_matrix_key_name(i), construct_matrix_key_value(wiis[i].transform) );

    config.saveCfgFile( config_file_path() );

    // NOTE: Always returns true for now
    return true;
}
