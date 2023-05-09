PROGRAM PRUN_CONFIGURATION_UTILITY(PRUN);

CONST
   DVTABADDRESS	=	$22CB;
   NDVD		=	$22CD;
   DEVINT	=	$22C0;
   DEVFIN	=	$22C2;
   USERFWA	=	$2280;
   AQUIET	=	$22E1;
   AMAPPING	=	$22E2;
   AULMEM	=	$22E3;
   ORD0		=	  $30;
   ORDA		=	  $41;
   HEADERSIZE	=	    8;
   MAXDEVICE	=	    8;
   NL		=     CHR(10);
   DOLLAR	=	  '$';
   BELL		=      CHR(7);
   FCHAN	=	   28;

TYPE
   BYTBOOL	=	RECORD
			  CASE INTEGER OF
			  1: (BYT : BYTE);
			  2: (LOG : BOOLEAN)
			END;
   ADDR		=	RECORD
	                  CASE INTEGER OF
	                  1:  (LSB,MSB  :  BYTE);
	                  2:  (INT :  INTEGER)
			END;
   HEXADDRESS	=	ARRAY [1..4] OF CHAR;
   SOCTADDRESS	=	ARRAY [1..7] OF CHAR;
   DEFBLK	=	ARRAY [1..7] OF CHAR;
   CSTRNG	=	ARRAY [1..24] OF CHAR;
   FNAME	=	ARRAY [1..16] OF CHAR;
   C3		=	ARRAY [0..2] OF CHAR;

VAR
   PRUN_NAME	:	FNAME;
   DEFAULT	:	DEFBLK;
   TABLEADDRESS :	ADDR;
   DEVTAB	:	ARRAY [1..MAXDEVICE] OF ADDR;
   INITADDRESS	:	ADDR;
   FINADDRESS	:	ADDR;
   QUIET	:	BYTBOOL;
   MAPPING	:	BYTBOOL;
   ULMEM	:	BYTBOOL;
   PRUN		:	FILE OF BYTE;
   OPTION	:	INTEGER;
   DONE		:	BOOLEAN;
   LINE		:	SOCTADDRESS;
   HEXAD	:	HEXADDRESS;
   ALTERED	:	BOOLEAN;
   CLEAR	:	CSTRNG;
   I		:	INTEGER;
   PAUSING	:	BOOLEAN;
   FCBADD	:	ADDR;
   BADENTRY	:	BOOLEAN;


(*$A=$22DF *)

   BUFFMODE	:	BOOLEAN;

(*$A=$22E0 *)

   DERRPR	:	BOOLEAN;

(*$S*)


(*****************)

FUNCTION UPPERCASE(CH : CHAR) : CHAR;
BEGIN
   IF CH IN ['a'..'z'] THEN UPPERCASE := CHR(ORD(CH)-$20)
		       ELSE UPPERCASE := CH;
END;

(*****************)

PROCEDURE CLEARSCREEN;
BEGIN
   WRITE(CLEAR);
END;

(*****************)

PROCEDURE ASSIGNNAME;
VAR
   I	:	INTEGER;
   WORK	:	FNAME;
   NONAME:	FNAME;
   CH	:	CHAR;
   IDX	:	INTEGER;
   EX	:	BOOLEAN;
   LAST :	INTEGER;
   EXT	:	C3;
BEGIN
   BUFFMODE := TRUE;
   NONAME := '                ';
   REPEAT
	WRITE('The currently assigned file is: ');
	FOR I:=1 TO 16 DO IF PRUN_NAME[I]<>' ' THEN WRITE(PRUN_NAME[I]);
	WRITELN(' .'); WRITELN;
	WRITELN('Enter a new name or <RETURN> for no change.');
	WRITELN('(Device SY0: and an extension of .ABS will be assumed).');
	WRITE('>');
	READLN(WORK);
	WRITELN;
	IF WORK<>NONAME THEN BEGIN
	   PRUN_NAME := NONAME;
	   FOR I:=1 TO 3 DO PRUN_NAME[I] := DEFAULT[I];
	   PRUN_NAME[4] := ':';
	   IF WORK[4]=':' THEN BEGIN
		PRUN_NAME[3] := WORK[3];
		IDX := 0;
	   END ELSE IDX := -4;
	   EX := FALSE;
	   LAST := 0;
	   FOR I:=5 TO 16 DO BEGIN
		CH := UPPERCASE(WORK[I+IDX]);
		IF CH='.' THEN EX := TRUE;
		PRUN_NAME[I] := CH;
		IF (CH=' ') AND (LAST=0) THEN LAST := I;
	   END;
	   IF NOT EX THEN BEGIN
		FOR I:=1 TO 3 DO PRUN_NAME[I+LAST] := DEFAULT[I+3];
		PRUN_NAME[LAST] := '.'
	   END;
	END;
   UNTIL WORK=NONAME;
   BUFFMODE := FALSE;
