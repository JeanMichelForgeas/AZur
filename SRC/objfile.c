/****************************************************************
 *
 *      File:      objfile.c
 *      Project:   gestion d'un fichier
 *
 *      Created:   12-11-92     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <graphics/gfxbase.h>
#include <workbench/workbench.h>
#include <clib/wb_protos.h>
#include <pragmas/wb_pragmas.h>

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikdrawauto.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objscreen.h"
#include "uiki:objwindow.h"
#include "uiki:objstring.h"
#include "uiki:objfileselector.h"
#include "uiki:objtimer.h"
#include "uiki:objwinfilesel.h"
#include "uiki:objreqstring.h"

#include "lci:objmenu.h"
#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"

#define STATUSBAR_HEIGHTOFFS  4
#define CMDLINE_HEIGHTOFFS    3


/****** Imported ************************************************/

extern void cm_CmdFromMenu();
extern void RawKeys( struct AZObjFile *fo, struct KeyHandle *kh );

extern struct Glob { ULONG toto; } __near GLOB;

extern struct UIKObjRegGerm __far ObjStatRegGerm;
extern struct UIKObjRegGerm __far ObjViewRegGerm;
extern struct UIKObjRegGerm __far ObjMenuRegGerm;


/****** Exported ***********************************************/

void ofi_SetActiveView( struct AZObjFile *fo, struct AZObjView *view );
void ofi_TickUpdateDisplay( ULONG notarrex, struct AZObjFile *fo );


/****** Statics ************************************************/

static void BlinkCursor( int nil, struct AZObjFile *fo );
static void MenuCancel( struct UIKObjMenu *mo, struct AZObjFile *fo );
static void Key2CmdLine( struct UIKObjStrGad *ostr, struct AZObjFile *fo );
static void CmdLine_InactHandler( struct UIKObjStrGad *ostr );

static struct TagItem __far ObjFile_BoolMap[] =
    {
    { AZTAG_FileFl_Iconify, AZFILEF_ICONIFY },
    { TAG_DONE }
    };

static UWORD __far CmdLineAuto[] = {
    COL_BLACK, MOVE_Xy, /*DRAW_Xy,*/ DRAW_XY, DRAW_xY,
    /*y_ADD_1,*/ X_SUB_1, Y_SUB_1,
    COL_GREY, MOVE_xy, DRAW_Xy, DRAW_XY, DRAW_xY, DRAW_xy,
    x_ADD_1, y_ADD_1, X_SUB_1, Y_SUB_1,
    END_AUTO
    };
static UWORD __far CmdLineAltAuto[] = {
    COL_BLACK, MOVE_Xy, /*DRAW_Xy,*/ DRAW_XY, DRAW_xY,
    /*y_ADD_1,*/ X_SUB_1, Y_SUB_1,
    COL_WHITE, MOVE_xy, DRAW_Xy, DRAW_XY, DRAW_xY, DRAW_xy,
    x_ADD_1, y_ADD_1, X_SUB_1, Y_SUB_1,
    END_AUTO
    };


/****************************************************************
 *
 *      Views handling
 *
 ****************************************************************/

