
        INCLUDE "i:lib/console_lib.i"

        INCLUDE "uiki:uikglobal.i"
        INCLUDE "uiki:uikglobal.i"

        INCLUDE "lci:azur.i"


    END


        XDEF    _AZMapANSI

****************************************************************
*   AZMapANSI()
****************************************************************

bFCA190
   dc.b      0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4

RFCA1A0
   moveq     #0,D2
   btst      #7,D0
   bne       lFCA1BE
   btst      #5,D0
   beq       lFCA1BE
   moveq     #0,D1
   move.b    D0,D1
   and.b     #7,D1
   move.b    bFCA190(PC,D1.w),D1
   bset      D1,D2
   add.w     D2,D2
lFCA1BE
   subq.w    #2,D2
   rts

RFCA1C2
   and.w     #$FF,D0
   move.b    bFCA190(PC,D0.w),D0
   and.w     #$FF,D1
   sub.b     bFCA190(PC,D1.w),D0
   rts
lFCA1D4
   move.l    D2,-(SP)
   bsr       RFCA1A0
   blt       lFCA208
lFCA1DA
   btst      #3,0(A0,D2.w)
   beq       lFCA204
   move.b    1(A0,D2.w),D1
   move.b    D1,D0
   and.w     #$0F,D0
   cmp.w     $3E(A6),D0
   ble       lFCA1F6
   move.w    D0,$3E(A6)
lFCA1F6
   lsr.b     #4,D1
   beq       lFCA204
   cmp.w     $40(A6),D0
   ble       lFCA204
   move.w    D0,$40(A6)
lFCA204
   subq.w    #2,D2
   bge       lFCA1DA
lFCA208
   move.l    (SP)+,D2
   rts

RFCA20C
   movem.l   D2/D3,-(SP)
   move.b    D0,D2
   moveq     #0,D0
lFCA214
   tst.b     D1
lFCA216
   beq       lFCA228
   move.b    D2,D3
   neg.b     D3
   and.b     D2,D3
   eor.b     D3,D2
   lsr.b     #1,D1
   bcc       lFCA216
   or.b      D3,D0
   bra       lFCA214
lFCA228
   movem.l   (SP)+,D2/D3
   rts
lFCA22E
   movem.l   D2-D5,-(SP)
   move.w    D0,D4
   move.b    D1,D5
   bsr       RFCA1A0
   blt       lFCA278
lFCA23C
   btst      #3,0(A0,D2.w)
   beq       lFCA274
   moveq     #0,D3
   move.b    1(A0,D2.w),D3
   and.b     #$0F,D3
   subq.w    #1,D3
   add.w     D3,D3
   tst.b     $20(A6,D3.w)
   bmi       lFCA264
   move.b    D2,D0
   move.b    $21(A6,D3.w),D1
   bsr       RFCA1C2
   bge       lFCA274
lFCA264
   move.b    D5,$20(A6,D3.w)
   move.b    D4,D0
   move.b    D2,D1
   lsr.b     #1,D1
   bsr       RFCA20C
   move.b    D0,$21(A6,D3.w)
lFCA274
   subq.w    #2,D2
   bge       lFCA23C
lFCA278
   movem.l   (SP)+,D2-D5
   rts
lFCA27E
   movem.l   D2-D7/A2,-(SP)
   btst      #7,D0
   bne       lFCA53C
   move.w    D0,D6
   move.w    D1,D7
   move.l    A0,A2
   and.w     #$0F,D0
   lea       bFCA190(PC),A0
   move.b    0(A0,D0.w),D0
   moveq     #0,D3
   bset      D0,D3
   moveq     #0,D2
   btst      #6,D6
   beq       lFCA31A
lFCA2A8
   moveq     #0,D4
   move.b    D2,D4
   add.b     D4,D4
   move.b    0(A2,D4.w),D4
   beq       lFCA310
   cmp.w     2(A6),D4
   blt       lFCA310
   cmp.l     4(A6),D4
   bgt       lFCA310
   moveq     #0,D1
   move.b    D2,D1
   add.b     D1,D1
   move.b    1(A2,D1.w),D1
   ext.w     D1
   lea       0(A2,D1.w),A0
   move.l    $0C(A6),A1
   move.w    D4,D1
   moveq     #0,D0
   bra       lFCA2DC
lFCA2DA
   cmpm.b    (A0)+,(A1)+
