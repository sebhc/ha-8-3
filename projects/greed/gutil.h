/********************************************************
** gutil.h
**
** template definitions for gutil library, plus useful
** shared values.
**
**	16 July 2022
**	Glenn Roberts
**
********************************************************/
#ifndef EXTERN
#define EXTERN extern
#endif

/* define "OSFREE" for OS-free (stand-alone) version */
#define	OSFREE


/* HDOS TICCNT location */
#define TICCNT	0x201B
EXTERN unsigned *Ticptr
#ifdef INIT
 = TICCNT
#endif
;
EXTERN unsigned timeout;

#define	TRUE	1
#define	FALSE	0

/* dice status 
**
** ACTIVE means the die is still in play and can be rolled.
** SCORING means the die achieved a score on this roll. This
** allows for animation of newly-scoring dice.  At the end
** of the roll SCORING dice get promoted to HELD. HELD
** dice are displayed in a different location with a
** different color.
**
** Once a die is HELD it will stay there until:
**
**	1. the turn is over and the next player does not want
**     to continue.
**  2. the player goes "bust", ending the turn
**  3. all five dice are HELD, then they get reset to ACTIVE
**     and play continues.
*/
#define	ACTIVE	0
#define	HELD		1
#define SCORING	2

/* screen row used for general messaging... */
#define	MSGROW	21

/* VRAM MAP
**
**		Mode: Graphics I
**
**		0..2047	(2K) Pattern Generator Table (PGTAB)
**		6144..6911 (768 bytes) Pattern Name Table (PNTAB)
**		7168..7296 (128 bytes) Sprite Attribute Table (SNTAB)
**		8192..8224 (32 bytes) Pattern Color Table (CGTAB)
**		14336..16383 (2K) Sprite Generator Table (SPTAB)
**
** in Grapics II mode the PG Table would expand from 2K to 6K
** and the color table would expand from 32 bytes to 6K.  These
** assignments could still be used in that case.
*/
#define	PGTAB	0
#define	PNTAB	6144
#define SNTAB	7168
#define CGTAB	8192
#define	SPTAB	14336

/* offsets into the pattern table */
/* base for ASCII set */
#define	ABASE	128

/* offset from standard to emphasized ASCII */
#define AOFFSET	64

/* base for dice = 0 */
/* offset between normal dice patterns and emphasized ones */
#define	EOFFSET	40

/* base for "GREED" dice */
#define GREEDBASE	80

/* "blank" pattern used to fill screen or blank a row
** this pattern is fully transparent so it simply
** displays the background color
*/
#define	NULLPAT 120

/* "solid" pattern used to create table for rolling
** this pattern is all 1's so it simply displays
** the foreground color
*/
#define	SOLIDPAT 121

#define NDICE	5
#define	WINSCORE	10000
#define MAXPLAYERS	4
#define	EMPTY	'X'

/* sprite-related definitions... */

/* bounding box for upper left corner of 32x32
** sprites. subtract 32 from the max values to allow
** for width of die.
*/
#define	DXMIN	64
#define DYMIN 40
#define	DXMAX 224
#define	DYMAX 136

/* the sprites are constrained to live on a grid whose
** vertices are on even multiples of 32. grid locations
** are in the range [0..NGX-1, 0..NGY-1]
*/
#define	NGX 6
#define	NGY	4

/* each die has:
**	value:	one less than face value of the die [0..5]
**	row:		Screen row of upper left corner
**	column:	Screen column of upper left corner
**	color;	Foreground color for the sprite
**	gridx:	x grid location for sprite
**	gridy:	y grid location for sprite
**	status:	ACTIVE = display as normal (die in play)
**		HELD = highlight in blue (die scores on this turn)
*/
struct die {
	int value;
	int row;
	int column;
	int color;
	int gridx;
	int gridy;
	int status;
};

/* players */
struct p {
	char *name;
	int score;
	int inthegame;
	char greenmask;
	char redmask;
};

/* the 768 patterns that define the screen */
EXTERN char screen[24][32];
EXTERN struct die dice[NDICE]
#ifdef INIT
 = {
	1, 5,  8, c_white,   0, 0, ACTIVE,
	2, 5, 13, c_ltyell,  0, 1, ACTIVE,
	3, 5, 18, c_cyan,    0, 2, ACTIVE,
	4, 5, 23, c_ltred,   0, 3, ACTIVE,
	5, 5, 28, c_magenta, 1, 0, ACTIVE
}
#endif
;

/* grid keeps track of sprite [row][column] */
EXTERN char grid[NGY][NGX];


/* routines defined in gutil.c */
int wait();
int mswait();
char *itoarj();
int showval();
int showstr();
int jsidle();
int rjs();
int rndrange();
int clrdice();
int clractive();
int cls();
int refresh();
int clrrow();
int fillbox();
int allheld();
int allactive();
int yloc();
int xloc();
int nrow();

/* these routines currently have OS dependencies */
#ifndef OSFREE
int puts();
int putval();
int dumpdie();
#endif