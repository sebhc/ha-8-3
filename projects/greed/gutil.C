/* gutil - GREED Utility functions
**
** these are broken out here to simplify and streamlilne
** the software build process
*/

#include "gutil.h"

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

/* itoarj - convert n to characters in s and right justify.
** l is the length of the string not including the terminating
** NUL. l must be at least 1.
*/
char *itoarj(n, s, l)
char s[];
int n, l;
{
	static int k;
	static char *p;

	if ((k = n) < 0)
		k = -k;
	p = s + l;
	/* first insert terminating NUL */
	*p-- = '\0';
	do {
		*p-- = k % 10 + '0';
	} while ((k /= 10) && (p >= s));
	if ((n < 0) && (p >= s))
		*p-- = '-';
	/* now pad with blank if needed */
	while (p >= s)
		*p-- = ' ';

	return (s);
}

/* the following routines are for console I/O and
** by default use OS-dependent routines (though
** they could be replaced with ROM-based
** equivalents)
*/
#ifndef OSFREE
/* output a string */
puts(s)
char *s;
{
	while (*s)
		putchar(*s++);
}

putval(v)
int v;
{
	static char s[10];
	
	itoarj(v,s,5);
	puts(s);
}
#endif

showval(v, n, r, c, e)
int v, n, r, c, e;
{
	static char s[10];
	
	itoarj(v, s, n);
	showstr(s, r, c, e);
}

/* showstr - display a string at a specified
** row and column. string is clipped to stay
** in the displayable space. if e is TRUE then
** show text using emphasized color.
*/
showstr(s,r,c,e)
char *s;
int r,c,e;
{
	static int row, col, offset;
	static char ch;
	
	row = r;
	col = c;

	while ((ch=(*s++)) != '\0') {
		/* eliminate non-printable characters */
		if ((ch>=' ') && (ch<='\177')) {
			/* clip to legal screen coordinates */
			if ((row<24) && (col<32)) {
				/* have valid char and location */
				offset = ABASE + ch - ' ';
				screen[row][col] = e ? (offset + AOFFSET) : offset;
				++col;
			}
		}
	}
}

/* jsidle - returns TRUE if the joystick
** buttons are idle (not pressed) and FALSE
** if a key press has been detected.
*/
jsidle(c)
char c;
{
	return (rdpsgport(c) == 0xFF);
}


/* rjs - read joystick */
rjs(c)
char c;
{
	int v, done;
	
	for (done=FALSE; !done; done = (v == rdpsgport(c))) {
		while ((v=rdpsgport(c)) == 0xFF)
			; /* wait for activity */

		/* now wait debounce time... */
		mswait(30);
	}

	while ((rdpsgport(c)) != 0xFF)
		; /* wait for button release */
	
	/* set bits to 1 for button selects */
	return (~v & 0xFF);
}

#ifndef OSFREE
/* dumpdie -- (debugging) dump out a die data structure */
dumpdie(d)
int d;
{
	puts("Die [");
	putval(dice[d].value);
	puts("]: ");
	putval(dice[d].row);
	putchar(' ');
	putval(dice[d].column);
	putchar(' ');
	putval(dice[d].color);
	putchar(' ');
	putval(dice[d].gridx);
	putchar(' ');
	putval(dice[d].gridy);
	putchar(' ');
	putval(dice[d].status);
	putchar('\n');
}
#endif

/* rndrange - return random integer in a range from
** min to max (inclusive).
*/
rndrange(min, max)
int min, max;
{
	return min + rnd(0)%(max-min+1);
}


/* clrdice - erase the entire dice holding area */
clrdice()
{
	fillbox(0, 8, 8, 31, NULLPAT);
}

/* clractive - erase the ACTIVE dice holding area */
clractive()
{
	fillbox(5, 8, 8, 31, NULLPAT);
}

/* cls - clear screen */
cls()
{
	int row, col;

	/* fill screen with the null pattern */
	for (row=0; row<24; row++)
		for (col=0; col<32; col++)
			screen[row][col] = NULLPAT;
}

/* refresh - repaint the whole screen from RAM to VRAM */
refresh()
{
	blockwrite(screen,PNTAB,768);
}


/* clrrow - clear a row */
clrrow(r)
int r;
{
	int col;
	
	for (col=0; col<32; col++)
		screen[r][col] = NULLPAT;
}

/* fillbox - clear an area */
fillbox(r0, c0, r1, c1, patt)
int r0, c0, r1, c1, patt;
{
	int row, col;
	
	for (row=r0; row<=r1; row++)
		for (col=c0; col<=c1; col++)
			screen[row][col] = patt & 0xFF;
}

/* allheld - return TRUE if all dice are in HELD state
*/
allheld()
{
	int j, result;
	
	result = TRUE;
	for (j=0; j<NDICE; j++) {
		if (dice[j].status != HELD) {
			result = FALSE;
			break;
		}
	}
		
	return result;
}



/* allactive -- reset all dice to active mode */
allactive()
{
	int j;
	
	for (j=0; j<NDICE; j++)
			dice[j].status = ACTIVE;
}
/* yloc -- return true screen row (pixel
** coordinates) for the specified grid y 
*/
yloc(gy)
int gy;
{
	return DYMIN + gy*32;
}

/* xloc -- return true screen column (pixel
** coordinates) for the specified grid x
*/
xloc(gx)
int gx;
{
	return DXMIN + gx*32;
}

/* nrow -- return the number of sprites
** in a specified grid row. This is used to ensure
** that we never have more than four in a
** row, as the VDP can only display up to
** 4 sprites in a row.
*/
nrow(iy)
int iy;
{
	int ix, n;
	
	for (ix=0, n=0; ix<NGX; ix++) {
		if (grid[iy][ix] != EMPTY)
			++n;
	}

	return n;
}
                                                                                                                                                                                                                                              