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


#include "irfilter.h"


bool IrFilter::process_tolerance(point_t const& pos_new) {
    bool ret = false;

    delta_t_t const delta_t = get_delta_t(m_last_time);
    if (pos_new.x == INVALID_IR_POS) {
	m_disappearing += delta_t;
	if (m_disappearing < DISAPPEARANCE_TOLERANCE)
	    ret = true; // Lets it pass this time
	// NOTE: No need to clear m_disappearing here
    }

    return ret;
}
point_t IrFilter::process_ir(point_t const& pos_new, unsigned int& move_tolerance) {
    bool const mouse_is_down = (m_pos_current.x != INVALID_IR_POS) && (pos_new.x != INVALID_IR_POS);
    if (mouse_is_down) {
	// Idea stolen from 'ujs': http://www.wiimoteproject.com/index.php?action=profile;u=1240
	m_old_positions.push_back(pos_new);
	if (m_old_positions.size() > MAX_NUMBER_OF_POSITIONS)
	    m_old_positions.pop_front();
	move_tolerance = MAX_NUMBER_OF_POSITIONS / m_old_positions.size() + 1; // Ask me if it's not clear

	point_t pos_average;
	for (std::list<point_t>::const_iterator iter = m_old_positions.begin(); iter != m_old_positions.end(); ++iter) {
	    pos_average.x += iter->x;
	    pos_average.y += iter->y;
	}
	pos_average.x /= m_old_positions.size();
	pos_average.y /= m_old_positions.size();

	return pos_average;
    }
    else {
	reset();
	return pos_new; // There's no point in calculating an invalid IR
    }
}
point_t IrFilter::process(point_t const& pos_new, unsigned int& move_tolerance) {
    point_t ret;

    if ( process_tolerance(pos_new) )
	ret = process_ir(m_pos_current, move_tolerance);
    else ret = process_ir(pos_new, move_tolerance);

    return ret;
}
