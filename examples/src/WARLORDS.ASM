;
;       WARLORDS.ASM    Color graphics Warlords game
;                       for H8 w/HA-8-3 graphics card.
;
;       Original version 11/06/83 \
;       Rewritten        11/03/86  \
;       Completion date  08/08/87   \
;       Last Update      11/29/87    \ by Les Bird
;
PGTADR  EQU     0000H   ; 0000 - 6143
PNTADR  EQU     1800H   ; 6144 - 6911
SATADR  EQU     1B00H   ; 6912 - 7039
; -- BLANK --           ; 7040 - 8191
CGTADR  EQU     2000H   ; 8192 -14335
SGTADR  EQU     3800H   ;14336 -16383
;
VDPDAT  EQU     0B8H
VDPCTL  EQU     0B9H
PSGDAT  EQU     0BAH
PSGCTL  EQU     0BBH
;JOYDAT  EQU     0BEH
;JOYCTL  EQU     0BEH
JOYDAT  EQU     0F6H	; relocated to avoid 274Q
JOYCTL  EQU     0F6H	; for Z67 in alternate location
CSTAT   EQU     0EDH
CDATA   EQU     0E8H
CIMSK   EQU     01H
COMSK   EQU     20H
;
TRUE    EQU     01H
FALSE   EQU     00H
;
INTRP   EQU     FALSE   ; Update score by interrupt.
DBUG    EQU     FALSE   ; Debug mode.
DBG1    EQU     FALSE   ; Include memory address table.
DBG2    EQU     FALSE   ; INR/DCR levels and slow motion mode.
BDOS    EQU     0005H
INT2    EQU     0008H   ; Clock interrupt vector.  Since this
INT3    EQU     0018H   ;  \    interrupt is performed automatically
CLOCK   EQU     000BH   ;   \   this is an ideal intercept point
;                       ;    \  for doing automatic updates.
        ORG     100H
        IF      DBG1
        JMP     START
; Data addresses in memory.
        DW      G0VMSG,MSGDLY,CCHAR,JPLYR,JDAT1,PNUMB,PLYCO
        DW      CBNUM,PSCR1A,SCRADR,BDLY,BNUMB,BDAT5A,BDAT10
        DW      BDAT11,BRFL00,BRFLCT,BTHR01,PLCNT,PLNEW,SNDBIT
        DW      SBRIK,SINIT,SKILL,SPADL,STHRO,SWALL,PGTTBL
        DW      NUMBR,SATTBL,PNTTBL,CGTTBL,SGTTBL
        ENDIF
;
START:  LXI     SP,STACK
        CALL    VINIT
        MVI     A,07H   ; Setup I/O JSTIK ports
        OUT     PSGCTL  ; for Input.
        MVI     A,038H
        OUT     PSGDAT
;
        IF      INTRP   ; Assemble if interrupts used --.
        DI              ; Set up clock interrupts       |
        LDA     INT2    ;     to update player's score. |
        STA     OLDINT2 ; Save old interrupt routine    |
        LHLD    INT2+1  ;      first so we can restore  |
        SHLD    OLDINT2+1 ;    it when exiting.         |
        LXI     H,INT2  ;                               |
        MVI     M,0C3H  ;                               |
        LXI     H,LDSAT ;                               |
        SHLD    INT2+1  ;                               |
        LDA     INT3    ; Set up console interrupts so  |
        STA     OLDINT3 ;     keyboard doesn't have to  |
        LHLD    INT3+1  ;     be polled.                |
        SHLD    OLDINT3+1 ;                             |
        LXI     H,CONIN ;                               |
        SHLD    INT3+1  ;                               |
        LXI     H,INT3  ;                               |
        MVI     M,0C3H  ;                               |
        EI              ;                               |
        ENDIF           ; <-----------------------------'
;
        IF      DBG2    ; ------------------------------,
        LXI     D,MSG01 ;Display message on console to |
        MVI     C,09H   ;     increase and decrease     |
        CALL    BDOS    ;     level and slow motion     |
        ENDIF           ;     delay.   <----------------'
;
        CALL    SLEVL
        JMP     BEGIN
;
VINIT:  LXI     B,0200H ; GM II
        CALL    VROUT
        LXI     B,0C201H
        CALL    VROUT
        LXI     B,PNTADR/1024*256+02H
        CALL    VROUT
        LXI     B,0FF03H        ; CGT register in GM II
        CALL    VROUT
        LXI     B,0304H         ; PGT register in GM II
        CALL    VROUT
        LXI     B,SATADR/128*256+05H
        CALL    VROUT
        LXI     B,SGTADR/2048*256+06H
        CALL    VROUT
        LXI     B,0107H
        CALL    VROUT
        CALL    LDPGT   ; Load characters by PNT data.
        CALL    LDCGT
        CALL    LDSGT
        CALL    LDPNT
        CALL    INTPSG
        RET
;
VROUT:  MOV     A,B
        OUT     VDPCTL
        MOV     A,C
        ORI     10000000B
        OUT     VDPCTL
        RET
;       Load Pattern Generator Table CGM II
;       *Taken from FUEL DEPOT by Les Bird*
;       (DE)=OFFSET     (BC)=POINTER
LDPGT:  LXI     H,PGTADR
        LXI     D,0300H
        LXI     B,PNTTBL
        CALL    VMSET
LDPG0:  LDAX    B
        INX     B
        PUSH    D
        CALL    LDPGT0
        POP     D
        DCX     D
        MOV     A,E
        ORA     D
        JNZ     LDPG0
        RET
LDPGT0: LXI     D,PGTTBL
        MOV     L,A
        MVI     H,00H
        DAD H ! DAD H
        DAD     H
        DAD     D
        MVI     E,08H
LDPGT1: MOV     A,M
        OUT     VDPDAT
        INX     H
        DCR     E
        JNZ     LDPGT1
        RET
;       Load Color Generator Table CGM II
;       *Taken from FUEL DEPOT by Les Bird*
;       (DE)=OFFSET     (BC)=COUNT
LDCGT:  LXI     H,CGTADR
        CALL    VMSET
        LXI     H,CGTTBL
        MOV     B,M
        INX     H
        MOV     C,M
        INX     H
        MOV     A,C
        ORA     B
        RZ
        CALL    LDCG1
        JMP     LDCGT+9
LDCG1:  MOV     A,M
        OUT     VDPDAT
        DCX     B
        MOV     A,C
        ORA     B
        JNZ     LDCG1
        INX     H
        RET
;       Load Sprites
;       (DE)    = File FCB data to load.
;       (HL)    = Storage in memory.
LDSGT:  LXI     H,ERMSG1
        SHLD    ERMSG0
        LXI     D,F1LFCB
        LXI     H,SGTTBL
        CALL    RDFILE
        PUSH    H       ; Save music data storage pointer.
        LXI     H,SGTADR
        LXI     D,SGTTBL
        LXI     B,0400H
        CALL    VMSET
        POP     H
INIT4:  LDAX    D
        INX     D
        OUT     0B8H
        DCX     B
        MOV     A,C
        ORA     B
        JNZ     INIT4
        RET
;       Load Sprite Attributes
LDSAT:  IF      INTRP   ; ------.
        DI              ;       |
        PUSH    PSW     ;       |
        PUSH    H       ;       |
        PUSH    D       ;       |
        PUSH    B       ;       |
        ENDIF           ; <-----'
        LXI     H,SATADR
        CALL    VMSET
        LXI     H,SATTBL
        MOV     A,M
        INX     H
        OUT     VDPDAT
        CPI     0D0H
        JNZ     $-6
        IF      INTRP   ; ------.
        JMP     SCORE   ;       |
        ENDIF           ; <-----'
        RET
;       Load Pattern Name Table CGM II
;       *Taken from FUEL DEPOT by Les Bird*
LDPNT:  LXI     H,PNTADR
        CALL    VMSET
        LXI     D,0000H
        MOV     A,E
        OUT     VDPDAT
        INX     D
        MOV     A,D
        CPI     03H
        JNZ     $-7
        RET
;       Read console character if interrupt pending.
CONIN:  IF      INTRP   ; --.
        DI              ;   |
        IN      CSTAT   ;   |
        ANI     CIMSK   ;   |
        JZ      CONIN+1 ;   |
        IN      CDATA   ;   |
        CPI     03H     ;   |
        JZ      REBOOT  ;   |
        STA     CCHAR   ;   |
        EI              ;   |
        ENDIF           ; <-'
;
        IF      NOT INTRP ; --  .
        MVI     E,0FFH  ;       |
        MVI     C,06H   ;       |
        CALL    BDOS    ;       |
        ORA     A       ;       |
        RZ              ;       |
        CPI     03H     ;       |
        JZ      REBOOT  ;       |
        STA     CCHAR   ;       |
        ENDIF           ; <-----'
;
        RET
; Read file from disk.  From SPACEWAR.ASM by Les Bird.
RDFILE: PUSH    H
        LXI     H,000CH
        DAD     D
        MVI     M,00H
        LXI     H,0020H
        DAD     D
        MVI     M,00H
        MVI     C,0FH
        PUSH    D
        CALL    BDOS
        POP     D
        POP     H
        INR     A
        JZ      FLERR
RDFIL0: PUSH    D
        PUSH    H
        MVI     C,14H
        CALL    BDOS
        POP     H
        PUSH    PSW
        LXI     D,0080H
        MVI     C,80H
RDFIL1: LDAX    D
        MOV     M,A
        INX H ! INX D
        DCR     C
        JNZ     RDFIL1
        POP     PSW
        POP     D
        ORA     A
        JZ      RDFIL0
        PUSH    H
        MVI     C,10H
        CALL    BDOS
        POP     H
        RET
FLERR:  LHLD    ERMSG0
        XCHG
        MVI     C,09H
        CALL    BDOS
        JMP     REBOOT
;       Setup Video Memory Address
VMSET:  MOV     A,L
        OUT     VDPCTL
        MOV     A,H
        ORI     01000000B
        OUT     VDPCTL
        RET
;       Return to Operating system
REBOOT: IF      INTRP   ; --.
        DI              ;   |
        LDA     OLDINT2 ;   |
        STA     INT2    ;   |
        LHLD    OLDINT2+1 ; |
        SHLD    INT2+1  ;   |
        LDA     OLDINT3 ;   |
        STA     INT3    ;   |
        LHLD    OLDINT3+1 ; |
        SHLD    INT3+1  ;   |
        EI              ;   |
        ENDIF           ; <-'
        CALL    INTPSG
        LXI     B,0107H
        CALL    VROUT
        JMP     0000H   ; Reboot by JMP to location 0000H.
;       Sound generator routine.
INTPSG: LXI     H,SINIT
MAKSND: PUSH    B
        MOV     C,M
        INX     H
