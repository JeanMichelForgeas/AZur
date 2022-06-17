/****************************************************************
 *
 *      File:       funcs_reqs.c    fonctions de requesters
 *      Project:    AZur            editeur de texte
 *
 *      Created:    06/05/93        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <graphics/gfxbase.h>

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikgfx.h"
#include "uiki:uikdrawauto.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwfilebut.h"
#include "uiki:objwfontbut.h"
#include "uiki:objchecktoggle.h"
#include "uiki:objradioexclude.h"
#include "uiki:objstring.h"
#include "uiki:objlist.h"
#include "uiki:objscreen.h"
#include "uiki:objwindow.h"
#include "uiki:objreqstring.h"
#include "uiki:objwinfontsel.h"
#include "uiki:objdisplaymode.h"
#include "uiki:objtimer.h"
#include "uiki:objpalette.h"
#include "uiki:objarexxsim.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"
#include "lci:azur_protos.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"

#define ENV1_FR_AUTOSAVE  10
#define ENV1_ATS_ACTIF    11
#define ENV1_ATS_NOM      12
#define ENV1_ATS_REP      13
#define ENV1_ATS_SECS     14
#define ENV1_ATS_MODS     15
#define ENV1_FR_BACKUP    16
#define ENV1_BKP_ACTIF    17
#define ENV1_BKP_MEMENOM  18
#define ENV1_BKP_NOM      19
#define ENV1_BKP_REP      20
#define ENV1_BKP_ROT      21
#define ENV1_FR_MODE      22
#define ENV1_MOD_MINITEL  23
#define ENV1_MOD_LECTURE  24
#define ENV1_MOD_PRIORITE 25
#define ENV1_MOD_DELCLIP  26
#define ENV1_FR_TAILLES   27
#define ENV1_TAI_PUDDLE   28
#define ENV1_TAI_IOBUF    29
#define ENV1_TAI_CMDLINE  30
#define ENV1_TAI_UNDOSIZE 31
#define ENV1_EMPTY        32

#define WIN1_FR_POLICE     0
#define WIN1_POL_INTER     1
#define WIN1_POL_VUES      2
#define WIN1_FR_OUVERTURE  3
#define WIN1_OUV_DEVANT    4
#define WIN1_OUV_AZUR      5
#define WIN1_OUV_PLEIN     6
#define WIN1_OUV_X         7
#define WIN1_OUV_Y         8
#define WIN1_OUV_LARG      9
#define WIN1_OUV_HAUT     10
#define WIN1_FR_FENETRE   11
#define WIN1_FEN_STATUS   12
#define WIN1_FEN_COMMAND  13
#define WIN1_FR_HORIZ     14
#define WIN1_HOR_BUT1     15
#define WIN1_HOR_FLECHE1  16
#define WIN1_HOR_PROP     17
#define WIN1_HOR_FLECHE2  18
#define WIN1_HOR_BUT2     19
#define WIN1_FR_VERTICAL  20
#define WIN1_VER_BUT1     21
#define WIN1_VER_FLECHE1  22
#define WIN1_VER_PROP     23
#define WIN1_VER_FLECHE2  24
#define WIN1_VER_BUT2     25
#define WIN1_EMPTY        26


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern UBYTE *make_listptr( struct TextFont *font, WORD totwidth, WORD maxheight, WORD *plh );

extern void __asm StrHandler_Signed();

extern void __asm getgad_check( register __a0 struct AZLayout *al, register __d0 UBYTE num, register __a1 ULONG *pflags, register __d1 ULONG flag );
extern ULONG __asm getgad_radio( register __a0 struct AZLayout *al, register __d0 UBYTE num );
extern ULONG __asm getgad_strnum( register __a0 struct AZLayout *al, register __d0 UBYTE num );
extern void __asm setgad_active( register __a0 struct AZLayout *al, register __d0 UBYTE num, register __d1 ULONG bool );
extern void __asm setgad_disable( register __a0 struct AZLayout *al, register __d0 UBYTE num, register __d1 ULONG bool );
extern void __asm setgad_strnum( register __a0 struct AZLayout *al, register __d0 UBYTE num, register __d1 ULONG val );

extern struct AZLayout __far AZL_PrefsEdit1[];
extern struct AZLayout __far AZL_PrefsEdit2[];

extern struct AZLayout __far AZL_PrefsEnv1[];
extern struct AZLayout __far AZL_PrefsEnv2[];

extern struct AZLayout __far AZL_PrefsWin1[];


/****** Exported ************************************************/


/****** Statics *************************************************/

static struct TagItem __far IntuitionScreenTags[] = {
    SA_SharePens,   TRUE,  /* --> pour la roue des couleurs - Intuition V39 (3.0) */
    SA_Interleaved, TRUE,  /* --> pour l'exemple - Intuition V39 (3.0) */
    TAG_END
    };


/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

extern void ofi_SetIconColor();
extern void PatchLastDoc();

static void verif_fontname( STRPTR fn, ULONG max )
{
  ULONG ok=0, len=StrLen( fn );

    if (len > 5)
        if (StrNCCmp( &fn[len-5], ".font" ) == 0)
            ok = 1;

    if (!ok && (len + 5 < max))
        StrCat( fn, ".font" );
}

static void change_inter_font( struct AZurPrefs *op, struct AZurPrefs *np )
{
  struct OpenWindowStuff OWS;
  struct List oldlist;
  struct UIKObjWindow *icon;
  struct AZObjFile *fo = AGP.FObj;
  struct AZObjView *activeview;

  struct AZViewInfo oldvi;
  struct UIKObjWindow *oldwo;
  struct UIKObj *oldstat;
  struct UIKObj *oldcmd;
  struct UIKObj *oldopen;
  struct UIKObj *oldsave;
  struct UIKObj *oldmenu;
  struct UIKObj *gad;

    verif_fontname( np->FontName, 32 );

    if (fo && fo->WO && (icon = UIK_Iconify( fo->WO )))
        {
        oldlist = fo->ViewList;
        __builtin_memset( &fo->ViewList, 0, sizeof(struct List) );
        NewList( &fo->ViewList );
        fo->ActiveView = 0;
        oldvi   = fo->VI;
        oldwo   = fo->WO;
        oldstat = fo->Stat;
        oldcmd  = fo->CmdLine;
        oldopen = fo->OpenFS;
        oldsave = fo->SaveFS;
        oldmenu = fo->Menu;
        fo->Stat    = 0;
        fo->CmdLine = 0;
        fo->Menu    = 0;

        __builtin_memset( &OWS, 0, sizeof(struct OpenWindowStuff) );
        OWS.tag_lefttop         = W2L(fo->WO->NW.LeftEdge,fo->WO->NW.TopEdge);
        OWS.tag_widthheight     = W2L(fo->WO->NW.Width,fo->WO->NW.Height);
        OWS.tag_textwidthheight = -1;

        if (! ofi_OpenWindowStuff( &OWS, UIK_ObjTitle( fo->WO ), UIKTAG_OBJWindow_ScreenAddr, AGP.WO->ScrAdd )) goto END_ERROR;

        activeview = fo->ActiveView;
        fo->ActiveView->Zone->TopLine = activeview->Zone->TopLine;
        fo->ActiveView->Zone->TopCol = activeview->Zone->TopCol;
        if (! UIK_Start( fo->WO )) goto END_ERROR;

        if (! ofi_AddMenuObj()) goto END_ERROR;
        if (! ofi_AddFileSelectors( UIK_OSt_Get( fo->OpenFS->FS->StrPatGad, 0 ),
                                    UIK_OSt_Get( fo->SaveFS->FS->StrPatGad, 0 ) )) goto END_ERROR;

        if (gad = UIK_GetObject( fo->WO, 0, UIKID_ICONGAD ))
            if (! UIK_AddVector( gad, (void*)ofi_SetIconColor, IDCMP_GADGETDOWN, 0 )) goto END_ERROR;
        if (! UIK_AddVector( fo->WO, (void*)PatchLastDoc, IDCMP_ACTIVEWINDOW, (ULONG)fo )) goto END_ERROR;

        ofi_ActivateFObjKeyTank();

        ofi_RemoveAllViewNodes( &oldlist );
        UIK_Remove( oldwo );

        PoolFree( AGP.SearchLayout );      AGP.SearchLayout = 0;
        PoolFree( AGP.Edit1Layout );       AGP.Edit1Layout = 0;
        PoolFree( AGP.Edit2Layout );       AGP.Edit2Layout = 0;
        PoolFree( AGP.Env1Layout );        AGP.Env1Layout = 0;
        PoolFree( AGP.Env2Layout );        AGP.Env2Layout = 0;
        PoolFree( AGP.WindowLayout );      AGP.WindowLayout = 0;
        PoolFree( AGP.ReplaceLayout );     AGP.ReplaceLayout = 0;
        PoolFree( AGP.SearchPrefsLayout ); AGP.SearchPrefsLayout = 0;
        PoolFree( AGP.PaletteLayout );     AGP.PaletteLayout = 0;
        }
    return;

  END_ERROR:
    ofi_RemoveAllViewNodes( &fo->ViewList );
    UIK_Remove( OWS.newwo );

    fo->ViewList = oldlist;
    fo->ActiveView = activeview;
    fo->WO = AGP.WO = oldwo;
    fo->VI = oldvi;
    fo->Stat = oldstat;
    fo->CmdLine = oldcmd;
    fo->OpenFS = (APTR)oldopen;
    fo->SaveFS = (APTR)oldsave;
    fo->Menu   = (APTR)oldmenu;
    UIK_Uniconify( icon );
    *np = *op;
}

