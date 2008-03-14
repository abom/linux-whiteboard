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


#include "gtk-gui.h"


void wii_left_clicked(WiiEventData const& data) {
    fake_button(LEFT_BUTTON, true);
    fake_button(LEFT_BUTTON, false);
}
void wii_right_button_down(WiiEventData const& data) {
    fake_button(RIGHT_BUTTON, true);
}
void wii_right_button_up(WiiEventData const& data) {
    fake_button(RIGHT_BUTTON, false);
}
void wii_begin_click_and_drag(WiiEventData const& data) {
    fake_button(LEFT_BUTTON, true);
}
void wii_end_click_and_drag(WiiEventData const& data) {
    fake_button(LEFT_BUTTON, false);
}
void wii_mouse_moved(WiiEventData const& data) {
    fake_move(data.cursor_pos.x, data.cursor_pos.y);
}


MainGtkWindow::MainGtkWindow(int argc,char *argv[]) :
    m_gtk_kit(argc, argv),
    m_gtk_main_window(0),
    m_gtk_output_scroll(0),
    m_gtk_output(0),
    m_output_buffer( Gtk::TextBuffer::create() ),
    m_gtk_toggle_wiimote(0),
    m_gtk_toggle_activation(0),
    m_gtk_calibrate(0),
    m_gtk_status_icon( Gtk::StatusIcon::create("whiteboard-status-icon") ),
    m_gtk_status_icon_menu(0),
    m_gtk_sim_quit(0),
    m_time_text_tag( m_output_buffer->create_tag("bold") )
{
    // WARNING: Not checking for *any* return values here
    // WARNING: Constructing paths this way is not safe/portable, but I don't want to bother with g_free()

    /* GUI */
    std::string const WINDOWS_DIR(WINDOWSDIR);
    std::string const PIXMAPS_DIR(PIXMAPSDIR);
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(WINDOWS_DIR + "/main-window.glade");

    refXml->get_widget("main-window", m_gtk_main_window);
    refXml->get_widget("output-scroll", m_gtk_output_scroll);
    refXml->get_widget("output", m_gtk_output);
    refXml->get_widget("toggle-wiimote", m_gtk_toggle_wiimote);
    refXml->get_widget("toggle-activation", m_gtk_toggle_activation);
    refXml->get_widget("calibrate", m_gtk_calibrate);
    refXml->get_widget("status-icon-menu", m_gtk_status_icon_menu);
    refXml->get_widget("sim-toggle-wiimote", m_gtk_sim_toggle_wiimote);
    refXml->get_widget("sim-toggle-activation", m_gtk_sim_toggle_activation);
    refXml->get_widget("sim-calibrate", m_gtk_sim_calibrate);
    refXml->get_widget("sim-quit", m_gtk_sim_quit);

    m_gtk_toggle_wiimote->signal_clicked().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_wiimote_clicked));
    m_gtk_toggle_activation->signal_clicked().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_activation_clicked));
    m_gtk_calibrate->signal_clicked().connect(sigc::mem_fun(*this, &MainGtkWindow::calibrate_clicked));
    m_gtk_status_icon->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::status_icon_clicked));
    m_gtk_status_icon->signal_popup_menu().connect(sigc::mem_fun(*this, &MainGtkWindow::status_icon_popup));
    m_gtk_sim_toggle_wiimote->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_wiimote_clicked));
    m_gtk_sim_toggle_activation->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_activation_clicked));
    m_gtk_sim_calibrate->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::calibrate_clicked));
    m_gtk_sim_quit->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::sim_quit_clicked));

    m_gtk_output->set_buffer(m_output_buffer);
    m_time_text_tag->property_font() = "bold";

    std::string const ICON_FILE(PIXMAPS_DIR + "/whiteboard.svg");
    m_gtk_main_window->set_icon_from_file(ICON_FILE);
    m_gtk_status_icon->set_from_file(ICON_FILE);

    /* Data */
    // NOTE: Load configs here
    /*if ( load_config(m_thread_data.transform) ) {
	print_to_output(_(  "Configuration file successfully loaded."
			    " To connect the Wiimote, press 1+2 on the Wiimote then click 'Connect'.\n"));
    }
    else print_to_output(_("Failed to load configuration file, you need to calibrate it before activating the Wiimote.\n"));*/

    m_wii_manager.tolerances(5, 700);

    m_wii_manager.events().left_clicked = sigc::ptr_fun(&wii_left_clicked);
    m_wii_manager.events().right_button_down = sigc::ptr_fun(&wii_right_button_down);
    m_wii_manager.events().right_button_up = sigc::ptr_fun(&wii_right_button_up);
    m_wii_manager.events().begin_click_and_drag = sigc::ptr_fun(&wii_begin_click_and_drag);
    m_wii_manager.events().end_click_and_drag = sigc::ptr_fun(&wii_end_click_and_drag);
    m_wii_manager.events().mouse_moved = sigc::ptr_fun(&wii_mouse_moved);
}

