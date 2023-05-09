/*
**	hershey
**
**	Reads and displays "Hershey" vector font characters
**
**	Usage:
**
**	hershey <fontfile
**
**
**	Glenn Roberts 18 December 2022
*/
struct pair {
	int x;
	int y;
};

struct pair parray[200];

#include "ha83.h"

#define	TRUE	1
#define	FALSE	0

#define SCALE	2

#define	X0	127
#define	Y0	95

/* HDOS TICCNT location */
#define TICCNT	0x201B

unsigned *Ticptr = TICCNT;
unsigned timeout;

static char digits[10][8] = {
	{0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70, 0x00},
	{0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00},
	{0x70, 0x88, 0x08, 0x30, 0x40, 0x80, 0xF8, 0x00},
	{0xF8, 0x08, 0x10, 0x30, 0x08, 0x88, 0x70, 0x00},
	{0x10, 0x30, 0x50, 0x90, 0xF8, 0x10, 0x10, 0x00},
	{0xF8, 0x80, 0xF0, 0x08, 0x08, 0x88, 0x70, 0x00},
	{0x38, 0x40, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00},
	{0xF8, 0x08, 0x10, 0x20, 0x40, 0x40, 0x40, 0x00},
	{0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70, 0x00},
	{0x70, 0x88, 0x88, 0x78, 0x08, 0x10, 0xE0, 0x00}
};

/* wait - wait a specified number of seconds */
wait(seconds)
unsigned seconds;
{
	mswait(seconds*1000);
}

/* mswait - wait a specified number of milliseconds */
mswait(milliseconds)
unsigned milliseconds;
{
	/* 2 ms per tick */
	timeout = *Ticptr + (milliseconds >> 1);
	while (*Ticptr != timeout)
		/* do nothing, just wait... */;
}

/* getint - returns a positive int from stdin reading
** a fixed field width of n. 
** returns -1 on error.
*/
getint(width)
int width;
{
	int i, v;
	char c;
	
	v = 0;
	for (i=0; i<width; i++) {
		if ((c = getnext()) == -1) {
			v = -1;
			break;
		}
		else if (isdigit(c))
			v = v*10 + c-'0';
	}
	return v;
}

/* getpair - read an x,y pair into the provided struture.
** each pair is two characters offset by 0x52 ('R').
** no error handling.
*/
getpair(p)
struct pair *p;
{
	p->x = getnext() - 'R';
	p->y = getnext() - 'R';
}

/* getnext - get next character (skip over CR and LF)
*/
getnext()
{
	char c;
	
	do {
		c = getchar();
		if ((c != '\n') && (c != '\r'))
			break;
	} while (1);
	
	return c;
}

/* drawglyph - draw a hershey glyph on the screen */
drawglyph(p, n)
struct pair *p;
int n;
{
	int i, penup;
	
	penup = TRUE;
	for (i=1; i<n; i++) {
		if ((p[i].x == -50) && (p[i].y == 0))
			penup = TRUE;
		else if (penup) {
			move(X0 + SCALE * p[i].x, Y0 + SCALE * p[i].y);
			penup = FALSE;
		}
		else
			draw(X0 + SCALE * p[i].x, Y0 + SCALE * p[i].y);
	}
}

main()
{
	int i, xg, glyph, npairs;
	
	/* initialize for graphics 2 mode plotting */
	initg2mode(c_black, 0, 0, c_black, c_dkgrn);
	bordercolor(c_black);
	compmode(TRUE);

	do {
		if ((glyph = getint(5)) == -1)
			break;
		npairs = getint(3);
	
	
		for (i=0; i<npairs; i++)
			getpair(&parray[i]);

		/* display 4-digit glyph number in upper left corner */
		for (xg=24; xg>=0; xg-=8) {
			blockwrite(digits+glyph%10, xg, 8);
			glyph /= 10;
		}
		drawglyph(parray, npairs);
			
		mswait(300);
		drawglyph(parray, npairs);
		
	} while (1);
}
