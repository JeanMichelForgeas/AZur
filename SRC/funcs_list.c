/****************************************************************
 *
 *      Project:   AZUR
 *      Function:  Gestion d'une liste simple
 *
 *      Created:   17/04/93 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objreqstring.h"
#include "uiki:objwinfilesel.h"
#include "uiki:objmemory.h"
#include "uiki:objstring.h"
#include "uiki:uikdrawauto.h"

#include "lci:aps.h"
#include "lci:aps_protos.h"
#include "lci:azur.h"
#include "lci:objfile.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:azur_protos.h"

struct _ReqList
    {
    struct AZRequest *AReq;

    struct UIKObjWindow *WO;
    struct UIKObjList   *List;
    struct UIKObj       *Prop;
    struct UIKObj       *Up;
    struct UIKObj       *Down;
    struct UIKObj       *Str;

    void    (*OKFunc)();
    void    (*CancelFunc)();
    ULONG   (*List2Str)();
    ULONG   FuncParm;
    ULONG   *Ptr2Null;
    ULONG   Flags;
    APTR    *PtrAR;
    UBYTE   *PtrButton;

    ULONG   SelSecs;
    ULONG   SelMicros;
    WORD    OldSel;
    };


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/

static UWORD __far PatProp[] = { 0xAAAA, 0x5555 };
static UWORD __far PropAuto[] = { x_ADD_1, x_ADD_1, X_SUB_1, X_SUB_1,
                                  y_ADD_1, Y_SUB_1,
                                  END_AUTO };

/****************************************************************
 *
 *      Utilities
 *
 ****************************************************************/

static void __regargs do_okfunc( struct _ReqList *rp, UBYTE *name )
{
    rp->AReq->Button = REQBUT_OK;
    if (rp->OKFunc) (*rp->OKFunc)( rp->FuncParm, name, rp->AReq ); // peut faire le remove de rp !!!
    if (rp->AReq->Obj == 0 && rp->PtrAR) *(rp->PtrAR) = 0;
    if (rp->PtrButton) *(rp->PtrButton) = REQBUT_OK;
}

static void __regargs do_cancelfunc( struct _ReqList *rp )
{
    rp->AReq->Button = REQBUT_CANCEL;
    if (rp->CancelFunc) (*rp->CancelFunc)( rp->FuncParm, rp->AReq ); // fait le remove de rp !!!
    if (rp->AReq->Obj == 0 && rp->PtrAR) *(rp->PtrAR) = 0;
    if (rp->PtrButton) *(rp->PtrButton) = REQBUT_CANCEL;
}

static void reqlist_closed( struct UIKObj *wo, struct _ReqList *rp )
{
    do_cancelfunc( rp );
}

//------------------------------------------------------------

static ULONG __regargs do_str_name( struct _ReqList *rp )
{
  UBYTE *ptr;

    if (rp->Str)
        {
        if (ptr = UIK_OSt_Get( rp->Str, 0 ))
            if (*ptr)
                {
                do_okfunc( rp, ptr );
                return(1);
                }
        }
    return(0);
}

static void __regargs do_list_name( struct _ReqList *rp, ULONG doubleclick )
{
  UBYTE *ptr=0;
  WORD sel;
  ULONG len;

    UIK_LockObjVars( rp->List );
    if ((sel = rp->List->Sel) != -1)
        {
        if (ptr = UIK_OLi_LineBuffer( rp->List , sel ))
            {
            len = BufLength( ptr );
            if (rp->List2Str) len = (*rp->List2Str)( rp->FuncParm, ptr, rp->AReq );
            if (rp->Str && len) UIK_OSt_Set( rp->Str, ptr, len );
            if (! doubleclick) ptr = 0;
            }
        }
    rp->OldSel = sel;
    UIK_UnlockObjVars( rp->List );

    if (ptr && *ptr) do_okfunc( rp, ptr );
}
/*
static ULONG __regargs do_str_name( struct _ReqList *rp )
{
  UBYTE *ptr, docname[256];

    if (rp->Str)
        {
        docname[0] = 0;
        if (ptr = UIK_OSt_Get( rp->Str, 0 ))
            StcCpy( docname, UIK_BaseName(ptr), 255 );
        if (docname[0])
            {
            do_okfunc( rp, docname );
            return(1);
            }
        }
    return(0);
}

static void __regargs do_list_name( struct _ReqList *rp, ULONG doubleclick )
{
  UBYTE *ptr, docname[256];
  WORD sel;
  ULONG len;

    docname[0] = 0;

    UIK_LockObjVars( rp->List );
    if ((sel = rp->List->Sel) != -1)
        {
        if (ptr = UIK_OLi_LineBuffer( rp->List , sel ))
            {
            len = BufLength( ptr );
            if (rp->List2Str) len = (*rp->List2Str)( rp->FuncParm, ptr );
            if (rp->Str && len) UIK_OSt_Set( rp->Str, ptr, len );
            if (doubleclick) StcCpy( docname, ptr, 255 );
            }
        }
    rp->OldSel = sel;
    UIK_UnlockObjVars( rp->List );

    if (docname[0]) do_okfunc( rp, docname );
}
*/

