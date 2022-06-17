/****************************************************************
 *
 *      File:       funcs_display.c fonctions d'affichage
 *      Project:    AZur            editeur de texte
 *
 *      Created:    12/05/93        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objview.h"
#include "lci:objfile.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern void oview_ScrollView( struct AZObjView *vo, ULONG vert, ULONG abslen, LONG len, LONG from );
extern void oview_Display( struct AZObjView *vo, LONG startline, LONG nlines, LONG startcol, LONG ncols );
extern void oview_ReDisplay( struct AZObjView *vo );
extern void oview_DisplayBlock( struct AZObjView *vo );
extern void oview_DisplayCmd( struct AZObjView *vo, struct AZDisplayCmd *dcmd );
extern void oview_ViewTopLeft( struct AZObjView *vo );
extern void oview_ResetProps( struct AZObjView *vo );
//extern void oview_DisplayStats( struct AZObjView *vo, ULONG title );
extern void oview_DisplayCurs( struct AZObjView *vo, ULONG line, ULONG col );


/****** Exported ************************************************/


/****** Statics *************************************************/


/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

void map_view_nodes1_nolock( void (*func)(), ULONG parm )
{
  struct Node *node;
  //struct RastPort *rp;

    if (AGP.FObj && AGP.WO && AGP.WO->Obj.Status == UIKS_STARTED && !TST_DISPLOCK)
        {
        //rp = AGP.WO->Obj.rp;
        for (node=AGP.FObj->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
            {
            func( node->ln_Name, parm );
            }
        }
}

static void map_view_nodes1( void (*func)(), ULONG parm )
{
  struct Node *node;
  struct RastPort *rp;

    if (AGP.FObj && AGP.WO && AGP.WO->Obj.Status == UIKS_STARTED && !TST_DISPLOCK)
        {
        rp = AGP.WO->Obj.rp;
        LockLayer( 0, rp->Layer );
        for (node=AGP.FObj->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
            {
            func( node->ln_Name, parm );
            }
        UnlockLayer( rp->Layer );
        }
}

static void map_view_nodes4( void (*func)(), ULONG p1, ULONG p2, ULONG p3, ULONG p4 )
{
  struct Node *node;
  struct RastPort *rp;

    if (AGP.FObj && AGP.WO && AGP.WO->Obj.Status == UIKS_STARTED && !TST_DISPLOCK)
        {
        rp = AGP.WO->Obj.rp;
        LockLayer( 0, rp->Layer );
        for (node=AGP.FObj->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
            {
            func( node->ln_Name, p1, p2, p3, p4 );
            }
        UnlockLayer( rp->Layer );
        }
}

//--------------------------------------------------------------

void func_RefreshZones()
{
  struct Node *node;

    if (AGP.FObj && AGP.WO && AGP.WO->Obj.Status == UIKS_STARTED)
        {
        for (node=AGP.FObj->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
            UIK_Refresh( ((struct AZObjView *)node->ln_Name)->Zone, 1 );
        }
}

void func_Scroll( ULONG vert, LONG len, LONG from )
{
    map_view_nodes4( oview_ScrollView, vert, ABS(len), len, from );
}

void func_Display( LONG startline, LONG nlines, LONG startcol, LONG ncols )
{
    if (!nlines || !ncols) return;
    map_view_nodes4( oview_Display, startline, nlines, startcol, ncols );
}

void func_ReDisplay()
{
    map_view_nodes1( oview_ReDisplay, 0 );
}

void func_DisplayBlock()
{
    map_view_nodes1( oview_DisplayBlock, 0 );
}

void func_DisplayCmd( struct AZDisplayCmd *dcmd )
{
    if (dcmd->DisplayNum || dcmd->ScrollNum)
        map_view_nodes1( oview_DisplayCmd, (ULONG)dcmd );
}

void func_ViewsTopLeft()
{
    map_view_nodes1( oview_ViewTopLeft, 0 );
}

void func_ResetProps()
{
    map_view_nodes1_nolock( oview_ResetProps, 0 );
}

void func_DisplayStats( ULONG title ) // On appelle directement à cause de DISPLOCK
{
//  struct Node *node;
//  struct RastPort *rp;

    if ((AGP.FObj && AGP.WO && AGP.WO->Obj.Status == UIKS_STARTED) && !TST_STATUSLOCK)
        {
        if (AGP.FObj->Stat) ostat_Display( AGP.FObj->Stat, title );
        /*
        rp = AGP.WO->Obj.rp;
        for (node=AGP.FObj->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
            {
            oview_DisplayStats( (struct AZObjView *)node->ln_Name, title );
            }
        */
        }
}

void func_DisplayCurs( ULONG line, ULONG col )
{
    //map_view_nodes4( oview_DisplayCurs, line, col, 0, 0 );
    if (AGP.FObj && AGP.WO && AGP.WO->Obj.Status == UIKS_STARTED && !TST_DISPLOCK)
        {
        oview_DisplayCurs( AGP.FObj->ActiveView, line, col );
        }
}