END;


(*****************)

PROCEDURE ASSIGNPRUN;
VAR
   CH	:	CHAR;
   I	:	INTEGER;
BEGIN
   DERRPR := FALSE; (* DISABLE FATAL DISK ERROR *)
   REPEAT
	ASSIGNNAME;
	ASSIGN(PRUN,PRUN_NAME);
	RESET(PRUN);
	IF EOF(PRUN) THEN BEGIN
	   CLEARSCREEN;
	   WRITELN(BELL,'That file does not exist on the specified drive!');
	   WRITELN('Please re-enter name.');
	   WRITELN;
	END;
   UNTIL NOT EOF(PRUN);
   DERRPR := TRUE; (* RE-ENABLE DISK ERRORS *)
   REWRITE(PRUN);  (* GET READY TO RE-CONFIGURE *)
END;

(*****************)

FUNCTION DIGIT(NUM : INTEGER) : CHAR;
BEGIN
   DIGIT := CHR(NUM + ORD0);
END;

(*****************)

FUNCTION SPLIT(NUM : INTEGER) : SOCTADDRESS;
VAR
   SOCT : SOCTADDRESS;
BEGIN
   SOCT[4] := '.';
   SOCT[1] := DIGIT(UNPACK(NUM,14,3));
   SOCT[2] := DIGIT(UNPACK(NUM,11,7));
   SOCT[3] := DIGIT(UNPACK(NUM,8,7));
   SOCT[5] := DIGIT(UNPACK(NUM,6,3));
   SOCT[6] := DIGIT(UNPACK(NUM,3,7));
   SOCT[7] := DIGIT(UNPACK(NUM,0,7));
   SPLIT := SOCT;
END;

(*****************)

FUNCTION POS(MEMORYADDRESS  :  INTEGER) : INTEGER;
BEGIN
   POS := MEMORYADDRESS - USERFWA + HEADERSIZE;
END;

(*****************)

FUNCTION HEXVAL(HEXCHAR : CHAR) : INTEGER;
VAR
   HEXV : INTEGER;
BEGIN
   HEXCHAR := UPPERCASE(HEXCHAR);
   IF HEXCHAR IN ['0'..'9'] THEN HEXV := ORD(HEXCHAR) - ORD0
   ELSE HEXV := ORD(HEXCHAR) - ORDA + 10;
   BADENTRY := NOT(HEXV IN [0..15]);
   HEXVAL := HEXV;
END;

(*****************)

PROCEDURE SHIFTADD (VAR NUM : INTEGER; DIGIT : CHAR);
VAR
   NUM16 : ADDR;
BEGIN
   NUM16.INT := NUM;
   NUM16.MSB := UNPACK(NUM16.MSB,0,$F)*16 + UNPACK(NUM16.LSB,4,$F);
   NUM16.LSB := UNPACK(NUM16.LSB,0,$F)*16 + HEXVAL(DIGIT);
   NUM := NUM16.INT;
END;

(*****************)

FUNCTION CONHEX (HEXAD : HEXADDRESS) : INTEGER;
VAR
   NUMBER : INTEGER;
   I	: INTEGER;
BEGIN
   NUMBER := 0;
   FOR I:=1 TO 4 DO
	IF HEXAD[I] <> ' ' THEN SHIFTADD(NUMBER,HEXAD[I]);
   CONHEX := NUMBER;
END;

(*****************)

FUNCTION CONOCT(OCTAD : SOCTADDRESS) : INTEGER;
VAR
   LSB,MSB : INTEGER;
   NUM16   : ADDR;
   I     : INTEGER;
