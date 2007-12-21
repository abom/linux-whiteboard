#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "meschach/matrix.h"

#include <X11/extensions/XTest.h>

//#include <regex.h>


extern int wii_connect(char *mac);
extern void wii_disconnect();

int rx=0, ry=0;

int SIZEX;
int SIZEY;

typedef struct {
	int x,y;
} point_t;

point_t p_screen[4];
point_t p_wii[4];

SDL_Surface *s;

float h11,h12,h13,h21,h22,h23,h31,h32;
int ready=0, can_exit = 0;

int event_has_occurred = 0;

char mac[100];


void buttonpress()
{
	can_exit = 1;
}


int infrared_data(int *v)
{
/*	printf (
		"(%d %d)\n"
		"(%d %d)\n"
		"(%d %d)\n"
		"(%d %d)\n", v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7]);*/
	/* rx = SIZEX - v[0];
	ry = SIZEY - v[1]; */

	if (ready)
	{
		rx =  (int) ( ( (float)  (h11*v[0] + h12*v[1] + h13) ) / ( (float) (h31*v[0] + h32*v[1] + 1) ) );
		ry =  (int) ( ( (float)  (h21*v[0] + h22*v[1] + h23) ) / ( (float) (h31*v[0] + h32*v[1] + 1) ) );
		//printf ("-------------(%d %d) ------------\n",rx,ry);
		event_has_occurred = 1;
	}
	else
	{
		rx = v[0]; ry = v[1];
	}
	
	if (rx<0)       rx = 0; 
	if (rx>=SIZEX)  rx = SIZEX-1;
	if (ry<0)       ry = 0;
	if (ry>=SIZEY)  ry = SIZEY-1;
}


void read_parameters()
{
	char buf[255];
	int sx,sy,bp;
	FILE *f = fopen("config.cfg","r");
	fscanf(f,"sizex = %d\n",&sx);
	fscanf(f,"sizey = %d\n",&sy);
	fscanf(f,"bpp = %d\n",&bp);
	fscanf(f,"mac = %s\n",&mac);
	fclose(f);
	
	SIZEX = sx;
	SIZEY = sy;
	
	printf("sizex = %d\n",SIZEX);
	printf("sizey = %d\n",SIZEY);
	printf("bpp = %d\n",bp);
	printf("mac = %s\n",mac);
}





void pixel(int x, int y)
{
	Uint32 *m;
	y *= s->w;
	m = (Uint32*) s->pixels + y + x;
	*m = SDL_MapRGB(s->format,255,255,255);
}




void draw_point(point_t *p)
{
	int i;
	for (i=p->x-10; i<p->x+10; i++)
		pixel(i,p->y);
	
	for (i=p->y-10; i<p->y+10; i++)
		pixel(p->x,i);
}



void printpoints()
{
	int i;
	for (i=0; i<4; i++)
		printf("Point %d --> (%d,%d) === (%d,%d)\n", 
			i,
			p_screen[i].x,
			p_screen[i].y,
			p_wii[i].x,
			p_wii[i].y);
}



void do_calcs()
{
	int i;

	MAT *H, *LU;
	VEC *x, *b;
	PERM *pivot;

	H = m_get(8,8);
	x = v_get(8);
	b = v_get(8);

	b->ve[0] = p_screen[0].x;
	b->ve[1] = p_screen[0].y;
	b->ve[2] = p_screen[1].x;
	b->ve[3] = p_screen[1].y;
	b->ve[4] = p_screen[2].x;
	b->ve[5] = p_screen[2].y;
	b->ve[6] = p_screen[3].x;
	b->ve[7] = p_screen[3].y;

	for (i=0; i<4; i++)
	{
		H->me[i*2][0] = p_wii[i].x;
		H->me[i*2][1] = p_wii[i].y;
		H->me[i*2][2] = 1;
		H->me[i*2][3] = 0;
		H->me[i*2][4] = 0;
		H->me[i*2][5] = 0;
		H->me[i*2][6] = -p_screen[i].x * p_wii[i].x;
		H->me[i*2][7] = -p_screen[i].x * p_wii[i].y;

		H->me[i*2+1][0] = 0;
		H->me[i*2+1][1] = 0;
		H->me[i*2+1][2] = 0;
		H->me[i*2+1][3] = p_wii[i].x;
		H->me[i*2+1][4] = p_wii[i].y;
		H->me[i*2+1][5] = 1;
		H->me[i*2+1][6] = -p_screen[i].y * p_wii[i].x;
		H->me[i*2+1][7] = -p_screen[i].y * p_wii[i].y;
	}

	LU = m_get(H->m, H->n);
	LU = m_copy(H,LU);
	pivot = px_get(H->m);
	LUfactor(LU,pivot);
	x = LUsolve(LU,pivot,b,VNULL);

	h11 = x->ve[0];
	h12 = x->ve[1];
	h13 = x->ve[2];
	h21 = x->ve[3];
	h22 = x->ve[4];
	h23 = x->ve[5];
	h31 = x->ve[6];
	h32 = x->ve[7];

}



void movePointer(int x, int y)
{
	Display *display = XOpenDisplay(0);
	XTestFakeMotionEvent(display,-1,x,y,0);
	XCloseDisplay(display);
}

void button(int p)
{
	Display *display = XOpenDisplay(0);
        XTestFakeButtonEvent(display,1,p,0);
	XCloseDisplay(display);
	printf("BUTTON!! %d\n",p);
}




int main()
{
	SDL_Event e;
	Uint32 black_color;
	Uint8 *k;
	int state = 0;
	int ok=1;
	int delta, t;
	int lastevent = 0;	

	read_parameters();

	SDL_Init(SDL_INIT_VIDEO);
	s = SDL_SetVideoMode(SIZEX,SIZEY,0,SDL_HWSURFACE | SDL_FULLSCREEN);
	black_color = SDL_MapRGB(s->format,0,0,0);
	
	if (wii_connect(mac) == 0)
		exit(1);

	p_screen[0].x = 50;	
	p_screen[0].y = 50;

	p_screen[1].x = SIZEX - 50;
	p_screen[1].y = 50;

	p_screen[2].x = 50;
	p_screen[2].y = SIZEY - 50;

	p_screen[3].x = SIZEX - 50;
	p_screen[3].y = SIZEY - 50;

	SDL_FillRect(s,0,black_color);
	draw_point(&p_screen[0]);	
	draw_point(&p_screen[1]);	
	draw_point(&p_screen[2]);	
	draw_point(&p_screen[3]);	

	while(1)
	{
		SDL_PollEvent(&e);
		k = SDL_GetKeyState(NULL);
		if (k[SDLK_ESCAPE]) { ok=0; break; }

		if (k[SDLK_SPACE]) { state++; k[SDLK_SPACE]=0; }

		if (state < 4) { p_wii[state].x = rx; p_wii[state].y = ry; }
		
		if (state == 4) { do_calcs(); state++; ready=1; }

		if (state > 4)
			break;

		SDL_UpdateRect(s,0,0,0,0);
	}

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	delta = t = SDL_GetTicks();
	while (!can_exit)
	{
		t = SDL_GetTicks();
		if (event_has_occurred)
		{ 
			event_has_occurred=0;
			movePointer(rx,ry); 
			if (lastevent == 0) { button(1); }
			lastevent = 1;
			delta = t; 
		}
		else
		{
			if ( (lastevent==1) && ((SDL_GetTicks() - delta)>50)) 
				{ button(0); lastevent = 0; }
		}
	}
	

	wii_disconnect();
	
	printpoints();
	
	return 0;
}

