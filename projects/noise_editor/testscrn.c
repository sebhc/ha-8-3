/* testscrn - test displaying screen created by SKETCH */

#include "printf.h"

#define	ON	1
#define	OFF	0

#define	TRUE	1
#define	FALSE	0

#define	NUL		'\000'
#define CTLC	'\003'
#define	BELL	'\007'
#define	CR		'\015'
#define	ESC		'\033'

/* single code escape sequences - See H19 Operation Manual
** for complete list
*/
#define	 HCD	'E'		/* Clear Display */
#define	 HEL	'l'		/* Erase line */
#define	 HSM	'x'		/* Set mode */
#define	 HDCA	'Y'		/* Direct Cursor Addressing */
#define	 HRAM	'z'		/* Reset to power-up */
#define	 HSCP	'j'		/* save cursor position */
#define	 HRCP	'k'		/* go to saved position */

#define CONSOLE 0350	/* console serial port	*/

/* external screen data */
extern char screen[];

char bullet[] = "\033F\136\033G";

/* data structure for bullet list */
struct blist {
	int value;
	int	r;
	int	c;
	char *onval;
	char *offval;
};

/* data structure for integer list */
struct ilist {
	int value;
	int r;
	int c;
	char *fmt;
};

/* channel period selectors
** default period is middle c
*/
struct ilist cperiod[] = {
	0654, 10, 22, "%4d",
	0654, 10, 31, "%4d",
	0654, 10, 40, "%4d"
};

/* channel amplitude selectors */
struct ilist camp[] = {
	15, 12, 23, "%3d",
	15, 12, 32, "%3d",
	15, 12, 41, "%3d"
};

/* noise period selector */
struct ilist nperiod[] = {
	017, 15, 18, "%2d"
};

/* envelope period selector */
struct ilist eperiod[] = {
	14332, 4, 68, "%5d"
};

/* Channel noise selectors */
struct blist cnoise[] = {
	OFF, 8, 24, bullet, " ",
	OFF, 8, 33, bullet, " ",
	OFF, 8, 42, bullet, " "
};

/* Channel tone selectors */
struct blist ctone[] = {
	OFF, 9, 24, bullet, " ",
	OFF, 9, 33, bullet, " ",
	OFF, 9, 42, bullet, " "
};

/* Envelope shape selector - this is a radio
** button interface. one and only one must
** be selected.
*/
struct blist eshape[] = {
	OFF,  8, 50, bullet, " ",
	ON,  10, 50, bullet, " ",
	OFF, 12, 50, bullet, " ",
	OFF, 14, 50, bullet, " ",
	OFF, 16, 50, bullet, " ",
	OFF, 18, 50, bullet, " ",
	OFF, 20, 50, bullet, " ",
	OFF, 22, 50, bullet, " "
};


/* showb - show bullet list. save cursor position
** and restore it before leaving.
*/
showb(l, n)
struct blist l[];
int n;
{
	int i;

	/* save cursor position */
	scr(HSCP);
	
	for (i=0; i<n; i++) {
		gotorc(l[i].r, l[i].c);
		puts( (l[i].value) ? l[i].onval : l[i].offval);
	}
	
	/* restore cursor position */
	scr(HRCP);
}

/* showi - show integer list. save cursor position
** and restore it before leaving. Negative values
** cause the "msg" to be displayed instead of the
** value.
*/
showi(l, n, msg)
struct ilist l[];
int n;
char *msg;
{
	int i;

	/* save cursor position */
	scr(HSCP);
	
	for (i=0; i<n; i++) {
		gotorc(l[i].r, l[i].c);
		if (l[i].value >=0)
			printf(l[i].fmt, l[i].value);
		else
			puts(msg);
	}
	
	/* restore cursor position */
	scr(HRCP);
}

/* showall - update values to the screen from
** all of the display lists.
*/
showall()
{
	showb(cnoise, 3);
	showb(ctone, 3);
	showb(eshape, 8);
	showi(cperiod, 3, "");
	/* amplitude < 0 indicates Envelope control */
	showi(camp, 3, "ENV");
	showi(nperiod, 1, "");
	showi(eperiod, 1, "");
}


/* scr - send an escape command to the screen */
scr(c)
char c;
{
	putchar(ESC);
	putchar(c);
}

/* gotorc - position cursor to row and column
** row and column start at upper left corner
** upper left corner is 1,1
*/
gotorc(row,col)
int row,col;
{
	scr(HDCA);
	putchar(' ' - 1 + row);
	putchar(' ' - 1 + col);
}

