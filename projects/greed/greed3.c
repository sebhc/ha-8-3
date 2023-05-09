/* greed - plays the dice game of GREED.
**
**
** link command:
** 	l80 greed,gutil,rnd,ha83,pio,clibrary/s,greed/n/e/m
**
** Glenn Roberts
**	16 July 2022
**
*/

/* globals live here */
#define EXTERN

/* stuctures and values initated here */
#define INIT

#include "ha83.h"
#include "gutil.h"
#include "patterns.h"

/* 32-entry color table */
char clrtab[32];

struct p player[MAXPLAYERS] = {
	"PLAYER1", 0, FALSE, 0x01, 0x02,
	"PLAYER2", 0, FALSE, 0x04, 0x08,
	"PLAYER3", 0, FALSE, 0x10, 0x20,
	"PLAYER4", 0, FALSE, 0x40, 0x80
};

/* define sprite dice for main screen */
struct sdie {
	int value;	/* 0-5 == 1 to 6 on the die */
	int x;			/* column (pixel coordinates) */
	int y;			/* row (pixel coordinates) */
	int dy;			/* speed */
	int spcolor;
} sdice[32];

/* table of session scores */
int sscores [5][4];

/* current player */
int current;

/* game counter */
int game;

/* number of players */
int nplayers;

/* scores for current roll and turn */
int rollscore, turnscore;

/* booleand to track state of game */
int gameover, turnover, sessover, maycontinue;

/* seed for random number generator */
unsigned randseed;



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

	/* create the six sprite patterns */
	for (j=0; j<6; j++)
		crpattern(j, &dpatt[j][0]);

	/* create the five dice sprites. initially link
	** them to their corresponding patterns
	*/
	for (j=0; j<NDICE; j++) {
		crsprite(j,  yloc(dice[j].gridy), xloc(dice[j].gridx),
				dice[j].color, 0);
		asgsprpattern(j, j);
	}

	/* leave them off for now... */
	lastsprite(0);
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
/*			dice[j].value = rndrange(0,5); */
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

/* dispgreed -- display the 5 "GREED" dice on the main screen */
dispgreed()
{
	int i, j, k, r, c, d;
	
	/* each die image is 4x4 pattern (32x32 pixels) so
	** loop over all 16. i loops over rows, j over columns
	*/
	r = 10;
	for (d=0; d<5 ; d++) {
		c = 4 + 5*d;
		k = 0;
		for (i=0; i<4; i++) {
			for (j=0; j<4; j++) {
				screen[r+i][c+j] = grpatt[d][k++] + GREEDBASE;
			}
		}
	}
}

/* rolldice -- generate new random face values
** and new grid locations for the dice, then
** move them there
*/
rolldice()
{
	int i, j, mw, gynew, gxnew, gyold, gxold;

	/* set up noise parameters for dice rolling sound */
	soundmode(0);
	
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
				envshape(1);

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

/* color1 - set up pattern colors for main intro screen */
color1()
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
			/* Dice with the word "GREED" in them */
			/* Black on Dk. Blue */
			clrtab[i] = c_white + 16*c_dkgrn;
		else if (i==15)
			/* patterns 120..127 (8) */
			/* white on transparent  */
			clrtab[i] = c_transp + 16*c_white;
		else if ((i>=16) && (i<=23))
			/* patterns 128..191 (64) */
			/* White on Transparent - normal ASCII  */
			clrtab[i] = c_transp + 16*c_white;
		else
			/* patterns 192..255 (64) */
			/* White on Dark Red - emphasized ASCII  */
			clrtab[i] = c_dkred + 16*c_white;
	}
}

/* color2 - set up pattern colors for main game*/
color2()
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
			/* Dice with the word "GREED" in them */
			/* Black on Dk. Blue */
			clrtab[i] = c_white + 16*c_black;
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
	showstr("PLAYER:", 22, 0, FALSE);
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

