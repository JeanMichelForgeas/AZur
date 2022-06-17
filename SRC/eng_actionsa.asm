;    OPT D+,O+,W-
*****************************************************************
*
*	Project:    AZUR
*	Function:   objects engine
*
*	Created:    12/04/93	Jean-Michel Forgeas
*
*****************************************************************


*******	Includes ************************************************

	INCLUDE	"exec/memory.i"
	INCLUDE	"lib/exec_lib.i"
	INCLUDE	"lib/dos_lib.i"

	INCLUDE	"uiki:uik_lib.i"

	INCLUDE	"lci:azur.i"
	INCLUDE	"lci:text.i"
	INCLUDE	"lci:eng_obj.i"


*******	Imported ************************************************

	XREF	_intAZ_SemaRead
	XREF	_intAZ_SemaFreeRead
	XREF	_intAZ_SemaWrite
	XREF	_intAZ_SemaFreeWrite


*******	Exported ************************************************

	XDEF	_eng_DoAction
	XDEF	_eng_RGNewAction
	XDEF	_eng_RGDeleteAction
	XDEF	_eng_RGNewAllActions
	XDEF	_eng_RGDeleteAllActions

	XDEF	_eng_RGNewActionFunc

	XDEF	_eng_Tags2Struct

	XDEF	_func_DoAction
	XDEF	_func_NewAction
	XDEF	_func_DeleteAction
	XDEF	_func_NewAllActions
	XDEF	_func_DeleteAllActions
	XDEF	_func_NewActionFunc


*****************************************************************


*****************************************************************
*
		    SECTION	text,CODE
*
*****************************************************************


*****************************************************************
*   ULONG eng_DoAction(	struct CmdObj *ao, UWORD type, ULONG parm );
*				       a1	 d0	     d1
*****************************************************************

_func_DoAction
    move.l  4(sp),a1
    movem.l 8(sp),d0-d1
_eng_DoAction
    ;------ Protection
    IFEQ    AZUR_PRO
    cmp.w   #(AZT_ActionUndo&$ffff),d0
    beq	    .executing
    ENDC

    ;------ Existe? execute?
    move.l  aco_RegGerm(a1),a0
    move.l  acg_Actions(a0),a0	    UIKBuffer des actions
    lsl.w   #2,d0
    move.l  0(a0,d0.w),d0	    pointeur sur des fonctions?
    beq	    .executing

    bset    #AZAFB_EXECUTING-24,aco_Flags(a1)
    bne	    .executing		     en	cours d'exécution

    ;------ Inits
    clr.b   aco_ResCode(a1)
    move.b  aco_ExeFrom(a1),az_ExeFrom(a4) ; pour création d'un	nouvel objet pendant DoAction()

    movem.l d2/a1-a3/a5,-(sp)

    ;------ Semaphore :	plusieurs actions doivent pouvoir s'exécuter en	même temps
    ;move.l  az_AZMast(a4),a0
    ;move.l  az_ObjSema(a0),a0
    ;jsr     _intAZ_SemaRead

    move.l  aco_RegGerm(a1),a2
    ;addq.l  #1,acg_SemaCount(a2)

    ;------ Exécution
    move.l  d0,a3		    a3 = pointeur sur les fonctions
    move.l  d1,a5
    move.l  a1,a2		    a2 = CmdObj
    move.l  -nubu_CurSize(a3),d2
    lsr.l   #3,d2		    d2 = nombre	de fonctions = len / CmdFunc_SIZE
    subq.l  #1,d2		    pour dbra
.loop
    move.l  acf_Func(a3),a1	    a3 = CmdFunc *
    move.l  a4,-(sp)
    movem.l a2/a3/a5,-(sp)
    jsr	    (a1)    ( struct CmdObj *ao, struct	CmdFunc	*af, ULONG parm, struct	AZurProc *ap )
    add.w   #16,sp
    tst.b   aco_ResCode(a2)
    bne.b   .endaction
    add.w   #CmdFunc_SIZE,a3
.nexts
    dbra    d2,.loop

.endaction
    ;move.l  az_AZMast(a4),a0
    ;move.l  az_ObjSema(a0),a0
    ;jsr     _intAZ_SemaFreeRead

    movem.l (sp)+,d2/a1-a3/a5
    ;move.l  aco_RegGerm(a1),a0
    ;subq.l  #1,acg_SemaCount(a0)

    bclr    #AZAFB_EXECUTING-24,aco_Flags(a1)
.executing
    rts


*****************************************************************
*
*
*****************************************************************
do_sem
    ;move.l  az_AZMast(a4),a0
    ;move.l  az_ObjSema(a0),a0
    ;jsr     _intAZ_SemaWrite

