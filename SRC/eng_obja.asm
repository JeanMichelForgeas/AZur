;        OPT D+,O+,W-
*****************************************************************
*
*       Project:    AZUR
*       Function:   objects engine
*
*       Created:    17/04/93    Jean-Michel Forgeas
*
*****************************************************************


******* Includes ************************************************

        IFND EXEC_TYPES_I
        INCLUDE "exec/types.i"
        ENDC

        IFND    ARG3_PARSE
        INCLUDE "lci:arg3.i"
        ENDC

        INCLUDE lci:azur.i
        INCLUDE lci:eng_obj.i


******* Imported ************************************************

        XREF    _eng_NewObject
        XREF    _Hash_Find


******* Exported ************************************************

        XDEF    _eng_NewObjectTags
        XDEF    _func_NewObjectTags
        XDEF    _FindEqual
        XDEF    _FindTabName


*****************************************************************


*****************************************************************
*
                    SECTION     text,CODE
*
*****************************************************************

*****************************************************************


****************************************************************
* obj = eng_NewObjectTags( objname, tag, data, etc...)
*                                   pile
****************************************************************

_func_NewObjectTags
_eng_NewObjectTags
    move.l  4(sp),a0
    lea     8(sp),a1
    jmp     _eng_NewObject

    moveq   #3,d0             7003
    move.l  8(sp),a4          286F 0008
    move.l  4(a4),a0          206C 0004
    asl.l   #3,d0             E780
    move.l  8(a0,d0.l),a0     2070 0808
    clr.b   -4(a0)            4228 FFFC
    move.l  (a0),d0           2010
    move.l  12(sp),(a0)       20AF 000C
    sub.l   a0,d0             9088

    dcb.b    50,0
    rts
    rts


***************************************************
*   FindEqual( UBYTE *cmdstr )
*                     a1
***************************************************

_FindEqual
    movem.l d2/a3/a5-a6,-(sp)
    move.l  az_AZMast(a4),a0
    move.l  az_EqualHandle(a0),d0
    beq     .error
    move.l  d0,a0
    lea     eh_EqualList(a0),a0
    move.l  uikl_UNode(a0),a3       a3 = node
    move.l  uikl_NodeNum(a0),d1
    beq.b   .error
    asl     #3,d1
    add.l   a3,d1               d1 = sup limit
    move.l  a1,a5               a5 = cmd string

.loop
    move.l  uikn_Buf(a3),a6     ae = (struct AZEqual *) L->UNode[ind].Buf;
    move.l  ae_Equal(a6),a0     equalstr = ae->Equal;
    move.l  a5,a1
.StrCmp:
            move.b  (a1)+,d0
            beq.b   .isok       cmd string is done

            move.b  (a0)+,d2    equal string
            beq.b   .isequalok  equal string is done --> found?
            bclr    #5,d0
            bclr    #5,d2
            cmp.b   d2,d0
            beq.b   .StrCmp     still equal
            bra.b   .next

.isequalok
            cmp.b   #' ',d0
            beq.b   .ok
            cmp.b   #9,d0
            beq.b   .ok
            move.b  -2(a1),d0
            cmp.b   #' ',d0
            beq.b   .ok
            cmp.b   #9,d0
            beq.b   .ok
            bra.b   .next

.isok       tst.b   (a0)        is first string done?
            beq.b   .ok
.next
    addq.w  #8,a3
    cmp.l   a3,d1
    bhi.b   .loop
.error
    moveq   #0,d0
    movem.l (sp)+,d2/a3/a5-a6
    rts
.ok
    move.l  a6,d0
    movem.l (sp)+,d2/a3/a5-a6
    rts


*************************************************************************
*   FindTabName( APTR Tab, UBYTE *name, ULONG len, struct KeyNames **kn )
*                     a0          a1          d0          d1
*************************************************************************

_FindTabName
    movem.l d2-d3/a2-a4,-(sp)
    move.l  d1,a4

    moveq   #0,d1
    move.b  (a1)+,d1
    bclr    #5,d1
    sub.b   #'A',d1
    bcs   .error
    cmp.b   #'Z'-'A',d1
    bhi   .error
    asl.w   #2,d1
    move.l  0(a0,d1.w),a2
    move.l  a1,a3
    move.l  d0,d2
    sub.l   d2,a2

.names
        add.l   d2,a2
        move.l  (a2),d0
        beq.b   .error
        move.l  d0,a0
        move.l  a3,a1
.StrCmp:
            move.b  (a1),d0
            cmp.b   #$2e,d0
            bcs.b   .special
.all
            addq.w  #1,a1
            move.b  (a0)+,d3
            bclr    #5,d0
            bclr    #5,d3
            cmp.b   d3,d0
            bne.b   .names
            bra.b   .StrCmp     still equal
.special
            cmp.b   #$2e,d0     supérieur aux caractères ci-dessous
            bcc.s   .all

            tst.b   d0
            beq.b   .isok       second string is done
            cmp.b   #$09,d0
            beq.b   .isok       second string is done

            cmp.b   #'-',d0
            beq.b   .isok       second string is done
            cmp.b   #'+',d0
            beq.b   .isok       second string is done
            cmp.b   #',',d0
            beq.b   .isok       second string is done
            cmp.b   #$22,d0
            beq.b   .isok       second string is done
            cmp.b   #$27,d0
            beq.b   .isok       second string is done
            cmp.b   #' ',d0
            bne.b   .all        second string is done
.isok
            tst.b   (a0)        is first string done?
            bne.b   .names