int MainGtkWindow::run() {
    m_gtk_kit.run();

    // NOTE: Always return 0 for now
    return 0;
}

void MainGtkWindow::toggle_wiimote_clicked() {
    if ( !m_wii_manager.connected() ) {
	if ( m_wii_manager.connect() ) {
	    sync_wiimote_state(true);
	    print_to_output(_("All Wiimotes are connected. Click 'Activate' to use your infrared pen.\n"));
	}
	else print_to_output(_("Failed to connect to the Wiimotes.\n"));
    }
    else {
	if ( m_wii_manager.disconnect() )
	    print_to_output(_("Successfully disconnected all Wiimotes.\n"));
	else print_to_output(_("There was an error disconnecting the Wiimotes. Hell's broken loose!!1.\n"));
	// But we assume it was successfully disconnected anyway, can't do anything about that
	sync_wiimote_state(false);
    }
}
void MainGtkWindow::toggle_activation_clicked() {
    if ( !m_wii_manager.activated() ) {
	m_wii_manager.activate();
	sync_activation_state(true);
    }
    else {
	m_wii_manager.deactivate();
	sync_activation_state(false);
    }
}
void MainGtkWindow::calibrate_clicked() {
    if ( m_wii_manager.calibrate() ) {
	//save_config(m_thread_data.transform); // NOTE: *SAVE!!!*
	print_to_output(_("Calibration succeeded.\n"));
    }
    else print_to_output(_("User escaped or there was an error during calibration.\n"));
}
void MainGtkWindow::status_icon_clicked() {
    m_gtk_main_window->property_visible() = !m_gtk_main_window->property_visible();
}
void MainGtkWindow::status_icon_popup(guint button, guint32 activate_time) {
    m_gtk_status_icon->popup_menu_at_position(*m_gtk_status_icon_menu, button, activate_time);
}
void MainGtkWindow::sim_quit_clicked() {
    // Cleans up as needed
    m_wii_manager.disconnect();

    m_gtk_kit.quit();
}

void MainGtkWindow::print_to_output(char const* text) {
    // Gets current time of day
    time_t const current_time = time(0);
    tm const* const _tm = localtime(&current_time);

    // NOTE: C funtion, sigh...
    char current_time_text[12];
    sprintf(current_time_text, "<%.2d:%.2d:%.2d> ", _tm->tm_hour, _tm->tm_min, _tm->tm_sec);

    // Moves to the end first
    m_output_buffer->place_cursor( m_output_buffer->end() );

    // Makes the time more visible
    m_output_buffer->insert_at_cursor(current_time_text);
    Gtk::TextBuffer::iterator time_text_begin = m_output_buffer->end();
    time_text_begin.backward_chars( sizeof(current_time_text) );
    m_output_buffer->apply_tag( m_time_text_tag, time_text_begin, m_output_buffer->end() );

    m_output_buffer->insert_at_cursor(text);

    // Moves to the end again to show the newest text
    Gtk::Adjustment *const vadj = m_gtk_output_scroll->get_vadjustment();
    vadj->set_value( vadj->get_upper() );
}
void MainGtkWindow::sync_activation_state(bool activated) {
    m_gtk_toggle_activation->set_label(activated ? _("De_activate") : _("_Activate"));
    //m_gtk_sim_toggle_activation->set_label(activated ? "De_activate" : "_Activate");
    m_gtk_calibrate->set_sensitive(!activated);
    m_gtk_sim_calibrate->set_sensitive(!activated);
}
void MainGtkWindow::sync_wiimote_state(bool wiimote_is_connected) {
    m_gtk_toggle_wiimote->set_label(wiimote_is_connected ? "gtk-disconnect" : "gtk-connect");
    //m_gtk_sim_toggle_wiimote->set_label(wiimote_is_connected ? "Dis_connect" : "_Connect");
    m_gtk_toggle_activation->set_sensitive(wiimote_is_connected);
    m_gtk_sim_toggle_activation->set_sensitive(wiimote_is_connected);
    m_gtk_calibrate->set_sensitive(wiimote_is_connected);
    m_gtk_sim_calibrate->set_sensitive(wiimote_is_connected);
}