static void SelectRPList( struct UIKObjList *objlist, struct _ReqList *rp )
{
  struct IntuiMessage *imsg = objlist->Obj.UIK->IntuiMsg;
  ULONG doubleclick = 0;

    if (imsg)
        {
        doubleclick = DoubleClick( rp->SelSecs, rp->SelMicros, imsg->Seconds, imsg->Micros );
        rp->SelSecs = imsg->Seconds; rp->SelMicros = imsg->Micros;
        if (rp->List->Sel != rp->OldSel) doubleclick = 0;
        }
    else doubleclick = 1; // sélection par joystick

    do_list_name( rp, doubleclick );
}

static void SelectRPKeys( struct UIKObj *win, struct _ReqList *rp )
{
  struct IntuiMessage *imsg = win->UIK->IntuiMsg;
  struct UIKObjList *olist = rp->List;
  UWORD alt, shift;

    if (imsg->Code & 0x80) return;
    alt = imsg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT);
    shift = imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT);

    switch (imsg->Code)
        {
        case 0x43:                                              /* ent */
        case 0x44:                                              /* ret */
            do_list_name( rp, 1 );
            break;
        case 0x4c:                                              /* haut */
            if (alt) UIK_OLi_SetListTop( olist, 0 );
            else if (shift) UIK_OLi_MoveListFrame( olist, -1 );
            else UIK_OLi_MoveListSel( olist, -1 );
            break;
        case 0x4d:                                              /* bas */
            if (alt) UIK_OLi_SetListTop( olist, -1 );
            else if (shift) UIK_OLi_MoveListFrame( olist, 1 );
            else UIK_OLi_MoveListSel( olist, 1 );
            break;
        }
}

static void StringKeys( struct UIKObjStrGad *sto, struct _ReqList *rp )
{
    WORD code = sto->Obj.UIK->IntuiMsg->Code;
    if (code == 0x43 || code == 0x44) do_str_name( rp );
}

static void GadOK( struct UIKObj *gad, struct _ReqList *rp )
{
    if (! do_str_name( rp )) do_list_name( rp, 1 );
}

static void GadCancel( struct UIKObj *gad, struct _ReqList *rp )
{
    do_cancelfunc( rp );
}

