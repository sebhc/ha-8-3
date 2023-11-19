/*
**	testjoy3
**
**	Testing "Atari" style joystick interface.
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
**
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

#define	TRUE	1
#define	FALSE	0

unsigned *Ticptr = TICCNT;
unsigned timeout;


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

/* itoa - convert n to characters in s. */
char *itoa(n, s)
char s[];
int n;
{
	static int c, k;
	static char *p, *q;

	if ((k = n) < 0)
		k = -k;
	q = p = s;
	do {
		*p++ = k % 10 + '0';
	} while (k /= 10);
	if (n < 0) *p++ = '-';
	*p = 0;
	while (q < --p) {
		c = *q; *q++ = *p; *p = c; }
	return (s);
}

abs(i) { return i < 0 ? -i : i; }

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
	char s[10];
	
	itoa(v,s);
	puts(s);
}

main()
{
	int i;
	
	do {
		puts("  A:  ");
		putval(rdpsgport('A'));
		puts("  B:  ");
		putval(rdpsgport('B'));
		putchar('\n');

		/* make sure ^C is detected if hit */
#ifdef	CPM
		CtlCk();
#endif
		mswait(500);

	} while(TRUE);
}
