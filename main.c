#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>

#include "matrix.h"

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

Display *display;

#define MAX_WII_X 1020
#define MAX_WII_Y 760


void buttonpress()
{
	can_exit = 1;
}


int infrared_data(int *v)
{
	if (ready)
	{
		rx =  (int) ( ( (float)  (h11*v[0] + h12*v[1] + h13) ) / ( (float) (h31*v[0] + h32*v[1] + 1) ) );
		ry =  (int) ( ( (float)  (h21*v[0] + h22*v[1] + h23) ) / ( (float) (h31*v[0] + h32*v[1] + 1) ) );
		//printf ("-------------(%d %d) ------------\n",rx,ry);
		event_has_occurred = 1;
		
		if (rx<0)       rx = 0; 
		if (rx>=SIZEX)  rx = SIZEX-1;
		if (ry<0)       ry = 0;
		if (ry>=SIZEY)  ry = SIZEY-1;
	}
	else
	{
		rx = v[0]; ry = v[1];
	}
	
}


void read_parameters(int argc, char *argv[])
{
	char buf[255];
	int sx,sy,bp;

//===========================================SIZE
        display = XOpenDisplay(0);
	int screen = DefaultScreen(display);	
	SIZEX = DisplayWidth(display,screen);
	SIZEY = DisplayHeight(display,screen);
	XCloseDisplay(display);

	printf("sizex = %d\n",SIZEX);
	printf("sizey = %d\n",SIZEY);

//============================================BP
	bp=32;	
	printf("bpp = %d\n",bp);

//============================================MAC
	if(argc>1){
		strcpy(mac,argv[1]);
		//str2ba(argv[1],&mac);
		printf("mac = %s\n",mac);
	}
	else{
		//mac = *BDADDR_ANY;
		mac[0]='#';
		printf("mac = ANY \n");
	}

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


void draw_square(point_t *p)
{
	int i;
	for (i=p->x-10; i<p->x+10; i++)
		pixel(i,p->y+10), pixel(i,p->y-10);

        for (i=p->y-10; i<p->y+10; i++)
                pixel(p->x-10,i), pixel(p->x+10,i);

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

	vector_t v;
	matrix_t m,r;

	newMatrix(&m,8,8);
	newVector(&v,8);

	setVectorElement(&v, (float) p_screen[0].x, 0);
	setVectorElement(&v, (float) p_screen[0].y, 1);
	setVectorElement(&v, (float) p_screen[1].x, 2);
	setVectorElement(&v, (float) p_screen[1].y, 3);
	setVectorElement(&v, (float) p_screen[2].x, 4);
	setVectorElement(&v, (float) p_screen[2].y, 5);
	setVectorElement(&v, (float) p_screen[3].x, 6);
	setVectorElement(&v, (float) p_screen[3].y, 7);

	for (i=0; i<4; i++)
	{
		setMatrixElement(&m, (float) p_wii[i].x, 0, i*2);
		setMatrixElement(&m, (float) p_wii[i].y, 1, i*2);
		setMatrixElement(&m, (float) 1, 2, i*2);
		setMatrixElement(&m, (float) 0, 3, i*2);
		setMatrixElement(&m, (float) 0, 4, i*2);
		setMatrixElement(&m, (float) 0, 5, i*2);
		setMatrixElement(&m, (float) (-p_screen[i].x * p_wii[i].x), 6, i*2);
		setMatrixElement(&m, (float) (-p_screen[i].x * p_wii[i].y), 7, i*2);

		setMatrixElement(&m, (float) 0, 0, i*2+1);
		setMatrixElement(&m, (float) 0, 1, i*2+1);
		setMatrixElement(&m, (float) 0, 2, i*2+1);
		setMatrixElement(&m, (float) p_wii[i].x, 3, i*2+1);
		setMatrixElement(&m, (float) p_wii[i].y, 4, i*2+1);
		setMatrixElement(&m, (float) 1, 5, i*2+1);
		setMatrixElement(&m, (float) (-p_screen[i].y * p_wii[i].x), 6, i*2+1);
		setMatrixElement(&m, (float) (-p_screen[i].y * p_wii[i].y), 7, i*2+1);
	}

	matrixInverse(&m);
	matrixDOTvector(&m,&v,&r);

	h11 = getMatrixElement(&r,0,0);
	h12 = getMatrixElement(&r,0,1);
	h13 = getMatrixElement(&r,0,2);
	h21 = getMatrixElement(&r,0,3);
	h22 = getMatrixElement(&r,0,4);
	h23 = getMatrixElement(&r,0,5);
	h31 = getMatrixElement(&r,0,6);
	h32 = getMatrixElement(&r,0,7);

	freeMatrix(&m);
	freeMatrix(&r);
	freeVector(&v);

}








void movePointer(int x, int y)
{
        display = XOpenDisplay(0);
	XTestFakeMotionEvent(display,-1,x,y,0);
	XCloseDisplay(display);
}

void button(int p)
{
        display = XOpenDisplay(0);
        XTestFakeButtonEvent(display,1,p,0);
	//printf("BUTTON!! %d\n",p);
	XCloseDisplay(display);
}




int main(int argc,char *argv[])
{
	if(argc>2){
		printf("ERROR: \n       Usage demo <mac> \n");
		return 0;
	}
	SDL_Event e;
	Uint32 black_color;
	Uint8 *k;
	int state = 0;
	int ok=1;
	int delta, t;
	int lastevent = 0;	
	int i;
	float xm1,ym1,xm2,ym2;

	read_parameters(argc,argv);
	
	if (wii_connect(mac) == 0)
		exit(1);
	
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER );
	s = SDL_SetVideoMode(SIZEX,SIZEY,0,SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF);
	black_color = SDL_MapRGB(s->format,0,0,0);

	p_screen[0].x = 50;	
	p_screen[0].y = 50;

	p_screen[1].x = SIZEX - 50;
	p_screen[1].y = 50;

	p_screen[2].x = 50;
	p_screen[2].y = SIZEY - 50;

	p_screen[3].x = SIZEX - 50;
	p_screen[3].y = SIZEY - 50;

	SDL_FillRect(s,0,black_color);

	xm1 = SIZEX / 2 - 100;
	xm2 = xm1 + 200;
	ym1 = SIZEY / 2 - 100;
	ym2 = ym1 + 200;

	t = 0;
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

		for (i = (int) xm1; i < (int) xm2; i++)
			pixel(i,ym1), pixel(i,ym2);

		for (i = (int) ym1; i < (int) ym2; i++)
			pixel(xm1,i), pixel(xm2,i);

		pixel( 
			xm1 + (int) ( ((float) rx / (float) MAX_WII_X )*200),
			ym2 - (int) ( ((float) ry / (float) MAX_WII_Y )*200)
		);

		draw_point(&p_screen[0]);	
		draw_point(&p_screen[1]);	
		draw_point(&p_screen[2]);	
		draw_point(&p_screen[3]);	

		for(i=0; i<state; i++)
			draw_square(&p_screen[i]);

		if ((state<4) && (t)) 
			draw_square(&p_screen[state]);

		t = ~ t; 

		//SDL_UpdateRect(s,0,0,0,0);
		SDL_Flip(s);
		SDL_Delay(100);
		SDL_FillRect(s,0,black_color);
	}

	printf("Quitting SDL..");
	SDL_Quit();	
	printf("Done\n");

	printpoints();

	SDL_Init(SDL_INIT_TIMER);

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
	//SDL_FreeSurface(s);
	SDL_Quit();
	
	return 0;
}

