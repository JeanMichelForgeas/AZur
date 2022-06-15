    OPT D+,O+,W-
*****************************************************************
*
*   Project:    DicServ
*   Function:   Entry search
*
*   Created:    23/12/94        Jean-Michel Forgeas
*
*****************************************************************


******* Includes ************************************************

    INCLUDE "exec/types.i"

 STRUCTURE  GroupHeader,0
    ULONG       gh_GroupLen     ; lui-même y-comris
    UWORD       gh_NumWords
    UBYTE       gh_pad
    UBYTE       gh_GroupChar
    LABEL   GroupHeader_SIZE    ; si change et > 8 modifier le addq.w

 STRUCTURE  WordInfo,0
    UBYTE       wi_WordFlags    ; en asm : << --> comme sur un long
    UBYTE       wi_EntryLen     ; taille de tous les WordInfo de cette entry
    UBYTE       wi_WordLen      ; offset à ajouter pour avoir le WordInfo suivant
    UBYTE       wi_Word
    LABEL   WordInfo_SIZE

;------ WordFlags
SB_STRING2      equ 0
SB_CONTINUE     equ 1
SB_CASECOMP     equ 2
SB_COMPLEMENT   equ 3
SB_ISVERB       equ 4
SB_ISADJECTIVE  equ 5

;------ Working flags
FUB_TRANSLATE   equ 0
FUB_CASE        equ 1
FUB_ACCENT      equ 2

MAXFOUNDENTRIES equ 100

* Commodore DBUG macro (needs debug.lib)
* Usage: pass name of format string,value,value
*        values may be register name, variablename(PC), or #value
* ex: DBUG fmtRtnD0,#testname,d0  ; format string, addr of rtn name, d0
DBUG    MACRO   * passed name of format string, with args on stack
        movem.l d0-d7/a0-a6,-(sp)
        move.l  \3,-(sp)
        move.l  \2,-(sp)
        lea     .fmt\@(pc),a0
        lea     (sp),a1
        XREF    KPrintF
        jsr     KPrintF
        addq.w  #8,sp
        movem.l (sp)+,d0-d7/a0-a6
        bra.b   .\@
.fmt\@  dc.b    \1,0
        cnop    0,4
.\@
        ENDM


******* Imported ************************************************


******* Exported ************************************************

    XDEF    _asm_SearchEntry
    XDEF    _asm_StrStripAccent
    XDEF    _asm_StrToLower
    XDEF    _asm_StrToUpper
    XDEF    _asm_StrIsWord

    XDEF    _LowerCharDef
    XDEF    _NormalCharDef
    XDEF    _LowerNoAccentCharDef
    XDEF    _NoAccentCharDef
    XDEF    _UpperCharDef
    XDEF    _WordCharDef

    XDEF    _FoundTable


*****************************************************************


*****************************************************************
*
            SECTION text,CODE
*
*****************************************************************


***************************************************************************
* ULONG asm_SearchEntry( struct GroupHeader *gh, UBYTE *str, ULONG flags );
*
* La chaîne qui arrive est déjà convertie en minuscules si la recherche
* n'est pas demandée en FUF_CASE, donc ne pas convertir la chaîne en
* entrèe. Pareil pour FUF_ACCENT.
***************************************************************************

_asm_SearchEntry
        movem.l 4(sp),a1                a1 = struct GroupHeader *gh
        movem.l 8(sp),a0                a0 = UBYTE *str
        movem.l 12(sp),d0               d0 = flags
        movem.l d2-d5/d7/a2-a5,-(sp)
        moveq   #0,d3
        moveq   #'*',d4
        move.l  d0,d5

        lea     _FoundTable(pc),a4
        clr.w   FoundNum

        btst    #FUB_CASE,d5
        bne.s   .do_case
.do_nocase
            btst    #FUB_ACCENT,d5
            bne.s   .do_accent1
.do_noaccent1
            lea     _LowerNoAccentCharDef(pc),a5
            bra.s   .start
.do_accent1
            lea     _LowerCharDef(pc),a5
            bra.s   .start
.do_case
            btst    #FUB_ACCENT,d5
            bne.s   .do_accent2
.do_noaccent2
            lea     _NoAccentCharDef(pc),a5
            bra.s   .start
.do_accent2
            lea     _NormalCharDef(pc),a5
            bra.s   .start

