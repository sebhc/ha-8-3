/*
	testrand - test reading joysticks (A/D)
	
*/

#include "ha83.h"

/* HDOS TICCNT location */
#define TICCNT	0x201B

#define	TRUE	1
#define	FALSE	0

unsigned *Ticptr = TICCNT;
unsigned timeout;
unsigned randseed;


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
		/* do nothing, just wait... */;
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
	int i, v;
	
	puts("Reading joystick values...\n");
	
	do {
		puts("Readings: ");
		for (i=0; i<8; i++) {
			v = rdadchan(i);
			putval(v);
			puts("  ");
		}
		putchar('\n');
		wait(1);
	} while(TRUE);
}
