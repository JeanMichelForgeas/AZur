;    OPT D+,O+,W-

        INCLUDE "lib/exec_lib.i"
        INCLUDE "dos/dosextens.i"
        INCLUDE exec/memory.i

        INCLUDE lci:azur.i
        INCLUDE lci:eng_obj.i
        INCLUDE uiki:uiklist.i
        INCLUDE uiki:uikbuf.i
        INCLUDE uiki:uik_lib.i


        XDEF    _GetProcSegment
        XDEF    _pos_tab
        XDEF    _next_tab
        XDEF    _prev_tab
        XDEF    _skip_endchar
        XDEF    _find_endchar
        XDEF    _skip_char
        XDEF    _find_char
        XDEF    _DoOnOffToggle
        XDEF    _PoolAlloc
        XDEF    _PoolFree
        XDEF    _StrHandler_HexDec
        XDEF    _StrHandler_Signed

        XDEF    _StackRest

        ;XDEF    _sync_request

        XREF    _ProcessMain
        XREF    _AsmAllocPooled
        XREF    _AsmFreePooled


        SECTION text,CODE


_StrHandler_HexDec
        move.b  (a1),d0
        cmp.b   #'$',d0
        beq.b   endok
        cmp.b   #'x',d0
        beq.b   endok
        cmp.b   #'0',d0
        blt.b   endno
        cmp.b   #'9',d0
        ble.b   endok
        cmp.b   #'A',d0
        blt.b   endno
        cmp.b   #'F',d0
        ble.b   endok
        cmp.b   #'a',d0
        blt.b   endno
        cmp.b   #'f',d0
        ble.b   endok
endno   moveq   #0,d0
        rts
endok   move.l  a1,d0
        rts

_StrHandler_Signed
        move.b  (a1),d0
        cmp.b   #'-',d0
        beq.b   endok
        cmp.b   #'+',d0
        beq.b   endok
        cmp.b   #'$',d0
        beq.b   endok
        cmp.b   #'x',d0
        beq.b   endok
        cmp.b   #'0',d0
        blt.b   endno
        cmp.b   #'9',d0
        ble.b   endok
        cmp.b   #'A',d0
        blt.b   endno
        cmp.b   #'F',d0
        ble.b   endok
        cmp.b   #'a',d0
        blt.b   endno
        cmp.b   #'f',d0
        ble.b   endok
        bra.b   endno


_GetProcSegment
    lea _kidseg(PC),A0
    move.l A0,D0
    lsr.l #2,D0
    rts

    cnop    0,4 ;Align to longword
    dc.l    16  ;Segment "length" (faked)
_kidseg:
    dc.l    0   ;Pointer to next segment
    jmp     _ProcessMain


***************************************************
*   LONG pos_tab( col )
*   d0            d0
***************************************************
            ; v->tablen - (v->col % v->tablen)
_pos_tab
    move.l  d0,a1               sauve col
    move.l  az_Prefs(a4),a0
    moveq   #0,d1
    move.b  azp_TabLen(a0),d1
    divu.w  d1,d0               v->col % v->tablen
    swap    d0
    tst.w   d0
    beq.b   .1
    sub.w   d0,d1               v->tablen - modulo
    add.w   d1,a1               col += offs  (d1 est étendu à un long à cause de a1)
.1  move.l  a1,d0
    rts


***************************************************
*   LONG next_tab( col )
*   d0             d0
***************************************************
            ; v->tablen - (v->col % v->tablen)
_next_tab
    move.l  d0,a1               sauve col
    move.l  az_Prefs(a4),a0
    moveq   #0,d1
    move.b  azp_TabLen(a0),d1
    divu.w  d1,d0
    swap    d0                  v->col % v->tablen
    sub.w   d0,d1               v->tablen - modulo
    add.w   d1,a1               col += offs  (d1 est étendu à un long à cause de a1)
    move.l  a1,d0
    rts