static void linkviews( struct AZObjFile *fo )
{
  struct Node *node;
  struct AZObjView *prev;


    for (prev=0, node=fo->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
        {
        if (prev) prev->NextView = (APTR)node->ln_Name;
        ((struct AZObjView *)node->ln_Name)->PrevView = (APTR)prev; prev = (APTR)node->ln_Name;
        ((struct AZObjView *)node->ln_Name)->NextView = 0;
        }
}

void ofi_RemoveAllViewNodes( struct List *viewlist )
{
  struct Node *node, *succ;

    // Ne pas faire de Remove() des nodes, car cela irait modifier
    // fo->ViewList
    for (node=viewlist->lh_Head; node->ln_Succ; )
        {
        succ = node->ln_Succ;
        UIK_Remove( (struct UIKObj *)node->ln_Name );
        FreeMem( node, sizeof(struct Node) );
        node = succ;
        }
}

static void RemoveViewNode( struct AZObjFile *fo, struct UIKObj *view )
{
  struct Node *node;

    for (node=fo->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
        {
        if (node->ln_Name == (APTR) view)
            {
            if ((APTR)fo->ActiveView == (APTR)view)
                {
                func_CursorErase();
                }
            Remove( node );
            linkviews( fo );
            UIK_Remove( view );
            FreeMem( node, sizeof(struct Node) );
            if ((APTR)fo->ActiveView == (APTR)view)
                {
                ofi_SetActiveView( fo, (APTR) fo->ViewList.lh_Head->ln_Name );
                func_CursorDraw();
                }
            break;
            }
        }
}

static struct AZObjView *AddViewNode( struct AZObjFile *fo, struct TagItem *tl )
{
  struct Node *node;

/*
kprintf( "List=%lx\n", &fo->ViewList );
kprintf( "lh_Head    =%lx\n", fo->ViewList.lh_Head );
kprintf( "lh_Tail    =%lx\n", fo->ViewList.lh_Tail );
kprintf( "lh_TailPred=%lx\n", fo->ViewList.lh_TailPred );
kprintf( "lh_Type    =%lx\n", fo->ViewList.lh_Type );
kprintf( "l_pad      =%lx\n", fo->ViewList.l_pad );
*/
    if (node = (APTR) AllocMem( sizeof(struct Node), MEMF_PUBLIC | MEMF_CLEAR ))
        {
/*
kprintf( "Node=%lx\n", node );
kprintf( "ln_Succ=%lx\n", node->ln_Succ );
kprintf( "ln_Pred=%lx\n", node->ln_Pred );
*/
        if (node->ln_Name = (APTR) UIK_AddPrivateObject( &ObjViewRegGerm, fo->WO, tl ))
            {
            if (fo->ActiveView)
                {
                func_CursorErase();
                ((struct AZObjView *)node->ln_Name)->Zone->TopLine = fo->ActiveView->Zone->TopLine;
                }
            AddTail( &fo->ViewList, node );
/*
kprintf( "  Node=%lx\n", node );
kprintf( "  ln_Succ=%lx\n", node->ln_Succ );
kprintf( "  ln_Pred=%lx\n", node->ln_Pred );
kprintf( "  List=%lx\n", &fo->ViewList );
kprintf( "  lh_Head    =%lx\n", fo->ViewList.lh_Head );
kprintf( "  lh_Tail    =%lx\n", fo->ViewList.lh_Tail );
kprintf( "  lh_TailPred=%lx\n", fo->ViewList.lh_TailPred );
kprintf( "  lh_Type    =%lx\n", fo->ViewList.lh_Type );
kprintf( "  l_pad      =%lx\n", fo->ViewList.l_pad );
*/
            ofi_SetActiveView( fo, (struct AZObjView *)node->ln_Name );
            linkviews( fo );
            return( (struct AZObjView *)node->ln_Name );
            }
        FreeMem( node, sizeof(struct Node) );
        }
    return(0);
}

//=================================================================

static void get_aspect( struct UIKObj *fo, ULONG ModeId )
{
    fo->XAspect = 0;  /* So we can tell when we've got it */
    if (UIK_IsSystemVersion2())
        {
        struct DisplayInfo DI;
        if (GetDisplayInfoData( NULL, (UBYTE *) &DI, sizeof(struct DisplayInfo), DTAG_DISP, ModeId ))
            {
            fo->XAspect = DI.Resolution.x;
            fo->YAspect = DI.Resolution.y;
            }
        }
    if (! fo->XAspect) /* If running under 1.3 or GetDisplayInfoData failed, use old method of guessing aspect ratio */
        {
        fo->XAspect = 44;
        fo->YAspect = GfxBase->DisplayFlags & PAL ? 44 : 52;
        if (ModeId & HIRES) fo->XAspect = fo->XAspect >> 1;
        if (ModeId & LACE)  fo->YAspect = fo->YAspect >> 1;
        }
}

static void auto_size( struct AZObjFile *fo, struct AZViewInfo *vi )
{
  LONG ratio, xinf, yinf, larg=vi->VBorderWidth, xa=fo->Obj.XAspect, ya=fo->Obj.YAspect;

    xinf = 11;
    yinf = 9;
    ratio = ya / xa;
    if (ratio >= 2)     // ratio 2 : largeur accrue de l'automate + 2
        { larg += 4; }
    else if (UIKBase->UIKPrefs->PrefsFlags & UIKF_SPLOOK) /* ratio 1 ou moins */
        { larg++; xinf++; yinf++; }

    if (vi->VBorderWidth < xinf) vi->VBorderWidth = xinf;
    if ((vi->HBorderHeight = (((vi->VBorderWidth*xa*100)/ya)+50)/100) < yinf) vi->HBorderHeight = yinf;
    vi->VBorderWidth = larg;

    if (ratio >= 2) // ratio 2 : largeur accrue de l'automate + 2
        {
        if (! (vi->VBorderWidth & 1)) vi->VBorderWidth++;
        if (! (vi->HBorderHeight & 1)) vi->HBorderHeight++;
        }
    else{             // ratio 1 : si look spécial + 1 trait gris
        if (UIKBase->UIKPrefs->PrefsFlags & UIKF_SPLOOK)
            {
            if (vi->VBorderWidth & 1) vi->VBorderWidth++;
            if (vi->HBorderHeight & 1) vi->HBorderHeight++;
            }
        else{
            if (! (vi->VBorderWidth & 1)) vi->VBorderWidth++;
            if (! (vi->HBorderHeight & 1)) vi->HBorderHeight++;
            }
        }

    vi->VArrHeight = vi->HBorderHeight * 2;
    vi->VButHeight = vi->HBorderHeight;
    vi->HArrWidth  = vi->VBorderWidth  * 2;
    vi->HButWidth  = vi->VBorderWidth;
}

static struct UIKObjWindow *IsUIKWindow( struct Window *w )
{
  struct UIKObj *uik, *obj = (struct UIKObj *)w->UserData;

    if (obj && !(((ULONG)obj) & 1))
        {
        if (TypeOfMem(obj) && (uik = obj->UIK))
            {
            if (! (((ULONG)uik) & 1))
                {
                if (TypeOfMem(uik) && uik == uik->UIK) // alors objet valide
                    {
                    return( (struct UIKObjWindow *)obj );
                    }
                }
            }
        }
    return(0);
}

struct Screen *screen_exists( struct Screen *isc )
{
  ULONG ilock;
  struct Screen *sc;

    ilock = LockIBase( 0 );
    sc = IntuitionBase->FirstScreen;
    while (sc)
        {
        if (isc == sc) break;
        sc = sc->NextScreen;
        }
    UnlockIBase( ilock );
    return( sc );
}

static void activate_plain_window( struct Screen *sc )
{
#define WINTABMAX 20

  struct UIKObjWindow *ow;
  ULONG i, j, ilock;
  struct Window *w, *wintab[WINTABMAX], *bdrop=0;
  struct Layer *l;

    if (sc && screen_exists( sc ))
        {
        Delay(1);

        Forbid();
        ilock = LockIBase( 0 );
        for (w=sc->FirstWindow, i=0; w && i < WINTABMAX; w=w->NextWindow)
            {
            if (ow = IsUIKWindow( w ))
                {
                if (ow->Obj.Status != UIKS_ICONIFIED && ow->UniconW == 0)
                    {
                    if (w->Flags & WFLG_BACKDROP) bdrop = w;
                    else{
                        wintab[i++] = w;
                        }
                    }
                }
            }
        UnlockIBase( ilock );

        if (i == 0) w = bdrop; else w = wintab[0];

        if (i > 1)
            {
            for (l=sc->LayerInfo.top_layer; l; l=l->back)
                {
                for (j=0; j < i; j++)
                    {
                    if (l->Window == wintab[j])
                        {
                        w = wintab[j]; goto BREAK_ALL; }
                    }
                }
            BREAK_ALL:
            }

        if (w) ActivateWindow( w );
        Permit();
        }
}

/****************************************************************
 *
 *      Methodes publiques
 *
 ****************************************************************/

void ofi_SetColorMask( void )
{
  struct AZurPrefs *p = AGP.Prefs;
  UBYTE i, colmask = p->TextPen | p->BackPen | p->BlockTPen | p->BlockBPen | p->CursTextPen | p->CursBackPen | p->CursBlockTPen | p->CursBlockBPen;

    for (i=0; i < NUMFOLDINFOS; i++)
        colmask |= (p->FDI[i].OnAPen | p->FDI[i].OnBPen | p->FDI[i].OffAPen | p->FDI[i].OffBPen);
    AGP.FObj->Mask = func_DoColorMask( AGP.WO, colmask );
}

void ofi_VerifViewSize( struct AZViewInfo *vi, ULONG fheight, ULONG borderswidth )
{
  UWORD ZoneMinHeight, ZoneMinWidth;
  UWORD ViewMinHeight, ViewMinWidth;
  ULONG elemask = AGP.Prefs->EleMask;

    ZoneMinHeight = 3;  // Zone : 1 en haut + 2 en bas
    //if (elemask & BUMF_STATUSBAR) ZoneMinHeight += fheight + 4; // Stat : 2 en haut + 2 en bas
    if (elemask & BUMF_BOTTOMBORDER) ZoneMinHeight += vi->HBorderHeight;

    ZoneMinWidth = 15;
    if (elemask & BUMF_RIGHTBORDER) ZoneMinWidth += vi->VBorderWidth;

    ViewMinHeight = 1;  // 1 en bas
    if (elemask & BUMF_BUTUP)    ViewMinHeight += vi->VButHeight;
    if (elemask & BUMF_UPDOWN1)  ViewMinHeight += vi->VArrHeight;
    if (elemask & BUMF_VERTPROP) ViewMinHeight += VERTKNOB_MINHEIGHT;
    if (elemask & BUMF_UPDOWN2)  ViewMinHeight += vi->VArrHeight;
    if (elemask & BUMF_BUTDOWN)  ViewMinHeight += vi->VButHeight;
    if (elemask & (BUMF_BOTTOMBORDER|BUMF_RIGHTBORDER)) ViewMinHeight += vi->HBorderHeight;

    ViewMinWidth = 0;
    if (elemask & BUMF_BUTLEFT)    ViewMinWidth += vi->HButWidth;
    if (elemask & BUMF_LEFTRIGHT1) ViewMinWidth += vi->HArrWidth;
    if (elemask & BUMF_HORIZPROP)  ViewMinWidth += HORIZKNOB_MINWIDTH;
    if (elemask & BUMF_LEFTRIGHT2) ViewMinWidth += vi->HArrWidth;
    if (elemask & BUMF_BUTRIGHT)   ViewMinWidth += vi->HButWidth;
    if (elemask & (BUMF_BOTTOMBORDER|BUMF_RIGHTBORDER)) ViewMinWidth += vi->VBorderWidth;

    vi->MinHeight = MAX(ZoneMinHeight,ViewMinHeight);
    vi->MinWidth = MAX(ZoneMinWidth,ViewMinWidth);
    vi->MinWidth = MAX((97-borderswidth),vi->MinWidth);

    if (vi->ViewBox.Height < vi->MinHeight) vi->ViewBox.Height = vi->MinHeight;
    if (vi->ViewBox.Width < vi->MinWidth) vi->ViewBox.Width = vi->MinWidth;
}

void ofi_LinkViews( struct AZObjFile *fo )
{
    linkviews( fo );
}

struct AZObjView *ofi_AddViewObj( struct AZObjFile *fo, struct TagItem *tl )
{
    return( (struct AZObjView *) UIK_AddPrivateObject( &ObjViewRegGerm, fo->WO, tl ) );
}

struct AZObjView *ofi_AddView( struct AZObjFile *fo, struct AZViewInfo *vi )
{
  struct Node *node;
  WORD l, t, w, h, vh;
  struct TagItem tl[] = { { UIKTAG_OBJ_LeftTop,     },
                          { UIKTAG_OBJ_WidthHeight, },
                          { TAG_END },
                        };
    l = vi->ViewBox.Left;
    t = vi->ViewBox.Top;
    w = vi->ViewBox.Width;
    h = vi->ViewBox.Height;
    for (node = AGP.FObj->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
        {
        vh = ((struct UIKObj *)node->ln_Name)->Box.Height;
        t += vh;
        h -= vh;
        }
    tl[0].ti_Data = W2L(l,t);
    tl[1].ti_Data = W2L(w,h);
    return( AddViewNode( fo, tl ) );
}

void ofi_RemView( struct AZObjFile *fo, struct UIKObj *view )
{
    RemoveViewNode( fo, view );
}

void ofi_ActualizeViewLimits( struct AZObjFile *fo )
{
  struct Node *node;
  ULONG pos=-1, wminheight=fo->WO->TopBorderHeight+fo->WO->BottomBorderHeight;
  struct UIKObj *v;

    if (fo->Stat) wminheight += fo->Stat->Box.Height;
    if (fo->CmdLine) wminheight += fo->CmdLine->Box.Height;

    for (node = fo->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
        {
        if ((v = (struct UIKObj *)node->ln_Name) == fo->ActiveView)
            {
            oview_SetResizeFlags( v, 0 );
            pos = 1;
            wminheight += fo->VI.MinHeight;
            }
        else{
            oview_SetResizeFlags( v, pos );
            wminheight += v->Box.Height;
            }
        }

    fo->WO->NW.MinHeight = wminheight;
    if (fo->WO->Obj.w) WindowLimits( fo->WO->Obj.w, 0, wminheight, 0, 0 );
}

void ofi_SetActiveView( struct AZObjFile *fo, struct AZObjView *view )
{
    if (view && view != fo->ActiveView)
        {
        fo->ActiveView = view;
        ofi_ActualizeViewLimits( fo );
        UPDATE_STAT_PROP(fo);
        }
}

struct AZObjView *ofi_WhichViewPointer( struct AZObjFile *fo )
{
  struct AZObjView *vo;
  struct Rectangle *r;
  struct Node *node;

    for (node = fo->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
        {
        vo = (struct AZObjView *)node->ln_Name;
        r = &vo->Zone->Obj.ObjExt->InnerRect;
        if (fo->MouseX >= r->MinX && fo->MouseX <= r->MaxX && fo->MouseY >= r->MinY && fo->MouseY <= r->MaxY)
            return( vo );
        }
    return(0);
}

struct UIKObjMenu *ofi_AddMenuObj()
{
  struct AZObjFile *fo = AGP.FObj;
  struct UIKObjMenu *obj=0;

    if (fo->WO)
        {
        if (fo->Menu == 0 && intAZ_LineBufLen( (UBYTE*)AGP.AZMast->MenuHandle->MenuEntry ) > sizeof(struct AZMenuEntry))
            {
            if (obj = (struct AZObjMenu *) UIK_AddPrivateObjectTagsB( UIKBase, &ObjMenuRegGerm, fo->WO,
                UIKTAG_Menu_Entry,              AGP.AZMast->MenuHandle->MenuEntry,
                TAG_END ))
                {
                if (UIK_AddVector( obj, (void*)MenuCancel, IDCMP_MENUVERIFY, (ULONG)fo ))
                    {
                    if (fo->WO->Obj.w)
                        {
                        if (UIK_Start( obj ))
                            return( obj );
                        }
                    else return( obj );
                    }
                UIK_Remove( obj );
                obj = 0;
                }
            }
        }
    return( obj );
}

void ofi_FS_OKFunc( struct UIKObjWinFileSel *wfs, UBYTE *filename )
{
    StrCpy( AGP.FileSelResult, filename );
    UIK_Stop( wfs );
    wfs->Obj.UserULong1 = REQBUT_OK;
    //(AGP.FObj->OpenFS_OKVector)( wfs, filename );
}
void ofi_FS_CancelFunc( int nil, struct UIKObjWinFileSel *wfs )
{
    UIK_Stop( wfs );
    wfs->Obj.UserULong1 = REQBUT_CANCEL;
    //(AGP.FObj->OpenFS_CancelVector)( 0, wfs );
}

ULONG ofi_AddFileSelectors( ULONG pat1, ULONG pat2 )
{
  struct AZObjFile *fo = AGP.FObj;

    if (! (fo->OpenFS = (struct UIKObjWinFileSel *) UIK_AddObjectTagsB( UIKBase, "UIKObj_WinFileSel", fo->WO,
            UIKTAG_OBJ_Title,               "",
            UIKTAG_OBJ_TitleFl_Addr,        TRUE,
            UIKTAG_OBJ_FontName,            AGP.Prefs->ViewFontName,
            UIKTAG_OBJ_FontHeight,          AGP.Prefs->ViewFontHeight,
            UIKTAG_WFS_OKFunc,              ofi_FS_OKFunc,
            UIKTAG_WFS_CancelFunc,          ofi_FS_CancelFunc,
            UIKTAG_WFS_InitPath,            fo->OpenFileName,
            UIKTAG_WFS_Pattern,             pat1, // pattern passée dans ProcessMain()
            UIKTAG_WFSFl_ReturnReturn,      TRUE,
            UIKTAG_WFSFl_OK_IfNotExists,    (TST_CREATEFILE) ? TRUE : FALSE,
            UIKTAG_WFSFl_LeavePath,         FALSE,
            TAG_END ))) goto END_ERROR;
    if (! (fo->SaveFS = (struct UIKObjWinFileSel *) UIK_AddObjectTagsB( UIKBase, "UIKObj_WinFileSel", fo->WO,
            UIKTAG_OBJ_Title,               "",
            UIKTAG_OBJ_TitleFl_Addr,        TRUE,
            UIKTAG_OBJ_FontName,            AGP.Prefs->ViewFontName,
            UIKTAG_OBJ_FontHeight,          AGP.Prefs->ViewFontHeight,
            UIKTAG_WFS_OKFunc,              ofi_FS_OKFunc,
            UIKTAG_WFS_CancelFunc,          ofi_FS_CancelFunc,
            UIKTAG_WFS_InitPath,            fo->SaveFileName,
            UIKTAG_WFS_Pattern,             pat2, // pattern passée dans ProcessMain()
            UIKTAG_WFSFl_ReturnReturn,      TRUE,
            UIKTAG_WFSFl_OK_IfNotExists,    TRUE,
            UIKTAG_WFSFl_LeavePath,         FALSE,
            TAG_END ))) goto END_ERROR;
    return(1);

  END_ERROR:
    return(0);
}

//==========================================================================

void set_intui_vars()
{
  struct IntuiMessage *imsg = AGP.UIK->IntuiMsg;
  struct AZObjFile *fo = AGP.FObj;

    if (imsg)
        {
#ifdef AZUR_DEMO
    if (fo->Text.NodeNum > AZURDEMO_MAXLINES+8)
        ExecBase->ThisTask->tc_SigRecvd |= (0x00030000 | SIGBREAKF_CTRL_C); /* pour valeur pas reconnaissable */
#endif
        /*if (fo->MouseX != imsg->MouseX)*/ { fo->OldMouseX = fo->MouseX; fo->MouseX = imsg->MouseX; }
        /*if (fo->MouseY != imsg->MouseY)*/ { fo->OldMouseY = fo->MouseY; fo->MouseY = imsg->MouseY; }
        fo->Seconds = imsg->Seconds; fo->Micros = imsg->Micros;
        if (imsg->Class != IDCMP_INTUITICKS || UIK_IsSystemVersion2()) fo->SysQuals = imsg->Qualifier;
        fo->MemButs &= imsg->Qualifier;
        }
}

//--------------------------------------
static void CtrlD_QuitVerify( struct AZObjFile *fo );

void ofi_QuitVerify( int nil, struct AZObjFile *fo )
{
  UBYTE buf[250];
  ULONG code, errmsg;

    if (fo->Flags & AZFILEF_MODIFIED && fo->WO)
        {
        sprintf( buf, "%ls%ls", UIK_LangString( fo, TEXT_MODIFIED ), UIK_LangString( fo, TEXT_QUITVERIFY ) );
        code = func_IntSmartRequest( 0, 0,0, 0,buf, 0,0,0,0, TEXT_CTLD_Save,TEXT_CTLD_SaveQuit,TEXT_CTLD_Cancel, 0, 0, 0, 0, 0 );
        switch (code)
            {
            case REQBUT_CANCEL:
                fo->CtrlDVect = CtrlD_QuitVerify;
                break;
            case REQBUT_MIDDLE:
                errmsg = func_SaveFile( AGP.FObj->OpenFileName, 0, 0, 0, 0, 0 );
                if (errmsg)
                    {
                    func_ReqShowText( UIK_LangString( AGP.UIK, errmsg ) );
                    break;
                    }
                else CLR_MODIFIED();
            case REQBUT_OK:
                Signal( FindTask(0), SIGBREAKF_CTRL_C );
                fo->Flags &= ~AZFILEF_MODIFIED; // empêche le autosave sur ctrl+c dans ce cas-là
                break;
            }
        }
    else Signal( FindTask(0), SIGBREAKF_CTRL_C );
}

static void CtrlD_QuitVerify( struct AZObjFile *fo )
{
    ofi_QuitVerify( 0, fo );
}

//--------------------------------------
static void BlinkCursor( int nil, struct AZObjFile *fo )
{
    if (fo->WO->Obj.w->FirstRequest) return;

    if (TST_BLINKON && fo->BlinkCount)
        {
        if (--fo->BlinkCount == 0)
            {
            if (fo->Flags & AZFILEF_CURSVISIBLE)
                {
                fo->BlinkCount = AGP.Prefs->BlinkOffDelay;
                fo->Flags &= ~AZFILEF_CURSVISIBLE;
                }
            else{
                fo->BlinkCount = AGP.Prefs->BlinkOnDelay;
                fo->Flags |= AZFILEF_CURSVISIBLE;
                }
            func_DisplayCurs( fo->Line, fo->Col );
            }
        }
    else if (! (fo->Flags & AZFILEF_CURSVISIBLE))  // si arrêt du clignotement dans les prefs
        {
        SETCURSOR_VISIBLE(fo);
        func_DisplayCurs( fo->Line, fo->Col );
        }
#ifdef AZUR_DEMO
    if (fo->Text.NodeNum > AZURDEMO_MAXLINES+9)
        ExecBase->ThisTask->tc_SigRecvd |= (0x00020000 | SIGBREAKF_CTRL_C); /* pour valeur pas reconnaissable */
#endif
}

//--------------------------------------
// Ne doit pas modifier AGP.RexxResult
// car est appelée depuis le gestionnaire ARexx

static __far demo_count=0;

void ofi_TickUpdateDisplay( ULONG notarexx, struct AZObjFile *fo )
{
#ifdef AZUR_DEMO
    if (++demo_count > 50) // 5 secondes
        {
        ULONG secs = AGP.Timer->TotalTime.tv_secs * 5;
        demo_count = 0;
        if (secs / 7 > ((AZURDEMO_MAXTIME*5)/7))  /* pour valeur pas reconnaissable */
            ExecBase->ThisTask->tc_SigRecvd |= (0x00010000 | SIGBREAKF_CTRL_C); /* pour valeur pas reconnaissable */
        }
#endif

    if (fo->WO && fo->WO->Obj.w && fo->WO->Obj.w->FirstRequest) return;

    if (notarexx && fo->TickCount++ < 1) return;
    fo->TickCount = 0;

    if (fo->Flags & AZFILEF_DRAWSTATSALL)
        {
        func_DisplayStats(1);
        fo->Flags &= ~AZFILEF_DRAWSTATSALL;
        }
    else if (fo->Flags & AZFILEF_DRAWSTATS)
        {
        func_DisplayStats(0);
        fo->Flags &= ~AZFILEF_DRAWSTATS;
        }
    if (fo->Flags & AZFILEF_DRAWPROPS)
        {
        func_ResetProps();
        fo->Flags &= ~AZFILEF_DRAWPROPS;
        }
    if (fo->Flags & AZFILEF_DRAWREQBM)
        {
        BM_update_palette();
        fo->Flags &= ~AZFILEF_DRAWREQBM;
        }
    if (fo->Flags & AZFILEF_DOAUTOSAVE)
        {
        AutosaveMod();
        fo->Flags &= ~AZFILEF_DOAUTOSAVE;
        }

#ifdef AZUR_DEMO
    if (demo_count == 50) // 5 secondes
        {
        if (fo->Text.NodeNum > AZURDEMO_MAXLINES+10)
            ExecBase->ThisTask->tc_SigRecvd |= (0x00010000 | SIGBREAKF_CTRL_C); /* pour valeur pas reconnaissable */
        }
#endif
}

//--------------------------------------
static void Inact_SetCursor( int nil, struct AZObjFile *fo )
{
    SETCURSOR_VISIBLE(fo);
    func_DisplayCurs( fo->Line, fo->Col );
    ozone_GhostCurs( fo->ActiveView->Zone );
    UIK_InactivateKeyTank( fo );
    UIK_InactivateKeyTank( fo->CmdLine );
    func_RemoveNullPointer();
}

//--------------------------------------
static void NewWindowSize( struct UIKObjWindow *wo, struct AZObjFile *fo )
{
    fo->VI.ViewBox.Width = wo->Obj.Box.Width - (wo->LeftBorderWidth + wo->RightBorderWidth);
    fo->VI.ViewBox.Height = wo->Obj.Box.Height - (wo->TopBorderHeight + wo->BottomBorderHeight);
    if (fo->Stat) fo->VI.ViewBox.Height -= fo->Stat->Box.Height;
    if (fo->CmdLine) fo->VI.ViewBox.Height -= fo->CmdLine->Box.Height;
}

//--------------------------------------
void ofi_ActivateFObjKeyTank()
{
  struct UIKObj *wo = AGP.FObj->WO;

    if (wo)
        {
        if (wo->Status == UIKS_ICONIFIED)
            {
            UIK_ActivateKeyTank( AGP.FObj );
            }
        else{
            if (wo->w == IntuitionBase->ActiveWindow)
                if (! wo->w->FirstRequest)
                    UIK_ActivateKeyTank( AGP.FObj );
            }
        }
}

static void WindowKeyTank( struct UIKObj *wo, struct AZObjFile *fo )
{
  APTR active = UIK_IsActiveKeyTank( wo );

    if (active == 0 || active != fo->CmdLine) ofi_ActivateFObjKeyTank();
    func_RemoveNullPointer();
}

void PatchLastDoc( struct UIKObj *wo, struct AZObjFile *fo )
{
    AGP.AZMast->LastActNode = (struct AZurNode *) func_PLTaskNode( AGP.Process );
    /* mis sur le patch de startafter
    AGP.Process->pr_WindowPtr = fo->WO->Obj.w;
    */
    func_DisplayCurs( fo->Line, fo->Col );
}

//-------------------------------------- évènements souris

static void DoMouseClick( struct AZObjFile *fo, ULONG event, UWORD qual )
{
  struct AZObjView *vo;

    if (vo = ofi_WhichViewPointer( fo ))
        {
        //------ Cette view est active
        if (vo != fo->ActiveView)
            {
            func_CursorErase();
            ofi_SetActiveView( fo, vo );
            }

        //------ La fenêtre devient KeyTank
        ofi_ActivateFObjKeyTank();

        fo->MemButs |= qual;
        FromMouse( event );
        }
}

static void WindowButtons( struct UIKObj *wo, struct AZObjFile *fo )
{
  UBYTE code = AGP.UIK->IntuiMsg->Code;
  ULONG event = 0;
  UWORD qual = 0;

    set_intui_vars();
    if (TST_INPUTLOCK) return;

    switch (code /*& (SELECTDOWN+SELECTUP+MIDDLEDOWN+MIDDLEUP+MENUDOWN+MENUUP)*/)
        {
        case SELECTDOWN:
            event = IECODE_LBUTTON; qual = IEQUALIFIER_LEFTBUTTON;
            // fo->MemButs |= IEQUALIFIER_LEFTBUTTON;   fait dans DoMouseClick()
            break;
        case SELECTUP:
            fo->MemButs &= ~IEQUALIFIER_LEFTBUTTON;
            break;

        case MIDDLEDOWN:
            event = IECODE_MBUTTON; qual = IEQUALIFIER_MIDBUTTON;
            // fo->MemButs |= IEQUALIFIER_MIDBUTTON;    fait dans DoMouseClick()
            break;
        case MIDDLEUP:
            fo->MemButs &= ~IEQUALIFIER_MIDBUTTON;
            break;

        case MENUDOWN:
            event = IECODE_RBUTTON; qual = IEQUALIFIER_RBUTTON;
            // fo->MemButs |= IEQUALIFIER_RBUTTON;      fait dans DoMouseClick()
            break;
        case MENUUP:
            fo->MemButs &= ~IEQUALIFIER_RBUTTON;
            break;
        }
    if (event) DoMouseClick( fo, event, qual );
}

static void MenuCancel( struct UIKObjMenu *mo, struct AZObjFile *fo )
{
  struct IntuiMessage *imsg = AGP.UIK->IntuiMsg;

    set_intui_vars();
    if (imsg->Code == MENUHOT && (imsg->Qualifier & IEQUALIFIER_RBUTTON))
        {
        struct MouseHandle *kh = (struct MouseHandle *) AGP.AZMast->MouseHandle;
        if (kh->TickFlags & SCODE_TESTRBUT)
            {
            //if (imsg->IDCMPWindow->TopEdge + imsg->MouseY > imsg->IDCMPWindow->WScreen->BarHeight)
            if (ofi_WhichViewPointer( fo ))
                {
                imsg->Code = MENUCANCEL;
                if (! TST_INPUTLOCK)
                    {
                    DoMouseClick( fo, IECODE_RBUTTON, IEQUALIFIER_RBUTTON );
                    }
                }
            }
        }
}

static void WindowMove( struct UIKObj *wo, struct AZObjFile *fo )
{
    set_intui_vars();
    if (TST_INPUTLOCK) return;
    if (fo->MemButs & IEQUALIFIER_LEFTBUTTON)
        FromMouse( SCODE_MOUSEMOVE );
#ifdef AZUR_DEMO
    if (fo->Text.NodeNum+3 > AZURDEMO_MAXLINES+3)
        fo->MaxLines = AZURDEMO_MAXLINES+3;
#endif
}

static void WindowTick( struct UIKObj *wo, struct AZObjFile *fo )
{
    set_intui_vars();
    FromMouse( SCODE_MOUSETICK );
    if (fo->MouseX != fo->OldMouseX || fo->MouseY != fo->OldMouseY)
        {
        func_RemoveNullPointer();
        if (fo->MemButs & (IEQUALIFIER_LEFTBUTTON|IEQUALIFIER_MIDBUTTON|IEQUALIFIER_RBUTTON))
            {
            FromMouse( SCODE_MOUSEMOVE );
            }
         }
#ifdef AZUR_DEMO
    if (fo->Text.NodeNum > AZURDEMO_MAXLINES)
        fo->MaxLines = AZURDEMO_MAXLINES;
#endif
}

//--------------------------------------

void ofi_SetIconColor()
{
  struct UIKObjWindow *wo, *icon;
  struct UIKObj *to;

    if (wo = AGP.FObj->WO)
        {
        wo->IconColors = (TST_MODIFIED)
            ? B2L(UIKCOL_LIGHT,UIKCOL_GREY,UIKCOL_WHITE,UIKCOL_WHITE)
            : B2L(UIKCOL_LIGHT,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_BLACK);

        if (wo->Obj.Status == UIKS_ICONIFIED)
            {
            if (icon = (struct UIKObjWindow *)wo->Obj.Icon)
                {
                if (to = icon->TitleBox)
                    {
                    *((ULONG*)&to->ActBoxPen) = wo->IconColors;
                    UIK_Refresh( to, 1 );
                    }
                }
            }
        }
}

//--------------------------------------

void ofi_ComputeViewBox( struct AZViewInfo *vi, struct UIKObjWindow *wo, ULONG fh )
{
    vi->ViewBox.Left = vi->ViewBox.Top = 0;
    vi->ViewBox.Width = wo->Obj.Box.Width - (wo->LeftBorderWidth + wo->RightBorderWidth);
    vi->ViewBox.Height = wo->Obj.Box.Height - (wo->TopBorderHeight + wo->BottomBorderHeight);
    if (AGP.Prefs->EleMask & BUMF_STATUSBAR)
        {
        vi->ViewBox.Top = fh + STATUSBAR_HEIGHTOFFS;
        vi->ViewBox.Height -= (fh + STATUSBAR_HEIGHTOFFS);
        }
    if (AGP.Prefs->EleMask & BUMF_CMDLINE) vi->ViewBox.Height -= (fh + CMDLINE_HEIGHTOFFS);
}

struct UIKObj *ofi_AddStat( ULONG lefttop, ULONG widthheight, UBYTE *fontname, ULONG fontheight )
{
  struct UIKObj *stat;

    if (! (stat = (struct UIKObj *) UIK_AddPrivateObjectTagsB( UIKBase, &ObjStatRegGerm, AGP.FObj->WO,
        UIKTAG_OBJ_LeftTop,             lefttop,
        UIKTAG_OBJ_WidthHeight,         widthheight,
        UIKTAG_OBJ_FontName,            fontname,
        UIKTAG_OBJ_FontHeight,          fontheight,
        UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
        UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
        TAG_END ))) return(0);
    return( stat );
}

struct UIKObj *ofi_AddCmdLine( ULONG lefttop, ULONG widthheight, UBYTE *fontname, ULONG fontheight )
{
  struct UIKObj *cmdl;

    if (! (cmdl = UIK_AddObjectTagsB( UIKBase, "UIKObj_String", AGP.FObj->WO,
        UIKTAG_OBJ_LeftTop,         lefttop,
        UIKTAG_OBJ_WidthHeight,     widthheight,
        UIKTAG_OBJ_BoxAutomate,     CmdLineAuto,
        UIKTAG_OBJ_BoxAltAutomate,  CmdLineAltAuto,
        UIKTAG_OBJ_BoxFl_Filled,    FALSE,
        UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
        UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
        UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
        UIKTAG_OBJ_FontName,        fontname,
        UIKTAG_OBJ_FontHeight,      fontheight,
        UIKTAG_OBJ_UserValue1,      AGP.FObj, // pour la routine si sous input.device
        UIKTAG_OBJStr_TextPens,     UIKCOLS(UIKCOL_BLACK,UIKCOL_BLACK,UIKCOL_WHITE,UIKCOL_GREY),
        UIKTAG_OBJStrFl_AutoHeight, FALSE,
        UIKTAG_OBJStr_InactiveFunc, CmdLine_InactHandler,
        TAG_END ))) return(0);
    if (! UIK_AddVector( cmdl, (void*)Key2CmdLine, IDCMP_RAWKEY, (ULONG)AGP.FObj ))
        { UIK_Remove( cmdl ); return(0); }
    return( cmdl );
}

//--------------------------------------
static void hdl_reactivate_fobj( struct AZObjFile *fo )
{
  struct UIKObj *wo = fo->WO;

    if (wo)
        {
        if (wo->Status == UIKS_ICONIFIED)
            {
            fo->Obj.UIK->ActiveKeyTank = fo;
            }
        else{
            if (wo->w)
                if (wo->w->Flags & WFLG_WINDOWACTIVE)
                    if (! wo->w->FirstRequest)
                        fo->Obj.UIK->ActiveKeyTank = fo;
            }
        }
#ifdef AZUR_DEMO
    if (fo->Text.NodeNum+2 > AZURDEMO_MAXLINES+2)
        fo->MaxLines = AZURDEMO_MAXLINES+2;
#endif
}

static void CmdLine_InactHandler( struct UIKObjStrGad *ostr )
{
    // l'input.device ne connait pas A4 donc pas AGP, et pas les librairies !!!
    if (! ostr->Obj.w->FirstRequest) // Signal( (APTR)ostr->Obj.UserULong1, SIGBREAKF_CTRL_E );
        hdl_reactivate_fobj( (APTR)ostr->Obj.UserULong1 );
}

static void Key2CmdLine( struct UIKObjStrGad *ostr, struct AZObjFile *fo )
{
  struct IntuiMessage *imsg = AGP.UIK->IntuiMsg;
  struct UIKKeySupp *ks;
  UWORD alt, shift, ctrl;
  UBYTE *cmd, *ptr, *last;
  ULONG len, num, add;

    if (!imsg || (imsg->Code & 0x80)) return;
    ks = &AGP.UIK->KeySupp;

    alt = imsg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT);
    ctrl = imsg->Qualifier & (IEQUALIFIER_CONTROL);
    shift = imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT);

    switch (imsg->Code)
        {
        case 0x45: break;        /* esc */
        case 0x4c:                                              /* haut */
            if ((num = fo->CmdLineList.NodeNum) > 1)
                {
                if (fo->CmdLineIndex)
                    {
                    ULONG sglen;
                    ptr = fo->CmdLineList.UNode[fo->CmdLineIndex-1].Buf;
                    sglen = intAZ_LineBufLen( ptr );
                    UIK_LockObjVars( fo->CmdLine );
                    ((struct UIKObjStrGad *)fo->CmdLine)->CursorPos = sglen;
                    if (UIK_OSt_Set( fo->CmdLine, ptr, sglen ))
                        fo->CmdLineIndex--;
                    UIK_UnlockObjVars( fo->CmdLine );
                    }
                }
            break;
        case 0x4d:                                              /* bas */
            if (num = fo->CmdLineList.NodeNum)
                {
                ULONG sglen;
                if (++fo->CmdLineIndex >= fo->CmdLineList.NodeNum)
                    fo->CmdLineIndex = fo->CmdLineList.NodeNum - 1;
                ptr = fo->CmdLineList.UNode[fo->CmdLineIndex].Buf;
                sglen = intAZ_LineBufLen( ptr );
                UIK_LockObjVars( fo->CmdLine );
                ((struct UIKObjStrGad *)fo->CmdLine)->CursorPos = sglen;
                UIK_OSt_Set( fo->CmdLine, ptr, sglen );
                UIK_UnlockObjVars( fo->CmdLine );
                }
            break;
        case 0x40:                                              /* space */
            break;

        case 0x43:                                              /* ent */
        case 0x44:                                              /* ret */
            len = 0;
            ptr = 0;
            UIK_LockObjVars( ostr );
            if (cmd = UIK_OSt_Get( ostr, &len ))
                if (len)
                    if (ptr = PoolAlloc( len+1 )) CopyMem( cmd, ptr, len+1 );
            UIK_UnlockObjVars( ostr );
            if (ptr && len)
                {
                if ((eng_ExecuteAZurCmd( ptr, FROMPGM, 0 )) == 0)
                    {   // ajoute à l'historique
                    add = 1;
                    if ((num = fo->CmdLineList.NodeNum) > 1)
                        {   // compare avec la dernière entrée
                        if (StrNCCmp( ptr, fo->CmdLineList.UNode[num-2].Buf ) == 0)
                            add = 0; // existe déjà
                        }
                    if (add)
                        {
                        if (last = UIK_nl_AllocNode( &fo->CmdLineList, num - 1, len, 0 ))
                            {
                            StrCpy( last, ptr );
                            if (num > AGP.Prefs->HistCmdLineMax)
                                UIK_nl_FreeNodes( &fo->CmdLineList, 0, 0 );
                            }
                        }
                    UIK_OSt_Set( ostr, "", 0 );
                    fo->CmdLineIndex = fo->CmdLineList.NodeNum - 1;
                    }
                PoolFree( ptr );
                }
            if (len && (ostr->Obj.w->Flags & WFLG_WINDOWACTIVE))
                if (! ostr->Obj.w->FirstRequest)
                    UIK_ActivateKeyTank( ostr );
            break;

        default:
            break;
        }
}

//--------------------------------------

static void AppWin_IconReceived( struct AZObjFile *fo )
{
  struct AppMessage *amsg;
  struct WBArg *argptr;
  UBYTE *ub;
  ULONG i;

    if (ub = BufAllocP( AGP.Pool, 0, 300 ))
        {
        while (amsg = (struct AppMessage *) GetMsg( fo->AppWinPort ))
            {
            if (amsg->am_Type == AMTYPE_APPWINDOW)
                {
                //------ Si plusieurs, iconification dans des nouvelles fenêtres
                argptr = amsg->am_ArgList;
                argptr++;
                for (i=1; i < amsg->am_NumArgs; i++)
                    {
                    CurrentDir( argptr->wa_Lock );
                    if (argptr->wa_Name && argptr->wa_Name[0] && lo_FileExists( argptr->wa_Name ))
                        {
                        //lo_GetPath( argptr->wa_Name, &ub );
                        if (! func_LaunchProcess( AZTAG_LaunchF_Iconify,   TRUE,
                                                  AZTAG_Launch_FileName,   argptr->wa_Name,
                                                  AZTAG_Launch_DirLock,    argptr->wa_Lock,
                                                  AZTAG_LaunchF_QuitAfter, TRUE,
                                                  TAG_END )) break;
                        }
                    argptr++;
                    }
                CurrentDir( AGP.CurDirLock );

                //------ Chargement du premier dans cette même fenêtre
                argptr = amsg->am_ArgList;
                CurrentDir( argptr->wa_Lock );
                lo_GetPath( argptr->wa_Name, &ub );
                i = BufLength( ub );
                if (ub[i-1] == '/') ub[i-1] = 0;
                CurrentDir( AGP.CurDirLock );

                if (argptr->wa_Name && argptr->wa_Name[0] && lo_FileExists( ub ))
                    {
                    BufTruncate( AGP.UBufTmp, 0 );
                    if (func_BufPrintf( &AGP.UBufTmp, 0, "File2Win CLEAR NOCURS \"%ls\"", ub ))
                        eng_ExecuteAZurCmd( AGP.UBufTmp, FROMPGM, 0 );
                    }

                }
            ReplyMsg( amsg );
            }
        BufFree( ub );
        }
    fo->AppWinVect = AppWin_IconReceived;
}

/****************************************************************
 *
 *      Germe
 *
 ****************************************************************/

static void test_over_window( WORD *pl, WORD *pt, WORD width, WORD height, struct UIKWinScrInfo *wsi, WORD TopBorderHeight, WORD RightBorderWidth )
{
  ULONG ilock;
  WORD mod;
  struct Screen *sc;
  struct Window *fw=0, *w;

    ilock = LockIBase( 0 );
    sc = IntuitionBase->FirstScreen;
    while (sc)
        {
        if (sc == wsi->Scr) { fw = sc->FirstWindow; break; }
        sc = sc->NextScreen;
        }
    RELOOP:
    w = fw; // si écran inexistant -> fw == 0
    mod = 0;
    while (w)
        {
        if (IsUIKWindow( w ))
            if (! (w->Flags & WFLG_BACKDROP))
                {
                if (w->LeftEdge == *pl && w->TopEdge == *pt)
                    {
                    if (*pl + width < wsi->Width)
                        {
                        *pl += AGP.FObj->VI.VBorderWidth + RightBorderWidth;
                        mod = 1;
                        }
                    if (*pt + height < wsi->Height)
                        {
                        *pt += TopBorderHeight;
                        mod = 1;
                        }
                    }
                if (mod) goto RELOOP;
                }
        w = w->NextWindow;
        }
    UnlockIBase( ilock );
}

static void WinStopFunc() // mis sur UserStopFunc
{
    AGP.Process->pr_WindowPtr = 0;
}

static int WindowStartAfter( struct UIKObjWindow *wo ) // patch le startafter générique
{
  struct AZObjFile *fo = AGP.FObj;
  int rc;

    AGP.Process->pr_WindowPtr = wo->Obj.w;
    func_ResetPointer();

    if (rc = (*fo->OldWOStartAfter)( wo ))
        if (WorkbenchBase && fo->AppWinPort)
            fo->AppWin = AddAppWindowA( 0, 0, wo->Obj.w, fo->AppWinPort, NULL );
#ifdef AZUR_DEMO
    if (fo->Text.NodeNum > AZURDEMO_MAXLINES+7)
        ExecBase->ThisTask->tc_SigRecvd |= (0x00010000 | SIGBREAKF_CTRL_C); /* pour valeur pas reconnaissable */
#endif
    return( rc );
}

static void WindowStop( struct UIKObjWindow *wo )  // patch le stop générique
{
  struct AZObjFile *fo = AGP.FObj;
  struct Screen *sc=0;
  APTR amsg;

    if (fo->AppWin)
        {
        while (amsg = (struct AppMessage *) GetMsg( fo->AppWinPort )) ReplyMsg( amsg );
        RemoveAppWindow( fo->AppWin );
        fo->AppWin = 0;
        }
    if (wo->Obj.w) sc = wo->Obj.w->WScreen;
    (*fo->OldWOStop)( wo ); // fermeture du screen UIK par défaut !!!
    if (wo->Obj.Parent->Status == UIKS_STARTED) activate_plain_window( sc );
}

ULONG ofi_OpenWindowStuff( struct OpenWindowStuff *ows, UBYTE *title, ULONG screentag, ULONG screendata )
{
  struct TextFont *font=0;
  struct UIKWinScrInfo wsi;
  struct UIKObjWindow *tmpwo;
  struct AZObjFile *fo = AGP.FObj;
  WORD winleft, wintop, winwidth, winheight, borderswidth, fh;

    if (! TST_NOWINDOW)
        {
        if (! (font = UIK_OpenFont( AGP.Prefs->ViewFontName, AGP.Prefs->ViewFontHeight ))) goto END_ERROR;
        fh = font->tf_YSize;

        if (screentag == TAG_IGNORE)
            {
            /*if (AGP.AZMast->ScreenObj)
                { screentag = UIKTAG_OBJWindow_ScreenAddr; screendata = (ULONG) AGP.AZMast->ScreenObj->Obj.sc; }
            else*/ if (AGP.AZMast->ScreenAdd)
                { screentag = UIKTAG_OBJWindow_ScreenAddr; screendata = (ULONG) AGP.AZMast->ScreenAdd; }
            else if (AGP.AZMast->PubScreen)
                { screentag = UIKTAG_OBJWindow_PubScreenName; screendata = (ULONG) AGP.AZMast->PubScreen; }
            else if (TST_FRONTSCREEN)
                { screentag = UIKTAG_OBJWindowFl_FrontScreen; screendata = TRUE; }
            }

        if (! (tmpwo = (struct UIKObjWindow *) UIK_AddObjectTagsB( UIKBase, "UIKObj_Window", fo,
            UIKTAG_OBJ_LeftTop,             W2L(0x7fff,0x7fff),
            UIKTAG_OBJ_WidthHeight,         W2L(1,1),
            UIKTAG_OBJWindowFl_NoFill,      TRUE,
            screentag,                      screendata,
            UIKTAG_OBJWindowFl_With_Depth,  TRUE,
            UIKTAG_OBJWindowFl_With_Size ,  TRUE,
            UIKTAG_OBJWindowFl_With_Close,  FALSE,
            UIKTAG_OBJWindowFl_With_Zoom,   FALSE,
            UIKTAG_OBJWindowFl_With_Icon,   FALSE,
            UIKTAG_OBJWindowFl_With_Drag,   FALSE,
            TAG_END ))) goto END_ERROR;
        UIK_Call( tmpwo, UIKFUNC_Window_GetWinScrInfo, (ULONG)&wsi, 0 );
        get_aspect( fo, wsi.ModeId );

        fo->VI = AGP.Prefs->VI;
        if (! TST_NOAUTOSIZE) auto_size( fo, &fo->VI );

        //------ Prend la taille de la fenêtre
        borderswidth = (tmpwo->LeftBorderWidth + tmpwo->RightBorderWidth);
        winleft   = fo->VI.ViewBox.Left;
        wintop    = fo->VI.ViewBox.Top;
        winwidth  = fo->VI.ViewBox.Width;
        winheight = fo->VI.ViewBox.Height;
        if (ows->tag_lefttop != -1)
            {
            if ((winleft = ows->tag_lefttop >> 16)   == (WORD)-1) winleft = fo->VI.ViewBox.Left; else fo->VI.ViewBox.Left = winleft;
            if ((wintop = ows->tag_lefttop & 0xffff) == (WORD)-1) wintop  = fo->VI.ViewBox.Top;  else fo->VI.ViewBox.Top  = wintop;
            }
        if (ows->tag_widthheight != -1)
            {
            if ((winwidth  = ows->tag_widthheight >> 16)    == 0xffff) winwidth  = fo->VI.ViewBox.Width;  else fo->VI.ViewBox.Width  = winwidth;
            if ((winheight = ows->tag_widthheight & 0xffff) == 0xffff) winheight = fo->VI.ViewBox.Height; else fo->VI.ViewBox.Height = winheight;
            }
        if (ows->tag_textwidthheight != -1)
            {
            winwidth  = ((ows->tag_textwidthheight >> 16) + 1) * font->tf_XSize;
            winheight = ((ows->tag_textwidthheight & 0xffff) + 1) * fh;

            if (AGP.Prefs->EleMask & BUMF_BOTTOMBORDER) winheight += fo->VI.HBorderHeight;
            if (AGP.Prefs->EleMask & BUMF_STATUSBAR) winheight += (fh + STATUSBAR_HEIGHTOFFS);
            if (AGP.Prefs->EleMask & BUMF_CMDLINE) winheight += (fh + CMDLINE_HEIGHTOFFS);
            winheight += (tmpwo->TopBorderHeight + tmpwo->BottomBorderHeight);

            if (AGP.Prefs->EleMask & BUMF_RIGHTBORDER) winwidth += fo->VI.VBorderWidth;
            winwidth += borderswidth;
            /*
            fo->VI.ViewBox.Width  = winwidth ;
            fo->VI.ViewBox.Height = winheight;
            */
            }
        if (! TST_FULLSCREEN) test_over_window( &winleft, &wintop, winwidth, winheight, &wsi, tmpwo->TopBorderHeight, tmpwo->RightBorderWidth );

        //------ Déduis la taille intérieure de la vue
        tmpwo->Obj.Box.Width = winwidth;
        tmpwo->Obj.Box.Height = winheight;
        ofi_ComputeViewBox( &fo->VI, tmpwo, fh );
        /*
        fo->VI.ViewBox.Left = 0;
        fo->VI.ViewBox.Top = 0;
        fo->VI.ViewBox.Width -= borderswidth;
        fo->VI.ViewBox.Height -= (tmpwo->TopBorderHeight + tmpwo->BottomBorderHeight);
        if (AGP.Prefs->EleMask & BUMF_STATUSBAR)
            {
            fo->VI.ViewBox.Top = fh + STATUSBAR_HEIGHTOFFS;
            fo->VI.ViewBox.Height -= (fh + STATUSBAR_HEIGHTOFFS);
            }
        if (AGP.Prefs->EleMask & BUMF_CMDLINE) fo->VI.ViewBox.Height -= (fh + CMDLINE_HEIGHTOFFS);
        */

        //------ Vérifie la taille minimum intérieure de la vue
        ofi_VerifViewSize( &fo->VI, fh, borderswidth );

        //------ Rajouter si elle était inférieure à la taille minimum
        winwidth  = fo->VI.ViewBox.Width + borderswidth;
        winheight = fo->VI.ViewBox.Height + (tmpwo->TopBorderHeight + tmpwo->BottomBorderHeight);
        if (AGP.Prefs->EleMask & BUMF_STATUSBAR) winheight += (fh + STATUSBAR_HEIGHTOFFS);
        if (AGP.Prefs->EleMask & BUMF_CMDLINE) winheight += (fh + CMDLINE_HEIGHTOFFS);

        //------ Supprime la fenêtre de test, ajoute la vraie
        UIK_Remove( tmpwo );

        if (! (ows->newwo = fo->WO = AGP.WO = (struct UIKObjWindow *) UIK_AddObjectTagsB( UIKBase, "UIKObj_Window", fo,
            UIKTAG_OBJ_LeftTop,             W2L(winleft,wintop),
            UIKTAG_OBJ_WidthHeight,         W2L(winwidth,winheight),
            UIKTAG_OBJWindow_MinimumWidthHeight, W2L(fo->VI.MinWidth+borderswidth,fo->VI.MinHeight),
            UIKTAG_OBJ_Title,               title,
            UIKTAG_OBJ_AltTitle,            UIK_LangString( AGP.UIK, TEXT_AZURNAME ),
            UIKTAG_OBJ_TitleFl_Addr,        TRUE,
            UIKTAG_OBJ_UserStopFunc,        WinStopFunc,
            UIKTAG_OBJ_FontName,            AGP.Prefs->FontName,
            UIKTAG_OBJ_FontHeight,          AGP.Prefs->FontHeight,
            UIKTAG_OBJWindowFl_AutoAdjustSize, FALSE,
            UIKTAG_OBJWindowFl_CloseStop,   FALSE,
            UIKTAG_OBJWindowFl_NoFill,      TRUE,
            UIKTAG_OBJWindowFl_AutoShrink,  TRUE,
            screentag,                      screendata,
            UIKTAG_OBJWindowFl_NearFullDisplay, TST_FULLSCREEN ? TRUE:FALSE,
            TAG_END ))) goto END_ERROR;

        //------ Détournements pour AppWindow
        fo->OldWOStartAfter = AGP.WO->Obj.StartAfterVector;
        fo->OldWOStop = AGP.WO->Obj.StopVector;
        AGP.WO->Obj.StartAfterVector = WindowStartAfter;
        AGP.WO->Obj.StopVector = WindowStop;

        //------ A cause du AutoShrink on recalcule
        ofi_ComputeViewBox( &fo->VI, ows->newwo, fh );

        if (! UIK_AddVector( ows->newwo, (void*)ofi_QuitVerify, IDCMP_CLOSEWINDOW, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)BlinkCursor, IDCMP_INTUITICKS, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)ofi_TickUpdateDisplay, IDCMP_INTUITICKS, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)Inact_SetCursor, IDCMP_INACTIVEWINDOW, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)NewWindowSize, IDCMP_NEWSIZE, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)WindowKeyTank, IDCMP_ACTIVEWINDOW, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)WindowKeyTank, IDCMP_REQCLEAR, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)WindowKeyTank, IDCMP_MOUSEBUTTONS, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)WindowButtons, IDCMP_MOUSEBUTTONS, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)WindowMove, IDCMP_MOUSEMOVE, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( ows->newwo, (void*)WindowTick, IDCMP_INTUITICKS, (ULONG)fo )) goto END_ERROR;

        if (AGP.Prefs->EleMask & BUMF_STATUSBAR)
            {
            if (! (fo->Stat = ofi_AddStat( W2L(0,0),
                                           W2L(fo->VI.ViewBox.Width,(fh+STATUSBAR_HEIGHTOFFS)),
                                           AGP.Prefs->ViewFontName,
                                           fh ))) goto END_ERROR;
            }

        if (AGP.Prefs->EleMask & BUMF_CMDLINE)
            {
            if (! (fo->CmdLine = ofi_AddCmdLine( W2L(0,(fo->VI.ViewBox.Top + fo->VI.ViewBox.Height)),
                                                 W2L(fo->VI.ViewBox.Width,(fh+CMDLINE_HEIGHTOFFS)),
                                                 AGP.Prefs->ViewFontName,
                                                 fh ))) goto END_ERROR;
            }

        if (! ofi_AddView( fo, &fo->VI )) goto END_ERROR;

        if (font) CloseFont( font );

        ofi_SetColorMask();
        }

    return(1);

  END_ERROR:
    if (font) CloseFont( font );
    return(0);
}

