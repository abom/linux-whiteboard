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

#include "configurator.h"
#include "auxiliary.h"
#include "wiicursor.h"
#include "wiicontrol.h"
#include "common.h"


/* Draws the calibration points, paints the
 * active point a blinking square, and paints the
 * calibrated ones static squares */
void draw_calibration_points(
    Cairo::RefPtr<Cairo::Context> cr,
    point_t const points[WIIMOTE_NUM_CALIBRATED_POINTS],
    unsigned int active, bool active_light_up);

class CalibrationWindow {
public:
    CalibrationWindow(cwiid_wiimote_t* wiimote, char const* user_message);

    /* Get calibration points from users
     * Points are written to p_wii
     * Returns true on success */
    bool get_calibration_points(WiimoteCalibratedPoints& p_wii);

    void quit(); // Tells GTK+ to quit, either successfully calibrated or not
private:
    /* Event handlers for the calibration window */
    bool calibration_area_key_pressed(GdkEventKey* event);
    bool calibration_area_exposed(GdkEventExpose* event);
    bool redraw_calibration_area();
    bool wiimote_blinking(); // Blinks the LEDs to notice user

    void calibration_right_button_down(WiiEventData const& data);
    void calibration_mouse_moved(WiiEventData const& data);
    void calibration_mouse_down(WiiEventData const& data);
    void calibration_mouse_up(WiiEventData const& data);
    void calibration_begin_click_and_drag(WiiEventData const& data);

    /* GUI stuff */
    Gtk::Window* m_gtk_window;
    Gtk::DrawingArea* m_gtk_calibration_area;
    // Don't worry about those below, these are just for fun
    unsigned int m_wiimote_blinking_lighted_up_led;
    int m_wiimote_blinking_led_direction;

    /* Data */
    char const* m_user_message;
    std::vector<WiimoteData> m_wiimote; // To be passed to m_thread_data
    WiiThreadFuncData m_thread_data;
    // Calibration data
    struct CalibrationData {
	WiimoteCalibratedPoints p_wii;
	unsigned int active_point; // Index of the current point that is being calibrated
	bool active_light_up; // Makes current point blink as needed
	point_t ir_pos; // Current IR pointer's location
	point_t ir_on_mouse_down; // IR pointer on left button down event
	delta_t_t waited; // For drawing of a pretty circle
	unsigned int move_tolerance; // For drawing of IR inaccuracies
	bool border_crossed; // Will be true if the IR pointer 'moved' (i.e., no longer valid as a point)

	CalibrationData() :
	    active_point(0),
	    active_light_up(true),
	    ir_pos(INVALID_IR_POS, 0),
	    ir_on_mouse_down(INVALID_IR_POS, 0),
	    waited(0),
	    move_tolerance(1), // NOTE: Thread problems, to avoid division-by-zero
	    border_crossed(false)
	{ }
    } m_cal_data;
};


#endif /* __GUI_H__ */