lFCA2DC
   dbne      D1,lFCA2DA
   bne       lFCA310
   cmp.w     2(A6),D4
   bne       lFCA2FA
   cmp.w     #1,(A6)
   bne       lFCA2FA
   move.b    D2,D0
   move.b    1(A5),D1
   bsr       RFCA1C2
   bge       lFCA310
lFCA2FA
   move.w    #1,(A6)
   move.w    D4,2(A6)
   move.b    D7,(A5)
   move.b    D6,D0
   move.b    D2,D1
   bsr       RFCA20C
   move.b    D0,1(A5)
lFCA310
   addq.w    #1,D2
   cmp.w     D2,D3
   bgt       lFCA2A8
   bra       lFCA53C
lFCA31A
   cmp.w     #1,2(A6)
   bgt       lFCA53C
   move.l    $0C(A6),A0
   move.b    (A0),D5
   btst      #5,D6
   beq       lFCA48E
lFCA332
   move.w    D2,D1
   add.b     D1,D1
   move.b    0(A2,D1.w),D0
   bne       lFCA372
   cmp.b     1(A2,D1.w),D5
   bne       lFCA470
lFCA344
   bsr       RFCA34A
   bra       lFCA470

RFCA34A
   cmp.w     #1,(A6)
   bne       lFCA35C
   move.b    D2,D0
   move.b    1(A5),D1
   bsr       RFCA1C2
   bge       lFCA370
lFCA35C
   move.l    #$010001,(A6)
   move.b    D7,(A5)
   move.b    D6,D0
   move.b    D2,D1
   bsr       RFCA20C
   move.b    D0,1(A5)
lFCA370
   rts
lFCA372
   btst      #0,D0
   beq       lFCA470
   move.b    1(A2,D1.w),D1
   ext.w     D1
   lea       0(A2,D1.w),A0
   cmp.b     (A0)+,D5
   beq       lFCA344
   cmp.w     #1,(A6)
   beq       lFCA470
   moveq     #1,D4
lFCA392
   cmp.b     (A0)+,D5
   bne       lFCA3E2
   tst.w     2(A6)
   beq       lFCA3AE
   cmp.w     #2,(A6)
   bne       lFCA3AE
   move.b    D2,D0
   move.b    3(A5),D1
   bsr       RFCA1C2
   bge       lFCA3E2
lFCA3AE
   moveq     #2,D0
   move.w    D0,(A6)
   cmp.l     8(A6),D0
   bgt       lFCA470
   move.w    #1,2(A6)
   lea       $20(A6),A1
   move.w    D4,D0
   add.w     D0,D0
   move.b    -2(A1,D0.w),(A5)
   move.b    -1(A1,D0.w),1(A5)
   move.b    D7,2(A5)
   move.b    D6,D0
   move.b    D2,D1
   bsr       RFCA20C
   move.b    D0,3(A5)
lFCA3E2
   addq.w    #1,D4
   cmp.w     $3E(A6),D4
   blt       lFCA392
   cmp.w     #2,(A6)
   beq       lFCA470
   bra       lFCA458
lFCA3F4
   cmp.b     (A0)+,D5
   bne       lFCA458
   tst.w     2(A6)
   beq       lFCA410
   cmp.w     #3,(A6)
   bne       lFCA410
   move.b    D2,D0
   move.b    5(A5),D1
   bsr       RFCA1C2
   bge       lFCA458
lFCA410
   moveq     #3,D0
   move.w    D0,(A6)
   cmp.l     8(A6),D0
   bgt       lFCA470
   move.w    #1,2(A6)
   lea       $20(A6),A1
   moveq     #0,D0
   move.w    D4,D0
   divu      $3E(A6),D0
   add.w     D0,D0
   move.b    -2(A1,D0.w),(A5)
   move.b    -1(A1,D0.w),1(A5)
   swap      D0
   add.w     D0,D0
   move.b    -2(A1,D0.w),2(A5)
   move.b    -1(A1,D0.w),3(A5)
   move.b    D7,4(A5)
   move.b    D6,D0
   move.b    D2,D1
   bsr       RFCA20C
   move.b    D0,5(A5)
lFCA458
   addq.w    #1,D4
   cmp.w     $40(A6),D4
   bge       lFCA470
   moveq     #0,D0
   move.l    D4,D0
   divu      $3E(A6),D0
   swap      D0
   tst.w     D0
   bne       lFCA3F4
   bra       lFCA458
