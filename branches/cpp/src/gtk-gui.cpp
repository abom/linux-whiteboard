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

    DEBUG_MSG(2, "Left clicked\n");
}
void wii_right_button_down(WiiEventData const& data) {
    fake_button(RIGHT_BUTTON, true);

    DEBUG_MSG(2, "Right button down\n");
}
void wii_right_button_up(WiiEventData const& data) {
    fake_button(RIGHT_BUTTON, false);

    DEBUG_MSG(2, "Right button up\n");
}
void wii_begin_click_and_drag(WiiEventData const& data) {
    fake_button(LEFT_BUTTON, true);

    DEBUG_MSG(2, "Begin click-n-drag\n");
}
void wii_end_click_and_drag(WiiEventData const& data) {
    fake_button(LEFT_BUTTON, false);

    DEBUG_MSG(2, "End click-n-drag\n");
}
void wii_mouse_moved(WiiEventData const& data) {
    fake_move(data.cursor_pos.x, data.cursor_pos.y);

    // Nothing here as it'd flood stdout
    //DEBUG_MSG(2, "Mouse moved: %dx%d\n", data.cursor_pos.x, data.cursor_pos.y);
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
    m_gtk_sim_connect(0),
    m_gtk_sim_disconnect(0),
    m_gtk_sim_activate(0),
    m_gtk_sim_deactivate(0),
    m_gtk_sim_calibrate(0),
    m_gtk_sim_quit(0),
    m_gtk_menu_quit(0),
    m_gtk_menu_about(0),
    m_gtk_about_dialog(0),
    m_gtk_connecting_window(0),
    m_gtk_label_wiimote_number(0),
    m_gtk_connecting_progress(0),
    m_time_text_tag( m_output_buffer->create_tag("bold") ),
    m_configurator( get_configurator() ),
    m_wii_manager(m_connect_events)
{
    // WARNING: Not checking for *any* return values here
    // WARNING: Constructing paths this way is not safe/portable, but I don't want to bother with g_free()

    DEBUG_MSG(1, "Initializing the main GUI...\n");

    /* Gets GUI components */
    std::string const WINDOWS_DIR(WINDOWSDIR);
    DEBUG_MSG(2, "Glade dir: %s\n", WINDOWS_DIR.c_str());
    std::string const PIXMAPS_DIR(PIXMAPSDIR); // NOTE: Glade's bug, 'icon' does nothing
    DEBUG_MSG(2, "Pixmap dir: %s\n", PIXMAPS_DIR.c_str());
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(WINDOWS_DIR + "/main-window.glade");
    DEBUG_MSG(1, "Loaded glade files\n");

    refXml->get_widget("main-window", m_gtk_main_window);
    refXml->get_widget("output-scroll", m_gtk_output_scroll);
    refXml->get_widget("output", m_gtk_output);
    refXml->get_widget("toggle-wiimote", m_gtk_toggle_wiimote);
    refXml->get_widget("toggle-activation", m_gtk_toggle_activation);
    refXml->get_widget("calibrate", m_gtk_calibrate);
    refXml->get_widget("status-icon-menu", m_gtk_status_icon_menu);
    refXml->get_widget("sim-connect", m_gtk_sim_connect);
    refXml->get_widget("sim-disconnect", m_gtk_sim_disconnect);
    refXml->get_widget("sim-activate", m_gtk_sim_activate);
    refXml->get_widget("sim-deactivate", m_gtk_sim_deactivate);
    refXml->get_widget("sim-calibrate", m_gtk_sim_calibrate);
    refXml->get_widget("sim-quit", m_gtk_sim_quit);
    refXml->get_widget("menuitem-close", m_gtk_menu_close);
    refXml->get_widget("menuitem-quit", m_gtk_menu_quit);
    refXml->get_widget("menuitem-about", m_gtk_menu_about);
    refXml->get_widget("about-dialog", m_gtk_about_dialog);
    refXml->get_widget("connecting-window", m_gtk_connecting_window);
    refXml->get_widget("connecting-window-label-wiimote-number", m_gtk_label_wiimote_number);
    refXml->get_widget("connecting-window-progress", m_gtk_connecting_progress);
    DEBUG_MSG(1, "Loaded GUI components\n");

    m_gtk_toggle_wiimote->signal_clicked().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_wiimote_clicked));
    m_gtk_toggle_activation->signal_clicked().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_activation_clicked));
    m_gtk_calibrate->signal_clicked().connect(sigc::mem_fun(*this, &MainGtkWindow::calibrate_clicked));
    m_gtk_status_icon->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::status_icon_clicked));
    m_gtk_status_icon->signal_popup_menu().connect(sigc::mem_fun(*this, &MainGtkWindow::status_icon_popup));
    // NOTE: '(Dis)connect' and '(De)activate' connections below are *wrong*,
    // but they work if we keep the state of the menu consistent.
    m_gtk_sim_connect->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_wiimote_clicked));
    m_gtk_sim_disconnect->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_wiimote_clicked));
    m_gtk_sim_activate->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_activation_clicked));
    m_gtk_sim_deactivate->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::toggle_activation_clicked));
    m_gtk_sim_calibrate->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::calibrate_clicked));
    m_gtk_sim_quit->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::sim_quit_clicked));
    m_gtk_menu_close->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::menu_close_clicked));
    m_gtk_menu_quit->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::sim_quit_clicked));
    m_gtk_menu_about->signal_activate().connect(sigc::mem_fun(*this, &MainGtkWindow::menu_about_clicked));
    m_gtk_about_dialog->signal_response().connect(sigc::mem_fun(*this, &MainGtkWindow::about_dialog_response));
    DEBUG_MSG(1, "All signals connected\n");

    /* Sets up widgets */
    m_gtk_output->set_buffer(m_output_buffer);
    m_time_text_tag->property_font() = "bold";

    std::string const ICON_FILE(PIXMAPS_DIR + "/whiteboard.png");
    m_gtk_main_window->set_icon_from_file(ICON_FILE);
    m_gtk_status_icon->set_from_file(ICON_FILE);
    m_gtk_about_dialog->set_icon_from_file(ICON_FILE);
    m_gtk_connecting_window->set_icon_from_file(ICON_FILE);
    m_gtk_main_window->signal_key_press_event().connect( sigc::mem_fun(*this, &MainGtkWindow::key_pressed) );

    sync_wiimote_state(false); // Disconnected by default

    DEBUG_MSG(1, "Widgets have been synchronized\n");

    /* Data */
    m_configurator.init(refXml);
    if ( m_configurator.load_other_config() )
	print_to_output(_("Configurations successfully loaded.\n"));
    else print_to_output(_("Failed to load configuration file.\n"));

    DEBUG_MSG(1, "Attempted to load config file\n");

    m_connect_events.start_each_connection.connect(
	sigc::mem_fun(*this, &MainGtkWindow::wiicursormanager_connect_start_connection));
    m_connect_events.finish_each_connection.connect(
	sigc::mem_fun(*this, &MainGtkWindow::wiicursormanager_connect_finish_connection));
    m_connect_events.done_connecting.connect(
	sigc::mem_fun(*this, &MainGtkWindow::wiicursormanager_connect_done_connecting)); 
    m_wii_manager.events().left_clicked = sigc::ptr_fun(&wii_left_clicked);
    m_wii_manager.events().right_button_down = sigc::ptr_fun(&wii_right_button_down);
    m_wii_manager.events().right_button_up = sigc::ptr_fun(&wii_right_button_up);
    m_wii_manager.events().begin_click_and_drag = sigc::ptr_fun(&wii_begin_click_and_drag);
    m_wii_manager.events().end_click_and_drag = sigc::ptr_fun(&wii_end_click_and_drag);
    m_wii_manager.events().mouse_moved = sigc::ptr_fun(&wii_mouse_moved);

    DEBUG_MSG(1, "Mouse event handlers connected\n");

    DEBUG_MSG(1, "Main GUI initialized\n");
}

