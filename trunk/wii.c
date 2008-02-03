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

#include <stdlib.h>
#include <stdio.h>

#include "wii.h"

void infrared_data (point_t ir_pos_new);
void button_pressed ();
void update_cursor ();

/* Prototypes */
void cwiid_callback (cwiid_wiimote_t * wiimote, int mesg_count,
					 union cwiid_mesg mesg[]);

void set_led_state (cwiid_wiimote_t * wiimote, unsigned char led_state)
{
	if (cwiid_command (wiimote, CWIID_CMD_LED, led_state))
	{
		fprintf (stderr, "Error setting LEDs \n");
	}
}
void set_rpt_mode (cwiid_wiimote_t * wiimote, unsigned char rpt_mode)
{
	if (cwiid_command (wiimote, CWIID_CMD_RPT_MODE, rpt_mode))
	{
		fprintf (stderr, "Error setting report mode\n");
	}
}

cwiid_wiimote_t *wii_connect (char *mac)
{
	/* Remember this when things get serious */
	/* cwiid_set_err(err); */

	bdaddr_t bdaddr;			/* bluetooth device address */
	if (mac[0] == '#')
		bdaddr = *BDADDR_ANY;
	else
		str2ba (mac, &bdaddr);

	/* Connect to the wiimote */
	cwiid_wiimote_t *wiimote = 0;
	printf ("Put Wiimote in discoverable mode now (press 1+2)...\n");
	if ((wiimote = cwiid_connect (&bdaddr, 0)))
	{
		printf ("Connected!!!\n");
		if (!cwiid_set_mesg_callback (wiimote, &cwiid_callback))
		{
			set_led_state (wiimote, 1);	/* Notifies the user */
			set_rpt_mode (wiimote, CWIID_RPT_IR | CWIID_RPT_BTN);
			cwiid_enable (wiimote, CWIID_FLAG_MESG_IFC);

			return wiimote;
		}
		else
			fprintf (stderr, "Unable to set message callback\n");
	}
	else
		fprintf (stderr, "Unable to connect to wiimote\n");

	return 0;
}

int wii_disconnect (cwiid_wiimote_t * wiimote)
{
	/* set_led_state(wiimote,0); */
	return cwiid_disconnect (wiimote);
}

/* Prototype cwiid_callback with cwiid_callback_t, define it with the actual
 * type - this will cause a compile error (rather than some undefined bizarre
 * behavior) if cwiid_callback_t changes */
/* cwiid_mesg_callback_t has undergone a few changes lately, hopefully this
 * will be the last.  Some programs need to know which messages were received
 * simultaneously (e.g. for correlating accelerometer and IR data), and the
 * sequence number mechanism used previously proved cumbersome, so we just
 * pass an array of messages, all of which were received at the same time.
 * The id is to distinguish between multiple wiimotes using the same callback.
 * */
void cwiid_callback (cwiid_wiimote_t * wiimote, int mesg_count,
					 union cwiid_mesg mesg[])
{
	static point_t ir_pos_new = { 0, 0 };
	static int i, j;
	for (i = 0; i != mesg_count; ++i)
	{
		switch (mesg[i].type)
		{
		case CWIID_MESG_BTN:
			printf ("Button Report: %.4X\n", mesg[i].btn_mesg.buttons);
			button_pressed ();
			break;
		case CWIID_MESG_IR:
			/* NOTE: We ONLY track the first point for now.
			 * Because infrared_data() currently only handles 1 point.
			 * As soon as we find the first valid point, stop. */
			for (j = 0; j != CWIID_IR_SRC_COUNT; ++j)
				if (mesg[i].ir_mesg.src[j].valid)
				{
					ir_pos_new.x = mesg[i].ir_mesg.src[j].pos[CWIID_X];
					ir_pos_new.y = mesg[i].ir_mesg.src[j].pos[CWIID_Y];

					infrared_data (ir_pos_new);
					break;
				}
			break;
		case CWIID_MESG_ERROR:
			if (cwiid_disconnect (wiimote))
			{
				fprintf (stderr, "Error on wiimote disconnect\n");
				exit (-1);
			}
			exit (0);
		default:
			break;
		}
	}
	update_cursor ();
}