lFCA470
   addq.w    #1,D2
   cmp.w     D2,D3
   bgt       lFCA332
   bra       lFCA53C
FCA47C
   dc.l      $00000000,$00000000,$00000008,$00080810
   dc.w      $E800
lFCA48E
   move.w    #$E800,D0
   btst      D6,D0
   bne       lFCA4AC
   move.l    A2,D1
   rol.w     #8,D1
   swap      D1
   rol.w     #8,D1
   lea       $1C(A6),A2
   move.l    D1,(A2)
   cmp.b     #7,D6
   bne       lFCA4AC
   moveq     #4,D3
lFCA4AC
   move.b    0(A2,D2.w),D0
   cmp.b     D5,D0
   bne       lFCA4B8
   bsr       RFCA34A
lFCA4B8
   cmp.b     #7,D6
   bne       lFCA534
   move.b    D0,D1
   and.b     #$C0,D1
   cmp.b     #'@',D1
   bne       lFCA534
   and.b     #$1F,D0
   cmp.b     D5,D0
   bne       lFCA500
   cmp.w     #1,(A6)
   bne       lFCA4E6
   move.b    D2,D0
   move.b    1(A5),D1
   bsr       RFCA1C2
   addq.b    #1,D0
   bge       lFCA534
lFCA4E6
   move.l    #$010001,(A6)
   move.b    D7,(A5)
   move.b    D6,D0
   move.b    D2,D1
   bsr       RFCA20C
   or.b      #4,D0
   move.b    D0,1(A5)
   bra       lFCA534
lFCA500
   or.b      #$80,D0
   cmp.b     D5,D0
   bne       lFCA534
   cmp.w     #1,(A6)
   bne       lFCA51C
   move.b    D2,D0
   move.b    1(A5),D1
   bsr       RFCA1C2
   addq.b    #2,D0
   bge       lFCA534
lFCA51C
   move.l    #$010001,(A6)
   move.b    D7,(A5)
   move.b    D6,D0
   move.b    D2,D1
   bsr       RFCA20C
   or.b      #6,D0
   move.b    D0,1(A5)
lFCA534
   addq.w    #1,D2
   cmp.w     D2,D3
   bgt       lFCA4AC
lFCA53C
   movem.l   (SP)+,D2-D7/A2
   rts

_AZMapANSI
RFCA542
   movem.l   D2/A2-A6,-(SP)
   move.l    A2,D2
   bne       lFCA54E
   move.l    $22(A6),A2
lFCA54E
   sub.w     #$42,SP
   move.l    SP,A6
   movem.l   D0/D1/A0-A2,4(A6)
   move.l    A1,A5
   clr.l     $18(A6)
   clr.l     $3E(A6)
   lea       lFCA1D4(PC),A2
   bsr       RFCA576
   moveq     #0,D0
   move.w    $3E(A6),D0
   beq       lFCA5D6
   bra       lFCA5AE

RFCA576
   move.l    $14(A6),A0
   move.l    (A0),A3
   move.l    4(A0),A4
   moveq     #$3F,D2
lFCA582
   move.b    (A3)+,D0
   moveq     #$3F,D1
   sub.w     D2,D1
   move.l    (A4)+,A0
   jsr       (A2)
   dbra      D2,lFCA582
   move.l    $14(A6),A0
   move.l    $10(A0),A3
   move.l    $14(A0),A4
   moveq     #$37,D2
lFCA59E
   move.b    (A3)+,D0
   moveq     #$77,D1
   sub.w     D2,D1
   move.l    (A4)+,A0
   jsr       (A2)
   dbra      D2,lFCA59E
   rts
lFCA5AE
   lea       $20(A6),A0
   bra       lFCA5B8
lFCA5B4
   move.w    #$FFFF,(A0)+
lFCA5B8
   dbra      D0,lFCA5B4
   lea       lFCA22E(PC),A2
   bsr       RFCA576
   addq.w    #1,$3E(A6)
   move.w    $40(A6),D0
   beq       lFCA5D6
   addq.w    #1,D0
   mulu      $3E(A6),D0
   move.w    D0,$40(A6)