INTPS0: MOV     A,M
        OUT     PSGCTL
        INX     H
        MOV     A,M
        OUT     PSGDAT
        INX     H
        DCR     C
        JNZ     INTPS0
        POP     B
        RET
;       Routine to read Joysticks.
JSTIK:  LXI     H,JDAT1
        LDA     JNUMB   ; Get next J-Stik number to read.
        OUT     JOYCTL  ; Set-up D/A for joystick input.
        MOV     C,A     ; Set up (BC) for offset to JDAT.
        MVI     B,00H
        INR A ! INR A   ; Increment twice to read X-Axis only.
        PUSH    H
        LXI     H,JPLYR ; Number of player's.
        ANA     M
        STA     JNUMB
        POP     H
        IN      JOYDAT  ; Get J-Stik X-Axis value.
        CPI     0D4H    ; Limit joystick input to
        JC      $+8     ; integer > 32 and < 212.
        MVI     A,0D4H
        JMP     $+10
        CPI     020H
        JNC     $+5
        MVI     A,20H
        DAD     B       ; Add offset to JDAT1 for storage.
        MOV     M,A     ; Store value in correct JDAT location.
        MVI     A,0EH
        OUT     PSGCTL
        IN      PSGCTL
        STA     JBUTN
        RET
;       Update player sprite data.
PMOVE:  LDA     PNUMB   ; Current player to move.
        INR     A
        ANI     03H
        STA     PNUMB
        MOV     C,A
        LDA     GMOPT   ; No update if computers
        ANI     0FH     ;  turn to move because they
        CMP     C       ;   update on screen in routine
        RC              ;    CMOVE.
        MOV     A,C
        RLC ! RLC ! RLC ; Multiply by 8 for P1DAT offset.
        MOV     C,A
        MVI     B,00H
        LXI     H,P1DAT
        DAD     B
        PUSH    H       ; (HL)=Pointer to player's data.
PMOV0:  MOV     E,M
        INX     H
        MOV     D,M     ; (DE)=Sprite # in SAT table.
        INX     H
        MOV     C,M
        INX     H
        MOV     B,M     ; (BC)=Points to JDAT data.
        INX     H
        LDAX    D
        CPI     0FFH    ; Dead player ?
        JZ      PMV1    ; Exit if so.
        LDAX    B
        CPI     80H
        JC      PMOV1   ; If JDAT < 128 then PMOV1.
        INX     H       ; else point to > 128 data.
PMOV1:  MOV     A,M
        STAX    D       ; Update SATTBL with new pattern number.
        POP     H
        LXI     D,0006H
        DAD     D       ; (HL)=Address of player movement routine.
        MOV     E,M
        INX     H
        MOV     D,M
        XCHG
        PCHL
PMV1:   POP     H       ; Exit routine if player is dead.
        RET
PMOV2:  LXI     H,SATTBL+12 ; Player #1 (Bottom Left) movement.
        LXI     B,516EH
        LDA     JDAT1
        PUSH    PSW
        CPI     80H
        JC      PMOV3   ; If < 128 then update X.
        POP     PSW
        SUI     11H     ; Else update Y in SATTBL.
        CPI     0B3H
        JC      $+5
        MVI     A,0B3H
        MOV     M,A
        INX     H
        MOV     M,B
        RET
PMOV3:  MOV     M,C     ; X update in SATTBL.
        INX     H
        POP     PSW
        SUI     30H
        CPI     51H
        JC      $+4
        XRA     A
        MOV     M,A
        RET
PMOV4:  LXI     H,SATTBL+8 ; Player #2 (Bottom Right) movement.
        LXI     B,9F6EH
        LDA     JDAT2
        PUSH    PSW
        CPI     80H
        JC      PMOV5
PMV4:   MOV     M,C     ; Adjust X for player 2 & 3.
        INX     H
        POP     PSW
        ADI     20H
        CPI     0F4H
        JC      $+5
        MVI     A,0F4H
        MOV     M,A
        RET
PMOV5:  POP     PSW     ; Adjust Y for player 2.
        SUI     40H
        MOV     C,A
        MVI     A,0B3H
        SUB     C
        CPI     0B3H    ; No greater than 180 on Y axis.
        JC      $+5
        MVI     A,0B3H
        MOV     M,A
        INX     H
        MOV     M,B
        RET
PMOV6:  LXI     H,SATTBL+4 ; Player #3 (Top Right) movement.
        LXI     B,9F40H
        LDA     JDAT3
        PUSH    PSW
        CPI     80H
        JNC     PMV4    ; Perform X movement.
PMOV7:  POP     PSW
        SUI     41H
PMV7:   CPI     41H
        JC      $+5
        MVI     A,0FFH
        MOV     M,A
        INX     H
        MOV     M,B
        RET
PMOV8:  LXI     H,SATTBL ; Player #4 (Top Left) movement.
        LXI     B,5140H
        LDA     JDAT4
        PUSH    PSW
        CPI     80H
        JC      PMOV3
        POP     PSW
        MOV     C,A
        MVI     A,0C0H
        SUB     C
        JMP     PMV7
; Routine to move up to 4 fireballs on the screen at one time.
BMOVE:  LHLD    GMDLY
        MOV     A,L
        ORA     H
        JZ      BMV01
        DCX     H
        SHLD    GMDLY
        MOV     A,H
        ORA     A
        RNZ
        MOV     A,L
        CPI     0AH
        JZ      RLEVL   ; Erase "LEVEL xx" on 000AH
        RET
;
BMV01:  LHLD    RLDLY   ; Release another fireball
        MOV     A,L     ;  when timer is 0000.
        ORA     H
        JZ      BMOV0
        DCX     H
        SHLD    RLDLY
        MOV     A,L
        ORA     H
        JNZ     BMOV0
        LXI     D,CHKDT3
        CALL    CHKPL6  ; Release ball.
;
BMOV0:  LDA     BNUMB   ; Current ball # to move.
        INR     A
        ANI     03H     ; Limit 4 balls only.
        STA     BNUMB
        RLC ! RLC ! RLC
        RLC             ; Multiply by 16 for offset.
        MOV     C,A
        MVI     B,00H
        LXI     H,BDAT1 ; Point to correct ball data block.
        DAD     B
        LXI     D,BDLY  ; Copy BDAT to working area BDLY.
        MVI     C,0AH
        PUSH    H
BMOV1:  MOV     A,M
        STAX    D
        INX     H
        INX     D
        DCR     C
        JNZ     BMOV1
        LDA     BDIR
        CPI     0FFH    ; Ball not released yet.
        JZ      BMVF
        ANI     080H    ; Ball held by player.
        JNZ     BMOV11
        LXI     H,BDLY
        DCR     M
        JNZ     TBALL
        LDA     BTMR
        ANI     0FH
        MOV     M,A     ; Re-new timer.
        LXI     H,BDAT0 ; Point to directional data.
        LDA     BDIR
        MOV     E,A
        MVI     D,00H
        DAD     D
        MOV     C,M
        MOV     A,C
        ANI     70H
        RRC ! RRC ! RRC ; Move to right nibble.
        RRC
        MOV     B,A
        MOV     A,C
        ANI     80H     ; Test for X+ or X-.
        LXI     H,BALX
        JNZ     BMOV2
        MOV     A,B
        ORA     A
        JZ      BMOV3
        MVI     A,0F5H  ; Increment ball X and
BMV1A:  CMP     M       ;  check for right border
        JZ      BMV1B   ;   bounce.
        INR     M
        DCR     B
        JNZ     BMV1A
        JMP     BMOV3
BMV1B:  LDA     BALY    ; Test for left half
        CPI     60H     ;  or right half of screen.
        LXI     H,BDAT5B ; Bottom half data.
        JNC     BMV1C
        LXI     H,BDAT5A ; Top half data.
BMV1C:  DAD     D
        MOV     A,M
        STA     BDIR
        STA     BBNC    ; Set ball bounce flag.
        PUSH    H
        LXI     H,SWALL ; Sound effects for
        CALL    MAKSND  ;  deflection off border.
        POP     H
        JMP     BMOV3
BMOV2:  XRA     A       ; Left border bounce.
BMV2A:  CMP     M
        JZ      BMV2B
        DCR     M
        DCR     B
        JNZ     BMV2A
        JMP     BMOV3
BMV2B:  LDA     BALY
        CPI     60H
        LXI     H,BDAT6B
        JNC     BMV1C
        LXI     H,BDAT6A
        JMP     BMV1C
BMOV3:  MOV     A,C
        ANI     07H
        MOV     B,A
        MOV     A,C
        ANI     08H
        LXI     H,BALY
        JNZ     BMOV4
        MOV     A,B
        ORA     A
        JZ      BMOV5
        MVI     A,0B4H
BMV3A:  CMP     M
        JZ      BMV3B   ; BMV3B if bottom bounce.
        INR     M
        DCR     B
        JNZ     BMV3A
        JMP     BMOV5
BMV3B:  LDA     BALX
        CPI     80H
        LXI     H,BDAT7B ; Right half data.
        JNC     BMV3C
        LXI     H,BDAT7A ; Left half data.
BMV3C:  DAD     D
        MOV     A,M
        STA     BDIR
        STA     BBNC
        PUSH    H
        LXI     H,SWALL
        CALL    MAKSND
        POP     H
        JMP     BMOV5
BMOV4:  MVI     A,0FDH
BMV4A:  CMP     M
        JZ      BMV4B
        DCR     M
        DCR     B
        JNZ     BMV4A
        JMP     BMOV5
BMV4B:  LDA     BALX
        CPI     80H
        LXI     H,BDAT8B
        JNC     BMV3C
        LXI     H,BDAT8A
        JMP     BMV3C
BMOV5:  LHLD    BADR    ; Update sprite X, Y.
        LDA     BALY
        MOV     M,A
        INX     H
        LDA     BALX
        MOV     M,A
        INX     H
        LDA     BDIR    ; Change ball appearance
        RLC ! RLC       ;  as it changes directions.
        ADI     14H
        MOV     M,A
; Check ball against graphics pixels.
        LDA     BBNC    ; Do not check pixels
        ORA     A       ;  if bounce occured.
        JNZ     BMVF
        LDA     BDIR    ; Get ball direction.
        RLC             ; Multiply by 2
        MOV     E,A
        MVI     D,00H   ; (DE)=(A)*2
        LXI     H,BDAT9
        DAD     D
        MOV     E,M
        INX     H
        MOV     D,M     ; (DE)=BQ1+offset.  (Pixels to add)
        RLC ! RLC       ; (A)=BDIR1*8
        MOV     C,A
        MVI     B,00H
        LXI     H,BDAT11
        DAD     B
        MOV     C,L
        MOV     B,H     ; (BC)=BQ2+offset.  (Reflected direction)
        MVI     A,08H
        STA     BWRK
