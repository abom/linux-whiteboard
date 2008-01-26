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


#include "matrix.h"
#include "wii.h"
#include "auxiliary.h"
#include "common.h"

#include <unistd.h>

/* Current infrared pointer location */
point_t ir_pos = {0, 0};
/* 4 calibrated points on the Wiimote */
point_t p_wii[4];
/* Remember to free it at the end of main()
 * Ideally cwiid_set_mesg_callback should allow passing along a data structure */
matrix_t *transform;

int ready = 0, can_exit = 0;
int event_has_occurred = 0;
/* '#' means any address, it will be processed by wii_connect() later */
/* MAC addresses are exactly 17 characters (00:11:22:33:44:55) */
char mac[18] = {'#'};

/* Prototypes */
void update_cursor();

void button_pressed()
{
    can_exit = 1;
}


void infrared_data(point_t ir_pos_new)
{
    if (ready)
    {
    	//printf("%d ------- %d \n", ir_pos_new.x, ir_pos_new.y);
	ir_pos.x = (matrixGetElement(transform,0,0)*ir_pos_new.x + matrixGetElement(transform,0,1)*ir_pos_new.y + matrixGetElement(transform,0,2)) /
	    (matrixGetElement(transform,0,6)*ir_pos_new.x + matrixGetElement(transform,0,7)*ir_pos_new.y + 1);
	ir_pos.y = (matrixGetElement(transform,0,3)*ir_pos_new.x + matrixGetElement(transform,0,4)*ir_pos_new.y + matrixGetElement(transform,0,5)) /
	    (matrixGetElement(transform,0,6)*ir_pos_new.x + matrixGetElement(transform,0,7)*ir_pos_new.y + 1);

    	//printf("%d ------- %d \n", ir_pos.x, ir_pos.y);
	event_has_occurred = 1;

	point_t const scr_size = screen_size();
	if (ir_pos.x<0)       ir_pos.x = 0; 
	if (ir_pos.x>=scr_size.x)  ir_pos.x = scr_size.x-1;
	if (ir_pos.y<0)       ir_pos.y = 0;
	if (ir_pos.y>=scr_size.y)  ir_pos.y = scr_size.y-1;
    }
    else ir_pos = ir_pos_new;
}


void read_param(int argc, char *argv[])
{
    /* Size */
    point_t const scr_size = screen_size();
    printf("Screen dimentions: %dx%d\n", scr_size.x, scr_size.y);

    /* MAC address */
    if (argc > 1)
	strncpy(mac, argv[1], sizeof(mac));
    printf("MAC address: %s\n", mac);
}


void update_cursor()
{
    static int lastevent = 0;
    static int delta = 0; /* WARNING: delta should be equal to t at first,
			   * but we may get away with it now. */
    static int semaphore = 0;
    int t;

    if (semaphore == 1)
    	return;
   
    t = get_ticks();
    if (event_has_occurred)
    { 
	event_has_occurred=0;
	fake_move(ir_pos.x,ir_pos.y); 
	//printf("%d %d\n", ir_pos.x, ir_pos.y);
	if (lastevent == 0) { fake_button(1, 1); }
	lastevent = 1;
	delta = t; 
    }
    else if ( (lastevent==1) && ((get_ticks() - delta)>50)) {
	fake_button(1, 0); 
	lastevent = 0; 
    }
}


int main(int argc,char *argv[])
{
    if (argc > 2)
    {
	printf("ERROR: \n       Usage demo <mac> \n");
	return 0;
    }
    else read_param(argc,argv);

    cwiid_wiimote_t* wiimote = wii_connect(mac);
    if (!wiimote)
	return -1;

    if (!get_calibration_points()) {
	print_points();

	printf("Calculating coefficients...");
	transform = calculate_transformation_matrix();
	printf("Done!\n");

	matrixPrint(transform);

	ready = 1;
	/* Start the timer by updating the last ticks*/
	get_ticks();
	while (!can_exit)
		usleep(100);
	
	matrixFree(transform);
    }

    int const return_val = wii_disconnect(wiimote);
    if (return_val)
	fprintf(stderr, "Error on wiimote disconnect\n");

    return return_val;
}
