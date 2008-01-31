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


#include "wii.h"


void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state)                                                                                         
{
    if (cwiid_command(wiimote, CWIID_CMD_LED, led_state)) {
	fprintf(stderr, "Error setting LEDs \n");
    }
}
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
    if (cwiid_command(wiimote, CWIID_CMD_RPT_MODE, rpt_mode)) {
	fprintf(stderr, "Error setting report mode\n");
    }
}

cwiid_wiimote_t* wii_connect(char *mac)
{
    /* Remember this when things get serious */
    /* cwiid_set_err(err); */

    bdaddr_t bdaddr; /* bluetooth device address */
    if (mac[0] == '#')
	bdaddr = *BDADDR_ANY;
    else
	str2ba(mac, &bdaddr);

    /* Connect to the wiimote */
    cwiid_wiimote_t* wiimote = 0;
    printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
    if ( (wiimote = cwiid_connect(&bdaddr, 0)) ) {
	printf("Connected!!!\n");

	set_led_state(wiimote, 1); /* Notifies the user */
	set_rpt_mode(wiimote, CWIID_RPT_IR | CWIID_RPT_BTN);
	cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC | CWIID_FLAG_NONBLOCK);

	return wiimote;
    }
    else fprintf(stderr, "Unable to connect to wiimote\n");

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
    int ret = 0;

    switch (mesg.type) {
	case CWIID_MESG_BTN:
	    if (buttons) {
		printf("Button Report: %.4X\n", mesg.btn_mesg.buttons);
		*buttons = mesg.btn_mesg.buttons;
	    }
	    break;
	case CWIID_MESG_IR:
	    /* NOTE: We ONLY track the first point for now.
	     * Because process() currently only handles 1 point.
	     * As soon as we find the first valid point, stop. */
	    if (ir_pos) {
		ir_pos->x = INVALID_IR_POS; // No valid IR events if it
					    // isn't set till the end of the loop
		for (int i = 0; i != CWIID_IR_SRC_COUNT; ++i)
		    if (mesg.ir_mesg.src[i].valid) {
			ir_pos->x = mesg.ir_mesg.src[i].pos[CWIID_X];
			ir_pos->y = mesg.ir_mesg.src[i].pos[CWIID_Y];
			break;
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
