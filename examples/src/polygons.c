/*
**	polygons
**
**	Adapted from the program PICTURE.PAS in "Graphics Support 
**	Routines for V3.8	Lucidata Pascal", Copyright (C) 1982 Polybytes
**	(public released 1988).
**
**	This program prompts the user for a number n (3 <= n <= 50)
**	and draws a regular polygon with n sides and then efficiently
**	connects the vertices of the polygon.
**
**	Demonstrates use of the TMS9918A "Graphics 2" mode point and line drawing.
**
**	Tailored for Software Toolworks C/80 3.1 with Mathpak
**	Use M80 for assembler. Required libraries: MATHLIB.REL, FLIBRARY.REL
**	CLIBRARY.REL and HA83.REL 
**
**	Recommended link command:
**
**	L80 POLYGONS,HA83,MATHLIB/S,FLIBRARY/S,CLIBRARY,POLYGONS/N/E/M
**
**	Adapted by Glenn Roberts 28 October 2022
*/
#include "ha83.h"

#define	PI	3.14159
#define	BEL	007
#define	NL	012

#define	TRUE	1
#define	FALSE	0

#define	NMIN	3
#define	NMAX	50

int barray[NMAX], carray[NMAX], harray[NMAX];
int i, n, n1, j, j1, j2, f, erasing, docomp;

float wpi, cos(), sin();

/* these utility routines are more efficient than scanf/printf */

/* atoi - convert string to integer */
atoi(s)
char *s;
{
	static int n, sign;
	sign = 1;
	n = 0;
	switch (*s) {
		case '-': sign = -1;
		case '+': ++s;
		}
	while (*s >= '0' && *s <= '9') n = 10 * n + *s++ - '0';
	return(sign * n);
}

/* input a string */
gets(s)
char *s;
{
	char c;
	
	while ((c=getchar()) != NL)
		*s++ = c;
	
	/* terminate the string */
	*s = '\0';
}

getval()
{
	static char lvalue[20];

	gets(lvalue);
	return atoi(lvalue);
}

/* output a string */
puts(s)
char *s;
{
	while (*s)
		putchar(*s++);
}


main()
{
	erasing = FALSE;

	do {
		/* toggle between drawing and erasing. */
		if (!erasing) {
			/* request the number of sides. must be in valid
			** range [NMIN .. NMAX] before moving on
			*/
			do {
				puts("Enter the number of sides for the polygon.\n");
				puts("Make it negative to use complement mode lines.");
				puts(" (in [3..50]) >");
				if ((n = getval()) < 0) {
					docomp = TRUE;
					n = -n;
				}
				else
					docomp = FALSE;
				/* exit when we have a valid n */
				if ((n >= NMIN) && (n <= NMAX))
					break;
				putchar(BEL);
			} while (TRUE);
				
			/* have valid n */
			wpi = 2.0 * PI/n;
			for (i=0; i<n; i++) {
				barray[i] = 95.0 * (1.0 + cos(i*wpi)) + 32.5;
				carray[i] = 95.0 * (1.0 + sin(i*wpi)) + 0.5;
			}
				
			/* initialize for graphics 2 mode plotting
			** and set complementary mode based on 'comp'
			*/
			initg2mode(c_dkblue, 0, 0, c_ltyell, c_dkgrn);
			compmode(docomp);
		}

		/* now draw the polygon and connect vertices */
		for (i=1; i<=n/2; i++) {
			for (j=0; j<n; j++)
				harray[j] = FALSE;
			if (i*2 == n) {
				for (j=i; j<n; j++)
					harray[j] = TRUE;
			}
			j2 = 0;
			f = TRUE;
			n1 = (i*2 == n) ? i : n;
			for (j=0; j<n1; j++) {
				j1 = j2;
				if (harray[j1]) {
					f = TRUE;
					while (harray[j1]) {
						if (++j1 == n)
							j1 = 0;
					}
				}	
				if ((j2 = j1+i) >= n)
					j2 -= n;
				harray[j1] = TRUE;
				if (f)
					move(barray[j1],carray[j1]);
				f = FALSE;
				draw(barray[j2],carray[j2]);
			}
		}
		if (!erasing) {
			puts("Press RETURN to erase the picture, or Ctrl-C to exit.\n");
			getchar();
		}
		erasing = !erasing;
		if (!docomp) 
			eraser(erasing);
	} while (TRUE);
}
