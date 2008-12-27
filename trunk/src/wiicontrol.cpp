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


#include "wiicontrol.h"


void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state)                                                                                         
{
    if (cwiid_command(wiimote, CWIID_CMD_LED, led_state)) {
	DEBUG_MSG(4, "Error setting LEDs.\n");
    }
}
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
    if (cwiid_command(wiimote, CWIID_CMD_RPT_MODE, rpt_mode)) {
	DEBUG_MSG(4, "Error setting report mode.\n");
    }
}

cwiid_wiimote_t* wii_connect(char *mac)
{
    /* Remember this when things get serious */
    /* cwiid_set_err(err); */

    bdaddr_t bdaddr; /* bluetooth device address */
    if (!mac)
	bdaddr = *BDADDR_ANY;
    else
	str2ba(mac, &bdaddr);

    /* Connect to the wiimote */
    cwiid_wiimote_t* wiimote = 0;
    if ( (wiimote = cwiid_connect(&bdaddr, 0)) ) {
	DEBUG_MSG(1, "Connected to a Wiimote\n");

	set_led_state(wiimote, WIIMOTE_LED_CONNECTED); /* Notifies user */
	set_rpt_mode(wiimote, CWIID_RPT_IR | CWIID_RPT_BTN);

	return wiimote;
    }
    else DEBUG_MSG(1, "Unable to connect to wiimote\n");

    return 0;
}
int wii_disconnect(cwiid_wiimote_t* wiimote)
{
    /* set_led_state(wiimote,0); */
    return cwiid_disconnect(wiimote);
}


/* Sends button and mouse events to process()
 * Returns -1 on error
 * ir_pos.x = INVALID_IR_POS when there's no valid IR event (considers it an IR_OFF)
 * NOTE: I assume that mesg.btn_mesg.buttons will NEVER be INVALID_BUTTON_MSG_ID
 * These INVALIDs are defined in common.h */
int process_messages(cwiid_mesg const& mesg, point_t* ir_pos, uint16_t* buttons)
{
    DEBUG_MSG(4, "Got Wiimote events\n");

    int ret = 0;

    switch (mesg.type) {
	case CWIID_MESG_BTN:
	    if (buttons) {
		DEBUG_MSG(4, "Button Report: %.4X\n", mesg.btn_mesg.buttons);
		*buttons = mesg.btn_mesg.buttons;
	    }
	    break;
	case CWIID_MESG_IR:
	    /* NOTE: We ONLY track the first point for now.
	     * Because process() currently only handles 1 point.
	     * Returns the IR point which is closest to the old IR. */
	    if (ir_pos) {
		// Get valid IR events
		std::vector<point_t> irs;
		for (unsigned int i = 0; i != CWIID_IR_SRC_COUNT; ++i) {
		    cwiid_ir_src const& current = mesg.ir_mesg.src[i];
		    if (current.valid)
			irs.push_back( point_t(current.pos[CWIID_X], current.pos[CWIID_Y] ) );
		}
		// Find the closest new IR
		point_t const ir_old = *ir_pos;
		ir_pos->x = INVALID_IR_POS; // No valid IR events if it
					    // isn't set till the end of the loop (meaning irs.size() == 0)
		unsigned int closest_distance = static_cast<unsigned int>(-1); // Max uint value, actually sqr(closest)
		for (unsigned int i = 0; i != irs.size(); ++i) {
		    unsigned int const distance = squared_distance(irs[i], ir_old);
		    if ( closest_distance > distance ) {
			closest_distance = distance;
			*ir_pos = irs[i]; // We can also use a 'closest' index here and update later
		    }
		}
	    }
	    break;
	case CWIID_MESG_ERROR:
	    ret = -1; // Doesn't do anything for now
	    break;
	default:
	    break;
    }

    return ret;
}