.loop
    ;tst.l   acg_SemaCount(a1)
    ;beq.b   .jsr
    ;cmp.l   az_CurrentGerm(a4),a1   ; si même germe, on n'attend pas car SemaCount toujours > 0
    ;beq.b   .jsr
    ;move.l  a1,-(sp)		 ; germe
    ;moveq   #5,d1		 ; 1/10e sec
    ;move.l  az_DOSBase(a4),a6
    ;jsr     _LVODelay(a6)
    ;move.l  (sp)+,a1		 ; germe
    ;bra.b   .loop
.jsr
    jsr	    (a3)

    ;move.l  az_AZMast(a4),a0
    ;move.l  az_ObjSema(a0),a0
    ;jsr     _intAZ_SemaFreeWrite ; préserve d0
    rts


*****************************************************************
*   ULONG eng_RGNewAllActions( struct CmdGerm *rg );
*					       a1
*****************************************************************
_func_NewAllActions
    move.l  4(sp),a1

_eng_RGNewAllActions
    movem.l a2-a4/a6,-(sp)
    lea	    RGNewAllActions(pc),a3
    bsr	    do_sem
    movem.l (sp)+,a2-a4/a6
    rts

RGNewAllActions
    move.l  az_UIKBase(a4),a6
    move.l  acg_Actions(a1),d0
    beq.b   .new

    lea	    acg_Actions(a1),a3	    a3 = adresse du pointeur de	l'UIKBuffer
    move.l  (a3),a2		    a2 = pointer sur l'UIKBuffer
    move.l  -nubu_CurSize(a2),d2    total = BufLength( (UBYTE*)ao->Actions ) / 4;
    lsr.l   #2,d2		    d2 = nombre	d'actions
    subq    #1,d2		    pour dbra
.loop
    move.l  (a2)+,d0		    free chaque	fonction
    beq.b   .n
    move.l  d0,a1
    jsr	    _LVOBufFree(a6)	    BufFree( (UBYTE*)(ao->Actions[num])	);
.n  dbra    d2,.loop
    bra.b   .end
    ;------------
.new
    move.l  a1,a3
    moveq   #16,d1
    move.l  d1,a0		    a0=	blksize
    moveq   #AZA_NUMACTIONS+1,d0
    lsl.l   #2,d0		    d0 = size =	total =	num * 4
    move.l  #MEMF_ANY|MEMF_CLEAR,d1
    jsr	    _LVOBufAlloc(a6)	    BufAlloc(...);
    move.l  d0,acg_Actions(a3)
.end
    rts


*****************************************************************
*   ULONG eng_RGNewAction( struct CmdGerm *cg );
*					   a1
*****************************************************************
_func_NewAction
    move.l  4(sp),a1

_eng_RGNewAction
    movem.l d2/a2-a4/a6,-(sp)
    lea	    _eng_NewAction(pc),a3
    bsr	    do_sem
    movem.l (sp)+,d2/a2-a4/a6
    rts

_eng_NewAction
    move.l  az_UIKBase(a4),a6
    move.l  a1,a2		    a2 = CmdGerm

    move.l  acg_Actions(a2),a0
    move.l  -nubu_CurSize(a0),d2    newtype = BufLength( (UBYTE*)cg->Actions ) / 4;
    move.l  d2,d0
    lsr.l   #2,d2		    d2 = newtype

    lea	    acg_Actions(a2),a0	    BufResizeS(	&(UBYTE*)cg->Actions, (newtype+1) * 4 )
    addq.l  #4,d0
    jsr	    _LVOBufResizeS(a6)
    tst.l   d0
    bne.b   .end		    newtype supérieur à	0
    moveq   #0,d2		    ou 0
.end
    move.l  d2,d0
    rts


*****************************************************************
*   void eng_RGDeleteAction( struct CmdGerm *cg, UWORD type );
*					     a1	       d0
*****************************************************************
_func_DeleteAction
    move.l  4(sp),a1
    move.l  8(sp),d0

_eng_RGDeleteAction
    movem.l a2-a4/a6,-(sp)
    lea	    _eng_DeleteAction(pc),a3
    bsr	    do_sem
    movem.l (sp)+,a2-a4/a6
    rts

_eng_DeleteAction
    move.l  az_UIKBase(a4),a6
    move.l  acg_Actions(a1),a1
    lsl.w   #2,d0
    lea	    0(a1,d0.w),a1
    jsr	    _LVOBufFreeS(a6)	    BufFreeS( &(UBYTE*)(cg->Actions[type]) );
    rts


*****************************************************************
*   void eng_RGDeleteAllActions( struct	CmdGerm	*cg );
*						 a1
*****************************************************************
_func_DeleteAllActions
    move.l  4(sp),a1

_eng_RGDeleteAllActions
    movem.l d2/a2-a4/a6,-(sp)
    lea	    _eng_DeleteAllActions(pc),a3
    bsr	    do_sem
    movem.l (sp)+,d2/a2-a4/a6
    rts