BEGIN
   MSB := 0;
   LSB := 0;
   BADENTRY := FALSE;
   FOR I:=1 TO 7 DO BEGIN
	IF OCTAD[I] IN ['0'..'7']
	THEN LSB := UNPACK(LSB,0,$FFF)*8 + ORD(OCTAD[I]) - ORD0
	ELSE IF OCTAD[I]='.' THEN BEGIN
	   MSB := LSB;
	   LSB := 0;
	END ELSE IF OCTAD[I]<>' ' THEN BADENTRY := TRUE;
   END;
   IF (MSB>255) OR (LSB>255) THEN BEGIN
	BADENTRY := TRUE;
	NUM16.INT := 0;
   END ELSE BEGIN
	NUM16.MSB := MSB;
	NUM16.LSB := LSB;
   END;
   CONOCT := NUM16.INT;
END;

(*****************)

PROCEDURE READCONFIG;
VAR
   I  :  INTEGER;
BEGIN
   POSITION(PRUN,POS(DVTABADDRESS));
   READ(PRUN,TABLEADDRESS.LSB,TABLEADDRESS.MSB);
   POSITION(PRUN,POS(TABLEADDRESS.INT));
   FOR I:=1 TO MAXDEVICE DO
	READ(PRUN,DEVTAB[I].LSB,DEVTAB[I].MSB);
   POSITION(PRUN,POS(DEVINT));
   READ(PRUN,INITADDRESS.LSB,INITADDRESS.MSB);
   READ(PRUN,FINADDRESS.LSB,FINADDRESS.MSB);
   POSITION(PRUN,POS(AQUIET));
   READ(PRUN,QUIET.BYT);
   POSITION(PRUN,POS(AMAPPING));
   READ(PRUN,MAPPING.BYT);
   POSITION(PRUN,POS(AULMEM));
   READ(PRUN,ULMEM.BYT);
END;

(*****************)

PROCEDURE UPDATEPRUN;
VAR
   I : INTEGER;
BEGIN
   IF ALTERED THEN BEGIN
	POSITION(PRUN,POS(TABLEADDRESS.INT));
	FOR I:=1 TO MAXDEVICE DO
	   WRITE(PRUN,DEVTAB[I].LSB,DEVTAB[I].MSB);
	POSITION(PRUN,POS(DEVINT));
	WRITE(PRUN,INITADDRESS.LSB,INITADDRESS.MSB);
	WRITE(PRUN,FINADDRESS.LSB,FINADDRESS.MSB);
	POSITION(PRUN,POS(AQUIET));
	WRITE(PRUN,QUIET.BYT);
	POSITION(PRUN,POS(AULMEM));
	WRITE(PRUN,ULMEM.BYT);
	POSITION(PRUN,POS(AMAPPING));
	WRITE(PRUN,MAPPING.BYT);
   END;
   ALTERED := FALSE;
END;

(*****************)

FUNCTION HEX(NUM : INTEGER) : HEXADDRESS;
VAR
   HX  :  HEXADDRESS;

(************)

FUNCTION HXCHAR(NUM  :  INTEGER) : CHAR;
BEGIN
   IF NUM IN [0..9] THEN HXCHAR := DIGIT(NUM)
	              ELSE HXCHAR := CHR(NUM - 10 + ORDA);
END;

(************)

BEGIN
   HX[1] := HXCHAR(UNPACK(NUM,12,$F));
   HX[2] := HXCHAR(UNPACK(NUM,8,$F));
   HX[3] := HXCHAR(UNPACK(NUM,4,$F));
   HX[4] := HXCHAR(UNPACK(NUM,0,$F));
   HEX := HX;
END;

(*****************)

PROCEDURE PRTDRVTBL;
VAR
   I	   :   INTEGER;
   ADDRESS   :   ADDR;
   CH	  :   CHAR;

