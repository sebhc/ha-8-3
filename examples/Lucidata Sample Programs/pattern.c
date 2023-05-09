/*
**	pattern
**
**	Adapted from the program PATTERN.PAS in "Graphics Support 
**	Routines for V3.8	Lucidata Pascal", Copyright (C) 1982 Polybytes
**	(public released 1988).
**
**	This program displays a repeated pattern of "X" and filled square symbols
**	across the screen in a 32x24 grid of 8x8 pixels each.
**
**	Demonstrates use of the TMS9918A VDP "Graphics 1" ("pattern") mode. 
**	Since there is not an initialization routine for Graphics 1 this program
**	demonstrates how the user can use the primitive VDP manipulation routines
**	to set up the registers, color tables and patterns to display the desired
**	result.
**
**	Tailored for Software Toolworks C/80 3.1
**	Use M80 for assembler. Required libraries: CLIBRARY.REL and HA83.REL 
**
**	Recommended link command:
**
**	L80 PATTERN,HA83,CLIBRARY,PATTERN/N/E/M
**
**	Adapted by Glenn Roberts 28 October 2022
*/
#include "ha83.h"

/* VRAM MAP
**
**		Mode: Graphics I ("pattern" mode)
**
**		0..2047	(2K) Pattern Generator Table (PGTAB)
**		2048..2079 (32 bytes) Pattern Color Table (CGTAB)
**		6144..6911 (768 bytes) Pattern Name Table (PNTAB)
**		7168..7296 (128 bytes) Sprite Name (Attribute) Table (SNTAB)
**
*/
#define	PGTAB	0
#define CGTAB	2048
#define	PNTAB	6144
#define SNTAB	7168

/*	two 8x8 pixel patterns to be displayed :
**
**	........
**	.XXXXXX.
**	.X....X.
**	.X.XX.X.
**	.X.XX.X.
**	.X....X.
**	.XXXXXX.
**	........
*/
char ptrn0[] = { 
	0x00, 0x7E, 0x42, 0x5A, 0x5A, 0x42, 0x7E, 0x00,
};
/*
**	........
**	.X....X.
**	..X..X..
**	...XX...
**	...XX...
**	..X..X..
**	.X....X.
**	........
*/
char ptrn1[] = { 
	0x00, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x00
};

char namtab[768];
char clrtab[32];
int i;


main()
{
	/* initialize VDP registers: no external video,
	** 16K RAM chips, blank display, pattern mode
	** (Graphics 1)
	*/
	vdpoptions(VPNEV+VP16K+VPDDP+VPPM);

	/* set up table addresses */
	pattgentable(PGTAB);
	colgentable(CGTAB);
	pattnametable(PNTAB);
	sprnametable(SNTAB);

	bordercolor(c_white);

	/* disable sprites */
	wrtvramdirect(SNTAB,0xD0);

	/* set up options again, this time with display enabled */
	vdpoptions(VPNEV+VP16K+VPEDP+VPPM);

	/* set up pattern name table, alternating entries between the
	** two patterns. There are 24*32=768 entries in the name table.
	** we will cycle through all 256 patterns defined in the pattern
	** table (see below) and then send the pattern name table to VRAM.
	*/
	for (i=0; i<768; i++)
		namtab[i] = i%256;
	blockwrite(namtab,PNTAB,768);

	/* here we define the patterns. although we only have two
	** patterns defined we want to create a full table of 256
	** possible patterns to demonstrate the use of color. the
	** first 8 patterns will be displayed with color 0, the next
	** 8 with color 1, the next 8 with color 2 and so on through
	** all 32 possible combinations in the color generator table
	** (see below). we will fill the table up (256*8=2048 bytes)
	*/
	for (i=0; i<256; i++)
		blockwrite(((i%2) ? ptrn0 : ptrn1), PGTAB+(i*8), 8);
	
	/* here we create the color generator table. there are 32 total
	** entries in the table. we cycle through all 16 possible colors
	** making the first 16 table entries have transparent background and
	** color foreground and then the next 16 transparent foreground
	** and color background. this will cause all possible color
	** combinations to be available
	*/
	for (i=0; i<16; i++)
		clrtab[i] = c_transp + 16*(i%16);
	for (i=16; i<32; i++)
		clrtab[i] = (i%16) + 16*c_transp;
	blockwrite(clrtab,CGTAB,32);
}