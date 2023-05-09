PROGRAM sprite;

(*
	This program generates a small sprite and moves it around on
	the screen, demonstrating the use of the built in EARLY bit
	control.
*)

CONST

(*$I GRCONST.GDF *)

	pi		=	3.141592654;

TYPE


(*$I GRTYPES.GDF *)


VAR
	x,y		:	INTEGER;
	color		:	colour;
	theta		:	INTEGER;
	angle		:	REAL;
	degtorad	:	REAL;
	flag		:	BOOLEAN;
	sintable	:	ARRAY [0..359] OF INTEGER;

(*$I VDPPROCS.GDF *)


PROCEDURE changecolor;
BEGIN
   IF color=white  THEN  color := midgreen  ELSE  color := SUCC(color);
   chgsprcolor(0,color);
END;


BEGIN
	WRITELN('360 moments while I set up a sin table...');
	flag := TRUE;
	degtorad := pi/180;
	FOR theta :=0 TO 359 DO
	   sintable[theta] := ROUND(80*SIN(degtorad*theta));
	color := midgreen;
	WRITELN('press cntrl-c when you have seen enough...');
	initmcmode(black,mag,ssize);
	defssprite(0,120,90,white,0,
			$3C,$42,$A5,$81,$C3,$BD,$42,$3C);
	REPEAT
	   FOR theta :=0 TO 359 DO BEGIN
		y := sintable[theta] + 90;
		x := sintable[(theta+90) mod 360] + 110;
		positsprite(0,x,y);
	   END;
	   changecolor;
	   flag := NOT flag;
	   IF flag THEN wrtearlybit(0,0)
		   ELSE wrtearlybit(0,1);
	UNTIL FALSE;
END.