BEGIN
   CLEARSCREEN;
   WRITELN;WRITELN('Device Driver Service Addresses:');
   WRITELN;
   FOR I:=1 TO MAXDEVICE DO BEGIN
	WRITE('Device #',(I-1):1);
	IF DEVTAB[I].INT = NDVD
	   THEN WRITELN('Null Device' : 16)
	   ELSE WRITELN('Service Address = ':23,DOLLAR,HEX(DEVTAB[I].INT),
	                           '(':4,SPLIT(DEVTAB[I].INT),'A)');
   END;
   FOR I:=1 TO 2 DO BEGIN
	IF I=1 THEN ADDRESS := INITADDRESS ELSE ADDRESS := FINADDRESS;
	IF I=1 THEN WRITE('Device Initialization Address = ')
	       ELSE WRITE('Device Termination Address    = ');
	IF ADDRESS.INT = NDVD
	   THEN WRITELN('Null')
	   ELSE WRITELN(DOLLAR,HEX(ADDRESS.INT),'(':4,SPLIT(ADDRESS.INT),'A)');
   END;
   IF PAUSING THEN BEGIN
	PAUSING := NOT PAUSING;
	WRITE('Hit <RETURN> to continue. >');
	BUFFMODE := TRUE; READLN(CH); BUFFMODE := FALSE;
   END;
END;

(*****************)

PROCEDURE PROMPT;
BEGIN
WRITELN(
'Enter the address in either Hexadecimal or Split Octal notation.',NL,
'A Hexadecimal number must be preceded by a $ sign. If the first',NL,
'character is a digit then the number will be interpreted as a',NL,
'Split Octal address which uses a period (.) as a separator.',NL,
'  Examples:',NL,
'	 $7500',NL,
'	 240.040',NL,
'If the first character is an N then the null address will be loaded.');
WRITELN;
WRITELN('Enter the address.');
WRITE('>');
END;

(*****************)

PROCEDURE INPUTADDRESS(VAR ADDRESS : INTEGER);
VAR
   I  : INTEGER;
BEGIN
   BUFFMODE := TRUE;
   READLN(LINE);
   BUFFMODE := FALSE;
   IF LINE[1] = DOLLAR THEN BEGIN
	FOR I:=2 TO 5 DO HEXAD[I-1] := LINE[I];
	ADDRESS := CONHEX(HEXAD);
   END
   ELSE
	IF LINE[1]='N' THEN ADDRESS := NDVD
	ELSE ADDRESS := CONOCT(LINE);
   IF BADENTRY THEN BEGIN
	WRITE(BELL);
	ADDRESS := NDVD;
   END;
END;

(*****************)

PROCEDURE PRINTMENU;
BEGIN
   CLEARSCREEN;
   WRITE('V3.8 Lucidata Pascal Run-time System Configuration.');
   WRITELN(' (HDOS)'); WRITELN;
   WRITELN('Choose from the following operations:');
   WRITELN('  1 -- Display current device driver table.');
   WRITELN('  2 -- Alter a driver address.');
   WRITELN('  3 -- Alter the initialization address.');
   WRITELN('  4 -- Alter the termination address.');
   WRITELN('  5 -- Alter/Examine the quiet mode flag. (QUIET)');
   WRITELN('  6 -- Alter/Examine the low memory usage flag. (ULMEM)');
   WRITELN('  7 -- Alter/Examine the lower case mapping flag. (MAPPING)');
   WRITELN('  8 -- Update PRUN to the currently set configuration.');
   WRITELN('  9 -- Configure a different copy of PRUN.');
   WRITELN(' 10 -- Quit, return to HDOS.');
   WRITELN;
END;

(*****************)

PROCEDURE GETOPTION;
BEGIN
   REPEAT
	WRITE('Enter the desired operation number. >');
	READLN(OPTION);
   UNTIL OPTION IN [1..10];
   IF OPTION IN [2..4] THEN ALTERED := TRUE;
   PAUSING := OPTION = 1;
END;

(*****************)

PROCEDURE SETDEVICEADDRESS;
VAR
   LDN : INTEGER;
BEGIN
   WRITELN;
   REPEAT
	WRITE('Enter the device number. >');
	READLN(LDN);
   UNTIL (LDN+1) IN [1..MAXDEVICE];
   PROMPT;
   INPUTADDRESS(DEVTAB[LDN+1].INT);
END;

(*****************)

PROCEDURE SETINITADDRESS;
BEGIN
   WRITELN;
   WRITELN('Enter the initialization address.');
   PROMPT;
   INPUTADDRESS(INITADDRESS.INT);