BMV6:   LHLD    BALY
        LDAX    D
        ANI     0F0H
        RAR ! RAR ! RAR
        RAR             ; Get X offset.
        ADD     H
        MOV     H,A     ; XBALL1=XBALL1+XOFFSET
        LDAX    D
        ANI     0FH
        ADD     L       ; YBALL1=YBALL1+YOFFSET
        MOV     L,A
        SHLD    CHKDT2
        PUSH    B
        PUSH    D
        XCHG
        MVI     B,00H
        MOV     A,E
        MOV     C,A
        ANI     07H
        MOV     L,A
        MOV     A,D
        ANI     0F8H
        ORA     L
        DI              ; Dont let sprite updating
        OUT     VDPCTL  ; interfere.
        MOV     L,A
        MOV     A,C
        ANI     0F8H
        RAR ! RAR ! RAR
        OUT     VDPCTL
        MOV     H,A
        PUSH    H
        LXI     H,PXLDAT
        MOV     A,B
        ANI     07H
        MOV     E,A
        DAD     D
        IN      VDPDAT
        EI
        ANA     M       ; Test pixel.
        POP     H
        JNZ     BMV8    ; Turn group of pixels off if true.
        POP     D
        POP     B       ; Else, check next pixel.
BMV7:   INX     D
        INX     B
        LXI     H,BWRK
        DCR     M
        JZ      BMVF
        JMP     BMV6
BMV8:   MOV     A,L
        ANI     0F8H    ; Round off to nearest block.
        DI
        OUT     VDPCTL
        MOV     A,H
        ORI     40H
        OUT     VDPCTL
        XRA     A
        MVI     C,08H
        OUT     VDPDAT
        DCR     C
        JNZ     $-3
        EI
        POP     D
        POP     B
        LDAX    B
        STA     BDIR
        LDA     BHIT
        ANI     0FH
        STA     BHIT
        MVI     A,08H
        STA     BTMR
        LHLD    RLDLY
        MOV     A,L
        ORA     H
        JZ      BMV9
        LXI     H,1000H
        SHLD    RLDLY
BMV9:   LXI     H,SBRIK+2 ; Sound effects for
        MVI     M,01FH  ;  hit on castle.
        INX H ! INX H
        MVI     M,0EH
        INX H ! INX H
        MVI     M,0FH
        LDA     SNDBIT
        ORI     01H
        STA     SNDBIT
        CALL    CHKPL5  ; No score if player hits
        JC      BMVA    ;  himself with fireball.
        CALL    BRKSCR
BMVA:   CALL    CHKPLR  ; Check to see if inside
        LXI     H,0000H ;  of castle is hit for each
        SHLD    CHKDT2  ;   player.
BMVF:   XRA     A
        STA     BNUM1
        STA     BBNC
BMOV0F: POP     H
        LXI     D,BDLY
        MVI     C,0AH
BMOV6:  LDAX    D
        MOV     M,A
        INX     H
        INX     D
        DCR     C
        JNZ     BMOV6
        LDA     GMOPT   ; If level completed, then
        ANI     80H     ;  RESET screen for new level.
        JNZ     RESET
        LDA     BNUMB   ; If ball number rolls over
        ORA     A       ;  to zero after ANI 03H then
        JNZ     BMOV0   ;   RET, else JMP BMOV0.
        RET
; Check ball against warlord shield.
TBALL:  LXI     H,BTMR  ; If ball was just
        MOV     A,M     ;  thrown, then skip
        ANI     0FH     ;   paddle check.
        MOV     C,A 
        MOV     A,M
        ANI     0F0H
        JZ      TBAL0
        SUI     10H
        ORA     C
        MOV     M,A
        JMP     BMOV0F
TBAL0:  LDA     BPLR    ; Current player to test
        INR     A       ;  ball against.
        ANI     0F3H
        STA     BPLR
        LXI     H,PLYR1
        ORA     A
        JZ      BMOV09
        LXI     H,PLYR2
        DCR     A
        JZ      BMOV09
        LXI     H,PLYR3
        DCR     A
        JZ      BMOV09
        LXI     H,PLYR4
BMOV09: PUSH    H       ; Save pointer to player data.
        INX H ! INX H   ; Determine which way
        XRA     A       ;  player paddle is facing.
        MOV     A,M
        POP     H
        CPI     0FFH    ; Player dead?
        JZ      BMOV0F  ; Exit if true.
        RRC ! RRC
        DCR     A       ; Check for zero paddle position.
        JZ      BMOV0A  ; If facing right then BMOV0A.
        DCR     A
        JZ      BMOV0B  ; down.
        DCR     A
        JZ      BMOV0C  ; left.
BMOV0D: LDA     BALY    ; Execute this if paddle is up.
        SUB     M
        CMA
        ADI     03H
        CPI     0EH
        JNC     BMOV0F  ; Ball is not in range.
        INX     H       ; Point to ball X.
        MOV     C,M
        LDA     BALX
        LXI     H,BRFL12
        CALL    BMOVD0
        JC      BMOV10
        JMP     BMOV0F
BMOV0C: INX     H       ; Test X first to see
        LDA     BALX    ;  if ball is in X range.
        SUB     M       ; This is paddle left test.
        CMA
        ADI     03H
        CPI     0EH
        JNC     BMOV0F
        DCX     H
        MOV     C,M
        LDA     BALY
        LXI     H,BRFL08
        LDA     BALY
        CALL    BMOVD0
        JC      BMOV10
        JMP     BMOV0F
BMOV0B: LDA     BALY    ; Execute this if paddle is down.
        SUB     M       ; Point to player X.
        ADI     08H
        CPI     0EH
        JNC     BMOV0F
        INX     H
        MOV     C,M
        LDA     BALX
        LXI     H,BRFL04
        CALL    BMOVD0
        JC      BMOV10
        JMP     BMOV0F
BMOV0A: INX     H       ; Execute this if paddle is right.
        LDA     BALX    ; Get player Y then point to X.
        SUB     M
        CMA
        ADI     08H
        CPI     0EH
        JNC     BMOV0F  ; Ball is not
        DCX     H       ;  in range with
        MOV     C,M     ;   paddle.
        LDA     BALY
        LXI     H,BRFL00
        CALL    BMOVD0
        JNC     BMOV0F
BMOV10: LDA     BPLR    ; Get player number.
        ANI     03H
        LXI     B,0101H
        ORA     A
        JZ      BMV09
        LXI     B,0204H
        DCR     A
        JZ      BMV09
        LXI     B,0310H
        DCR     A
        JZ      BMV09
        LXI     B,0440H
BMV09:  LDA     JBUTN   ; See if player is
        ANA     C       ;  already holding a
        JNZ     BMV10   ;   ball and if so then
        LDA     BHLD1   ;    just richochet ball
        CMP     B       ;     normally.
        JZ      BMV10
        LDA     BHLD2
        CMP     B
        JZ      BMV10
        LDA     BHLD3
        CMP     B
        JZ      BMV10
        LDA     BHLD4
        CMP     B
        JZ      BMV10
        MOV     A,B
        STA     BHLD    ; Player number holding
        STA     BHIT    ;  and last to hit ball.
        LDA     BDIR    ; Set hold bit
        ORI     80H     ;  in ball direction
        STA     BDIR    ;   byte.
        JMP     BMOV0F
BMV10:  LDA     BHIT    ; Test ball-paddle hit so
        ADI     10H     ;  ball does not get in endless
        STA     BHIT    ;   loop.
        JNC     $+4
        INR     M
        MOV     A,M
        ANI     0FH
        STA     BDIR
        LDA     BPLR
        ANI     03H
        INR     A
        MOV     C,A
        LDA     BHIT
        ANI     0F0H
        ORA     C
        STA     BHIT
        LDA     BTMR
        CPI     02H
        JZ      BMV11
        DCR     A
        STA     BTMR
BMV11:  LXI     H,SPADL+2
        MVI     M,10H
        INX H ! INX H
        MVI     M,0EH
        INX H ! INX H
        MVI     M,0FH
        LDA     SNDBIT
        ORI     04H
        STA     SNDBIT
        JMP     BMOV0F
;
BMOVD0: SUB     C
        MOV     B,A
        LXI     D,BRFLCT
BMOVD1: LDAX    D
        CPI     080H    ; End of range data.
        RZ
        CMP     B
        JZ      BMOVD2
        INX D ! INX H
        JMP     BMOVD1
BMOVD2: STC
        RET
; Execute this routine if ball is being held.
BMOV11: LDA     BHLD    ; Get player number
        LXI     H,PLYR1 ;  who is holding ball.
        MVI     C,01H
        DCR     A
        JZ      BMOV12
        LXI     H,PLYR2
        MVI     C,04H
        DCR     A
        JZ      BMOV12
        LXI     H,PLYR3
        MVI     C,10H
        DCR     A
        JZ      BMOV12
        LXI     H,PLYR4
        MVI     C,40H
BMOV12: LDA     JBUTN
        ANA     C
        JNZ     BMOV13
        PUSH    H
        LHLD    BADR
        XCHG
        POP     H
        MOV     A,M
        STA     BALY
        STAX    D       ; Update sprite Y.
        INX     H
        INX     D
        MOV     A,M
        STA     BALX
        STAX    D       ; Update sprite X.
        INX     D
        LDA     BPLR
        SUI     10H
        LDA     BDIR
        JNC     $+4
        INR     A
        MOV     C,A
        ANI     0FH
        ORI     80H
        STA     BDIR
        MOV     A,C
        RLC ! RLC
        ADI     14H
        STAX    D       ; Update sprite number.
        JMP     BMOV0F
BMOV13: INX H ! INX H
        LDA     BHLD
        DCR     A
        JZ      BMOV17  ; Player #1 throw.
        DCR     A
        JZ      BMOV16  ; Player #2.
        DCR     A
        JZ      BMOV15  ; #3.
BMOV14: MOV     A,M     ; Get paddle right or down
        CPI     04H     ;  position.
        JZ      BMV14A
        LXI     H,BTHR04 ; Paddle is down.
        LDA     PLYR4+1
        JMP     BMV14B
BMV14A: LXI     H,BTHR04+12
        LDA     PLYR4
BMV14B: ANI     0F8H
        RRC ! RRC ! RRC ; Divide by 8.
        MOV     E,A
        MVI     D,00H
        DAD     D
        JMP     BMOV18
BMOV15: MOV     A,M
        CPI     08H
        JZ      BMV15A  ; Paddle is down.
        LXI     H,BTHR03+12
        LDA     PLYR3
        JMP     BMV14B
BMV15A: LXI     H,PLYR3+1
        MVI     A,0FFH
        SUB     M
        LXI     H,BTHR03
        JMP     BMV14B
