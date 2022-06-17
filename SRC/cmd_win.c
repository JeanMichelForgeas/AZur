/****************************************************************
 *
 *      Project:   AZUR
 *      Function:  objets
 *
 *      Created:   17/04/93 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"
#include "uiki:objarexxsim.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:objfile.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/


/****************************************************************
 *
 *      Objets
 *
 ****************************************************************/

struct UIKObjWindow *get_winicon_ptrs( struct UIKObjWindow *wo, struct Window **pw )
{
    *pw = 0;
    if (wo)
        {
        if (! (*pw = wo->Obj.w))
            if (!(wo = (struct UIKObjWindow *) wo->Obj.Icon) || !(*pw = wo->Obj.w))
                wo = 0;
        }
    return( wo );
}

//»»»»»» Cmd_WinClose       QUIET=FORCE/S,ALL/S,SAVE/S

void CmdFunc_WinClose_Do( struct CmdObj *co )
{
    struct Process *proc = co->ArgRes[1] ? AGP.AZMast->Process : AGP.Process;

/*
    if (co->ArgRes[1])   // ALL ?
        {
        proc = AGP.AZMast->Process;
        if (co->ArgRes[3]) AGP.AZMast->Prefs->Flags1 &= ~AZP_RESIDENT;
        }
    else{
        proc = AGP.Process;
        }
*/

    if (!co->ArgRes[0] && !co->ArgRes[1] && co->ArgRes[2])
        {
        if (AGP.FObj && (AGP.FObj->Flags & AZFILEF_MODIFIED) && !(TST_SAVELOCK))
            func_ExecuteAZurCmd( "Win2File SAME", 0 );
        }

    Signal( proc, co->ArgRes[0] ? SIGBREAKF_CTRL_C : SIGBREAKF_CTRL_D );
}

//»»»»»» Cmd_File2NewWin    ICONIFY/S,FILEREQ/S,CMD/K,FROM=MACRO/K,PORTNAME/K,NW=NOWINDOW/S,
//                          RO=READONLY/S,PRI=PRIORITY/K/N,PAT=PATTERN/K,NAME/F,BLOCK/S,
//                          LEFT/K/N,TOP/K/N,WIDTH/K/N,HEIGHT/K/N,CREATE/S,NI=NOICON/S,CURSNAME/S