static int GenericAdd( struct AZObjFile *fo, struct TagItem *taglist )
{
  struct OpenWindowStuff OWS;
  UBYTE *filename;
  //struct TextFont *font=0;

    AGP.FObj = fo;

    if (! UIK_nl_AllocListP( NULL, &fo->Text, 256, MEMF_ANY, AGP.Prefs->PuddleSize*1024, AGP.Prefs->PuddleSize*1024 )) goto END_ERROR;
    if (! UIK_nl_AllocNode( &fo->Text, -1, 0, 0 )) goto END_ERROR; // nécessaire dès le début

    fo->Ind[0] = * UIK_LangString( fo, TEXT_Char_Sensitive );
    fo->Ind[1] = * UIK_LangString( fo, TEXT_Char_Insensitive );
    fo->Ind[2] = * UIK_LangString( fo, TEXT_Char_Add );
    fo->Ind[3] = * UIK_LangString( fo, TEXT_Char_Overlay );
    fo->Ind[4] = * UIK_LangString( fo, TEXT_Char_Modified );
    fo->Ind[5] = * UIK_LangString( fo, TEXT_Char_Unmodified );
    fo->Ind[6] = * UIK_LangString( fo, TEXT_Char_AutoFormat );
    fo->Ind[7] = * UIK_LangString( fo, TEXT_Char_NoFormat );
    fo->Ind[8] = * UIK_LangString( fo, TEXT_Char_DebugSet );
    fo->Ind[9] = * UIK_LangString( fo, TEXT_Char_NoDebug );

    if (! func_BlockInit()) goto END_ERROR;
    if (! func_MacroInit()) goto END_ERROR;
    if (! func_BMInit()) goto END_ERROR;

    if (! (fo->OpenFileName = BufAllocP( AGP.Pool, 0, 10 ))) goto END_ERROR;
    if (! (fo->SaveFileName = BufAllocP( AGP.Pool, 0, 10 ))) goto END_ERROR;
    if (! (fo->BkupFileName = BufAllocP( AGP.Pool, 0, 10 ))) goto END_ERROR;
    if (! (fo->AutsFileName = BufAllocP( AGP.Pool, 0, 10 ))) goto END_ERROR;
    if (! (fo->LineString = BufAllocP( AGP.Pool, 0, 20 ))) goto END_ERROR;
    if (! (fo->ColString = BufAllocP( AGP.Pool, 0, 20 ))) goto END_ERROR;
    if (! (fo->ReqString = BufAllocP( AGP.Pool, 0, 20 ))) goto END_ERROR;
    if (! (fo->HexString = BufAllocP( AGP.Pool, 0, 20 ))) goto END_ERROR;

    fo->Flags = PackBoolTags( fo->Flags, taglist, ObjFile_BoolMap );
    fo->MaxLines = AGP.AZMast->MaxLines;
#ifdef AZUR_DEMO
    fo->MaxLines = AZURDEMO_MAXLINES;
#endif
    fo->LeftMargin = AGP.Prefs->LeftMargin;
    fo->RightMargin = AGP.Prefs->RightMargin;

    if (! (fo->SeRep = PoolAlloc( sizeof(struct SearchReplace) ))) goto END_ERROR;
    if (! (fo->SeRep->SearchStr = BufAllocP( AGP.Pool, 0, 20 ))) goto END_ERROR;
    if (! (fo->SeRep->ReplaceStr = BufAllocP( AGP.Pool, 0, 20 ))) goto END_ERROR;
    if (! (fo->SeRep->ConvSearchStr = BufAllocP( AGP.Pool, 0, 4 ))) goto END_ERROR;
    if (! (fo->SeRep->ConvReplaceStr = BufAllocP( AGP.Pool, 0, 4 ))) goto END_ERROR;
    fo->SeRep->SearchFlags = AGP.Prefs->SearchFlags;

    NewList( &fo->ViewList );

    if (! TST_NOWINDOW)
        {
        if (! UIK_nl_AllocListP( AGP.Pool, &fo->CmdLineList, 10, 0,0,0 )) goto END_ERROR;
        if (! UIK_nl_AllocNode( &fo->CmdLineList, -1, 0, 0 )) goto END_ERROR;

        if (! UIK_AddVector( fo, (void*)RawKeys, IDCMP_RAWKEY, (ULONG)AGP.AZMast->KeyHandle )) goto END_ERROR;

        OWS.tag_lefttop         = GetTagData( UIKTAG_OBJ_LeftTop, -1, taglist );
        OWS.tag_widthheight     = GetTagData( UIKTAG_OBJ_WidthHeight, -1, taglist );
        OWS.tag_textwidthheight = GetTagData( AZTAG_File_TextWidthHeight, -1, taglist );
        if (OWS.tag_widthheight != -1 || OWS.tag_textwidthheight != -1) CLR_FULLSCREEN;

        if (! ofi_OpenWindowStuff( &OWS, UIK_LangString( AGP.UIK, TEXT_UnTitled ), TAG_IGNORE, 0 )) goto END_ERROR;

        if (fo->AppWinPort = (struct MsgPort *) CreatePort( 0, 0 ))
            {
            fo->AppWinVect = AppWin_IconReceived;
            UIK_AddVector( fo->Obj.UIK, (void*)&fo->AppWinVect, 1<<fo->AppWinPort->mp_SigBit, (ULONG)fo );
            }

        }
    fo->CtrlDVect = CtrlD_QuitVerify;
    if (! UIK_AddVector( fo->Obj.UIK, (void*)&fo->CtrlDVect, SIGBREAKF_CTRL_D, (ULONG)fo )) goto END_ERROR;
 /*
    if (! TST_NOWINDOW)
        {
        ULONG screentag=TAG_IGNORE, screendata=0;
        struct UIKWinScrInfo wsi;
        struct UIKObjWindow *tmpwo;
        WORD winleft, wintop, winwidth, winheight, borderswidth, fh;
        ULONG val;

        if (! (font = UIK_OpenFont( AGP.Prefs->ViewFontName, AGP.Prefs->ViewFontHeight ))) goto END_ERROR;
        fh = font->tf_YSize;

        /*if (AGP.AZMast->ScreenObj)
            { screentag = UIKTAG_OBJWindow_ScreenAddr; screendata = (ULONG) AGP.AZMast->ScreenObj->Obj.sc; }
        else*/ if (AGP.AZMast->ScreenAdd)
            { screentag = UIKTAG_OBJWindow_ScreenAddr; screendata = (ULONG) AGP.AZMast->ScreenAdd; }
        else if (AGP.AZMast->PubScreen)
            { screentag = UIKTAG_OBJWindow_PubScreenName; screendata = (ULONG) AGP.AZMast->PubScreen; }
        else if (TST_FRONTSCREEN)
            { screentag = UIKTAG_OBJWindowFl_FrontScreen; screendata = TRUE; }

        if (! (tmpwo = (struct UIKObjWindow *) UIK_AddObjectTagsB( UIKBase, "UIKObj_Window", fo,
            UIKTAG_OBJ_LeftTop,             W2L(0x7fff,0x7fff),
            UIKTAG_OBJ_WidthHeight,         W2L(1,1),
            UIKTAG_OBJWindowFl_NoFill,      TRUE,
            screentag,                      screendata,
            UIKTAG_OBJWindowFl_With_Depth,  TRUE,
            UIKTAG_OBJWindowFl_With_Size ,  TRUE,
            UIKTAG_OBJWindowFl_With_Close,  FALSE,
            UIKTAG_OBJWindowFl_With_Zoom,   FALSE,
            UIKTAG_OBJWindowFl_With_Icon,   FALSE,
            UIKTAG_OBJWindowFl_With_Drag,   FALSE,
            TAG_END ))) goto END_ERROR;
        UIK_Call( tmpwo, UIKFUNC_Window_GetWinScrInfo, (ULONG)&wsi, 0 );
        get_aspect( AGP.FObj, wsi.ModeId );

        fo->VI = AGP.Prefs->VI;
        if (! TST_NOAUTOSIZE) auto_size( fo );

        //------ Prend la taille de la fenêtre
        borderswidth = (tmpwo->LeftBorderWidth + tmpwo->RightBorderWidth);
        winleft   = fo->VI.ViewBox.Left;
        wintop    = fo->VI.ViewBox.Top;
        winwidth  = fo->VI.ViewBox.Width;
        winheight = fo->VI.ViewBox.Height;
        if ((val = GetTagData( UIKTAG_OBJ_LeftTop, -1, taglist )) != -1)
            {
            if ((winleft = val >> 16)   == (WORD)-1) winleft = fo->VI.ViewBox.Left; else fo->VI.ViewBox.Left = winleft;
            if ((wintop = val & 0xffff) == (WORD)-1) wintop  = fo->VI.ViewBox.Top;  else fo->VI.ViewBox.Top  = wintop;
            }
        if ((val = GetTagData( UIKTAG_OBJ_WidthHeight, -1, taglist )) != -1)
            {
            CLR_FULLSCREEN;
            if ((winwidth  = val >> 16)    == 0xffff) winwidth  = fo->VI.ViewBox.Width;  else fo->VI.ViewBox.Width  = winwidth;
            if ((winheight = val & 0xffff) == 0xffff) winheight = fo->VI.ViewBox.Height; else fo->VI.ViewBox.Height = winheight;
            }
        if ((val = GetTagData( AZTAG_File_TextWidthHeight, -1, taglist )) != -1)
            {
            CLR_FULLSCREEN;
            winwidth  = ((val >> 16) + 1) * font->tf_XSize;
            winheight = ((val & 0xffff) + 1) * fh;

            if (AGP.Prefs->EleMask & BUMF_BOTTOMBORDER) winheight += fo->VI.HBorderHeight;
            if (AGP.Prefs->EleMask & BUMF_STATUSBAR) winheight += (fh + STATUSBAR_HEIGHTOFFS);
            if (AGP.Prefs->EleMask & BUMF_CMDLINE) winheight += (fh + CMDLINE_HEIGHTOFFS);
            winheight += (tmpwo->TopBorderHeight + tmpwo->BottomBorderHeight);

            if (AGP.Prefs->EleMask & BUMF_RIGHTBORDER) winwidth += fo->VI.VBorderWidth;
            winwidth += borderswidth;
            /*
            fo->VI.ViewBox.Width  = winwidth ;
            fo->VI.ViewBox.Height = winheight;
            */
            }
        if (! TST_FULLSCREEN) test_over_window( &winleft, &wintop, winwidth, winheight, &wsi, tmpwo->TopBorderHeight, tmpwo->RightBorderWidth );

        //------ Déduis la taille intérieure de la vue
        tmpwo->Obj.Box.Width = winwidth;
        tmpwo->Obj.Box.Height = winheight;
        ofi_ComputeViewBox( &fo->VI, tmpwo, fh );
        /*
        fo->VI.ViewBox.Left = 0;
        fo->VI.ViewBox.Top = 0;
        fo->VI.ViewBox.Width -= borderswidth;
        fo->VI.ViewBox.Height -= (tmpwo->TopBorderHeight + tmpwo->BottomBorderHeight);
        if (AGP.Prefs->EleMask & BUMF_STATUSBAR)
            {
            fo->VI.ViewBox.Top = fh + STATUSBAR_HEIGHTOFFS;
            fo->VI.ViewBox.Height -= (fh + STATUSBAR_HEIGHTOFFS);
            }
        if (AGP.Prefs->EleMask & BUMF_CMDLINE) fo->VI.ViewBox.Height -= (fh + CMDLINE_HEIGHTOFFS);
        */

        //------ Vérifie la taille minimum intérieure de la vue
        ofi_VerifViewSize( &fo->VI, fh, borderswidth );

        //------ Rajouter si elle était inférieure à la taille minimum
        winwidth  = fo->VI.ViewBox.Width + borderswidth;
        winheight = fo->VI.ViewBox.Height + (tmpwo->TopBorderHeight + tmpwo->BottomBorderHeight);
        if (AGP.Prefs->EleMask & BUMF_STATUSBAR) winheight += (fh + STATUSBAR_HEIGHTOFFS);
        if (AGP.Prefs->EleMask & BUMF_CMDLINE) winheight += (fh + CMDLINE_HEIGHTOFFS);

        //------ Supprime la fenêtre de test, ajoute la vraie
        UIK_Remove( tmpwo );

        if (! (ows->newwo = AGP.WO = fo->WO = (struct UIKObjWindow *) UIK_AddObjectTagsB( UIKBase, "UIKObj_Window", fo,
            UIKTAG_OBJ_LeftTop,             W2L(winleft,wintop),
            UIKTAG_OBJ_WidthHeight,         W2L(winwidth,winheight),
            UIKTAG_OBJWindow_MinimumWidthHeight, W2L(fo->VI.MinWidth+borderswidth,fo->VI.MinHeight),
            UIKTAG_OBJ_Title,               UIK_LangString( AGP.UIK, TEXT_UnTitled ),
            UIKTAG_OBJ_AltTitle,            UIK_LangString( AGP.UIK, TEXT_AZURNAME ),
            UIKTAG_OBJ_TitleFl_Addr,        TRUE,
            UIKTAG_OBJ_UserStopFunc,        WinStopFunc,
            UIKTAG_OBJWindowFl_AutoAdjustSize, FALSE,
            UIKTAG_OBJWindowFl_CloseStop,   FALSE,
            UIKTAG_OBJWindowFl_NoFill,      TRUE,
            UIKTAG_OBJWindowFl_AutoShrink,  TRUE,
            screentag,                      screendata,
            UIKTAG_OBJWindowFl_NearFullDisplay, TST_FULLSCREEN ? TRUE:FALSE,
            TAG_END ))) goto END_ERROR;

        //------ A cause du AutoShrink on recalcule
        ofi_ComputeViewBox( &fo->VI, fo->WO, fh );

        if (! UIK_AddVector( fo->WO, (void*)ofi_QuitVerify, IDCMP_CLOSEWINDOW, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)BlinkCursor, IDCMP_INTUITICKS, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)ofi_TickUpdateDisplay, IDCMP_INTUITICKS, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)Inact_SetCursor, IDCMP_INACTIVEWINDOW, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)NewWindowSize, IDCMP_NEWSIZE, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)WindowKeyTank, IDCMP_ACTIVEWINDOW, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)WindowKeyTank, IDCMP_REQCLEAR, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)WindowKeyTank, IDCMP_MOUSEBUTTONS, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)WindowButtons, IDCMP_MOUSEBUTTONS, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)WindowMove, IDCMP_MOUSEMOVE, (ULONG)fo )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)WindowTick, IDCMP_INTUITICKS, (ULONG)fo )) goto END_ERROR;

        /*
        {
        struct UIKObj *obj;
        if (obj = UIK_GetObject( fo->WO, NULL, UIKID_DEPTGAD ))
            if (! (UIK_AddVector( obj, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        if (obj = UIK_GetObject( fo->WO, NULL, UIKID_ZOOMGAD ))
            if (! (UIK_AddVector( obj, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        if (obj = UIK_GetObject( fo->WO, NULL, UIKID_ICONGAD ))
            if (! (UIK_AddVector( obj, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        }
        */

        if (AGP.Prefs->EleMask & BUMF_STATUSBAR)
            {
            if (! (fo->Stat = ofi_AddStat( W2L(0,0),
                                           W2L(fo->VI.ViewBox.Width,(fh+STATUSBAR_HEIGHTOFFS)),
                                           AGP.Prefs->ViewFontName,
                                           fh ))) goto END_ERROR;
            }

        if (AGP.Prefs->EleMask & BUMF_CMDLINE)
            {
            if (! (fo->CmdLine = ofi_AddCmdLine( W2L(0,(fo->VI.ViewBox.Top + fo->VI.ViewBox.Height)),
                                                 W2L(fo->VI.ViewBox.Width,(fh+CMDLINE_HEIGHTOFFS)),
                                                 AGP.Prefs->ViewFontName,
                                                 fh ))) goto END_ERROR;
            }

        if (! ofi_AddView( fo )) goto END_ERROR;

        if (font) CloseFont( font );
        }
 */

    filename = (UBYTE *) GetTagData( AZTAG_File_FileName, 0, taglist );
    if (! filename) { filename = ""; /* fo->OpenFileName[0] = 0 bug UIK1.32 */; }
    BufSetS( filename, StrLen( filename ), &fo->OpenFileName );
 //kprintf( "------------------\n%ls\n-------------------------\n", fo->OpenFileName ); //Delay(50);

    return(1);

  END_ERROR:
 //   if (font) CloseFont( font );
    return(0);
}

