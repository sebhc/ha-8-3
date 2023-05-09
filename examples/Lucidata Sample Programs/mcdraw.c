/*
**	mcdraw
**
**	Adapted from the program VECTORS.PAS in "Graphics Support 
**	Routines for V3.8	Lucidata Pascal", Copyright (C) 1982 Polybytes
**	(public released 1988).
**
**	This program draws a series of concentric rectangles of varying
**	colors.
**
**	Demonstrates use of the TMS9918A "Multi-color" mode point and line drawing.
**
**	Tailored for Software Toolworks C/80 3.1
**	Use M80 for assembler. Required libraries: CLIBRARY.REL and HA83.REL 
**
**	Recommended link command:
**
**	L80 MCDRAW,HA83,CLIBRARY,MCDRAW/N/E/M
**
**	Adapted by Glenn Roberts 31 October 2022
*/
#include "ha83.h"

#define MAXY		24
#define	MAXX		32

int color,xborder,yborder,mag,ssize;

/* increment color, skipping over transparent */
changecolor()
{
	color = (color + 1) % 16;
	if (color == c_transp)
		++color;
}


main()
{
	color = c_black;
	mag = 0;
	ssize = 0;
	
  initmcmode(c_dkblue,mag,ssize);
	
	/* Loop forever */
  do {
		xborder = 0;
		yborder = 0;
		do {
			mcmove(xborder,yborder,color);
			mcdraw(63-xborder,yborder,color);
			mcdraw(63-xborder,47-yborder,color);
			mcdraw(xborder,47-yborder,color);
			mcdraw(xborder,yborder,color);
			changecolor();
			++yborder;
			++xborder;
		} while (yborder<MAXY);
		do {
			mcmove(xborder,23,color);
			mcdraw(63-xborder,23,color);
			mcmove(xborder,24,color);
			mcdraw(63-xborder,24,color);
			changecolor();
			++xborder;
		} while (xborder<MAXX);
		xborder = 22;
		yborder = 22;
		do {
			mcmove(xborder,yborder,color);
			mcdraw(63-xborder,yborder,color);
			mcdraw(63-xborder,47-yborder,color);
			mcdraw(xborder,47-yborder,color);
			mcdraw(xborder,yborder,color);
			changecolor();
			--yborder;
			--xborder;
		} while (yborder >= 0);
  } while (1);
}
