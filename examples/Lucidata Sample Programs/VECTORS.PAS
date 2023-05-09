PROGRAM test;

(*
	This program draws a series of vectors on the screen. It
	demonstrates the use of the MOVE and PLOT functions for
	the HA83 function.
*)


CONST

(*$I GRCONST.GDF *)


TYPE

(*$I GRTYPES.GDF *)


VAR
	color		:	colour;
	xborder,
	yborder		:	INTEGER;

(*$I VDPPROCS.GDF *)


PROCEDURE changecolor;
BEGIN
   IF color=white  THEN  color:=black  ELSE  color:=SUCC(color);
END;


BEGIN
   color := black;
   initmcmode(darkblue,nomag,ssize);
   REPEAT
	xborder := 0;
	yborder := 0;
	REPEAT
	   mcmove(xborder,yborder,color);
	   mcdraw(63-xborder,yborder,color);
	   mcdraw(63-xborder,47-yborder,color);
	   mcdraw(xborder,47-yborder,color);
	   mcdraw(xborder,yborder,color);
	   changecolor;
	   yborder:=yborder+1;
	   xborder:=xborder+1;
	UNTIL yborder=24;
	REPEAT
	   mcmove(xborder,23,color);
	   mcdraw(63-xborder,23,color);
	   mcmove(xborder,24,color);
	   mcdraw(63-xborder,24,color);
	   changecolor;
	   xborder := xborder+1;
	UNTIL xborder = 32;
	xborder := 22;
	yborder := 22;
	REPEAT
	   mcmove(xborder,yborder,color);
	   mcdraw(63-xborder,yborder,color);
	   mcdraw(63-xborder,47-yborder,color);
	   mcdraw(xborder,47-yborder,color);
	   mcdraw(xborder,yborder,color);
	   changecolor;
	   yborder:=yborder-1;
	   xborder:=xborder-1;
	UNTIL yborder=-1;
   UNTIL FALSE;
END.
