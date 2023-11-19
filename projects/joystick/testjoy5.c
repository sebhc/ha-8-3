/*	testjoy4
**
**	Testing "Atari" style joystick interface. 
**
** Draws a simple "cannon" icon and lets you
** move it back and forth with Joystick #1
**	
*/
#include "ha83.h"

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

#define	TRUE	1
#define	FALSE	0

/* total screen size */
#define XWIDTH	256
#define	YHEIGHT	192

/* delta  to move sprites on each iteration  */
#define DX	2

/* bounding range for cannon movement */
#define	XMIN	10
#define	XMAX	240

/* bit masks for joystick button functions */
#define	UP		0x01
#define	DOWN	0x02
#define	FIRE	0x03
#define	LEFT	0x01
#define	RIGHT	0x02

char csprite[64] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x03, 0x0F, 0x1F, 0x1F, 0x1F, 0x0C, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0xE0, 0xF0, 0xF0, 0xF0, 0x60
};

/* sprite structure */
struct sstruct {
	int x;
	int y;
	int color;
	char *pattern;
} cannon;

/* joystick state structure */
struct jstruct {
	int u;
	int d;
	int l;
	int r;
	int f;
} joystick;

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
		/*  just wait... */;
}

main()
{
	int smag, ssize, porta, portb;
	
	/* multi-color; black bg; 16x16 sprites; no magnification */
	smag = 0;
	ssize = 1;
	initmcmode(c_black, smag, ssize);

  /* create the cannon sprite */
	cannon.x = XWIDTH/2;
	cannon.y = 170;
	cannon.color = c_dkblue;
	cannon.pattern = csprite;
	defsprite(0, cannon.x, cannon.y, cannon.color, 0, cannon.pattern);
	
	/* now just loop, checking joystick #1 and updating the
	** sprite accordingly
	*/
	do {
		/* update joystick status */
		porta = rdpsgport('A');
		if (!(porta & FIRE))
			joystick.f = TRUE;
		else
			joystick.f = FALSE;

		portb = rdpsgport('B');
		joystick.l = !(portb & LEFT);
		joystick.r = !(portb & RIGHT);
		
		/* now update the cannon sprite position */
		if (joystick.l)
			cannon.x -= DX;
		if (joystick.r)
			cannon.x += DX;
			
		/* make sure we stay in range */
		if (cannon.x < XMIN) 
			cannon.x = XMIN;
		if (cannon.x > XMAX)
			cannon.x = XMAX;
		
		/* now update the actual sprite position and color */
		positsprite(0, cannon.x, cannon.y);
		chgsprcolor(0, cannon.color);

		/* make sure ^C is detected if hit */
#ifdef	CPM
		CtlCk();
#endif
		mswait(10);
	} while(TRUE);
}
