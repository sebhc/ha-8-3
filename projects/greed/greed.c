/* dice - display dice on the screen
*/

#include "ha83.h"

/* HDOS TICCNT location */
#define TICCNT	0x201B

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

/* grid keeps track of dice [row][column] */
char grid[NGY][NGX];

/* the 768 patterns that define the screen */
static char screen[24][32];

/* 32-entry color table */
static char clrtab[32];

/* Patterns to form dice */
static char ptrns[33][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0},
	{0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0},
	{0xFF, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
	{0xFF, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x81, 0x83},
	{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
	{0x83, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
	{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x81, 0x83},
	{0x83, 0x81, 0x80, 0x80, 0x80, 0x80, 0x81, 0x83},
	{0xFF, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
	{0xFF, 0x03, 0x01, 0x01, 0x01, 0x01, 0x81, 0xC1},
	{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
	{0xC1, 0x81, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
	{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x81, 0xC1},
	{0xC1, 0x81, 0x01, 0x01, 0x01, 0x01, 0x81, 0xC1},
	{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xFF},
	{0x83, 0x81, 0x80, 0x80, 0x80, 0x80, 0xC0, 0xFF},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},
	{0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},
	{0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},
	{0xC3, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},
	{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0xFF},
	{0xC1, 0x81, 0x01, 0x01, 0x01, 0x01, 0x03, 0xFF},
	{0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0},
	{0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03},
	{0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0xC3},
	{0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03},
	{0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0},
	{0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03}
};


/*	ASCII character set from TMS9918A manual
**
**  !"#$%&'()*+,-./0123456789:;<=>?
** @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
** `abcdefghijklmnopqrstuvwxyz{>}~
*/

static char alpha[96][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x20, 0x00},
	{0x50, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x50, 0x50, 0xF8, 0x50, 0xF8, 0x50, 0x50, 0x00},
	{0x20, 0x78, 0xA0, 0x70, 0x28, 0xF0, 0x20, 0x00},
	{0xC0, 0xC8, 0x10, 0x20, 0x40, 0x98, 0x18, 0x00},
	{0x40, 0xA0, 0xA0, 0x40, 0xA8, 0x90, 0x68, 0x00},
	{0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x20, 0x40, 0x80, 0x80, 0x80, 0x40, 0x20, 0x00},
	{0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00},

	{0x20, 0xA8, 0x70, 0x20, 0x70, 0xA8, 0x20, 0x00},
	{0x00, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x40, 0x00},
	{0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00},
	{0x00, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00},
	{0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70, 0x00},
	{0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00},
	{0x70, 0x88, 0x08, 0x30, 0x40, 0x80, 0xF8, 0x00},
	{0xF8, 0x08, 0x10, 0x30, 0x08, 0x88, 0x70, 0x00},

	{0x10, 0x30, 0x50, 0x90, 0xF8, 0x10, 0x10, 0x00},
	{0xF8, 0x80, 0xF0, 0x08, 0x08, 0x88, 0x70, 0x00},
	{0x38, 0x40, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00},
	{0xF8, 0x08, 0x10, 0x20, 0x40, 0x40, 0x40, 0x00},
	{0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70, 0x00},
	{0x70, 0x88, 0x88, 0x78, 0x08, 0x10, 0xE0, 0x00},
	{0x00, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x20, 0x00, 0x20, 0x20, 0x40, 0x00},
	{0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x00},
	{0x00, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x00, 0x00},

	{0x40, 0x20, 0x10, 0x08, 0x10, 0x20, 0x40, 0x00},
	{0x70, 0x88, 0x10, 0x20, 0x20, 0x00, 0x20, 0x00},
	{0x70, 0x88, 0xA8, 0xB8, 0xB0, 0x80, 0x78, 0x00},
	{0x20, 0x50, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x00},
	{0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0, 0x00},
	{0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00},
	{0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00},
	{0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00},
	{0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0x80, 0x00},
	{0x78, 0x80, 0x80, 0x80, 0x98, 0x88, 0x78, 0x00},

	{0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00},
	{0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00},
	{0x08, 0x08, 0x08, 0x08, 0x08, 0x88, 0x70, 0x00},
	{0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88, 0x00},
	{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8, 0x00},
	{0x88, 0xD8, 0xA8, 0xA8, 0x88, 0x88, 0x88, 0x00},
	{0x88, 0x88, 0xC8, 0xA8, 0x98, 0x88, 0x88, 0x00},
	{0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00},
	{0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80, 0x00},
	{0x70, 0x88, 0x88, 0x88, 0xA8, 0x90, 0x68, 0x00},

	{0xF0, 0x88, 0x88, 0xF0, 0xA0, 0x90, 0x88, 0x00},
	{0x70, 0x88, 0x80, 0x70, 0x08, 0x88, 0x70, 0x00},
	{0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00},
	{0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00},
	{0x88, 0x88, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00},
	{0x88, 0x88, 0x88, 0xA8, 0xA8, 0xD8, 0x88, 0x00},
	{0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0x00},
	{0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x20, 0x00},
	{0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xF8, 0x00},
	{0xF8, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xF8, 0x00},

	{0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00},
	{0xF8, 0x18, 0x18, 0x18, 0x18, 0x18, 0xF8, 0x00},
	{0x00, 0x00, 0x20, 0x50, 0x88, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00},
	{0x40, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x70, 0x88, 0xF8, 0x88, 0x88, 0x00},
	{0x00, 0x00, 0xF0, 0x48, 0x70, 0x48, 0xF0, 0x00},
	{0x00, 0x00, 0x78, 0x80, 0x80, 0x80, 0x78, 0x00},
	{0x00, 0x00, 0xF0, 0x48, 0x48, 0x48, 0xF0, 0x00},
	{0x00, 0x00, 0xF0, 0x80, 0xE0, 0x80, 0xF0, 0x00},

	{0x00, 0x00, 0xF0, 0x80, 0xE0, 0x80, 0x80, 0x00},
	{0x00, 0x00, 0x78, 0x80, 0xB8, 0x88, 0x70, 0x00},
	{0x00, 0x00, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x00},
	{0x00, 0x00, 0xF8, 0x20, 0x20, 0x20, 0xF8, 0x00},
	{0x00, 0x00, 0x70, 0x20, 0x20, 0xA0, 0xE0, 0x00},
	{0x00, 0x00, 0x90, 0xA0, 0xA0, 0xe0, 0x90, 0x00},
	{0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0xF8, 0x00},
	{0x00, 0x00, 0x88, 0xD8, 0xA8, 0x88, 0x88, 0x00},
	{0x00, 0x00, 0x88, 0xC8, 0xA8, 0x98, 0x88, 0x00},
	{0x00, 0x00, 0xF8, 0x88, 0x88, 0x88, 0xF8, 0x00},

	{0x00, 0x00, 0xF0, 0x88, 0xF0, 0x80, 0x80, 0x00},
	{0x00, 0x00, 0xF8, 0x88, 0xA8, 0x90, 0xE8, 0x00},
	{0x00, 0x00, 0xF8, 0x88, 0xF8, 0xA0, 0x90, 0x00},
	{0x00, 0x00, 0x78, 0x80, 0x70, 0x08, 0xF0, 0x00},
	{0x00, 0x00, 0xF8, 0x20, 0x20, 0x20, 0x20, 0x00},
	{0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00},
	{0x00, 0x00, 0x88, 0x88, 0x90, 0xA0, 0x40, 0x00},
	{0x00, 0x00, 0x88, 0x88, 0xA8, 0xD8, 0x88, 0x00},
	{0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88, 0x00},
	{0x00, 0x00, 0x88, 0x50, 0x20, 0x20, 0x20, 0x00},

	{0x00, 0x00, 0xF8, 0x10, 0x20, 0x40, 0xF8, 0x00},
	{0x38, 0x40, 0x20, 0xC0, 0x20, 0x40, 0x38, 0x00},
	{0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00},
	{0xE0, 0x10, 0x20, 0x18, 0x20, 0x10, 0xE0, 0x00},
	{0x40, 0xA8, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xA8, 0x50, 0xA8, 0x50, 0xA8, 0x50, 0xA8, 0x00}
};

static char nullpatt[8] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static char solidpatt[8] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* indices of patterns for each die - six possible
** die values [1..6] and each one represented as
** 4x4 array of 8-bit values
*/
static char dicepatt [6][16] = {
	{ 6,26,26,12, 8, 3, 4,14, 8, 2, 1,14,18,20,20,24 },
	{ 6,26,28,13, 8, 0, 2,15,10, 4, 0,14,19,21,20,24 },
	{ 6,26,28,13, 8, 3, 5,15,10, 5, 1,14,19,21,20,24 },
	{ 7,27,28,13, 9, 1, 2,15,10, 4, 3,16,19,21,22,25 },
	{ 7,27,28,13, 9,30, 5,15,10, 5,30,16,19,21,22,25 },
	{ 7,27,28,13,11,31,32,17,11,31,32,17,19,21,22,25 }
};

/* patterns defining the dice sprites */
char dpatt[6][32] = {
		{0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x81, 
		 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF,
		 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x81,
		 0x81, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF},

		{0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
		 0x80, 0x80, 0x80, 0x80, 0x98, 0x98, 0x80, 0xFF,
		 0xFF, 0x01, 0x01, 0x19, 0x19, 0x01, 0x01, 0x01,
		 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF},
		
		{0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x81, 
		 0x81, 0x80, 0x80, 0x98, 0x98, 0x80, 0x80, 0xFF,
		 0xFF, 0x01, 0x01, 0x19, 0x19, 0x01, 0x01, 0x81,
		 0x81, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF},
		
		{0xFF, 0x80, 0x80, 0x98, 0x98, 0x80, 0x80, 0x80, 
		 0x80, 0x80, 0x80, 0x98, 0x98, 0x80, 0x80, 0xFF,
		 0xFF, 0x01, 0x01, 0x19, 0x19, 0x01, 0x01, 0x01,
		 0x01, 0x01, 0x01, 0x19, 0x19, 0x01, 0x01, 0xFF},
		
		{0xFF, 0x80, 0x80, 0x98, 0x98, 0x80, 0x80, 0x81, 
		 0x81, 0x80, 0x80, 0x98, 0x98, 0x80, 0x80, 0xFF,
		 0xFF, 0x01, 0x01, 0x19, 0x19, 0x01, 0x01, 0x81,
		 0x81, 0x01, 0x01, 0x19, 0x19, 0x01, 0x01, 0xFF},

		 {0xFF, 0x80, 0x80, 0x98, 0x98, 0x80, 0x80, 0x98, 
		  0x98, 0x80, 0x80, 0x98, 0x98, 0x80, 0x80, 0xFF,
		  0xFF, 0x01, 0x01, 0x19, 0x19, 0x01, 0x01, 0x19,
		  0x19, 0x01, 0x01, 0x19, 0x19, 0x01, 0x01, 0xFF}
};		

/* each die has:
**		value:	one less than face value of the die [0..5]
**		row:		Screen row of upper left corner
**		column:	Screen column of upper left corner
**		color;	Foreground color for the sprite
**		gridx:	x grid location for sprite
**		gridy:	y grid location for sprite
**		status:	ACTIVE = display as normal (die in play)
**						HELD = highlight in blue (die scores on this turn)
*/
struct die {
	int value;
	int row;
	int column;
	int color;
	int gridx;
	int gridy;
	int status;
} dice[NDICE] = {
	1, 5,  8, c_white,   0, 0, ACTIVE,
	2, 5, 13, c_ltyell,  0, 1, ACTIVE,
	3, 5, 18, c_cyan,    0, 2, ACTIVE,
	4, 5, 23, c_ltred,   0, 3, ACTIVE,
	5, 5, 28, c_magenta, 1, 0, ACTIVE
};

/* players */
struct p {
	char *name;
	int score;
	int inthegame;
	char greenmask;
	char redmask;
} player[MAXPLAYERS] = {
	"PLAYER1", 0, FALSE, 0x01, 0x02,
	"PLAYER2", 0, FALSE, 0x04, 0x08,
	"PLAYER3", 0, FALSE, 0x10, 0x20,
	"PLAYER4", 0, FALSE, 0x40, 0x80
};

/* current player */
static int current;

/* number of players */
static int nplayers;

unsigned *Ticptr = TICCNT;
unsigned timeout;
unsigned randseed;

/* wait - wait a specified number of seconds */
wait(seconds)
unsigned seconds;
{
	mswait(seconds*1000);
}

/* make a plink noise for a die */
plink()
{
	envshape(1);
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

/* itoarj - convert n to characters in s and right justify.
** l is the length of the string not including the terminating
** NUL. l must be at least 1.
*/
char *itoarj(n, s, l)
char s[];
int n, l;
{
	static int k;
	static char *p;

	if ((k = n) < 0)
		k = -k;
	p = s + l;
	/* first insert terminating NUL */
	*p-- = '\0';
	do {
		*p-- = k % 10 + '0';
	} while ((k /= 10) && (p >= s));
	if ((n < 0) && (p >= s))
		*p-- = '-';
	/* now pad with blank if needed */
	while (p >= s)
		*p-- = ' ';

	return (s);
}

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
	static char s[10];
	
	itoarj(v,s,5);
	puts(s);
}

showval(v, n, r, c, e)
int v, n, r, c, e;
{
	static char s[10];
	
	itoarj(v, s, n);
	showstr(s, r, c, e);
}

/* dumpdie -- (debugging) dump out a die data structure */
dumpdie(d)
int d;
{
	puts("Die [");
	putval(dice[d].value);
	puts("]: ");
	putval(dice[d].row);
	putchar(' ');
	putval(dice[d].column);
	putchar(' ');
	putval(dice[d].color);
	putchar(' ');
	putval(dice[d].gridx);
	putchar(' ');
	putval(dice[d].gridy);
	putchar(' ');
	putval(dice[d].status);
	putchar('\n');
}


/* yloc -- return true screen row (pixel
** coordinates) for the specified grid y 
*/
yloc(gy)
int gy;
{
	return DYMIN + gy*32;
}

/* xloc -- return true screen column (pixel
** coordinates) for the specified grid x
*/
xloc(gx)
int gx;
{
	return DXMIN + gx*32;
}

/* nrow -- return the number of sprites
** in a specified grid row. This is used to ensure
** that we never have more than four in a
** row, as the VDP can only display up to
** 4 sprites in a row.
*/
nrow(iy)
int iy;
{
	int ix, n;
	
	for (ix=0, n=0; ix<NGX; ix++) {
		if (grid[iy][ix] != EMPTY)
			++n;
	}

	return n;
}

/* movesprite -- move a sprite from (x0,y0) to
** (x1,y1) where x is column number (in pixel space
** [0..255]) and y is row number [0..191].
**
** the sprite is first moved along the x dimension (to
** the correct column) then to the right y posision (row).
*/
movesprite(s, x0, y0, x1, y1)
int s, x0, y0, x1, y1;
{
	static int x, y, dx, dy, sx, sy, e2, error;
	
	/* move one pixel at a time */
	dx = (x0 < x1) ? 1 : -1;
	dy = (y0 < y1) ? 1 : -1;
	
	x = x0;
	y = y0;

	positsprite(s, x, y);

	/* move to the right column... */
	while (x != x1) {
		x += dx;
		positsprite(s, x, y);
	}
	
	/* and the right row ... */
	while (y != y1) {
		y += dy;
		positsprite(s, x, y);
	}
}

/* initsprites -- create 5 sprites and 6 sprite
** patterns. update the corresponding dice data
** structure
*/
initsprites()
{
	int j;
	
	for (j=0; j<NDICE; j++)
		defsprite(j,  yloc(dice[j].gridy), xloc(dice[j].gridx),
				dice[j].color, 0, &dpatt[j][0]);
	
	/* leave them off for now... */
	lastsprite(0);

	/* create the sixth pattern separately */
	crpattern(5, &dpatt[5][0]);
	}

/* lastsprite -- specify where to stop
** processing sprites. 0 turns off all sprites
*/
lastsprite(s)
int s;
{
	int i;
	
	for (i=0; i<s; i++)
		/* restore y value to any that had been disabled */
		wrtvramdirect(SNTAB + i*4, yloc(dice[i].gridy));
	
	/* disable sprites beyond these 5 */
	wrtvramdirect(SNTAB + s*4, 0xD0);
}


/* initsound -- initialize the sound settings for
** the AY-3-8910
*/
initsound()
{
	noiseperiod(0);
	psgoptions(FALSE,FALSE,FALSE,TRUE,TRUE,TRUE);
	envenable('A');
	envenable('B');
	envenable('C');
	ecyclperiod(1500);
}

/* rndrange - return random integer in a range from
** min to max (inclusive).
*/
rndrange(min, max)
int min, max;
{
	return min + rnd(0)%(max-min+1);
}

/* initdice - set initial locations for dice sprites
** and initialize their face values to and
** velocities for dice when rolling. also
** initialize dice values to random numbers.
** note that sprites are only present for dice whose
** status is ACTIVE.
*/
initdice()
{
	int i, j, gy, gx;
	
	/* mark all cells as initially empty */
	for (i=0; i<NGX; i++) {
		for (j=0; j<NGY; j++) {
			grid[j][i] = EMPTY;
		}
	}

	for (j=0; j<NDICE; j++) {
		if (dice[j].status == ACTIVE) {
			/* find an available grid cell, making
			** sure to not put more than four
			** in the same row
			*/
			do {
				gx = rndrange(0, NGX-1);
				gy = rndrange(0, NGY-1);
			} while ((grid[gy][gx] != EMPTY) ||
					(nrow(gy) == 4));
		
			/* mark cell in use */
			grid[gy][gx] = j;

			/* set sprite object attributes */
			dice[j].gridx = gx;
			dice[j].gridy = gy;
			dice[j].value = rndrange(0,5);
			asgsprpattern(dice[j].value, j);
		}
	}
}

/* showdice - update dice imagery in screen[][] */
showdice()
{
	int j;
	
	clrdice();
	for (j=0; j<NDICE; j++)
		dispdie(j);
}

/* dispdie - display a die in its resting location
** on the screen. the status field indicates whether
** the die should be highlighted and offset to the
** "saved dice" row (currently 5 rows above the
** row for "active" dice).
*/
dispdie(n)
int n;
{
	int i, j, k, r, c, offset, iv;

	/* value [0..5] represents [1..6] for face value */
	iv = dice[n].value;
	offset = 0;
	r = dice[n].row;
	c = dice[n].column;
	
	/* dice being held are displayed a row above
	** the active dice row and with a different
	** pattern (color).
	*/
	if (dice[n].status == HELD) {
		r -= 5;
		offset = EOFFSET;
	}

	/* each die image is 4x4 pattern (32x32 pixels) so
	** loop over all 16. i loops over rows, j over columns
	*/
	k = 0;
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			screen[r+i][c+j] = dicepatt[iv][k++] + offset;
		}
	}
}

/* showstr - display a string at a specified
** row and column. string is clipped to stay
** in the displayable space. if e is TRUE then
** show text using emphasized color.
*/
showstr(s,r,c,e)
char *s;
int r,c,e;
{
	static int row, col, offset;
	static char ch;
	
	row = r;
	col = c;

	while ((ch=(*s++)) != '\0') {
		/* eliminate non-printable characters */
		if ((ch>=' ') && (ch<='\177')) {
			/* clip to legal screen coordinates */
			if ((row<24) && (col<32)) {
				/* have valid char and location */
				offset = ABASE + ch - ' ';
				screen[row][col] = e ? (offset + AOFFSET) : offset;
				++col;
			}
		}
	}
}

/* cls - clear screen */
cls()
{
	int row, col;

	/* fill screen with the null pattern */
	for (row=0; row<24; row++)
		for (col=0; col<32; col++)
			screen[row][col] = NULLPAT;
}

/* clrrow - clear a row */
clrrow(r)
int r;
{
	int col;
	
	for (col=0; col<32; col++)
		screen[r][col] = NULLPAT;
}

/* fillbox - clear an area */
fillbox(r0, c0, r1, c1, patt)
int r0, c0, r1, c1, patt;
{
	int row, col;
	
	for (row=r0; row<=r1; row++)
		for (col=c0; col<=c1; col++)
			screen[row][col] = patt & 0xFF;
}


/* rolldice -- generate new random face values
** and new grid locations for the dice, then
** move them there
*/
rolldice()
{
	int i, j, mw, gynew, gxnew, gyold, gxold;

	/* mw = ms to wait after each loop */
	mw = 8;
	for (i=0; i<5; i++) {
		for (j=0; j<NDICE; j++) {
			/* only "roll" active dice */
			if (dice[j].status == ACTIVE) {
				/* find an available grid cell, making
				** sure to not put more than four
				** in the same row
				**
				*/
				gxold = dice[j].gridx;
				gyold = dice[j].gridy;
		
				do {
					gxnew = rndrange(0, NGX-1);
					gynew = rndrange(0, NGY-1);
				} while ((grid[gynew][gxnew] != EMPTY) ||
						(nrow(gynew) == 4));
		
				/* mark new cell in use and free up the old one */
				grid[gynew][gxnew] = j;
				grid[gyold][gxold] = EMPTY;

				/* update sprite object location */
				dice[j].gridx = gxnew;
				dice[j].gridy = gynew;
		
				/* set a new random value for the die and
				** update the sprite pattern
				*/
				dice[j].value = rndrange(0,5);
				asgsprpattern(dice[j].value, j);
		
				/* now make the sprite actually move on the screen */
				positsprite(j,xloc(gxnew),yloc(gynew));
			
				/* make a sound */
				plink();
				/* wait and increase the wait each time */
				mswait(mw);
			}
		}
		/* gradually slow things down ... */
		mw *= 2;
	}
}


/* evaluate - look at the just-completed dice roll and
** evaluate the scoring. Change dice display status to
** indicate which ones scored.
** return score for this round.
*/
evaluate()
{
	int i, j, score, mvalue;
	static int count[6];
	
	/* count how many of each value - looking for 3 or more
	** of a kind. only check dice that were just rolled
	** (i.e. status == 0).
	*/
	for (i=0; i<6; i++) {
		count[i] = 0;
		for (j=0; j<NDICE; j++)
			if ((dice[j].status == ACTIVE) && (dice[j].value == i))
				++count[i];
	}
	
	/* now update the score and highlight scoring dice */
	score = 0;
	
	/* set mvalue to value for multiple dice */
	mvalue = 0;
	
	/* Process score based on how many of each value were rolled */
	for (i=0; i<6; i++) {
		switch (count[i]) {
			case 1:
			case 2:
				/* one or two of a kind, check for aces and fives */
				if (i==0)
					/* aces */
					score += 100 * count[i];
				else if (i==4)
					/* fives */
					score += 50 * count[i];
				break;
			case 3:
				/* 3 of a kind! */
				mvalue = i;
				if (i==0)
					/* aces */
					score += 1000;
				else
					/* else 100 times face value */
					score += 100*(i+1);
				break;
			case 4:
				/* 4 of a kind! */
				mvalue = i;
				if (i==0)
					/* aces */
					score += 2000;
				else
					/* else 200 times face value */
					score += 200*(i+1);
				break;
			case 5:
				/* 5 of a kind! */
				mvalue = i;
				score = WINSCORE;
				break;
		}
	}
	
	/* now change the status on any scoring dice */
	for (j=0; j<NDICE; j++) {
		/* look only at just-rolled dice */
		if (dice[j].status == ACTIVE) {
			/* flag any that scored */
			if ((dice[j].value == mvalue) ||
					(dice[j].value == 0) ||
					(dice[j].value == 4) )
				dice[j].status = SCORING;
		}
	}

	return score;
}

/* setcolors - set up pattern colors */
setcolors()
{
	int i;
	
	/* set up pattern colors*/
	for (i=0; i<32; i++) {
		if ((i>=0) && (i<=4))
			/* patterns 0..39 (40) */
			/* B&W - normal dice*/
			clrtab[i] = c_white + 16*c_black;
		else if ((i>=5) && (i<=9))
			/* patterns 40..79 (40) */
			/* Black on Lt. Blue - emphasized dice */
			clrtab[i] = c_ltblue + 16*c_black;
		else if ((i>=10) && (i<=14))
			/* patterns 80..119 (40) */
			/* Light Blue on Gray - held dice - depricated */
			clrtab[i] = c_gray + 16*c_ltblue;
		else if (i==15)
			/* patterns 120..127 (8) */
			/* white on transparent  */
			clrtab[i] = c_transp + 16*c_white;
		else if ((i>=16) && (i<=23))
			/* patterns 128..191 (64) */
			/* Black on Transparent - normal ASCII  */
			clrtab[i] = c_transp + 16*c_black;
		else
			/* patterns 192..255 (64) */
			/* White on Dark Red - emphasized ASCII  */
			clrtab[i] = c_dkred + 16*c_white;
	}
}

/* allheld - return TRUE if all dice are in HELD state
*/
allheld()
{
	int j, result;
	
	result = TRUE;
	for (j=0; j<NDICE; j++) {
		if (dice[j].status != HELD) {
			result = FALSE;
			break;
		}
	}
		
	return result;
}


/* allactive -- reset all dice to active mode */
allactive()
{
	int j;
	
	for (j=0; j<NDICE; j++)
			dice[j].status = ACTIVE;
}

/* promote -- promote all dice that scored on
** a given roll.  any die with status==SCORING
** gets promoted to HELD.
*/
promote()
{
	int j;
	
	for (j=0; j<NDICE; j++)
		if (dice[j].status == SCORING)
				dice[j].status = HELD;
}

/* rjs - read joystick */
rjs(c)
char c;
{
	int v, done;
	
	for (done=FALSE; !done; done = (v == rdpsgport(c))) {
		while ((v=rdpsgport(c)) == 0xFF)
			; /* wait for activity */

		/* now wait debounce time... */
		mswait(30);
	}

	while ((rdpsgport(c)) != 0xFF)
		; /* wait for button release */
	
	/* set bits to 1 for button selects */
	return (~v & 0xFF);
}

/* refresh - repaint the whole screen from RAM to VRAM */
refresh()
{
	blockwrite(screen,PNTAB,768);
}

/* showscores - display all of the scoring labels
** and show current game scores for all players.
** Highlight the score of the current player. The
** individual ROLL and TURN scores themselves
** are not displayed by this routine.
*/
showscores()
{
	int i;
	
	/* first place the static content into screen[][] */
	showstr("TURN", 0, 2, FALSE);
	showstr("SCORE:", 1, 1, FALSE);
	showstr("ROLL", 5, 2, FALSE);
	showstr("SCORE:", 6, 1, FALSE);
	showstr("GAME", 22, 2, FALSE);
	showstr("SCORE:", 23, 1, FALSE);
	for (i=0; i<nplayers; i++) {
		showstr("     ", 22, 9+6*i, (i == current));
		showval(i+1, 1, 22, 11+6*i, (i == current));
		if (player[i].inthegame)
			showval(player[i].score, 5, 23, 9+6*i, (i == current));
		else
			showstr("-----", 23, 9+6*i, (i == current));
	}
}


/* clrdice - erase the entire dice holding area */
clrdice()
{
	fillbox(0, 8, 8, 31, NULLPAT);
}

/* clractive - erase the ACTIVE dice holding area */
clractive()
{
	fillbox(5, 8, 8, 31, NULLPAT);
}

/* configscreen - do initial screen setup */
configscreen()
{
	/* Initialize VDP registers:
	**
	**	No External Videro
	**	16K RAM
	**	Disable display
	**	Pattern mode (Graphics I)
	**	S1 sprites (16x16)
	**	M1 Magnified sprites (32x32)
	*/
	vdpoptions(VPNEV+VP16K+VPDDP+VPPM+VPS1+VPM1);

	/* set up table addresses */
	pattgentable(PGTAB);
	colgentable(CGTAB);
	pattnametable(PNTAB);
	sprnametable(SNTAB);
	sprpatrntable(SPTAB);
	
	bordercolor(c_dkgrn);

	/* set up options again with display enabled */
	vdpoptions(VPNEV+VP16K+VPEDP+VPPM+VPS1+VPM1);

	/* clear the screen */
	cls();
	
	/* load Pattern Generator Table (VRAM) */
	
	/* standard dice patterns */
	blockwrite(ptrns,PGTAB,33*8);
	/* emphasized dice patterns */
	blockwrite(ptrns,PGTAB+(EOFFSET*8),33*8);

	/* blank pattern */
	blockwrite(nullpatt,PGTAB+(NULLPAT*8),8);
	/* solid pattern */
	blockwrite(solidpatt,PGTAB+(SOLIDPAT*8),8);

	/* standard ASCII (upper case only) */
	blockwrite(alpha,PGTAB+(ABASE*8),64*8);
	/* emphasized ASCII (upper case only) */
	blockwrite(alpha,PGTAB+((ABASE+AOFFSET)*8),64*8);

	/* set up pattern colors */
	setcolors();
	/* send color gen table to VRAM */
	blockwrite(clrtab,CGTAB,32);
}

/* flashwinners -- flash the scoring dice on this
** roll by alternetly changing their color
*/
flashwinners()
{
	int i, j;
	
	/* flash three times */
	for (i=0; i<3; i++) {
		
		/* first set all scoring dice to transparent */
		for (j=0; j<NDICE; j++)
			if (dice[j].status == SCORING)
				chgsprcolor(j, c_transp);
		mswait(100);
			
		/* set all scoring dice to blue */
		for (j=0; j<NDICE; j++)
			if (dice[j].status == SCORING)
				chgsprcolor(j, c_dkblue);
		mswait(100);
	}
}

/* hideinactive -- hide sprites that are no longer
** active (status != ACTIVE)
*/
hideinactive()
{
	int j;
	
	for (j=0; j<NDICE; j++)
		if (dice[j].status != ACTIVE) {
			/* move to the left edge of the screen and
			** make them transparent (invisible)
			*/
			chgsprcolor(j, c_transp);
			positsprite(j,0,32*j);
		}
}

/* resetsprites -- reset sprite colors */
resetsprites()
{
	int j;
	
	for (j=0; j<NDICE; j++)
		chgsprcolor(j, dice[j].color);
}

/* movewinners -- animate the movement of
** the sprites for the winning dice back to the home
** locations of their permanent dice displays. Do this
** in reverse order so that one-by-one the sprites can
** be disabled and the permanent dice can be displayed.
*/
movewinners()
{
	int j;
	
	for (j=NDICE-1; j>=0; j--) {
		if (dice[j].status != ACTIVE) {
			/* compute final destination in pixel coordinates.
			** the permanent dice are displayed using pattern
			** position numbers (col: 0..31; row: 0..23) but
			** the sprite positions are done using pixel
			** coordinates. also any "held" dice are displayed
			** five positions up from the "active" ones.
			*/
			/* DEBUGGING */
			dumpdie(j);
			movesprite(j,
				xloc(dice[j].gridx),
				yloc(dice[j].gridy), 
				8 * (dice[j].column),
				8 * (dice[j].row - 5) 
			);
		
			/* now turn off the sprite and turn on the permanent
			** version.
			*/
			lastsprite(j);
			dispdie(j);
			refresh();
		}
	}
}

main()
{
	int rollscore, turnscore, gameover, turnover;
	int maycontinue, button;

	configscreen();
	initsprites();
	initsound();

	/* should prompt for number of players here... */
	nplayers = 2;
	
	/* now run the dice game */
	randseed = *Ticptr;
	rnd(randseed);
	
	gameover = FALSE;
	/* start with player 1 */
	/* optional: could roll dice to see who goes first... */
	current = 0;
	turnscore = 0;
	maycontinue = FALSE;
	while (!gameover) {
		/* Each cycle through this loop is another turn. Each
		** turn moves to the next player, whose name is
		** displayed at the top
		*/
		turnover = FALSE;
		/* optional: display player's name at lower left */
		/* showstr(player[current].name, 1, MSGROW, TRUE); */
		showscores();
		/* clear previous roll and turn scores and message */
		showstr("     ", 8, 1, FALSE);
		/* leave turn score in case continuing */
		if (!maycontinue)
			showstr("     ", 3, 1, FALSE);
		clrrow(MSGROW);
		
		/*     ***  Beginning New Turn  ***
		**
		** two options here: if eligible, player may build
		** upon the previous roll. Prompt them for their choice.
		** Otherwise clear all the dice and start with a fresh roll.
		*/
		if (maycontinue) {
			/* come here if player has the option of continuing
			** to build on the previous roll.
			*/
			showstr("GREEN:", 11, 1, FALSE);
			showstr("CONT", 12, 1, FALSE);
			showstr("RED:", 15, 1, FALSE);
			showstr("NEW ", 16, 1, FALSE);
			showdice();
			refresh();
			/* wait for button press. only accept presses by the
			** currently rolling player.
			*/
			wrtpsgport('B',~(player[current].greenmask | player[current].redmask));
			do {
				button = rjs('A');
			} while ((button & (player[current].greenmask | player[current].redmask)) == 0);
			if (button & player[current].redmask) {
				/* RED button pressed - start fresh */
				clrdice();
				allactive();
				turnscore = 0;
			}
			wrtpsgport('B',0xFF);
			refresh();
		}
		else {
			/* beginning of new turn, but no option to build on
			** previous roll. mark all dice as ACTIVE, reset turn
			** score to zero and wait for the roll button
			*/
			allactive();
			initdice();
			showdice();
			turnscore = 0;
			/* prompt the player to roll */
			wrtpsgport('B',~player[current].greenmask);
			showstr("GREEN:", 11, 1, FALSE);
			showstr("ROLL", 12, 1, FALSE);
			showstr("    ", 15, 1, FALSE);
			showstr("    ", 16, 1, FALSE);
			/* paint the screen, wait for an answer */
			refresh();
			while ((rjs('A') & player[current].greenmask) == 0)
				; /* wait for current player to press GREEN button */
			wrtpsgport('B',0xFF);
		}
		
		/* now just loop through rolls of the dice until the
		** current player's turn is over...
		*/
		while (!turnover ) {
			/* Each pass through this loop is a roll of the dice.
			/* Roll the dice, evaluate the outcome and flag dice to
			** highlight, update the display in RAM and then  write
			** from RAM to VRAM to display it, then pause while
			** player examines the results and prompt them for
			** next action.
			*/
			
			/* ready the table for rolling. first keep the HELD dice
			** displayed but clear the others to make room for rolling.
			** enable all sprites but hide the non ACTIVE ones by
			** making them transparent and positioning them to the side,
			** then refresh the display
			*/
			clractive();
			lastsprite(5);
			hideinactive();
			refresh();
			
			/* roll the dice!  */
			rolldice();

			/* after the roll assess which dice are scoring dice and
			** what the total roll score is, then flash the sprites
			** for the winning dice, promote them and have the sprites
			** move in an animated fashion to the location for HELD dice.
			** then update the roll and turn scores, turn off all sprites 
			** update the ACTIVE dice display and refresh the screen.
			*/
			rollscore = evaluate();
			flashwinners();
			promote();
			movewinners();
			lastsprite(0);
			showval(rollscore, 5, 8, 1, FALSE);
			showval(turnscore, 5, 3, 1, FALSE);
			showdice();
			refresh();
		
			/* now analyze options and interact with the player to
			** determine what's next (or if turn is over then position
			** things for next player...
			*/
			
			/* first check for any special conditions: player went "bust";
			** player hit a "grand slam" (5-of-a-kind on one roll), player
			** earned too many points (went over WINSCORE), or player
			** used up all 5 dice.
			*/
			if (rollscore == 0) {
				/* no points scored this roll. the player's turn
				** is over and any points accumulated on this turn
				** are lost.
				*/
				turnover = TRUE;
				turnscore = 0;
				clrrow(MSGROW);
				showstr("BUST!", MSGROW, 4, TRUE);
				showval(turnscore, 5, 3, 1, FALSE);
			} else if (rollscore == WINSCORE) {
				/* any player who rolls WINSCORE on a single roll
				** becomes an Instant Winner!
				*/
				turnscore = WINSCORE;
				turnover = TRUE;
				clrrow(MSGROW);
				showstr("5 OF A KIND - INSTANT WIN!", MSGROW, 4, TRUE);
			} else {
				/* points were earned on this roll but must first
				** check if the turn score would put the player
				** over WINSCORE.  this is not allowed (must roll
				** the exact amount to win) in which case points on
				** this turn would be forefeited and the turn is over
				*/
				turnscore += rollscore;
				if (player[current].score+turnscore > WINSCORE) {
					/* too many points. must win by exact count, turn
					** is over and lose all points
					*/
					turnover = TRUE;
					turnscore = 0;
					clrrow(MSGROW);
					showstr("NEED EXACTLY 10,000 TO WIN", MSGROW, 4, TRUE);
				} else if (allheld()) {
					/* all dice have been used up. the rules say the player
					** must continue for at least one more roll.  need to reset
					** all dice to ACTIVE status and prompt the player for
					** another roll
					*/
					clrrow(MSGROW);
					showstr("ALL DICE USED", MSGROW, 4, TRUE);
				} else {
					/* if none of the above special conditions are met
					** then the player is still alive on this turn.still alive. wait for player to decide if they
					** take the money (keep) or continue the round
					** (enter).
					*/
					clrrow(MSGROW);
				}
			}
			refresh();
			
			/* Roll is over, check if turn is over or continuing... */
			if (!turnover) {
				/* turn not yet over. player either used up all the dice
				** or has to decide to keep or continue
				*/
				showval(turnscore, 5, 3, 1, FALSE);
				if (allheld() || ((player[current].score == 0) && (turnscore < 500))) {
					/* if all five dice are scoring dice or not yet in the game
					** the player must keep rolling
					*/
					showstr("GREEN:", 11, 1, FALSE);
					showstr("ROLL", 12, 1, FALSE);
					showstr("    ", 15, 1, FALSE);
					showstr("    ", 16, 1, FALSE);
					/* paint the screen and wait for an answer */
					if (allheld()) {
						allactive();
						resetsprites();
					}
					showdice();
					refresh();
					wrtpsgport('B',~player[current].greenmask);
					while ((rjs('A') & player[current].greenmask) == 0)
						; /* wait for current player to press GREEN button */
					wrtpsgport('B',0xFF);
				}
				else {
					/* player has enough points to get into the game, or is 
					** already in the game.  see if they want to continue
					*/
					showstr("GREEN:", 11, 1, FALSE);
					showstr("ROLL", 12, 1, FALSE);
					showstr("RED:", 15, 1, FALSE);
					showstr("KEEP", 16, 1, FALSE);

					/* paint the screen and wait for an answer */
					showdice();
					refresh();
					/* wait for button press. only accept presses by the
					** currently rolling player
					*/
					wrtpsgport('B',~(player[current].greenmask | player[current].redmask));
					do {
						button = rjs('A');
					} while ((button & (player[current].greenmask | player[current].redmask)) == 0);
					wrtpsgport('B',0xFF);
					if (button & player[current].redmask) {
						/* RED button pressed - end turn */
						turnover = TRUE;
					}
				}
			} else {
				/* turn is over - clean up */
				resetsprites();
			}
		}
		
		/* the player's turn is now over. perform final processing
		** and prepare for next player's turn
		*/
		
		/* update player scoring */
		if (turnscore == WINSCORE) {
			/* five-of-a-kind was rolled this turn!
			** this instantly puts you "in the game"
			** and the game will be over with a
			** winning score of WINSCORE. Zero their
			** score here since we add it back below.
			*/
			player[current].inthegame = TRUE;
			player[current].score = 0;
		}
		else if ((!player[current].inthegame) && (turnscore >= 500)){
			player[current].inthegame = TRUE;
			/* print message here? "you're in the game now" ? */
		}

		/* add in points but only if "in the game" */
		if (player[current].inthegame)
			player[current].score += turnscore;
			
		showscores();
		/* once WINSCORE has been achieved the game is over */
		if (player[current].score == WINSCORE)	{
			clrrow(MSGROW);
			showstr("GAME OVER!  ", MSGROW, 4, TRUE);
			refresh();
			gameover = TRUE;
		} else {
			/* prepare for next player's turn */
			
			/* Next player is up... */
			if (++current == nplayers)
				current = 0;
			
			/* if there are scored points the next player may
			** opt to keep them and continue rolling, but only
			** if they're already in the game and the points
			** wouldn't put them over the top.
			*/
			if ((turnscore > 0) && player[current].inthegame &&
					(player[current].score + turnscore < WINSCORE))
				maycontinue = TRUE;
			else
				maycontinue = FALSE;

		/* indicate turn is over and pause ... */
		clrrow(MSGROW);
		showstr("TURN OVER!  ", MSGROW, 4, TRUE);
		refresh();
		wait(2);
		}
	}
}
