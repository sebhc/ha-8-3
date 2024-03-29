/*
**	txchar
**
**	Fill the screen with the ASCII character set
**
**	ASCII character set is from Texas Instruments TMS9918A Data
**	Manual.
**
**	Demonstrates use of the TMS9918A "Text" mode to display characters.
**
**	Tailored for Software Toolworks C/80 3.1
**	Use M80 for assembler. Required libraries: CLIBRARY.REL and HA83.REL 
**
**	Recommended link command:
**
**	L80 TXCHAR,HA83,STDLIB/S,CLIBRARY,TXCHAR/N/E/M
**
**	Adapted by Glenn Roberts 5 November 2022
*/
#include "ha83.h"
#include "ascii.h"

#define	NROWS	24
#define	NCOLS	40
#define	EOF	-1
#define	NL	012

/* VRAM MAP
**
**		Mode: Text
**
**		0..2047	(2K) Pattern Generator Table (PGTAB)
**		6144..6911 (960 bytes) Pattern Name Table (PNTAB)
**
*/
#define	PGTAB	0
#define	PNTAB	6144

int row, col;
char screen[NROWS][NCOLS];

/* output a string */
puts(s)
char *s;
{
	while (*s)
		putchar(*s++);
}

inittxmode(color0, color1)
unsigned color0, color1;
{
	/* Initialize VDP registers:
	**
	**	No External Video
	**	16K RAM
	**	Disable display
	**	Text mode
	*/
	vdpoptions(VPNEV+VP16K+VPDDP+VPTX);

	/* set up table addresses */
	pattgentable(PGTAB);
	pattnametable(PNTAB);
	
	/* load ASCII patterns into Pattern Generator Table */
	blockwrite(alpha, PGTAB, 1024);
	
	/* set text background and foreground colors */
	bordercolor((color1<<4 & color0) & 0xFF);
	
	/* debug */
	bordercolr(0xC1);
	
	/* enable display */
	vdpoptions(VPNEV+VP16K+VPEDP+VPTX);
}

cls()
{
	int i;
	
	for (i=0; i<NROWS; i++) {
		clrrow(i);
		blockwrite(&screen[i][0],PNTAB+i*NCOLS, NCOLS);
	}
}

clrrow(r)
int r;
{
	int c;
	char *ch;
	
	ch = &screen[r][0];
	for (c=0; c<NCOLS; c++)
		*ch++ = ' ';
}

main()
{
	int i, j, ch;
	char c;
	
	puts("This program reads text typed on the console\n");
	puts("or redirected via the '<' character and displays\n");
	puts("the text on the video screen using TEXT mode\n");
	
  inittxmode(c_black, c_dkgrn);
	
	cls();

	row = 0;
	col = 0;	
	while ((c=getchar()) != EOF) {
		if (c == NL) {
			/* New line! first write the current row, then
			** advance the row counter.  wrap around to top when
			** screen is full. clear the new row before filling
			** with new characters. if more than NCOLS characters
			** are received for a given row the excess characters
			** are dropped.
			*/
			blockwrite(&screen[row][0], PNTAB+row*NCOLS, NCOLS);
			col = 0;
			if (++row == NROWS)
				row = 0;
			clrrow(row);
		} else {
				/* just add the character to the array */
				if (col < NCOLS)
					screen[row][col++] = c & 0xFF;
		}
	}
	
	/* if any data left on current row, print it */
	if (col != 0)
		blockwrite(&screen[row][0],PNTAB+row*NCOLS, NCOLS);

}
