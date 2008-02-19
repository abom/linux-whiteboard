/* Copyright (C) 2008 Pere Negre
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


#include <memory> // auto_ptr


#include "gui.h"
#include "wiicursor.h"


int main(int argc,char *argv[])
{
    if (argc < 3)
    {
	/* Size */
	point_t const scr_size = screen_size();
	printf("Screen dimentions: %dx%d\n", scr_size.x, scr_size.y);
	/* MAC address - We must make a copy since wii_connect may change it
	 * '#' means any address and MAC addresses are exactly 17 characters */
	char mac[18] = {'#'};
	if (argc == 2)
	    strncpy(mac, argv[1], sizeof(mac));
	printf("MAC address: %s\n", mac);

	cwiid_wiimote_t* wiimote = wii_connect(mac);
	if (wiimote) {
	    bool transform_matrix_correct = false;
	    matrix_t transform(3, 3); // NOTE: We shouldn't have to know about its dimensions :-s

	    if ( !load_config(transform) ) {
		point_t p_wii[4];
		if ( !get_calibration_points(wiimote, p_wii) ) {
		    print_points(p_wii);
		    transform = calculate_transformation_matrix(p_wii);
		    save_config(transform);
		    transform_matrix_correct = true;
		}
		else fprintf(stderr, "User escaped or there was an error while calibrating the Wii.\n");
	    }
	    else transform_matrix_correct = true;

	    if (transform_matrix_correct) {
		std::auto_ptr<WiiCursor> const wc ( new WiiCursor(wiimote, transform) );
		wc->process(); // The main loop
	    }

	    if (!wii_disconnect(wiimote))
		return 0;
	    else fprintf(stderr, "Error on wiimote disconnect\n");
	}
	else fprintf(stderr, "Error on wiimote connect\n");
    }
    else printf("ERROR - Usage: demo [MAC-address]\n");

    return -1;
}