.start
        move.w  gh_NumWords(a1),d7
        beq.s   .end
        subq.w  #1,d7                   d7 = nombre d'entrées - 1
        addq.w  #GroupHeader_SIZE,a1    a1 pointe sur le 1er WordInfo
        move.b  (a0)+,d0                d0 = 1er char du mot à chercher, a0 sur 2ème char (faire attention si d0==0)

        move.l  a0,a2
        move.l  a1,a3
.loop
        move.l  (a1)+,d1                d1 = WordFlags, EntryLen, WordLen, Word[0] == 1ère lettre du mot
        move.b  d1,d3                   d1.b jamais égal à 0 car testé lors de AddEntry
        move.b  0(a5,d3.w),d3
        cmp.b   d0,d3                   1ère lettre ok ?
        bne.s   .next_entry
        move.l  a0,a2                   a2 et a3 pointent sur 2ème char
        move.l  a1,a3
        move.b  d0,d2
.cmp
            tst.b   d2
            beq.s   .ok
            move.b  (a2)+,d2
            move.b  (a3)+,d3
            move.b  0(a5,d3.w),d3
            cmp.b   d2,d3
            beq.s   .cmp
            cmp.b   d4,d2   '*'
            beq.s   .ok
            cmp.b   d4,d3   '*'
            beq.s   .ok

.next_entry
        swap    d1
        ext.w   d1              EntryLen
        add.w   d1,a1           WordInfo suivant
        dbra    d7,.loop
        bra.s   .end
.ok
        btst    #FUB_TRANSLATE,d5
        beq.s   .okstore
        btst    #SB_STRING2+24,d1
        beq.s   .next_entry
.okstore
        move.w  FoundNum(pc),d2
        addq.w  #1,FoundNum
        add.w   d2,d2
        add.w   d2,d2
        subq.w  #4,a1           début du WordInfo
        move.l  a1,0(a4,d2.w)   stocke le WordInfo trouvé
        addq.w  #4,a1
        bra.s   .next_entry     cherche une autre occurence possible à cause des '*'
.end
        moveq   #0,d0
        move.w  FoundNum,d0
        movem.l (sp)+,d2-d5/d7/a2-a5
        rts


_FoundTable dcb.b   4*MAXFOUNDENTRIES
FoundNum    dc.w    0

; print23
;     ext.w   d2
;     ext.l   d2
;     DBUG    <'%lc%lc '>,d2,d3
;     rts
; println
;     DBUG    <10>,d2,d3
;     rts

_NormalCharDef
    dc.b    $00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0A,$0B,$0C,$0D,$0E,$0F
    dc.b    $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$1A,$1B,$1C,$1D,$1E,$1F
    dc.b    $20,$21,$22,$23,$24,$25,$26,$27,$28,$29,$2A,$2B,$2C,$2D,$2E,$2F
    dc.b    $30,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3A,$3B,$3C,$3D,$3E,$3F
    dc.b    $40,$41,$42,$43,$44,$45,$46,$47,$48,$49,$4A,$4B,$4C,$4D,$4E,$4F
    dc.b    $50,$51,$52,$53,$54,$55,$56,$57,$58,$59,$5A,$5B,$5C,$5D,$5E,$5F
    dc.b    $60,$61,$62,$63,$64,$65,$66,$67,$68,$69,$6A,$6B,$6C,$6D,$6E,$6F
    dc.b    $70,$71,$72,$73,$74,$75,$76,$77,$78,$79,$7A,$7B,$7C,$7D,$7E,$7F
    dc.b    $80,$81,$82,$83,$84,$85,$86,$87,$88,$89,$8A,$8B,$8C,$8D,$8E,$8F
    dc.b    $90,$91,$92,$93,$94,$95,$96,$97,$98,$99,$9A,$9B,$9C,$9D,$9E,$9F
    dc.b    $A0,$A1,$A2,$A3,$A4,$A5,$A6,$A7,$A8,$A9,$AA,$AB,$AC,$AD,$AE,$AF
    dc.b    $B0,$B1,$B2,$B3,$B4,$B5,$B6,$B7,$B8,$B9,$BA,$BB,$BC,$BD,$BE,$BF
    dc.b    $C0,$C1,$C2,$C3,$C4,$C5,$C6,$C7,$C8,$C9,$CA,$CB,$CC,$CD,$CE,$CF
    dc.b    $D0,$D1,$D2,$D3,$D4,$D5,$D6,$D7,$D8,$D9,$DA,$DB,$DC,$DD,$DE,$DF
    dc.b    $E0,$E1,$E2,$E3,$E4,$E5,$E6,$E7,$E8,$E9,$EA,$EB,$EC,$ED,$EE,$EF
    dc.b    $F0,$F1,$F2,$F3,$F4,$F5,$F6,$F7,$F8,$F9,$FA,$FB,$FC,$FD,$FE,$FF

