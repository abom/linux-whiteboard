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


#include <cmath>
#include <list>

#include "common.h"
#include "auxiliary.h"


// Filters out 'bad' IR signals
class IrFilter {
public:
    // Needs constant access to current IR location
    IrFilter(point_t const& pos_current) :
	m_pos_current(pos_current)
    {
	reset();
    }

    void reset() {
	m_old_positions.clear();

	// Resets the timer
	m_last_time = 0;
	get_delta_t(m_last_time);

	m_disappearing = 0;
    }

    // Gets a new IR location, brings it through the engine
    // and returns the desired new IR location.
    point_t process(point_t const& pos_new);
private:
    // Used by process()
    bool process_tolerance(point_t const& pos_new); // Returns true if tolerating
    point_t process_ir(point_t const& pos_new);

    point_t const& m_pos_current;
    std::list<point_t> m_old_positions;
    delta_t_t m_last_time;
    delta_t_t m_disappearing; // How long the signal has been off
    // NOTE: All these below should be heuristically guessed instead
    static unsigned int const MAX_NUMBER_OF_POSITIONS = 7;
    static delta_t_t const DISAPPEARANCE_TOLERANCE = 50; // In milliseconds
};


#endif /* __PHYSICS_H__ */
