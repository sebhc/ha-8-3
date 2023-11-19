/*
**	globe - patterned after GLOBE.ASM from HUG 885-1098
**
**	there are 6 patterns each representing a 16x16 cell (8x8 bits)
**	these are read into 6 diferent Pattern Generator Tables (2K each)
**	and then they are displayed in sequence by changing the PGTAB
**	address in the VDP.  The images are displayed in a 16x16
**	block area centered in the screen.
**
**	G. Roberts 8 April 2023
*/
#include "ha83.h"

#define	TRUE	1
#define	FALSE	0

#define	BLOCKSIZE	256

/* VRAM MAP
**
**		Mode: Graphics I
**
**		0..767 (768 bytes) Pattern Name Table (PNTAB)
**		768..895 (128 bytes) Sprite Attribute Table (SNTAB)
**		896..927 (32 bytes) Pattern Color Table (CGTAB)
**		2048..14335	(2K) * 6 Pattern Generator Table(s) (PGTAB)
**		0 Sprite Generator Table (SPTAB) (sprites not used)
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
#define	SPTAB	0

#define FGCOL	c_white
#define BGCOL	c_dkblue

#define	NROWS	24
#define	NCOLS	32

/* array where we build the Pattern Name Table */
char pnt [NROWS][NCOLS];

/* millisecond timer utility to pace the speed */

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

/* mswait - wait a specified number of milliseconds */
mswait(milliseconds)
unsigned milliseconds;
{
	/* 2 ms per tick */
	timeout = *Ticptr + (milliseconds >> 1);
	while (*Ticptr != timeout)
		/* do nothing, just wait... */;
}

/* use puts() for simple text I/O (avoids printf) */
puts(s)
char *s;
{
	while (*s)
		putchar(*s++);
}

/* loadimages - read six images and load them in VRAM */
loadimages()
{
	static int i, vaddr, fh, rc;
	static char buff[BLOCKSIZE];
	
	if ((fh=fopen("GLOBE.DAT", "rb")) == 0)
		rc = -1;
	else {
		rc = 0;
		/* load 6 2K images, reading one block at
		** a time, and load them in consecutive locations
		** in VRAM starting at PGTAB.
		**
		** 48 * BLOCKSIZE = 6 * 2048 = 12228 bytes
		*/
		vaddr = PGTAB;
		for (i=0; i<48; i++) {
			if ((read(fh, buff, BLOCKSIZE)) != BLOCKSIZE) {
				rc = -1;
				break;
			}
			blockwrite(buff, vaddr, BLOCKSIZE);
			vaddr += BLOCKSIZE;
		}
	}
	
	return rc;
}

/* pntsetup - created the pattern name table to display
** the images in a 16x16 block area centered on the screen
*/
pntsetup()
{
	static int row, col, pattern;
	
	pattern = 0;

	for (row=0; row<NROWS; row++) {
		for (col=0; col<NCOLS; col++) {
			/* within the bounding box install patterns
			** sequentially 0..256, otherwise 0 outside the box
			*/
			if ((row>=4) && (row<20) && (col>=8) && (col<24))
				pnt[row][col] = pattern++;
			else
				pnt[row][col] = 0;
		}
	}
	
	/* now install it in VRAM */
	blockwrite(pnt, PNTAB, NROWS*NCOLS);
}

/* vdpinit - initialize the Video Display Processor settings 
** (but leave the display off)
*/
vdpinit()
{
	static int i;
	
	/* Initialize VDP:
	**
	**	No External Videro
	**	16K RAM
	**	Disable display
	**	Pattern mode (Graphics I)
	**	Small sprites (8x8)
	**	No sprite magnification
	*/
	vdpoptions(VPNEV+VP16K+VPDDP+VPPM);

	/* set up table addresses */
	pattgentable(PGTAB);
	colgentable(CGTAB);
	pattnametable(PNTAB);
	sprnametable(SNTAB);
	sprpatrntable(SPTAB);
	
	/* set border and background the same */
	bordercolor(BGCOL);
	
	/* set up color table - all the same */
	for (i=0; i<32;i++)
		wrtvramdirect(CGTAB+i, FGCOL*16+BGCOL);
	
	/* construct the special Pattern Name Table */
	pntsetup();
	
	/* disable sprite processing */
	wrtvramdirect(SNTAB, 0xD0);
}

main()
{
	static int i;
	
	/* initialize the VDP */
	vdpinit();

	/* read the image files and store in VRAM */
	if ((loadimages()) != 0)
		puts("Error loading images\n");
	else {
		/* let's go live! - set up options again with display enabled */
		vdpoptions(VPNEV+VP16K+VPEDP+VPPM);
	
		/* now just loop through the images. all we have to to to
		** change the image is change the pointer to the pattern
		** generator table, then pause briefly for animation effect.
		*/
		do {
			for (i=6; i>0; i--) {
				pattgentable(i*PGTAB);
				mswait(128);
			}
#ifdef	CPM
			CtlCk();
#endif
		} while (TRUE);
	}
}