***************************************************
*   LONG prev_tab( col )
*   d0             d0
***************************************************

_prev_tab
    tst.l   d0
    beq.s   .end                évite un résultat négatif
    move.l  d0,a1               sauve col
    move.l  az_Prefs(a4),a0
    moveq   #0,d1
    move.b  azp_TabLen(a0),d1
    divu.w  d1,d0
    swap    d0                  v->col % v->tablen

    tst.w   d0
    bne.s   .2
    move.w  d1,d0               if 0 -> tablen
.2
    sub.w   d0,a1               col -= offs  (d0 est étendu à un long à cause de a1)
    move.l  a1,d0
.end
    rts


***************************************************
*   skip_char( UBYTE *p, UBYTE c1, UBYTE c2 )
*                     a0       d0        d1
***************************************************

_skip_char
    sub.l   a1,a1
.loop
    tst.b   (a0)
    beq.s   .end
    cmp.b   (a0),d0
    beq.s   .ad
    cmp.b   (a0),d1
    bne.s   .end
.ad addq.w  #1,a0
    addq.w  #1,a1
    bra.s   .loop
.end
    move.l  a1,d0
    rts


***************************************************
*   find_char( UBYTE *p, UBYTE c1, UBYTE c2 )
*                     a0       d0        d1
***************************************************

_find_char
    sub.l   a1,a1
.loop
    tst.b   (a0)
    beq.s   .end
    cmp.b   (a0),d0
    beq.s   .end
    cmp.b   (a0)+,d1
    beq.s   .end
    addq.w  #1,a1
    bra.s   .loop
.end
    move.l  a1,d0
    rts


************************************************************
*   find_endchar( UBYTE *p, UBYTE c1, UBYTE c2, ULONG len )
*   d0                   a0       d0        d1        a1
************************************************************

_find_endchar
    add.l   a0,a1       a1 = après fin de chaine
.loop
    cmp.l   a0,a1
    bls.s   .end

    cmp.b   -(a1),d0
    beq.s   .equal
    cmp.b   (a1),d1
    bne.s   .loop
.equal
    addq.w  #1,a1
.end
    move.l  a1,d0
    sub.l   a0,d0
    rts


************************************************************
*   skip_endchar( UBYTE *p, UBYTE c1, UBYTE c2, ULONG len )
*   d0                   a0       d0        d1        a1
************************************************************

_skip_endchar
    add.l   a0,a1       a1 = après fin de chaine
.loop
    cmp.l   a0,a1
    bls.s   .end

    cmp.b   -(a1),d0
    beq.s   .loop
    cmp.b   (a1),d1
    beq.s   .loop
.diff
    addq.w  #1,a1
.end
    move.l  a1,d0
    sub.l   a0,d0
    rts


**********************************************************
*   desired = DoOnOffToggle( ULONG *parms, ULONG initial )
*   d0                          a0               d0
**********************************************************

_DoOnOffToggle
    tst.l   (a0)+
    bne.s   .on
    tst.l   (a0)+
    bne.s   .off
    tst.l   (a0)
    bne.s   .toggle
.on
    moveq   #-1,d0
    rts
.off
    moveq   #0,d0
    rts
.toggle
    not.l   d0
    rts


***************************************************************
* UBYTE * __asm PoolAlloc( register __d0 ULONG memSize );
***************************************************************

_PoolAlloc
    ;move.l  #MEMF_ANY|MEMF_CLEAR,d1
    ;XREF    _AllocSpMem
    ;jmp     _AllocSpMem

    move.l  a6,-(sp)
    move.l  4.w,a6

    move.l  az_Pool(a4),a0
    addq.l  #4,d0               ; Get space for tracking
    move.l  d0,-(sp)            ; Save the size
    jsr     _AsmAllocPooled     ; Call pool...
    move.l  (sp)+,d1            ; Get size back...
    tst.l   d0                  ; Check for error
    beq.s   .avp_fail           ; If NULL, failed!
    move.l  d0,a0               ; Get pointer...
    move.l  d1,(a0)+            ; Store size
    move.l  a0,d0               ; Get result
