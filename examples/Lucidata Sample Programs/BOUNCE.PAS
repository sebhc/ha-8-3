PROGRAM bounce;

(*
This program is an adaptation of the Tiny Pascal program
that appeared in Remark #17. This version uses Multi-Color
mode for drawing the box.
*)


CONST

(*$I GRCONST.GDF *)


TYPE

(*$I GRTYPES.GDF *)

VAR
i	:	INTEGER;


(*$I VDPPROCS.GDF *)

PROCEDURE drawbox;
BEGIN
	mcfill(10,11,1,47,darkblue);
	mcfill(53,54,1,47,darkblue);
	mcfill(12,52,46,47,darkblue);
END;

PROCEDURE throwball;
VAR
vx,vy,x,y,i	:	INTEGER;
BEGIN
 x := 50;
 y := 10;
 vx := 15;
 vy := 0;
 WHILE ((vx<>0) OR (vy <>0) OR (y<173)) DO BEGIN
	IF y<175 THEN vy := vy+2;
	x := x+vx;
	y := y+vy;
	IF x>208 THEN BEGIN
		x := 416-x;
		vx := -vx DIV 4;
	END;
	IF x<48 THEN BEGIN
		x:=96-x;
		vx :=-vx DIV 4;
	END;
	IF y>176 THEN BEGIN
		y := 352-y;
		vy := -vy DIV 2;
	END;
	positsprite(0,x,y);
	FOR i:=1 TO 34  DO;
 END;
END;

PROCEDURE initialize;
VAR
i,j,k,rowst	:	INTEGER;

BEGIN

initmcmode(gray,nomag,ssize);

defssprite(0,0,0,midred,0,
	$3C,$7E,$FF,$FF,$FF,$FF,$7E,$3C);

END;

BEGIN
initialize;
drawbox;
REPEAT
	throwball;
	FOR i:=1 TO 1000 DO;
UNTIL FALSE;
END.
