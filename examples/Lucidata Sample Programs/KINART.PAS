PROGRAM kineticart;

(*
	This program draws a moving vector on the screen, slightly
	coordinated with a set of frequencies from the sound chip.
*)

CONST

(*$I GRCONST.GDF *)
	xmin		=	0;
	ymin		=	0;
	xmax		=	255;
	ymax		=	191;
	maxline		=	150;
	xfreq		=	1000;



TYPE

(*$I GRTYPES.GDF *)


	vecs		=	RECORD
				  xs,ys,xe,ye : INTEGER
				END;


VAR
	clrindex	:	INTEGER;
	seed		:	INTEGER;
	x1,y1,x2,y2	:	INTEGER;
	dx1,dx2,dy1,dy2	:	INTEGER;
	vecstore	:	ARRAY [1..maxline] OF vecs;
	count		:	INTEGER;
	lcount		:	INTEGER;
	inum		:	INTEGER;
	i,j		:	INTEGER;
	complement	:	BOOLEAN;

(*$I VDPPROCS.GDF *)
(*$I PSGPROCS.GDF *)


FUNCTION random : INTEGER;
CONST
   mult		=	31;
   modval	=	1009;
BEGIN
   seed := (seed*mult) MOD modval;
   random := seed DIV 251 - 4;
END;

PROCEDURE initialize;
VAR
   ch : CHAR;
(*$A=$2230 *)
   peek2230 : BYTE;
(*$A=$2240 *)
   peek2240 : BYTE;
(*$A=$2250 *)
   peek2250 : BYTE;
(*$A=$2260 *)
   peek2260 : BYTE;
(*$S *)
BEGIN
   WRITE('Complement (C) or Normal (N) vectors? >');
   READ(CH); WRITELN;
   complement := ch IN ['C','c'];
   x1 := peek2250;
   x2 := peek2260;
   y1 := UNPACK(peek2240,0,$7F);
   y2 := UNPACK(peek2230,0,$7F);
   seed := 137;
   initg2mode(darkblue,nomag,ssize,lightgreen,darkblue);
   compmode(complement);
   tonefreq('A',0);
   tonefreq('B',0);
   tonefreq('C',0);
   chanampl('A',8);
   chanampl('B',8);
   envshape(1);
   ecycltime(2);
   envenable('A');
   envenable('B');
   envenable('C');
   psgoptions(TRUE,TRUE,TRUE,FALSE,FALSE,FALSE);
   FOR i:=1 TO maxline DO BEGIN
	vecstore[i].xs := 0;
	vecstore[i].ys := 0;
	vecstore[i].xe := 0;
	vecstore[i].ye := 0;
   END;
   inum := 100;
END;


PROCEDURE vector(xo,yo,xd,yd : INTEGER);
BEGIN
   eraser(FALSE);
   move(xo,yo);
   draw(xd,yd);
END;

PROCEDURE erase(xo,yo,xd,yd : INTEGER);
BEGIN
   IF NOT complement THEN eraser(TRUE);
   move(xo,yo);
   draw(xd,yd);
END;

BEGIN
   initialize;
   count := 0;
   REPEAT
     FOR lcount :=1 TO maxline DO BEGIN
	erase(vecstore[lcount].xs,vecstore[lcount].ys,
	       vecstore[lcount].xe,vecstore[lcount].ye);
	tonefreq('B',vecstore[lcount].xs+
				    vecstore[lcount].ys+
				    vecstore[lcount].xe+
				    vecstore[lcount].ye);
	IF count=0 THEN BEGIN
	   dx1 := random;
	   dx2 := random;
	   dy1 := random;
	   dy2 := random;
	   count := ABS(random)*inum+1;
	END;
	IF NOT((x1+dx1) IN [xmin..xmax]) THEN dx1 := -dx1;
	x1 := x1+dx1;
	IF NOT((y1+dy1) IN [ymin..ymax]) THEN dy1:=-dy1;
	y1 := y1+dy1;
	IF NOT ((x2+dx2) IN [xmin..xmax]) THEN dx2 := -dx2;
	x2 := x2+dx2;
	IF NOT ((y2+dy2) IN [ymin..ymax]) THEN dy2 := -dy2;
	y2 := y2+dy2;
	vector(x1,y1,x2,y2);
	vecstore[lcount].xs := x1;
	vecstore[lcount].ys := y1;
	vecstore[lcount].xe := x2;
	vecstore[lcount].ye := y2;
	tonefreq('A',x1+x2+y1+y2);
	tonefreq('C',ABS(x1+y1-x2-y2));
	envshape(1);
	ecycltime(2);
	count := count-1;
     END;
   UNTIL FALSE;
END.
