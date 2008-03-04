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


void CalibrationWindow::calibration_right_button_down(WiimoteEventData const& data) {
    m_cal_data.p_wii[m_cal_data.active_point++] = data.ir_pos;
    if (m_cal_data.active_point == 4)
	finish_calibration_thread();
}
void CalibrationWindow::calibration_mouse_moved(WiimoteEventData const& data) {
    m_cal_data.ir_pos = data.ir_pos;
    m_cal_data.waited = data.waited;
}
void CalibrationWindow::calibration_mouse_down(WiimoteEventData const& data) {
    m_cal_data.ir_on_mouse_down = data.ir_on_mouse_down;
    m_cal_data.border_crossed = false;
}
void CalibrationWindow::calibration_begin_click_and_drag(WiimoteEventData const& data) {
    m_cal_data.border_crossed = true;
}
void* calibration_thread_func(void* ptr) {
    ASSERT(ptr != 0, "No data has been passed along");
    CalibrationWindow& data = *static_cast<CalibrationWindow*>(ptr);
    ASSERT(data.m_calibration_thread != 0, "This thread has become immortal, omfg!!!1");

    WiiCursor wc;
    wc.signal_right_button_down() = sigc::mem_fun(data, &CalibrationWindow::calibration_right_button_down);
    wc.signal_mouse_moved() = sigc::mem_fun(data, &CalibrationWindow::calibration_mouse_moved);
    wc.signal_mouse_down() = sigc::mem_fun(data, &CalibrationWindow::calibration_mouse_down);
    wc.signal_begin_click_and_drag() = sigc::mem_fun(data, &CalibrationWindow::calibration_begin_click_and_drag);
    wc.process( data.m_wiimote, matrix_t(3, 3), data.MOVE_TOLERANCE, data.WAIT_TOLERANCE, data.m_thread_running); // The main loop

    cwiid_disable(data.m_wiimote, CWIID_FLAG_MESG_IFC);
    data.quit();

    return 0;
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

    // WARNING: We should *really* use relative coordinates (0.0-1.0) here.
    // Improvements are welcome, I'm too lazy to clean it up.

    // Wiimote's viewing area
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->set_line_width(2.0);
    unsigned int const WII_VIEWING_AREA_RADIUS = 100;
    point_t const screen_center( scr_size.x/2, scr_size.y/2 );
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
    double const hand_angle = FULL_CIRCLE * static_cast<double>(m_cal_data.waited) / static_cast<double>(WAIT_TOLERANCE);
    cr->set_source_rgb(0.75, 1.0, 1.0);
    cr->set_line_width(2.0);
    cr->move_to(circle_time_pos.x, circle_time_pos.y);
    cr->arc(circle_time_pos.x, circle_time_pos.y, CIRCLE_RADIUS, -FULL_CIRCLE/4.0, hand_angle-FULL_CIRCLE/4.0);
    cr->fill();

    // Another circle showing how far off the current IR pointer from its original position
    point_t const circle_move_pos(screen_center.x+CIRCLE_RADIUS*1.5, screen_center.y+WII_VIEWING_AREA_RADIUS*2);
    // Boundary
    if (!m_cal_data.border_crossed)
	cr->set_source_rgb(0.0, 1.0, 1.0);
    else cr->set_source_rgb(1.0, 0.0, 0.0);
    cr->set_line_width(3.0);
    cr->arc(circle_move_pos.x, circle_move_pos.y, CIRCLE_RADIUS, 0, FULL_CIRCLE);
    cr->stroke();
    // Relative IR position
    unsigned int const DOT_RADIUS = 4;
    point_t const dot_pos(
	circle_move_pos.x+static_cast<int>((double)CIRCLE_RADIUS/(double)MOVE_TOLERANCE)*(m_cal_data.ir_pos.x-m_cal_data.ir_on_mouse_down.x),
	circle_move_pos.y+static_cast<int>((double)CIRCLE_RADIUS/(double)MOVE_TOLERANCE)*(m_cal_data.ir_pos.y-m_cal_data.ir_on_mouse_down.y)
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

void CalibrationWindow::start_calibration_thread() {
    // NOTE: Not checking for any return value here                                                                                                            
    m_thread_running = true;
    pthread_create(&m_calibration_thread, 0, &calibration_thread_func, this);
}
void CalibrationWindow::finish_calibration_thread() {
    m_thread_running = false;
    pthread_join(m_calibration_thread, 0);
}

void CalibrationWindow::quit() {
    m_gtk_window->hide();
}

CalibrationWindow::CalibrationWindow(cwiid_wiimote_t* wiimote, CalibrationData& cal_data) :
    m_wiimote(wiimote),
    m_cal_data(cal_data),
    m_calibration_thread(0),
    m_thread_running(false),
    m_gtk_window(0),
    m_gtk_calibration_area(0)
{
    // Gets the widgets
    std::string const DATA_DIR(DATADIR);
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(DATA_DIR + "/calibration-window.glade");

    refXml->get_widget("calibration-window", m_gtk_window);
    refXml->get_widget("calibration-area", m_gtk_calibration_area);

    // Prepares the widgets
    m_gtk_calibration_area->modify_bg( Gtk::STATE_NORMAL, Gdk::Color("black") );
    m_gtk_calibration_area->signal_expose_event().connect( sigc::mem_fun(*this, &CalibrationWindow::calibration_area_exposed) );
    m_gtk_window->signal_key_press_event().connect( sigc::mem_fun(*this, &CalibrationWindow::calibration_area_key_pressed) );
    m_gtk_window->maximize();
}


int CalibrationWindow::get_calibration_points() {
    Gtk::Main gtk_kit(0, 0); // NOTE: Bypassing potential arguments for GTK+ here

    // Starts the main loop
    start_calibration_thread();
    sigc::connection redraw_connection =
	Glib::signal_timeout().connect(
	    sigc::mem_fun(*this, &CalibrationWindow::redraw_calibration_area), 100 );

    gtk_kit.run(*m_gtk_window);

    redraw_connection.disconnect();
    if (m_thread_running)
	finish_calibration_thread();

    return m_cal_data.active_point != 4;
}