lFCA5D6
   tst.l     4(A6)
   ble       lFCA61C
   tst.l     8(A6)
   ble       lFCA5FA
   clr.l     (A6)
   lea       lFCA27E(PC),A2
   bsr       RFCA576
   moveq     #0,D0
   move.w    (A6),D0
   bne       lFCA5F4
   moveq     #0,D0
   bra       lFCA5FC
lFCA5F4
   cmp.l     8(A6),D0
   ble       lFCA602
lFCA5FA
   moveq     #-1,D0
lFCA5FC
   move.l    D0,$18(A6)
   bra       lFCA642
lFCA602
   sub.l     D0,8(A6)
   add.l     D0,$18(A6)
   add.l     D0,D0
   add.l     D0,A5
   move.w    2(A6),D0
   sub.l     D0,4(A6)
   add.l     D0,$0C(A6)
   bra       lFCA5D6
lFCA61C
   move.l    $18(A6),D2
   move.l    $10(A6),A0
   bra       lFCA63E
lFCA626
   move.w    (A0),D0
   btst      #3,D0
   beq       lFCA632
   bset      #$0F,D0
lFCA632
   move.b    D0,D1
   and.w     #7,D1
   move.b    bFCA650(PC,D1.w),D0
   move.w    D0,(A0)+
lFCA63E
   dbra      D2,lFCA626
lFCA642
   move.l    $18(A6),D0
   add.w     #$42,SP
   movem.l   (SP)+,D2/A2-A6
   rts
