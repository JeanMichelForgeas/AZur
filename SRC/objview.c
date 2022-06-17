/****************************************************************
 *
 *      File:      objview.c
 *      Project:   gestion d'un fichier
 *
 *      Created:   12-11-92     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uikdrawauto.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objproplift.h"

#include "lci:azur.h"
#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"
#include "lci:azur_protos.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

//extern void ofi_ActivateFObjKeyTank();

extern struct UIKObjRegGerm __far ObjZoneRegGerm;
extern struct UIKObjRegGerm __far ObjSplitRegGerm;
extern UWORD __far AutoDrawBox_Light[];

extern struct UIKBitMap * __far ButBitMapTable[];


/****** Exported ***********************************************/

ULONG oview_ChangeViewPosition( struct AZObjView *vo, LONG newtopline, LONG newtopcol );
void oview_ScrollView( struct AZObjView *vo, ULONG vert, ULONG abslen, LONG len, LONG from );
void oview_Display( struct AZObjView *vo, LONG startline, LONG nlines, LONG startcol, LONG ncols );
void oview_ReDisplay( struct AZObjView *vo );
void oview_FillWindow( struct AZObjView *vo );
void oview_DisplayBlock( struct AZObjView *vo );
void oview_DisplayCmd( struct AZObjView *vo, struct AZDisplayCmd *dcmd );
//void oview_DisplayStats( struct AZObjView *vo, ULONG drawtitle );

void UpdateHProp( struct AZObjView *vo, ULONG draw );
void oview_DisplayView( struct AZObjView *vo );


/****** Statics ************************************************/

//static void oview_ScrollOneView( struct AZObjView *vo, ULONG vert, ULONG abslen, LONG len, LONG from );

static UWORD __far PatProp[] = { 0xAAAA, 0x5555 };
static UWORD __far PropAuto[] = { x_ADD_N,2, y_ADD_1, X_SUB_N,2, Y_SUB_1, END_AUTO };


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

static int CrossResize( struct UIKObj *obj, struct UIKObjWindow *wo )
{
  struct Region *reg=0, *tmp;

    if ((obj->Id == UIKID_REQUEST)
     || (obj->Id == UIKID_SCREEN)
     || (obj->Id == UIKID_WINDOW && obj != wo)) return(1);

    UIK_LockObjVars( obj );
    if (reg = (struct Region *) NewRegion())
        {
        if (OrRectRegion( reg, &obj->ObjExt->InnerRect )) { tmp = obj->Region; obj->Region = reg; reg = tmp; }
        if (reg) DisposeRegion( reg );
        }
    if (obj->ResizeVector) (*obj->ResizeVector)( obj );
    UIK_UnlockObjVars( obj );

    if (obj->List.NodeNum) return( UIK_nl_CrossList( obj, CrossResize, 1, (ULONG)wo ) );
    else return(1);
}


/****************************************************************
 *
 *      Methodes
 *
 ****************************************************************/

static ULONG __far FuncTable[] = {
    (ULONG) 0,
    };


/****************************************************************
 *
 *      Liens
 *
 ****************************************************************/

static void DecTimer( int nil, struct AZObjView *vo )
{
    if (vo->ArrowTimer > 0) vo->ArrowTimer--;
}

static void StartTimer( int nil, struct AZObjView *vo )
{
    vo->ArrowTimer = 3;
}

static void ZeroTimer( int nil, struct AZObjView *vo )
{
    vo->ArrowTimer = 0;
}

static ULONG install_delay( struct UIKObj *obj, struct AZObjView *vo )
{
    if (! UIK_AddVector( obj, (void*)StartTimer, GADGETDOWN, (ULONG)vo )) goto END_ERROR;
    if (! UIK_AddVector( obj, (void*)DecTimer, INTUITICKS, (ULONG)vo )) goto END_ERROR;
    if (! UIK_AddVector( obj, (void*)ZeroTimer, GADGETUP, (ULONG)vo )) goto END_ERROR;
    return(1);
  END_ERROR:
    return(0);
}

//--------------------------

static void DisplayCursor()
{
  struct AZObjFile *fo = AGP.FObj;

    if (AGP.UIK->IntuiMsg)
        if (AGP.UIK->IntuiMsg->Class == IDCMP_GADGETDOWN)
            {
            SETCURSOR_VISIBLE(fo);
            func_DisplayCurs( fo->Line, fo->Col );
            }
}

//-------------------------- Vertical

static LONG link_upbut_2_vprop( struct UIKObj *gad, struct UIKObjPropGad *prop, int nil, struct AZObjView *vo )
{
  LONG rc=0;

    if (prop->LPos)
        {
        UIK_OPr_ChangePropGadget( prop, -1, -1, 0 );
        rc = 1;
        }
    return( rc );
}