/* cfggame - screen setup for game play */
cfggame()
{
	/* Initialize VDP registers:
	**
	**	No External Video
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
	blockwrite(dptrns,PGTAB,33*8);
	/* emphasized dice patterns */
	blockwrite(dptrns,PGTAB+(EOFFSET*8),33*8);
	/* dice with "GREED" in them */
	blockwrite(gptrns,PGTAB+(GREEDBASE*8),24*8);

	/* blank pattern */
	blockwrite(nullpatt,PGTAB+(NULLPAT*8),8);
	/* solid pattern */
	blockwrite(solidpatt,PGTAB+(SOLIDPAT*8),8);

	/* standard ASCII (upper case only) */
	blockwrite(alpha,PGTAB+(ABASE*8),64*8);
	/* emphasized ASCII (upper case only) */
	blockwrite(alpha,PGTAB+((ABASE+AOFFSET)*8),64*8);

	/* set up pattern colors */
	color2();
	/* send color gen table to VRAM */
	blockwrite(clrtab,CGTAB,32);
}

/* cfgmain - screen setup for main intro screen */
cfgmain()
{
	/* Initialize VDP registers:
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
	
	bordercolor(c_black);

	/* set up options again with display enabled */
	vdpoptions(VPNEV+VP16K+VPEDP+VPPM);

	/* clear the screen */
	cls();
	
	/* load Pattern Generator Table (VRAM) */
	
	/* standard dice patterns */
	blockwrite(dptrns,PGTAB,33*8);
	/* emphasized dice patterns */
	blockwrite(dptrns,PGTAB+(EOFFSET*8),33*8);
	/* dice with "GREED" in them */
	blockwrite(gptrns,PGTAB+(GREEDBASE*8),24*8);

	/* blank pattern */
	blockwrite(nullpatt,PGTAB+(NULLPAT*8),8);
	/* solid pattern */
	blockwrite(solidpatt,PGTAB+(SOLIDPAT*8),8);

	/* standard ASCII (upper case only) */
	blockwrite(alpha,PGTAB+(ABASE*8),64*8);
	/* emphasized ASCII (upper case only) */
	blockwrite(alpha,PGTAB+((ABASE+AOFFSET)*8),64*8);

	/* set up pattern colors */
	color1();
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
			/* dumpdie(j); */
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

/* soundmode -- the AY-3-8910 can be configured in
** different ways depending on the sound desired.
** sound configurations are all controlled here.
*/
soundmode(m)
int m;
{
	switch (m) {
		case 0:
		/* noise parameters for dice rolling sound */
		noiseperiod(0);
		psgoptions(FALSE,FALSE,FALSE,TRUE,TRUE,TRUE);
		envenable('A');
		envenable('B');
		envenable('C');
		ecyclperiod(1500);
		break;
		
		case 1:
		/* sound parameters for single channel tones, e.g.
		** joystick "beeps". period must be specified
		** and then calling envshape() will trigger
		** the sound play
		*/
		noiseperiod(0);
		psgoptions(TRUE,FALSE,FALSE,FALSE,FALSE,FALSE);
		toneperiod('A', 250);
		envenable('A');
		chanampl('B', 0);
		chanampl('C', 0);
		ecyclperiod(4000);
		break;
		
		case 2:
		/* sound parameters for 3-note chords */
		noiseperiod(0);
		psgoptions(TRUE,TRUE,TRUE,FALSE,FALSE,FALSE);
		/* default to C major */
		toneperiod('A', 428);
		toneperiod('B', 339);
		toneperiod('C', 285);
		envenable('A');
		envenable('B');
		envenable('C');
		ecyclperiod(10000);
		break;	
	}
}

greenblip(w)
int w;
{
	toneperiod('A', 250);
	envshape(1);
	if (w > 0) {
		mswait(w);
	}
}

redblip(w)
int w;
{
	toneperiod('A', 500);
	envshape(1);
	if (w > 0) {
	/* env period of 4000; t=4000/6991=.5722 sec. */
		mswait(w);
	}
}

chord()
{
	soundmode(2);
	envshape(1);
	/* env period of 10000; t=10000/6991=1.4304 sec. */
	mswait(715);
	mswait(715);
}

/* newturn - begin a new turn. two options here:
** if eligible, player may build upon the previous
** roll. Prompt them for their choice. Otherwise
** clear all the dice and start with a fresh roll.
*/
newturn()
{
	int button;
	
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
			redblip(572);
			clrdice();
			allactive();
			turnscore = 0;
		} else {
			greenblip(572);
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
		greenblip(572);
		wrtpsgport('B',0xFF);
	}
}

