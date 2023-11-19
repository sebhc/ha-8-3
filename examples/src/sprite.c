/*
**	sprite
**
**	Adapted from the program SPRITE.PAS in "Graphics Support 
**	Routines for V3.8	Lucidata Pascal", Copyright (C) 1982 Polybytes
**	(public released 1988).
**
**	This program generates a sprite and	moves it around on the screen
**	in a circular motion. The "early bit" is set alternately on each
**	pass, causing the icon to disappear off the left border when the
** 	bit is set. The transcendental sin() function is used to generate
**	points in a circle so there is a pause at the beginning of execution
**	while these computations are made.
**
**	Demonstrates use of the TMS9918A sprite creation, movement and use
**	of the early bit
**
**	Tailored for Software Toolworks C/80 3.1 with Mathpak
**	Use M80 for assembler. Required libraries: MATHLIB.REL, FLIBRARY.REL
**	CLIBRARY.REL and HA83.REL 
**
**	To compile for CP/M be sure to define "CPM":
**
**	c -qCPM=1 sprite
**
**	Recommended link command:
**
**	L80 SPRITE,HA83,MATHLIB/S,FLIBRARY/S,CLIBRARY,SPRITE/N/E/M
**
**	Adapted by Glenn Roberts 31 October 2022
**	updated 8 November 2023 (CP/M support)
*/
#include "ha83.h"

#define	PI		3.141592654

#define	TRUE	1
#define	FALSE	0

/* Smiley sprite pattern
**
**	..XXXX..
**	.X....X.
**	X.X..X.X
**	X......X
**	XX....XX
**	X.XXXX.X
**	.X....X.
**	..XXXX..
*/
char smiley[] = { 0x3C,0x42,0xA5,0x81,0xC3,0xBD,0x42,0x3C };

int sintable[360];
int	x,y,color,theta,flag,mag,ssize;
float degtorad, sin();

/* millisecond timer utility to pace the speed */

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

/* use puts() for simple text I/O (avoids printf) */
puts(s)
char *s;
{
	while (*s)
		putchar(*s++);
}

main()
{
	mag = 1;
	ssize = 0;
	flag = TRUE;
	degtorad = PI/180.0;
	color = c_midgrn;
	
	puts("360 moments while I set up a sin table...\n");
	for (theta=0; theta<360; theta++) {
		sintable[theta] = 80.0*sin(degtorad*theta) + 0.5;
	}

	initmcmode(c_black, mag, ssize);
	defsprite(0, 120, 90, c_white, 0, smiley);
	
	do {
		/* move sprite through 360 degree circle */
	  for (theta=0; theta<360; theta++) {
			y = sintable[theta] + 90;
			x = sintable[(theta+90) % 360] + 92;
			positsprite(0,x,y);
			/* wait 10ms to slow things down for easier viewing */
			mswait(10);
#ifdef	CPM
			CtlCk();
#endif
	  }
		
		/* change sprite color (skip transparent and black) */
		if (color==c_white)
			color = c_midgrn;
		else
			++color;
		chgsprcolor(0,color);
		
		/* now repeat with different earlybit setting. setting
		** earlybit shifts sprite left 32 pixels allowing it
		** to smoothly move in and out of the left boundary
		*/
		flag = !flag;
		if (flag)
			wrtearlybit(0,0);
		else
			wrtearlybit(0,1);
	} while(TRUE);
}