_LowerNoAccentCharDef
    dc.b    $00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0A,$0B,$0C,$0D,$0E,$0F
    dc.b    $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$1A,$1B,$1C,$1D,$1E,$1F
    dc.b    $20,$21,$22,$23,$24,$25,$26,$27,$28,$29,$2A,$2B,$2C,$2D,$2E,$2F
    dc.b    $30,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3A,$3B,$3C,$3D,$3E,$3F
    dc.b    $40,$61,$62,$63,$64,$65,$66,$67,$68,$69,$6A,$6B,$6C,$6D,$6E,$6F
    dc.b    $70,$71,$72,$73,$74,$75,$76,$77,$78,$79,$7A,$5B,$5C,$5D,$5E,$5F
    dc.b    $60,$61,$62,$63,$64,$65,$66,$67,$68,$69,$6A,$6B,$6C,$6D,$6E,$6F
    dc.b    $70,$71,$72,$73,$74,$75,$76,$77,$78,$79,$7A,$7B,$7C,$7D,$7E,$7F
    dc.b    $80,$81,$82,$83,$84,$85,$86,$87,$88,$89,$8A,$8B,$8C,$8D,$8E,$8F
    dc.b    $90,$91,$92,$93,$94,$95,$96,$97,$98,$99,$9A,$9B,$9C,$9D,$9E,$9F
    dc.b    $A0,$A1,$A2,$A3,$A4,$A5,$A6,$A7,$A8,$A9,$AA,$AB,$AC,$AD,$AE,$AF
    dc.b    $B0,$B1,$B2,$B3,$B4,$B5,$B6,$B7,$B8,$B9,$BA,$BB,$BC,$BD,$BE,$BF
    dc.b    $61,$61,$61,$61,$61,$61,$E6,$63,$65,$65,$65,$65,$69,$69,$69,$69
    dc.b    $D0,$6E,$6F,$6F,$6F,$6F,$6F,$F7,$4F,$75,$75,$75,$75,$79,$FE,$79
    dc.b    $61,$61,$61,$61,$61,$61,$E6,$63,$65,$65,$65,$65,$69,$69,$69,$69
    dc.b    $6F,$6E,$6F,$6F,$6F,$6F,$6F,$F7,$6F,$75,$75,$75,$75,$79,$FE,$79


*************************************************************************
* void asm__StrStripAccent( UBYTE *str );
*                                  a0
*************************************************************************

_asm_StrStripAccent
        move.l  4(sp),a0
        moveq   #0,d0
.loop   move.b  (a0),d0
        move.b  _NoAccentCharDef(pc,d0.w),(a0)+
        bne.s   .loop
.end    rts