BMOV16: MOV     A,M
        CPI     0CH
        JZ      BMV16A  ; Paddle is left
        LXI     H,PLYR2+1 ; else paddle is up.
        MVI     A,0FFH
        SUB     M
        LXI     H,BTHR02
        JMP     BMV14B
BMV16A: LXI     H,PLYR2
        MVI     A,0BFH
        SUB     M
        LXI     H,BTHR02+12
        JMP     BMV14B
BMOV17: MOV     A,M
        CPI     04H
        JZ      BMV17A  ; Jump if paddle is right.
        LXI     H,BTHR01
        LDA     PLYR1+1
        JMP     BMV14B
BMV17A: LXI     H,PLYR1
        MVI     A,0BFH
        SUB     M
        LXI     H,BTHR01+12
        JMP     BMV14B
BMOV18: MOV     A,M
        STA     BDIR
        XRA     A
        STA     BHLD
        MVI     A,62H   ;  6xH is delay to skip paddle check.
        STA     BTMR
        LXI     H,STHRO+4
        MVI     M,00H   ; Channel (A) pitch (fine)
        INX H ! INX H
        MVI     M,04H   ; Channel (A) pitch (course).
        INX H ! INX H
        MVI     M,0FH   ; Channel (A) amplitude.
        LDA     SNDBIT
        ORI     08H
        STA     SNDBIT
        JMP     BMOV0F
; Move computer player's
CMOVE:  LDA     CBNUM   ; Get computer # to move.
        INR     A
        ANI     03H
        STA     CBNUM
        RZ
        DCR     A
        JZ      C2MOV
        DCR     A
        JZ      C3MOV
C4MOV:  LDA     GMOPT   ; .0000 contains number of
        ANI     0FH     ;  human players.
        CPI     03H
        RNC
        LDA     PLYR4+2
        CPI     0FFH
        RZ
        LXI     D,PLYR4
        LXI     H,CJDT4
        JMP     CMOV0
C3MOV:  LDA     GMOPT
        ANI     0FH
        CPI     02H
        RNC
        LDA     PLYR3+2
        CPI     0FFH
        RZ
        LXI     D,PLYR3
        LXI     H,CJDT3
        JMP     CMOV0
C2MOV:  LDA     GMOPT
        ANI     0FH
        CPI     01H
        RNC
        LDA     PLYR2+2
        CPI     0FFH
        MVI     A,01H
        RZ
        LXI     D,PLYR2
        LXI     H,CJDT2
; Move computer player #4.
CMOV0:  SHLD    COMHL   ; Determine which ball
        XCHG            ;  is closest to castle
        SHLD    COMDE   ;   and defend against it.
        LXI     H,BALL1
        SHLD    BDST1+1
        CALL    CMOV1
        STA     BDST1
        LXI     H,BALL2
        SHLD    BDST2+1
        CALL    CMOV1
        STA     BDST2
        LXI     H,BALL3
        SHLD    BDST3+1
        CALL    CMOV1
        STA     BDST3
        LXI     H,BALL4
        SHLD    BDST4
        CALL    CMOV1
        LXI     H,BDST3
        CMP     M
        JNC     CMOV00  ; Ball #3 closer.
        MOV     M,A     ; Put #4 data in
        LHLD    BDST4   ;  #3 space.
        SHLD    BDST3+1
CMOV00: LXI     H,BDST1 ; Compare ball #1 & #2.
        LDA     BDST2
        CMP     M
        JNC     CMOV01
        MOV     M,A
        LHLD    BDST2+1
        SHLD    BDST1+1
CMOV01: LXI     H,BDST1
        LDA     BDST3
        CMP     M
        JNC     CMOV02
        LHLD    BDST3+1
        SHLD    BDST1+1
CMOV02: LDA     LEVEL
        CPI     05H
        JC      $+5
        MVI     A,05H
        RLC
        STA     COMCNT
CMV02B: LHLD    BDST1+1
        MOV     C,L
        MOV     B,H
        LHLD    COMDE
        XCHG
        LHLD    COMHL
        LDA     CBNUM
        CPI     01H
        JNZ     CMV02A
        LDAX    B       ; Decide if Y needs adjusting
        CPI     70H     ;  for player#2. (Bottom right)
        JNC     CMOV03
        JMP     CMOV06
CMV02A: LDAX    B       ; Get ball Y
        CPI     40H
        JNC     CMOV06