void CmdFunc_File2NewWin_Do( struct CmdObj *co )
{
  UBYTE *ub=0, *ptr=0;
  ULONG *n, errmsg=0, priority;
  WORD left, top, width, height;

    ptr = co->ArgRes[9];
    if (co->ArgRes[10])    // BLOCK ?
        {
        if (errmsg = func_Block2Buf( &ub, 1 )) goto END_ERROR;
        patch_lf( ub );
        ptr = ub;
        }
    else if (co->ArgRes[17])  // CURSNAME
        {
        if (errmsg = func_Curs2FileName( &ub )) goto END_ERROR;
        ptr = ub;
        }
    else if (!ptr && AGP.FObj)  // AGP.FObj == 0 si master
        {
        if (lo_GetFIB( AGP.FObj->OpenFileName ))
            {
            if (! (ub = BufAlloc( 256, 0, MEMF_ANY ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            if (BufSetS( AGP.FObj->OpenFileName, StrLen(AGP.FObj->OpenFileName), &ub ))
                if (AGP.FIB.fib_DirEntryType < 0) *(UIK_BaseName( ub )) = 0;
            ptr = ub;
            }
        }
    left = top = width = height = -1;
    if (n = co->ArgRes[11]) left   = (WORD)*n;
    if (n = co->ArgRes[12]) top    = (WORD)*n;
    if (n = co->ArgRes[13]) width  = (WORD)*n;
    if (n = co->ArgRes[14]) height = (WORD)*n;

    if (co->ArgRes[7]) priority = *(ULONG*)co->ArgRes[7];
    else{
        priority = AGP.Process->pr_Task.tc_Node.ln_Pri;
        if (AGP.Process == AGP.AZMast->Process) priority--;
        }

    if (! func_LaunchProcess( AZTAG_LaunchF_Iconify,    co->ArgRes[0],
                              AZTAG_LaunchF_FileReq,    co->ArgRes[1],
                              AZTAG_Launch_FromCmd,     co->ArgRes[2],
                              AZTAG_Launch_FromScript,  co->ArgRes[3],
                              AZTAG_Launch_PortName,    co->ArgRes[4],
                              AZTAG_LaunchF_NoWindow,   co->ArgRes[5],
                              AZTAG_LaunchF_ReadOnly,   co->ArgRes[6],
                              AZTAG_Launch_Priority,    priority,
                              AZTAG_Launch_Pattern,     co->ArgRes[8],
                              AZTAG_Launch_FileName,    ptr ? ptr : "",
                              AZTAG_Launch_ResultPort,  &AGP.RexxResult,
                              AZTAG_Launch_CmdObj,      co,
                              AZTAG_LaunchF_QuitAfter,  co->ArgRes[5]?0:1,
                              AZTAG_Launch_LeftTop,     W2L(left,top),
                              AZTAG_Launch_WidthHeight, W2L(width,height),
                              AZTAG_LaunchF_CreateFile, co->ArgRes[15]?1:0,
                              AZTAG_LaunchF_IgnoreIcon, co->ArgRes[16]?1:0,
                              TAG_END )) errmsg = TEXT_ERROR_CreateTask;
  END_ERROR:
    BufFree( ub );
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_WinIconify

void CmdFunc_WinIconify_Do( struct CmdObj *co )
{
  ULONG desired, icon;

    if (! AGP.FObj->WO) return;

    icon = (AGP.FObj->WO->Obj.Status == UIKS_ICONIFIED) ? -1 : 0;
    desired = DoOnOffToggle( &co->ArgRes[0], icon );
    if (desired)
        {
        if (icon == 0)
            {
            ofi_SetIconColor();
            if (! UIK_Iconify( AGP.FObj->WO )) intAZ_SetCmdResult( co, TEXT_NOMEMORY );
            }
        }
    else{
        if (icon) UIK_UniconifyWindow( AGP.FObj->WO );
        }
}

//»»»»»» Cmd_ViewUpdate     ALL/S

void CmdFunc_ViewUpdate_Do( struct CmdObj *co )
{
  ULONG lock = TST_DISPLOCK;

    CLR_DISPLOCK;
    if (co->ArgRes[0])
        {
        func_DisplayStats(1);
        func_ResetProps();
        func_ReDisplay();
        }
    else{
        if (AGP.FObj->WO && AGP.FObj->WO->Obj.Status == UIKS_STARTED)
            {
            struct RastPort *rp = AGP.FObj->WO->Obj.rp;
            LockLayer( 0, rp->Layer );
            ozone_ReDisplay( AGP.FObj->ActiveView->Zone );
            UnlockLayer( rp->Layer );
            }
        }
    if (lock) SET_DISPLOCK;
}

//»»»»»» Cmd_Win2Back

void CmdFunc_Win2Back_Do( struct CmdObj *co )
{
  struct Window *w;

    if (get_winicon_ptrs( AGP.FObj->WO, &w ))
        WindowToBack( w );
}

//»»»»»» Cmd_Win2Front

void CmdFunc_Win2Front_Do( struct CmdObj *co )
{
  struct Window *w;

    if (get_winicon_ptrs( AGP.FObj->WO, &w ))
        {
        WindowToFront( w );
        ScreenToFront( w->WScreen );
        }
}

//»»»»»» Cmd_WinAct

void CmdFunc_WinAct_Do( struct CmdObj *co )
{
  struct Window *w;

    if (get_winicon_ptrs( AGP.FObj->WO, &w ))
        ActivateWindow( w );
}

//»»»»»» Cmd_WinChange      LEFT=LEFTEDGE/N,TOP=TOPEDGE/N,WIDTH/N,HEIGHT/N

void CmdFunc_WinChange_Do( struct CmdObj *co )
{
  struct UIKObjWindow *wo;
  struct Window *w;
  struct Screen *s;
  WORD left, top, width, height;

    if (! (wo = get_winicon_ptrs( AGP.FObj->WO, &w ))) return;
    s = w->WScreen;

    left   = (WORD) (co->ArgRes[0] ? *((LONG*)co->ArgRes[0]) : w->LeftEdge);
    top    = (WORD) (co->ArgRes[1] ? *((LONG*)co->ArgRes[1]) : w->TopEdge);
    width  = (WORD) (co->ArgRes[2] ? *((LONG*)co->ArgRes[2]) : w->Width);
    height = (WORD) (co->ArgRes[3] ? *((LONG*)co->ArgRes[3]) : w->Height);

    if (left   == -1) left   = w->LeftEdge;
    if (top    == -1) top    = w->TopEdge;
    if (width  == -1) width  = w->Width;
    if (height == -1) height = w->Height;

    if (wo->NW.MaxWidth < width) width = wo->NW.MaxWidth;
    if (wo->NW.MaxHeight < height) height = wo->NW.MaxHeight;
    if (wo->NW.MinWidth > width) width = wo->NW.MinWidth;
    if (wo->NW.MinHeight > height) height = wo->NW.MinHeight;

    if (left < 0) left = 0;
    if (width > s->Width) width = s->Width;
    if (left + width > s->Width) left = s->Width - width;

    if (top < 0) top = 0;
    if (height > s->Height) height = s->Height;
    if (top + height > s->Height) top = s->Height - height;

    if (UIK_IsSystemVersion2())
        {
        ChangeWindowBox( w, left, top, width, height );
        }
    else{
        if ((left + w->Width <= s->Width) && (top + w->Height <= s->Height))
            {
            MoveWindow( w, left - w->LeftEdge, top - w->TopEdge );
            SizeWindow( w, width - w->Width, height - w->Height );
            }
        else if ((w->LeftEdge + width <= s->Width) && (w->TopEdge + height <= s->Height))
            {
            SizeWindow( w, width - w->Width, height - w->Height );
            MoveWindow( w, left - w->LeftEdge, top - w->TopEdge );
            }
        else{
            MoveWindow( w, 0, 0 );
            SizeWindow( w, width - w->Width, height - w->Height );
            MoveWindow( w, left - w->LeftEdge, top - w->TopEdge );
            }
        }
}

//»»»»»» Cmd_WinLast

struct AZurNode *func_PLTaskNode( struct Task *task )
{
  struct AZurNode *node, *res=0;
  struct AZur *mast = AGP.AZMast;

    ObtainSemaphore( &mast->ProcSem );
    for (node=(struct AZurNode *)mast->ProcList.lh_Head; node->ln_Succ; node = (struct AZurNode *)node->ln_Succ)
        {
        if (task == node->ln_Task) { res = node; break; }
        }
    ReleaseSemaphore( &mast->ProcSem );
    return( res );
}

struct AZurNode *func_PLFindNode( struct AZurNode *searchnode )
{
  struct AZurNode *node, *res=0;
  struct AZur *mast = AGP.AZMast;

    ObtainSemaphore( &mast->ProcSem );
    for (node=(struct AZurNode *)mast->ProcList.lh_Head; node->ln_Succ; node = (struct AZurNode *)node->ln_Succ)
        {
        if (searchnode == node) { res = node; break; }
        }
    ReleaseSemaphore( &mast->ProcSem );
    return( res );
}

struct AZurNode *func_PLLastActiveNode( void )
{
  struct AZurNode *node;

    Forbid();
    if (! (node = func_PLFindNode( AGP.AZMast->LastActNode ))) node = (struct AZurNode *)AGP.AZMast->ProcList.lh_Head;
    Permit();
    return( node );
}

static void do_win_act( ULONG type )
{
  struct AZurNode *node;
  UBYTE name[30], cmd[100];

    Forbid();
    node = func_PLLastActiveNode();
    if (type == 1)  // prev
        {
        node = (struct AZurNode *)node->ln_Pred;
        if (node->ln_Pred == 0) node = (struct AZurNode *)AGP.AZMast->ProcList.lh_TailPred;
        }
    else if (type == 2) // next
        {
        node = (struct AZurNode *)node->ln_Succ;
        if (node->ln_Succ == 0) node = (struct AZurNode *)AGP.AZMast->ProcList.lh_Head;
        }
    StrCpy( name, node->ln_PortName );
    Permit();
    sprintf( cmd, "ADDRESS %ls; Iconify OFF; Win2Front; WinAct;", name );
    UIK_Call( AGP.ARexx, UIKFUNC_ARexxSim_SendSyncCmd, (ULONG)cmd, 1 );
    BufSetS( name, StrLen(name), &AGP.RexxResult );
}

void CmdFunc_WinLast_Do( struct CmdObj *co )
{
    do_win_act( 0 );
}

//»»»»»» Cmd_WinNext

void CmdFunc_WinNext_Do( struct CmdObj *co )
{
    do_win_act( 2 );
}

//»»»»»» Cmd_WinPrev

void CmdFunc_WinPrev_Do( struct CmdObj *co )
{
    do_win_act( 1 );
}

//»»»»»» Cmd_WinZoom        FULL/S,ON/S,OFF/S,TOGGLE/S

void CmdFunc_WinZoom_Do( struct CmdObj *co )
{
  struct UIKObjWindow *wo;
  ULONG size = -1; // toggle

    if (! (wo = AGP.FObj->WO)) return;
    if (wo->Obj.Status == UIKS_ICONIFIED) return;

    if (co->ArgRes[1]) size = 1;
    else if (co->ArgRes[2]) size = 0;
    UIK_Call( wo, UIKFUNC_Window_Zip, size, (ULONG)co->ArgRes[0] );
}

//»»»»»» Cmd_Screen2Back

void CmdFunc_Screen2Back_Do( struct CmdObj *co )
{
  struct Window *w;

    if (get_winicon_ptrs( AGP.WO, &w ))
        ScreenToBack( w->WScreen );
}

//»»»»»» Cmd_Screen2Front

void CmdFunc_Screen2Front_Do( struct CmdObj *co )
{
  struct Window *w;

    if (get_winicon_ptrs( AGP.WO, &w ))
        ScreenToFront( w->WScreen );
}