bFCA650
    dc.b      0,1,$10,$11,8,9,$18,$19,0,0,$FF
    dc.b      3,$FF,1,$FE,0,0,0,0,0,0,0,0,$FF
    dc.b      $BF,$FF,$EF,$FF,$EF,$FF,$F7,$47
    dc.b      $FF,$FF,$7F,0,$80,0,7,3,7,3,3
    dc.b      3,7,3,3,3,3,7,1,7,$80,0,7,7,$27
    dc.b      7,7,$27,$27,$27,$27,7,7,7,$80
    dc.b      0,0,0,$27,7,7,$27,$27,$27,$27
    dc.b      $27,7,1,1,$80,$80,0,0,0,3,7,7
    dc.b      7,7,7,$27,7,1,1,1,$80,0,0,0,0
    dc.b      $22,0,$41,0,4,2,0,$41,$41,$41
    dc.b      0,$41,$41,$41,$41,$41,$41,$41
    dc.b      $41,$41,$41,$41,$41,$41,$41,$41
    dc.b      0,0,0,0,0,$40,$80,$80,$80,$80
    dc.b      $80,$80,$80,$80,$80,$80,$80,$80
    dc.b      $80,$80,$41,$41,$41,$41,$80,$80
    dc.b      $80,$80,$80,$80,$7E,$60,$7E,$60
    dc.b      $21,$B9,$21,$31,$40,$B2,$40,$32
    dc.b      $23,$B3,$23,$33,$24,$A2,$24,$34
    dc.b      $25,$BC,$25,$35,$5E,$BD,$5E,$36
    dc.b      $26,$BE,$26,$37,$2A,$B7,$2A,$38
    dc.b      $28,$AB,$28,$39,$29,$BB,$29,$30
    dc.b      $5F,$2D,$5F,$2D,$2B,$3D,$2B,$3D
    dc.b      $7C,$5C,$7C,$5C,0,0,0,0,0,0,0
    dc.b      $30,$C5,$E5,$51,$71,$B0,$B0,$57
    dc.b      $77,0,$FC,$A9,$3A,$AE,$AE,$52
    dc.b      $72,$DE,$FE,$54,$74,0,$FC,$A9
    dc.b      $C6,0,$FC,$A9,$AA,0,$FC,$A9,$56
    dc.b      0,$FC,$A9,$8E,$B6,$B6,$50,$70
    dc.b      $7B,$5B,$7B,$5B,$7D,$5D,$7D,$5D
    dc.b      0,0,0,0,0,0,0,$31,0,0,0,$32,0
    dc.b      0,0,$33,0,$FC,$A9,$1E,$A7,$DF
    dc.b      $53,$73,$D0,$F0,$44,$64,0,$FC
    dc.b      $A8,$CE,0,$FC,$A8,$DE,0,$FC,$A8
    dc.b      $EE,0,$FC,$A8,$FE,0,$FC,$A9,$0E
    dc.b      $A3,$A3,$4C,$6C,$3A,$3B,$3A,$3B
    dc.b      $22,$27,$22,$27,0,0,0,0,0,0,0
    dc.b      0,0,0,0,$34,0,0,0,$35,0,0,0,$36
    dc.b      $BB,$AB,$3E,$3C,$AC,$B1,$5A,$7A
    dc.b      $F7,$D7,$58,$78,$C7,$E7,$43,$63
    dc.b      $AA,$AA,$56,$76,$BA,$BA,$42,$62
    dc.b      0,$FC,$A9,$72,$BF,$B8,$4D,$6D
    dc.b      $3C,$2C,$3C,$2C,$3E,$2E,$3E,$2E
    dc.b      $3F,$2F,$3F,$2F,0,0,0,0,0,0,0
    dc.b      $2E,0,0,0,$37,0,0,0,$38,0,0,0
    dc.b      $39,0,$FC,$A9,$E2,0,0,0,8,0,$FC
    dc.b      $A9,$EC,0,0,0,$0D,0,0,$0A,$0D
    dc.b      0,0,$9B,$1B,0,0,0,$7F,0,$FC,$A9
    dc.b      $F3,0,$FC,$A9,$FF,0,$FC,$AA,$0B
    dc.b      0,0,0,$2D,0,$FC,$AA,$17,0,$FC
    dc.b      $AA,$23,0,$FC,$AA,$2B,0,$FC,$AA
    dc.b      $33,0,$FC,$AA,$3C,0,$FC,$AA,$45
    dc.b      0,$FC,$AA,$50,0,$FC,$AA,$5B,0
    dc.b      $FC,$AA,$66,0,$FC,$AA,$71,0,$FC
    dc.b      $AA,$7C,0,$FC,$AA,$87,0,$FC,$AA
    dc.b      $92,0,$FC,$AA,$9D,0,$FC,$AA,$A8
    dc.b      0,0,0,$28,0,0,0,$29,0,0,0,$2F
    dc.b      0,0,0,$2A,0,0,0,$2B,0,$FC,$AA
    dc.b      $B3,0,0,0,0,0,0,0,0,0,0,0,0,0
    dc.b      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    dc.b      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    dc.b      0,0,0,0,0,0,0,0,0,0,0,0,0,0,$FC
    dc.b      $AA,$B8,0,$FC,$AA,$C4,0,$FC,$AA
    dc.b      $D0,0,$FC,$AA,$DC,0,0,0,0,0,0
    dc.b      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    dc.b      0,0,0,0,$66,0,$46,8,1,8,1,0,6
    dc.b      0,6,0,$86,0,$86,0,$67,0,$47,8
    dc.b      2,8,2,0,7,0,7,0,$87,0,$87,0,$68
    dc.b      0,$48,8,3,8,3,0,8,0,8,0,$88,0
    dc.b      $88,0,$6A,0,$4A,8,4,8,4,0,$0A
    dc.b      0,$0A,0,$8A,0,$8A,0,$6B,0,$4B
    dc.b      8,5,8,5,0,$0B,0,$0B,0,$8B,0,$8B
    dc.b      1,$10,1,$16,0,$E6,0,$C6,0,1,0
    dc.b      1,0,$81,0,$81,$61,$E1,$E0,$E2
    dc.b      $E3,$E4,$41,$C1,$C0,$C2,$C3,$C4
    dc.b      1,$10,1,$16,0,$A9,0,$A9,0,5,0
    dc.b      5,0,$85,0,$85,$65,$E9,$E8,$EA
    dc.b      $65,$EB,$45,$C9,$C8,$CA,$45,$CB
    dc.b      1,$10,1,$16,0,$A1,0,$A6,0,9,0
    dc.b      9,0,$89,0,$89,$69,$ED,$EC,$EE
    dc.b      $69,$EF,$49,$CD,$CC,$CE,$49,$CF
    dc.b      1,$10,1,$16,0,$AD,0,$AF,0,$0E
    dc.b      0,$0E,0,$8E,0,$8E,$6E,$6E,$6E
    dc.b      $6E,$F1,$6E,$4E,$4E,$4E,$4E,$D1
    dc.b      $4E,1,$10,1,$16,0,$F8,0,$D8,0
    dc.b      $0F,0,$0F,0,$8F,0,$8F,$6F,$F3
    dc.b      $F2,$F4,$F5,$F6,$4F,$D3,$D2,$D4
    dc.b      $D5,$D6,1,$10,1,$16,0,$B5,0,$B5
    dc.b      0,$15,0,$15,0,$95,0,$95,$75,$FA
    dc.b      $F9,$FB,$75,$FC,$55,$DA,$D9,$DB
    dc.b      $55,$DC,1,$10,1,$16,0,$A4,0,$A5
    dc.b      $55,$DC,1,$10,1,$16,0,$A4
    dc.b      0,$A5,0,$19,0,$19,0,$99
    dc.b      0,$99,$79,$FD,$79,$79,$79
    dc.b      $FF,$59,$DD,$59,$59,$59
    dc.b      $59,1,4,0,$A0,$20,$B4,$60
    dc.b      $5E,$7E,$A8,1,4,2,5,9,$9B
    dc.b      $5A,4,4,4,8,$9B,$34,$30
    dc.b      $7E,$9B,$35,$30,$7E,4,4
    dc.b      4,8,$9B,$34,$31,$7E,$9B
    dc.b      $35,$31,$7E,4,4,4,8,$9B
    dc.b      $34,$32,$7E,$9B,$35,$32
    dc.b      $7E,4,4,4,8,$9B,$32,$30
    dc.b      $7E,$9B,$33,$30,$7E,2,4
    dc.b      2,6,$9B,$41,$9B,$54,2,4
    dc.b      2,6,$9B,$42,$9B,$53,2,4
    dc.b      3,6,$9B,$43,$9B,$20,$40
    dc.b      2,4,3,6,$9B,$44,$9B,$20
    dc.b      $41,3,4,4,7,$9B,$30,$7E
    dc.b      $9B,$31,$30,$7E,3,4,4,7
    dc.b      $9B,$31,$7E,$9B,$31,$31
    dc.b      $7E,3,4,4,7,$9B,$32,$7E
    dc.b      $9B,$31,$32,$7E,3,4,4,7
    dc.b      $9B,$33,$7E,$9B,$31,$33
    dc.b      $7E,3,4,4,7,$9B,$34,$7E
    dc.b      $9B,$31,$34,$7E,3,4,4,7
    dc.b      $9B,$35,$7E,$9B,$31,$35
    dc.b      $7E,3,4,4,7,$9B,$36,$7E
    dc.b      $9B,$31,$36,$7E,3,4,4,7
    dc.b      $9B,$37,$7E,$9B,$31,$37
    dc.b      $7E,3,4,4,7,$9B,$38,$7E
    dc.b      $9B,$31,$38,$7E,3,4,4,7
    dc.b      $9B,$39,$7E,$9B,$31,$39
    dc.b      $7E,3,2,$9B,$3F,$7E,4,4
    dc.b      4,8,$9B,$34,$33,$7E,$9B
    dc.b      $35,$33,$7E,4,4,4,8,$9B
    dc.b      $32,$31,$7E,$9B,$33,$31
    dc.b      $7E,4,4,4,8,$9B,$34,$34
    dc.b      $7E,$9B,$35,$34,$7E,4,4
    dc.b      4,8,$9B,$34,$35,$7E,$9B
    dc.b      $35,$35,$7E,$75,$73,$61
    dc.b      0,$75,$73,$61,$31,0,0,0
    dc.b      0,$4A,$FC,0,$FC,$AA,$F4
    dc.b      0,$FC,$AB,$0E,1,$28,0,$D8
    dc.b      0,$FC,$AB,$28,0,$FC,$AB
    dc.b      $30,0,$FC,$AB,$58,$4A,$FC
    dc.b      0,$FC,$AB,$0E,0,$FC,$BA
    dc.b      $0C,1,$28,0,$C4,0,$FC,$AB
    dc.b      $3A,0,$FC,$AB,$40,0,$FC
    dc.b      $AD,$78,$72,$6F,$6D,$62
    dc.b      $6F,$6F,$74,0,$72,$6F,$6D
    dc.b      $62,$6F,$6F,$74,$0D,$0A
    dc.b      0,$73,$74,$72,$61,$70,0
    dc.b      $73,$74,$72,$61,$70,$20
    dc.b      $34,$30,$2E,$31,$20,$28
    dc.b      $38,$2E,$33,$2E,$39,$33
    dc.b      $29,$0D,$0A,0,0,0,$48,$E7
