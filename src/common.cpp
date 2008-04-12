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


#include "common.h"


void DEBUG_MSG(unsigned int level, char const* s, ...) {
#ifdef DEBUG_LVL
    unsigned int const current_debug_level = DEBUG_LVL;
    if (current_debug_level >= level) {
	std::string const padding(level, '-');
	printf("%sDEBUG %d: ", padding.c_str(), level);

	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
    }
#endif // DEBUG_LVL
}
