/*
**	hershey
**
**	Displays 96 "Hershey" vector font characters
**	as a 6 x 16 table on the screen
**
**	Glenn Roberts 31 December 2022
*/
#include "ha83.h"
#include "printf.h"

#define	TRUE	1
#define	FALSE	0

struct font {
  int glyph;
  int nstrokes;
	int width;
  char *strokes;
} ftab[96];

/* drawglyph - draw a hershey glyph on the screen
** xorg is the center of the bounding box, yorg is
** the upper coordinate of the bounding box.
*/
drawglyph(gp, xorg, yorg)
struct font *gp;
int xorg, yorg;
{
	int penup, n, xmin, xmax, ymin, ymax, x, y;
	char *s;
	
	penup = TRUE;
	n = gp->nstrokes;
	if (n > 0) {
		s = gp->strokes;
		xmin = *s++ - 'R';
		xmax = *s++ - 'R';
		gp->width = xmax - xmin;
		xmin = xmax = ymin = ymax = 0;
		while (--n > 0) {
			x = *s++ - 'R';
			y = *s++ - 'R';
			if ((x == -50) && (y == 0))
				penup = TRUE;
			else {
				/* update bounding box */
				if (x < xmin)
					xmin = x;
				if (x > xmax)
					xmax = x;
				if (y < ymin)
					ymin = y;
				if (y > ymax)
					ymax = y;
				
				/* now do the draw or move */
				if (penup) {
					move(xorg + x, yorg + y);
					penup = FALSE;
				}
				else
					draw(xorg + x, yorg + y);
			}
		}
		gp->width = xmax - xmin;
		printf("w= %d\n", gp->width);
	}
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

/* loadfont - read font data from stdin and load the table */
loadfont(fp)
struct font *fp;
{
	int i, j, n, rc;
	char *s;
	
	rc = 0;
	for (i=0; i<96; i++) {
		fp->glyph = getint(5);
		n = getint(3);
		if ((s=alloc(2*n + 1)) == -1) {
			rc = -1;
			break;
		} else {
			/* allocation was successful, read in and
			** store the stroke data.
			*/
			fp->nstrokes = n;
			fp->strokes = s;
			for (j=0; j<2*n; j++)
				*s++ = getnext();
			*s = '\0';
		}
		/* point to next entry in table */
		fp++;
	}
	return rc;
}


main()
{
	int i, j, x0, y0;
	struct font *fp;
	char ch;
	
	/* initialize for graphics 2 mode plotting */
	initg2mode(c_black, 0, 0, c_black, c_dkgrn);
	bordercolor(c_black);

	fp = ftab;
	ch = ' ';
	
	if (loadfont(fp) == -1) {
/*		printf("Error loading font\n"); */
	} else {
		/* display all glyphs as a table of 6 rows
		** and 16 columns.
		*/
		for (y0=32,i=0; i<6; i++) {
			/* 16 columns */
			for (x0=8,j=0; j<8; j++) {
				drawglyph(fp, x0+j*16, y0+i*16);
				x0 += fp->width;
				fp++;
				ch++;
			}
			/* next row */
			y0 += 16;
		}
	}
}