CMOV03: MOV     C,A     ; Adjust player Y.
        LDA     CBNUM
        CPI     02H
        JZ      CMOV07  ; Move player #3 (Top right).
        LDAX    D       ; Y move for player#2 (BR) & #4 (TL).
        INR     A       ; CYCO 0FFh=000h (keeps#4 from sleeping).
        CMP     C       ; C=ball Y axis.
        RZ
        JNC     CMOV05
        JMP     CMOV04
CMOV07: LDAX    D       ; Y move for player#3 (TR).
        ADI     01H
        CMP     C
        RZ
        JC      CMOV05
CMOV04: MOV     A,M
        CPI     21H
        RC
        DCR     M
        JMP     CMOV0F  ; Move computers until LEVEL counter
CMOV05: MOV     A,M     ;  is zero or until even with ball.
        CPI     0D3H
        RNC
        INR     M
        JMP     CMOV0F
CMOV06: INX B ! INX D   ; Advance ball pointer and
        LDA     CBNUM   ;  player pointer.
        CPI     03H
        JZ      CMOV08
        LDAX    B       ; Move player#2 & #3 (Bottom/top right).
        CPI     0A0H
        RC
        ADI     02H
        JMP     CMOV09
CMOV08: LDAX    B       ; Move player#4 X. (top left)
        CPI     58H
        RNC
CMOV09: MOV     C,A
        LDAX    D
        CMP     C
        RZ
        JNC     CMOV04
        JMP     CMOV05
CMOV0F: LDA     CBNUM
        DCR     A
        JNZ     C0MOV
        LHLD    CJDT2
        SHLD    JDAT2
        LXI     H,P2DAT
        JMP     CMV0F
C0MOV:  DCR     A
        JNZ     C1MOV
        LHLD    CJDT3
        SHLD    JDAT3
        LXI     H,P3DAT
        JMP     CMV0F
C1MOV:  LHLD    CJDT4
        SHLD    JDAT4
        LXI     H,P4DAT
CMV0F:  CALL    PMOV0-1 ; Update sprite on screen.
        LXI     H,COMCNT ; Get times to repeat computer
        DCR     M       ;  move according to LEVEL.
        JNZ     CMV02B
        RET
; Compute ball Y distance for player#3 & #4.
CMOV1:  MOV     A,M
        CPI     0D0H
        RZ
        LDA     CBNUM
        CPI     01H
        MOV     A,M
        JNZ     CMV2A
        MVI     A,78H   ; Compute 120-Y for player#2.
        SUB     M
        JNC     CMOV2
        XRA     A
        JMP     CMOV2
CMV2A:  SUI     48H     ; Y-72
        JNC     CMOV2
        XRA     A
CMOV2:  MOV     C,A
        LDA     CBNUM
        CPI     03H
        JC      CMOV5
; Compute ball X distance for player#1 & #4.
CMOV3:  INX     H
        MOV     A,M
        SUI     50H     ; X-80
        JNC     CMOV4
        XRA     A
CMOV4:  ADD     C
        RET
; Compute ball X distance for player#2 & #3.
CMOV5:  INX     H
        MVI     A,0ABH
        SUB     M       ; Ball X-168.
        JNC     CMOV6
        XRA     A
CMOV6:  ADD     C
        RET
;       Check to see if player killed.
CHKPLR: LHLD    CHKDT2  ; Get ball X, Y.
        XCHG
        MOV     A,D
        LXI     H,CHKDT0
        CPI     30H     ; Is ball on player#1 or #4 side?
        JC      CHKPL1  ;  Jump CHKPL1 if so,
        LXI     H,CHKDT1;  else check for player#2 or #3.
        CPI     0D0H
        JNC     CHKPL1
        RET
CHKPL1: LXI     B,000AH
        MOV     A,E
        CPI     20H
        JC      CHKPL2
        DAD     B
        CPI     0A0H
        RC
CHKPL2: MOV     E,M
        INX     H
        MOV     D,M
        INX     H
        MOV     A,M
        STAX    D       ; Put paddle in dead (Y spot)
        INX D ! INX H   ;  location off of screen.
        MOV     A,M
        STAX    D       ; X spot.
        INX D ! INX H
        MVI     A,0FFH
        STAX    D       ; Put dead byte in sprite #.
        MOV     E,M
        INX     H
        MOV     D,M
        INX     H
        LDA     PLCNT   ; Set player bit to 1 in PLCNT
        ORA     M       ;  to signify that player is dead.
        STA     PLCNT
        INX     H
        XCHG
        LXI     B,0430H
        DI
        PUSH    D
        CALL    VMSET
CHKPL3: XRA     A
        OUT     VDPDAT
        DCR     C
        JNZ     CHKPL3
        DCR     B
        JZ      CHKPL4
        LXI     D,0100H
        DAD     D
        MVI     C,30H
        JMP     CHKPL3-3
CHKPL4: EI
        LXI     H,SKILL+2 ;  hits himself with fireball.
        MVI     M,10H
        INX H ! INX H
        MVI     M,0EH
        INX H ! INX H
        MVI     M,0FH 
        LDA     SNDBIT
        ORI     02H
        STA     SNDBIT
        CALL    CHKPL5  ; Check for no score awarded
        JC      CHK04B  ;   if player hits self.
        CALL    KILSCR  ; Warlord killed.
CHK04B: POP     D
        LDA     GMOPT   ; Check number of human players
        ANI     0FH     ;  and test to see if they're
        MVI     C,01H   ;   dead.  JMP to CHKPL9 if true.
        JZ      CHK04A
        DCR     A
        MVI     C,03H
        JZ      CHK04A
        MVI     C,07H
CHK04A: LDA     PLCNT
        ANI     0FH
        ANA     C
        CMP     C
        JZ      CHKPL9  ; JMP CHKPL9 if all human opponents
        LDA     PLCNT   ;  are eliminated.
        ADI     10H
        STA     PLCNT
        ANI     0F0H
        CPI     30H     ; All three others dead?
        JNZ     CHKPL6  ;  release ball if not.
        LDA     GMOPT
        ORI     80H
        STA     GMOPT
        RET
CHKPL5: LHLD    CHKDT2  ; If player hits self, then return
        LDA     BHIT    ;  with CY so no score awarded.
        DCR     A
        JZ      CHK05D
        DCR     A
        JZ      CHK05C
        DCR     A
        JZ      CHK05B
CHK05A: MOV     A,H     ; Check player #4.
        CPI     80H
        RNC
        MOV     A,L
        CPI     60H
        RET
CHK05B: MVI     A,80H   ; Check player #3.
        CMP     H
        RNC
        MOV     A,L
        CPI     60H
        RET
CHK05C: MVI     A,80H   ; Check player #2.
        CMP     H
        RNC
        MVI     A,60H
        CMP     L
        RET
CHK05D: MOV     A,H     ; Check player #1.
        CPI     80H
        RNC
        MVI     A,60H
        CMP     L
        RET
CHKPL6: LXI     H,BDAT2+2 ; Release ball after kill.
        MOV     A,M     ; Get ball 2 direction
        CPI     0FFH    ;  to see if still not released
        JZ      CHKPL7  ;   and release if so.
        LXI     H,BDAT3+2
        MOV     A,M
        CPI     0FFH
        JZ      CHKPL7
        LXI     H,BDAT4+2
CHKPL7: LDAX    D       ; Get ball release direction.
        MOV     M,A
        INX H ! INX D
        LDAX    D       ; Get ball Y
        MOV     M,A
        INX H ! INX D
        LDAX    D
        MOV     M,A     ; and ball X.
CHKPL8: LHLD    BDAT4+3
        SHLD    BALL4
        LHLD    BDAT3+3
        SHLD    BALL3
        LHLD    BDAT2+3
        SHLD    BALL2
        LHLD    BDAT1+3
        SHLD    BALL1
        RET
; Game Over routine.
CHKPL9: LDA     GMOPT
        ORI     40H     ; Set GAME OVER bit.
        STA     GMOPT
        RET
;       Routine to update player's score by interrupt.
SCORE:  LXI     H,SCRDT
        MOV     A,M
        INR     A
        ANI     03H
        MOV     M,A
        LDA     GMOPT
        ANI     0FH     ; # of human players.
        CMP     M
        JC      SCOR0F  ; No update for computers.
        MOV     A,M
        RLC ! RLC ! RLC ;
        RLC             ; (A)*16.
        MOV     E,A
        MVI     D,00H
        LXI     H,PSCR1A
        DAD     D
        SHLD    SCRPLR  ; (HL)=Current displayed score.
        LXI     D,0008H
        DAD     D
        XCHG            ; (DE)=Projected score.
        LHLD    SCRPLR
SCOR00: INX D ! INX H
        LDAX    D
        ORA     A
        JZ      SCOR0D  ; Exit routine if no change in score.
        CMP     M
        JZ      SCOR00  ; If change, then jump SCOR2.
SCOR01: LHLD    SCRPLR
        CALL    SCOR04
SCOR02: LDA     SCRDT
        LXI     H,SCRADR ; Player score addresses in VRAM.
        RLC
        MOV     E,A
        MVI     D,00H
        DAD     D
        MOV     E,M
        INX     H
        MOV     D,M
        XCHG            ; (HL)=Player score area in VRAM.
        CALL    VMSET
        LHLD    SCRPLR
        CALL    SCOR03
        JMP     SCOR0F
SCOR03: INX     H
        MOV     A,M
        ORA     A
        RZ
        PUSH    H
        CALL    LDPGT0
        POP     H
        JMP     SCOR03
SCOR0D: LDA     SCRDT
        MVI     C,06H
        LHLD    SCRPLR
        INX     H
        XCHG
        LXI     H,PNT024
        ORA     A
        JZ      SCOR0E
        LXI     H,PNT024+26
        DCR     A
        JZ      SCOR0E
        LXI     H,PNTTBL+26
        DCR     A
        JZ      SCOR0E
        LXI     H,PNTTBL
SCOR0E: LDAX    D
        MOV     M,A
        INX D ! INX H
        DCR     C
        JNZ     SCOR0E
;
SCOR0F: IF      INTRP   ; ----------------------.
        POP     B       ;                       |
        POP     D       ;                       |
        POP     H       ;                       |
        POP     PSW     ;                       |
OLDINT2 JMP     0000H   ; Filled in by START:   |
        ENDIF           ; <---------------------'
        RET
;
SCOR04: LXI     D,0006H
        DAD     D       ; Look at last digit first.
        MOV     A,M
        ADI     05H
SCOR05: MOV     M,A
        CPI     16H
        RC              ; If no flip of digit, just display
        MVI     M,0CH   ;  else reset current dig to zero
        DCX     H       ;   and increase previous one.
        MOV     A,M
        ORA     A
        RZ
        INR     A
        JMP     SCOR05
; Bonus 5000 for last survivor.
BONUS:  LDA     PLCNT
        LXI     H,PSCR1B
        LXI     D,LEVL1
        ANI     01H
        JZ      BONUS0
        LDA     PLCNT
        LXI     H,PSCR2B
        LXI     D,LEVL2
        ANI     02H
        JZ      BONUS0
        LDA     PLCNT
        LXI     H,PSCR3B
        LXI     D,LEVL3
        ANI     04H
        JZ      BONUS0
        LXI     H,PSCR4B
        LXI     D,LEVL4
BONUS0: LDAX    D       ; Update level's player has
        INR     A       ;  completed.
        STAX    D
        INX H ! INX H
        INX     H       ; Set (HL) to thousands unit.
        MVI     C,05H
BONUS1: MOV     A,M     ; Add 1 to digit pointed
        INR     A       ;  to by (HL).
        PUSH    H
        CALL    SCOR05
        POP     H
        DCR     C
        JNZ     BONUS1
        RET
; Update score for player who hit ball last.
KILSCR: LDA     GMOPT
        ANI     0FH
        INR     A
        LXI     H,BHIT
        CMP     M
        RC
        CALL    CHKSCR
        RC
        INX H ! INX H   ; Set (HL) to thousands unit.
        INX     H
        MVI     C,02H
        JMP     BONUS1
; Brick score.
BRKSCR: LDA     GMOPT
        ANI     0FH
        INR     A
        LXI     H,BHIT
        CMP     M
        RC
        CALL    CHKSCR
        RC
        PUSH    H
        CALL    SCOR04
        POP     H
        LXI     D,0005H
        DAD     D
        MVI     C,02H
        JMP     BONUS1
; Check to see that player is not already dead.
CHKSCR: MOV     A,M
        DCR     A
        MOV     L,A
        LXI     D,SATTBL+14
        JZ      CHKSC0
        LXI     D,SATTBL+10
        DCR     A
        JZ      CHKSC0
        LXI     D,SATTBL+6
        DCR     A
        JZ      CHKSC0
        LXI     D,SATTBL+2
CHKSC0: LDAX    D
        CPI     0FFH
        JNZ     CHKSC1
        STC
        RET
CHKSC1: MVI     H,00H
        DAD H ! DAD H   ; BHIT*4.
        DAD H ! DAD H   ; BHIT*16.
        LXI     D,PSCR1B
        DAD     D
        XRA     A
        RET
; Reset game.
RESET:  DI
        LXI     B,0107H
        CALL    VROUT
        CALL    LDPGT
        CALL    LDCGT
        CALL    BONUS   ; 5000 bonus for last survivor.
        LXI     H,LEVEL ; Increase level
        INR     M       ;  of play.
        LXI     H,LVLNUM+1
        INR     M
        CALL    DLEVL   ; Display level blocks.
        CALL    SLEVL   ; Display "LEVEL xx" message.
        LDA     GMOPT
        ANI     07FH
        STA     GMOPT   ; Reset game options.
        XRA     A
        STA     PLCNT
        STA     BHLD1
        STA     BHLD2
        STA     BHLD3
        STA     BHLD4
        MVI     A,0FH
        STA     BDAT1+2
        MVI     A,50H
        STA     BALL1+2
        MVI     A,0FFH
        STA     BDAT2+2 ; Kill the rest
        STA     BDAT3+2 ;  of the fireballs.
        STA     BDAT4+2
        LXI     H,7858H
        SHLD    BDAT1+3
        LXI     H,0D0D0H
        SHLD    BDAT2+3 ; Sprite delimeters.
        SHLD    BDAT3+3
        SHLD    BDAT4+3
        MVI     A,03H
        STA     BHIT
        STA     BDAT1+7 ; Reset last player hit.
        STA     BDAT2+7
        STA     BDAT3+7
        STA     BDAT4+7
        MVI     A,06H
        STA     BDAT1+1 ; Reset ball speed.
        STA     BDAT2+1
        STA     BDAT3+1
        STA     BDAT4+1
        LXI     H,0800H ; Game delay between levels.
        SHLD    GMDLY
        LXI     H,1000H
        SHLD    RLDLY
        LXI     H,PLNEW
        LXI     D,PLYR4
        MVI     C,10H
REST0:  MOV     A,M
        STAX    D
        INX H ! INX D
        DCR     C
        JNZ     REST0
        EI
        JMP     CHKPL8
; Display level blocks for each player.
DLEVL:  LDA     GMOPT
        ANI     0FH
        MOV     B,A
        CPI     03H
        JC      D1LVL
        LXI     H,0100H
        CALL    VMSET
        LDA     LEVL4
        CALL    D4LVL
D1LVL:  MOV     A,B
        CPI     02H
        JC      D2LVL
        LXI     H,01D0H
        CALL    VMSET
        LDA     LEVL3
        CALL    D4LVL
D2LVL:  MOV     A,B
        CPI     01H
        JC      D3LVL
        LXI     H,16D0H
        CALL    VMSET
        LDA     LEVL2
        CALL    D4LVL
D3LVL:  LXI     H,1600H
        CALL    VMSET
        LDA     LEVL1
D4LVL:  MOV     C,A     ; Display player level's
        DCR     C       ;  completed.
        RZ
        MVI     A,31H
        CALL    LDPGT0
        JMP     D4LVL+1
; Show "LEVEL xx" message on monitor.
SLEVL:  LXI     H,LVLNUM+1
        MOV     A,M
        CALL    SCOR05  ; Adjust level numerals.
        LXI     H,0A60H
        LXI     D,LVLMSG
        LXI     B,LVCOLR
        JMP     GMOV01  ; Display message.
; Reset "LEVEL xx" message.
RLEVL:  LXI     H,0A60H
        LXI     D,LVLRST
        LXI     B,LVCOLR
        JMP     GMOV01
; Sound generator.
SNDGN:  LDA     SNDBIT  ; Brick hit sound.
        ANI     01H
        JZ      SNDG1
        LXI     H,SDLY0
        DCR     M
        JNZ     SNDG1
        MVI     M,08H
        LXI     H,SBRIK+6
        DCR     M
        JNZ     SNDG0
        LDA     SNDBIT
        ANI     0FEH
        STA     SNDBIT
SNDG0:  LXI     H,SBRIK
        CALL    MAKSND
SNDG1:  LDA     SNDBIT  ; Kill warlord sound.
        ANI     02H
        JZ      SNDG3
        LXI     H,SDLY1
        DCR     M
        JNZ     SNDG3
        MVI     M,20H
        LXI     H,SKILL+6
        DCR     M
        JNZ     SNDG2
        LDA     SNDBIT
        ANI     0FDH
        STA     SNDBIT
SNDG2:  LXI     H,SKILL
        CALL    MAKSND
SNDG3:  LDA     SNDBIT  ; Paddle hit sound.
        ANI     04H
        JZ      SNDG5
        LXI     H,SDLY2
        DCR     M
        JNZ     SNDG5
        MVI     M,08H
        LXI     H,SPADL+6
        DCR     M
        JNZ     SNDG4
        LDA     SNDBIT
        ANI     0FBH
        STA     SNDBIT
SNDG4:  LXI     H,SPADL
        CALL    MAKSND
SNDG5:  LDA     SNDBIT  ; Throw fireball sound.
        ANI     08H
        RZ
        LXI     H,SDLY3
        DCR     M
        RNZ
        MVI     M,08H
        LXI     D,0030H
        LHLD    STHRO+10
        DAD     D
        SHLD    STHRO+10
        MOV     A,L
        STA     STHRO+4
        MOV     A,H
        STA     STHRO+6
        LXI     H,STHRO+9
        DCR     M
        JNZ     SNDG6
        MVI     M,10H
        LDA     SNDBIT
        ANI     0F7H
        STA     SNDBIT
        LXI     H,STHRO+8
        MVI     M,00H
        LXI     H,0300H
        SHLD    STHRO+10
SNDG6:  LXI     H,STHRO
        JMP     MAKSND
; Reset game.
GMOVER: LXI     H,SINIT
        CALL    MAKSND
        LXI     H,0160H
        LXI     D,G0VMSG ; "WARLORDS"
        LXI     B,G0COLR ; Color for "WARLORDS"
        CALL    GMOV01
        LXI     H,0378H
        LXI     D,G1VMSG ; "   BY   "
        LXI     B,G1COLR
        CALL    GMOV01
        LXI     H,0560H
        LXI     D,G2VMSG ; "LES BIRD"
        LXI     B,G2COLR
        CALL    GMOV01
        LXI     H,0858H
        LXI     D,G3VMSG ; "GAME OVER"
        LXI     B,G3COLR
        CALL    GMOV01
        LXI     H,0B10H
        LXI     D,G4VMSG ; "S = START GAME"
        LXI     B,G4COLR
        CALL    GMOV01
        LXI     H,0B90H
        LXI     D,G5VMSG ; "Q = QUIT GAME"
        LXI     B,G5COLR
        CALL    GMOV01
        JMP     GMOV03
; Display message on VDP.
GMOV01: DI
        CALL    VMSET
        PUSH    H
        PUSH    B
GMOV02: LDAX    D
        CPI     0FFH
        JZ      GV0201
        PUSH    D
        CALL    LDPGT0
        POP     D
        INX     D
        JMP     GMOV02
GV0201: POP     B
        POP     H
        LXI     D,2000H
        DAD     D
        CALL    VMSET
        MOV     L,C
        MOV     H,B
        JMP     LDCGT+9
GMOV03: LXI     H,0000H
        CALL    VMSET
        LXI     H,PSCR4A
        CALL    SCOR03
        LXI     H,00D0H
        CALL    VMSET
        LXI     H,PSCR3A
        CALL    SCOR03
        LXI     H,1700H
        CALL    VMSET
        LXI     H,PSCR1A
        CALL    SCOR03
        LXI     H,17D0H
        CALL    VMSET
        LXI     H,PSCR2A
        CALL    SCOR03
        EI
        LXI     B,0107H
;
GMOV04: IF      NOT INTRP ; ----.
        PUSH    B       ;       |
        CALL    CONIN   ;       |
        POP     B       ;       |
        ENDIF           ; <-----'
        LDA     CCHAR
        CPI     'S'
        JZ      GMOV05
        CPI     'Q'
        JZ      REBOOT
        LHLD    MSGDLY
        DCX     H
        SHLD    MSGDLY
        MOV     A,H
        ORA     L
        JNZ     GMOV04
        LHLD    MSGDL0
        SHLD    MSGDLY
        MOV     A,B
        INR     A
        ANI     0FH
        MOV     B,A
        DI
        CALL    VROUT
        EI
        JMP     GMOV04
GMOV05: XRA     A
        STA     CCHAR
        STA     LEVEL
        INR     A
        STA     LEVL1
        STA     LEVL2
        STA     LEVL3
        STA     LEVL4
        LDA     GMOPT
        ANI     0FH
        STA     GMOPT
        LXI     H,0C0CH
        SHLD    LVLNUM
        SHLD    PSCR1A+1
        SHLD    PSCR1A+3
        SHLD    PSCR1A+5
        SHLD    PSCR1B+1
        SHLD    PSCR1B+3
        SHLD    PSCR1B+5
        SHLD    PSCR2A+1
        SHLD    PSCR2A+3
        SHLD    PSCR2A+5
        SHLD    PSCR2B+1
        SHLD    PSCR2B+3
        SHLD    PSCR2B+5
        SHLD    PSCR3A+1
        SHLD    PSCR3A+3
        SHLD    PSCR3A+5
        SHLD    PSCR3B+1
        SHLD    PSCR3B+3
        SHLD    PSCR3B+5
        SHLD    PSCR4A+1
        SHLD    PSCR4A+3
        SHLD    PSCR4A+5
        SHLD    PSCR4B+1
        SHLD    PSCR4B+3
        SHLD    PSCR4B+5
        SHLD    PNTTBL
        SHLD    PNTTBL+2
        SHLD    PNTTBL+4
        SHLD    PNTTBL+26
        SHLD    PNTTBL+28
        SHLD    PNTTBL+30
        SHLD    PNT024
        SHLD    PNT024+2
        SHLD    PNT024+4
        SHLD    PNT024+26
        SHLD    PNT024+28
        SHLD    PNT024+30
        CALL    RESET
        JMP     BEGIN
;
BEGIN:  IF      NOT INTRP ; ----.
        CALL    CONIN   ;       |
        CALL    LDSAT   ;       |
        CALL    SCORE   ;       |
        ENDIF           ; <-----'
;
        IF      DBG2
        LDA     SLOMOD
        ORA     A
        JZ      BEGN0
        LHLD    SLODLY
SLOMO:  DCX     H
        MOV     A,L
        ORA     H
        JNZ     SLOMO
        ENDIF
;
BEGN0:  CALL    JSTIK
        CALL    CMOVE   ; Update computer JDAT data.
        CALL    PMOVE   ; Update player SAT data.
        CALL    BMOVE   ; Update ball SAT data.
        CALL    SNDGN   ; Generate sound effects.
        LDA     GMOPT
        ANI     40H
        JNZ     GMOVER
        LDA     CCHAR   ; Check for new player's
;
        IF      DBG2
        CPI     '<'     ; Decrease level of play.
        JZ      LVLDCR
        CPI     '>'     ; Increase level of play.
        JZ      LVLINR
        CPI     '-'     ; Subtract 20h (32) from slow mo.
        JZ      SLOSUB
        CPI     '='     ; Add 20h (32) to slow mo.
        JZ      SLOADD
        CPI     'S'     ; Engage slow motion mode.
        JZ      SLOENG
        ENDIF
;
        CPI     ' '
        CZ      PAUSE
        CPI     02FH
        JC      BEGIN
        CPI     '4'
        JNC     BEGIN
        SUI     30H
        MOV     C,A
        LDA     GMOPT
        ANI     0F0H
        ORA     C
        STA     GMOPT
        XRA     A
        STA     CCHAR
        JMP     BEGIN
;
PAUSE:  CALL    CONIN
        ORA     A
        JZ      PAUSE
        RET
;
        IF      DBG2
LVLDCR: LXI     H,LEVEL
        DCR     M
        LXI     H,LVLNUM+1
        DCR     M
        CALL    SLEVL
        JMP     DBGEXT
LVLINR: LXI     H,LEVEL
        INR     M
        LXI     H,LVLNUM+1
        INR     M
        CALL    SLEVL
        JMP     DBGEXT
SLOSUB: LHLD    SLODLY
        LXI     D,0FFE0H
        DAD     D
        SHLD    SLODLY
        JMP     DBGEXT
SLOADD: LHLD    SLODLY
        LXI     D,0020H
        DAD     D
        SHLD    SLODLY
DBGEXT: XRA     A
        STA     CCHAR
        JMP     BEGIN
SLOENG: LDA     SLOMOD  ; Toggle slow motion mode.
        CMA
        STA     SLOMOD
        JMP     DBGEXT
        ENDIF
;
OLDINT3 DS      064H
STACK:  DS      01H
;
        IF      DBG2
SLODLY: DW      0100H
SLOMOD: DB      00H
        ENDIF
;
;               W  A  R  L  O  R  D  S
G0VMSG: DB      44,22,39,33,36,39,25,40,255
G0COLR: DB      00H,40H,01H,00H,00H,00H
;               B  Y
G1VMSG: DB      23,46,255
G1COLR: DB      00H,10H,51H,00H,00H,00H
;               L  E  S     B  I  R  D
G2VMSG: DB      33,26,40,00,23,30,39,25,255
G2COLR: DB      00H,40H,51H,00H,00H,00H
;               G  A  M  E        O  V  E  R
G3VMSG: DB      28,22,34,26,00,00,36,43,26,39,255
G3COLR: DB      00H,50H,01H,00H,00H,00H
;               S  =  S  T  A  R  T     G  A  M  E
G4VMSG: DB      40,48,40,41,22,39,41,00,28,22,34,26,255
G4COLR: DB      00H,60H,31H,00H,00H,00H
;               Q  =  Q  U  I  T        G  A  M  E
G5VMSG: DB      38,48,38,42,30,41,00,00,28,22,34,26,255
G5COLR: DB      00H,60H,91H,00H,00H,00H
;               L  E  V  E  L
LVLMSG: DB      33,26,43,26,33,00
LVLNUM: DB      12,13,255
LVCOLR: DB      00H,00H,00H
LVLRST: DB      00,00,00,00,00,00,00,00,255
;
        IF      DBG2
MSG01:  DB      01BH,'E',0AH
        DB      '  Warlords by Les Bird  Debugging features.',0DH,0AH
        DB      '  November 29, 1987  ',0DH,0AH
        DB      0AH
        DB      '  Keys:    <   Decrement level of play',0DH,0AH
        DB      '           >   Increment level of play',0DH,0AH
        DB      '           -   Subtract 32 from slow motion delay',0DH,0AH
        DB      '           =   Add 32 to slow motion delay',0DH,0AH
        DB      '           S   Engage slow motion mode.',0DH,0AH
        DB      '$'
        ENDIF
;
ERMSG0: DW      0000H
ERMSG1: DB      01BH,'EFile WARLORDS.SPR not on default drive!',07H,'$'
;
MSGDLY: DW      0080H
MSGDL0: DW      0080H
;
F1LFCB: DB      00H             ; Default drive.
        DB      'WARLORDS'      ; Filename.
        DB      'SPR'           ; File type.
        DB      00H,00H,00H,00H ; ex, s1, s2, rc
        DB      0,0,0,0,0,0,0,0 ;
        DB      0,0,0,0,0,0,0,0 ; Filled in by system.
        DB      00H,00H,00H,00H ; cr, r1, r2, r3.
;
CCHAR:  DB      00H     ; Last key pressed on keyboard.
GMOPT:  DB      00H
GMDLY:  DW      0500H
RLDLY:  DW      1000H
LEVEL:  DB      01H
SCRDT:  DB      00H
;
JPLYR:  DB      07H
JNUMB:  DB      00H
JBUTN:  DB      00H
JDAT1:  DW      0080H
JDAT2:  DW      0080H
JDAT3:  DW      0080H
JDAT4:  DW      0080H
CJDT2:  DW      0080H
CJDT3:  DW      0080H
CJDT4:  DW      0080H
; Level count for each player.
LEVL1:  DB      01H
LEVL2:  DB      01H
LEVL3:  DB      01H
LEVL4:  DB      01H
;
PNUMB:  DB      00H
P1DAT:  DW      SATTBL+14,JDAT1,0410H,PMOV2
P2DAT:  DW      SATTBL+10,JDAT2,100CH,PMOV4
P3DAT:  DW      SATTBL+6,JDAT3,080CH,PMOV6
P4DAT:  DW      SATTBL+2,JDAT4,0408H,PMOV8
;
PLYCO:  DB      00H
PLXCO:  DB      00H
PLDIR:  DB      00H
BLADR:  DW      0000H
; Computer player data block.
BDST1:  DB      00H,00H,00H
BDST2:  DB      00H,00H,00H
BDST3:  DB      00H,00H,00H
BDST4:  DW      0000H
CBNUM:  DB      00H
COMCNT: DB      00H
COMDE:  DW      0000H
COMHL:  DW      0000H
; Score data block.
PSCR1A: DB      00,12,12,12,12,12,12,00
PSCR1B: DB      00,12,12,12,12,12,12,00
PSCR2A: DB      00,12,12,12,12,12,12,00
PSCR2B: DB      00,12,12,12,12,12,12,00
PSCR3A: DB      00,12,12,12,12,12,12,00
PSCR3B: DB      00,12,12,12,12,12,12,00
PSCR4A: DB      00,12,12,12,12,12,12,00
PSCR4B: DB      00,12,12,12,12,12,12,00
SCRADR: DW      1700H,17D0H,00D0H,0000H
SCRPLR: DW      0000H
; Ball data block.
BDLY:   DB      00H
BTMR:   DB      00H
BDIR:   DB      00H
BALY:   DB      00H
BALX:   DB      00H
BADR:   DW      0000H   ; Ball address in SATTBL.
BHIT:   DB      03H     ; Last player to hit ball --> (1-4)
BPLR:   DB      00H     ; Player to test ball against (0-3) <--, .->(0-16)
BHLD:   DB      00H     ; Player holding ball ------> (1-4)    | |
BWRK:   DB      00H     ;                                      | |
BBNC:   DB      00H     ;                       +--------------+-+-----+
PADR:   DW      0000H   ;                       | Left nibble contains |
;                                               |  delay for rotating  |
BNUMB:  DB      00H     ;                       |   ball when held.    |
BNUM1:  DB      00H     ;                       +----------------------+
BDAT0:  DB      0C0H,0B9H,0AAH,09BH,00CH,01BH,02AH,039H
        DB      040H,031H,022H,013H,004H,093H,0A2H,0B1H
BDAT1:  DB      006,006,015,088,120
        DW      SATTBL+16
        DB      03,00
BHLD1:  DB      000,000,000,000,000,000,000
BDAT2:  DB      006,006,255,208,208
        DW      SATTBL+20
        DB      03,00
BHLD2:  DB      000,000,000,000,000,000,000
BDAT3:  DB      006,006,255,208,208
        DW      SATTBL+24
        DB      03,00
BHLD3:  DB      000,000,000,000,000,000,000
BDAT4:  DB      006,006,255,208,208
        DW      SATTBL+28
        DB      03,00
BHLD4:  DB      000,000,000,000,000,000,000
;       Ball wall reflection data.
; Current BDIR = 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
BDAT5A: DB      00,00,00,00,00,03,02,01,15,15,14,13
BDAT5B: DB      00,00,00,00,00,03,02,01,01,15,14,13
BDAT6A: DB      09,07,06,05,00,00,00,00,00,00,00,00,00,11,10,09
BDAT6B: DB      07,07,06,05,00,00,00,00,00,00,00,00,00,11,10,09
BDAT7A: DB      00,00,00,00,00,00,00,00,00,07,06,05,05,03,02,01
BDAT7B: DB      00,00,00,00,00,00,00,00,00,07,06,05,03,03,02,01
BDAT8A: DB      00,15,14,13,11,11,10,09
BDAT8B: DB      00,15,14,13,13,11,10,09
BDAT9:  DW      BDAT10,BDAT10+8,BDAT10+8,BDAT10+8,BDAT10+16,BDAT10+24
        DW      BDAT10+24,BDAT10+24,BDAT10+32,BDAT10+40,BDAT10+40,BDAT10+40
        DW      BDAT10+48,BDAT10+56,BDAT10+56,BDAT10+56
;       Ball pixel movement data.
BDAT10: DB      065H,056H,047H,048H,049H,05AH,06BH,078H ; 0
        DB      049H,048H,047H,056H,065H,075H,085H,078H ; 1,2,3
        DB      047H,056H,065H,075H,085H,096H,0A7H,078H ; 4
        DB      065H,075H,085H,096H,0A7H,0A8H,0A9H,078H ; 5,6,7
        DB      085H,096H,0A7H,0A8H,0A9H,09AH,08BH,078H ; 8
        DB      0A7H,0A8H,0A9H,09AH,08BH,07BH,06BH,078H ; 9,10,11
        DB      0A9H,09AH,08BH,07BH,06BH,05AH,04AH,078H ; 12
        DB      08BH,07BH,06BH,05AH,049H,048H,047H,078H ; 13,14,15
;       Ball brick reflection data.
BDAT11: DB      005H,006H,006H,007H,009H,00AH,00AH,00BH ; DIR0
        DB      005H,006H,007H,007H,00FH,00FH,00EH,00DH ; DIR1
        DB      005H,006H,006H,007H,00DH,00EH,00EH,00FH ; DIR2
        DB      007H,006H,005H,005H,00DH,00DH,00EH,00FH ; DIR3
        DB      009H,00AH,00BH,00BH,00DH,00DH,00EH,00FH ; DIR4
        DB      009H,00AH,00BH,00BH,001H,001H,002H,003H ; DIR5
        DB      009H,00AH,00AH,00BH,001H,002H,002H,003H ; DIR6
        DB      00BH,00AH,009H,009H,001H,001H,002H,003H ; DIR7
        DB      00DH,00EH,00FH,00FH,001H,001H,002H,003H ; DIR8
        DB      00DH,00EH,00FH,00FH,007H,007H,006H,005H ; DIR9
        DB      00DH,00EH,00EH,00FH,005H,006H,006H,007H ; DIR10
        DB      00FH,00EH,00DH,00DH,005H,005H,006H,007H ; DIR11
        DB      001H,002H,003H,003H,005H,005H,006H,007H ; DIR12
        DB      001H,002H,003H,003H,00BH,00BH,00AH,009H ; DIR13
        DB      001H,002H,002H,003H,009H,00AH,00AH,00BH ; DIR14
        DB      003H,002H,001H,001H,009H,009H,00AH,00BH ; DIR15
;       Paddle reflection data.
BRFL00: DB      11,11,11,10,10,10,09,09,09,07,07,07,06,06,06,05,05,05
BRFL04: DB      09,09,09,10,10,10,11,11,11,13,13,13,14,14,14,15,15,15
BRFL08: DB      13,13,13,14,14,14,15,15,15,01,01,01,02,02,02,03,03,03
BRFL12: DB      07,07,07,06,06,06,05,05,05,03,03,03,02,02,02,01,01,01
BRFLCT: DB      007,006,005,004,003,002,001,000,255
        DB      254,253,252,251,250,249,248,247,246
        DB      128
BTHR01: DB      04,04,04,05,05,05,05,05,05,05,06,06
        DB      08,08,08,07,07,07,07,07,07,07,06,06
BTHR02: DB      04,04,04,03,03,03,03,03,03,03,02,02
        DB      00,00,00,01,01,01,01,01,01,01,02,02
BTHR03: DB      12,12,12,13,13,13,13,13,13,13,14,14
        DB      00,00,00,15,15,15,15,15,15,15,14,14
BTHR04: DB      12,12,12,11,11,11,11,11,11,11,10,10
        DB      08,08,08,09,09,09,09,09,09,09,10,10
PXLDAT: DB      080H,040H,020H,010H,008H,004H,002H,001H
        DB      00FH,00FH,00FH,00FH,0F0H,0F0H,0F0H,0F0H
;
PLCNT:  DB      00H
CHKDT0: DW      PLYR4,00C0H,0000H
        DB      008H,00AH,004H,004H  ; PLDEAD bit,BDIR,Y,X.
        DW      PLYR1,10C0H,1400H
        DB      001H,006H,0B0H,004H
CHKDT1: DW      PLYR3,20C0H,00D0H
        DB      004H,00EH,004H,0F8H
        DW      PLYR2,30C0H,14D0H
        DB      002H,002H,0B0H,0F8H
CHKDT2: DW      0000H
CHKDT3: DB      00EH,058H,078H
PLNEW:  DB      008,080,004,014,064,224,008,014 ; Player#4,3
        DB      168,158,012,014,110,016,016,014 ;  "  " #2,1
; Sound effects data.
SNDBIT: DB      00H
SDLY0:  DB      08H
SDLY1:  DB      20H
SDLY2:  DB      08H
SDLY3:  DB      10H
SBRIK:  DB      03H
        DB      06H,1FH,07H,0EH,0AH,00H
SINIT:  DB      0EH
        DB      00H,00H,01H,03H,02H,00H,03H,00H,04H,00H,05H,00H
        DB      06H,10H,07H,0EH,08H,10H,09H,00H,0AH,10H,0BH,00H
        DB      0CH,20H,0DH,00H
SKILL:  DB      03H
        DB      06H,10H,07H,0EH,09H,00H
SPADL:  DB      03H
        DB      06H,10H,07H,0EH,0AH,00H
STHRO:  DB      04H
        DB      07H,0EH,00H,00H,01H,02H,08H,0FH,08H,00H,04H
SWALL:  DB      07H
        DB      00H,00H,01H,0AH,07H,0EH,08H,10H,0BH,00H,0CH,02H
        DB      0DH,00H
;
PGTTBL: DB      000,000,000,000,000,000,000,000 ; 0             #1
        DB      051,255,204,051,255,204,255,051 ; 1
        DB      000,000,000,000,000,000,000,000 ; *NOT DEFINED* #3
        DB      255,255,255,255,255,255,255,255 ; 3
BLKUP:  DB      243,255,255,255,255,007,007,007 ; UP BLOCK      #5
        DB      207,255,255,255,255,224,224,224 ;
BLKRT:  DB      031,031,031,031,030,254,255,255 ; RIGHT BLOCK
        DB      255,255,254,030,031,031,031,031 ;
BLKDN:  DB      007,007,007,255,255,255,255,243 ; DOWN BLOCK
        DB      224,224,224,255,255,255,255,207 ;
BLKLT:  DB      248,248,248,248,120,127,255,255 ; LEFT BLOCK
        DB      255,255,127,120,248,248,248,248 ;
NUMBR:  DB      000,062,067,069,073,081,097,062 ; #0            #13
        DB      000,008,024,008,008,008,008,028 ; #1
        DB      000,062,065,001,030,032,064,127 ; #2
        DB      000,062,065,001,014,001,065,063 ; #3
        DB      000,004,012,020,036,126,004,004 ; #4
        DB      000,127,064,124,002,001,066,060 ; #5
        DB      000,062,065,064,126,065,065,062 ; #6
        DB      000,127,065,002,004,008,016,032 ; #7
        DB      000,062,065,065,062,065,065,062 ; #8            #21
        DB      000,062,065,065,063,001,065,062 ; #9               
        DB      000,056,068,130,254,130,130,130 ; "A"           #23
        DB      000,252,130,130,252,130,130,252 ; "B"           #24
        DB      000,060,066,128,128,128,066,060 ; "C"           #25
        DB      000,248,132,130,130,130,132,248 ; "D"           #26
        DB      000,254,128,128,240,128,128,254 ; "E"           #27
        DB      000,254,128,128,240,128,128,128 ; "F"           #28
        DB      000,056,068,128,128,142,068,060 ; "G"           #29
        DB      000,130,130,130,254,130,130,130 ; "H"           #30
        DB      000,028,008,008,008,008,008,028 ; "I"           #31
        DB      000,062,008,008,008,008,136,112 ; "J"           #32
        DB      000,132,136,144,224,144,136,132 ; "K"           #33
        DB      000,128,128,128,128,128,128,254 ; "L"           #34
        DB      000,130,198,170,146,130,130,130 ; "M"           #35
        DB      000,130,194,162,146,138,134,130 ; "N"           #36
        DB      000,056,068,130,130,130,068,056 ; "O"           #37
        DB      000,252,130,130,252,128,128,128 ; "P"           #38
        DB      000,056,068,130,130,138,068,058 ; "Q"           #39
        DB      000,252,130,130,252,136,132,130 ; "R"           #40
        DB      000,126,128,128,124,002,002,252 ; "S"           #41
        DB      000,254,016,016,016,016,016,016 ; "T"           #42
        DB      000,130,130,130,130,130,068,056 ; "U"           #43
        DB      000,130,130,068,068,040,040,016 ; "V"           #44
        DB      000,130,130,130,146,170,198,130 ; "W"           #45
        DB      000,130,068,040,016,040,068,130 ; "X"           #46
        DB      000,130,068,040,016,016,016,016 ; "Y"           #47
        DB      000,254,004,008,016,032,064,254 ; "Z"           #48
        DB      000,000,000,252,000,252,000,000 ; "="           #49
        DB      000,000,000,056,124,254,254,254 ; "^" LEVEL MRK #50
;
SATTBL: 
PLYR4:  DB      008,080,004,014
PLYR3:  DB      064,224,008,014
PLYR2:  DB      168,158,012,014
PLYR1:  DB      110,016,016,014
BALL1:  DB      088,120,080,015
BALL2:  DB      208,208,255,015
BALL3:  DB      208,208,255,015
BALL4:  DB      208,208,255,015
        DB      208
;       Castle Display Memory Map
PNTTBL: DB      12,12,12,12,12,12,01,01,01,01,06,00,00,00,00,00         ;
        DB      00,00,00,00,00,10,01,01,01,01,12,12,12,12,12,12         ; 1
        DB      03,03,03,03,03,03,01,01,01,01,07,00,00,00,00,00         ;
        DB      00,00,00,00,00,11,01,01,01,01,03,03,03,03,03,03         ; 2
        DB      03,03,03,03,03,03,01,01,01,01,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,01,01,01,01,03,03,03,03,03,03         ; 3
        DB      03,03,03,03,03,03,01,01,01,01,06,00,00,00,00,00         ;
        DB      00,00,00,00,00,10,01,01,01,01,03,03,03,03,03,03         ; 4
        DB      01,01,01,01,01,01,01,01,01,01,07,00,00,00,00,00         ;
        DB      00,00,00,00,00,11,01,01,01,01,01,01,01,01,01,01         ; 5
        DB      01,01,01,01,01,01,01,01,01,01,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,01,01,01,01,01,01,01,01,01,01         ; 6
        DB      01,01,01,01,01,01,01,01,01,01,06,00,00,00,00,00         ;
        DB      00,00,00,00,00,10,01,01,01,01,01,01,01,01,01,01         ; 7
        DB      01,01,01,01,01,01,01,01,01,01,07,00,00,00,00,00         ;
        DB      00,00,00,00,00,11,01,01,01,01,01,01,01,01,01,01         ; 8
        DB      08,09,00,00,08,09,00,00,08,09,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,08,09,00,00,08,09,00,00,08,09         ; 9
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ; 10
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ; 11
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ; 12
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ; 13
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ; 14
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ;   
        DB      00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00         ; 15
        DB      04,05,00,00,04,05,00,00,04,05,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,04,05,00,00,04,05,00,00,04,05         ; 16
        DB      01,01,01,01,01,01,01,01,01,01,06,00,00,00,00,00         ;
        DB      00,00,00,00,00,10,01,01,01,01,01,01,01,01,01,01         ; 17
        DB      01,01,01,01,01,01,01,01,01,01,07,00,00,00,00,00         ;
        DB      00,00,00,00,00,11,01,01,01,01,01,01,01,01,01,01         ; 18
        DB      01,01,01,01,01,01,01,01,01,01,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,01,01,01,01,01,01,01,01,01,01         ; 19
        DB      01,01,01,01,01,01,01,01,01,01,06,00,00,00,00,00         ;
        DB      00,00,00,00,00,10,01,01,01,01,01,01,01,01,01,01         ; 20
        DB      03,03,03,03,03,03,01,01,01,01,07,00,00,00,00,00         ;
        DB      00,00,00,00,00,11,01,01,01,01,03,03,03,03,03,03         ; 21
        DB      03,03,03,03,03,03,01,01,01,01,00,00,00,00,00,00         ;
        DB      00,00,00,00,00,00,01,01,01,01,03,03,03,03,03,03         ; 22
        DB      03,03,03,03,03,03,01,01,01,01,06,00,00,00,00,00         ;
        DB      00,00,00,00,00,10,01,01,01,01,03,03,03,03,03,03         ; 23
PNT024: DB      12,12,12,12,12,12,01,01,01,01,07,00,00,00,00,00         ;
        DB      00,00,00,00,00,11,01,01,01,01,12,12,12,12,12,12         ; 24
;
CGTTBL: DB      000H,030H,0E1H,000H,010H,051H,000H,010H,041H
        DB      000H,060H,0E1H,000H,010H,061H,000H,010H,091H
        DB      000H,030H,0E1H                                  ; L1
        DB      000H,030H,071H,000H,010H,051H,000H,010H,041H
        DB      000H,060H,0E1H,000H,010H,061H,000H,010H,091H
        DB      000H,030H,071H                                  ; L2
        DB      000H,030H,071H,000H,010H,051H,000H,010H,041H
        DB      000H,060H,0E1H,000H,010H,061H,000H,010H,091H
        DB      000H,030H,071H                                  ; L3
        DB      000H,030H,071H,000H,010H,051H,000H,010H,041H
        DB      000H,060H,0E1H,000H,010H,061H,000H,010H,091H
        DB      000H,030H,071H                                  ; L4
        DB      000H,040H,051H,000H,010H,041H,000H,060H,0E1H
        DB      000H,010H,061H,000H,040H,091H                   ; L5
        DB      000H,040H,051H,000H,010H,041H,000H,060H,0E1H
        DB      000H,010H,061H,000H,040H,091H                   ; L6
        DB      000H,050H,041H,000H,060H,0E1H,000H,050H,061H    ; L7
        DB      000H,050H,041H,000H,060H,0E1H,000H,050H,061H    ; L8
        DB      008H,000H,0E1H                                  ; L9-16
        DB      000H,050H,0A1H,000H,060H,0E1H,000H,050H,0C1H    ; L17
        DB      000H,050H,0A1H,000H,060H,0E1H,000H,050H,0C1H    ; L18
        DB      000H,040H,0B1H,000H,010H,0A1H,000H,060H,0E1H
        DB      000H,010H,0C1H,000H,040H,031H                   ; L19
        DB      000H,040H,0B1H,000H,010H,0A1H,000H,060H,0E1H
        DB      000H,010H,0C1H,000H,040H,031H                   ; L20
        DB      000H,030H,071H,000H,010H,0B1H,000H,010H,0A1H
        DB      000H,060H,0E1H,000H,010H,0C1H,000H,010H,031H
        DB      000H,030H,071H                                  ; L21
        DB      000H,030H,071H,000H,010H,0B1H,000H,010H,0A1H
        DB      000H,060H,0E1H,000H,010H,0C1H,000H,010H,031H
        DB      000H,030H,071H                                  ; L22
        DB      000H,030H,071H,000H,010H,0B1H,000H,010H,0A1H
        DB      000H,060H,0E1H,000H,010H,0C1H,000H,010H,031H
        DB      000H,030H,071H                                  ; L23
        DB      000H,030H,0E1H,000H,010H,0B1H,000H,010H,0A1H
        DB      000H,060H,0E1H,000H,010H,0C1H,000H,010H,031H
        DB      000H,030H,0E1H                                  ; L24
        DB      000H,000H,000H  ; End of table.
;
SGTTBL: EQU     $
;
        END     START

