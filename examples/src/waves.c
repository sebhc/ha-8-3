/*
**	waves
**
**	Adapted from the program WAVES.PAS in "Graphics Support 
**	Routines for V3.8	Lucidata Pascal", Copyright (C) 1982 Polybytes
**	(public released 1988).
**
**	This program plots and re-plots a sin wave across the	screen
**	in Graphics 2 mode, changing colors and doing	a little area
**	filling and clearing along the way.
**
**	Demonstrates use of the TMS9918A VDP "Graphics 2" mode, areafill(),
**	and areaclear() routines. 
**
**	Tailored for Software Toolworks C/80 3.1 with Mathpak
**	Use M80 for assembler. Required libraries: MATHLIB.REL, FLIBRARY.REL
**	CLIBRARY.REL and HA83.REL 
**
**	Recommended link command:
**
**	L80 WAVES,HA83,MATHLIB/S,FLIBRARY/S,CLIBRARY,WAVES/N/E/M
**
**	Adapted by Glenn Roberts 1 November 2022
*/
#include "ha83.h"

#define	PI	3.14159

int i, j, color0, color1;
int sinewave[256];

float sin();

/* puts() is much simpler than printf() */

/* output a string */
puts(s)
char *s;
{
	while (*s)
		putchar(*s++);
}

/* succ - return next color in sequence, skipping
** gray, white and transparent.
*/
int succ(c)
int c;
{
	int nxtcolor;
	
	if ((nxtcolor = ++c) == c_gray)
		nxtcolor = c_black;
	
	return nxtcolor;
}

main()
{
	puts("Please standby while I make waves.\n");
	for (i=0; i<256; i++)
		sinewave[i] = 32.5 - (32.0 * sin(PI*i/25.0));
	color0 = c_black;
	color1 = succ(color0);
	initg2mode(c_dkblue, 0, 0, color0, color1);
	
	/* loop 100 times drawing the sinewave, filling and
	** clearing, crawling down the screen as we go.
	*/
	for (j=0; j<100; j++ ) {
		move(0, j+32);
		for (i=0; i<256; i++)
			draw(i, j+sinewave[i]);
		color0 = color1;
		color1 = succ(color1);
		colorset(0, 255, 0, 191, color0, color1);
		areafill(100, 150, 75, 125);
		areaclear(100, 150, 75, 125);
	}
}
