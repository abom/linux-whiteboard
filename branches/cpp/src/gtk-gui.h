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
#include <pthread.h>
#include <string>
#include <ctime>
#include <cstdio>

#include "common.h"
#include "matrix.h"
#include "auxiliary.h"
#include "gui.h"
#include "wiicursor.h"
#include "events.h"


// NOTE: GUI stuff is allowed to be a bit messy since they are disposable

// NOTE: The *only* reason this is not in MainGtkWindow is
// because pthread is a C library
void* wii_thread_func(void* ptr);


/* NOTE: Read the GNOME HIG! */
class MainGtkWindow {
public:
    MainGtkWindow(int argc,char *argv[]);

    /* Returns 0 on success */
    int run();
private:
    friend void* wii_thread_func(void* ptr); // NOTE: Friends can see your private :-<

    /* Event handlers */
    void toggle_wiimote_clicked();
    void toggle_activation_clicked();
    void calibrate_clicked();
    void status_icon_clicked();
    void status_icon_popup(guint button, guint32 activate_time);
    void sim_quit_clicked();
    void start_wii_thread();
    void finish_wii_thread();

    /* Helpers */
    void print_to_output(char const* text);
    void sync_configuration_state(bool configuration_is_valid);
    void sync_activation_state(bool activated);
    void sync_wiimote_state(bool wiimote_is_connected);
    bool wiimote_connected() const;

    /* GUI */
    Gtk::Main m_gtk_kit;
    Gtk::Window* m_gtk_main_window;
    Gtk::ScrolledWindow* m_gtk_output_scroll;
    Gtk::TextView* m_gtk_output;
    Glib::RefPtr<Gtk::TextBuffer> m_output_buffer;
    Gtk::Button* m_gtk_toggle_wiimote;
    Gtk::Button* m_gtk_toggle_activation;
    Gtk::Button* m_gtk_calibrate;
    Glib::RefPtr<Gtk::StatusIcon> m_gtk_status_icon;
    Gtk::Menu* m_gtk_status_icon_menu;
    Gtk::MenuItem* m_gtk_sim_toggle_wiimote;
    Gtk::MenuItem* m_gtk_sim_toggle_activation;
    Gtk::MenuItem* m_gtk_sim_calibrate;
    Gtk::MenuItem* m_gtk_sim_quit;

    Glib::RefPtr<Gtk::TextTag> m_time_text_tag;

    /* Data */
    bool m_transform_matrix_correct;
    matrix_t m_transform;
    cwiid_wiimote_t* m_wiimote;
    pthread_t m_wii_thread;
    bool m_thread_running;
};


#endif /* ----- #ifndef __GTK_GUI_H__  ----- */
