PROGRAM maze_painter;

(*
	This maze drawing program is adapted from the UCSD Pascal
	program by Thomas Edward Neldner which appeared in the May
	1982 issue of BYTE magazine.
*)


CONST
   height	=	17;
   heightp1	=	18;
   width	=	23;
   widthp1	=	24;

(*$I GRCONST.GDF *)

TYPE

(*$I GRTYPES.GDF *)

VAR
   q,z,a,b,i,j,x :	INTEGER;
   m		 :	ARRAY [0..heightp1,0..widthp1] OF INTEGER;
   seed		 :	REAL;
   finished	 :	BOOLEAN;
   ch		 :	CHAR;
   xp		 :	INTEGER;
   xt,yt	 :	INTEGER;

(*$I VDPPROCS.GDF *)

PROCEDURE l150;
BEGIN
   m[i,j] := m[i,j]-1;
   j := j+1;
   m[i,j] := 4;
   q := 0;
END;

PROCEDURE l380;
BEGIN
   j := j-1;
   m[i,j] := 3;
   q := 0;
END;

PROCEDURE l420;
BEGIN
   m[i,j] := m[i,j]-2;
   i := i+1;
   m[i,j] := 4;
   q := 0;
END;

PROCEDURE l470;
BEGIN
   i := i-1;
   m[i,j] := 2;
   q := 0;
END;

FUNCTION random(x : INTEGER) : INTEGER;
BEGIN
   seed := seed*27.36947 + 31.8723423;
   seed := seed - TRUNC(seed);
   random := TRUNC(seed*x)+1;
END;

PROCEDURE initialize;
BEGIN
   WRITE('Enter a number for the maze seed. >');
   READ(seed);
   seed := ABS(seed);
   REPEAT seed:=seed*13 UNTIL seed>1;
   REPEAT seed:=seed/17 UNTIL seed>1;

   (* initialize the maze matrix *)

   FOR i:=1 TO height DO FOR j:=1 TO width DO m[i,j] := 0;
   FOR i:=0 TO widthp1 DO BEGIN
	m[0,i] := 1;
	m[heightp1,i] := 1;
   END;
   FOR i:=0 TO heightp1 DO BEGIN
	m[i,0] := 1;
	m[i,widthp1] := 1;
   END;

   (* initialize the screen and draw the border *)

   initg2mode(lightblue,nomag,ssize,lightblue,black);
   move(11,9);
   draw(243,9);
   draw(243,181);
   draw(11,181);
   draw(11,9);
   move(13,11);
   draw(241,11);
   draw(241,179);
   draw(13,179);
   draw(13,11);

   (* make the entrance to the maze *)

   eraser(TRUE);
   xt := random(width-2) * 10;
   move(xt+13,9);
   draw(xt+21,9);
   move(xt+13,11);
   draw(xt+21,11);
   eraser(FALSE);
   move(xt+13,11);
   draw(xt+13,0);
   draw(xt+21,0);
   draw(xt+21,11);
   xp := xt+16;

   i := 1;
   j := 1;
   a := 1;
   b := 1;
   q := 0;
   m[i,j] := 4;
   finished := FALSE;
END;

BEGIN
   initialize;
   WHILE NOT finished DO BEGIN

	(* create one row of the maze *)

	x := 0;
	IF m[i,j-1]<>0 THEN x := x+8;
	IF m[i-1,j]<>0 THEN x := x+4;
	IF m[i+1,j]<>0 THEN x := x+2;
	IF m[i,j+1]<>0 THEN x := x+1;
	CASE x OF

	1: CASE random(3) OF
		1: l380;
		2: l420;
		3: l470
	   END;
	2: CASE random(3) OF
		1: l150;
		2: l380;
		3: l470
	   END;
	3: CASE random(2) OF
		1: l380;
		2: l470
	   END;
	4: CASE random(3) OF
		1: l150;
		2: l380;
		3: l420
	   END;
	5: CASE random(2) OF
		1: l380;
		2: l420
	   END;
	6: CASE random(2) OF
		1: l150;
		2: l380
	   END;
	7: l380;
	8: CASE random(3) OF
		1: l150;
		2: l420;
		3: l470
	   END;
	9: CASE random(2) OF
		1: l420;
		2: l470
	   END;
	10:CASE random(2) OF
		1: l150;
		2: l470
	   END;
	11:l470;
	12:CASE random(2) OF
		1: l150;
		2: l420
	   END;
	13:l420;
	14:l150;
	15:
	
	END;

	IF x=15 THEN BEGIN
	   IF b<>width THEN b:=b+q
	   ELSE BEGIN

		(* draw in the walls for this row *)

		yt := (a-1)*10;
		FOR j:=1 TO width DO BEGIN
			xt := (j-1)*10;
			CASE m[a,j] OF

			2: BEGIN
				move(xt+21,yt+10);
				draw(xt+21,yt+20);
				draw(xt+23,yt+20);
				draw(xt+23,yt+10);
				draw(xt+21,yt+10);
			   END;
			3: BEGIN
				move(xt+12,yt+19);
				draw(xt+22,yt+19);
				draw(xt+22,yt+21);
				draw(xt+12,yt+21);
				draw(xt+12,yt+19);
			   END;
			4: BEGIN
				move(xt+12,yt+19);
				draw(xt+21,yt+19);
				draw(xt+21,yt+10);
				draw(xt+23,yt+10);
				draw(xt+23,yt+21);
				draw(xt+12,yt+21);
				draw(xt+12,yt+19);
			   END
			END OTHERWISE;
		END;

		IF a=height THEN finished := TRUE
		ELSE BEGIN
		   b := 1;
		   a := a+1;
		END;
	   END;
	   i := a;
	   j := b;
	   q := 1;
	END;
   END;

   (* make an exit at a random location *)

   xt := (random(width-2)+1) * 10;
   eraser(TRUE);
   move(xt+13,179);
   draw(xt+21,179);
   move(xt+13,181);
   draw(xt+21,181);
   eraser(FALSE);
   move(xt+13,179);
   draw(xt+13,191);
   draw(xt+21,191);
   draw(xt+21,179);

   WRITELN(CHR(7),'touch a key to paint the maze...');
   READ(ch);
   paint(xp,0,1);
END.