static ULONG link_downbut_2_vprop( struct UIKObj *gad, struct UIKObjPropGad *prop, int nil, struct AZObjView *vo )
{
  ULONG p, rc=0;

    if (AGP.FObj->Text.NodeNum+PLUSLINES >= (vo->Zone->Rows - 1)) p = AGP.FObj->Text.NodeNum+PLUSLINES - (vo->Zone->Rows - 1);
    else p = 0;
    if (p != vo->Zone->TopLine)
        {
        UIK_OPr_ChangePropGadget( prop, -1, -1, p );
        rc = 1;
        }
    return( rc );
}

static ULONG link_updown_2_vprop( struct UIKObj *gad, struct UIKObjPropGad *prop, int nil, struct AZObjView *vo )
{
  ULONG p, max, rc=0;

    if (vo->ArrowTimer) goto END_ERROR;

    p = prop->LPos;
    if (gad->MouseY < gad->Box.Top + gad->Box.Height/2)
        {
        if (p) { p--; rc = 1; }
        }
    else{
        p++;
        max = prop->LTot - prop->LVis;
        if (p > max) p = max;
        if (p != prop->LPos) rc = 1;
        }
    if (rc) UIK_OPr_ChangePropGadget( prop, -1, -1, p );
  END_ERROR:
    return( rc );
}

static void link_vprop( struct UIKObjPropGad *prop, struct AZObjZone *zo, int nil, struct AZObjView *vo )
{
    DisplayCursor();
    if (oview_ChangeViewPosition( vo, prop->LPos, zo->TopCol ))
        {
        UPDATE_STAT(AGP.FObj);
        if (AGP.UIK->IntuiMsg) ofi_TickUpdateDisplay( 0, AGP.FObj );
        }
}

//-------------------------- Horizontal

static LONG link_leftbut_2_hprop( struct UIKObj *gad, struct UIKObjPropGad *prop, int nil, struct AZObjView *vo )
{
  LONG rc=0;

    if (prop->LPos)
        {
        UIK_OPr_ChangePropGadget( prop, -1, -1, 0 );
        rc = 1;
        }
    return( rc );
}

static LONG link_rightbut_2_hprop( struct UIKObj *gad, struct UIKObjPropGad *prop, int nil, struct AZObjView *vo )
{
  struct AZObjZone *zo = vo->Zone;
  LONG p, rc=0;

    if (zo->HPropLineLen >= (zo->Cols - 1)) p = zo->HPropLineLen - (zo->Cols - 1);
    else p = 0;
    if (p != zo->TopCol)
        {
        UIK_OPr_ChangePropGadget( prop, -1, -1, p );
        rc = 1;
        }
    return( rc );
}

static LONG link_leftright_2_hprop( struct UIKObj *gad, struct UIKObjPropGad *prop, int nil, struct AZObjView *vo )
{
  struct AZObjZone *zo = vo->Zone;
  LONG p, max, rc=0;

    if (vo->ArrowTimer) goto END_ERROR;

    p = prop->LPos;
    if (gad->MouseX < gad->Box.Left + gad->Box.Width/2)
        {
        if (p) { p--; rc = 1; }
        }
    else{
        p++;
        max = prop->LTot - prop->LVis;
        if (p > max) p = max;
        if (p != prop->LPos) rc = 1;
        }
    if (rc) UIK_OPr_ChangePropGadget( prop, -1, -1, p );
  END_ERROR:
    return( rc );
}

static void link_hprop( struct UIKObjPropGad *prop, struct AZObjZone *zo, int nil, struct AZObjView *vo )
{
    DisplayCursor();
    if (oview_ChangeViewPosition( vo, zo->TopLine, prop->LPos ))
        {
        UPDATE_STAT(AGP.FObj);
        }
}

/****************************************************************
 *
 *      Germe
 *
 ****************************************************************/