END;

(*****************)

PROCEDURE SETFINADDRESS;
BEGIN
   WRITELN;
   WRITELN('Enter the termination address.');
   PROMPT;
   INPUTADDRESS(FINADDRESS.INT);
END;
(*****************)

PROCEDURE SETMODEFLAGS;
VAR
   ANS	:	C3;
   I	:	INTEGER;
   STATE:	BOOLEAN;
BEGIN
   CLEARSCREEN;
   CASE OPTION OF

5: WRITELN(
'The Quiet flag controls the actions of the run-time system during',NL,
'sign-on and sign-off. If the Quiet flag is OFF, PRUN will print out',NL,
'memory limits and stack size during sign-on, and the End of Program',NL,
'Execution message during sign-off. No messages will be printed if the',NL,
'Quiet flag is ON.');

6: WRITELN(
'The ULMEM flag controls the memory usage of the Lucidata Pascal',NL,
'Compiler. If ULMEM is ON, the run-time system will attempt to allocate',
NL,
'memory from $820 to $13FF (10.040A to 23.377A) for file buffers and',NL,
'special compiler functions, allowing the compiler to operate in a',NL,
'non-paging mode with a slightly smaller system. If ULMEM is OFF, no',NL,
'use will be made of this memory area.');

7: WRITELN(
'The MAPPING flag controls the lower case mapping function of the',NL,
'Lucidata Pascal Compiler. If MAPPING is ON, ALL lower case characters',
NL,
'in symbols and keywords will be mapped to upper case. If MAPPING is',NL,
'OFF, no mapping will take place.')

   END;
   WRITELN;

   WRITE('The current setting of the ');

   CASE OPTION OF
	5: WRITE('QUIET ');
	6: WRITE('ULMEM ');
	7: WRITE('MAPPING ')
   END;

   WRITE('flag is ');

   CASE OPTION OF
	5: STATE := QUIET.LOG;
	6: STATE := ULMEM.LOG;
	7: STATE := MAPPING.LOG
   END;

   IF STATE THEN WRITE('ON') ELSE WRITE('OFF');
   WRITELN('.');
   WRITELN;
   BUFFMODE := TRUE;
   REPEAT
	WRITE('Enter ON , OFF , or <RETURN> for no change. >');
	READLN(ANS);
	FOR I:=0 TO 2 DO ANS[I] := UPPERCASE(ANS[I]);
   UNTIL (ANS='ON ') OR (ANS='OFF') OR (ANS='   ');
   IF ANS<>'   ' THEN BEGIN
	ALTERED := TRUE;
	STATE := ANS = 'ON ';
	CASE OPTION OF
	   5: QUIET.LOG := STATE;
	   6: ULMEM.LOG := STATE;
	   7: MAPPING.LOG := STATE
	END;
   END;
   BUFFMODE := FALSE;

END;

(*****************)

PROCEDURE CHANGEFILE;
BEGIN
   CLEARSCREEN;
   ALTERED := FALSE;
   BADENTRY := FALSE;
   ASSIGNPRUN;
   READCONFIG;
END;

(******************MAIN PROCEDURE*********************************)

BEGIN
   FOR I:=1 TO 24 DO CLEAR[I] := NL;
   ALTERED := FALSE;
   BADENTRY := FALSE;
   DEFAULT := 'SY0ABS ';
   PRUN_NAME := 'SY0:PRUN.ABS    ';
   CLEARSCREEN;
   WRITELN('Please identify the name of the PRUN you wish to configure.');
   WRITELN; WRITELN;
   ASSIGNPRUN;
   READCONFIG;
   DONE := FALSE;
   REPEAT
	PRINTMENU;
	GETOPTION;
	IF OPTION IN [1..4] THEN PRTDRVTBL;
	CASE OPTION OF

	   1 : ;

	   2 : SETDEVICEADDRESS;

	   3 : SETINITADDRESS;

	   4 : SETFINADDRESS;

	   5,6,7 : SETMODEFLAGS;

	   8 : UPDATEPRUN;

	   9 : CHANGEFILE;

	  10 : DONE := TRUE

	END;
   UNTIL DONE;
END.
                                                                  