PROGRAM multicolor_paint_demo;
CONST

(*$I GRCONST.GDF *)

TYPE

(*$I GRTYPES.GDF *)

VAR

   color	:	colour;
   nxtcolor	:	colour;
   oldcolor	:	colour;

(*$I VDPPROCS.GDF *)

PROCEDURE newcolor;
BEGIN
   IF color=white THEN color:=black ELSE color:=SUCC(color);
   IF color=white THEN nxtcolor:=black ELSE nxtcolor:=SUCC(color);
END;

BEGIN
   color:=transparent;
   newcolor;
   initmcmode(lightblue,nomag,ssize);
   REPEAT
	mcmove(5,5,color);
	mcdraw(59,5,color);
	mcdraw(59,43,color);
	mcdraw(5,43,color);
	mcdraw(5,5,color);

	mcmove(19,15,color);
	mcdraw(44,15,color);
	mcdraw(44,33,color);
	mcdraw(19,33,color);
	mcdraw(19,15,color);

	mcmove(25,20,color);
	mcdraw(39,20,color);
	mcdraw(39,28,color);
	mcdraw(25,28,color);
	mcdraw(25,20,color);

	oldcolor := color;
	mcpaint(32,24,nxtcolor,oldcolor);
	newcolor;
	mcpaint(32,18,nxtcolor,oldcolor);
	newcolor;
	mcpaint(32,10,nxtcolor,oldcolor);
	newcolor;
	mcpaint(32,2,color,oldcolor);
	newcolor;
   UNTIL FALSE; (* or control-c *)
END.
