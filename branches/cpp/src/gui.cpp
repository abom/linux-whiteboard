/* Copyright (C) 2008 Pere Negre                                                                                                                              
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


#include "gui.h"


void CalibrationWindow::calibration_right_button_down(WiiEventData const& data) {
    m_cal_data.p_wii[m_cal_data.active_point++] = data.ir_pos;
}
void CalibrationWindow::calibration_mouse_moved(WiiEventData const& data) {
    m_cal_data.ir_pos = data.ir_pos;
    m_cal_data.waited = data.waited;
    m_cal_data.move_tolerance = data.move_tolerance;
}
void CalibrationWindow::calibration_mouse_down(WiiEventData const& data) {
    m_cal_data.ir_on_mouse_down = data.ir_on_mouse_down;
    m_cal_data.border_crossed = false;
}
void CalibrationWindow::calibration_mouse_up(WiiEventData const& data) {
    m_cal_data.ir_pos.x = INVALID_IR_POS; // So it will not be drawn later
    if (m_cal_data.active_point == 4)
	quit();
}
void CalibrationWindow::calibration_begin_click_and_drag(WiiEventData const& data) {
    m_cal_data.border_crossed = true;
}


void draw_calibration_points(Cairo::RefPtr<Cairo::Context> cr, point_t const points[4], unsigned int active, bool active_light_up) {
    unsigned int const RADIUS = 10;
    for (unsigned int i = 0; i != sizeof(points); ++i) {
	{ // Draws a cross inside the rectangle for easier viewing
	    cr->move_to(points[i].x-RADIUS, points[i].y-RADIUS);
	    cr->line_to(points[i].x+RADIUS, points[i].y+RADIUS);
	    cr->move_to(points[i].x+RADIUS, points[i].y-RADIUS);
	    cr->line_to(points[i].x-RADIUS, points[i].y+RADIUS);
	}
	// Draws the bounding boxes if needed
	if ( (i < active) || ((i == active) && active_light_up) )
	    cr->rectangle(points[i].x-RADIUS, points[i].y-RADIUS, RADIUS*2, RADIUS*2);
    }
}

bool CalibrationWindow::calibration_area_key_pressed(GdkEventKey* event) {
    if (event->keyval == GDK_Escape)
	quit();

    return true;
}
bool CalibrationWindow::calibration_area_exposed(GdkEventExpose* event) {
    Glib::RefPtr<Gdk::Window> window = m_gtk_calibration_area->get_window();
    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
    point_t const scr_size = screen_size();
    point_t const screen_center( scr_size.x/2, scr_size.y/2 );

    // WARNING: We should *really* use relative coordinates (0.0-1.0) here.
    // Improvements are welcome, I'm too lazy to clean it up.

    // Draws a custom user messages, controllable from outside
    cr->set_font_size(32.0);
    Cairo::TextExtents text_extents;
    cr->get_text_extents(m_user_message, text_extents);
    Point<double> const text_position_user_message(
	screen_center.x-text_extents.x_bearing-text_extents.width/2.0,
	screen_center.y-text_extents.y_bearing-text_extents.height/2.0-200.0);
    cr->move_to(text_position_user_message.x, text_position_user_message.y);
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->show_text(m_user_message);
    // Draws a static informational message
    std::string const escape_message(_("Press 'Escape' (Esc) to quit calibration"));
    cr->set_font_size(16.0);
    cr->get_text_extents(escape_message, text_extents);
    Point<double> const text_position_escape_message(
	screen_center.x-text_extents.x_bearing-text_extents.width/2.0,
	scr_size.y-text_extents.y_bearing-text_extents.height/2.0-40.0);
    cr->move_to(text_position_escape_message.x, text_position_escape_message.y);
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->show_text(escape_message);

    // Wiimote's viewing area
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->set_line_width(2.0);
    unsigned int const WII_VIEWING_AREA_RADIUS = 100;
    cr->rectangle(  screen_center.x-WII_VIEWING_AREA_RADIUS, screen_center.y-WII_VIEWING_AREA_RADIUS,
	    WII_VIEWING_AREA_RADIUS*2, WII_VIEWING_AREA_RADIUS*2);
    cr->stroke();

    // Current IR pointer's location, from the Wiimote's eye
    // NOTE: Needs improvement, it looks fugly
    point_t const& ir_pos = m_cal_data.ir_pos; // Readability
    if (ir_pos.x != INVALID_IR_POS) { // There's no point in drawing an invalid IR pointer
	cr->set_source_rgb(1.0, 1.0, 0.0);
	cr->set_line_width(2.0);
	point_t const MAX_WII(1020, 760);
	point_t const ir_pointer_center(
		(screen_center.x-WII_VIEWING_AREA_RADIUS) + static_cast<int>( ((float) ir_pos.x / (float) MAX_WII.x)*(WII_VIEWING_AREA_RADIUS*2)),
		(screen_center.y+WII_VIEWING_AREA_RADIUS) - static_cast<int>( ((float) ir_pos.y / (float) MAX_WII.y)*(WII_VIEWING_AREA_RADIUS*2)) );
	cr->rectangle(ir_pointer_center.x-1.0, ir_pointer_center.y-1.0, 1.0, 1.0);
	cr->stroke();
    }

    // 4 calibration points
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->set_line_width(2.0);
    m_cal_data.active_light_up = !m_cal_data.active_light_up;
    point_t p_screen[4];
    screen_corners(p_screen);
    draw_calibration_points(cr, p_screen, m_cal_data.active_point, m_cal_data.active_light_up); // 4 calibration points
    cr->stroke();

    // Draws bounding boxes around the screen's edges, for easier viewing
    unsigned int const BOX_THICKNESS = 3;
    unsigned int const NUMBER_OF_BOXES = 3;
    cr->set_line_width(BOX_THICKNESS);
    for (unsigned int i = 0; i != NUMBER_OF_BOXES; ++i) {
	cr->set_source_rgb(i*0.25, 1.0, 1.0-i*0.25); // Varies the colors a bit
	cr->rectangle(i*BOX_THICKNESS, i*BOX_THICKNESS, scr_size.x-i*BOX_THICKNESS*2, scr_size.y-i*BOX_THICKNESS*2); // Yeah, '2'
	cr->stroke();
    }

    // A circle showing the elapsed time since the left button down event
    double const FULL_CIRCLE = 2*M_PI;
    unsigned int const CIRCLE_RADIUS = 50;
    point_t const circle_time_pos(screen_center.x-CIRCLE_RADIUS*1.5, screen_center.y+WII_VIEWING_AREA_RADIUS*2);
    // Boundary
    cr->set_source_rgb(0.0, 1.0, 0.0);
    cr->set_line_width(3.0);
    cr->arc(circle_time_pos.x, circle_time_pos.y, CIRCLE_RADIUS, 0, FULL_CIRCLE);
    cr->stroke();
    // 'Hand'?
    double const hand_angle = FULL_CIRCLE * static_cast<double>(m_cal_data.waited) / static_cast<double>(m_thread_data.wait_tolerance);
    cr->set_source_rgb(0.75, 1.0, 1.0);
    cr->set_line_width(2.0);
    cr->move_to(circle_time_pos.x, circle_time_pos.y);
    cr->arc(circle_time_pos.x, circle_time_pos.y, CIRCLE_RADIUS, -FULL_CIRCLE/4.0, hand_angle-FULL_CIRCLE/4.0);
    cr->fill();

    // Another circle showing how far off the current IR pointer from its original position
    point_t const circle_move_pos(screen_center.x+CIRCLE_RADIUS*1.5, screen_center.y+WII_VIEWING_AREA_RADIUS*2);
    unsigned int const MAX_NUMBER_OF_POSITIONS = 7; // NOTE: This is only temporary!
    double const circle_radius_move = MAX_NUMBER_OF_POSITIONS * m_cal_data.move_tolerance;
    // Boundary
    if (!m_cal_data.border_crossed)
	cr->set_source_rgb(0.0, 1.0, 1.0);
    else cr->set_source_rgb(1.0, 0.0, 0.0);
    cr->set_line_width(3.0);
    cr->arc(circle_move_pos.x, circle_move_pos.y, circle_radius_move, 0, FULL_CIRCLE);
    cr->stroke();
    // Relative IR position
    unsigned int const DOT_RADIUS = 4;
    point_t const dot_pos(
	circle_move_pos.x+static_cast<int>(circle_radius_move/m_cal_data.move_tolerance)*(m_cal_data.ir_pos.x-m_cal_data.ir_on_mouse_down.x),
	circle_move_pos.y+static_cast<int>(circle_radius_move/m_cal_data.move_tolerance)*(m_cal_data.ir_pos.y-m_cal_data.ir_on_mouse_down.y)
    );
    if (!m_cal_data.border_crossed) {
	cr->set_source_rgb(1.0, 1.0, 0.0);
	cr->set_line_width(3.0);
	cr->arc(dot_pos.x, dot_pos.y, DOT_RADIUS, 0, FULL_CIRCLE);
	cr->fill();
    }

    return true;
}
bool CalibrationWindow::redraw_calibration_area() {
    m_gtk_calibration_area->queue_draw();

    return true;
}

void CalibrationWindow::quit() {
    // Cleans up as needed
    finish_wii_thread(m_thread_data);

    m_gtk_window->hide();
}

CalibrationWindow::CalibrationWindow(std::vector<WiimoteAndTransformMatrix>& wiimotes, CalibrationData& cal_data, std::string const& user_message) :
    m_gtk_window(0),
    m_gtk_calibration_area(0),
    m_cal_data(cal_data),
    m_user_message(user_message)
{
    // Gets the widgets
    std::string const WINDOWS_DIR(WINDOWSDIR);
    std::string const PIXMAPS_DIR(PIXMAPSDIR);
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(WINDOWS_DIR + "/calibration-window.glade");

    refXml->get_widget("calibration-window", m_gtk_window);
    refXml->get_widget("calibration-area", m_gtk_calibration_area);

    // Prepares the widgets
    m_gtk_calibration_area->modify_bg( Gtk::STATE_NORMAL, Gdk::Color("black") );
    m_gtk_calibration_area->signal_expose_event().connect( sigc::mem_fun(*this, &CalibrationWindow::calibration_area_exposed) );

    std::string const ICON_FILE(PIXMAPS_DIR + "/whiteboard.png");
    m_gtk_window->set_icon_from_file(ICON_FILE);
    m_gtk_window->signal_key_press_event().connect( sigc::mem_fun(*this, &CalibrationWindow::calibration_area_key_pressed) );
    m_gtk_window->maximize();
    m_gtk_window->show();

    // Data
    m_thread_data.wiimotes = wiimotes;

    m_thread_data.wait_tolerance = 1000;

    m_thread_data.events.right_button_down = sigc::mem_fun(*this, &CalibrationWindow::calibration_right_button_down);
    m_thread_data.events.mouse_moved = sigc::mem_fun(*this, &CalibrationWindow::calibration_mouse_moved);
    m_thread_data.events.mouse_down = sigc::mem_fun(*this, &CalibrationWindow::calibration_mouse_down);
    m_thread_data.events.mouse_up = sigc::mem_fun(*this, &CalibrationWindow::calibration_mouse_up);
    m_thread_data.events.begin_click_and_drag = sigc::mem_fun(*this, &CalibrationWindow::calibration_begin_click_and_drag);
}


int CalibrationWindow::get_calibration_points() {
    Gtk::Main gtk_kit(0, 0); // NOTE: Bypassing potential arguments for GTK+ here

    // Starts the main loop
    start_wii_thread(m_thread_data);
    sigc::connection redraw_sigc_connection =
	Glib::signal_timeout().connect(
	    sigc::mem_fun(*this, &CalibrationWindow::redraw_calibration_area), 100 );

    gtk_kit.run(*m_gtk_window);

    // Finished at this point, whether succeeded or escaped by user
    redraw_sigc_connection.disconnect();
    // NOTE: No need to finish_wii_thread() here as it is handled by
    // quit() if user selected 'Quit' from the menu. It self-terminated otherwise.

    return m_cal_data.active_point != 4;
}
