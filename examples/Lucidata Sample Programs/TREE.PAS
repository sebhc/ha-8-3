PROGRAM TRAVERSE ;
   (* A PROGRAM WHICH USES POINTERS
      TO BUILD A BINARY TREE
      AND THEN TRAVERSE IT IN 3 DIFFERENT ORDERS *)

   (* WITH ACKNOWLEDGEMENTS TO
        ' PASCAL USER MANUAL ' BY JENSEN + WIRTH *)
TYPE
   PTR = ^ NODE ;
   NODE = RECORD
             INFO : CHAR ;
             LLINK,RLINK : PTR
           END;
VAR
   ROOT : PTR ;
   CH   : CHAR ;

PROCEDURE PREORDER ( P : PTR ) ;
   BEGIN
      IF P <> NIL THEN
      BEGIN
         WRITE(P^.INFO);
         PREORDER(P^.LLINK);
         PREORDER(P^.RLINK);
      END;
   END;

PROCEDURE POSTORDER ( P : PTR ) ;
   BEGIN
      IF P <> NIL THEN
      BEGIN
         POSTORDER(P^.LLINK);
         WRITE(P^.INFO);
         POSTORDER(P^.RLINK);
      END;
   END;

PROCEDURE ENDORDER( P : PTR ) ;
   BEGIN
      IF P <> NIL THEN
      BEGIN
         ENDORDER(P^.LLINK);
         ENDORDER(P^.RLINK);
         WRITE(P^.INFO);
      END;
   END;

PROCEDURE ENTER ( VAR P : PTR ) ;
   BEGIN
      READ(CH);
      IF CH <> '.' THEN
      BEGIN
         NEW(P);
         P^.INFO:=CH;
         ENTER(P^.LLINK);
         ENTER(P^.RLINK);
      END
      ELSE P:=NIL ;
   END;

BEGIN
   WRITELN('ENTER DATA - FOR EXAMPLE, THE FOLLOWING LINE');
   WRITELN('ABC..DE..FG...HI..JKL..M..N..');
   WRITELN;
   ENTER(ROOT);     WRITELN;
   PREORDER(ROOT);  WRITELN;
   POSTORDER(ROOT); WRITELN;
   ENDORDER(ROOT);  WRITELN;
END.
                                                                                                                                                         