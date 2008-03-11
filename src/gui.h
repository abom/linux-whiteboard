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


#ifndef  __GUI_H__
#define  __GUI_H__


#include <gtkmm.h>
#include <gdkmm.h>
#include <libglademm.h>
#include <cairomm/context.h>

#include "auxiliary.h"
#include "wiicursor.h"
#include "common.h"


// For future needs and to avoid point_t p_wii[4] BS
struct CalibrationData {
    point_t p_wii[4];
    unsigned int active_point; // Index of the current point that is being calibrated
    bool active_light_up; // Makes current point blink as needed
    point_t ir_pos; // Current IR pointer's location
    point_t ir_on_mouse_down; // IR pointer on left button down event
    delta_t_t waited; // For drawing of a pretty circle
    bool border_crossed; // Will be true if the IR pointer 'moved' (i.e., no longer valid as a point)

    CalibrationData() :
	active_point(0),
	active_light_up(true),
	ir_pos(INVALID_IR_POS, 0),
	waited(0),
	border_crossed(false)
    { }
};


/* Draws the 4 calibration points, paints the
 * active point a blinking square, and paints the
 * calibrated ones static squares */
void draw_calibration_points(Cairo::RefPtr<Cairo::Context> cr, point_t const points[4], unsigned int active, bool active_light_up);

// NOTE: The *only* reason this is not in CalibrationWindow is
// because pthread is a C library
void* calibration_thread_func(void* ptr);

class CalibrationWindow {
public:
    CalibrationWindow(cwiid_wiimote_t* wiimote, CalibrationData& cal_data);

    /* Get 4 calibration points from users
     * Points are written to p_wii
     * Returns 0 on success, -1 on error or user escapes */
    int get_calibration_points();
private:
    /* Event handlers for the calibration window */
    bool calibration_area_key_pressed(GdkEventKey* event);
    bool calibration_area_exposed(GdkEventExpose* event);
    bool redraw_calibration_area();

    void calibration_right_button_down(WiiEventData const& data);
    void calibration_mouse_moved(WiiEventData const& data);
    void calibration_mouse_down(WiiEventData const& data);
    void calibration_begin_click_and_drag(WiiEventData const& data);

    void quit(); // Tells GTK+ to quit, either successfully calibrated or not

    /* GUI stuff */
    Gtk::Window* m_gtk_window;
    Gtk::DrawingArea* m_gtk_calibration_area;

    /* Data */
    CalibrationData& m_cal_data;
    WiiThreadFuncData m_thread_data;
};


#endif /* __GUI_H__ */
