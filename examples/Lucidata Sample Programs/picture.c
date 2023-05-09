/*
	program picture
	
	this program draws a regular polygon on the screen and
	then efficiently connects the vertices of the polygon.
*/
#include "ha83.h"

#define	PI	3.14159;
#define	BEL	'\007'

#define	TRUE	1
#define	FALSE	0

type

	boolar	=	array [1..50] of boolean;



int n, erasing, cmode, done;

var

int b[50], c[50];

	h	:	boolar;
	hnot	:	boolar;
	i,n,n1,j,j1,j2 : integer;
	f	:	boolean;
	erasing :	boolean;
	ch	  :	char;
	cmode	:	boolean;


main()
{
	int i, n, mag, ssize, erasing;
	float wpi;
	
	/* toggle between drawing and erasing */
	erasing = FALSE;
	mag = 0;
	ssize = 0;

	/* loop until 0 entered for lines */

	do {
		if (!erasing) {
			do {
				puts("Enter the number of sides for the polygon.\n");
				puts("Make it negative to use complement mode lines.");
				puts(" (in [3..50]) >");
				n = getval();
				cmode = (n < 0);
				n = abs(n);
				if ((n < 3) || (n > 50))
					putchar(BEL);
			} while ((n < 3) || (n > 50))
				
			/* have valid n */
			wpi = 2.0 * PI/n;
			for (i=0; i<n; i++) {
				hnot[i] = FALSE;
				b[i] = round(95.0 * (1 + cos((i)*wpi))) + 32;
				c[i] = round(95.0 * (1 + sin((i)*wpi)));
			}
				
			/* initialize for graphics 2 mode plotting */
			initg2mode(c_dkblue,mag,ssize,c_ltyell,c_dkgrn);
			compmode(cmode);
		}

		for (i=0; i<n/2; i++) {
			h := hnot; /* ???? */
		if i*2=n then for j:=i+1 to n do h[j] := true;
	j2 := 1;
	f := true;
	if i*2=n then n1:=i else n1:=n;
	for j:=1 to n1 do begin
		j1 := j2;
		if h[j1] then begin
		 f := true;
		 while h[j1] do begin
			j1 := j1+1;
			if j1>n then j1:=j1-n;
		 end;
		end;
		j2 := j1+i;
		if j2>n then j2:=j2-n;
		h[j1]:=true;
		if f then move(b[j1],c[j1]);
		f := false;
		draw(b[j2],c[j2]);
	end;
   end;
   if not erasing then begin
	writeln('touch a key to erase the picture.');
	read(ch);
   end;
   erasing := not erasing;
   if not cmode then eraser(erasing); /* toggle the erase function. */
		} while (TRUE) /* end repeat until forever */
}