int MainGtkWindow::run() {
    DEBUG_MSG(1, "About to run the main GUI...\n");
    m_gtk_kit.run();
    DEBUG_MSG(1, "Main GUI's run() exit\n");

    // NOTE: Always return 0 for now
    return 0;
}

void MainGtkWindow::toggle_wiimote_clicked() {
    DEBUG_MSG(1, "Toggle Wiimote button clicked\n");

    if ( !m_wii_manager.connected() ) {
	DEBUG_MSG(1, "Wiimotes not connected\n");
	// NOTE: This is mainly to force the GUI to update itself
	// WARNING: Somehow I still feel the problem has not
	// been completely solved.
	print_to_output(_("Preparing to connect...\n"));

	sync_wiimote_state_connection_phase(true);
	DEBUG_MSG(1, "Widgets synchrorized to 'connecting' phase\n");

	DEBUG_MSG(1, "Begin connecting to all Wiimotes...\n");
	m_wii_manager.connect();
	// The stopping process will be handled in wiicursor_connect_done_connecting()
    }
    else {
	DEBUG_MSG(1, "Wiimotes connected\n");
	// Activation
	if ( m_wii_manager.activated() ) {
	    DEBUG_MSG(1, "Wiimotes activated. Deactivating...\n");
	    toggle_activation_clicked();
	}
	// Configurations
	// NOTE: We don't check anything here because we will *not* do anything if it fails
	DEBUG_MSG(1, "Saving config file...\n");
	m_configurator.save_config();
	// Connections
	if ( m_wii_manager.disconnect() )
	    print_to_output(_("Successfully disconnected all Wiimotes.\n"));
	else print_to_output(_("There was an error disconnecting the Wiimotes. Hell's broken loose!!1.\n"));
	DEBUG_MSG(1, "Attempted to disconnect all Wiimotes\n");
	// But we assume it was successfully disconnected anyway, can't do anything about that
	sync_wiimote_state(false);
	DEBUG_MSG(1, "Widgets synchronized\n");
    }
}
void MainGtkWindow::toggle_activation_clicked() {
    DEBUG_MSG(1, "Toggle Wiimote activation button clicked\n");

    if ( !m_wii_manager.activated() ) {
	sync_activation_state(true);
	m_wii_manager.activate();
    }
    else {
	m_wii_manager.deactivate();
	sync_activation_state(false);
    }
}
void MainGtkWindow::calibrate_clicked() {
    DEBUG_MSG(1, "Calibrate button clicked\n");

    //if ( m_wii_manager.calibrate() )
	//print_to_output(_("Calibration succeeded.\n"));
    //else print_to_output(_("User escaped or there was an error during calibration.\n"));
    m_wii_manager.calibrate();
}
void MainGtkWindow::status_icon_clicked() {
    DEBUG_MSG(1, "Status icon clicked\n");

    m_gtk_main_window->property_visible() = !m_gtk_main_window->property_visible();
}
void MainGtkWindow::status_icon_popup(guint button, guint32 activate_time) {
    DEBUG_MSG(1, "Status icon popped up\n");

    m_gtk_status_icon->popup_menu_at_position(*m_gtk_status_icon_menu, button, activate_time);
}
void MainGtkWindow::sim_quit_clicked() {
    DEBUG_MSG(1, "Quit menu option clicked\n");

    // Cleans up as needed
    if ( m_wii_manager.connected() ) {
	DEBUG_MSG(1, "Wiimotes connected. Disconnecting...\n");
	toggle_wiimote_clicked();
    }

    DEBUG_MSG(1, "Quitting main GUI...\n");
    m_gtk_kit.quit();
}
void MainGtkWindow::menu_close_clicked() {
    DEBUG_MSG(1, "Main GUI closed\n");

    m_gtk_main_window->hide();
}
void MainGtkWindow::menu_about_clicked() {
    //DEBUG_MSG(1, "About dialog opening...\n");

    // NOTE: I haven't figured out how to set
    // the focus to this dialog.
    m_gtk_about_dialog->show();
}
void MainGtkWindow::about_dialog_response(int response_id) {
    //DEBUG_MSG(1, "About dialog responding...\n");

    switch (response_id) {
	case Gtk::RESPONSE_CANCEL:
	    m_gtk_about_dialog->hide();
	    break;
	default:
	    break;
    }
}