/* ena25 - enable the 25th line */
ena25()
{
	/* set mode... */
	scr(HSM);
	putchar('1');
}

/* DIRECT console output commands may not be
** needed - only use direct port I/O for
** input...
*/

/* conout - output a character directly 
** to the console serial port
*/
conout(c)
char c;
{
	/* Wait for Transmit Hold Register empty */
	while ((inp(CONSOLE+5) & 0x20) == 0)
		/* wait ... */;
	/* Then transmit the character */
	outp(CONSOLE,c);
}

/* out_str - output a string directly to the console port
*/
out_str(s)
char *s;
{
	while (*s != NUL)
		conout(*s++);
}

/* conin - return a character directly
** from the console port, return NUL if none available
*/
conin()
{
	/* check for Data Ready */
	if ((inp(CONSOLE+5) & 0x01) == 0)
		/* return -1 if no data available */
		return 0;
	else
		/* read the character from the port and return it */
		return inp(CONSOLE);
}

/* copen - initialize console */
copen()
{
	/* disable console serial port interrupts
	** so that we can talk directly to the console
	** serial port (HDOS version)
	*/
	outp(CONSOLE+1,0);
}

/* cclose - close out settings on console */
cclose()
{
	/* re-enable console serial port interrupts */
	outp(CONSOLE+1,0x01);	
}

/* output a string */
puts(s)
char *s;
{
	while (*s)
		putchar(*s++);
}

/* toupper - convert character to upper case */
toupper(c) {
	if (c >= 'a' && c <= 'z')
		return (c - 0x20);
	return (c);
}

/* input a string (with echo) */
gets(s)
char *s;
{
	int done;
	char c;

	/* ensure null string if CR is hit */
	*s = NUL;
	for (done=FALSE; !done; ) {
		while ((c = conin()) == 0)
			/* wait for key press */;
		if (c != CR) {
			putchar(c); /* echo it ... */
			*s++ = c;
		} else
			done = TRUE;
	}
	
	/* terminate the string */
	*s = NUL;
}

/* atoi - convert string to integer */
atoi(s)
char *s;
{
	static int n, sign;
	sign = 1;
	n = 0;
	switch (*s) {
		case '-': sign = -1;
		case '+': ++s;
		}
	while (*s >= '0' && *s <= '9') n = 10 * n + *s++ - '0';
	return(sign * n);
}

getval()
{
	static char lvalue[20];

	gets(lvalue);
	return atoi(lvalue);
}

/* playsound - play the sound based on current values */
playsound()
{
	int i;
	
	noiseperiod(nperiod[0].value);
	
	/* mixer settings - turn tones, noise channels on/off */
	psgoptions(ctone[0].value, ctone[1].value, ctone[2].value,
		cnoise[0].value, cnoise[1].value, cnoise[2].value);
		
	/* amplitudes (or envelope control) */
	if (camp[0].value < 0) 
		envenable('A');
	else
		chanampl('A', camp[0].value);
	if (camp[1].value < 0) 
		envenable('B');
	else
		chanampl('B', camp[1].value);
	if (camp[2].value < 0) 
		envenable('C');
	else
		chanampl('C', camp[2].value);

	/* tone periods */
	for (i=0; i<3; i++)
		toneperiod('A'+i, cperiod[i].value);
	
	/* envelope period */
	ecyclperiod(eperiod[0].value);
	
	/* finally, set envelope shape to start playing... */
	for (i=0; i<8; i++) {
		if (eshape[i].value) {
			envshape(i);
			break;
		}
	}	
}


