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

#include "auxiliary.h"


// Handles configurations
class Configurator {
public:
    Configurator(Glib::RefPtr<Gnome::Glade::Xml>& refXml) :
	m_refXml(refXml),
	m_gtk_right_click_time(0),
	m_right_click_time(0)
    { }

    // NOTE: This function must be called *after* 'refXml' has
    // been initialized in caller's code.
    void init();

    delta_t_t const& right_click_time() const {
	return m_right_click_time;
    }

    bool load_config();
    bool save_config();
private:
    /* GUI */
    Glib::RefPtr<Gnome::Glade::Xml>& m_refXml; // To access GUI's elements
    Gtk::SpinButton* m_gtk_right_click_time;

    /* Event handlers */
    void right_click_time_changed() {
	m_right_click_time = m_gtk_right_click_time->get_value_as_int();
    }

    /* Data */
    delta_t_t m_right_click_time;
};


#endif /* __CONFIGURATOR_H__ */

