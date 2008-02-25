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


void* wii_thread_func(void* ptr) {
    ASSERT(ptr != 0, "No data has been passed along");
    MainGtkWindow& data = *static_cast<MainGtkWindow*>(ptr);
    ASSERT(data.m_wii_thread != 0, "This thread has become immortal, omfg!!!1");

    std::auto_ptr<WiiCursor> const wc ( new WiiCursor(data.m_wiimote, data.m_transform) );
    wc->process(data.m_thread_running); // The main loop

    return 0;
}


MainGtkWindow::MainGtkWindow(int argc,char *argv[]) :
    m_kit(argc, argv),
    m_gtk_main_window(0),
    m_gtk_output(0),
    m_output_buffer( Gtk::TextBuffer::create() ),
    m_gtk_toggle_wiimote(0),
    m_gtk_toggle_activation(0),
    m_gtk_calibrate(0),
    m_transform_matrix_correct(false),
    m_transform(3, 3), // NOTE: We shouldn't have to know about its dimensions :-s . New class?.
    m_wiimote(0), // Also tells us whether the Wiimote has been connected or not
    m_wii_thread(0),
    m_thread_running(false)
{
    // WARNING: Not checking for *any* return values here

    /* GUI */
    char const*const GLADE_MAIN_WINDOW_PATH = "main-window.glade";
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLADE_MAIN_WINDOW_PATH);

    refXml->get_widget("main-window", m_gtk_main_window);
    refXml->get_widget("output", m_gtk_output);
    refXml->get_widget("toggle-wiimote", m_gtk_toggle_wiimote);
    refXml->get_widget("toggle-activation", m_gtk_toggle_activation);
    refXml->get_widget("calibrate", m_gtk_calibrate);

    m_gtk_toggle_wiimote->signal_clicked().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_wiimote_clicked));
    m_gtk_toggle_activation->signal_clicked().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_activation_clicked));
    m_gtk_calibrate->signal_clicked().connect(sigc::mem_fun(*this, &MainGtkWindow::calibrate_clicked));

    m_gtk_output->set_buffer(m_output_buffer);

    /* Data */
    if ( load_config(m_transform) ) {
	sync_configuration_state(true);
	print_to_output("Configuration file successfully loaded.\n");
    }
    else {
	sync_configuration_state(false);
	print_to_output("Failed to load configuration file, you need to calibrate it before connecting the Wiimote.\n");
    }
}

int MainGtkWindow::run() {
    m_kit.run(*m_gtk_main_window);

    // NOTE: Always return 0 for now
    return 0;
}

void MainGtkWindow::toggle_wiimote_clicked() {
    if ( !wiimote_connected() ) {
	m_wiimote = wii_connect(0); // NOTE: We don't care about any command-line parameters for now
	if (m_wiimote) {
	    sync_wiimote_state(true);
	    print_to_output("Successfully connected to the Wiimote.\n");
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
    point_t p_wii[4];
    if ( !get_calibration_points(m_wiimote, p_wii) ) {
	m_transform = calculate_transformation_matrix(p_wii);                                                                                       
	save_config(m_transform);
	sync_configuration_state(true);
	print_to_output("Successfully calibrated and configuration file has been saved.\n");
    }
    else print_to_output("User escaped or there was an error while calibrating the Wii.\n");
}

void MainGtkWindow::start_wii_thread() {
    // NOTE: Not checking for any return value here
    m_thread_running = true;
    pthread_create(&m_wii_thread, 0, &wii_thread_func, this);
    cwiid_enable(m_wiimote, CWIID_FLAG_MESG_IFC);
}
void MainGtkWindow::finish_wii_thread() {
    m_thread_running = false;
    pthread_join(m_wii_thread, 0);
    cwiid_disable(m_wiimote, CWIID_FLAG_MESG_IFC);
}

void MainGtkWindow::print_to_output(char const* text) {
    m_output_buffer->insert_at_cursor(text);
}
void MainGtkWindow::sync_configuration_state(bool configuration_is_valid) {
    m_transform_matrix_correct = configuration_is_valid;
}
void MainGtkWindow::sync_activation_state(bool activated) {
    m_gtk_toggle_activation->set_label(activated ? "De_activate" : "_Activate");
    m_gtk_calibrate->set_sensitive(!activated);
}
void MainGtkWindow::sync_wiimote_state(bool wiimote_is_connected) {
    m_gtk_toggle_wiimote->set_label(wiimote_is_connected ? "Dis_connect" : "_Connect");
    m_gtk_toggle_activation->set_sensitive(wiimote_is_connected);
    m_gtk_calibrate->set_sensitive(wiimote_is_connected);
}

bool MainGtkWindow::wiimote_connected() const {
    return m_wiimote ? true : false;
}