/* dorolls -- loop through rolls of the dice until the
** current player's turn is over...
*/
dorolls()
{
	int button;
	
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
		
		/* set sound mode for beeps */
		soundmode(1);

		/* after the roll assess which dice are scoring dice and
		** what the total roll score is, then flash the sprites
		** for the winning dice, promote them and have the sprites
		** move in an animated fashion to the location for HELD dice.
		** then update the roll and turn scores, reset sprites to
		** their correct colors, turn off all sprite display,
		** update the ACTIVE dice display and refresh the screen.
		*/
		rollscore = evaluate();
		flashwinners();
		promote();
		movewinners();
		resetsprites();
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
			wait(1);
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
				wait(1);
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

				showdice();
				refresh();
				wrtpsgport('B',~player[current].greenmask);
				while ((rjs('A') & player[current].greenmask) == 0)
					; /* wait for current player to press GREEN button */
				greenblip(572);
				wrtpsgport('B',0xFF);
				/* now demote the HELD dice back to ACTIVE status 
				** and prepare the sprites for next animation.
				*/
				if (allheld()) {
					allactive();
					resetsprites();
					showdice();
				}
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
					redblip(572);
					turnover = TRUE;
				} else {
					greenblip(572);
				}
			}
		} else {
			/* turn is over - clean up */
			resetsprites();
		}
	}
}

/* endturn -- perform final processing and prepare for 
** the next player's turn
*/
endturn()
{
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
	showstr("TURN OVER!  ", MSGROW, 12, TRUE);
	refresh();
	wait(1);
	}
}

/* onegame - play a single game. a tournament can
** consist of multiple game plays
*/
onegame()
{
	gameover = FALSE;
	/* start with player 1 (current = 0) */
	/* optional: could roll dice to see who goes first... */
	current = 0;
	turnscore = 0;
	maycontinue = FALSE;

	/* Loop through turns by each player until the
	** gameover flag is set, indicating someone won.
	*/
	while (!gameover) {
		turnover = FALSE;
		/* possibly display current player's name on the
		** screen here...
		*/
		
		/* update the scoring display. clear previous roll
		** score but if continuation is a possibility then
		** leave the current turn score displayed (as the
		** player will be building on that...)
		*/
		showscores();
		showstr("     ", 8, 1, FALSE);
		if (!maycontinue)
			showstr("     ", 3, 1, FALSE);
		clrrow(MSGROW);
		
		/* Each turn has three phases: */
		
		/* 1) begin new turn */
		newturn();
	
		/* 2) loop through rolls of the dice until the
		** current player's turn is over...
		*/
		dorolls();
	
		/* 3) when turn is over perform final processing
		** and prepare for next player
		*/
		endturn();
	}
	wait(2);
}

/* randdie - used with the main screen sprites */
randdie(i)
int i;
{
	sdice[i].x = rndrange(0,31)*8;
	sdice[i].y = rndrange(0,23)*8;
	sdice[i].dy = rndrange(2,4);
	sdice[i].value = rndrange(0,5);
	sdice[i].color = rndrange(c_midgrn, c_white);
}

/* protected - return TRUE if sprite i is located within
** the bounds of the protected screen region
*/
protected(i)
int i;
{
	int col, row;
	
	col = sdice[i].x;
	row = sdice[i].y;
	
	return ((row >= 72) && (row <=128) &&
			(col >=24) && (col <=224));
}