_NoAccentCharDef
    dc.b    $00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0A,$0B,$0C,$0D,$0E,$0F
    dc.b    $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$1A,$1B,$1C,$1D,$1E,$1F
    dc.b    $20,$21,$22,$23,$24,$25,$26,$27,$28,$29,$2A,$2B,$2C,$2D,$2E,$2F
    dc.b    $30,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3A,$3B,$3C,$3D,$3E,$3F
    dc.b    $40,$41,$42,$43,$44,$45,$46,$47,$48,$49,$4A,$4B,$4C,$4D,$4E,$4F
    dc.b    $50,$51,$52,$53,$54,$55,$56,$57,$58,$59,$5A,$5B,$5C,$5D,$5E,$5F
    dc.b    $60,$61,$62,$63,$64,$65,$66,$67,$68,$69,$6A,$6B,$6C,$6D,$6E,$6F
    dc.b    $70,$71,$72,$73,$74,$75,$76,$77,$78,$79,$7A,$7B,$7C,$7D,$7E,$7F
    dc.b    $80,$81,$82,$83,$84,$85,$86,$87,$88,$89,$8A,$8B,$8C,$8D,$8E,$8F
    dc.b    $90,$91,$92,$93,$94,$95,$96,$97,$98,$99,$9A,$9B,$9C,$9D,$9E,$9F
    dc.b    $A0,$A1,$A2,$A3,$A4,$A5,$A6,$A7,$A8,$A9,$AA,$AB,$AC,$AD,$AE,$AF
    dc.b    $B0,$B1,$B2,$B3,$B4,$B5,$B6,$B7,$B8,$B9,$BA,$BB,$BC,$BD,$BE,$BF
    dc.b    $41,$41,$41,$41,$41,$41,$C6,$43,$45,$45,$45,$45,$49,$49,$49,$49
    dc.b    $D0,$4E,$4F,$4F,$4F,$4F,$4F,$D7,$4F,$55,$55,$55,$55,$59,$DE,$DF
    dc.b    $61,$61,$61,$61,$61,$61,$E6,$63,$65,$65,$65,$65,$69,$69,$69,$69
    dc.b    $6F,$6E,$6F,$6F,$6F,$6F,$6F,$F7,$6F,$75,$75,$75,$75,$79,$FE,$79


*************************************************************************
* void asm_StrToLower( UBYTE *str );
*                             a0
*************************************************************************

_asm_StrToLower
        move.l  4(sp),a0
        moveq   #0,d0
.loop   move.b  (a0),d0
        move.b  _LowerCharDef(pc,d0.w),(a0)+
        bne.s   .loop
.end    rts

_LowerCharDef
    dc.b    $00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0A,$0B,$0C,$0D,$0E,$0F
    dc.b    $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$1A,$1B,$1C,$1D,$1E,$1F
    dc.b    $20,$21,$22,$23,$24,$25,$26,$27,$28,$29,$2A,$2B,$2C,$2D,$2E,$2F
    dc.b    $30,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3A,$3B,$3C,$3D,$3E,$3F
    dc.b    $40,$61,$62,$63,$64,$65,$66,$67,$68,$69,$6A,$6B,$6C,$6D,$6E,$6F
    dc.b    $70,$71,$72,$73,$74,$75,$76,$77,$78,$79,$7A,$5B,$5C,$5D,$5E,$5F
    dc.b    $60,$61,$62,$63,$64,$65,$66,$67,$68,$69,$6A,$6B,$6C,$6D,$6E,$6F
    dc.b    $70,$71,$72,$73,$74,$75,$76,$77,$78,$79,$7A,$7B,$7C,$7D,$7E,$7F
    dc.b    $80,$81,$82,$83,$84,$85,$86,$87,$88,$89,$8A,$8B,$8C,$8D,$8E,$8F
    dc.b    $90,$91,$92,$93,$94,$95,$96,$97,$98,$99,$9A,$9B,$9C,$9D,$9E,$9F
    dc.b    $A0,$A1,$A2,$A3,$A4,$A5,$A6,$A7,$A8,$A9,$AA,$AB,$AC,$AD,$AE,$AF
    dc.b    $B0,$B1,$B2,$B3,$B4,$B5,$B6,$B7,$B8,$B9,$BA,$BB,$BC,$BD,$BE,$BF
    dc.b    $E0,$E1,$E2,$E3,$E4,$E5,$E6,$E7,$E8,$E9,$EA,$EB,$EC,$ED,$EE,$EF
    dc.b    $D0,$F1,$F2,$F3,$F4,$F5,$F6,$F7,$D8,$F9,$FA,$FB,$FC,$FD,$FE,$FF
    dc.b    $E0,$E1,$E2,$E3,$E4,$E5,$E6,$E7,$E8,$E9,$EA,$EB,$EC,$ED,$EE,$EF
    dc.b    $F0,$F1,$F2,$F3,$F4,$F5,$F6,$F7,$F8,$F9,$FA,$FB,$FC,$FD,$FE,$FF


*************************************************************************
* void asm_StrToUpper( UBYTE *str );
*                             a0
*************************************************************************

_asm_StrToUpper
        move.l  4(sp),a0
        moveq   #0,d0
.loop   move.b  (a0),d0
        move.b  _UpperCharDef(pc,d0.w),(a0)+
        bne.s   .loop
.end    rts

