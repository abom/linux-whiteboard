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

#include "common.h"
#include "auxiliary.h"


typedef Point<double> point_t_phys;


// Imagine you're pulling the cursor with a string
point_t string_physics(point_t const& pos_current, point_t const& pos_new, unsigned int string_length);


// Filters out 'bad' IR signals
class IrFilter {
public:
    // Needs constant access to current IR location
    IrFilter(point_t const& pos_current) :
	m_pos_current(pos_current),
	m_move_tolerance(0)
    { }

    unsigned int& move_tolerance() { return m_move_tolerance; }

    // Gets a new IR location, brings it through the engine
    // and returns the desired new IR location.
    point_t process(point_t const& pos_new);
private:
    point_t const& m_pos_current;
    // NOTE: All these below should be heuristically guessed instead
    unsigned int m_move_tolerance;
};


#endif /* __PHYSICS_H__ */