ULONG func_ReqList( struct AZRequest *ar, ULONG tags, ... )
{
  struct UIKObjWindow *wo=0;
  struct UIKObj *box, *obj;
  struct _ReqList *rp;
  WORD strheight=0, left, top, width, height;
  struct Window *w;
  struct TagItem *taglist = (APTR)&tags;
  ULONG flags;
  UBYTE *selname, button=0;

    if (!AGP.WO || !(w = AGP.WO->Obj.w)) goto END_ERROR;

    flags = GetTagData( ART_ReqFlags, 0, taglist );
    if (flags & ARF_STRGAD) strheight = 4 + AGP.WO->Obj.ObjExt->Font->tf_YSize + 5;

    left = w->LeftEdge + (w->Width - 186) / 2;
    top = w->TopEdge + (w->Height - (126+strheight)) / 2;
    width = 186;
    height = 123 + strheight;
    if (ar)
        {
        ar->Obj = 0;
        ar->ErrMsg = 0;
        if (ar->Width && ar->Height) { left = ar->Left; top = ar->Top; width = ar->Width; height = ar->Height; }
        }

    if (! (wo = (struct UIKObjWindow *) UIK_AddObjectTagsB( UIKBase, "UIKObj_Window", AGP.WO,
            UIKTAG_OBJ_LeftTop,         W2L(left,top),
            UIKTAG_OBJ_WidthHeight,     W2L(width,height),
            UIKTAG_OBJ_Title,           GetTagData( ART_ReqTitle, 0, taglist ),
            UIKTAG_OBJ_TitleFl_Addr,    TRUE,
            UIKTAG_OBJWindow_MinimumWidthHeight, W2L(130,65+strheight),
            UIKTAG_OBJWindowFl_CloseStop,   FALSE,
            UIKTAG_OBJWindowFl_CloseRemove, FALSE,
            UIKTAG_OBJWindowFl_With_Icon,   FALSE,
            UIKTAG_OBJWindowFl_With_Zoom,   FALSE,
            TAG_END ))) goto END_ERROR;

    if (! (box = (struct UIKObjWindow *) UIK_AddObjectTagsB( UIKBase, "UIKObj_Box", wo,
            UIKTAG_OBJ_LeftTop,     W2L(0,0),
            UIKTAG_OBJ_WidthHeight, W2L(width,height),
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) goto END_ERROR;

    if (! (obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_Memory", wo,
            UIKTAG_OBJMem_MemSize,  sizeof(struct _ReqList) + sizeof(struct AZRequest),
            UIKTAG_OBJMem_MemType,  MEMF_CLEAR,
            TAG_END ))) goto END_ERROR;
    rp = (struct _ReqList *) UIK_OMe_MemPtr( obj );
    if (ar == 0) ar = (struct AZRequest *) (((ULONG)rp) + sizeof(struct _ReqList));

    ar->Flags |= flags;
    ar->Button = REQBUT_NOEVENT;

    rp->AReq       = ar;
    rp->WO         = wo;
    rp->OKFunc     = (APTR) GetTagData( ART_OKFunc,       0, taglist );
    rp->CancelFunc = (APTR) GetTagData( ART_CancelFunc,   0, taglist );
    rp->List2Str   = (APTR) GetTagData( ART_List2StrFunc, 0, taglist );
    rp->FuncParm   = GetTagData( ART_UserData, 0, taglist );

    if (! (rp->List = (struct UIKObjList *)UIK_AddObjectTagsB( UIKBase, "UIKObj_List", box,
            UIKTAG_OBJ_ParentRelative,      TRUE,
            UIKTAG_OBJ_LeftTop,             W2L(9,7),
            UIKTAG_OBJ_WidthHeight,         W2L(width-34,height-29-strheight),
            UIKTAG_OBJList_TextPtr,         GetTagData( ART_ListInit, (ULONG)"", taglist ),
            UIKTAG_OBJListFl_ClickSel,      TRUE,
            UIKTAG_OBJListFl_SelDispl,      TRUE,
            UIKTAG_OBJListFl_Sorted,        (flags & ARF_NOSORT) ? FALSE : TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) goto END_ERROR;
    wo->Obj.UserULong1 = (ULONG)rp->List;

    if (selname = (UBYTE*) GetTagData( ART_ListSelect, 0, taglist ))
        {
        struct UIKList *ul = &rp->List->ListText;
        WORD num=-1;
        while (++num < ul->NodeNum)
            {
            if (StrNCCmp( ul->UNode[num].Buf, selname ) == 0)
                {
                UIK_OLi_SelectList( rp->List, num );
                if ((num -= (rp->List->Vis / 2)) >= 0) UIK_OLi_SetListTop( rp->List, num );
                break;
                }
            }
        }

    if (! (rp->Up = UIK_AddObjectTagsB( UIKBase, "UIKObj_Up", box,
            UIKTAG_OBJ_ParentRelative,      TRUE,
            UIKTAG_OBJ_LeftTop,             W2L(width-25,7),
            UIKTAG_OBJ_WidthHeight,         W2L(16,12),
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            TAG_END ))) goto END_ERROR;

    if (! (rp->Prop = UIK_AddObjectTagsB( UIKBase, "UIKObj_PropLift", rp->List, // rp->List pour que le Stop agisse aussi sur le prop
            UIKTAG_OBJ_LeftTop,             W2L(width-27,7+rp->Up->Box.Height-1),
            UIKTAG_OBJ_WidthHeight,         W2L(20,2+rp->List->Obj.Box.Height-2*rp->Up->Box.Height),
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJProp_Vertical,        TRUE,
            UIKTAG_OBJ_Pattern,             (ULONG)PatProp,
            UIKTAG_OBJ_PatternPower,        1,
            UIKTAG_OBJ_BoxAutomate,         (ULONG) PropAuto,
            UIKTAG_OBJ_BoxAltAutomate,      (ULONG) PropAuto,
            UIKTAG_OBJ_BoxFl_Filled,        TRUE,
            TAG_END ))) goto END_ERROR;

    if (! (rp->Down = UIK_AddObjectTagsB( UIKBase, "UIKObj_Down", box,
            UIKTAG_OBJ_ParentRelative,      TRUE,
            UIKTAG_OBJ_LeftTop,             W2L(width-25,7+rp->List->Obj.Box.Height-rp->Up->Box.Height),
            UIKTAG_OBJ_WidthHeight,         W2L(16,12),
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) goto END_ERROR;

    if (ar->Flags & ARF_STRGAD)
        {
        if (! (rp->Str = (struct UIKObjStrGad *) UIK_AddObjectTagsB( UIKBase, "UIKObj_String", box,
            UIKTAG_OBJ_ParentRelative,      TRUE,
            UIKTAG_OBJ_LeftTop,             W2L(9,7+rp->List->Obj.Box.Height+3),
            UIKTAG_OBJ_WidthHeight,         W2L(rp->List->Obj.Box.Width+16,0),
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJStrFl_Activate,       TRUE,
            TAG_END ))) goto END_ERROR;
        if ( ! UIK_AddVector( rp->Str, (void *)StringKeys, RAWKEY, (ULONG)rp )) goto END_ERROR;
        }

    if (! (obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_OK", box,
            UIKTAG_OBJ_ParentRelative,      TRUE,
            UIKTAG_OBJ_LeftTop,             W2L(9,height-18),
            UIKTAG_OBJ_WidthHeight,         W2L(53,13),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) goto END_ERROR;
    if ( ! UIK_AddVector( obj, (void *)GadOK, GADGETUP, (ULONG)rp )) goto END_ERROR;

    if (! (obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_Cancel", box,
            UIKTAG_OBJ_ParentRelative,      TRUE,
            UIKTAG_OBJ_LeftTop,             W2L(width-9-53,height-18),
            UIKTAG_OBJ_WidthHeight,         W2L(53,13),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            TAG_END ))) goto END_ERROR;
    if ( ! UIK_AddVector( obj, (void *)GadCancel, GADGETUP, (ULONG)rp )) goto END_ERROR;

    /* On lie l'ascenceur a la liste */
    if (! UIK_Link( rp->Prop, rp->List, 0, MOUSEMOVE|GADGETDOWN, 0 )) goto END_ERROR;

    /* On lie les boutons Up et Down à l'ascenceur */
    if (! UIK_Link( rp->Up, rp->Prop, 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) goto END_ERROR;
    if (! UIK_Link( rp->Down, rp->Prop, 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) goto END_ERROR;

    if ( ! UIK_AddVector( wo, (void *)reqlist_closed, CLOSEWINDOW, (ULONG)rp )) goto END_ERROR;
    if ( ! UIK_AddVector( wo, (void*)SelectRPKeys, RAWKEY, (ULONG)rp )) goto END_ERROR;
    if ( ! UIK_AddVector( rp->List, (void *)SelectRPList, GADGETDOWN, (ULONG)rp )) goto END_ERROR;

    if (! UIK_Start( wo )) goto END_ERROR;

    ar->Obj = wo;
    ar->WO = wo;
    if (ar->Flags & ARF_ASYNC) return(0);

    rp->PtrAR = (APTR*)&ar;
    rp->PtrButton = &button;
    while (ar->Button == REQBUT_NOEVENT)
        {
        ULONG events = UIK_WaitFirstEvents( AGP.UIK, SIGBREAKF_CTRL_C );
        if (events & SIGBREAKF_CTRL_C) { Signal( FindTask(0), SIGBREAKF_CTRL_C ); goto END_ERROR; }
        if (! ar) goto END_REMOVED;
        if (! ar->Obj) break;
        }
    if (ar->Button == REQBUT_CANCEL) ar->ErrMsg = TEXT_CMDCANCELLED;
    return( ar->ErrMsg );

  END_REMOVED:
    return( (ULONG)((button == REQBUT_CANCEL) ? TEXT_CMDCANCELLED : 0) );

  END_ERROR:
    if (ar) { ar->Button = REQBUT_CANCEL; ar->ErrMsg = TEXT_CMDCANCELLED; }
    UIK_Remove( wo );
    return( TEXT_CMDCANCELLED );
}
