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


/*point_t string_physics(point_t const& pos_current, point_t const& pos_new, unsigned int string_length) {
    point_t_phys const displacement(pos_new.x-pos_current.x, pos_new.y-pos_current.y);
    double const distance = sqrt( static_cast<double>(squared_distance(pos_current, pos_new)) );
    point_t ret = pos_new; // By default
    if (distance > string_length) {
	point_t_phys const normalized(displacement.x/distance, displacement.y/distance);
	point_t_phys const new_displacement(normalized.x*string_length, normalized.y*string_length);
	ret.x = pos_current.x+new_displacement.x;
	ret.y = pos_current.y+new_displacement.y;
    }

    return ret;
}*/


point_t IrFilter::process(point_t pos_new) {
    delta_t_t const delta_t = get_delta_t(m_last_time);
    if (pos_new.x == INVALID_IR_POS) {
	m_disappearing += delta_t;
	if (m_disappearing < DISAPPEARANCE_TOLERANCE)
	    pos_new = m_pos_current; // Lets it pass this time
	else m_disappearing = 0;
    }

    bool const mouse_is_down = (m_pos_current.x != INVALID_IR_POS) && (pos_new.x != INVALID_IR_POS);
    if (mouse_is_down) {
	// Idea stolen from 'ujs': http://www.wiimoteproject.com/index.php?action=profile;u=1240
	m_old_positions.push_back(pos_new);
	if (m_old_positions.size() > MAX_NUMBER_OF_POSITIONS)
	    m_old_positions.pop_front();

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
