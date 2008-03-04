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


void wii_left_clicked(WiimoteEventData const& data) {
    fake_button(LEFT_BUTTON, true);
    fake_button(LEFT_BUTTON, false);
}
void wii_right_button_down(WiimoteEventData const& data) {
    fake_button(RIGHT_BUTTON, true);
}
void wii_right_button_up(WiimoteEventData const& data) {
    fake_button(RIGHT_BUTTON, false);
}
void wii_begin_click_and_drag(WiimoteEventData const& data) {
    fake_button(LEFT_BUTTON, true);
}
void wii_end_click_and_drag(WiimoteEventData const& data) {
    fake_button(LEFT_BUTTON, false);
}
void wii_mouse_moved(WiimoteEventData const& data) {
    fake_move(data.cursor_pos.x, data.cursor_pos.y);
}
void* wii_thread_func(void* ptr) {
    ASSERT(ptr != 0, "No data has been passed along");
    MainGtkWindow& data = *static_cast<MainGtkWindow*>(ptr);
    ASSERT(data.m_wii_thread != 0, "This thread has become immortal, omfg!!!1");

    unsigned int const MOVE_TOLERANCE = 5;
    unsigned int const WAIT_TOLERANCE = 700;
    WiiCursor wc;
    wc.signal_left_clicked() = sigc::ptr_fun(&wii_left_clicked);
    wc.signal_right_button_down() = sigc::ptr_fun(&wii_right_button_down);
    wc.signal_right_button_up() = sigc::ptr_fun(&wii_right_button_up);
    wc.signal_begin_click_and_drag() = sigc::ptr_fun(&wii_begin_click_and_drag);
    wc.signal_end_click_and_drag() = sigc::ptr_fun(&wii_end_click_and_drag);
    wc.signal_mouse_moved() = sigc::ptr_fun(&wii_mouse_moved);
    wc.process( data.m_wiimote, data.m_transform, MOVE_TOLERANCE, WAIT_TOLERANCE, data.m_thread_running); // The main loop

    cwiid_disable(data.m_wiimote, CWIID_FLAG_MESG_IFC);

    return 0;
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
    m_time_text_tag( m_output_buffer->create_tag("bold") ),
    m_transform_matrix_correct(false),
    m_transform(3, 3), // NOTE: We shouldn't have to know about its dimensions :-s . New class?.
    m_wiimote(0), // Also tells us whether the Wiimote has been connected or not
    m_wii_thread(0),
    m_thread_running(false)
{
    // WARNING: Not checking for *any* return values here
    // WARNING: Constructing paths this way is not safe/portable, but I don't want to bother with g_free()

    /* GUI */
    std::string const DATA_DIR(DATADIR);
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(DATA_DIR + "/main-window.glade");

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

    m_gtk_status_icon->set_from_file(DATA_DIR + "/icon.svg");

    /* Data */
    if ( load_config(m_transform) ) {
	sync_configuration_state(true);
	print_to_output("Configuration file successfully loaded."
			" To connect the Wiimote, press 1+2 on the Wiimote"
			" then click the 'Connect' button.\n");
    }
    else {
	sync_configuration_state(false);
	print_to_output("Failed to load configuration file, you need to calibrate it before connecting the Wiimote.\n");
    }
}

int MainGtkWindow::run() {
    m_gtk_kit.run();

    // NOTE: Always return 0 for now
    return 0;
}

void MainGtkWindow::toggle_wiimote_clicked() {
    if ( !wiimote_connected() ) {
	//m_gtk_msg_connect_wiimote->show();
	m_wiimote = wii_connect(0); // NOTE: We don't care about any command-line parameters for now
	if (m_wiimote) {
	    sync_wiimote_state(true);
	    print_to_output("Successfully connected to the Wiimote."
			    " Click 'Activate' to use your infrared pen.\n");
	}
	else print_to_output("Failed to connect to the Wiimote.\n");
    }
    else {
	if (m_thread_running)
	    finish_wii_thread();
	if (!wii_disconnect(m_wiimote))
	    print_to_output("Successfully disconnected the Wiimote.\n");
	else print_to_output("There was an error disconnecting the Wiimote. Hell's broken loose!!1.\n");
	// But we assume it was successfully disconnected anyway, can't do anything about that
	m_wiimote = 0;
	sync_wiimote_state(false);
    }
}
void MainGtkWindow::toggle_activation_clicked() {
    if (!m_thread_running) {
	start_wii_thread();
	sync_activation_state(true);
    }
    else {
	finish_wii_thread();
	sync_activation_state(false);
    }
}
void MainGtkWindow::calibrate_clicked() {
    CalibrationData cal_data;
    CalibrationWindow cal_window(m_wiimote, cal_data);
    if ( !cal_window.get_calibration_points() ) {
	m_transform = calculate_transformation_matrix(cal_data.p_wii);
	save_config(m_transform);
	sync_configuration_state(true);
	print_to_output("Successfully calibrated and configuration file has been saved.\n");
    }
    else print_to_output("User escaped or there was an error while calibrating the Wii.\n");

    cwiid_disable(m_wiimote, CWIID_FLAG_MESG_IFC);
}
void MainGtkWindow::status_icon_clicked() {
    if ( m_gtk_main_window->property_visible() )
	m_gtk_main_window->hide();
    else m_gtk_main_window->show();
}
void MainGtkWindow::status_icon_popup(guint button, guint32 activate_time) {
    m_gtk_status_icon->popup_menu_at_position(*m_gtk_status_icon_menu, button, activate_time);
}
void MainGtkWindow::sim_quit_clicked() {
    // Cleans up as needed
    if ( wiimote_connected() )
	toggle_wiimote_clicked();

    m_gtk_kit.quit();
}

void MainGtkWindow::start_wii_thread() {
    // NOTE: Not checking for any return value here
    // NOTE: Remember to find out how to clear the Wiimote cache
    m_thread_running = true;
    pthread_create(&m_wii_thread, 0, &wii_thread_func, this);
}
void MainGtkWindow::finish_wii_thread() {
    if (m_thread_running) {
	m_thread_running = false;
	pthread_join(m_wii_thread, 0);
    }
}

void MainGtkWindow::print_to_output(char const* text) {
    // Gets current time of day
    time_t const current_time = time(0);
    tm const* const _tm = localtime(&current_time);

    // NOTE: C funtion, sign...
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
void MainGtkWindow::sync_configuration_state(bool configuration_is_valid) {
    m_transform_matrix_correct = configuration_is_valid;
}
void MainGtkWindow::sync_activation_state(bool activated) {
    m_gtk_toggle_activation->set_label(activated ? "De_activate" : "_Activate");
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

bool MainGtkWindow::wiimote_connected() const {
    return m_wiimote ? true : false;
}
