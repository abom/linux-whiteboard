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

#include "auxiliary.h"

point_t screen_size ()
{
	Display *display = XOpenDisplay (0);
	int screen = DefaultScreen (display);
	point_t scr_size =
		{ DisplayWidth (display, screen), DisplayHeight (display, screen) };
	XCloseDisplay (display);

	return scr_size;
}
static void screen_corners (point_t p_screen[4])
{
	point_t scr_size = screen_size ();
	int PADDING = 50;

	p_screen[0].x = PADDING;
	p_screen[0].y = PADDING;
	p_screen[1].x = scr_size.x - PADDING;
	p_screen[1].y = PADDING;
	p_screen[2].x = PADDING;
	p_screen[2].y = scr_size.y - PADDING;
	p_screen[3].x = scr_size.x - PADDING;
	p_screen[3].y = scr_size.y - PADDING;
}

static void pixel (SDL_Surface * surface, int x, int y)
{
	Uint32 *m = (Uint32 *) surface->pixels + y * surface->w + x;
	*m = SDL_MapRGB (surface->format, 255, 255, 255);
}
static void draw_point (SDL_Surface * surface, point_t p)
{
	int i;
	for (i = p.x - 10; i < p.x + 10; ++i)
		pixel (surface, i, p.y);

	for (i = p.y - 10; i < p.y + 10; ++i)
		pixel (surface, p.x, i);
}
static void draw_square (SDL_Surface * surface, point_t p)
{
	int i;
	for (i = p.x - 10; i < p.x + 10; ++i)
	{
		pixel (surface, i, p.y + 10);
		pixel (surface, i, p.y - 10);
	}

	for (i = p.y - 10; i < p.y + 10; ++i)
	{
		pixel (surface, p.x - 10, i);
		pixel (surface, p.x + 10, i);
	}
}

static void draw_calibration_points (SDL_Surface * surface, point_t points[4],
									 int active, int active_light_up)
{
	int i;
	for (i = 0; i != 4; ++i)
	{
		draw_point (surface, points[i]);
		if ((i < active) || ((i == active) && active_light_up))
			draw_square (surface, points[i]);
	}
}

int get_calibration_points ()
{
	point_t const scr_size = screen_size ();
	SDL_Event e;
	Uint8 *k;
	int state = 0;
	SDL_Surface *surface;
	Uint32 black_color;
	int xm1, xm2, ym1, ym2;
	int i;
	int MAX_WII_X = 1020;
	int MAX_WII_Y = 760;
	point_t p_screen[4];

	SDL_Init (SDL_INIT_VIDEO);
	surface =
		SDL_SetVideoMode (0, 0, 0,
						  SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF);
	black_color = SDL_MapRGB (surface->format, 0, 0, 0);

	SDL_FillRect (surface, 0, black_color);

	xm1 = scr_size.x / 2 - 100;
	xm2 = xm1 + 200;
	ym1 = scr_size.y / 2 - 100;
	ym2 = ym1 + 200;

	screen_corners (p_screen);
	int active_light_up = 0;
	while (1)
	{
		SDL_PollEvent (&e);
		k = SDL_GetKeyState (NULL);
		if (k[SDLK_ESCAPE])
			return -1;

		if (k[SDLK_SPACE])
		{
			state++;
			k[SDLK_SPACE] = 0;
		}

		if (state < 4)
		{
			p_wii[state].x = ir_pos.x;
			p_wii[state].y = ir_pos.y;
		}

		if (state >= 4)
			break;

		for (i = (int) xm1; i < (int) xm2; i++)
			pixel (surface, i, ym1), pixel (surface, i, ym2);

		for (i = (int) ym1; i < (int) ym2; i++)
			pixel (surface, xm1, i), pixel (surface, xm2, i);

		pixel (surface,
			   xm1 + (int) (((float) ir_pos.x / (float) MAX_WII_X) * 200),
			   ym2 - (int) (((float) ir_pos.y / (float) MAX_WII_Y) * 200));

		draw_calibration_points (surface, p_screen, state, active_light_up);
		active_light_up = !active_light_up;

		SDL_Flip (surface);
		SDL_Delay (100);
		SDL_FillRect (surface, 0, black_color);
	}

	printf ("Quitting SDL..");
	SDL_FreeSurface (surface);
	SDL_Quit ();
	printf ("Done\n");

	return 0;
}

void print_points ()
{
	int i;
	point_t p_screen[4];
	screen_corners (p_screen);

	for (i = 0; i < 4; i++)
		printf ("Point %d --> (%d,%d) === (%d,%d)\n",
				i, p_screen[i].x, p_screen[i].y, p_wii[i].x, p_wii[i].y);
}

matrix_t *calculate_transformation_matrix ()
{
	point_t p_screen[4];
	screen_corners (p_screen);
	matrix_t *m = matrixNew (8, 8), *n = matrixNew (1, 8);
	matrix_t *r;
	int i = 0;

	for (i = 0; i != 4; ++i)
	{
		matrixSetElement (n, (float) p_screen[i].x, 0, i * 2);
		matrixSetElement (n, (float) p_screen[i].y, 0, i * 2 + 1);
	}

	for (i = 0; i != 4; ++i)
	{
		matrixSetElement (m, (float) p_wii[i].x, 0, i * 2);
		matrixSetElement (m, (float) p_wii[i].y, 1, i * 2);
		matrixSetElement (m, (float) 1, 2, i * 2);
		matrixSetElement (m, (float) 0, 3, i * 2);
		matrixSetElement (m, (float) 0, 4, i * 2);
		matrixSetElement (m, (float) 0, 5, i * 2);
		matrixSetElement (m, (float) (-p_screen[i].x * p_wii[i].x), 6, i * 2);
		matrixSetElement (m, (float) (-p_screen[i].x * p_wii[i].y), 7, i * 2);

		matrixSetElement (m, (float) 0, 0, i * 2 + 1);
		matrixSetElement (m, (float) 0, 1, i * 2 + 1);
		matrixSetElement (m, (float) 0, 2, i * 2 + 1);
		matrixSetElement (m, (float) p_wii[i].x, 3, i * 2 + 1);
		matrixSetElement (m, (float) p_wii[i].y, 4, i * 2 + 1);
		matrixSetElement (m, (float) 1, 5, i * 2 + 1);
		matrixSetElement (m, (float) (-p_screen[i].y * p_wii[i].x), 6,
						  i * 2 + 1);
		matrixSetElement (m, (float) (-p_screen[i].y * p_wii[i].y), 7,
						  i * 2 + 1);
	}

	matrixInverse (m);
	r = matrixMultiply (m, n);

	matrixFree (n);
	matrixFree (m);

	return r;
}

void fake_move (int x, int y)
{
	Display *display = XOpenDisplay (0);
	XTestFakeMotionEvent (display, -1, x, y, 0);
	XCloseDisplay (display);
}
void fake_button (int button, int is_press)
{
	Display *display = XOpenDisplay (0);
	XTestFakeButtonEvent (display, button, is_press, 0);
	XCloseDisplay (display);
}

unsigned long get_ticks ()
{
	static long t1, t2;
	static struct timeval last = { 0, 0 };
	static struct timeval current = { 0, 0 };
	gettimeofday (&current, 0);
	t1 = last.tv_sec * 1000 + last.tv_usec / 1000;
	t2 = current.tv_sec * 1000 + current.tv_usec / 1000;
	return t2 - t1;
}