static void GenericRemove( struct AZObjFile *fo )
{
  struct Node *node;

    while (node = RemHead( &fo->ViewList )) FreeMem( node, sizeof(struct Node) );

    UIK_nl_FreeList( &fo->Text );

    /* pooled
    BufFree( fo->SeRep->ReplaceString );
    BufFree( fo->SeRep->SearchString );
    PoolFree( fo->SeRep );

    BufFree( fo->ReqString );
    BufFree( fo->ColString );
    BufFree( fo->LineString );
    BufFree( fo->AutsFileName );
    BufFree( fo->BkupFileName );
    BufFree( fo->SaveFileName );
    BufFree( fo->OpenFileName );
    */

    func_BMCleanup();
    func_MacroCleanup();
    func_BlockCleanup();

    if (fo->AppWinPort) DeletePort( fo->AppWinPort );
}

static int GenericStartAfter( struct AZObjFile *fo )
{
  ULONG namegiven = 1;


    if (! (fo->Flags & AZFILEF_STARTSET))
        {
        fo->Flags |= AZFILEF_STARTSET;

        if (! TST_NOWINDOW)
            {
            struct UIKObj *gad;
            struct Window *w;

            if (w = fo->WO->Obj.w)
                {
                if (w->WScreen != AGP.IntuiBase->FirstScreen) ScreenToFront( w->WScreen );
                }

            if (gad = UIK_GetObject( fo->WO, 0, UIKID_ICONGAD ))
                {
                if (! UIK_AddVector( gad, (void*)ofi_SetIconColor, IDCMP_GADGETDOWN, 0 )) goto END_ERROR;
                }

            //------ Prépare les sous-objets
            ofi_AddMenuObj();
            ofi_AddFileSelectors( fo->Obj.UserULong1, fo->Obj.UserULong1 );

            ofi_ActivateFObjKeyTank();
            }

        if (intAZ_LineBufLen( fo->OpenFileName ) == 0)
            {
            namegiven = 0;
            }

        if (namegiven == 1)
            {
            ULONG rc;
            struct CmdObj *co;

            func_BufPrintf( &AGP.UBufTmp, 0, "File2Win CLEAR NOCURS %ls %ls %ls \"%ls\"",
                        (TST_CREATEFILE)?"CREATE":"",
                        (TST_IGNOREICON)?"NOICON":"",
                        (TST_READONLY)?"READONLY":"",
                        fo->OpenFileName );
            rc = eng_ExecuteAZurCmd( AGP.UBufTmp, FROMPGM, 0 );

            /* Si on est ici cette tache n'a pas encore signalé la tâche parente
             * car on est SYNCHRONE. Donc on peut regarder si on vient de ARexx
             * ou pas afin de faire rater le Start et de pouvoir retourner un code
             * erreur.
             */
            co = AGP.AZMast->Parms->ParmCmdObj;
            if (co && (co->ExeFrom & FROMAREXX)) { if (rc != 0) return(0); }
            }
        else{
            func_ResetFile();
            }

        if (! TST_NOWINDOW)
            {
            //------ Iconifie si besoin
            if (fo->Flags & AZFILEF_ICONIFY)
                {
                ofi_SetIconColor();
                UIK_Iconify( fo->WO );
                }
            else{
                if (namegiven == 1)
                    {
                    func_ReDisplay();
                    }
                else{
                    if (TST_OPENREQ)
                        {
                        func_BufPrintf( &AGP.UBufTmp, 0, "File2Win CLEAR NOCURS %ls %ls %ls",
                                    (TST_CREATEFILE)?"CREATE":"",
                                    (TST_IGNOREICON)?"NOICON":"",
                                    (TST_READONLY)?"READONLY":""
                                    );
                        eng_ExecuteAZurCmd( AGP.UBufTmp, FROMPGM, 0 );
                        }
                    }
                }
            //------ Ici après le start pour éviter deadlock si lancé avec nom de fichier (semaphore)
            if (! UIK_AddVector( fo->WO, (void*)PatchLastDoc, IDCMP_ACTIVEWINDOW, (ULONG)fo )) goto END_ERROR;
            }
        }
    CLR_CREATEFILE;
    CLR_IGNOREICON;
    return(1);
  END_ERROR:
    return(0);
}

