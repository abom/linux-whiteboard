/* Copyright (C) 2007 L. Donnie Smith <cwiid@abstrakraft.org>
 * 
 *  (modified by Pere Negre)
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


#ifndef  __WII_H__
#define  __WII_H__


#include <cwiid.h>

#include "auxiliary.h"
#include "common.h"


void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);

cwiid_wiimote_t* wii_connect(char *mac);
int wii_disconnect(cwiid_wiimote_t* wiimote);
// User may or may not care about IR and/or button events, hence the pointers
// WARNING: ir_pos MUST be preset to the old IR location, I used this
// as an optimization: It used to have ir_pos and ir_old.
int process_messages(cwiid_mesg const& mesg, point_t* ir_pos, uint16_t* buttons);


#endif /* __WII_H__ */