void MainGtkWindow::wiicursormanager_connect_start_connection() {
    unsigned int const index = m_wii_manager.connected() + 1;

    // WARNING: C function. I'd have used std::ostringstream if not for l10n.
    char out[1024];
    sprintf(out, _("Connecting to Wiimote #%d... "), index);
    print_to_output(out);
    m_gtk_label_wiimote_number->set_text(out);
    m_gtk_connecting_window->show();
    DEBUG_MSG(1, "Connection window openned for Wiimote #%d...\n");

    m_progressbar_pulse_connection =
	Glib::signal_timeout().connect(
	    sigc::mem_fun(*this, &MainGtkWindow::connecting_window_progressbar_pulse), 50 );
    DEBUG_MSG(1, "Activated 'pulse' progressbar\n");
}
void MainGtkWindow::wiicursormanager_connect_finish_connection() {
    bool const connected = m_wii_manager.last_connection_succeeded();

    print_to_output(connected ? _("Succeeded!.\n") : _("Failed.\n"), false);

    m_progressbar_pulse_connection.disconnect();
    DEBUG_MSG(1, "Deactivated 'pulse' progressbar\n");

    m_gtk_connecting_window->hide();
    DEBUG_MSG(1, "Connection window closed\n");
}
void MainGtkWindow::wiicursormanager_connect_done_connecting() {
    DEBUG_MSG(1, "Done connecting to all Wiimotes\n");

    unsigned int const number_of_connected = m_wii_manager.connected();

    sync_wiimote_state_connection_phase(false);
    DEBUG_MSG(1, "Widgets synched to 'not connected' state\n");
    // WARNING: C function. I'd have used std::ostringstream if not for l10n.
    if (number_of_connected) {
	char out[1024];
	sprintf(out, _("Successfully connected to %d Wiimote(s). Loading all Wiimotes's configurations...\n"), number_of_connected);
	print_to_output(out);

	if ( m_configurator.load_wiimotes_config() )
	    print_to_output(_("Done!. Click 'Activate' to use your infrared pen.\n"));
	else print_to_output(_( "Failed."
				" Please check if you have the same number of Wiimotes"
				" as earlier. You need to calibrate before activating.\n"));
	DEBUG_MSG(1, "Attempted to load config file...\n");

	sync_wiimote_state(true);
	DEBUG_MSG(1, "Widgets synched to 'ready to activate and calibrate' state\n");
    }
    else print_to_output(_("Unable to connect to any Wiimote.\n"));
}
bool MainGtkWindow::connecting_window_progressbar_pulse() {
    m_gtk_connecting_progress->pulse();

    return true;
}
bool MainGtkWindow::key_pressed(GdkEventKey* event) {
    DEBUG_MSG(1, "Key pressed on the main window\n");

    bool const will_close = (event->keyval == GDK_Escape);
    if (will_close)
	menu_close_clicked();

    return true;
}

