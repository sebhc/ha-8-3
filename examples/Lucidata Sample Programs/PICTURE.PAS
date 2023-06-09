PROGRAM picture;

(*
	This program draws a regular polygon on the screen and
	then efficiently connects the vertices of the polygon.
*)

CONST

(*$I GRCONST.GDF *)

	pi		=	3.14159;

TYPE

	boolar	=	ARRAY [1..50] OF BOOLEAN;

(*$I GRTYPES.GDF *)

VAR
	b,c	:	ARRAY [1..50] OF INTEGER;
	h	:	boolar;
	hnot	:	boolar;
	i,n,n1,j,j1,j2 : INTEGER;
	wpi	:	REAL;
	f	:	BOOLEAN;
	erasing :	BOOLEAN;
	ch	  :	CHAR;
	cmode	:	BOOLEAN;



(*$I VDPPROCS.GDF *)



(*****************)

BEGIN

erasing := FALSE;	(* Toggle between drawing and erasing *)

(* Enter a do until control-c loop *)

REPEAT
   IF NOT erasing THEN BEGIN
	REPEAT
		WRITELN('Enter the number of sides for the polygon.');
		WRITE('Make it negative to use complement mode lines.');
		WRITE(' (IN [3..50]) >');
		READ(n);
		cmode := n<0;
		n := ABS(n);
		IF NOT(n IN [3..50]) THEN WRITELN(CHR(7));
	UNTIL n IN [3..50];
	wpi := 2*pi/n;
	FOR i:=1 TO n DO BEGIN
		hnot[i] := FALSE;
		b[i] := ROUND(95*(1+COS((i-1)*wpi))) + 32;
		c[i] := ROUND(95*(1+SIN((i-1)*wpi)));
	END;

	(* initialize for Graphics 2 mode plotting *)

	initg2mode(darkblue,nomag,ssize,lightyellow,darkgreen);
	compmode(cmode);
   END;

   FOR i:=1 TO n DIV 2 DO BEGIN
	h := hnot;
	IF i*2=n THEN FOR j:=i+1 TO n DO h[j] := TRUE;
	j2 := 1;
	f := TRUE;
	IF i*2=n THEN n1:=i ELSE n1:=n;
	FOR j:=1 TO n1 DO BEGIN
		j1 := j2;
		IF h[j1] THEN BEGIN
		 f := TRUE;
		 WHILE h[j1] DO BEGIN
			j1 := j1+1;
			IF j1>n THEN j1:=j1-n;
		 END;
		END;
		j2 := j1+i;
		IF j2>n THEN j2:=j2-n;
		h[j1]:=TRUE;
		IF f THEN move(b[j1],c[j1]);
		f := FALSE;
		draw(b[j2],c[j2]);
	END;
   END;
   IF NOT erasing THEN BEGIN
	WRITELN('Touch a key to erase the picture.');
	READ(ch);
   END;
   erasing := NOT erasing;
   IF NOT cmode THEN eraser(erasing); (* Toggle the erase function. *)
UNTIL FALSE; (* end REPEAT UNTIL FOREVER *)
END.
                                                                                                                                                                                                                            