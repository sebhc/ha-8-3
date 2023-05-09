/*
**	aflag - patterned after AFLAG.ASM from HUG 885-1098
**
**	displays an image of the american flag. this program 
**	demonstrates the use of Grapics I mode. there are
**	four distinct patterns and five possible foreground/
**	background color options. together there are 8
**	combinations of patterns and colors needed to display
**	the flag.  here the Pattern Name Table is statically
**	defined to show which pattern/color combination is
**	to be displayed in each of the 768 8x8 pixel positions.
**	all the program has to do is define the contents of the
**	three tables: Pattern Generator Table (which contains
**	the 8 pattern/color combinations), the Pattern Color
**	Table (which contains the five color combinations),
**	and the Pattern Name Table (which is simply loaded
**	from the pnt[][] array).
**
**	G. Roberts 9 April 2023
*/
#include "ha83.h"

/* VRAM MAP
**
**		Mode: Graphics I
**
**		0..767 (768 bytes) Pattern Name Table (PNTAB)
**		768..895 (128 bytes) Sprite Attribute Table (SNTAB)
**		896..927 (32 bytes) Pattern Color Table (CGTAB)
**		2048..14335	(2K) * 6 Pattern Generator Table(s) (PGTAB)
**		4096 Sprite Generator Table (SPTAB) (sprites not used)
**
** this program loads 6 different images at 6 different
** locaitons in the Pattern Generator Table, then animates
** the screen by cycling through the images
**
*/
#define	PNTAB	0
#define SNTAB	768
#define CGTAB	896
#define	PGTAB	2048
#define	SPTAB	4096

#define BGCOL	c_transp

#define	NROWS	24
#define	NCOLS	32

/* color combinations used in the pattern name table
**
**	X  - background (transparent)
**	BT - half blue, half transparent
**	RT - half red, half transparent
**	B  - solid blue
**  BS - blue with white star
**  R  - solid red
**  W  - solid wite
**  WR - half red, half white
*/
#define	X		0
#define	BT	8
#define	RT	16
#define	B		24
#define	BS	25
#define	R		32
#define	W		33
#define	WR	34

/* blank (all background) cell */
char patt1[8] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};
/* half background/foreground solid cell */
char patt2[8] = {
	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF 
};
/* star pattern in foreground color */
char patt3[8] = {
	0x00, 0x08, 0x08, 0x1C, 0x7F, 0x1C, 0x36, 0x22 
};
/* solid (all foreground) cell */
char patt4[8] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF 
};

/* Pattern Name Table showing a U.S. flag that is 29 cells wide and 20 cells high */
char pnt [NROWS][NCOLS] = {
	{X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X},
	{X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X},
	{X, X,BT,BT,BT,BT,BT,BT,BT,BT,BT,BT,BT,BT,BT,RT,RT,RT,RT,RT,RT,RT,RT,RT,RT,RT,RT,RT,RT,RT,RT, X},
	{X, X, B,BS, B,BS, B,BS, B,BS, B,BS, B,BS, B, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, X},
	{X, X, B, B,BS, B,BS, B,BS, B,BS, B,BS, B, B, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, X}, 
	{X, X, B,BS, B,BS, B,BS, B,BS, B,BS, B,BS, B,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR, X},
	{X, X, B, B,BS, B,BS, B,BS, B,BS, B,BS, B, B, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, X}, 
	{X, X, B,BS, B,BS, B,BS, B,BS, B,BS, B,BS, B, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, X},
	{X, X, B, B,BS, B,BS, B,BS, B,BS, B,BS, B, B,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR, X}, 
	{X, X, B,BS, B,BS, B,BS, B,BS, B,BS, B,BS, B, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, X},
	{X, X, B, B,BS, B,BS, B,BS, B,BS, B,BS, B, B, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, X}, 
	{X, X, B,BS, B,BS, B,BS, B,BS, B,BS, B,BS, B,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR, X},
	{X, X, B, B, B, B, B, B, B, B, B, B, B, B, B, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, X},
	{X, X, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, X},
	{X, X,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR, X},
	{X, X, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, X},
	{X, X, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, X},
	{X, X,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR, X},
	{X, X, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, X},
	{X, X, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, X},
	{X, X,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR,WR, X},
	{X, X, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, X},
	{X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X},
	{X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X}
};

/* loadpatt - load a pattern into the pattern generator table */
loadpatt(vaddr, patt)
int vaddr;
char *patt;
{
	static int i;
	
	for (i=0; i<8; i++)
		wrtvramdirect(vaddr++, *patt++);
}

/* vdpinit - initialize the Video Display Processor settings */
vdpinit()
{
	/* Initialize VDP:
	**
	**	No External Videro
	**	16K RAM
	**	Enable display
	**	Pattern mode (Graphics I)
	**	Small sprites (8x8)
	**	No sprite magnification
	*/
	vdpoptions(VPNEV+VP16K+VPEDP+VPPM);

	/* set up table addresses */
	pattgentable(PGTAB);
	colgentable(CGTAB);
	pattnametable(PNTAB);
	sprnametable(SNTAB);
	sprpatrntable(SPTAB);
	
	/* set background color */
	bordercolor(BGCOL);
	
	/* disable sprite processing */
	wrtvramdirect(SNTAB, 0xD0);
}

main()
{
	/* initialize the VDP */
	vdpinit();

	/* set up Pattern Color Table */
	wrtvramdirect(CGTAB,   c_transp*16 + c_transp);	/* 0-7 */
	wrtvramdirect(CGTAB+1, c_dkblue*16 + c_transp); /* 8-15 */
	wrtvramdirect(CGTAB+2, c_dkred*16  + c_transp);	/* 16-23 */
	wrtvramdirect(CGTAB+3, c_white*16  + c_dkblue);	/* 24-31 */
	wrtvramdirect(CGTAB+4, c_dkred*16  + c_white );	/* 32-39 */
	
	/* set up  Pattern Generator Table */
	loadpatt(PGTAB+X,    patt1);	/* solid background */
	loadpatt(PGTAB+8*BT, patt2);	/* half blue, half background */
	loadpatt(PGTAB+8*RT, patt2);	/* half red, half background */
	loadpatt(PGTAB+8*B,  patt1);	/* solid blue */
	loadpatt(PGTAB+8*BS, patt3);	/* white stars, blue background */
	loadpatt(PGTAB+8*R,  patt4);	/* solid red */
	loadpatt(PGTAB+8*W,  patt1);	/* solid white */
	loadpatt(PGTAB+8*WR, patt2);	/* half red, half white */
	
	/* now just insert the flag pattern in the Pattern Name Table */
	blockwrite(pnt, PNTAB, NROWS*NCOLS);
}
