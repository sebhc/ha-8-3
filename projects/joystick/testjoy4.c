/*	testjoy4
**
**	Testing "Atari" style joystick interface. Display
**	four sprites (one per joystick) and update their
**	position based on joystick action. The "fire"
**	button will cause the sprite to turn read for
**	as long as it is held down.
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
#define DX	1
#define DY	1

/* number of joysticks */
#define	NJS	4

/* number of sprites */
#define NSPR	4

/* bit masks for joystick button functions */
#define	UP		0x01
#define	DOWN	0x02
#define	FIRE	0x03
#define	LEFT	0x01
#define	RIGHT	0x02

/* sprite patterns for digits 1, 2, 3 and 4 */
char spats[NSPR][8] = {
	{0x08, 0x18, 0x28, 0x08, 0x08, 0x08, 0x08, 0x3E},
	{0x3E, 0x41, 0x01, 0x02, 0x1C, 0x20, 0x40, 0x7F},
	{0x3E, 0x41, 0x01, 0x01, 0x1E, 0x01, 0x41, 0x3E},
	{0x02, 0x06, 0x0A, 0x12, 0x22, 0x7F, 0x02, 0x02}
};

/* sprite structure */
struct sstruct {
	int x;
	int y;
	int color;
	char *pattern;
} sprite[NSPR];

/* default colors for the four sprites */
int spcolors[NSPR] = { c_midgrn, c_cyan, c_dkyell, c_magenta };

/* joystick state structure */
struct jstruct {
	int u;
	int d;
	int l;
	int r;
	int f;
} joystick[NJS];

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


/* getjs - sets the state of the four joystick structures based on 
** the current status of the joystick switches.
**
**	reads the state of ports A and B in the psg to determine state
**
**	Joysticks are interfaced via the PSG parallel ports:
**
** Stick:    #1      #2      #3      #4
**  ===================================	
**	Up       A0      A2      A4      A6
**	Dn       A1      A3      A5      A7
**	Lt       B0      B2      B4      B6
**	Rt       B1      B3      B5      B7
**
**	Values are 0 when asserted, 1 otherwise
**
** "Fire" button asserts Up and Dn at same time
*/
getjs()
{
	int i, porta, portb;
	struct jstruct *js;
	
	porta = rdpsgport('A');
	portb = rdpsgport('B');
	js = &joystick[0];
	
	for (i=0; i<NJS; i++) {
		if (!(porta & FIRE))
			js->f = TRUE;
		else {
			js->f = FALSE;
			js->u = !(porta & UP);
			js->d = !(porta & DOWN);
		}
		js->l = !(portb & LEFT);
		js->r = !(portb & RIGHT);
		
		/* shift  both ports right 2 bits and
		** point to next joystick entry
		*/
		porta >>= 2;
		portb >>= 2;
		++js;
	}
}

initsprites()
{
	int i;
	struct sstruct *sp;
	
	for (i=0; i<NSPR; i++) {
		sp = &sprite[i];
		
		/* populate sprite data structure. position sprites
		** initially in the center of the screen but offset
		** a bit so they're all visible.
		*/
		sp->x = XWIDTH/2 +  ((i%2) ? 10 : -10);
		sp->y = YHEIGHT/2 + ((i<2) ? -10 : 10);
		sp->color = spcolors[i];
		sp->pattern = &spats[i][0];
		
		/* now create the sprite */
		defsprite(i, sp->x, sp->y, sp->color, 0, sp->pattern);
	}
}


updatesprites()
{
	int i;
	struct sstruct *sp;
	struct jstruct *js;
	
	for (i=0; i<NJS; i++) {
		/* first update the sprite data structures based on
		** joystick input. NOTE: below assumes there is one
		** sprite per joystick
		*/
		js = &joystick[i];
		sp = &sprite[i];
		
		/* change color to Dark Red while fire button is down */
		if (js->f)
			sp->color = c_dkred;
		else
			sp->color = spcolors[i];
		
		/* change the sprite position according to joystick action */
		if (js->u)
			sp->y -= DY;
		if (js->d)
			sp->y += DY;
		if (js->l)
			sp->x -= DX;
		if (js->r)
			sp->x += DX;
			
		/* make sure we stay inside the screen area */
		sp->x %= XWIDTH;
		sp->y %= YHEIGHT;
		
		/* now update the actual sprite position and color */
		positsprite(i, sp->x, sp->y);
		chgsprcolor(i, sp->color);
	}
}

main()
{
	/* multi-color; black bg; 8x8 sprites; no magnification */
	initmcmode(c_black, 0, 0);
	initsprites();
	
	/* now just loop, checking joysticks and updating the
	** sprites accordingly
	*/
	do {
		/* update joystick status */
		getjs();
		
		/* update sprite conditions */
		updatesprites();

		/* make sure ^C is detected if hit */
#ifdef	CPM
		CtlCk();
#endif
		mswait(10);
	} while(TRUE);
}
