/*
**	hershey
**
**	Load and display a "Hershey" vector font from the ".JHF"
**	file specified on the command line.
**
**  To compile for CP/M be sure to #define CPM:
**
**		c -qCPM=1 hershey
**
**	Glenn Roberts 31 March 2023
*/
#include "ha83.h"

/* ASCII characters */
#include "ascii.h"

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

/* prnstring - print an ASCII string on the screen.
** starting at specified row and column. rows
** are numbered 0..23, columns 0..31. 0,0 is the
** upper left corner. if the string would be off
** the screen it is clipped.
*/
prnstring(s, r, c)
char *s;
int r, c;
{
	while ((*s != 0) && 
				(r >= 0) && (r < 24) &&
				(c >= 0) && (c < 32)) {
		blockwrite(alpha[*s], 8*(r*32 + c), 8);
		c++;
		s++;
	}
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
			x += xorg;
			y += yorg;
			if (penup) {
				move(x, y);
				penup = FALSE;
			}
			else {
				draw(x, y);
			}
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

/* getint - returns a positive int via file handle fh,
** reading a fixed field width of n. 
** returns -1 on error.
*/
getint(fh, width)
int fh, width;
{
	int i, v;
	char c;
	
	v = 0;
	for (i=0; i<width; i++) {
		if ((c = getnext(fh)) == -1) {
			v = -1;
			break;
		}
		else if (isdigit(c))
			v = v*10 + c-'0';
	}
	return v;
}

/* getnext - get next character via file handle fh
** (skip over CR and LF)
*/
getnext(fh)
int fh;
{
	char c;
	
	do {
		c = getc(fh);
		if ((c != '\n') && (c != '\r'))
			break;
	} while (1);
	
	return c;
}

/* loadfont - read font data from file 'fname' and
** load the table
*/
loadfont(fname, fp)
char *fname;
struct font *fp;
{
	int i, j, n, rc, xmin, xmax, ymin, ymax, px, py, fh;
	/* dynamically allocated array of points */
	struct point *p;

	rc = 0;
	/* Hershey font files are ASCII so open as normal file */
	if ((fh=fopen(fname, "r")) == 0)
		rc = -1;
	else {
		for (i=0; i<96; i++) {
			fp->glyph = getint(fh, 5);
			n = getint(fh, 3);
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
					px = getnext(fh) - 'R';
					py = getnext(fh) - 'R';
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
		fclose(fh);
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


main(argc, argv)
int argc;
char *argv[];
{
	int i, x0, y0, c;
	struct font *fp;
	static char filename[20];
	
	/* initialize for graphics 2 mode plotting */
	initg2mode(c_black, 0, 0, c_black, c_dkgrn);
	bordercolor(c_black);

	fp = ftab;
	
	if (argc > 1) {
		if (loadfont(argv[1], fp) != 0) {
			puts("Error loading font file %s\n", argv[1]);
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
		/* now remove any drive specifier prefix
		** strip off file name extension and print
		** the result in the lower right corner
		** of the screen.
		*/
		if ((c=index(argv[1], ":")) != -1)
			strcpy(filename, argv[1]+c+1);
		else
			strcpy(filename, argv[1]);
		if ((c=index(filename, ".")) != -1)
			filename[c] = 0;
		
		prnstring(filename, 23, 24);
		wait(5);
	}
}