main()
{
	int i, alldone, done, valid, val;
	char cmd, cv;
	
	/* open console in raw mode */
	copen();
	
	/* paint screen fixed background */
	puts(screen);
	
	/* display all of the current selector values */
	showall();
	
	/* enable 25th line and then go there */
	ena25();
	gotorc(25,1);

	/* command loop...  */
	for (alldone=FALSE; !alldone; ) {
		scr(HEL);
		puts("\nCommand?");
		while ((cmd = conin()) == 0)
			/* wait for command... */;
		switch (toupper(cmd)) {
			case 'E':
				alldone = TRUE;
				break;
			case 'A':
				/* Channel amplitude */
				for (done=FALSE; !done; ) {
					scr(HEL);
					puts("\nWhich channel amplitude? [A,B,C], Return: exit");
					while ((cv = conin()) == 0)
						/* wait for key press */;
					cv = toupper(cv);
					if ((cv >= 'A') && (cv <= 'C')) {
						/* have valid channel designator, now get amplitude value */
						for (valid=FALSE; !valid; ) {
							scr(HEL);
							puts("\nAmplitude value [0-15] (-1 for Envelope control):");
							val = getval();
							if ((val<0) || ((val >=0) && (val <=15))) {
								/* valid amplitude entered. update the requested
								** amplitide value and refresh amplitude display.
								*/
								valid = TRUE;
								camp[cv-'A'].value = val;
								showi(camp, 3, "ENV");
							}
							else
								putchar(BELL);
						}
					} else if (cv == CR)
						done = TRUE;
					else
						/* bad value */
						putchar(BELL);
				}
				break;
			case 'P':
				/* Period for channel, envelope or noise */
				for (done=FALSE; !done; ) {
					scr(HEL);
					puts("\n[A,B,C], [E]nvelope or [N]oise? Return: exit");
					while ((cv = conin()) == 0)
						/* wait for key press */;
					cv = toupper(cv);
					if (((cv >= 'A') && (cv <= 'C')) || (cv == 'E') || (cv == 'N')) {
						/* have valid designator, now get period value */
						for (valid=FALSE; !valid; ) {
							scr(HEL);
							puts("\nPeriod value:");
							val = getval();
							switch (cv) {
								case 'A':
								case 'B':
								case 'C':
									/* Channel tone period - must be [0..4095] */
									if ((val>=0) && (val <= 4095)) {
										valid = TRUE;
										cperiod[cv-'A'].value = val;
										showi(cperiod, 3, "");
									} else
										putchar(BELL);
									break;
								case 'E':
									/* Envelope period - accept any value */
									valid = TRUE;
									eperiod[0].value = val;
									showi(eperiod, 1, "");
									break;
								case 'N':
									/* Noise period - must be [0..31] */
									if ((val>=0) && (val <= 31)) {
										valid = TRUE;
										nperiod[0].value = val;
										showi(nperiod, 1, "");
									} else
										putchar(BELL);
									break;
							}
						}
					} else if (cv == CR)
						done = TRUE;
					else
						/* bad value */
						putchar(BELL);
				}
				break;
			case 'S':
				/* select envelope shape */
				for (done=FALSE; !done; ) {
					scr(HEL);
					puts("\nEnvelope Shape [0-7], Return: no change:");
					while ((cv = conin()) == 0)
						/* wait for value to be entered ... */;
					if ((cv >= '0') && (cv <= '7')) {
						/* value in range */
						done = TRUE;
						val = cv - '0';
						for (i=0; i<8; i++)
							if (i == val)
								eshape[i].value = ON;
							else
								eshape[i].value = OFF;
						showb(eshape, 8);
					} else if (cv == CR)
						/* abort on carriage return */
						done = TRUE;
					else
						/* bad value */
						putchar(BELL);
				}
				break;
			case 'N':
				/* Toggle noise on/off on channels A, B or C */
				for (done=FALSE; !done; ) {
					scr(HEL);
					puts("\nToggle Noise [A,B,C], Return when done:");
					while ((cv = conin()) == 0)
						/* wait for value to be entered ... */;
					cv = toupper(cv);
					if ((cv >= 'A') && (cv <= 'C')) {
						/* value in range */
						val = cv - 'A';
						/* toggle between on and off */
						cnoise[val].value = !cnoise[val].value;
						showb(cnoise, 3);
					} else if (cv == CR)
						/* abort on carriage return */
						done = TRUE;
					else
						/* bad value */
						putchar(BELL);
				}
				break;
			case 'T':
				/* Toggle tone on/off on channels A, B or C */
				for (done=FALSE; !done; ) {
					scr(HEL);
					puts("\nToggle Tone [A,B,C], Return when done:");
					while ((cv = conin()) == 0)
						/* wait for value to be entered ... */;
					cv = toupper(cv);
					if ((cv >= 'A') && (cv <= 'C')) {
						/* value in range */
						val = cv - 'A';
						/* toggle between on and off */
						ctone[val].value = !ctone[val].value;
						showb(ctone, 3);
					} else if (cv == CR)
						/* abort on carriage return */
						done = TRUE;
					else
						/* bad value */
						putchar(BELL);
				}
				break;
			case 'H':
				/* let's hear the sound! */
				playsound();
				break;
			default:
				putchar(BELL);
				break;
		}
	}
	
	/* reset the H19 */
	scr(HRAM);
	
	/* restore console settings */
	cclose();
}

