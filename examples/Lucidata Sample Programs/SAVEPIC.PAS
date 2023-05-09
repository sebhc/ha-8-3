PROGRAM test(image);

(*
	This program demonstrates a method of reading a block of
	VRAM and saving it to disk. This example creates a display
	on the screen, saves it, clears the screen, and then restores
	the display.

	To execute this program, type:

	GRUN SAVEPIC IMAGE

	where IMAGE is the name of the file that will be used to save
	the image.
*)

CONST

(*$I GRCONST.GDF *)


TYPE
(*$I GRTYPES.GDF *)

VAR
	gdata		:	ARRAY [1..1536] OF BYTE;
	image		:	FILE OF BYTE;
	ch		:	CHAR;
	return		:	INTEGER;
	i		:	INTEGER;
	color		:	colour;
	xborder,
	yborder		:	INTEGER;

(*$I VDPPROCS.GDF *)
(*$I VDPMISC.GDF *)

PROCEDURE changecolor;
BEGIN
   IF color=white  THEN  color := black  ELSE  color := SUCC(color);
END;


BEGIN

(*	Initialize the system and put a picture on the screen. *)

	color := black;
	initmcmode(darkblue,nomag,ssize);
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

(*	Read the image from the screen and write it out to disk *)

	REWRITE(image);
	return := blockread(gdata[1],0,1536);
	IF return<>0 THEN BEGIN
	   WRITE(CHR(7),'OOPS. RETURN IS',return);
	   HALT;
	END;
	FOR i:=1 TO 1536 DO WRITE(image,gdata[i]);

(*	Clear the screen, rewind the image file, and read the
	image back in.
*)

	initmcmode(darkblue,nomag,ssize);
	RESET(image);
	FOR i:=1 TO 1536 DO READ(image,gdata[i]);

(*	Wait for the operator, then blast the image back to the screen.*)

	WRITELN('Touch a key to write image to VRAM.');
	READ(ch);
	blockwrite(gdata[1],0,1536);
END.