.avp_fail
    move.l  (sp)+,a6
    rts


************************************************************
* void __asm PoolFree( register __a1 void *memory );
************************************************************

_PoolFree
    ;XREF    _FreeSpMem
    ;jmp     _FreeSpMem

    move.l  a6,-(sp)
    move.l  4.w,a6

    move.l  az_Pool(a4),a0
    move.l  a1,d0
    beq.s   .end
    move.l  -(a1),d0            ; Get size / ajust pointer
    jsr     _AsmFreePooled
.end
    move.l  (sp)+,a6
    rts


**************************
* ULONG __asm StackRest();
**************************

_StackRest
    move.l  4.w,a1
    move.l  $0114(a1),a1
    move.l  a7,d0
    sub.l   TC_SPLOWER(a1),d0
    rts


**********************************************************

    END

************************************************************
* ULONG sync_request( struct UIKObj *rs );
************************************************************

    XREF    _AllocNewStack
    XREF    _RestoreStack

_sync_request
    LINK      A5,#$FFEC
    MOVEM.L   D6-D7/A3/A6,-(A7)
    MOVEA.L   $08(A5),A3
    MOVEQ     #$00,D7
    MOVEQ     #$FF,D0
    MOVE.L    D0,$4C(A3)
    bsr.b     _StackRest
    CMPI.L    #$00000DAC,D0
    BGE.B     L0164

;       if (! (newstack = AllocNewStack( stackbuf, 10000 ))) return( REQBUT_CANCEL );
        MOVE.L    #$00002710,D0
        LEA       -16(A5),A0
        JSR       _AllocNewStack(PC)
        MOVE.L    D0,D7
        MOVE.L    D7,D0
        BNE.B     L0164
        MOVEQ     #$00,D0
        BRA.B     L01B4

;       while (rs->UserULong1 == -1)
L0164
        MOVEQ     #$FF,D0
        CMP.L     $4C(A3),D0
        BNE.B     L01A4
;           {
;           ULONG events;
;           events = UIK_WaitFirstEvents( AGP.UIK, SIGBREAKF_CTRL_C );
            MOVEA.L   $18(A4),A0
            MOVE.L    #$00001000,D0
            MOVEA.L   (A4),A6
            JSR       -1188(A6)
            MOVE.L    D0,D6

;           if (events & SIGBREAKF_CTRL_C)
            BTST      #$C,D6
            BEQ.B     .test2
;               {
;               Signal( FindTask(0), SIGBREAKF_CTRL_C );
                SUBA.L    A1,A1
                MOVEA.L   4.w,A6
                JSR       -294(A6)
                MOVEA.L   D0,A1
                MOVE.L    #$00001000,D0
                JSR       -324(A6)
;               rs->UserULong1 = REQBUT_CTRL_C;
                MOVEQ     #$03,D0
                MOVE.L    D0,$4C(A3)
                BRA.B     L0164
;               }
.test2
;           else if (AGP.UIK->Obj.Status != UIKID_STARTED) rs->UserULong1 = REQBUT_CANCEL;
            MOVEA.L   $18(A4),A0
            TST.B     uo_Status(A0)
            BEQ.B     .next
                MOVEQ     #$00,D0
                MOVE.L    D0,$4C(A3)
                BRA.B     L0164

.next
            BRA.B     L0164
;           }


;       if (newstack)
L01A4
        TST.L     D7
        BEQ.B     L01B0
;           {
;           RestoreStack( stackbuf );
            LEA       -16(A5),A0
            JSR       _RestoreStack(PC)
;           }
;
;       return( rs->UserULong1 );
L01B0
        MOVE.L    $4C(A3),D0
; }
L01B4
    MOVEM.L   (A7)+,D6-D7/A3/A6
    UNLK      A5
    RTS
