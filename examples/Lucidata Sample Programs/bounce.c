/*
**	bounce
**
**	Adapted from the program BOUNCE.PAS in "Graphics Support 
**	Routines for V3.8	Lucidata Pascal", Copyright (C) 1982 Polybytes
**	(public released 1988).
**
**	This program is an adaptation of the tiny pascal program that appeared
**	in the Heath Users Group REMark magazine issue #17. It displays a bouncing
**	ball in a repeating animation loop.
**
**	Demonstrates use of the TMS9918A "Multicolor" mode overlaid with an 8x8 pixel
**	moving sprite.
**
**	Tailored for Software Toolworks C/80 3.1
**	Use M80 for assembler. Required libraries: CLIBRARY.REL and HA83.REL 
**
**	Recommended link command:
**
**	L80 BOUNCE,HA83,CLIBRARY,BOUNCE/N/E/M
**
**	Adapted by Glenn Roberts 30 October 2022
*/
#include "ha83.h"

/*	sprite pattern for bouncing ball:
**
**	..XXXX..
**	.XXXXXX.
**	XXXXXXXX
**	XXXXXXXX
**	XXXXXXXX
**	.XXXXXX.
**	..XXXX..
*/
char ball[] = {0x3c,0x7e,0xff,0xff,0xff,0xff,0x7e,0x3c};

/* drawbox - draws three edges of a box to contain the
** bouncing ball
*/
drawbox()
{
	mcfill(10,11,1,47,c_dkblue);
	mcfill(53,54,1,47,c_dkblue);
	mcfill(12,52,46,47,c_dkblue);
}

/* throwball - performs the bouncing ball animation */
throwball()
{
	int vx,vy,x,y,i;
	
	x = 50;
	y = 10;
	vx = 15;
	vy = 0;
	while ((vx != 0) || (vy != 0) || (y < 173)) {
		/* first update positions and velocities */
		if (y < 175)
			vy = vy + 2;
		
		x = x + vx;
		y = y + vy;
		
		if (x > 208) {
			x = 416 - x;
			vx = -vx/4;
		}
		if (x < 48) {
			x = 96 - x;
			vx = -vx/4;
		}
		if (y > 176) {
			y = 352 - y;
			vy = -vy/2;
		}
		
		/* now reposition the ball */
		positsprite(0,x,y);
		
		/* slow the show down a bit... (the Lucidata Pascal
		** version runs with an interpreter but this version
		** is native code so it is faster.
		*/
		for (i=1; i<600; i++)
			;
	}
}

main()
{
	int i, mag, ssize;

	mag = 0;
	ssize = 0;
	
	/* initialize multicolor mode, create the sprite, draw the
	** bounding box, then loop forever bouncing the ball
	*/
	initmcmode(c_gray,mag,ssize);
	defsprite(0,0,0,c_midred,0,ball);
	drawbox();
	do {
		throwball();
		/* delay, then repeat... */
		for (i=1; i<10000; i++)
			;
	} while(1);
}
