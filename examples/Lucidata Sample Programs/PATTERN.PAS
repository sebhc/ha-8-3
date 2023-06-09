PROGRAM pattern;

(*
	This program demonstrates a method of using the primitive
	VDP manipulation routines to set up a simple pattern on the
	screen using the pattern mode of the VDP.
*)


CONST

(*$I GRCONST.GDF *)

(* VDP CONSTANTS *)

	vpnev		=	0;   (* NO EXTERNAL VIDEO *)
	vp16k		=	$80; (* 16K RAM CHIPS *)
	vpddp		=	0;   (* BLANK DISPLAY *)
	vpedp		=	$40; (* ENABLE DISPLAY *)
	vpdi		=	0;   (* DISABLE INTERRUPTS *)
	vppm		=	0;   (* PATTERN MODE *)


TYPE

(*$I GRTYPES.GDF *)

VAR
   ptrns	:	ARRAY [0..15] OF BYTE; (* USE 2 PATTERNS *)
   namtab	:	ARRAY [0..767] OF BYTE;
   clrtab	:	ARRAY [0..31] OF BYTE;
   i,p		:	INTEGER;

(*$I VDPPROCS.GDF      We only need the bordercolor procedure from this file *)
(*$I VDPMISC.GDF	*)


PROCEDURE load(index,bits : INTEGER);
BEGIN
   ptrns[index] := bits;
END;

FUNCTION clrindex(color : colour) : INTEGER;
BEGIN
   clrindex := ORD(color) - 1;
END;

BEGIN
	(* INITIALIZE VDP REGISTERS *)

	(* SET UP PATTERN MODE WITH DISPLAY OFF *)
	vdpoptions(vpnev+vp16k+vpddp+vppm);

	(* SET UP TABLE ADDRESSES *)
	pattgentable(0);	(* PATTERN GENERATOR TABLE *)
	pattnametable(6144);(* PATTERN NAME TABLE *)
	colgentable(2048);(* COLOR GENERATOR TABLE *)
	sprnametable(7168);(* SPRITE NAME TABLE *)

	(* SET BORDER COLOR *)
	bordercolor(white);

	(* DISABLE SPRITES *)
	wrtvramdirect(7168,$D0);

	(* SET UP OPTIONS AGAIN WITH DISPLAY ENABLED *)
	vdpoptions(vpnev+vp16k+vpedp+vppm);

	(* SET UP NAME TABLE *)
	FOR i:=0 TO 767 DO IF ODD(i)
	   THEN namtab[i] := 1 ELSE namtab[i] := 0;

	(* SET UP TWO PATTERNS IN GENERATOR TABLE *)
	load(0,0);
	load(1,$7E);
	load(2,$42);
	load(3,$5A);
	load(4,$5A);
	load(5,$42);
	load(6,$7E);
	load(7,0);
	load(8,0);
	load(9,$42);
	load(10,$24);
	load(11,$18);
	load(12,$18);
	load(13,$24);
	load(14,$42);
	load(15,0);

	(* MAKE COLOR GEN TABLE *)
	FOR i:=0 TO 31 DO
	   IF ODD(i) THEN clrtab[i] := clrindex(darkgreen)*16
				       +clrindex(transparent)
		     ELSE clrtab[i] := clrindex(darkred)*16
				       +clrindex(transparent);

	(* SEND NAME TABLE TO VRAM *)
	blockwrite(namtab[0],6144,768);

	(* SEND PATTERNS TO VRAM *)
	blockwrite(ptrns[0],0,16);

	(* SEND COLOR GEN TABLE TO VRAM *)
	blockwrite(clrtab[0],2048,32);

END.
                                                                                                    