static void change_view_font( struct AZurPrefs *op, struct AZurPrefs *np )
{
  struct Node *node;
  struct AZObjView *view, *act=0, *corr=0;
  struct AZObjFile *fo=AGP.FObj;
  WORD left, top, width, height, fontdiff_status=0, fontdiff_cmdl=0, fontdiff;
  struct TagItem tl[] = { { UIKTAG_OBJ_LeftTop,     },
                          { UIKTAG_OBJ_WidthHeight, },
                          { TAG_END } };

    verif_fontname( np->ViewFontName, 32 );

    if (fo)
        {
        if (fo->Stat) fontdiff_status = np->ViewFontHeight - op->ViewFontHeight;
        if (fo->CmdLine) fontdiff_cmdl = np->ViewFontHeight - op->ViewFontHeight;
        fontdiff = fontdiff_status + fontdiff_cmdl;

        ofi_ComputeViewBox( &fo->VI, fo->WO, np->ViewFontHeight );
        ofi_VerifViewSize( &fo->VI, np->ViewFontHeight, (fo->WO->LeftBorderWidth + fo->WO->RightBorderWidth) );
        for (node=fo->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
            {
            view = (struct AZObjView *)node->ln_Name;
            if (view->Obj.Box.Height < fo->VI.MinHeight)
                {
                func_ReqShowText( UIK_LangString( AGP.UIK, TEXT_FONTTOOHIGH ) );
                goto END_ERROR;
                }
            if (view->Obj.Box.Height - fontdiff >= fo->VI.MinHeight) corr = view;
            }
        if (! corr)
            {
            func_ReqShowText( UIK_LangString( AGP.UIK, TEXT_FONTTOOHIGH ) );
            goto END_ERROR;
            }

        func_CursorErase();

        if (fo->Stat /*&& fontdiff_status*/)
            {
            struct UIKObj *stat;
            left = fo->Stat->Box.Left - fo->WO->LeftBorderWidth;
            top = fo->Stat->Box.Top - fo->WO->TopBorderHeight;
            if (stat = (struct UIKObj *) ofi_AddStat( W2L(left,top),
                                                      W2L(fo->Stat->Box.Width,fo->Stat->Box.Height+fontdiff_status),
                                                      np->ViewFontName,
                                                      np->ViewFontHeight ))
                {
                UIK_Remove( fo->Stat );
                fo->Stat = stat;
                UIK_Start( fo->Stat );
                }
            }

        for (node=fo->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
            {
            view = (struct AZObjView *)node->ln_Name;
            left = view->Obj.Box.Left - fo->WO->LeftBorderWidth;
            top = view->Obj.Box.Top - fo->WO->TopBorderHeight + fontdiff_status;
            width = view->Obj.Box.Width;
            height = view->Obj.Box.Height;
            if (view == corr) height -= fontdiff;
            tl[0].ti_Data = W2L(left,top);
            tl[1].ti_Data = W2L(width,height);

            if (! (view = (struct AZObjView *) ofi_AddViewObj( fo, tl ))) break;
            view->Zone->TopLine = ((struct AZObjView *)node->ln_Name)->Zone->TopLine;
            if (! UIK_Start( view )) { UIK_Remove( view ); break; }

            if ((APTR)fo->ActiveView == (APTR)node->ln_Name) act = view;
            UIK_Remove( (struct UIKObj *)node->ln_Name );
            node->ln_Name = (APTR) view;
            }
        ofi_LinkViews( fo );
        ofi_SetActiveView( fo, act ? act : (APTR) fo->ViewList.lh_Head->ln_Name );

        if (fo->CmdLine /*&& fontdiff_cmdl*/)
            {
            struct UIKObj *cmdl;
            left = fo->CmdLine->Box.Left - fo->WO->LeftBorderWidth;
            top = fo->CmdLine->Box.Top - fo->WO->TopBorderHeight - fontdiff_cmdl;
            if (cmdl = (struct UIKObj *) ofi_AddCmdLine( W2L(left,top),
                                                         W2L(fo->CmdLine->Box.Width,fo->CmdLine->Box.Height+fontdiff_cmdl),
                                                         np->ViewFontName,
                                                         np->ViewFontHeight ))
                {
                UIK_Remove( fo->CmdLine );
                fo->CmdLine = cmdl;
                UIK_Start( fo->CmdLine );
                }
            }

        ofi_ActualizeViewLimits( fo );
        func_CursorDraw();
        }
    return;
  END_ERROR:
    StrCpy( np->ViewFontName, op->ViewFontName );
    np->ViewFontHeight = op->ViewFontHeight;
}

void ActualizePrefs( ULONG display )
{
  struct AZurPrefs *np=AGP.Prefs, *op=AGP.TmpPrefs;
  struct AZObjFile *fo = AGP.FObj;
  struct Screen *sc;

    if (&AGP != AGP.AZMast)
        {
        if (op->AutsIntSec != np->AutsIntSec)
            {
            if (op->AutsIntSec == 0) UIK_Call( AGP.Timer, UIKFUNC_Timer_Start, 0, 0 );
            if (np->AutsIntSec != 0) UIK_Call( AGP.Timer, UIKFUNC_Timer_SetInterval, np->AutsIntSec, 0 );
#ifndef AZUR_DEMO
            else UIK_Call( AGP.Timer, UIKFUNC_Timer_Stop, 0, 0 );
#endif
            }
        if ((op->Flags1 & AZP_AUTOSAVE) != TST_AUTOSAVE)
            {
            if (TST_AUTOSAVE) UIK_Call( AGP.Timer, UIKFUNC_Timer_Start, 0, 0 );
#ifndef AZUR_DEMO
            else UIK_Call( AGP.Timer, UIKFUNC_Timer_Stop, 0, 0 );
#endif
            }
        }

    if (op->Priority != np->Priority)
        {
        SetTaskPri( AGP.Process, np->Priority );
        }

    if (fo)
        {
        fo->LeftMargin = np->LeftMargin;
        fo->RightMargin = np->RightMargin;
        if (display)
            func_CursorPosReal( fo->Line, fo->Col, 1, 0 ); // remet le curseur dans les marges ou actualise TextPrep()
        }

    if (AGP.AZMast->ScreenObj && (sc = AGP.AZMast->ScreenObj->Obj.sc))
        LoadRGB4( &sc->ViewPort, np->ColorTable, 1 << AGP.AZMast->Prefs->ScreenDepth );

    if ((np->EleMask != op->EleMask) || StrNCCmp( op->FontName, np->FontName ) || (op->FontHeight != np->FontHeight)) change_inter_font( op, np );
    else if (StrNCCmp( op->ViewFontName, np->ViewFontName ) || (op->ViewFontHeight != np->ViewFontHeight)) change_view_font( op, np );

    /*
    if ((op->Flags2 & AZP_LEAVEREXCASE) != (np->Flags2 & AZP_LEAVEREXCASE))
        {
        UBYTE *portname = (UBYTE*) UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_ARexxName, 0, 0 );
        UBYTE *oldprefsportname = (AGP.AZMast->Process == AGP.Process) ? op->MastPortName : op->PortName;
        strncpy( portname, oldprefsportname, StrLen(portname) );
        if (! TST_LEAVEREXCASE) UIK_StrToUpper( portname );
        }
    */

    if (fo)
        {
        if (fo->WO) ofi_SetColorMask();
        if (display) func_RefreshZones();
        UPDATE_STAT(fo);
        }

    if ((AGP.AZMast->Prefs->Flags2 & AZP_APPICON) && !(np->Flags2 & AZP_APPICON))
        { AGP.AZMast->Prefs->Flags2 &= ~AZP_APPICON; func_StopAppIcon(); }
    else if (!(AGP.AZMast->Prefs->Flags2 & AZP_APPICON) && (np->Flags2 & AZP_APPICON))
        { AGP.AZMast->Prefs->Flags2 |= AZP_APPICON; func_StartAppIcon(); }
}

//-------------------------------------------------------------------------------

static void add_button( struct UIKObjReqString *rs, struct AZLayout *al, UBYTE num, ULONG title, void (*func)() )
{
  struct UIKObj *ok;

    if (ok = UIK_GetObject( rs->Req, 0, UIKID_OKGAD ))
        {
        WORD h = ok->Box.Height;
        UBYTE *ptr = UIK_LangString( AGP.UIK, title );
        WORD w = UIK_StrWidth( ok->ObjExt->Font, ptr, StrLen(ptr) ) + 20;
        WORD l = (rs->Req->Box.Width - w) / 2 - ok->Box.Left;

        if (ok = UIK_AddObjectTagsB( UIKBase, "UIKObj_Boolean", ok,
                UIKTAG_OBJ_ParentRelative,  TRUE,
                UIKTAG_OBJ_LeftTop,         W2L(l,0),
                UIKTAG_OBJ_WidthHeight,     W2L(w,h),
                UIKTAG_OBJ_Title,           title,
                UIKTAG_OBJ_ActInactPens,    UIKCOLS(UIKCOL_WHITE,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_WHITE),
                UIKTAG_OBJ_TitleUnderscore, '_',
                UIKTAG_OBJ_BoxFl_Filled,    TRUE,
                UIKTAG_OBJ_BoxFl_NoPush,    TRUE,
                TAG_END ))
            if (! UIK_AddVector( ok, (void*)func, GADGETUP, (ULONG)rs ))
                UIK_Remove( ok );
            else al[num].Obj = ok;
        }
}

void verif_maxval( struct UIKObj *obj )
{
  ULONG rega4, val, max;

    rega4 = __builtin_getreg( 12 );
    __builtin_putreg( 12, obj->UserULong1 ); // A4 pointe sur la structure des variables

    if (max = obj->UserULong2)
        {
        val = Str2Num( UIK_OSt_Get( obj, 0 ) );
        if (val > max)
            {
            UBYTE buf[20]; sprintf( buf, "%ld", max );
            UIK_OSt_Set( obj, buf, StrLen(buf) );
            UIK_Refresh( obj, 1 );
            }
        }

    __builtin_putreg( 12, rega4 ); // restore A4
}

void verif_maxval_signed( struct UIKObj *obj )
{
  ULONG rega4;
  LONG val, max;

    rega4 = __builtin_getreg( 12 );
    __builtin_putreg( 12, obj->UserULong1 ); // A4 pointe sur la structure des variables

    if (max = obj->UserULong2)
        {
        val = Str2Num( UIK_OSt_Get( obj, 0 ) );
        if (val > max)
            {
            UBYTE buf[20]; sprintf( buf, "%ld", max );
            UIK_OSt_Set( obj, buf, StrLen(buf) );
            UIK_Refresh( obj, 1 );
            }
        if (val < -(max+1))
            {
            UBYTE buf[20]; sprintf( buf, "%ld", -(max+1) );
            UIK_OSt_Set( obj, buf, StrLen(buf) );
            UIK_Refresh( obj, 1 );
            }
        }

    __builtin_putreg( 12, rega4 ); // restore A4
}

//===============================================================================

static UBYTE *prep_edit2( struct TextFont *font, APTR userdata )
{
  struct AZLayout *al, *al2, *al3;
  WORD totwidth, maxheight;
  WORD listheight;
  UBYTE *mem;

    //------ Placement des frames
    if (! AGP.Edit2Layout)
        {
        if (! (AGP.Edit2Layout = (struct AZLayout *) LayoutPrepare( font, AZL_PrefsEdit2, userdata ))) return(0);
        LayoutWidth( font, AGP.Edit2Layout );
        }
    al = &AGP.Edit2Layout[0];   // Couleurs
    al2 = &AGP.Edit2Layout[9];  // Blocs
    al3 = &AGP.Edit2Layout[18]; // Plis

    //------ Création de place dans le ReqString
    al->Top = al2->Top = al3->Top = font->tf_YSize / 2;
    totwidth = al->Width + INTERFRAME_WIDTH + al2->Width + INTERFRAME_WIDTH + al3->Width;
    maxheight = MAX(al->TotH,al2->TotH);
    maxheight = MAX(maxheight,al3->TotH);

    if (! (mem = make_listptr( font, totwidth, maxheight, &listheight ))) return(0);

    //------ Ajustement du Top des frames
    if ((maxheight = (listheight - maxheight) / 2) > 0)
        { al->Top += maxheight; al2->Top += maxheight; al3->Top += maxheight; }

    return( mem );
}

static ULONG AddReqEdit2( struct AZRequest *ar ) // appelée par func_IntSmartRequest(), WO existe
{
  struct UIKObjReqString *rs = (struct UIKObjReqString *)ar->Obj;
  struct AZLayout *al = AGP.Edit2Layout;
  struct AZurPrefs *p = AGP.Prefs;
  WORD diff;

    //------ Calcul des abscisses
    diff = (rs->Text->Obj.Box.Width - (al[0].TotW + al[9].TotW + al[18].TotW));
    //al[0].Left = 0;
    al[9].Left = al[0].TotW + diff/2;
    al[18].Left = al[9].Left + al[9].TotW + diff/2;

    //------ Mise des flags des gadgets
    setgad_active( al, 10, p->BlockMode == BLKMODE_CHAR);
    setgad_active( al, 11, p->BlockMode == BLKMODE_WORD);
    setgad_active( al, 12, p->BlockMode == BLKMODE_LINE);
    setgad_active( al, 13, p->BlockMode == BLKMODE_RECT);
    setgad_active( al, 14, p->BlockMode == BLKMODE_FULL);
    setgad_active( al, 17, TST_DEFAULTBLM );

    setgad_active( al, 21, p->FDI[0].OnFlags & FSF_UNDERLINED);
    setgad_active( al, 22, p->FDI[0].OnFlags & FSF_BOLD);
    setgad_active( al, 23, p->FDI[0].OnFlags & FSF_ITALIC);

    //------ Ajout des objets
    if (! LayoutAddObjects( rs, AGP.Edit2Layout, &AGP, 0 )) goto END_ERROR;

    //------ Contenu des Strings
    setgad_strnum( al, 1, p->TextPen );
    setgad_strnum( al, 2, p->BackPen );
    setgad_strnum( al, 3, p->BlockTPen );
    setgad_strnum( al, 4, p->BlockBPen );
    setgad_strnum( al, 5, p->CursTextPen );
    setgad_strnum( al, 6, p->CursBackPen );
    setgad_strnum( al, 7, p->CursBlockTPen );
    setgad_strnum( al, 8, p->CursBlockBPen );

    setgad_strnum( al, 15, p->BlockXLim );
    setgad_strnum( al, 16, p->BlockYLim );

    setgad_strnum( al, 19, p->FDI[0].OnAPen );
    setgad_strnum( al, 20, p->FDI[0].OnBPen );

    return(1);

  END_ERROR:
    //AGP.WorkFlags &= ~WRF_PRFREQEDIT2;
    UIK_ObjEnable( AGP.Edit1Layout[35].Obj, 1 );
    return(0);
}

//-------------------------------------

static WORD edit1_maxheight( struct AZLayout *al )
{
  WORD maxboxheight=0, maxa, maxb, maxc;

    maxa = al[0].TotH + al[6].TotH;
    maxboxheight = maxa;
    maxb = al[10].TotH + al[15].TotH;
    if (maxboxheight < maxb) maxboxheight = maxb;
    maxc = al[19].TotH + al[31].TotH;
    if (maxboxheight < maxc) maxboxheight = maxc;

    return( maxboxheight );
}

static UBYTE *prep_edit1( struct TextFont *font, APTR userdata )
{
  struct AZLayout *al, *al2;
  WORD totwidth, maxheight=0;
  WORD listheight;
  UBYTE *mem;

    //------ Placement des frames
    if (! (al = AGP.Edit1Layout))
        {
        if (! (AGP.Edit1Layout = (struct AZLayout *) LayoutPrepare( font, AZL_PrefsEdit1, userdata ))) return(0);

        al = &AGP.Edit1Layout[0];  // Ecriture
        al2 = &AGP.Edit1Layout[6]; // Indentation
        al->TotW = al2->TotW = al->Width = al2->Width = MAX(al->Width,al2->Width);

        al = &AGP.Edit1Layout[10];  // Tabulation
        al2 = &AGP.Edit1Layout[15]; // Clignotement
        al->TotW = al2->TotW = al->Width = al2->Width = MAX(al->Width,al2->Width);

        al = &AGP.Edit1Layout[19];  // Affichage
        al2 = &AGP.Edit1Layout[31]; // Marges
        al->TotW = al2->TotW = al->Width = al2->Width = MAX(al->Width,al2->Width);

        LayoutWidth( font, AGP.Edit1Layout );
        al = AGP.Edit1Layout;
        }

    //------ Création de place dans le ReqString
    al[0].Top = al[10].Top = al[19].Top = font->tf_YSize / 2;
    totwidth = al[0].Width + INTERFRAME_WIDTH + al[10].Width + INTERFRAME_WIDTH + al[19].Width;
    maxheight = edit1_maxheight( al ) + font->tf_YSize / 2; // plus l'espace inter box

    al[6].Top  = /*al[0].Top  +*/ maxheight - al[6].Height ;
    al[15].Top = /*al[10].Top +*/ maxheight - al[15].Height;
    al[31].Top = /*al[19].Top +*/ maxheight - al[31].Height;

    if (! (mem = make_listptr( font, totwidth, maxheight, &listheight ))) return(0);

    //------ Ajustement du Top des frames
    if ((maxheight = (listheight - maxheight) / 2) > 0)
        { al[0].Top += maxheight; al[6].Top += maxheight; al[10].Top += maxheight; al[15].Top += maxheight; al[19].Top += maxheight; al[31].Top += maxheight; }

    return( mem );
}

static void StartEdit2End( struct AZRequest *ar )
{
    if (ar->Button == REQBUT_OK)
        {
        struct AZurPrefs *p = AGP.Prefs;
        struct AZLayout *al = AGP.Edit2Layout; // est créé par prep_edit2 ci-avant
        ULONG val=0;

        CopyMem( AGP.Prefs, AGP.TmpPrefs, sizeof(struct AZurPrefs) );

        p->TextPen       = getgad_strnum( al, 1 );
        p->BackPen       = getgad_strnum( al, 2 );
        p->BlockTPen     = getgad_strnum( al, 3 );
        p->BlockBPen     = getgad_strnum( al, 4 );
        p->CursTextPen   = getgad_strnum( al, 5 );
        p->CursBackPen   = getgad_strnum( al, 6 );
        p->CursBlockTPen = getgad_strnum( al, 7 );
        p->CursBlockBPen = getgad_strnum( al, 8 );

        if (getgad_radio( al, 10 )) p->BlockMode = BLKMODE_CHAR;
        if (getgad_radio( al, 11 )) p->BlockMode = BLKMODE_WORD;
        if (getgad_radio( al, 12 )) p->BlockMode = BLKMODE_LINE;
        if (getgad_radio( al, 13 )) p->BlockMode = BLKMODE_RECT;
        if (getgad_radio( al, 14 )) p->BlockMode = BLKMODE_FULL;
        p->BlockXLim = getgad_strnum( al, 15 );
        p->BlockYLim = getgad_strnum( al, 16 );
        getgad_check( al, 17, &p->Flags1, AZP_DEFAULTBLM );

        p->FDI[0].OnAPen = getgad_strnum( al,19 );
        p->FDI[0].OnBPen = getgad_strnum( al,20 );
        getgad_check( al, 21, &val, FSF_UNDERLINED );
        getgad_check( al, 22, &val, FSF_BOLD );
        getgad_check( al, 23, &val, FSF_ITALIC );
        p->FDI[0].OnFlags = val;

        ActualizePrefs(1);
        }
    //AGP.WorkFlags &= ~WRF_PRFREQEDIT2;
    UIK_ObjEnable( AGP.Edit1Layout[35].Obj, 1 );
}

static void StartEdit2( int nil, struct UIKObjReqString *rs )
{
  UBYTE *mem;

    //if (AGP.WorkFlags & WRF_PRFREQEDIT2) return; // la window existe car lancé depuis Edit1
    if (! (mem = prep_edit2( rs->Obj.ObjExt->Font, 0 ))) return;
    //AGP.WorkFlags |= WRF_PRFREQEDIT2;
    UIK_ObjDisable( AGP.Edit1Layout[35].Obj, 1 );

    func_IntSmartRequest( rs->Req, TEXT_Edit1_TitleReq,0, 0,mem, 0,0, 0,0, -1,0,-1, AddReqEdit2, ARF_WINDOW|ARF_ASYNC /*|ARF_ACTIVATE|ARF_RETQUIET*/, StartEdit2End, 0 );
    UIK_MemSpFree( mem );
}

static ULONG AddReqEdit1( struct AZRequest *ar ) // appelée par func_IntSmartRequest(), WO existe
{
  struct UIKObjReqString *rs = (struct UIKObjReqString *)ar->Obj;
  struct AZLayout *al = AGP.Edit1Layout;
  struct AZurPrefs *p = AGP.Prefs;
  WORD diff;

    //------ Calcul des abscisses des frames
    diff = (rs->Text->Obj.Box.Width - (al[0].TotW + al[10].TotW + al[19].TotW)) / 2;
    //al[0].Left = al[6].Left = 0;
    al[10].Left = al[15].Left = al[0].TotW + diff;
    al[19].Left = al[31].Left = al[10].Left + al[10].TotW + diff;

    //------ Mise des flags des gadgets
    setgad_active( al, 1, TST_CHARDELBLOCK );
    setgad_active( al, 2, TST_OVERLAY );
    setgad_active( al, 3, TST_STRIPCHANGE );
    setgad_active( al, 4, TST_AUTOFORMAT );
    setgad_active( al, 5, TST_CORRECTCASE );    setgad_disable( al, 5, 1 );

    setgad_active( al, 7, p->IndentType == NOINDENT);
    setgad_active( al, 8, p->IndentType == INDENT_STAY);
    setgad_active( al, 9, p->IndentType == INDENT_TTX);

    setgad_active( al, 11, TST_REALTAB );       setgad_disable( al, 11, 1 );
    setgad_active( al, 12, TST_TAB2SPACE );
    setgad_active( al, 13, TST_SPACE2TAB );

    setgad_active( al, 16, TST_BLINKON );

    setgad_active( al, 20, TST_TABDISPLAYS );   setgad_disable( al, 20, 1 );
    setgad_active( al, 22, TST_TABDISPLAYE );   setgad_disable( al, 22, 1 );
    setgad_active( al, 24, TST_SPACEDISPLAY );  setgad_disable( al, 24, 1 );
    setgad_active( al, 26, TST_SHOWEOL );
    setgad_active( al, 28, TST_SHOWEOF );
    setgad_active( al, 30, TST_NUMBERLINES );   setgad_disable( al, 30, 1 );

    setgad_active( al, 33, TST_USEMARGIN );     //

    setgad_disable( al, 21, 1 );  // faire les Disable avant les AddObject
    setgad_disable( al, 23, 1 );
    setgad_disable( al, 25, 1 );

    //------ Ajout des objets ------
    if (! LayoutAddObjects( rs, AGP.Edit1Layout, &AGP, 0 )) goto END_ERROR;

    //------ Contenu des Strings
    setgad_strnum( al, 14, p->TabLen );

    setgad_strnum( al, 17, p->BlinkOnDelay );
    setgad_strnum( al, 18, p->BlinkOffDelay );

    setgad_strnum( al, 21, p->CharTabS );
    *((UBYTE*) UIK_ObjTitle( al[21].Obj )) = p->CharTabS;
    setgad_strnum( al, 23, p->CharTabE );
    *((UBYTE*) UIK_ObjTitle( al[23].Obj )) = p->CharTabE;
    setgad_strnum( al, 25, p->CharSpace );
    *((UBYTE*) UIK_ObjTitle( al[25].Obj )) = p->CharSpace;
    setgad_strnum( al, 27, p->CharEOL );
    *((UBYTE*) UIK_ObjTitle( al[27].Obj )) = p->CharEOL;
    setgad_strnum( al, 29, p->CharEOF );
    *((UBYTE*) UIK_ObjTitle( al[29].Obj )) = p->CharEOF;

    setgad_strnum( al, 32, p->LeftMargin ); // nombre d'espaces à insérer
    setgad_strnum( al, 34, p->RightMargin );

    //------ Bouton pour Req 2
    add_button( rs, al, 35, TEXT_Edit1_CoulBlocs, StartEdit2 );
    /*
    if (ok = UIK_GetObject( rs->Req, NULL, UIKID_BOOLGAD ))
      if (ok = UIK_GetObject( rs->Req, ok, UIKID_BOOLGAD ))
        {
        WORD h = ok->Box.Height;
        UBYTE *ptr = UIK_LangString( AGP.UIK, TEXT_Edit1_CoulBlocs );
        WORD w = UIK_StrWidth( ok->ObjExt->Font, ptr, StrLen(ptr) ) + 20;
        WORD l = (rs->Req->Box.Width - w) / 2 - ok->Box.Left;

        if (ok = UIK_AddObjectTagsB( UIKBase, "UIKObj_Boolean", ok,
                UIKTAG_OBJ_ParentRelative,  TRUE,
                UIKTAG_OBJ_LeftTop,         W2L(l,0),
                UIKTAG_OBJ_WidthHeight,     W2L(w,h),
                UIKTAG_OBJ_Title,           TEXT_Edit1_CoulBlocs,
                UIKTAG_OBJ_ActInactPens,    UIKCOLS(UIKCOL_WHITE,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_WHITE),
                UIKTAG_OBJ_TitleUnderscore, '_',
                UIKTAG_OBJ_BoxFl_Filled,    TRUE,
                UIKTAG_OBJ_BoxFl_NoPush,    TRUE,
                TAG_END ))
            if (! UIK_AddVector( ok, (void*)StartEdit2, GADGETUP, (ULONG)rs ))
                UIK_Remove( ok );
            else al[35].Obj = ok;
        }
    */

    return(1);

  END_ERROR:
    return(0);
}

void show_edit1char( struct UIKObj *obj )
{
  ULONG rega4;
  struct RastPort *rp;
  WORD x;
  UBYTE *title;

    rega4 = __builtin_getreg( 12 );
    __builtin_putreg( 12, obj->UserULong1 ); // A4 pointe sur la structure des variables

    verif_maxval( obj );

    rp = UIK_GetRastPort( obj );
    title = UIK_ObjTitle( obj );
    x = obj->Box.Left - 6 - UIK_CharWidth( obj->ObjExt->Font, *title );
    SetAPen( rp, UIK_GetColor( obj, UIKCOL_GREY ) );
    SetDrMd( rp, JAM2 );
    RectFill( rp, x, obj->Box.Top, obj->Box.Left - 1, obj->Box.Top + obj->Box.Height - 1 );

    *title = (UBYTE) Str2Num( UIK_OSt_Get( obj, 0 ) );
    UIK_Refresh( obj, 1 );

    __builtin_putreg( 12, rega4 ); // restore A4
}

void check_left_margin( struct UIKObj *obj )
{
  struct AZLayout *al;
  LONG left, right;
  ULONG rega4;

    rega4 = __builtin_getreg( 12 );
    __builtin_putreg( 12, obj->UserULong1 ); // A4 pointe sur la structure des variables

    al = AGP.Edit1Layout;
    left = getgad_strnum( al, 32 );
    right = getgad_strnum( al, 34 );
    if (left < 0) setgad_strnum( al, 32, left = 0 );
    if (left > right-1) setgad_strnum( al, 32, right-1 );

    __builtin_putreg( 12, rega4 ); // restore A4
}
void check_right_margin( struct UIKObj *obj )
{
  struct AZLayout *al;
  LONG left, right;
  ULONG rega4;

    rega4 = __builtin_getreg( 12 );
    __builtin_putreg( 12, obj->UserULong1 ); // A4 pointe sur la structure des variables

    al = AGP.Edit1Layout;
    left = getgad_strnum( al, 32 );
    right = getgad_strnum( al, 34 );
    if (right < 1) setgad_strnum( al, 34, right = 1 );
    if (right < left+1) setgad_strnum( al, 34, left+1 );

    __builtin_putreg( 12, rega4 ); // restore A4
}

static void ReqEditPrefsEnd( struct AZRequest *ar )
{
    if (ar->Button == REQBUT_OK)
        {
        struct AZurPrefs *p = AGP.Prefs;
        struct AZLayout *al = AGP.Edit2Layout; // est créé par prep_edit2 ci-avant
        ULONG *pf1 = &p->Flags1, *pf2 = &p->Flags2;

        CopyMem( AGP.Prefs, AGP.TmpPrefs, sizeof(struct AZurPrefs) );

        al = AGP.Edit1Layout; // est créé par prep_edit1 ci-avant

        getgad_check( al, 1, pf1, AZP_CHARDELBLOCK );
        getgad_check( al, 2, pf1, AZP_OVERLAY );
        getgad_check( al, 3, pf2, AZP_STRIPCHANGE );
        getgad_check( al, 4, pf2, AZP_AUTOFORMAT );
        getgad_check( al, 5, pf2, AZP_CORRECTCASE );

        if (getgad_radio( al, 7 )) p->IndentType = NOINDENT;
        if (getgad_radio( al, 8 )) p->IndentType = INDENT_STAY;
        if (getgad_radio( al, 9 )) p->IndentType = INDENT_TTX;

        getgad_check( al, 11, pf2, AZP_REALTAB );
        getgad_check( al, 12, pf2, AZP_TAB2SPACE );
        getgad_check( al, 13, pf2, AZP_SPACE2TAB );
        p->TabLen = getgad_strnum( al, 14 );

        getgad_check( al, 16, pf2, AZP_BLINKON );
        p->BlinkOnDelay = getgad_strnum( al, 17 );
        p->BlinkOffDelay = getgad_strnum( al, 18 );

        getgad_check( al, 20, pf2, AZP_TABDISPLAYS );
        p->CharTabS = getgad_strnum( al, 21 );
        getgad_check( al, 22, pf2, AZP_TABDISPLAYE );
        p->CharTabE = getgad_strnum( al, 23 );
        getgad_check( al, 24, pf2, AZP_SPACEDISPLAY );
        p->CharSpace = getgad_strnum( al, 25 );
        getgad_check( al, 26, pf2, AZP_SHOWEOL );
        p->CharEOL = getgad_strnum( al, 27 );
        getgad_check( al, 28, pf2, AZP_SHOWEOF );
        p->CharEOF = getgad_strnum( al, 29 );
        getgad_check( al, 30, pf2, AZP_NUMBERLINES );

        p->LeftMargin = getgad_strnum( al, 32 );
        getgad_check( al, 33, pf2, AZP_USEMARGIN );
        p->RightMargin = getgad_strnum( al, 34 );
        }
    func_ReqClose( 0, ar );
    if (ar->Button == REQBUT_OK) ActualizePrefs(1);
}

ULONG func_ReqOpenEditPrefs( struct AZRequest *ar )
{
  struct UIKObj *wo;
  struct TextFont *font;
  UBYTE *mem=0;

    if (! (wo = AGP.WO)) goto END_ERROR;
    font = wo->ObjExt->Font;
    if (! (mem = prep_edit1( font, 0 ))) goto END_ERROR;

    func_IntSmartRequest( 0, TEXT_Edit1_TitleReq,0, 0,mem, 0,0, 0,0, -1,0,-1, AddReqEdit1, ARF_WINDOW|ARF_ASYNC/*|ARF_ACTIVATE|ARF_RETQUIET*/, ReqEditPrefsEnd, ar );
    UIK_MemSpFree( mem );
    return( ar->ErrMsg );

  END_ERROR:
    UIK_MemSpFree( mem );
    ar->Button = REQBUT_CANCEL;
    ar->ErrMsg = TEXT_NOMEMORY;
    return( TEXT_NOMEMORY );
}

//==========================================================================================

static UBYTE *prep_env2( struct TextFont *font, APTR userdata )
{
  struct AZLayout *al, *al2;
  WORD totwidth, maxheight;
  WORD listheight;
  UBYTE *mem;

    //------ Placement des frames
    if (! AGP.Env2Layout)
        {
        if (! (al = AGP.Env2Layout = (struct AZLayout *) LayoutPrepare( font, AZL_PrefsEnv2, userdata ))) return(0);

        al2 = &AGP.Env2Layout[7]; // Fichiers
        if (al->TotW > al2->TotW) al2->TotW = al2->Width = al->TotW;
        else al->TotW = al->Width = al2->TotW;

        LayoutWidth( font, AGP.Env2Layout );
        }
    al = &AGP.Env2Layout[0];  // ARexx
    al2 = &AGP.Env2Layout[7]; // Fichiers

    //------ Création de place dans le ReqString
    maxheight = al->TotH + al2->TotH + font->tf_YSize / 2;
    al->Top = font->tf_YSize / 2;
    al2->Top = maxheight - al2->Height;
    totwidth = al->TotW;

    if (! (mem = make_listptr( font, totwidth, maxheight, &listheight ))) return(0);

    //------ Ajustement du Top des frames
    if ((maxheight = (listheight - maxheight) / 2) > 0)
        { al->Top += maxheight; al2->Top += maxheight; }

    return( mem );
}

static ULONG AddReqEnv2( struct AZRequest *ar ) // appelée par func_IntSmartRequest(), WO existe
{
  struct UIKObjReqString *rs = (struct UIKObjReqString *)ar->Obj;
  struct AZLayout *al = AGP.Env2Layout;
  struct AZurPrefs *p = AGP.Prefs;
  WORD diff;

    //------ Calcul des abscisses et largeurs
    if ((diff = rs->Text->Obj.Box.Width - al[0].TotW) > 0)
        {
        al[0].Width = (al[0].TotW += diff);
        al[7].Width = (al[7].TotW += diff);
        LayoutWidth( rs->Obj.ObjExt->Font, al );
        }

    //------ Mise des flags des gadgets
    setgad_active( al, 1, TST_REXXNAMESEP);
    setgad_active( al, 3, TST_REXXNAMENUM);
    setgad_active( al, 4, TST_LEAVEREXCASE);

    //------ Ajout des objets
    if (! LayoutAddObjects( rs, AGP.Env2Layout, &AGP, 0 )) goto END_ERROR;

    //------ Contenu des Strings
    UIK_OSt_Set( (struct UIKObjStrGad *) al[2].Obj, &p->CharRexxDot, 1 );
    UIK_OSt_Set( (struct UIKObjStrGad *) al[5].Obj, p->MastPortName, StrLen(p->MastPortName) );
    UIK_OSt_Set( (struct UIKObjStrGad *) al[6].Obj, p->PortName, StrLen(p->PortName) );

    UIK_OSt_Set( (struct UIKObjStrGad *) al[8].Obj, p->PatOpen, StrLen(p->PatOpen) );
    UIK_OSt_Set( (struct UIKObjStrGad *) ((struct UIKObjWFileBut *)al[9].Obj)->OS, p->ParmFile, StrLen(p->ParmFile) );
    UIK_OSt_Set( (struct UIKObjStrGad *) al[10].Obj, p->ARexxConsoleDesc, StrLen(p->ARexxConsoleDesc) );
    UIK_OSt_Set( (struct UIKObjStrGad *) al[11].Obj, p->DOSConsoleDesc, StrLen(p->DOSConsoleDesc) );

    return(1);

  END_ERROR:
    //AGP.WorkFlags &= ~WRF_PRFREQENV2;
    UIK_ObjEnable( AGP.Env1Layout[ENV1_EMPTY].Obj, 1 );
    return(0);
}

//-------------------------------------

static WORD env1_maxheight( struct AZLayout *al )
{
  WORD maxboxheight=0, maxa, maxb, maxc;

    maxa = al[0].TotH + al[4].TotH;
    maxboxheight = maxa;
    maxb = al[ENV1_FR_AUTOSAVE].TotH + al[ENV1_FR_MODE].TotH;
    if (maxboxheight < maxb) maxboxheight = maxb;
    maxc = al[ENV1_FR_BACKUP].TotH + al[ENV1_FR_TAILLES].TotH;
    if (maxboxheight < maxc) maxboxheight = maxc;

    return( maxboxheight );
}

static UBYTE *prep_env1( struct TextFont *font, APTR userdata )
{
  struct AZLayout *al, *al2;
  WORD totwidth, maxheight=0;
  WORD listheight;
  UBYTE *mem;

    //------ Placement des frames
    if (! (al = AGP.Env1Layout))
        {
        if (! (AGP.Env1Layout = (struct AZLayout *) LayoutPrepare( font, AZL_PrefsEnv1, userdata ))) return(0);

        al = &AGP.Env1Layout[0];
        al2 = &AGP.Env1Layout[4];
        al->TotW = al2->TotW = al->Width = al2->Width = MAX(al->Width,al2->Width);

        al = &AGP.Env1Layout[ENV1_FR_AUTOSAVE];
        al2 = &AGP.Env1Layout[ENV1_FR_MODE];
        al->TotW = al2->TotW = al->Width = al2->Width = MAX(al->Width,al2->Width);

        al = &AGP.Env1Layout[ENV1_FR_BACKUP];
        al2 = &AGP.Env1Layout[ENV1_FR_TAILLES];
        al->TotW = al2->TotW = al->Width = al2->Width = MAX(al->Width,al2->Width);

        LayoutWidth( font, AGP.Env1Layout );
        al = AGP.Env1Layout;
        }

    //------ Création de place dans le ReqString
    al[0].Top = al[ENV1_FR_AUTOSAVE].Top = al[ENV1_FR_BACKUP].Top = font->tf_YSize / 2;
    totwidth = al[0].Width + INTERFRAME_WIDTH + al[ENV1_FR_AUTOSAVE].Width + INTERFRAME_WIDTH + al[ENV1_FR_BACKUP].Width;
    maxheight = env1_maxheight( al ) + font->tf_YSize / 2; // plus l'espace entre les deux box

    al[4].Top = /*al[0].Top +*/ maxheight - al[4].Height ;
    al[ENV1_FR_MODE].Top = /*al[ENV1_FR_AUTOSAVE].Top +*/ maxheight - al[ENV1_FR_MODE].Height;
    al[ENV1_FR_TAILLES].Top = /*al[ENV1_FR_BACKUP].Top +*/ maxheight - al[ENV1_FR_TAILLES].Height;

    if (! (mem = make_listptr( font, totwidth, maxheight, &listheight ))) return(0);

    //------ Ajustement du Top des frames
    if ((maxheight = (listheight - maxheight) / 2) > 0)
        { al[0].Top += maxheight; al[4].Top += maxheight; al[ENV1_FR_AUTOSAVE].Top += maxheight; al[ENV1_FR_MODE].Top += maxheight; al[ENV1_FR_BACKUP].Top += maxheight; al[ENV1_FR_TAILLES].Top += maxheight; }

    return( mem );
}

static void StartEnv2End( struct AZRequest *ar )
{
    if (ar->Button == REQBUT_OK)
        {
        struct AZurPrefs *p = AGP.Prefs;
        struct AZLayout *al = AGP.Env2Layout; // est créé par prep_env2 ci-avant

        CopyMem( AGP.Prefs, AGP.TmpPrefs, sizeof(struct AZurPrefs) );

        getgad_check( al, 1, &p->Flags1, AZP_REXXNAMESEP );
        getgad_check( al, 3, &p->Flags1, AZP_REXXNAMENUM );
        getgad_check( al, 4, &p->Flags2, AZP_LEAVEREXCASE );

        p->CharRexxDot = *(UIK_OSt_Get( (struct UIKObjStrGad *) al[2].Obj, 0 ));
        StrCpy( p->MastPortName, UIK_OSt_Get( (struct UIKObjStrGad *) al[5].Obj, 0 ) );
        StrCpy( p->PortName, UIK_OSt_Get( (struct UIKObjStrGad *) al[6].Obj, 0 ) );

        StrCpy( p->PatOpen, UIK_OSt_Get( (struct UIKObjStrGad *) al[8].Obj, 0 ) );
        StcCpy( p->ParmFile, UIK_OSt_Get( (struct UIKObjStrGad *) ((struct UIKObjWFileBut *)al[9].Obj)->OS, 0 ), 64 );
        StrCpy( p->ARexxConsoleDesc, UIK_OSt_Get( (struct UIKObjStrGad *) al[10].Obj, 0 ) );
        StrCpy( p->DOSConsoleDesc, UIK_OSt_Get( (struct UIKObjStrGad *) al[11].Obj, 0 ) );

        ActualizePrefs(1);
        }
    //AGP.WorkFlags &= ~WRF_PRFREQENV2;
    UIK_ObjEnable( AGP.Env1Layout[ENV1_EMPTY].Obj, 1 );
}

static void StartEnv2( int nil, struct UIKObjReqString *rs )
{
  UBYTE *mem;

    //if (AGP.WorkFlags & WRF_PRFREQENV2) return; // la window existe car lancé depuis Env1
    if (! (mem = prep_env2( rs->Obj.ObjExt->Font, 0 ))) return;
    //AGP.WorkFlags |= WRF_PRFREQENV2;
    UIK_ObjDisable( AGP.Env1Layout[ENV1_EMPTY].Obj, 1 );

    func_IntSmartRequest( rs->Req, TEXT_Env1_TitleReq,0, 0,mem, 0,0, 0,0, -1,0,-1, AddReqEnv2, ARF_WINDOW|ARF_ASYNC /*|ARF_ACTIVATE|ARF_RETQUIET*/, StartEnv2End, 0 );
    UIK_MemSpFree( mem );
}

void endisable_bkpname( struct UIKObj *obj )
{
  struct AZLayout *al = AGP.Env1Layout;

    if (obj->Flags & BOX_OVER)
        UIK_ObjDisable( al[ENV1_BKP_NOM].Obj, 1 );
    else
        UIK_ObjEnable( al[ENV1_BKP_NOM].Obj, 1 );
}

static void able_autosauve( ULONG dis )
{
  struct AZLayout *al = AGP.Env1Layout;

    if (dis)
        {
        al[ENV1_ATS_NOM].Flags |= AZLF_BOX_DISABLE;
        al[ENV1_ATS_REP].Flags |= AZLF_BOX_DISABLE;
        al[ENV1_ATS_SECS].Flags |= AZLF_BOX_DISABLE;
        al[ENV1_ATS_MODS].Flags |= AZLF_BOX_DISABLE;
        }
    else{
        al[ENV1_ATS_NOM].Flags &= ~AZLF_BOX_DISABLE;
        al[ENV1_ATS_REP].Flags &= ~AZLF_BOX_DISABLE;
        al[ENV1_ATS_SECS].Flags &= ~AZLF_BOX_DISABLE;
        al[ENV1_ATS_MODS].Flags &= ~AZLF_BOX_DISABLE;
        }
}

void endisable_autosauve( struct UIKObj *obj )
{
  struct AZLayout *al = AGP.Env1Layout;

    if (obj->Flags & BOX_OVER)
        {
        UIK_ObjDisable( al[ENV1_ATS_NOM].Obj, 1 );
        UIK_ObjDisable( al[ENV1_ATS_REP].Obj, -1 );
        UIK_ObjDisable( al[ENV1_ATS_SECS].Obj, 1 );
        UIK_ObjDisable( al[ENV1_ATS_MODS].Obj, 1 );
        }
    else{
        UIK_ObjEnable( al[ENV1_ATS_NOM].Obj, 1 );
        UIK_ObjEnable( al[ENV1_ATS_REP].Obj, -1 );
        UIK_ObjEnable( al[ENV1_ATS_SECS].Obj, 1 );
        UIK_ObjEnable( al[ENV1_ATS_MODS].Obj, 1 );
        }
}

static void able_backup( ULONG dis )
{
  struct AZLayout *al = AGP.Env1Layout;

    if (dis)
        {
        al[ENV1_BKP_MEMENOM].Flags |= AZLF_BOX_DISABLE;
        al[ENV1_BKP_NOM].Flags |= AZLF_BOX_DISABLE;
        al[ENV1_BKP_REP].Flags |= AZLF_BOX_DISABLE;
        al[ENV1_BKP_ROT].Flags |= AZLF_BOX_DISABLE;
        }
    else{
        al[ENV1_BKP_MEMENOM].Flags &= ~AZLF_BOX_DISABLE;
        if (al[ENV1_BKP_MEMENOM].Flags & AZLF_BOX_ACTIVE) al[ENV1_BKP_NOM].Flags &= ~AZLF_BOX_DISABLE;
        al[ENV1_BKP_REP].Flags &= ~AZLF_BOX_DISABLE;
        al[ENV1_BKP_ROT].Flags &= ~AZLF_BOX_DISABLE;
        }
}

void endisable_backup( struct UIKObj *obj )
{
  struct AZLayout *al = AGP.Env1Layout;

    if (obj->Flags & BOX_OVER)
        {
        UIK_ObjDisable( al[ENV1_BKP_MEMENOM].Obj, 1 );
        UIK_ObjDisable( al[ENV1_BKP_NOM].Obj, 1 );
        UIK_ObjDisable( al[ENV1_BKP_REP].Obj, -1 );
        UIK_ObjDisable( al[ENV1_BKP_ROT].Obj, 1 );
        }
    else{
        UIK_ObjEnable( al[ENV1_BKP_MEMENOM].Obj, 1 );
        if (! (al[ENV1_BKP_MEMENOM].Obj->Flags & BOX_OVER)) UIK_ObjEnable( al[ENV1_BKP_NOM].Obj, 1 );
        UIK_ObjEnable( al[ENV1_BKP_REP].Obj, -1 );
        UIK_ObjEnable( al[ENV1_BKP_ROT].Obj, 1 );
        }
}

void Env1_SaveLFCR()
{
    UIK_Call( AGP.Env1Layout[7].Obj, UIKFUNC_Check_Unselect, 0, 0 );
}
void Env1_SaveLFCRLF()
{
    UIK_Call( AGP.Env1Layout[6].Obj, UIKFUNC_Check_Unselect, 0, 0 );
}

static ULONG AddReqEnv1( struct AZRequest *ar ) // appelée par func_IntSmartRequest(), WO existe
{
  struct UIKObjReqString *rs = (struct UIKObjReqString *)ar->Obj;
  struct AZLayout *al = AGP.Env1Layout;
  struct AZurPrefs *p = AGP.Prefs;
  WORD diff;
  UBYTE buf[32];

    //------ Calcul des abscisses des frames
    diff = (rs->Text->Obj.Box.Width - (al[0].TotW + al[ENV1_FR_AUTOSAVE].TotW + al[ENV1_FR_BACKUP].TotW)) / 2;
    //al[0].Left = al[4].Left = 0;
    al[ENV1_FR_AUTOSAVE].Left = al[ENV1_FR_MODE].Left = al[0].TotW + diff;
    al[ENV1_FR_BACKUP].Left = al[ENV1_FR_TAILLES].Left = al[ENV1_FR_AUTOSAVE].Left + al[ENV1_FR_AUTOSAVE].TotW + diff;

    //------ Mise des flags des gadgets
    setgad_active( al, 1, TST_STRIPLOAD);
    setgad_active( al, 2, TST_LOADCR2LF|TST_LOADCRLF2LF);
    setgad_active( al, 3, TST_LOADICON);

    setgad_active( al, 5, TST_STRIPSAVE);
    setgad_active( al, 6, TST_SAVELF2CR);
    setgad_active( al, 7, TST_SAVELF2CRLF);
    setgad_active( al, 8, TST_SAVEICON);

    setgad_active( al, ENV1_ATS_ACTIF, TST_AUTOSAVE);

    setgad_active( al, ENV1_BKP_ACTIF, TST_KEEPBKP);
    setgad_active( al, ENV1_BKP_MEMENOM, TST_BKPFILENAME);

    setgad_active( al, ENV1_MOD_MINITEL, TST_MINITEL);
    setgad_active( al, ENV1_MOD_LECTURE, TST_READONLY);

    // faire les disable des strings avant les addobject
    setgad_disable( al, ENV1_BKP_NOM, TST_BKPFILENAME ? 0 : 1 );
    setgad_disable( al, ENV1_TAI_UNDOSIZE, 1 );

    able_autosauve( al[ENV1_ATS_ACTIF].Flags & AZLF_BOX_ACTIVE ? 0 : 1 );
    able_backup( al[ENV1_BKP_ACTIF].Flags & AZLF_BOX_ACTIVE ? 0 : 1 );

    //------ Ajout des objets ------
    if (! LayoutAddObjects( rs, AGP.Env1Layout, &AGP, 0 )) goto END_ERROR;

    //------ Contenu des Strings
    UIK_OSt_Set( (struct UIKObjStrGad *) al[9].Obj, p->IconTool, StrLen(p->IconTool) );

    setgad_strnum( al, ENV1_ATS_SECS, p->AutsIntSec );
    setgad_strnum( al, ENV1_ATS_MODS, p->AutsIntMod );
    UIK_OSt_Set( (struct UIKObjStrGad *) ((struct UIKObjWFileBut *)al[ENV1_ATS_REP].Obj)->OS, p->AutsDirName, StrLen(p->AutsDirName) );
    UIK_OSt_Set( (struct UIKObjStrGad *) al[ENV1_ATS_NOM].Obj, p->AutsName, StrLen(p->AutsName) );

    setgad_strnum( al, ENV1_BKP_ROT, p->BackupRot );
    UIK_OSt_Set( (struct UIKObjStrGad *) ((struct UIKObjWFileBut *)al[ENV1_BKP_REP].Obj)->OS, p->BackupDirName, StrLen(p->BackupDirName) );
    UIK_OSt_Set( (struct UIKObjStrGad *) al[ENV1_BKP_NOM].Obj, p->BackupName, StrLen(p->BackupName) );

    sprintf( buf, "%ld", p->Priority ); UIK_OSt_Set( al[ENV1_MOD_PRIORITE].Obj, buf, StrLen(buf) );
    ((struct UIKObjStrGad *)al[ENV1_MOD_PRIORITE].Obj)->UserHook = (ULONG)StrHandler_Signed;

    setgad_strnum( al, ENV1_MOD_DELCLIP, p->DelClip );

    setgad_strnum( al, ENV1_TAI_PUDDLE, p->PuddleSize);
    setgad_strnum( al, ENV1_TAI_IOBUF, p->IOBufLength);
    setgad_strnum( al, ENV1_TAI_CMDLINE, p->HistCmdLineMax);
    //setgad_strnum( al, ENV1_TAI_MAXUNDO, p->UndoMaxNum);
    setgad_strnum( al, ENV1_TAI_UNDOSIZE, p->UndoMaxSize);

    //------ Bouton pour Req 2
    add_button( rs, al, ENV1_EMPTY, TEXT_Env1_TitleButton, StartEnv2 );

    return(1);

  END_ERROR:
    return(0);
}

static void ReqEnvPrefsEnd( struct AZRequest *ar )
{
    if (ar->Button == REQBUT_OK)
        {
        struct AZurPrefs *p = AGP.Prefs;
        struct AZLayout *al;
        ULONG *pf1 = &p->Flags1, *pf2 = &p->Flags2;

        CopyMem( AGP.Prefs, AGP.TmpPrefs, sizeof(struct AZurPrefs) );

        al = AGP.Env1Layout; // est créé par prep_env1 ci-avant

        getgad_check( al, 1, pf2, AZP_STRIPLOAD);
        getgad_check( al, 2, pf2, AZP_LOADCR2LF|AZP_LOADCRLF2LF);
        getgad_check( al, 3, pf2, AZP_LOADICON);

        getgad_check( al, 5, pf2, AZP_STRIPSAVE);
        getgad_check( al, 6, pf2, AZP_SAVELF2CR);
        getgad_check( al, 7, pf2, AZP_SAVELF2CRLF);
        getgad_check( al, 8, pf1, AZP_SAVEICON);

        getgad_check( al, ENV1_ATS_ACTIF, pf1, AZP_AUTOSAVE);

        getgad_check( al, ENV1_BKP_ACTIF, pf1, AZP_KEEPBKP);
        getgad_check( al, ENV1_BKP_MEMENOM, pf1, AZP_BKPFILENAME);

        getgad_check( al, ENV1_MOD_MINITEL, pf1, AZP_MINITEL);
        getgad_check( al, ENV1_MOD_LECTURE, pf1, AZP_READONLY);

        StrCpy( p->IconTool, UIK_OSt_Get( (struct UIKObjStrGad *) al[9].Obj, 0 ) );

        StcCpy( p->AutsDirName, UIK_OSt_Get( (struct UIKObjStrGad *) ((struct UIKObjWFileBut *)al[ENV1_ATS_REP].Obj)->OS, 0 ), 64 );
        StcCpy( p->AutsName, UIK_OSt_Get( (struct UIKObjStrGad *) al[ENV1_ATS_NOM].Obj, 0 ), 16 );
        p->AutsIntSec = getgad_strnum( al, ENV1_ATS_SECS );
        p->AutsIntMod = getgad_strnum( al, ENV1_ATS_MODS );

        StcCpy( p->BackupDirName, UIK_OSt_Get( (struct UIKObjStrGad *) ((struct UIKObjWFileBut *)al[ENV1_BKP_REP].Obj)->OS, 0 ), 64 );
        StcCpy( p->BackupName, UIK_OSt_Get( (struct UIKObjStrGad *) al[ENV1_BKP_NOM].Obj, 0 ), 16 );
        p->BackupRot = getgad_strnum( al, ENV1_BKP_ROT );

        p->Priority = getgad_strnum( al, ENV1_MOD_PRIORITE );
        p->DelClip = getgad_strnum( al, ENV1_MOD_DELCLIP );

        p->PuddleSize = getgad_strnum( al, ENV1_TAI_PUDDLE );
        p->IOBufLength = getgad_strnum( al, ENV1_TAI_IOBUF );
        p->HistCmdLineMax = getgad_strnum( al, ENV1_TAI_CMDLINE );
        //p->UndoMaxNum = getgad_strnum( al, ENV1_TAI_MAXUNDO );
        p->UndoMaxSize = getgad_strnum( al, ENV1_TAI_UNDOSIZE);
        }
    func_ReqClose( 0, ar );
    if (ar->Button == REQBUT_OK) ActualizePrefs(1);
}

ULONG func_ReqOpenEnvPrefs( struct AZRequest *ar )
{
  struct UIKObj *wo;
  struct TextFont *font;
  UBYTE *mem=0;

    if (! (wo = AGP.WO)) goto END_ERROR;
    font = wo->ObjExt->Font;
    if (! (mem = prep_env1( font, 0 ))) goto END_ERROR;

    func_IntSmartRequest( 0, TEXT_Env1_TitleReq,0, 0,mem, 0,0, 0,0, -1,0,-1, AddReqEnv1, ARF_WINDOW|ARF_ASYNC /*|ARF_ACTIVATE|ARF_RETQUIET*/, ReqEnvPrefsEnd, ar );
    UIK_MemSpFree( mem );
    return( ar->ErrMsg );

  END_ERROR:
    UIK_MemSpFree( mem );
    ar->Button = REQBUT_CANCEL;
    ar->ErrMsg = TEXT_NOMEMORY;
    return( TEXT_NOMEMORY );
}

//==========================================================================================

static void color_ok( struct UIKObjPalette *op )
{
  struct Screen *sc;

    UIK_Remove( (struct UIKObj *) op->Obj.UserULong1 );
    CopyMem( AGP.TmpPrefs->ColorTable, AGP.Prefs->ColorTable, MAX_UIKCOLORS*2 );
    if (AGP.AZMast->ScreenObj && (sc = AGP.AZMast->ScreenObj->Obj.sc))
        LoadRGB4( &sc->ViewPort, AGP.Prefs->ColorTable, 1 << AGP.Prefs->ScreenDepth );
    UIK_ObjEnable( AGP.WindowLayout[WIN1_EMPTY].Obj, 1 );
}

static void color_cancel( struct UIKObjPalette *op )
{
    UIK_Remove( (struct UIKObj *)op->Obj.UserULong1 );
    UIK_ObjEnable( AGP.WindowLayout[WIN1_EMPTY].Obj, 1 );
}

static void color_restore( struct UIKObjPalette *op )
{
    CopyMem( AGP.Prefs->ColorTable, AGP.TmpPrefs->ColorTable, MAX_UIKCOLORS*2 );
}

static void color_modify( struct UIKObjPalette *op, struct UIKColorDef *ucd )
{
    AGP.TmpPrefs->ColorTable[ucd->ColorNum] = ucd->B12Color;
}

static void ScreenColors( int nil, struct UIKObjReqString *rs /*struct UIKObj *obj*/ )
{
  struct UIKObj *os, *op;
  ULONG nowheel=FALSE;
  struct IntuiMessage *imsg;

    UIK_ObjDisable( AGP.WindowLayout[WIN1_EMPTY].Obj, 1 );
    CopyMem( AGP.Prefs, AGP.TmpPrefs, sizeof(struct AZurPrefs) );

    if (imsg = rs->Obj.UIK->IntuiMsg)
        if (imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT|IEQUALIFIER_LALT|IEQUALIFIER_RALT))
            nowheel = TRUE;
    if (GfxBase->LibNode.lib_Version < 39)
            nowheel = TRUE;

    if (! (os = UIK_AddObjectTagsB( UIKBase, "UIKObj_Screen", rs->Req,
            UIKTAG_OBJ_WidthHeight,         W2L(350,200),
            UIKTAG_OBJ_Title,               TEXT_PaletteTitle,
            UIKTAG_OBJ_FontName,            AGP.Prefs->FontName,
            UIKTAG_OBJ_FontHeight,          AGP.Prefs->FontHeight,
            UIKTAG_ObjScreen_ViewModes,     AGP.Prefs->ScreenModeId,
            UIKTAG_ObjScreen_ColorTable,    AGP.Prefs->ColorTable,
            UIKTAG_OBJScreen_RedirectColors,&AGP.Prefs->ColRegs,
            UIKTAG_OBJScreenFl_Public,      TRUE,
            UIKTAG_OBJScreenFl_MaxDepth,    TRUE,
            UIKTAG_OBJScreenFl_AutoCenter,  TRUE,
            UIKTAG_OBJScreen_IntuiTags,     (ULONG)IntuitionScreenTags,
            TAG_END ))) goto END_ERROR;

    if (! (op = UIK_AddObjectTagsB( UIKBase, "UIKObj_Palette", os,
            UIKTAG_OBJ_LeftTop,             W2L(150,15),
            UIKTAG_OBJ_Title,               TEXT_PaletteTitle,
            UIKTAG_OBJ_AltTitle,            TEXT_PaletteTitle,
            UIKTAG_OBJ_UserValue1,          os,
            UIKTAG_Palette_FirstTitleNum,   TEXT_Win1__Palette,
            UIKTAG_Palette_OKFunc,          (ULONG)color_ok,
            UIKTAG_Palette_CancelFunc,      (ULONG)color_cancel,
            UIKTAG_Palette_RestoreFunc,     (ULONG)color_restore,
            UIKTAG_Palette_ModifyFunc,      (ULONG)color_modify,
            UIKTAG_Palette_NoColorWheel,    nowheel,
            UIKTAG_Palette_EditColorNum,    MIN(32,1L << AGP.Prefs->ScreenDepth),
            UIKTAG_PaletteFl_DefaultScreen, FALSE,
            TAG_END ))) goto END_ERROR;

    if (UIK_Start( os ))
        return;

  END_ERROR:
    UIK_Remove( os );
    UIK_ObjEnable( AGP.WindowLayout[WIN1_EMPTY].Obj, 1 );
}

//------------------------------------------------------------

static UBYTE *prep_win1( struct TextFont *font, APTR userdata )
{
  struct AZLayout *al;
  WORD maxwidth, sub, maxheight=0;
  WORD listheight;
  UBYTE *mem;

    //------ Placement des frames
    if (! (al = AGP.WindowLayout))
        {
        if (! (AGP.WindowLayout = (struct AZLayout *) LayoutPrepare( font, AZL_PrefsWin1, userdata ))) return(0);
        al = AGP.WindowLayout;

        maxwidth = MAX(al[WIN1_FR_OUVERTURE].Width,al[WIN1_FR_FENETRE].Width);
        maxwidth = MAX(maxwidth,al[WIN1_FR_HORIZ].Width);

        sub = INTERFRAME_WIDTH + al[WIN1_FR_VERTICAL].Width;
        maxwidth = MAX(al[WIN1_FR_POLICE].Width,maxwidth+sub);

        al[WIN1_FR_POLICE].TotW = al[WIN1_FR_POLICE].Width = maxwidth;
        al[WIN1_FR_OUVERTURE].TotW = al[WIN1_FR_OUVERTURE].Width = al[WIN1_FR_FENETRE].TotW = al[WIN1_FR_FENETRE].Width = al[WIN1_FR_HORIZ].TotW = al[WIN1_FR_HORIZ].Width = maxwidth - sub;

        LayoutWidth( font, AGP.WindowLayout );
        }
    else maxwidth = al[WIN1_FR_POLICE].TotW;

    //------ Création de place dans le ReqString
    al[WIN1_FR_POLICE].Top = font->tf_YSize / 2;
    al[WIN1_FR_OUVERTURE].Top = al[WIN1_FR_POLICE].Top + al[WIN1_FR_POLICE].Height + font->tf_YSize;
    al[WIN1_FR_FENETRE].Top = al[WIN1_FR_OUVERTURE].Top + al[WIN1_FR_OUVERTURE].Height + font->tf_YSize;
    al[WIN1_FR_HORIZ].Top = al[WIN1_FR_FENETRE].Top + al[WIN1_FR_FENETRE].Height + font->tf_YSize;

    /*
    maxheight = al[WIN1_FR_POLICE].TotH +
                al[WIN1_FR_OUVERTURE].TotH +
                al[WIN1_FR_FENETRE].TotH +
                al[WIN1_FR_HORIZ].TotH +
                (font->tf_YSize * 3) / 2;  // ceci fait une imprécision de 1 pixel sur le placement en bas
    */
    maxheight = font->tf_YSize / 2 +
                al[WIN1_FR_POLICE].Height+
                al[WIN1_FR_OUVERTURE].Height +
                al[WIN1_FR_FENETRE].Height +
                al[WIN1_FR_HORIZ].Height +
                font->tf_YSize * 3;
    al[WIN1_FR_VERTICAL].Top = maxheight - al[WIN1_FR_VERTICAL].Height;

    if (! (mem = make_listptr( font, maxwidth, maxheight, &listheight ))) return(0);

    //------ Ajustement du Top des frames
    if ((maxheight = (listheight - maxheight) / 2) > 0)
        { al[WIN1_FR_POLICE].Top += maxheight; al[WIN1_FR_OUVERTURE].Top += maxheight; al[WIN1_FR_FENETRE].Top += maxheight; al[WIN1_FR_HORIZ].Top += maxheight; al[WIN1_FR_VERTICAL].Top += maxheight; }

    return( mem );
}

void inact_horiz_prop( struct UIKObj *obj )
{
  struct AZLayout *al;
  ULONG rega4;

    rega4 = __builtin_getreg( 12 );
    __builtin_putreg( 12, obj->UserULong1 ); // A4 pointe sur la structure des variables
    al = AGP.WindowLayout;
    if (obj->Flags & BOX_OVER)
        {
        UIK_Call( al[WIN1_HOR_BUT1   ].Obj, UIKFUNC_Check_Unselect, 0, 0 );
        UIK_Call( al[WIN1_HOR_FLECHE1].Obj, UIKFUNC_Check_Unselect, 0, 0 );
        UIK_Call( al[WIN1_HOR_FLECHE2].Obj, UIKFUNC_Check_Unselect, 0, 0 );
        UIK_Call( al[WIN1_HOR_BUT2   ].Obj, UIKFUNC_Check_Unselect, 0, 0 );
        UIK_ObjDisable( al[WIN1_HOR_BUT1   ].Obj, 1 );
        UIK_ObjDisable( al[WIN1_HOR_FLECHE1].Obj, 1 );
        UIK_ObjDisable( al[WIN1_HOR_FLECHE2].Obj, 1 );
        UIK_ObjDisable( al[WIN1_HOR_BUT2   ].Obj, 1 );
        }
    else{
        UIK_ObjEnable( al[WIN1_HOR_BUT1   ].Obj, 1 );
        UIK_ObjEnable( al[WIN1_HOR_FLECHE1].Obj, 1 );
        UIK_ObjEnable( al[WIN1_HOR_FLECHE2].Obj, 1 );
        UIK_ObjEnable( al[WIN1_HOR_BUT2   ].Obj, 1 );
        }
    __builtin_putreg( 12, rega4 ); // restore A4
}
void inact_vert_prop( struct UIKObj *obj )
{
  struct AZLayout *al;
  ULONG rega4;

    rega4 = __builtin_getreg( 12 );
    __builtin_putreg( 12, obj->UserULong1 ); // A4 pointe sur la structure des variables
    al = AGP.WindowLayout;
    if (obj->Flags & BOX_OVER)
        {
        UIK_Call( al[WIN1_VER_BUT1   ].Obj, UIKFUNC_Check_Unselect, 0, 0 );
        UIK_Call( al[WIN1_VER_FLECHE1].Obj, UIKFUNC_Check_Unselect, 0, 0 );
        UIK_Call( al[WIN1_VER_FLECHE2].Obj, UIKFUNC_Check_Unselect, 0, 0 );
        UIK_Call( al[WIN1_VER_BUT2   ].Obj, UIKFUNC_Check_Unselect, 0, 0 );
        UIK_ObjDisable( al[WIN1_VER_BUT1   ].Obj, 1 );
        UIK_ObjDisable( al[WIN1_VER_FLECHE1].Obj, 1 );
        UIK_ObjDisable( al[WIN1_VER_FLECHE2].Obj, 1 );
        UIK_ObjDisable( al[WIN1_VER_BUT2   ].Obj, 1 );
        }
    else{
        UIK_ObjEnable( al[WIN1_VER_BUT1   ].Obj, 1 );
        UIK_ObjEnable( al[WIN1_VER_FLECHE1].Obj, 1 );
        UIK_ObjEnable( al[WIN1_VER_FLECHE2].Obj, 1 );
        UIK_ObjEnable( al[WIN1_VER_BUT2   ].Obj, 1 );
        }
    __builtin_putreg( 12, rega4 ); // restore A4
}

static ULONG AddReqWin1( struct AZRequest *ar ) // appelée par func_IntSmartRequest(), WO existe
{
  struct UIKObjReqString *rs = (struct UIKObjReqString *)ar->Obj;
  struct AZLayout *al = AGP.WindowLayout;
  struct AZurPrefs *p = AGP.Prefs;
  struct UIKObjWFontBut *wfb;
  struct TextFont *font;
  WORD diff;
  UBYTE buf[32];

    //------ Calcul des abscisses des frames
    diff = (rs->Text->Obj.Box.Width - al[WIN1_FR_POLICE].TotW) / 2;
    al[WIN1_FR_POLICE].Left = al[WIN1_FR_OUVERTURE].Left = al[WIN1_FR_FENETRE].Left = al[WIN1_FR_HORIZ].Left = diff;
    al[WIN1_FR_VERTICAL].Left = al[WIN1_FR_POLICE].Left + al[WIN1_FR_POLICE].TotW - al[WIN1_FR_VERTICAL].TotW;

    //------ Mise des flags des gadgets
    setgad_active( al, WIN1_OUV_DEVANT, TST_FRONTSCREEN );
    setgad_active( al, WIN1_OUV_AZUR,   TST_AZURSCREEN  );
    setgad_active( al, WIN1_OUV_PLEIN,  TST_FULLSCREEN  );

    setgad_active( al, WIN1_FEN_STATUS , p->EleMask & BUMF_STATUSBAR );
    setgad_active( al, WIN1_FEN_COMMAND, p->EleMask & BUMF_CMDLINE   );

    if (! (p->EleMask & BUMF_HORIZPROP))
        p->EleMask &= ~(BUMF_BUTLEFT|BUMF_LEFTRIGHT1|BUMF_LEFTRIGHT2|BUMF_BUTRIGHT);
    setgad_active( al, WIN1_HOR_BUT1   , p->EleMask & BUMF_BUTLEFT    );
    setgad_active( al, WIN1_HOR_FLECHE1, p->EleMask & BUMF_LEFTRIGHT1 );
    setgad_active( al, WIN1_HOR_PROP   , p->EleMask & BUMF_HORIZPROP  );
    setgad_active( al, WIN1_HOR_FLECHE2, p->EleMask & BUMF_LEFTRIGHT2 );
    setgad_active( al, WIN1_HOR_BUT2   , p->EleMask & BUMF_BUTRIGHT   );
    if (! (p->EleMask & BUMF_HORIZPROP))
        {
        setgad_disable( al, WIN1_HOR_BUT1   , 1 );
        setgad_disable( al, WIN1_HOR_FLECHE1, 1 );
        setgad_disable( al, WIN1_HOR_FLECHE2, 1 );
        setgad_disable( al, WIN1_HOR_BUT2   , 1 );
        }

    if (! (p->EleMask & BUMF_VERTPROP))
        p->EleMask &= ~(BUMF_BUTUP|BUMF_UPDOWN1|BUMF_UPDOWN2|BUMF_BUTDOWN);
    setgad_active( al, WIN1_VER_BUT1   , p->EleMask & BUMF_BUTUP    );
    setgad_active( al, WIN1_VER_FLECHE1, p->EleMask & BUMF_UPDOWN1  );
    setgad_active( al, WIN1_VER_PROP   , p->EleMask & BUMF_VERTPROP );
    setgad_active( al, WIN1_VER_FLECHE2, p->EleMask & BUMF_UPDOWN2  );
    setgad_active( al, WIN1_VER_BUT2   , p->EleMask & BUMF_BUTDOWN  );
    if (! (p->EleMask & BUMF_VERTPROP))
        {
        setgad_disable( al, WIN1_VER_BUT1   , 1 );
        setgad_disable( al, WIN1_VER_FLECHE1, 1 );
        setgad_disable( al, WIN1_VER_FLECHE2, 1 );
        setgad_disable( al, WIN1_VER_BUT2   , 1 );
        }

    //------ Ajout des objets ------

    if (! LayoutAddObjects( rs, AGP.WindowLayout, &AGP, 0 )) goto END_ERROR;

    //------ Contenu des Strings
    wfb = (struct UIKObjWFontBut *)al[WIN1_POL_INTER].Obj;
    StcCpy( p->FontName, UIK_OSt_Get( wfb->OS, 0 ), 32 );

    if (font = UIK_OpenFont( p->FontName, p->FontHeight ))
        {
        UIK_Font2TTA( font, &wfb->TTA );
        StrCpy( wfb->FontName, p->FontName );
        wfb->TTA.tta_Name = wfb->FontName;
        UIK_OSt_Set( wfb->OS,  p->FontName, StrLen(p->FontName) );
        sprintf( buf, "%ld", p->FontHeight );
        UIK_OSt_Set( wfb->OSH, buf, StrLen(buf) );
        CloseFont( font );
        }
    wfb = (struct UIKObjWFontBut *)al[WIN1_POL_VUES].Obj;
    if (font = UIK_OpenFont( p->ViewFontName, p->ViewFontHeight ))
        {
        UIK_Font2TTA( font, &wfb->TTA );
        StrCpy( wfb->FontName, p->ViewFontName );
        wfb->TTA.tta_Name = wfb->FontName;
        UIK_OSt_Set( wfb->OS,  p->ViewFontName, StrLen(p->ViewFontName) );
        sprintf( buf, "%ld", p->ViewFontHeight );
        UIK_OSt_Set( wfb->OSH, buf, StrLen(buf) );
        CloseFont( font );
        }
    setgad_strnum( al, WIN1_OUV_X   , p->VI.ViewBox.Left   );
    setgad_strnum( al, WIN1_OUV_Y   , p->VI.ViewBox.Top    );
    setgad_strnum( al, WIN1_OUV_LARG, p->VI.ViewBox.Width  );
    setgad_strnum( al, WIN1_OUV_HAUT, p->VI.ViewBox.Height );

    //------ Bouton pour palette
    add_button( rs, al, WIN1_EMPTY, TEXT_Win1_TitleButton, ScreenColors );

    return(1);

  END_ERROR:
    return(0);
}

static void ReqWin1PrefsEnd( struct AZRequest *ar )
{
    if (ar->Button == REQBUT_OK)
        {
        struct UIKObjWFontBut *wfb;
        struct AZurPrefs *p = AGP.Prefs;
        struct AZLayout *al;
        ULONG *pf1 = &p->Flags1, *pfe = &p->EleMask;

        CopyMem( AGP.Prefs, AGP.TmpPrefs, sizeof(struct AZurPrefs) );

        al = AGP.WindowLayout; // est créé par prep_win1 ci-avant

        getgad_check( al, WIN1_OUV_DEVANT , pf1, AZP_FRONTSCREEN );
        getgad_check( al, WIN1_OUV_AZUR   , pf1, AZP_AZURSCREEN  );
        getgad_check( al, WIN1_OUV_PLEIN  , pf1, AZP_FULLSCREEN  );

        getgad_check( al, WIN1_FEN_STATUS , pfe, BUMF_STATUSBAR );
        getgad_check( al, WIN1_FEN_COMMAND, pfe, BUMF_CMDLINE   );

        getgad_check( al, WIN1_HOR_BUT1   , pfe, BUMF_BUTLEFT    );
        getgad_check( al, WIN1_HOR_FLECHE1, pfe, BUMF_LEFTRIGHT1 );
        getgad_check( al, WIN1_HOR_PROP   , pfe, BUMF_HORIZPROP  );
        getgad_check( al, WIN1_HOR_FLECHE2, pfe, BUMF_LEFTRIGHT2 );
        getgad_check( al, WIN1_HOR_BUT2   , pfe, BUMF_BUTRIGHT   );

        getgad_check( al, WIN1_VER_BUT1   , pfe, BUMF_BUTUP     );
        getgad_check( al, WIN1_VER_FLECHE1, pfe, BUMF_UPDOWN1   );
        getgad_check( al, WIN1_VER_PROP   , pfe, BUMF_VERTPROP  );
        getgad_check( al, WIN1_VER_FLECHE2, pfe, BUMF_UPDOWN2   );
        getgad_check( al, WIN1_VER_BUT2   , pfe, BUMF_BUTDOWN   );

        p->VI.ViewBox.Left   = getgad_strnum( al, WIN1_OUV_X    );
        p->VI.ViewBox.Top    = getgad_strnum( al, WIN1_OUV_Y    );
        p->VI.ViewBox.Width  = getgad_strnum( al, WIN1_OUV_LARG );
        p->VI.ViewBox.Height = getgad_strnum( al, WIN1_OUV_HAUT );

        wfb = (struct UIKObjWFontBut *)al[WIN1_POL_INTER].Obj;
        StrCpy( p->FontName, UIK_OSt_Get( wfb->OS, 0 ) );
        p->FontHeight = Str2Num( UIK_OSt_Get( wfb->OSH, 0 ) );

        wfb = (struct UIKObjWFontBut *)al[WIN1_POL_VUES].Obj;
        StrCpy( p->ViewFontName, UIK_OSt_Get( wfb->OS, 0 ) );
        p->ViewFontHeight = Str2Num( UIK_OSt_Get( wfb->OSH, 0 ) );

        // le Layout peut être égal à 0 si la fenêtre a été réallouée.
        }
    func_ReqClose( 0, ar );
    if (ar->Button == REQBUT_OK) ActualizePrefs(1);
}

ULONG func_ReqOpenWindowPrefs( struct AZRequest *ar )
{
  struct UIKObj *wo;
  UBYTE *mem=0;

    if (! (wo = AGP.WO)) goto END_ERROR;
    if (! (mem = prep_win1( wo->ObjExt->Font, 0 ))) goto END_ERROR;

    func_IntSmartRequest( 0, TEXT_PREFSWINDOW,0, 0,mem, 0,0, 0,0, -1,0,-1, AddReqWin1, ARF_WINDOW|ARF_ASYNC /*|ARF_ACTIVATE|ARF_RETQUIET*/, ReqWin1PrefsEnd, ar );
    UIK_MemSpFree( mem );
    return( ar->ErrMsg );

  END_ERROR:
    UIK_MemSpFree( mem );
    ar->Button = REQBUT_CANCEL;
    ar->ErrMsg = TEXT_NOMEMORY;
    return( TEXT_NOMEMORY );
}