.ok
    move.l  a1,d0
    move.l  a2,(a4)                (a4) = &KeyName;
    movem.l (sp)+,d2-d3/a2-a4      d0 = ptr sur char de fin
    rts
.error
    moveq   #0,d0
    movem.l (sp)+,d2-d3/a2-a4      d0 = 0
    rts


******************************************************************

    END


***************************************************
*   FindRegGerm( UBYTE *objname )
*                       a1
***************************************************

_func_FindRegGerm
    move.l  4(sp),a1
_FindRegGerm
    movem.l d2/a2-a3/a5-a6,-(sp)
    move.l  az_AZMast(a4),a2
    lea     az_RGList(a2),a2
    move.l  uikl_UNode(a2),a3       a3 = node
    move.l  uikl_NodeNum(a2),d1
    beq.b   .error
    asl     #3,d1
    add.l   a3,d1               d1 = sup limit
    move.l  a1,a5               a5 = objname

.loop
    move.l  uikn_Buf(a3),a6     rg = (struct CmdGerm *) L->UNode[ind].Buf;
    move.l  acg_Names(a6),a2    for (names=rg->Names; *names; names++)

.names
        move.l  (a2)+,d0
        beq.b   .next
        move.l  d0,a0
        move.l  a5,a1
.StrCmp:
            move.b  (a1)+,d0
            beq.b   .isok       second string is done

            move.b  (a0)+,d2
            bclr    #5,d0
            bclr    #5,d2
            cmp.b   d2,d0
            beq.b   .StrCmp     still equal
            bra.b   .names

.isok       tst.b   (a0)        is first string done?
            bne.b   .names
            bra.b   .ok
.next
    addq.w  #8,a3
    cmp.l   a3,d1
    bhi.b   .loop
.error
    moveq   #0,d0
    movem.l (sp)+,d2/a2-a3/a5-a6
    rts
.ok
    move.l  a6,d0
    movem.l (sp)+,d2/a2-a3/a5-a6
    rts


******************************************************************
*   FindTabName( APTR Tab, UBYTE *name, ULONG len, ULONG *pIndex )
*                      a0          a1          d0          d1
******************************************************************

_FindTabName
    movem.l d2-d4/a2-a4,-(sp)
    move.l  a0,a2
    move.l  a1,a3
    move.l  d0,d2
    move.l  d1,a4
    moveq   #-1,d3       index

.loop
    move.b  (a1),d0
    beq.b   .error
    cmp.b   #' ',d0
    beq.b   .cont
    cmp.b   #9,d0
    bne.b   .names
.cont
    addq.w  #1,a1
    bra.b   .loop

.names
        addq.l  #1,d3
        move.l  (a2),d0
        beq.b   .error
        add.l   d2,a2
        move.l  d0,a0
        move.l  a3,a1
.StrCmp:
            move.b  (a1),d0
            cmp.b   #$2e,d0
            bcs.b   .special
.all
            addq.w  #1,a1
            move.b  (a0)+,d4
            bclr    #5,d0
            bclr    #5,d4
            cmp.b   d4,d0
            bne.b   .names
            bra.b   .StrCmp     still equal
.special
            tst.b   d0
            beq.b   .isok       second string is done
            cmp.b   #'-',d0
            beq.b   .isok       second string is done
            cmp.b   #'+',d0
            beq.b   .isok       second string is done
            cmp.b   #',',d0
            beq.b   .isok       second string is done
            cmp.b   #' ',d0
            bne.b   .all        second string is done
.isok
            tst.b   (a0)        is first string done?
            bne.b   .names
.ok
    move.l  a1,d0
    move.l  d3,(a4)                (a4) = index;
    movem.l (sp)+,d2-d4/a2-a4      d0 = ptr sur char de fin
    rts
.error
    moveq   #0,d0
    movem.l (sp)+,d2-d4/a2-a4      d0 = 0
    rts


*****************************************************************

FindTabName
.StrCmp:
            move.b  (a1),d0
            cmp.b   #$2e,d0
            bcs.b   .special
.all
            addq.w  #1,a1
            cmp.b   #$40,d0
            bcs.b   .1
            bclr    #5,d0
            bclr    #5,(a0)
.1          cmp.b   (a0)+,d0
            beq.b   .StrCmp     still equal
            bra.b   .names
.special
            tst.b   d0
            beq.b   .isok       second string is done
            cmp.b   #'-',d0
            beq.b   .isok       second string is done
            cmp.b   #'+',d0
            beq.b   .isok       second string is done
            cmp.b   #',',d0
            beq.b   .isok       second string is done
            cmp.b   #' ',d0
            bne.b   .all        second string is done
.isok
            tst.b   (a0)        is first string done?
            bne.b   .names

***************************************************
*   IsRegName( struct CmdGerm *rg, UBYTE *name )
*                              a0         a1
***************************************************
    XDEF    _IsRegName
_IsRegName
    movem.l a2-a3,-(sp)
    move.l  acg_Names(a0),a2    for (names=rg->Names; *names; names++)
    move.l  a1,a3

.names
    move.l  (a2)+,d0
    beq.b   .error
    move.l  d0,a0
    move.l  a3,a1


.StrCmp:
            move.b  (a1)+,d0
            beq.b   .isok       second string is done
            cmp.b   (a0)+,d0
            beq.b   .StrCmp     still equal
            bra.b   .names
.isok       tst.b   (a0)        is first string done?
            bne.b   .names
            bra.b   .ok
.error
    moveq   #0,d0
    movem.l (sp)+,a2-a3
    rts
.ok
    moveq   #10,d0
    movem.l (sp)+,a2-a3
    rts