void MainGtkWindow::print_to_output(char const* text) {
    print_to_output(text, true);
}
void MainGtkWindow::print_to_output(char const* text, bool add_time_stamp) {
    DEBUG_MSG(3, "Printing messages to main output\n");

    if (add_time_stamp) {
	// Gets current time of day
	time_t const current_time = time(0);
	tm const* const _tm = localtime(&current_time);
	// Text representation of the current time
	// WARNING: C function. I don't know of a C++ equivalent of '%.2d'
	char current_time_text[12];
	sprintf(current_time_text, "(%.2d:%.2d:%.2d) ", _tm->tm_hour, _tm->tm_min, _tm->tm_sec);
	// Inserts and makes the time more visible
	m_output_buffer->insert(m_output_buffer->end(), current_time_text);
	DEBUG_MSG(4, "Added timestamp\n");
	Gtk::TextBuffer::iterator time_text_begin = m_output_buffer->end();
	time_text_begin.backward_chars( sizeof(current_time_text) );
	m_output_buffer->apply_tag( m_time_text_tag, time_text_begin, m_output_buffer->end() );
	DEBUG_MSG(4, "Applied tag\n");
    }

    m_output_buffer->insert(m_output_buffer->end(), text);
    DEBUG_MSG(4, "Added the message\n");

    // Scrolls to the newest text
    Gtk::Adjustment *const vadj = m_gtk_output_scroll->get_vadjustment();
    vadj->set_value( vadj->get_upper() );
    DEBUG_MSG(4, "Adjusted the scrollbar\n");

    DEBUG_MSG(3, "Done printing messages\n");
}
void MainGtkWindow::sync_activation_state(bool activated) {
    m_gtk_toggle_activation->set_label(activated ? _("De_activate") : _("_Activate"));
    m_gtk_calibrate->set_sensitive(!activated);

    m_gtk_sim_activate->set_sensitive(!activated);
    m_gtk_sim_deactivate->set_sensitive(activated);
    m_gtk_sim_calibrate->set_sensitive(!activated);
}
void MainGtkWindow::sync_wiimote_state(bool connected) {
    m_gtk_toggle_wiimote->set_label(connected ? "gtk-disconnect" : "gtk-connect");
    m_gtk_toggle_activation->set_sensitive(connected);
    m_gtk_calibrate->set_sensitive(connected);

    m_gtk_sim_connect->set_sensitive(!connected);
    m_gtk_sim_disconnect->set_sensitive(connected);
    m_gtk_sim_activate->set_sensitive(connected);
    m_gtk_sim_deactivate->set_sensitive(false);
    m_gtk_sim_calibrate->set_sensitive(connected);
}
void MainGtkWindow::sync_wiimote_state_connection_phase(bool starting) {
    m_gtk_toggle_wiimote->set_sensitive(!starting);
    m_gtk_sim_connect->set_sensitive(!starting);
}
