PROGRAM plotsinwave;

(*
	This program plots and re-plots a sin wave across the
	screen in Graphics 2 mode, changing colors and doing
	a little area filling and clearing along the way.

*)


CONST

(*$I GRCONST.GDF *)

	pi		=	3.14159;


TYPE

(*$I GRTYPES.GDF *)


VAR
   i,j : INTEGER;
   sinewave : ARRAY [0..255] OF INTEGER;

   color : colour;

(*$I VDPPROCS.GDF *)


PROCEDURE changecolor;
BEGIN
   IF color=magenta THEN color := black ELSE color := SUCC(color);
END;

BEGIN

WRITELN('Please standby while I make waves.');
FOR i:=0 TO 255 DO sinewave[i]:=32-ROUND(32*SIN(pi*i/25));
color := black;
initg2mode(darkblue,nomag,ssize,color,SUCC(color));
FOR j:=0 TO 100 DO BEGIN (* CRAWL DOWN THE SCREEN *)
   move(0,j+32);
   FOR i:=0 TO 255 DO
      draw(i,j+sinewave[i]);
   changecolor;
   colorset(0,255,0,191,color,SUCC(color));
   areafill(100,150,75,125);
   areaclear(100,150,75,125);
END;
END.
                                                                                                                       