/* mainscreen -- display main screen. this is
** the first screen presented when the program
** begins.
*/
mainscreen()
{
	int done, i;
	
	/* set up small sprites */
	/* create the six sprite patterns */
	for (i=0; i<6; i++)
		crpattern(i, &spatt[i][0]);

	/* create 32 sprites at random locations with random
	** sprite patterns
	*/
	for (i=0; i<32; i++) {
		/* select random die characteristics and create
		** the associated sprite, then assign it the
		** corresponding sprite pattern...
		*/
		randdie(i);
		crsprite(i, sdice[i].x, sdice[i].y, sdice[i].spcolor, 0);
		asgsprpattern(sdice[i].value, i);
	}
	
	cls();
	dispgreed();
	showstr("PRESS ANY BUTTON TO START", 15, 4, FALSE);
	refresh();
	
	/* turn on all the button LEDs */
	wrtpsgport('B',0x00);
	
	/* now display the sprites while waiting for the joystick
	** button to be pressed 
	*/
	do {
		/* now move the sprites down the screen and
		** mix it up when they hit the bottom
		*/
		for (i=0; i<32; i++) {
			sdice[i].y += sdice[i].dy;
			/* if over protected area make sprite transparent */
			chgsprcolor(i, protected(i) ? c_transp : sdice[i].spcolor);
			if (sdice[i].y > 183) {
				/* at the bottom of the screen. randomize the
				** die again, move it to the top and update
				** the sprite pattern and color
				*/
				randdie(i);
				sdice[i].y = 0;
				asgsprpattern(sdice[i].value, i);
				chgsprcolor(i, sdice[i].spcolor);
			}
			/* now move it ... */
			positsprite(i, sdice[i].x, sdice[i].y);
		}
	} while (jsidle('A'));
	
	/* play a chord */
	chord();

	/* go back to single tone mode */
	soundmode(1);
	
	
	/* now find out how many players */
	cls();
	nplayers = 2;
	showstr("HOW MANY PLAYERS?", 10, 7, FALSE);
	showstr("GREEN BUTTON: CHANGE", 11, 6, FALSE);
	showstr("RED BUTTON: ACCEPT", 12, 7, FALSE);
	refresh();
	done = FALSE;
	while (!done) {
		showval(nplayers, 1, 14, 15, TRUE);
		refresh();
		if (rjs('A') & 0x55) {
			/* a green button was hit */
			greenblip(0);
			if (++nplayers > 4)
				nplayers = 1;
		}
		else {
			redblip(0);
			done = TRUE;
		}
	}
	wrtpsgport('B',0xFF);
	cls();
	refresh();
}

/* message - print a message on the screen */
message(s, p)
char *s;
int p;
{
	clrrow(MSGROW);
	showstr(s, MSGROW, 4, TRUE);
}

/* newsession - show cumulative game scores and ask if players
** want to play another session
*/
newsession()
{
	int i, j;
	
	/* first record each player's score in the table
	** and reset their score and game status in preparation
	** for the next game.
	*/
	for (i=0; i<nplayers; i++) {
		sscores[game][i] = player[i].score;
		player[i].score = 0;
		player[i].inthegame = FALSE;
	}
	
	++game;
	cls();
	showstr("SESSION SCORES:", 3, 9, FALSE);
	showstr("PLAYER:", 6, 0, FALSE);
	for (i=0; i<nplayers; i++) {
		showval(i+1, 1, 6, 10+6*i, FALSE);
		showstr("_____", 7, 8+6*i, FALSE);
		for (j=0; j<game; j++) {
			showval(sscores[j][i], 5, 9+j*2, 8+6*i, 
				(sscores[j][i] == WINSCORE) ? TRUE : FALSE);
		}
	}
	for (j=0; j<game; j++) {
		showstr("GAME", 9+j*2, 0, FALSE);
		showval(j+1, 1, 9+j*2, 4, FALSE);
	}


	/* all LEDs on */
	wrtpsgport('B',0x00);

	/* enforce five game limit per session */
	if (game == 5) {
		showstr("SESSION OVER!", 15, 9, TRUE);
		showstr("PRESS ANY BUTTON TO EXIT", 16, 4, TRUE);
		refresh();
		rjs('A');
		sessover = TRUE;
	} else {
		showstr("PLAY ANOTHER ROUND?", 19, 6, FALSE);
		showstr("GREEN: YES, RED: NO", 20, 6, FALSE);
		refresh();
		if (rjs('A') & 0xAA) {
			/* a red button was hit */
			redblip(572);
			sessover = TRUE;
		} else {
			greenblip(572);
		}
	}
	/* all LEDs off */
	wrtpsgport('B',0xFF);
	
	if (sessover) {
		cls();
		showstr("THANKS FOR PLAYING!", 10, 6, TRUE);
		refresh();
		wait(4);
	}
	cls();
	refresh();
}


main()
{
	/* initial sound mode is for chords */
	soundmode(2);
	
	cfgmain();
	mainscreen();

	cfggame();
	initsprites();
	/* now run the dice game */
	randseed = *Ticptr;
	rnd(randseed);
	
	/* multiple sessions of play */
	sessover = FALSE;
	game = 0;
	while (!sessover) {
		onegame();
		newsession();
	}
	
	/* write anything to port 66Q to reset HA-8-3 */
	outp(066, 0);
}
