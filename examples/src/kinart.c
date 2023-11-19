/*
**	kinart
**
**	Adapted from the program KINART.PAS in "Graphics Support 
**	Routines for V3.8	Lucidata Pascal", Copyright (C) 1982 Polybytes
**	(public released 1988).
**
**	This program draws a moving vector on the screen, slightly coordinated 
**	with a set of frequencies from the sound chip. As the vector lengths
**	change the frequencies of the tones change as well.
**
**	Demonstrates use of the TMS9918A "Graphics 2" mode line drawing as well
**	as use of the AY-3-8910 programmable sound generator.
**
**	Tailored for Software Toolworks C/80 3.1
**	Use M80 for assembler. Required libraries: STDLIB.REL, CLIBRARY.REL
**	and HA83.REL 
**
**	Recommended link command:
**
**	L80 KINART,HA83,RND,STDLIB/S,CLIBRARY,KINART/N/E/M
**
**	Adapted by Glenn Roberts 31 October 2022
*/
#include "ha83.h"

#define	TRUE	1
#define	FALSE	0

#define	YMAX		191
#define	WIDTH		256
#define	HEIGHT	192
#define	MAXLINE	150

/* values for random number generator */
#define MULT		31
#define MODVAL	1009


struct vecs	{
	int	xs;
	int ys;
	int	xe;
	int	ye;
} vecstore[MAXLINE];

/* global since main() and initialize() both access */
int	seed;
int	inum;
int complement;
int	x1,y1,x2,y2;

//* millisecond timer utility to pace the speed */

/* pointer to 2-ms clock (note: HDOS and CP/M use different
** locations.
*/
#ifdef CPM

/* CP/M puts it in low RAM */
#define TICCNT  0x000B
#else
/* HDOS TICCNT = 040.033A */
#define TICCNT  0x201B

#endif

unsigned *Ticptr = TICCNT;
unsigned timeout;


/* mswait - wait a specified number of milliseconds */
mswait(milliseconds)
unsigned milliseconds;
{
	/* 2 ms per tick */
	timeout = *Ticptr + (milliseconds >> 1);
	while (*Ticptr != timeout)
		/* do nothing, just wait... */;
}

/* simple I/O routines (faster than printf) */

/* output a string */
puts(s)
char *s;
{
	while (*s)
		putchar(*s++);
}

/* random number generator */
int random()
{
	seed = (seed*MULT) % MODVAL;
	return (seed/251 - 4);
}

putval(v)
int v;
{
	char s[10];
	
	itoa(v,s);
	puts(s);
}

/* initialize all values */
initialize()
{
	int i;
	char ch;

	puts("Complement (C) or Normal (N) vectors? >");
	ch = getchar();
	putchar('\n');
	complement = ((ch=='C') || (ch=='c'));
	
	/* seed random number generator */
	rnd(12345);
	
	x1 = rnd(0)
	x2 = *p4;
	y1 = *p2 & 0x7F;
	y2 = *p1 & 0x7F;
	seed = 137;
	 
	/* initialize G2 mode graphics */
	initg2mode(c_dkblue,0,0,c_ltgrn,c_dkblue);
	compmode(complement);
	
	/* set up sound channels ... */
	tonefreq('A',0);
	tonefreq('B',0);
	tonefreq('C',0);
	chanampl('A',8);
	chanampl('B',8);
	envshape(1);
	ecycltime(2);
	envenable('A');
	envenable('B');
	envenable('C');
	psgoptions(TRUE,TRUE,TRUE,FALSE,FALSE,FALSE);
	
	/* clear vector history array */
	for (i=0; i<MAXLINE; i++) {
		vecstore[i].xs = 0;
		vecstore[i].ys = 0;
		vecstore[i].xe = 0;
		vecstore[i].ye = 0;
	}
	inum = 100;
}

vector(xo,yo,xd,yd)
int xo,yo,xd,yd;
{
	eraser(FALSE);
	move(xo,yo);
	draw(xd,yd);
}

erase(xo,yo,xd,yd)
int xo,yo,xd,yd;
{
	if (!complement)
		eraser(TRUE);
	move(xo,yo);
	draw(xd,yd);
}

main()
{
	int	dx1,dx2,dy1,dy2, count, lcount;
	
	initialize();
	count = 0;
	do {
		for (lcount=0; lcount<MAXLINE; lcount++) {
			/* erase older vector */
			erase(vecstore[lcount].xs,
				vecstore[lcount].ys,
				vecstore[lcount].xe,
				vecstore[lcount].ye);
			
			/* make some music */
			tonefreq('B',vecstore[lcount].xs+
				vecstore[lcount].ys+
				vecstore[lcount].xe+
				vecstore[lcount].ye);
			
			/* set x and y deltas as random values */
			if (count==0) {
				dx1 = random();
				dx2 = random();
				dy1 = random();
				dy2 = random();
				count = abs(random())*inum+1;
			}
			
			/* correct deltas to keep things in bounds */
			if ((x1+dx1)<0  || (x1+dx1)>=WIDTH)
				dx1 = -dx1;
			x1 += dx1;
			if ((y1+dy1)<0  ||  (y1+dy1)>=HEIGHT)
				dy1 = -dy1;
			y1 += dy1;
			if ((x2+dx2) < 0  ||  (x2+dx2)>=WIDTH)
				dx2 = -dx2;
			x2 += dx2;
			if ((y2+dy2) < 0  ||  (y2+dy2)>=HEIGHT)
				dy2 = -dy2;
			y2 += dy2;
			
			/* draw the new vector */
			vector(x1,y1,x2,y2);
			
			/* save a copy for erasing later */
			vecstore[lcount].xs = x1;
			vecstore[lcount].ys = y1;
			vecstore[lcount].xe = x2;
			vecstore[lcount].ye = y2;
			
			/* make some music */
			tonefreq('A',x1+x2+y1+y2);
			tonefreq('C',abs(x1+y1-x2-y2));
			envshape(1);
			ecycltime(2);
			mswait(50);
			count--;
		}
	} while(TRUE);
}
