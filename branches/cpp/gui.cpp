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


#include "gui.h"


point_t screen_size() {
    Display* display = XOpenDisplay(0);
    int const screen = DefaultScreen(display);
    point_t const scr_size( DisplayWidth(display, screen), DisplayHeight(display, screen) );
    XCloseDisplay(display);

    return scr_size;
}
void screen_corners(point_t p_screen[4]) {
    point_t const scr_size = screen_size();
    int const PADDING = 50;

    // By a user's request
    p_screen[0] = point_t(PADDING, PADDING);
    p_screen[1] = point_t(scr_size.x - PADDING, PADDING);
    p_screen[3] = point_t(PADDING, scr_size.y - PADDING);
    p_screen[2] = point_t(scr_size.x - PADDING, scr_size.y - PADDING);
}


void draw_point(SDL_Surface* surface, point_t const& p, unsigned int radius) {
    sge_Line(surface, p.x, p.y-radius, p.x, p.y+radius, SDL_COLOR_WHITE);
    sge_Line(surface, p.x-radius, p.y, p.x+radius, p.y, SDL_COLOR_WHITE);
}
void draw_rectangle(SDL_Surface* surface, point_t const& p, point_t dim, Uint32 color) {
    sge_Rect(surface, p.x-dim.x, p.y-dim.y, p.x+dim.x, p.y+dim.y, color);
}
void draw_square(SDL_Surface* surface, point_t const& p, unsigned int radius, Uint32 color) {
    draw_rectangle(surface, p, point_t(radius, radius), color);
}
void draw_calibration_points(SDL_Surface* surface, point_t const points[4], int active, int active_light_up) {
    unsigned int const RADIUS = 10;
    for (int i = 0; i != sizeof(points); ++i) {
	draw_point(surface, points[i], RADIUS);
	if ( (i < active) || ((i == active) && active_light_up) )
	    draw_square(surface, points[i], RADIUS, SDL_COLOR_WHITE);
    }
}
// NOTE: Needs MUCH improvement, it's too long and monolithic
int get_calibration_points(cwiid_wiimote_t* wiimote, point_t p_wii[4]) {
    point_t const scr_size = screen_size();

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface* surface = SDL_SetVideoMode(0, 0, 0, SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF);
    Uint32 const black_color = SDL_MapRGB(surface->format,0,0,0);

    SDL_FillRect(surface,0,black_color);

    // Sets up the timer
    delta_t_t last_time = 0;
    get_delta_t(last_time);

    // These are taken from wiicursor.cpp, improvements are welcome
    delta_t_t waited = 0;
    unsigned int const MOVE_TOLERANCE = 25;
    unsigned int const WAIT_TOLERANCE = 700;
    bool mouse_down = false;
    point_t ir_on_mouse_down;
    // ...
    int active_light_up  = 0;
    point_t ir_pos(INVALID_IR_POS, 0);
    int active_point = 0;
    while (active_point != 4) {
	// IR events
	{   
	    int msg_count = 0;
	    union cwiid_mesg* msgs = 0;
	    // NOTE: See wiicursor.cpp's NOTE about this
	    timespec tspec;
	    while (!cwiid_get_mesg(wiimote, &msg_count, &msgs, &tspec))
		for (int i = 0; i != msg_count; ++i) {
		    point_t const ir_old = ir_pos;
		    process_messages(msgs[i], &ir_pos, 0);
		    if ( (ir_old.x == INVALID_IR_POS) && (ir_pos.x != INVALID_IR_POS) ) {
			ir_on_mouse_down = ir_pos;
			mouse_down = true;
			get_delta_t(last_time); // 'Sort of' resets the timer
			printf("MOUSE DOWN\n.");
		    }
		    if ( (ir_old.x != INVALID_IR_POS) && (ir_pos.x == INVALID_IR_POS) ) {
			mouse_down = false; // NOTE: Repetitive :<
			waited = 0; // Written in 3 different places, what if it was in C++?
			printf("MOUSE UP\n.");
		    }
		}
	}

	// Timer stuff
	if (mouse_down) {
	    if (waited > WAIT_TOLERANCE) {
		p_wii[active_point] = ir_pos;
		++active_point;
		mouse_down = false;
		waited = 0;
		printf("POINT %d calibrated.\n", active_point);
	    }
	    else {
		waited += get_delta_t(last_time);
		printf("Mouse has been down for: %lld milliseconds.\n", waited);
	    }

	    if ( squared_distance(ir_pos, ir_on_mouse_down) > sqr(MOVE_TOLERANCE) ) {
		mouse_down = false;
		waited = 0;
		printf("MOUSE MOVED by sqrt(%d) pixels :) .\n", squared_distance(ir_pos, ir_on_mouse_down));
	    }
	}

	// SDL stuff
	SDL_FillRect(surface,0,black_color);
	SDL_PollEvent(0);

	Uint8 const*const k = SDL_GetKeyState(0);
	if (k[SDLK_ESCAPE])
	    return -1;

	// Interface stuff
	active_light_up = !active_light_up;
	unsigned int const WII_VIEWING_AREA_RADIUS = 100;
	point_t const screen_center( scr_size.x/2, scr_size.y/2 );
	draw_square(surface, screen_center, WII_VIEWING_AREA_RADIUS, SDL_COLOR_WHITE); // Wiimote's viewing area

	if (ir_pos.x != INVALID_IR_POS) { // There's no point in drawing an invalid IR pointer
	    point_t const MAX_WII(1020, 760);
	    point_t const ir_pointer_center(
		    (screen_center.x-WII_VIEWING_AREA_RADIUS) + static_cast<int>( ((float) ir_pos.x / (float) MAX_WII.x)*(WII_VIEWING_AREA_RADIUS*2)),
		    (screen_center.y+WII_VIEWING_AREA_RADIUS) - static_cast<int>( ((float) ir_pos.y / (float) MAX_WII.y)*(WII_VIEWING_AREA_RADIUS*2)) );
	    draw_square(surface, ir_pointer_center, 1, SDL_COLOR_WHITE); // Current IR pointer's location, from the Wiimote's eye
	}

	point_t p_screen[4];
	screen_corners(p_screen);
	draw_calibration_points(surface, p_screen, active_point, active_light_up); // 4 calibration points

	/* Draws bounding boxes around the screen's edges, for easier viewing
	 * NOTE: The values of screen_center = screen_dimensions */
	unsigned int const BOX_THICK = 5;
	unsigned int colors[3] = {SDL_COLOR_RED, SDL_COLOR_YELLOW, SDL_COLOR_GREEN};
	for (unsigned int i = 0; i != 3; ++i)
	    for (unsigned int j = 0; j != BOX_THICK; ++j)
		draw_rectangle( surface, screen_center, point_t(screen_center.x-i*j, screen_center.y-i*j), colors[i] );

	// SDL again
	SDL_Flip(surface);
	SDL_Delay(100);
    }

    printf("Quitting SDL..");
    SDL_Quit();
    printf("Done\n");

    return 0;
}


void print_points(point_t const p_wii[4]) {
    point_t p_screen[4];
    screen_corners(p_screen);

    int i;
    for (i=0; i<4; i++)
	printf("Point %d --> (%d,%d) === (%d,%d)\n",
		i, p_screen[i].x, p_screen[i].y, p_wii[i].x, p_wii[i].y);
}
