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


#ifndef  __GTK_GUI_H__
#define  __GTK_GUI_H__


#include <gtkmm.h>
#include <libglademm.h>
#include <string>
#include <cstdio>
#include <glib.h>

#include "wiicursormanager.h"
#include "configurator.h"
#include "common.h"


// NOTE: GUI stuff is allowed to be a bit messy since they are disposable


// Events which are processed by MainGtkWindow
void wii_left_clicked(WiiEventData const& data);
void wii_right_button_down(WiiEventData const& data);
void wii_right_button_up(WiiEventData const& data);
void wii_begin_click_and_drag(WiiEventData const& data);
void wii_end_click_and_drag(WiiEventData const& data);
void wii_mouse_moved(WiiEventData const& data);


class MainGtkWindow {
public:
    MainGtkWindow(int argc,char *argv[]);

    /* Returns 0 on success */
    int run();
private:
    /* Event handlers */
    void toggle_wiimote_clicked();
    void toggle_activation_clicked();
    void calibrate_clicked();
    void status_icon_clicked();
    void status_icon_popup(guint button, guint32 activate_time);
    void menu_close_clicked();
    void sim_quit_clicked();
    void menu_about_clicked();
    void about_dialog_response(int response_id);
    void wiicursormanager_connect_start_connection();
    void wiicursormanager_connect_finish_connection();
    void wiicursormanager_connect_done_connecting();
    bool connecting_window_progressbar_pulse();
    bool key_pressed(GdkEventKey* event);

    void print_to_output(char const* text);
    void print_to_output(char const* text, bool add_time_stamp);

    /* Helpers */
    void sync_activation_state(bool activated);
    void sync_wiimote_state(bool wiimote_is_connected);
    void sync_wiimote_state_connection_phase(bool starting); // If false then it has finished

    /* GUI */
    //Glib::RefPtr<Gnome::Glade::Xml> m_refXml; // Not necessary
    // Main window
    Gtk::Main m_gtk_kit;
    Gtk::Window* m_gtk_main_window;
    Gtk::ScrolledWindow* m_gtk_output_scroll;
    Gtk::TextView* m_gtk_output;
    Glib::RefPtr<Gtk::TextBuffer> m_output_buffer;
    Gtk::Button* m_gtk_toggle_wiimote;
    Gtk::Button* m_gtk_toggle_activation;
    Gtk::Button* m_gtk_calibrate;
    // Status icon menu
    Glib::RefPtr<Gtk::StatusIcon> m_gtk_status_icon;
    Gtk::Menu* m_gtk_status_icon_menu;
    Gtk::MenuItem* m_gtk_sim_connect;
    Gtk::MenuItem* m_gtk_sim_disconnect;
    Gtk::MenuItem* m_gtk_sim_activate;
    Gtk::MenuItem* m_gtk_sim_deactivate;
    Gtk::MenuItem* m_gtk_sim_calibrate;
    Gtk::MenuItem* m_gtk_sim_quit;
    // Main menu
    Gtk::MenuItem* m_gtk_menu_close;
    Gtk::MenuItem* m_gtk_menu_quit;
    Gtk::MenuItem* m_gtk_menu_about;
    Gtk::MenuItem* m_gtk_menu_instructions;
    // About dialog box
    Gtk::AboutDialog* m_gtk_about_dialog;
    // Connecting window
    Gtk::Window* m_gtk_connecting_window;
    Gtk::Label* m_gtk_label_wiimote_number;
    Gtk::ProgressBar* m_gtk_connecting_progress;
    sigc::connection m_progressbar_pulse_connection;
    // Instructions window
    Gtk::Window* m_gtk_instructions_window;
    Gtk::Image* m_gtk_image_arrangement;
    Gtk::Button* m_gtk_instructions_close;

    Glib::RefPtr<Gtk::TextTag> m_time_text_tag;

    /* Data */
    Configurator& m_configurator;
    WiiCursorManagerConnectEvents m_connect_events;
    WiiCursorManager m_wii_manager;
};


#endif /* __GTK_GUI_H__ */
