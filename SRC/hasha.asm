;    OPT O+,W-
*****************************************************************
*
*       Project:    UIK
*       Function:   Complémentaires de hashage
*
*       Created:    17/11/93    Jean-Michel Forgeas
*
*****************************************************************


******* Includes ************************************************

        INCLUDE "exec/types.i"


******* Imported ************************************************


******* Exported ************************************************

        XDEF    _HPJW


*****************************************************************


*****************************************************************
*
                    SECTION     text,CODE
*
*****************************************************************


***************************************************
*   HPJW( UBYTE *str )
*                a0
***************************************************
* Hashing function taken from "Compilers..." from Aho/Sethi/Ullman */
* extern ULONG __asm HPJW( register __a0 char *C );
*
*ULONG __asm HPJW( register __a0 char *C )
*{
*  char *p;
*  ULONG h=0, g;
*
*    for (p=C; *p != EOS; p++)
*    {
*        h = (h << 24) + (*p);
*        if ((g = h & 0xf0000000) != 0)
*            {
*            h = h ^ (g >> 24);
*            h = h ^ g;
*            }
*    }
*    return( h /*(UWORD) (h % PRIME)*/ );
*}

PRIME   equ 211

_HPJW
    moveq   #0,d0       h = 0
    bra.b   .next
.loop
    swap    d0
    clr.w   d0
    lsr.l   #8,d0       (h << 24)
    move.b  d1,d0       + (*p)
    move.l  #$f0000000,d1
    and.l   d0,d1       if ((g = h & 0xf0000000) != 0)
    beq.b   .next

    move.l  d1,a1       g->
    swap    d1
    clr.w   d1
    lsr.l   #8,d1       (g << 24)
    eor.l   d1,d0       h = h ^ (g >> 24)
    move.l  a1,d1       ->g
    eor.l   d1,d0       h = h ^ g
.next
    move.b  (a0)+,d1
    bne.b   .loop

.modulo ; :32 % :16
    moveq   #0,d1
    swap    d0
    move.w  d0,d1
    beq.s   .1
    divu    #PRIME,d1
.1  swap    d0
    move.w  d0,d1
    divu    #PRIME,d1
    swap    d1        ; d1.w == reste

    moveq   #0,d0
    move.w  d1,d0
    rts



*****************************************************************
        END