static struct TagItem __far GermTagList[] =
{
   { UIKTAG_GEN_InactKeyTank, hdl_reactivate_fobj },
   { TAG_END }
};

static struct UIKObjGerm __far Germ = {
    "AZURObj_File",
    GenericAdd,         /*int     (*AddVector)(); */
    NULL,               /*int     (*StartVector)(); */
    NULL,               /*void    (*StopVector)(); */
    GenericRemove,      /*void    (*RemoveVector)(); */
    NULL,               /*void    (*RefreshVector)(); */
    NULL,               /*void    (*ResizeVector)(); */
    NULL,               /*void    (*ExceptActVector)(); */
    NULL,               /*void    (*ExceptInaVector)(); */
    NULL,               /*void    (*ExceptMovVector)(); */
    NULL,               /*void    (*ExceptKeyVector)(); */
    GenericStartAfter,  /*int     (*StartAfterVector)(); */
    NULL,               /*int     (*SetGetValues)(); */
    NULL,               /*ULONG   *ExtentedVectors; */
    GermTagList,        /*struct TagItem *GermTagList; */
    0,                  /*UWORD   MinWidth; */
    0,                  /*UWORD   MinHeight; */
    0,                  /*ULONG   DefaultWidthHeight; */
    0,                  /*UWORD   ObjId; */
    sizeof(struct AZObjFile),   /*UWORD   ObjSize; */
    0, /* flags par defaut */
    };

struct UIKObjRegGerm __far ObjFileRegGerm = { &Germ, 0, 0 ,0 };
