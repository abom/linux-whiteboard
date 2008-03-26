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

bool Configurator::load_config() {
    return false;
}
bool Configurator::save_config() {
    return false;
}