_eng_DeleteAllActions
    move.l  az_UIKBase(a4),a6
    lea	    acg_Actions(a1),a3	    a3 = adresse du pointeur de	l'UIKBuffer
    move.l  (a3),a2		    a2 = pointer sur l'UIKBuffer
    move.l  a2,d0
    beq.b   .end

    move.l  -nubu_CurSize(a2),d2    total = BufLength( (UBYTE*)ao->Actions ) / 4;
    lsr.l   #2,d2		    d2 = nombre	d'actions
    subq    #1,d2		    pour dbra
.loop
    move.l  (a2)+,d0		    free chaque	fonction
    beq.b   .n
    move.l  d0,a1
    jsr	    _LVOBufFree(a6)	    BufFree( (UBYTE*)(ao->Actions[num])	);
.n  dbra    d2,.loop

    move.l  a3,a1		    free l'UIKBuffer des actions
    jsr	    _LVOBufFreeS(a6)	    BufFreeS( &(UBYTE*)(ao->Actions) );
.end
    move.l  d2,d0
    rts


*****************************************************************
*   ULONG eng_RGNewActionFunc( struct CmdGerm *cg, UWORD type, AZAFunc *af, ULONG end );
*					       a1	 d0		d1	  a0
*****************************************************************
_func_NewActionFunc
    move.l  4(sp),a1
    movem.l 8(sp),d0-d1
    move.l  16(sp),a0

_eng_RGNewActionFunc
    movem.l d2/a2-a4/a6,-(sp)
    move.l  a0,d2
    lea	    _eng_NewActionFunc(pc),a3
    bsr	    do_sem
    movem.l (sp)+,d2/a2-a4/a6
    rts

_eng_NewActionFunc
    move.l  az_UIKBase(a4),a6
    move.l  d1,a3		    a3 = AZAFunc à insérer

    move.l  acg_Actions(a1),a2	    a2 = cg->Actions
    lsl.w   #2,d0		    d0 = offset	de l'action = type *= 4
    lea	    0(a2,d0.w),a2	    a2 = struct	AZAction **base	= &cg->Actions[type]
    move.l  (a2),d0		    if (!*base)
    bne.b   .baseok

    move.l  #2*CmdFunc_SIZE,a0	    a0=	blksize
    moveq   #0,d0		    d0 = size =	total =	num * 4
    move.l  #MEMF_ANY|MEMF_CLEAR,d1
    jsr	    _LVOBufAlloc(a6)	    BufAlloc(...);

    tst.l   d0
    beq.b   .end
    move.l  d0,(a2)

.baseok
    move.l  d2,d1		    end	?
    beq.b   .deb
    move.l  d0,a0
    move.l  -nubu_CurSize(a0),d1    start
.deb
    move.l  a3,a0		    src
    moveq   #CmdFunc_SIZE,d0   len
    move.l  a2,a1		    pdest
    jsr	    _LVOBufPasteS(a6)
.end
    rts


*****************************************************************
*   void eng_RGDeleteActionFunc( struct	CmdGerm	*cg, UWORD type, void (*func)()	);
*						 a1	   d0		d1
*****************************************************************
;_func_DeleteActionFunc
;    move.l  4(sp),a1
;    movem.l 8(sp),d0-d1
;_eng_RGDeleteActionFunc
;    movem.l a2-a3,-(sp)
;    lea     _eng_DeleteActionFunc(pc),a3
;    bsr     do_sem
;    movem.l (sp)+,a2-a3
;    rts
;
;_eng_DeleteActionFunc
;    rts


*****************************************************************
*   ULONG eng_Tags2Struct( struct TagItem *tl, APTR rgobj );
*   d0					   a0	    a1
*****************************************************************

*****************************************************************
* This routine (GetUserTag) returns the	next Tag into a0 after
* processing the system	Tags.
* If the end of	TagList	is reached, the	Z flag is 1, else
* we are on a valid Tag. So a usual test after calling this
* routine can be: 'beq.b .end'

more	move.l	4(a2),a2	    adress of next tag is in ti_Data
	bra.b	_GetUserTag	     have it
skip	addq.w	#8,a2		    skip 2 tags	(this &	the next)
next	addq.w	#8,a2		    next tag
_GetUserTag
	move.l	(a2),d0		    d0 = tag
	beq.b	.end		    if (tag == TAG_DONE) return(0) Z flag set !
	subq.l	#1,d0
	beq.b	next		    if (tag == TAG_IGNORE) continue
	subq.l	#1,d0
	beq.b	more		    if (tag == TAG_MORE) goto more
	subq.l	#1,d0
	beq.b	skip		    if (tag == TAG_SKIP) goto skip
.ok
	move.l	a2,d0		    ok,	found a	valid Tag.
