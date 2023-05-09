/*
	vectors - this program draws a series of vectors on the screen. it
	demonstrates the use of the move and plot functions for
	the ha83 function.
*/

/* the following should go into a .h file... /GFR/ */

#define	VPNEV		0			/* no external video */
#define	VP16K		0x80	/* 16k ram chips */
#define	VPDDP		0			/* blank display */
#define	VPEDP		0x40	/* enable display */
#define	VPDI		0			/* disable interrupts */
#define	VPPM		0			/* pattern mode */

#define	c_transp		0
#define	c_black			1
#define	c_midgrn		2
#define	c_ltgrn			3
#define	c_dkblue		4
#define	c_ltblue		5
#define	c_dkred			6
#define	c_cyan			7
#define	c_midred		8
#define	c_ltred			9
#define	c_dkyell		10
#define	c_ltyell		11
#define	c_dkgrn			12
#define	c_magenta		13
#define	c_gray			14
#define	c_white			15

#define MAXY		24
#define	MAXX		32

int color,xborder,yborder,mag,ssize;

/* increment color, skipping over transparent */
changecolor()
{
	color = (color + 1) % 16;
	if (color == c_transp)
		++color;
}


main()
{
	color = c_black;
	mag = 0;
	ssize = 0;
	
  initmcmode(c_dkblue,mag,ssize);
	
	/* Loop forever */
  do {
		xborder = 0;
		yborder = 0;
		do {
			mcmove(xborder,yborder,color);
			mcdraw(63-xborder,yborder,color);
			mcdraw(63-xborder,47-yborder,color);
			mcdraw(xborder,47-yborder,color);
			mcdraw(xborder,yborder,color);
			changecolor();
			++yborder;
			++xborder;
		} while (yborder<MAXY);
		do {
			mcmove(xborder,23,color);
			mcdraw(63-xborder,23,color);
			mcmove(xborder,24,color);
			mcdraw(63-xborder,24,color);
			changecolor();
			++xborder;
		} while (xborder<MAXX);
		xborder = 22;
		yborder = 22;
		do {
			mcmove(xborder,yborder,color);
			mcdraw(63-xborder,yborder,color);
			mcdraw(63-xborder,47-yborder,color);
			mcdraw(xborder,47-yborder,color);
			mcdraw(xborder,yborder,color);
			changecolor();
			--yborder;
			--xborder;
		} while (yborder >= 0);
  } while (1);
}
