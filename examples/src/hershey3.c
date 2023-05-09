/*
**	hershey
**
**	Displays 96 "Hershey" vector font characters
**	as a 6 x 16 table on the screen
**
**	Glenn Roberts 2 January 2023
*/
#include "ha83.h"

#define	TRUE	1
#define	FALSE	0

/* each glyph consists of one or more points */
struct point {
	char x;
	char y;
};

/* each font consists of 96 glyphs starting at 0x20 (space) */
struct font {
  int glyph;
  int nstrokes;
	int width;
	int height;
  struct point *strokes;
} ftab[96];

/* since i basically hate printf... */

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
	char s[10];
	
	itoa(v,s);
	puts(s);
}

/* drawglyph - draw a hershey glyph on the screen
** xorg,yorg is the upper left corner of the bounding box.
*/
drawglyph(gp, xorg, yorg)
struct font *gp;
int xorg, yorg;
{
	int i, penup, n, x, y;
	
	penup = TRUE;
	n = gp->nstrokes;
	xorg += gp->width/2 + 1;
	/* skip [0] which contains bounding box x,y */
	for (i=1; i<n; i++) {
		x = gp->strokes[i].x;
		y = gp->strokes[i].y;
		if ((x == -50) && (y == 0))
			penup = TRUE;
		else {
			/* now do the draw or move */
			if (penup) {
				move(xorg + x, yorg + y);
				penup = FALSE;
			}
			else
				draw(xorg + x, yorg + y);
		}
	}
}

/* strwidth - compute the width (in pixels) of a string where
** ip is the number of pixels to be inserted between each
** glyph
 */
strwidth(s, gp, ip)
char *s;
struct font *gp;
int ip;
{
	int w;
	char c;
	
	w = 0;
	while ((c=(*s++))) {
		w += gp[c-' '].width + 1;
	}
	
	if (w)
		--w;
	return w;
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
	int i, j, n, rc, xmin, xmax, ymin, ymax, px, py;
	struct point *p;
	
	rc = 0;
	for (i=0; i<96; i++) {
		fp->glyph = getint(5);
		n = getint(3);
		fp->nstrokes = n;
		if ((p=alloc(2*n)) == -1) {
			rc = -1;
			break;
		} else {
			/* allocation was successful, read in and
			** store the stroke data in decimal form, 
			** while keeping track of overall max width
			** and height.
			*/
			xmin = xmax = ymin = ymax = 0;
			fp->strokes = p;
			for (j=0; j<n; j++) {
				px = getnext() - 'R';
				py = getnext() - 'R';
				/* keep track of the max and min values
				** we see for x and y. skip first x,y pair
				** (overall bounding box) and skip any 
				** Pen Up instructions
				*/
				if ((j>0) && !((px == -50) && (py == 0))) {
					if (px < xmin)
						xmin = px;
					if (px > xmax)
						xmax = px;
					if (py < ymin)
						ymin = py;
					if (py > ymax)
						ymax = py;
				}
				fp->strokes[j].x = px;
				fp->strokes[j].y = py;
			}
			/* blank is not zero width or height */
			if (xmax == xmin)
				fp->width = 5;
			else
				fp->width  = xmax - xmin + 1;
			
			if (ymax == ymin)
				fp->height = 5;
			else
				fp->height = ymax - ymin + 1;
		}
		/* and keep doing this for all 96 glyphs... */
		fp++;
	}
	return rc;
}

/* showstr - display string s at specificed location x,y where
** x,y is the upper left corner of the bounding box. fp points
** to the font structure. insert ip pixels between characters.
**
** does not do bounds check so writing beyond the edge of the
** screen will cause odd results.
*/
showstr(s, fp, x, y, ip)
char *s;
struct font *fp;
int x, y, ip;
{
	char c;
	
	while (c=(*s++)) {
		drawglyph(fp+c-' ', x, y);
		x += fp[c-' '].width + ip;
	}
}


main()
{
	int i, x0, y0, spacing;
	struct font *fp;
	
	static char *line[] = {
		"Merry Christmas",
		"",
		"and",
		"",
		"Happy New Year"
	};
	
	/* initialize for graphics 2 mode plotting */
	initg2mode(c_black, 0, 0, c_black, c_dkgrn);
	bordercolor(c_black);

	fp = ftab;
	spacing = 1;
	
	if (loadfont(fp) == -1) {
		puts("Error loading font\n");
	} else {
		/* display all glyphs */
		y0 = 24;
		x0 = 0;
		for (i=0; i<96; i++) {
			if ((x0 + fp->width) > 256) {
				y0 += 24;
				x0 = 0;
			}
			drawglyph(fp, x0, y0);
			x0 += fp->width + 1;
			fp++;
		}
	}
}
