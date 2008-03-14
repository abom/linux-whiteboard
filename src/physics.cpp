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


#include "physics.h"


PhysicsEngine::PhysicsEngine(point_t const& pos_current) :
    m_pos_current(pos_current),
    m_spring_constant(1.0),
    m_mass(1.0)
{
    reset();
}

point_t PhysicsEngine::process(point_t const& pos_new) {
    bool const data_is_valid = (m_pos_current.x != INVALID_IR_POS) && (pos_new.x != INVALID_IR_POS);
    if (data_is_valid) {
	double const delta_t = get_delta_t(m_last_time)/1000.0; // In seconds

	// Force caused by the movement
	point_t_phys const displacement(pos_new.x-m_pos_current.x, pos_new.y-m_pos_current.y);
	point_t_phys const total_force(m_spring_constant*displacement.x, m_spring_constant*displacement.y);

	point_t_phys const acceleration(total_force.x/m_mass, total_force.y/m_mass);
	m_velocity_current.x += acceleration.x*delta_t;
	m_velocity_current.y += acceleration.y*delta_t;

	point_t const new_pos(m_pos_current.x+m_velocity_current.x, m_pos_current.y+m_velocity_current.y);
	return new_pos;
    }
    else {
	reset();
	return pos_new; // There's no point in calculating an invalid IR
    }
}
