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


#include "events.h"


void fake_move(int x, int y)
{
    Display* display = XOpenDisplay(0);
    XTestFakeMotionEvent(display, -1, x, y, 0);
    //printf("MOUSE MOVE: %d %d\n", x, y);
    XCloseDisplay(display);
}
void fake_button(int button, bool pressed)
{
    Display* display = XOpenDisplay(0);
    XTestFakeButtonEvent(display, button, pressed, 0);
    printf("MOUSE %d's state changed to %d\n", button, pressed);
    XCloseDisplay(display);
}