.end	rts			    Z == 1 ou 0	!!!

*****************************************************************
*
_eng_Tags2Struct
	movem.l	d2/a2-a3/a5,-(sp)
	moveq	#-1,d2		    version minimum trouvée
	sub.l	#CmdFunc_SIZE,sp
	move.l	a0,d0		    if (ATagList == 0) return(0)
	beq.b	.end
	move.l	d0,a2		    a2 = TagList
	move.l	a1,a3		    a3 = RegGerm
	move.l	sp,a5		    a5 = CmdFunc
	clr.l	4(a5)		    RAZ	CmdFunc
	bra.b	.loop

.next	addq.w	#8,a2
.loop
	cmp.l	#TAG_SKIP,(a2)
	bhi.b	.ok
	bsr.b	_GetUserTag	    next tag
	beq.b	.end		    if end of list goto	end
.ok
	pea	.next(pc)
	movem.l	(a2),d0-d1	    d0 = tag, d1 = value
	bclr	#29,d0		    teste AZT_ACTION
	bne.b	.TAG_Action
	bclr	#28,d0		    teste AZT_GEN
	bne.b	.TAG_Germe
	bclr	#27,d0		    teste AZT_OBJ
	bne.b	.TAG_Obj
	rts
.end
	move.l	d2,d0		    retourne version trouvée
.err	add.l	#CmdFunc_SIZE,sp
	movem.l	(sp)+,d2/a2-a3/a5
	rts

.TAG_Action	;----------	    d0 = type, d1 = value
	move.l	d1,(a5)		    acf_Func = ti_Data
	move.l	a3,a1		    a1 = reggerm
	move.l	a5,d1		    d1 = AZAFunc
	move.l	#.ret,(sp)
	bra	_eng_RGNewActionFunc
.ret	tst.l	d0
	bne.b	.next
	bra.b	.err

.TAG_Germe	;----------	    d1 = value
	lsl.w	#2,d0		    d0 = offset	= tag *	4
	move.l	germfunc_table(pc,d0.w),a0
	jmp	(a0)

.TAG_Obj	;----------	    d1 = value
	lsl.w	#2,d0		    d0 = offset	= tag *	4
	move.l	objfunc_table(pc,d0.w),a0
	jmp	(a0)

germfunc_table
	dc.l	.putname
	dc.l	.putsize
	dc.l	.putid
	dc.l	.putver
	dc.l	.putudata
	dc.l	.putargt
	dc.l	.putflags

.putname    move.l  d1,acg_Names(a3)
	    rts
.putsize    move.w  d1,acg_ObjSize(a3)
	    rts
.putid	    move.w  d1,acg_Id(a3)
	    rts
.putver	    move.l  d1,acg_Version(a3)
	    rts
.putudata   move.l  d1,acg_UserData(a3)
	    rts
.putflags   move.l  d1,acg_Flags(a3)
	    rts
.putargt	moveq	#0,d0		d0 = num items
		move.l	d1,a1		Template existe?
		move.l	d1,acg_CmdTemplate(a3)
		beq.b	.end
.pafilter	move.b	(a1)+,d1
		beq.b	.end
		cmp.b	#',',d1
		beq.b	.pafilter
		addq.l	#1,d0		num_item++
.paloop		move.b	(a1)+,d1	c = *Template++
		beq.b	.end
		cmp.b	#',',d1
		bne.b	.paloop
		bra.b	.pafilter
.end		move.b	d0,acg_Items(a3)
	    rts

objfunc_table
	dc.l	.objflags
	dc.l	.objudata
	dc.l	.objcmdline
	dc.l	.objextrabuf
	dc.l	.objexefrom
	dc.l	.objversion

.objflags   move.l  d1,aco_Flags(a3)
	    rts
.objudata   move.l  d1,aco_UserData(a3)
	    rts
.objcmdline move.l  d1,aco_PArg+arg_CmdLine(a3)
;	      move.l  d1,a0
;	      btst.b  #PAB_ACCEPTEOL,arg_Flags+3(a3)
;	      beq     .ocl2
;
; .ocl	      tst.b   (a0)+
;	      bne.b   .ocl
;	      subq.w  #1,a0
;	      bra.b   .ok
;
; .ocl2	      tst.b   (a0)
;	      beq.b   .ok
;	      cmp.b   #10,(a0)+
;	      bne.b   .ocl2
;
; .ok	      suba.l  d1,a0
;	     move.l  a0,aco_PArg+arg_CmdLength(a3)
	    move.l  #$7fffffff,aco_PArg+arg_CmdLength(a3)
	    rts
.objextrabuf move.l d1,aco_ExtraBuf(a3)
	    rts
.objexefrom move.b  d1,aco_ExeFrom(a3)
	    rts
.objversion move.l  d1,d2
	    rts


*****************************************************************
	END