_UpperCharDef
    dc.b    $00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0A,$0B,$0C,$0D,$0E,$0F
    dc.b    $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$1A,$1B,$1C,$1D,$1E,$1F
    dc.b    $20,$21,$22,$23,$24,$25,$26,$27,$28,$29,$2A,$2B,$2C,$2D,$2E,$2F
    dc.b    $30,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3A,$3B,$3C,$3D,$3E,$3F
    dc.b    $40,$41,$42,$43,$44,$45,$46,$47,$48,$49,$4A,$4B,$4C,$4D,$4E,$4F
    dc.b    $50,$51,$52,$53,$54,$55,$56,$57,$58,$59,$5A,$5B,$5C,$5D,$5E,$5F
    dc.b    $60,$41,$42,$43,$44,$45,$46,$47,$48,$49,$4A,$4B,$4C,$4D,$4E,$4F
    dc.b    $50,$51,$52,$53,$54,$55,$56,$57,$58,$59,$5A,$7B,$7C,$7D,$7E,$7F
    dc.b    $80,$81,$82,$83,$84,$85,$86,$87,$88,$89,$8A,$8B,$8C,$8D,$8E,$8F
    dc.b    $90,$91,$92,$93,$94,$95,$96,$97,$98,$99,$9A,$9B,$9C,$9D,$9E,$9F
    dc.b    $A0,$A1,$A2,$A3,$A4,$A5,$A6,$A7,$A8,$A9,$AA,$AB,$AC,$AD,$AE,$AF
    dc.b    $B0,$B1,$B2,$B3,$B4,$B5,$B6,$B7,$B8,$B9,$BA,$BB,$BC,$BD,$BE,$BF
    dc.b    $C0,$C1,$C2,$C3,$C4,$C5,$C6,$C7,$C8,$C9,$CA,$CB,$CC,$CD,$CE,$CF
    dc.b    $D0,$D1,$D2,$D3,$D4,$D5,$D6,$D7,$D8,$D9,$DA,$DB,$DC,$DD,$DE,$DF
    dc.b    $C0,$C1,$C2,$C3,$C4,$C5,$C6,$C7,$C8,$C9,$CA,$CB,$CC,$CD,$CE,$CF
    dc.b    $F0,$D1,$D2,$D3,$D4,$D5,$D6,$F7,$D8,$D9,$DA,$DB,$DC,$DD,$DE,$FF


*************************************************************************
* void asm_StrIsWord( UBYTE *str );
*                            a0
*************************************************************************

_asm_StrIsWord
        move.l  4(sp),a0
        moveq   #0,d0
.loop   move.b  (a0)+,d0
        beq.s   .ok
        move.b  _WordCharDef(pc,d0.w),d0
        bne.s   .loop
        bra.s   .error
.ok     moveq   #0,d0
        rts
.error  moveq   #-1,d0
        rts

_WordCharDef
;          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;          |0 1 2 3 4 5 6 7 8 9 a b c d e f|
;+---------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;| 0000 00 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 0001 10 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 0010 20 |  ! " # $ % & ' ( ) * + , - . /|
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 0011 30 |0 1 2 3 4 5 6 7 8 9 : ; < = > ?|
    dc.b    1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
;| 0100 40 |@ A B C D E F G H I J K L M N O|
    dc.b    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 0101 50 |P Q R S T U V W X Y Z [ \ ] ^ _|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1
;| 0110 60 |` a b c d e f g h i j k l m n o|
    dc.b    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 0111 70 |p q r s t u v w x y z { } ~  |
    dc.b    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0
;| 1000 80 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1001 90 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1010 a0 |  ¡ ¢ £ ¤ ¥ ¦ § ¨ © ª « ¬ ­ ® ¯|
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1011 b0 |° ± ² ³ ´ µ ¶ · ¸ ¹ º » ¼ ½ ¾ ¿|
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1100 c0 |À Á Â Ã Ä Å Æ Ç È É Ê Ë Ì Í Î Ï|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 1101 d0 |Ð Ñ Ò Ó Ô Õ Ö × Ø Ù Ú Û Ü Ý Þ ß|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 1110 e0 |à á â ã ä å æ ç è é ê ë ì í î ï|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 1111 f0 |ð ñ ò ó ô õ ö ÷ ø ù ú û ü ý þ ÿ|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;+---------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


