;    OPT D+,O+,W-
*****************************************************************
*
*       Project:    AZur
*       Function:   Patch
*
*       Created:    05/07/94    Jean-Michel Forgeas
*
*****************************************************************


******* Includes ************************************************

        INCLUDE "exec/types.i"
        INCLUDE "exec/ables.i"
        INCLUDE "lib/console_lib.i"
        INCLUDE "lib/exec_lib.i"
        INCLUDE "lib/dos_lib.i"
        INCLUDE "uiki:uikglobal.i"
        INCLUDE "UIKI:uik_lib.i"
        INCLUDE "LCI:azur.i"
        INCLUDE "LCI:eng_obj.i"
        INCLUDE "LCI:objfile.i"


******* Imported ************************************************


******* Exported ************************************************

        XDEF    _CodeText
        XDEF    _DecodeText


*****************************************************************


*****************************************************************
*
                    SECTION     text,CODE
*
*****************************************************************


**********************************************
*   length = CodeText( key, string )
*     d0               a0     a1
***********************************************

_CodeText:
        movem.l 4(sp),a0-a1             a0=key, a1=string
        moveq   #0,d0

.20     tst.b   (a1)
        beq.s   .90                     fin de chaine
        move.b  (a0)+,d1
        bne.s   .30                     fin de clef
        movea.l 4(sp),a0
        move.b  (a0)+,d1

.30     eor.b   d1,(a1)+
        addq.l  #1,d0
        bra.s   .20

.90     rts

***********************************************
*   DecodeText( key, string, length )
*               a0     a1      d0
***********************************************

_DecodeText:
        movem.l 4(sp),a0-a1             a0=key, a1=string
        move.l  12(sp),d0               d0=length
        move.l  a0,d1
        bra.s   .40

.20     move.b  (a0)+,d1
        bne.s   .30                     fin de clef
        movea.l 4(sp),a0
        move.b  (a0)+,d1

.30     eor.b   d1,(a1)+
.40     dbra    d0,.20

.90     rts