static int GenericAdd( struct AZObjView *vo, struct TagItem *taglist )
{
  WORD l, t, w, h, ph, zl, zt, zw, zh, bw, bh;
  struct AZViewInfo *vi = &AGP.FObj->VI;
  struct UIKBitMap **bmtable;
  ULONG elemask = AGP.Prefs->EleMask;
  struct TagItem tl[] = { { UIKTAG_OBJ_LeftTop,                  },
                          { UIKTAG_OBJ_WidthHeight,              },
                          { UIKTAG_OBJ_ParentRelative,      TRUE },
                          { UIKTAG_OBJ_Sz_AttachedFl_Left,  FALSE},
                          { UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE },
                          { UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE },
                          { UIKTAG_OBJ_Sz_AttachedFl_Bot,   FALSE},
                          { UIKTAG_OBJ_BoxAltAutomate,      NULL },
                          { UIKTAG_OBJ_BoxFl_Filled,        TRUE },
                          { UIKTAG_GEN_ActInactPens, UIKCOLS(UIKCOL_WHITE,UIKCOL_GREY,UIKCOL_GREY,UIKCOL_WHITE) },
                          { UIKTAG_OBJ_Title,                    },
                          { UIKTAG_OBJ_TitleFl_Image,       TRUE },
                          { TAG_END },
                          { UIKTAG_OBJ_BoxAutomate,         NULL },
                          { UIKTAG_OBJ_Pattern,             0    },
                          { UIKTAG_OBJ_PatternPower,        1    },
                          { UIKTAG_OBJProp_Vertical,        TRUE },
                          { TAG_END },
                        };

    bmtable = (vo->Obj.AutoRatio == 1) ? &ButBitMapTable[0] : &ButBitMapTable[6];
    zl = zt = 0;
    zw = vo->Obj.Box.Width + 1;
    bw = zw - (vi->VBorderWidth + 1);
    if (elemask & BUMF_RIGHTBORDER) zw = bw;
    zh = vo->Obj.Box.Height - 1; // -1 pour ligne noire en bas
    bh = zh - vi->HBorderHeight;
    if (elemask & BUMF_BOTTOMBORDER) zh = bh;

    l = bw; t = 0; w = vi->VBorderWidth; h = vi->HBorderHeight;
    tl[7].ti_Data = (ULONG) AutoDrawBox_Light;
    ph = bh;

    //----------
    if (elemask & BUMF_BUTUP)
        {
        tl[0].ti_Data = W2L(l,t);
        tl[1].ti_Data = W2L(w,vi->VButHeight);
        tl[10].ti_Data = (ULONG)bmtable[0];
        if (! (vo->VBut1 = UIK_AddObject( "UIKObj_Boolean", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->VBut1, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        t += vi->VButHeight;
        ph -= vi->VButHeight;
        }

    if (elemask & BUMF_UPDOWN1)
        {
        tl[0].ti_Data = W2L(l,t);
        tl[1].ti_Data = W2L(w,vi->VArrHeight);
        tl[10].ti_Data = (ULONG)bmtable[1];
        if (! (vo->VUpDown1 = UIK_AddObject( "UIKObj_Boolean", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->VUpDown1, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        if (! install_delay( vo->VUpDown1, vo )) goto END_ERROR;
        t += vi->VArrHeight;
        ph -= vi->VArrHeight;
        }

    if (elemask & BUMF_VERTPROP)
        {
        if (elemask & BUMF_UPDOWN2) ph -= vi->VArrHeight;
        if (elemask & BUMF_BUTDOWN) ph -= vi->VButHeight;
        //ph -= vi->HBorderHeight;  // Bouton Split
        tl[0].ti_Data = W2L(l-2,t-1);
        tl[1].ti_Data = W2L(w+4,ph+2);
        tl[6].ti_Data = (ULONG) TRUE;
        tl[7].ti_Data = (ULONG) PropAuto;
        tl[10].ti_Data = 0;
        tl[12].ti_Tag = TAG_IGNORE;
        tl[13].ti_Data = (ULONG) PropAuto;
        tl[14].ti_Data = (ULONG) PatProp;
        if (! (vo->VProp = UIK_AddObject( "UIKObj_PropLift", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->VProp, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        vo->VProp->UserULong1 = (ULONG) vo;
        t += ph;
        tl[12].ti_Tag = TAG_END;
        tl[7].ti_Data = (ULONG) AutoDrawBox_Light;
        tl[5].ti_Data = (ULONG) FALSE;
        }

    if (elemask & BUMF_UPDOWN2)
        {
        t = bh - vi->VArrHeight;
        if (elemask & BUMF_BUTDOWN) t -= vi->VButHeight;
        tl[0].ti_Data = W2L(l,t);
        tl[1].ti_Data = W2L(w,vi->VArrHeight);
        tl[10].ti_Data = (ULONG)bmtable[1];
        if (! (vo->VUpDown2 = UIK_AddObject( "UIKObj_Boolean", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->VUpDown2, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        if (! install_delay( vo->VUpDown2, vo )) goto END_ERROR;
        t += vi->VArrHeight;
        }

    if (elemask & BUMF_BUTDOWN)
        {
        t = bh - vi->VButHeight;
        tl[0].ti_Data = W2L(l,t);
        tl[1].ti_Data = W2L(w,vi->VButHeight);
        tl[10].ti_Data = (ULONG)bmtable[2];
        if (! (vo->VBut2 = UIK_AddObject( "UIKObj_Boolean", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->VBut2, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        t += vi->VButHeight;
        }

    //----------
    tl[3].ti_Data = (ULONG) TRUE;
    tl[4].ti_Data = (ULONG) FALSE;
    l = 0; t = bh;
    ph = bw;

    if (elemask & BUMF_BUTLEFT)
        {
        tl[0].ti_Data = W2L(l,t);
        tl[1].ti_Data = W2L(vi->HButWidth,h);
        tl[10].ti_Data = (ULONG)bmtable[3];
        if (! (vo->HBut1 = UIK_AddObject( "UIKObj_Boolean", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->HBut1, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        l += vi->HButWidth;
        ph -= vi->HButWidth;
        }

    if (elemask & BUMF_LEFTRIGHT1)
        {
        tl[0].ti_Data = W2L(l,t);
        tl[1].ti_Data = W2L(vi->HArrWidth,h);
        tl[10].ti_Data = (ULONG)bmtable[4];
        if (! (vo->HLeftRight1 = UIK_AddObject( "UIKObj_Boolean", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->HLeftRight1, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        if (! install_delay( vo->HLeftRight1, vo )) goto END_ERROR;
        l += vi->HArrWidth;
        ph -= vi->HArrWidth;
        }

    if (elemask & BUMF_HORIZPROP)
        {
        if (elemask & BUMF_LEFTRIGHT2) ph -= vi->HArrWidth;
        if (elemask & BUMF_BUTRIGHT) ph -= vi->HButWidth;
        //ph -= vi->VBorderWidth;  // Bouton Split
        tl[0].ti_Data = W2L(l-2,t-1);
        tl[1].ti_Data = W2L(ph+4,h+2);
        tl[4].ti_Data = (ULONG) TRUE;
        tl[7].ti_Data = (ULONG) PropAuto;
        tl[10].ti_Data = 0;
        tl[12].ti_Tag = TAG_IGNORE;
        tl[13].ti_Data = (ULONG) PropAuto;
        tl[14].ti_Data = (ULONG) PatProp;
        tl[16].ti_Data = FALSE;
        if (! (vo->HProp = UIK_AddObject( "UIKObj_PropLift", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->HProp, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        vo->HProp->UserULong1 = (ULONG) vo;
        l += ph;
        tl[12].ti_Tag = TAG_END;
        tl[7].ti_Data = (ULONG) AutoDrawBox_Light;
        tl[3].ti_Data = (ULONG) FALSE;
        }

    if (elemask & BUMF_LEFTRIGHT2)
        {
        l = bw - vi->HArrWidth;
        if (elemask & BUMF_BUTRIGHT) l -= vi->HButWidth;
        tl[0].ti_Data = W2L(l,t);
        tl[1].ti_Data = W2L(vi->HArrWidth,h);
        tl[10].ti_Data = (ULONG)bmtable[4];
        if (! (vo->HLeftRight2 = UIK_AddObject( "UIKObj_Boolean", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->HLeftRight2, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        if (! install_delay( vo->HLeftRight2, vo )) goto END_ERROR;
        l += vi->HArrWidth;
        }

    if (elemask & BUMF_BUTRIGHT)
        {
        l = bw - vi->HButWidth;
        tl[0].ti_Data = W2L(l,t);
        tl[1].ti_Data = W2L(vi->HButWidth,h);
        tl[10].ti_Data = (ULONG)bmtable[5];
        if (! (vo->HBut2 = UIK_AddObject( "UIKObj_Boolean", vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->HBut2, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        l += vi->HButWidth;
        }

    //----------
    if (elemask & (BUMF_BOTTOMBORDER|BUMF_RIGHTBORDER))
        {
        tl[0].ti_Data = W2L(bw,t);
        tl[1].ti_Data = W2L(w,h);
        if (! (vo->Split = UIK_AddPrivateObject( &ObjSplitRegGerm, vo, tl ))) goto END_ERROR;
        //if (! (UIK_AddVector( vo->Split, ofi_ActivateFObjKeyTank, IDCMP_GADGETUP, 0 ))) goto END_ERROR;
        }

/*
    if (elemask & BUMF_STATUSBAR)
        {
        if (! (vo->Stat = (struct UIKObj *) UIK_AddPrivateObjectTagsB( UIKBase, &ObjStatRegGerm, vo,
            UIKTAG_OBJ_ParentRelative,  TRUE,
            UIKTAG_OBJ_LeftTop,             W2L(zl,zt),
            UIKTAG_OBJ_WidthHeight,         W2L(zw,AGP.Prefs->ViewFontHeight+4),
            UIKTAG_OBJ_UserValue1,          vo,
            UIKTAG_OBJ_FontName,            AGP.Prefs->ViewFontName,
            UIKTAG_OBJ_FontHeight,          AGP.Prefs->ViewFontHeight,
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            TAG_END ))) goto END_ERROR;
        zt += (AGP.Prefs->ViewFontHeight + 4);
        zh -= (AGP.Prefs->ViewFontHeight + 4);
        }
*/

    if (! (vo->Zone = (struct AZObjZone *) UIK_AddPrivateObjectTagsB( UIKBase, &ObjZoneRegGerm, vo,
        UIKTAG_OBJ_ParentRelative,      TRUE,
        UIKTAG_OBJ_LeftTop,             W2L(zl,zt),
        UIKTAG_OBJ_WidthHeight,         W2L(zw,zh),
        UIKTAG_OBJ_FontName,            AGP.Prefs->ViewFontName,
        UIKTAG_OBJ_FontHeight,          AGP.Prefs->ViewFontHeight,
        UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
        UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
        UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
        UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
        TAG_END ))) goto END_ERROR;
    vo->Zone->Obj.ResizeVector( vo->Zone );

    if (vo->VProp)
        {
        if (vo->VBut1)    if (! UIK_Link( vo->VBut1, vo->VProp, (void*)link_upbut_2_vprop, MOUSEMOVE|GADGETDOWN|GADGETUP, (ULONG)vo )) goto END_ERROR;
        if (vo->VUpDown1) if (! UIK_Link( vo->VUpDown1,  vo->VProp, (void*)link_updown_2_vprop, MOUSEMOVE|GADGETDOWN|GADGETUP|INTUITICKS, (ULONG)vo )) goto END_ERROR;
        if (vo->VUpDown2) if (! UIK_Link( vo->VUpDown2,  vo->VProp, (void*)link_updown_2_vprop, MOUSEMOVE|GADGETDOWN|GADGETUP|INTUITICKS, (ULONG)vo )) goto END_ERROR;
        if (vo->VBut2)    if (! UIK_Link( vo->VBut2, vo->VProp, (void*)link_downbut_2_vprop, MOUSEMOVE|GADGETDOWN|GADGETUP, (ULONG)vo )) goto END_ERROR;
        //if (! UIK_Link( vo->VProp, vo->Zone,  (void*)gaddown_vprop, GADGETDOWN, (ULONG)vo )) goto END_ERROR;
        if (! UIK_Link( vo->VProp, vo->Zone,  (void*)link_vprop, MOUSEMOVE|GADGETDOWN|GADGETUP|INTUITICKS, (ULONG)vo )) goto END_ERROR;
        }

    if (vo->HProp)
        {
        if (vo->HBut1)       if (! UIK_Link( vo->HBut1, vo->HProp, (void*)link_leftbut_2_hprop, MOUSEMOVE|GADGETDOWN|GADGETUP, (ULONG)vo )) goto END_ERROR;
        if (vo->HLeftRight1) if (! UIK_Link( vo->HLeftRight1, vo->HProp, (void*)link_leftright_2_hprop, MOUSEMOVE|GADGETDOWN|GADGETUP|INTUITICKS, (ULONG)vo )) goto END_ERROR;
        if (vo->HLeftRight2) if (! UIK_Link( vo->HLeftRight2, vo->HProp, (void*)link_leftright_2_hprop, MOUSEMOVE|GADGETDOWN|GADGETUP|INTUITICKS, (ULONG)vo )) goto END_ERROR;
        if (vo->HBut2)       if (! UIK_Link( vo->HBut2, vo->HProp, (void*)link_rightbut_2_hprop, MOUSEMOVE|GADGETDOWN|GADGETUP, (ULONG)vo )) goto END_ERROR;
        //if (! UIK_Link( vo->HProp, vo->Zone,  (void*)gaddown_hprop, GADGETDOWN, (ULONG)vo )) goto END_ERROR;
        if (! UIK_Link( vo->HProp, vo->Zone,  (void*)link_hprop, MOUSEMOVE|GADGETDOWN|GADGETUP/*|INTUITICKS*/, (ULONG)vo )) goto END_ERROR;
        }

    return(1);
  END_ERROR:
    return(0);
}

static void GenericRefresh( struct UIKObj *vo )
{
  WORD y = vo->Box.Top+vo->Box.Height-1;

    SetAPen( vo->rp, UIK_GetColor( vo, UIKCOL_BLACK ) );
    Move( vo->rp, vo->Box.Left, y );
    Draw( vo->rp, vo->Box.Left+vo->Box.Width-1, y );
}

//-----------------------------------------------------------------

void oview_Display( struct AZObjView *vo, LONG startline, LONG nlines, LONG startcol, LONG ncols )
{
    ozone_Display( vo->Zone, startline, nlines, startcol, ncols );
    UpdateHProp( vo, 0 );
}

void oview_ReDisplay( struct AZObjView *vo )
{
    ozone_ReDisplay( vo->Zone );
    UpdateHProp( vo, 0 );
}

void oview_DisplayBlock( struct AZObjView *vo )
{
    ozone_DisplayBlock( vo->Zone );
    UpdateHProp( vo, 0 );
}

void oview_DisplayCmd( struct AZObjView *vo, struct AZDisplayCmd *dcmd )
{
    ozone_DisplayCmd( vo->Zone, dcmd );
    UpdateHProp( vo, 0 );
}

void oview_ViewTopLeft( struct AZObjView *vo )
{
    ozone_ZoneTopLeft( vo->Zone );
    //UpdateHProp( vo, 0 );
    //oview_DisplayStats( vo, 1 );
}

void oview_ResetProps( struct AZObjView *vo )
{
  struct AZObjZone *zo = vo->Zone;

    if (vo->VProp) UIK_OPr_ChangePropGadget( vo->VProp, AGP.FObj->Text.NodeNum+PLUSLINES, zo->Rows-1, zo->TopLine );
    if (vo->HProp) UIK_OPr_ChangePropGadget( vo->HProp, zo->HPropLineLen, zo->Cols-1, zo->TopCol );
}

/*
void oview_DisplayStats( struct AZObjView *vo, ULONG drawtitle )
{
    if (vo->Stat)
        {
//        LockLayer( 0, vo->Obj.rp->Layer );
        ostat_Display( vo->Stat, drawtitle );
//        UnlockLayer( vo->Obj.rp->Layer );
        }
}
*/

void oview_ScrollView( struct AZObjView *vo, ULONG vert, ULONG abslen, LONG len, LONG from )
{
  struct AZObjZone *zo = vo->Zone;

//    LockLayer( 0, vo->Obj.rp->Layer ); // car appelée aussi en interne
//    UIK_ClipObject( zo );
    if (vert) ozone_VScroll( zo, abslen, len, from );
    else ozone_HScroll( zo, abslen, len, from );
//    UIK_UnclipObject( zo );
    if (vert) UpdateHProp( vo, 0 );
//    UnlockLayer( vo->Obj.rp->Layer );  // car appelée aussi en interne
}

void oview_DisplayCurs( struct AZObjView *vo, ULONG line, ULONG col )
{
    ozone_DisplayCurs( vo->Zone, line, col );
}

ULONG oview_ChangeViewPosition( struct AZObjView *vo, LONG newtopline, LONG newtopcol )
{
  struct AZObjZone *zo = vo->Zone;
  LONG numlines = AGP.FObj->Text.NodeNum;
  LONG lastline = numlines + PLUSLINES;
  LONG lastcol, len;
  ULONG changed = 0;

    if (newtopline < 0) newtopline = 0;
    if (newtopline + zo->Rows-1 > lastline) newtopline = lastline - (zo->Rows-1);
    if (newtopline < 0) newtopline = 0;
    if (newtopline >= numlines) newtopline = numlines - 1;
    if (len = newtopline - zo->TopLine)
        {
        zo->TopLine = newtopline;
        oview_ScrollView( vo, 1, (len > 0 ? len : -len), len, newtopline );
        HGetMaxLen( zo, zo->TopLine );
        changed++;
        }

    if (! changed) HGetMaxLen( zo, zo->TopLine ); // à faire car si agrandi en horizontal...
    lastcol = zo->NewHPropLineLen; // à faire après HGetMaxLen()
    if (newtopcol < 0) newtopcol = 0;
    /* enlevé, car les vecteurs branchés sur les bordures entrainaient un scrolling si trop à droite
    if (newtopcol + zo->Cols > lastcol) newtopcol = lastcol - zo->Cols;
    if (newtopcol < 0) newtopcol = 0;
    */
    if (len = newtopcol - zo->TopCol)
        {
        zo->TopCol = newtopcol;
        oview_ScrollView( vo, 0, (len > 0 ? len : -len), len, newtopcol );
        changed++;
        }
    return( changed );
}

//----------- view uniquement, pas de func_...

void UpdateHProp( struct AZObjView *vo, ULONG draw )
{
  struct AZObjZone *zo = vo->Zone;

    if (zo && vo->HProp)
        {
        if (zo->NewHPropLineLen != zo->HPropLineLen)
            {
            zo->HPropLineLen = zo->NewHPropLineLen;
            if (draw) UIK_OPr_ChangePropGadget( vo->HProp, zo->HPropLineLen, zo->Cols-1, zo->TopCol );
            else AGP.FObj->Flags |= AZFILEF_DRAWPROPS;
            }
        }
}

void oview_DisplayView( struct AZObjView *vo )
{
    if (! vo->Zone) return;

//    LockLayer( 0, vo->Obj.rp->Layer );
    oview_ReDisplay( vo );
//    UnlockLayer( vo->Obj.rp->Layer );
}

void oview_ChangeView( struct AZObjView *vo, UWORD topchange, UWORD height )
{
  WORD offs = height - vo->Obj.Box.Height; // positif si agrandi
  struct UIKObj *o;
  struct Rectangle *r;

    if (topchange)
        {
        vo->Obj.Box.Top += -offs;
        vo->Obj.Box.Height = height; vo->Obj.ObjExt->InnerRect.MinY += -offs;

        if (o = vo->VBut1)   { o->Box.Top += -offs;   r = &o->ObjExt->InnerRect; r->MinY += -offs; r->MaxY += -offs; }
        if (o = vo->VUpDown1){ o->Box.Top += -offs;   r = &o->ObjExt->InnerRect; r->MinY += -offs; r->MaxY += -offs; }
        if (o = vo->VProp)   { o->Box.Top += -offs; }
        if (o = vo->VProp)   { o->Box.Height += offs; r = &o->ObjExt->InnerRect; r->MinY += -offs; }

        //if (o = vo->Stat)    { o->Box.Top += -offs; r = &o->ObjExt->InnerRect; r->MinY += -offs; r->MaxY += -offs; }
        if (o = vo->Zone)    { o->Box.Top += -offs; }
        if (o = vo->Zone)    { o->Box.Height += offs; r = &o->ObjExt->InnerRect; r->MinY += -offs; }
        }
    else // bottom change
        {
        vo->Obj.Box.Height = height; vo->Obj.ObjExt->InnerRect.MaxY += offs;

        if (o = vo->VProp)   { o->Box.Height += offs; r = &o->ObjExt->InnerRect; r->MaxY += offs; }
        if (o = vo->VUpDown2){ o->Box.Top += offs;    r = &o->ObjExt->InnerRect; r->MinY += offs; r->MaxY += offs; }
        if (o = vo->VBut2)   { o->Box.Top += offs;    r = &o->ObjExt->InnerRect; r->MinY += offs; r->MaxY += offs; }

        if (o = vo->HBut1)      { o->Box.Top += offs; r = &o->ObjExt->InnerRect; r->MinY += offs; r->MaxY += offs; }
        if (o = vo->HLeftRight1){ o->Box.Top += offs; r = &o->ObjExt->InnerRect; r->MinY += offs; r->MaxY += offs; }
        if (o = vo->HProp)      { o->Box.Top += offs; r = &o->ObjExt->InnerRect; r->MinY += offs; r->MaxY += offs; }
        if (o = vo->HLeftRight2){ o->Box.Top += offs; r = &o->ObjExt->InnerRect; r->MinY += offs; r->MaxY += offs; }
        if (o = vo->HBut2)      { o->Box.Top += offs; r = &o->ObjExt->InnerRect; r->MinY += offs; r->MaxY += offs; }

        if (o = vo->Split)   { o->Box.Top += offs;    r = &o->ObjExt->InnerRect; r->MinY += offs; r->MaxY += offs; }
        if (o = vo->Zone)    { o->Box.Height += offs; r = &o->ObjExt->InnerRect; r->MaxY += offs; }
        }

    CrossResize( vo, AGP.FObj->WO );
    UIK_Refresh( vo, -1 );
}

void oview_SetResizeFlags( struct AZObjView *vo, ULONG pos )
{
    switch (pos)
        {
        case -1:
            vo->Obj.ResizeFlags = UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            if (vo->VBut1   ) vo->VBut1   ->ResizeFlags = UO_ATTACHED_RIGHT;
            if (vo->VUpDown1) vo->VUpDown1->ResizeFlags = UO_ATTACHED_RIGHT;
            if (vo->VProp   ) vo->VProp   ->ResizeFlags = UO_ATTACHED_RIGHT;
            if (vo->VUpDown2) vo->VUpDown2->ResizeFlags = UO_ATTACHED_RIGHT;
            if (vo->VBut2   ) vo->VBut2   ->ResizeFlags = UO_ATTACHED_RIGHT;

            if (vo->HBut1      ) vo->HBut1       ->ResizeFlags = 0;
            if (vo->HLeftRight1) vo->HLeftRight1 ->ResizeFlags = 0;
            if (vo->HProp      ) vo->HProp       ->ResizeFlags = UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            if (vo->HLeftRight2) vo->HLeftRight2 ->ResizeFlags = UO_ATTACHED_RIGHT;
            if (vo->HBut2      ) vo->HBut2       ->ResizeFlags = UO_ATTACHED_RIGHT;

            if (vo->Split) vo->Split->ResizeFlags = UO_ATTACHED_RIGHT;
            //if (vo->Stat)  vo->Stat ->ResizeFlags = UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            vo->Zone->Obj.ResizeFlags = UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            break;
        case 0:
            vo->Obj.ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_TOP | UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            if (vo->VBut1   ) vo->VBut1   ->ResizeFlags = UO_ATTACHED_RIGHT;
            if (vo->VUpDown1) vo->VUpDown1->ResizeFlags = UO_ATTACHED_RIGHT;
            if (vo->VProp   ) vo->VProp   ->ResizeFlags = UO_ATTACHED_TOP | UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            if (vo->VUpDown2) vo->VUpDown2->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            if (vo->VBut2   ) vo->VBut2   ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;

            if (vo->HBut1      ) vo->HBut1       ->ResizeFlags = UO_ATTACHED_BOT;
            if (vo->HLeftRight1) vo->HLeftRight1 ->ResizeFlags = UO_ATTACHED_BOT;
            if (vo->HProp      ) vo->HProp       ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            if (vo->HLeftRight2) vo->HLeftRight2 ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            if (vo->HBut2      ) vo->HBut2       ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;

            if (vo->Split) vo->Split->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            //if (vo->Stat)  vo->Stat ->ResizeFlags = UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            vo->Zone->Obj.ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_TOP | UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            break;
        case 1:
            vo->Obj.ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            if (vo->VBut1   ) vo->VBut1   ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            if (vo->VUpDown1) vo->VUpDown1->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            if (vo->VProp   ) vo->VProp   ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            if (vo->VUpDown2) vo->VUpDown2->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            if (vo->VBut2   ) vo->VBut2   ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;

            if (vo->HBut1      ) vo->HBut1       ->ResizeFlags = UO_ATTACHED_BOT;
            if (vo->HLeftRight1) vo->HLeftRight1 ->ResizeFlags = UO_ATTACHED_BOT;
            if (vo->HProp      ) vo->HProp       ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            if (vo->HLeftRight2) vo->HLeftRight2 ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            if (vo->HBut2      ) vo->HBut2       ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;

            if (vo->Split) vo->Split->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_RIGHT;
            //if (vo->Stat)  vo->Stat ->ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            vo->Zone->Obj.ResizeFlags = UO_ATTACHED_BOT | UO_ATTACHED_LEFT | UO_ATTACHED_RIGHT;
            break;
        }
}

//-----------------------------------------------------------------

static struct UIKObjGerm __far Germ = {
    "AZURObj_View",
    GenericAdd,         /*int     (*AddVector)(); */
    NULL,               /*int     (*StartVector)(); */
    NULL,               /*void    (*StopVector)(); */
    NULL,               /*void    (*RemoveVector)(); */
    GenericRefresh,     /*void    (*RefreshVector)(); */
    NULL,               /*void    (*ResizeVector)(); */
    NULL,               /*void    (*ExceptActVector)(); */
    NULL,               /*void    (*ExceptInaVector)(); */
    NULL,               /*void    (*ExceptMovVector)(); */
    NULL,               /*void    (*ExceptKeyVector)(); */
    NULL,               /*int     (*StartAfterVector)(); */
    NULL,               /*int     (*SetGetValues)(); */
    FuncTable,          /*ULONG   *ExtentedVectors; */
    NULL,               /*ULONG   ExReserved[1]; */
    0,                  /*UWORD   MinWidth; */
    0,                  /*UWORD   MinHeight; */
    0,                  /*ULONG   DefaultWidthHeight; */
    0,                  /*UWORD   ObjId; */
    sizeof(struct AZObjView),   /*UWORD   ObjSize; */
    BOX_INNERZONE | BOX_SEMAPHORE | BOX_TRANSLATELEFTTOP, /* flags par defaut */
    };

struct UIKObjRegGerm __far ObjViewRegGerm = { &Germ, 0, 0 ,0 };
