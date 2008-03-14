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


#ifndef  __PHYSICS_H__
#define  __PHYSICS_H__


#include "common.h"
#include "auxiliary.h"


typedef Point<double> point_t_phys;

// Intead of directly returns the IR pointer location,
// this class considers the difference between old and new
// IR locations a force acting on the actual pointer. Hence
// making it move in a smooth manner.
class PhysicsEngine {
public:
    // Needs constant access to current IR location
    PhysicsEngine(point_t const& pos_current);

    // Resets all of its physics data, used when MOUSE_DOWN
    void reset() {
	// Initializes the timer
	m_last_time = 0;
	get_delta_t(m_last_time);

	m_velocity_current = point_t_phys();
    }

    // Gets a new IR location, brings it through the engine
    // and returns the desired new IR location.
    point_t process(point_t const& pos_new);
private:
    point_t const& m_pos_current;
    delta_t_t m_last_time; // Hooke's law
    point_t_phys m_velocity_current;
    // NOTE: All of the below should be heurisically initialized rather than hard-coded
    double const m_spring_constant;
    double const m_mass;
};


#endif /* __PHYSICS_H__ */
