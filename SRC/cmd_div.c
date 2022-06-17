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
#include "uiki:objwinfilesel.h"

#include "lci:objmenu.h"
#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:objfile.h"
#include "lci:objzone.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include <rexx/storage.h>
#include <rexx/errors.h>


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern UBYTE __far VersionTag[];

extern LONG func_FoldSet( LONG sline, LONG eline, struct AZDisplayCmd *dinfo );
extern LONG func_FoldUnset( LONG line, struct AZDisplayCmd *dinfo );
extern LONG func_FoldOn( LONG line, struct AZDisplayCmd *dinfo );
extern LONG func_FoldOff( LONG line, struct AZDisplayCmd *dinfo );
extern LONG func_FoldOffSub( LONG line, struct AZDisplayCmd *dinfo );


/****** Exported ***********************************************/


/****** Statics ************************************************/

static ULONG __far lock_flags[] = { AZP_DISPLOCK, AZP_INPUTLOCK, AZP_READONLY, AZP_SAVELOCK, AZP_STATUSLOCK, AZP_AREXXLOCK };
static ULONG __far qual_flags[] = { IEQUALIFIER_CONTROL, IEQUALIFIER_LSHIFT, IEQUALIFIER_LALT, IEQUALIFIER_LCOMMAND, SQUAL_QUAL1, SQUAL_QUAL2, SQUAL_QUAL3, SQUAL_LOCK1, SQUAL_LOCK2, SQUAL_LOCK3, SQUAL_ANYSPECIAL };


/****************************************************************
 *
 *      Objets
 *
 ****************************************************************/

//»»»»»» Cmd_Qualifier      CTRL/S,SHIFT/S,ALT/S,AMIGA/S,QUAL1/S,QUAL2/S,QUAL3/S,LOCK1/S,LOCK2/S,LOCK3/S,ANY/S,ON/S,OFF/S,TOGGLE/S

void CmdFunc_Qualifier_Do( struct CmdObj *co )
{
  struct KeyHandle *kh = (struct KeyHandle *) AGP.AZMast->KeyHandle;
  ULONG i;

    for (i=0; i<11; i++)
        {
        if (co->ArgRes[i])
            func_SetQualifier( qual_flags[i], DoOnOffToggle( &co->ArgRes[11], (kh->AZQual & qual_flags[i]) ? -1 : 0 ) );
        }
    if (co->ArgRes[7] || co->ArgRes[8] || co->ArgRes[9]) if (AGP.FObj) UPDATE_STAT(AGP.FObj);
}

//»»»»»» Cmd_Help           COMMAND/A,VB=VERBOSE/S,FULL/S,T=TEMPLATE/S,S=SYNONYM/S,F=FLAGS/S

void CmdFunc_Help_Do( struct CmdObj *co )
{
  struct CmdGerm *cg;
  ULONG len;

    CopyProt();

    if (! (cg = (struct CmdGerm *) func_FindRegGerm( (APTR)co->ArgRes[0] )))
        cg = func_LoadGerm( (UBYTE*)co->ArgRes[0] );

    if (cg)
        {
        BufTruncate( AGP.RexxResult, 0 );
        if (co->ArgRes[2] || co->ArgRes[3] || (!co->ArgRes[4] && !co->ArgRes[5]))
            {
            func_BufPrintf( &AGP.RexxResult, -1, ":%ls\n", cg->CmdTemplate ? cg->CmdTemplate : "" );
            }
        if (co->ArgRes[2] || co->ArgRes[4])
            {
            UBYTE **names;
            for (names=cg->Names; *names; names++) func_BufPrintf( &AGP.RexxResult, -1, "=%ls", *names );
            func_BufPrintf( &AGP.RexxResult, -1, "\n" );
            }
        if (co->ArgRes[2] || co->ArgRes[5])
            {
            func_BufPrintf( &AGP.RexxResult, -1, "(%lc%lc%lc%lc%lc%lc%lc%lc%lc)\n",
                (cg->Flags & AZAFF_DOMODIFY)   ? 'Y' : '.',
                (cg->Flags & AZAFF_OKINMACRO)  ? 'O' : '.',
                (cg->Flags & AZAFF_OKINAREXX)  ? 'A' : '.',
                (cg->Flags & AZAFF_OKINMENU)   ? 'M' : '.',
                (cg->Flags & AZAFF_OKINKEY)    ? 'K' : '.',
                (cg->Flags & AZAFF_OKINMOUSE)  ? 'S' : '.',
                (cg->Flags & AZAFF_OKINJOY)    ? 'J' : '.',
                (cg->Flags & AZAFF_OKINMASTER) ? 'T' : '.',
                (cg->Flags & AZAFF_OKINHOTKEY) ? 'I' : '.' );
            }
        if (len = intAZ_LineBufLen(AGP.RexxResult)) BufTruncate( AGP.RexxResult, len-1 );
        }
    else intAZ_SetCmdResult( co, TEXT_CmdNotRegistered );
}

//»»»»»» Cmd_Illegal

void CmdFunc_Illegal_Do( struct CmdObj *co )
{
    /*
    ULONG action = func_NewAction( co->RegGerm );
    kprintf( "action=%ld\n", action );
    func_DeleteAction( co->RegGerm, action );
    */
    intAZ_SetCmdResult( co, TEXT_IllegalFunc );
}

//»»»»»» Cmd_AbortARexx

void CmdFunc_AbortARexx_Do( struct CmdObj *co )
{
    SET_ABORTAREXX;
}

//»»»»»» Cmd_Lock           D=DISPLAY/S,I=INPUT/S,M=MODIFY/S,S=SAVE/S,STATUS/S,A=AREXX/S,ON/S,OFF/S,TOGGLE/S

#define TOTLOCK 6

void CmdFunc_Lock_Do( struct CmdObj *co )
{
  ULONG i, flag, upd_stat=0, avant=0, apres=0;
  struct Window *w;

    if (TST_INPUTLOCK || TST_DISPLOCK) avant = 1;

    for (i=0; i<TOTLOCK; i++)
        {
        if (co->ArgRes[i])
            {
            upd_stat = 1;
            flag = lock_flags[i];
            if (DoOnOffToggle( &co->ArgRes[TOTLOCK], (AGP.Prefs->Flags1 & flag) ? -1 : 0 )) AGP.Prefs->Flags1 |= flag;
            else AGP.Prefs->Flags1 &= ~flag;
            }
        }
    if (co->ArgRes[0] && (co->ArgRes[TOTLOCK+1] || (co->ArgRes[TOTLOCK+2] && !TST_DISPLOCK)))
        {
        //func_DisplayStats(1);
        func_ReDisplay();
        }
    if (co->ArgRes[1] && AGP.FObj && AGP.FObj->WO)
        {
        if (w = AGP.FObj->WO->Obj.w)
            {
            //if (co->ArgRes[TOTLOCK+1] || (co->ArgRes[TOTLOCK+2] && !TST_INPUTLOCK))
            if (!TST_INPUTLOCK) w->Flags &= ~RMBTRAP;
            else w->Flags |= RMBTRAP;
            }
        }
    if (co->ArgRes[0] || co->ArgRes[1])
        {
        if (TST_INPUTLOCK || TST_DISPLOCK) apres = 1;

        if (avant == 0 && apres == 1) func_SetMainWaitPointer();
        else if (avant == 1 && apres == 0) func_SetMainSystemPointer();
        }

    if (upd_stat) if (AGP.FObj) UPDATE_STAT(AGP.FObj);
}

//»»»»»» Cmd_Load           NAME/F/A

void CmdFunc_Load_Do( struct CmdObj *co )
{
    if (! eng_NewGerm( co->ArgRes[0] ))
        intAZ_SetCmdResult( co, AGP.RexxError );
}

//»»»»»» Cmd_Unload         FORCE/S,NAME/F/A

void CmdFunc_Unload_Do( struct CmdObj *co )
{
  LONG count;

    count = func_UnloadGermForce( co->ArgRes[0], co->ArgRes[1] );
    if (count == -1) intAZ_SetCmdResult( co, TEXT_GermNotFound );
    else if (count != 0) intAZ_SetCmdResult( co, TEXT_GermInUse );
}

//»»»»»» Cmd_Version

void CmdFunc_Version_Do( struct CmdObj *co )
{
  UBYTE *ptr = UIK_LangString( AGP.UIK, TEXT_COPYRIGHT );
  ULONG len, num;

    for (len=num=0; ; len++) if (ptr[len] == '\n') if (++num == 2) break;
    ptr[len] = 0;

    func_SetCmdResult( co, 0, 0, &VersionTag[7] );
    BufPasteS( "\n", 1, &AGP.RexxResult, intAZ_LineBufLen(AGP.RexxResult) );
    BufPasteS( ptr, len, &AGP.RexxResult, intAZ_LineBufLen(AGP.RexxResult) );
}

//»»»»»» Cmd_Beep

void CmdFunc_Beep_Do( struct CmdObj *co )
{
    DisplayBeep( NULL );
}

//»»»»»» Cmd_ActivateCmdLine

void CmdFunc_ActivateCmdLine_Do( struct CmdObj *co )
{
    if (AGP.FObj && AGP.FObj->CmdLine) UIK_ActivateKeyTank( AGP.FObj->CmdLine );
}

//»»»»»» Cmd_InactivateCmdLine

void CmdFunc_InactivateCmdLine_Do( struct CmdObj *co )
{
    UIK_ActivateKeyTank( AGP.FObj );
}

//»»»»»» Cmd_Debug

void CmdFunc_Debug_Do( struct CmdObj *co )
{
    Debug(0);
}

//»»»»»» Cmd_Block2Mac      CLEAR/S,AP=APPEND/S

void CmdFunc_Block2Mac_Do( struct CmdObj *co )
{
  ULONG errmsg=0;
  UBYTE *ub=0, *errorname=0;

    if (! (errmsg = func_Block2Buf( &ub, (ULONG)co->ArgRes[0] )))
        {
        if (! co->ArgRes[1]) func_MacroReset(); // APPEND
        if (! (errmsg = func_Buf2Macro( &ub, &errorname )))
            if (! func_MacroExists()) errmsg = TEXT_ERR_NoRecMacro;
        }
    if (errmsg) SetCmdResultWith( co, errmsg, errorname );
    BufFree( ub );
}

//»»»»»» Cmd_Mac2Win

void CmdFunc_Mac2Win_Do( struct CmdObj *co )
{
  ULONG errmsg=0;
  UBYTE *ub=0;

    if (! (errmsg = func_Macro2Buf( &ub )))
        // if (! (errmsg = func AZ_TextPrep( AGP.FObj->Line, AGP.FObj->Col, 0, 0, 0 )))
            errmsg = func_TextPut( ub, BufLength(ub), 0, 1 );
    BufFree( ub );
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_MacEnd

void CmdFunc_MacEnd_Do( struct CmdObj *co )
{
    func_MacroEndRecord();
}

//»»»»»» Cmd_MacDelete

void CmdFunc_MacDelete_Do( struct CmdObj *co )
{
    if (func_MacroExists()) func_MacroDelete();
    else intAZ_SetCmdResult( co, TEXT_ERR_NoRecMacro );
}

//»»»»»» Cmd_MacPlay        COUNT/N

void CmdFunc_MacPlay_Do( struct CmdObj *co )
{
  ULONG chgptr, errmsg=0, num=0;

    if (! func_MacroExists()) { errmsg = TEXT_ERR_NoRecMacro; goto END_ERROR; }

    func_MacroEndRecord();

    if (! co->ArgRes[0]) { if (func_ReqNumber( &num ) == REQBUT_CANCEL) goto END_ERROR; }
    else num = *((ULONG*)co->ArgRes[0]);

    chgptr = (co->ExeFrom == FROMAREXX) ? 0 : 1;
    errmsg = func_MacroPlay( num, chgptr ); // erreur déjà mise par la commande

  END_ERROR:
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_MacRecord      AP=APPEND/S,QUIET/S

void CmdFunc_MacRecord_Do( struct CmdObj *co )
{
  ULONG chgptr, errmsg=0;

    chgptr = (co->ExeFrom == FROMAREXX) ? 0 : 1;
    if (! co->ArgRes[0]) func_MacroReset(); // APPEND
    errmsg = func_MacroRecord( (ULONG)co->ArgRes[1], chgptr );
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_RexxNameObtain     NAME/A
//
//    Multi-queue sync mechanism, one queue by name

static void queue_remove( struct WaitAppl *node )
{
    Remove( node );
    UIK_MemSpFree( node );
}

void rxn_Cleanup()
{
  struct WaitAppl *node;

    ObtainSemaphore( &AGP.AZMast->RexxNameSem );
    for (node=(struct WaitAppl *)AGP.AZMast->RexxNameList.lh_Head; node->Node.ln_Succ; node = (struct WaitAppl *)node->Node.ln_Succ)
        {
        if (node->RMsg)
            {   // ReplyARexxMsg( ARexx, node->RMsg, 20, 0, 20, "Command cancelled" );
            /* Ici le message n'appartient pas forcement à cette tâche-la
             * mais l'important est de seulement remplir les champs et de
             * répondre au message.
             */
            AGP.ARexx->ResultCode  = 20;
            AGP.ARexx->ErrorCode   = TEXT_CMDCANCELLED;
            AGP.ARexx->ErrorString = UIK_LangString( AGP.UIK, TEXT_CMDCANCELLED );
            UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_ReplyARexxMsg, (ULONG)node->RMsg, 0 );
            }
        }
    ReleaseSemaphore( &AGP.AZMast->RexxNameSem );
}

static struct WaitAppl *queue_add( UBYTE *name, APTR rmsg )
{
  struct WaitAppl *node;

    if (node = (struct WaitAppl *) UIK_MemSpAlloc( sizeof(struct WaitAppl) + StrLen( name ), MEMF_PUBLIC|MEMF_CLEAR ))
        {
        strcpy( node->Name, name );
        node->RMsg = rmsg;
        AddTail( &AGP.AZMast->RexxNameList, node );
        }
    return( node );
}

static struct WaitAppl *queue_search( UBYTE *name )
{
  struct WaitAppl *node, *found=0;

    for (node=(struct WaitAppl *)AGP.AZMast->RexxNameList.lh_Head; node->Node.ln_Succ; node = (struct WaitAppl *)node->Node.ln_Succ)
        {
        if (StrCmp( name, node->Name ) == 0) { found = node; break; }
        }
    return( found );
}

void CmdFunc_RexxNameObtain_Do( struct CmdObj *co )
{
  UBYTE *name = co->ArgRes[0];
  APTR rmsg = AGP.ARexx->RMsg;
  ULONG errmsg = 0;

    ObtainSemaphore( &AGP.AZMast->RexxNameSem );
    if (queue_search( name ))
        {
        if (queue_add( name, rmsg )) co->ResCode = (UBYTE)-1; // on garde le message si le nom est réservé
        else errmsg = TEXT_NOMEMORY;
        }
    else{
        if (queue_add( name, 0 )) ;
        else errmsg = TEXT_NOMEMORY;
        }
    ReleaseSemaphore( &AGP.AZMast->RexxNameSem );

    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_RexxNameRelease    NAME/A

void CmdFunc_RexxNameRelease_Do( struct CmdObj *co )
{
  UBYTE *name = co->ArgRes[0];
  ULONG errmsg = 0;
  UBYTE keyword=0;
  struct WaitAppl *node;
  APTR msg;

    ObtainSemaphore( &AGP.AZMast->RexxNameSem );
    while (node = queue_search( name ))
        {
        keyword = 1;
        /* Le premier n'a pas de RMsg car il a été répondu (pas de blocage pour le premier).
         * Par contre le node du suivant reste et devient premier à son tour.
         * On enlève seulement le node du premier, celui du deuxième reste.
         */
        if (msg = node->RMsg)
            {   /* ReplyARexxMsg( ARexx, msg, 0, 0, 0, 0 ); */
            AGP.ARexx->ResultCode   = RC_OK;
            AGP.ARexx->ResultString = "";
            UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_ReplyARexxMsg, (ULONG)msg, 0 );
            node->RMsg = 0;
            }
        else queue_remove( node );
        if (msg) break;
        }
    ReleaseSemaphore( &AGP.AZMast->RexxNameSem );

    if (keyword == 0) errmsg = TEXT_Search_NotFound;

    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_RexxNameClear

void CmdFunc_RexxNameClear_Do( struct CmdObj *co )
{
  UBYTE keyword = 0, *name = co->ArgRes[0];
  struct WaitAppl *node, *curr;
  ULONG errmsg = 0;

    ObtainSemaphore( &AGP.AZMast->RexxNameSem );
    for (node=(struct WaitAppl *)AGP.AZMast->RexxNameList.lh_Head; node->Node.ln_Succ; )
        {
        keyword = 1;
        curr = node;
        node = (struct WaitAppl *)node->Node.ln_Succ;
        if (StrCmp( name, curr->Name ) == 0)
            {
            if (curr->RMsg)
                {   // ReplyARexxMsg( ARexx, curr->RMsg, 5, 0, 5, "Command cancelled by 'ClearAppl'" );
                /* Ici le message n'appartient pas forcement à cette tâche-la
                 * mais l'important est de seulement remplir les champs et de
                 * répondre au message.
                 */
                AGP.ARexx->ResultCode  = intAZ_GetErrorSeverity( TEXT_CMDCANCELLED );
                AGP.ARexx->ErrorCode   = TEXT_CMDCANCELLED;
                AGP.ARexx->ErrorString = UIK_LangString( AGP.UIK, TEXT_CMDCANCELLED );
                UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_ReplyARexxMsg, (ULONG)curr->RMsg, 0 );
                }
            queue_remove( curr );
            }
        }
    ReleaseSemaphore( &AGP.AZMast->RexxNameSem );

    if (keyword == 0) errmsg = TEXT_Search_NotFound;

    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_Fold       LINE/N,ENDLINE/N,SET/S,ADDSAME/S,UNSET/S,ON/S,OFF/S,TOGGLE/S,SUB=NESTED/S,ALL/S

#define ARGFO_LINE    co->ArgRes[0]
#define ARGFO_ENDLINE co->ArgRes[1]
#define ARGFO_SET     co->ArgRes[2]
#define ARGFO_ADDSAME co->ArgRes[3]
#define ARGFO_UNSET   co->ArgRes[4]
#define ARGFO_ON      co->ArgRes[5]
#define ARGFO_OFF     co->ArgRes[6]
#define ARGFO_TOGGLE  co->ArgRes[7]
#define ARGFO_SUB     co->ArgRes[8]    // pour UNSET, ON, OFF
#define ARGFO_ALL     co->ArgRes[9]    // pour UNSET, ON, OFF

#define FPDOF_SET   0x0001
#define FPDOF_UNSET 0x0002
#define FPDOF_ON    0x0004
#define FPDOF_OFF   0x0008
#define FPDOF_SUB   0x0010
#define FPDOF_LOOP  0x0020
#define FPDOF_ALL   0x0040

struct foldparms {
    LONG  sline;
    LONG  eline;
    ULONG flags;
    ULONG *parg;
    struct AZDisplayCmd DI;
    ULONG errmsg;
    LONG  finishline;
    ULONG num_act;
    };

static ULONG do_loop( struct foldparms *fp, ULONG (*func)() )
{
  LONG sline, line, col, change;
  UWORD actid, id;

    if (fp->flags & FPDOF_LOOP)
        {
        if (actid = fo_GetLimits( fp->sline, (UWORD)-1, &sline, &line, 1 )) fp->sline = sline;
        else actid = 1;

        line = fp->eline + 1;
        while ((id = func_FoldGetPrev( &line, &col )) && (line >= fp->sline))
            {
            if (id == actid || ((id > actid) && (fp->flags & FPDOF_SUB)))
                {
                change = func( line, &fp->DI );
                fp->num_act++;
                fp->finishline = line;
                if ((change > 0) && (fp->flags & FPDOF_SUB)) line += change + 1;
                fp->eline += change;
                }
            }
        }
    else{
        change = func( fp->sline, &fp->DI );
        fp->num_act++;
        fp->finishline = fp->DI.DisplayFrom;
        fp->eline += change;
        }
    return(0);
}

static void fold_action( struct foldparms *fp )
{
    fp->flags &= ~(FPDOF_ON|FPDOF_OFF);
    //         ON            OFF           TOGGLE
    if ((fp->parg[0] || fp->parg[1] || fp->parg[2]) && !(fp->flags & FPDOF_UNSET))
        {
        ULONG i, last=AGP.FObj->Text.NodeNum-1, folded=0;
        struct UIKNode *node=AGP.FObj->Text.UNode;
        if (fp->flags & FPDOF_ALL)
            { for (i=0; i <= last; i++, node++) { if (node->Flags & UNF_FOLDED) { folded = 1; break; } } }
        else folded = fo_IsLineFolded( fp->sline );

        if (DoOnOffToggle( fp->parg, folded ? -1 : 0 )) fp->flags |= FPDOF_ON;
        else fp->flags |= FPDOF_OFF;
        }

    if (fp->flags & FPDOF_SET)
        {
        func_FoldSet( fp->sline, fp->eline, &fp->DI );
        fp->num_act++;
        }
    else if (fp->flags & FPDOF_UNSET)
        {
        if (do_loop( fp, func_FoldUnset )) return;
        fp->finishline = -1; // on ne bouge pas le curseur
        }
    if (fp->flags & FPDOF_ON)
        {
        if (do_loop( fp, func_FoldOn )) return;
        }
    else if (fp->flags & FPDOF_OFF)
        {
        if (do_loop( fp, (fp->flags & FPDOF_SUB) ? func_FoldOffSub : func_FoldOff )) return;
        }
}

static ULONG fold_bloc_action( struct AZBlock *azb, ULONG id, struct foldparms *fp )
{
    fp->sline = azb->LineStart;
    fp->eline = azb->LineEnd;
    func_BlockRemove( id, 0, 0 );
    fold_action( fp );
    return( fp->errmsg );
}

void CmdFunc_Fold_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  struct foldparms FP;
  ULONG block=0;

    __builtin_memset( &FP, 0, sizeof(struct foldparms) );
    FP.finishline = -1;

    //------------------ Flags
    if (ARGFO_SET)
        {
        FP.flags |= FPDOF_SET;
        }
    else if (ARGFO_UNSET)
        {
        FP.flags |= FPDOF_UNSET;
        }

    if (ARGFO_SUB && !(FP.flags & FPDOF_SET))
        {
        FP.flags |= FPDOF_SUB;
        }

    FP.parg = (ULONG*)&ARGFO_ON;

    //------------------ Limites
    func_CursorErase();

    if (func_BlockExists())
        {
        block = 1;
        FP.flags |= FPDOF_LOOP;
        FP.flags &= ~FPDOF_ALL;
        func_BlockApply( (void*)fold_bloc_action, 1, -1, 1, &FP );
        }
    else{
        if (ARGFO_LINE)
            {
            FP.sline = *((LONG*)ARGFO_LINE);
            if (FP.sline > 0) FP.sline--;
            }
        else FP.sline = fo->Line;
        if (ARGFO_ENDLINE)
            {
            FP.eline = *((LONG*)ARGFO_ENDLINE);
            if (FP.eline > 0) FP.eline--;
            FP.flags |= FPDOF_LOOP;
            }
        else if (ARGFO_SET) fo_GetIndentLimit( FP.sline, &FP.eline, ARGFO_ADDSAME );
        else FP.eline = FP.sline;

        if (ARGFO_ALL)
            {
            FP.sline = 0;
            FP.eline = fo->Text.NodeNum - 1;
            FP.flags |= FPDOF_LOOP;
            FP.flags |= FPDOF_ALL;
            }
        else if (ARGFO_SUB)
            {
            fo_GetLimits( FP.sline, -1, &FP.sline, &FP.eline, 1 );
            FP.flags |= FPDOF_LOOP;
            }

        fold_action( &FP );
        }

    if (block || FP.num_act > 1) func_ReDisplay();
    else if (FP.num_act == 1) func_DisplayCmd( &FP.DI );
    else FP.finishline = -1; // func_CursorDraw();

    if (FP.finishline != -1)
        {
        func_CursorPos( FP.finishline, fo->Col );
        if (FP.flags & FPDOF_OFF)
            {
            struct AZObjZone *zo = fo->ActiveView->Zone;
            LONG sline, eline, num=0, newtop=zo->TopLine;
            fo_GetLimits( FP.finishline, -1, &sline, &eline, 0 );
            if ((num = eline - (zo->TopLine + zo->Rows - 2)) > 0) newtop += num;
            if (newtop > FP.finishline) newtop = FP.finishline;
            oview_ChangeViewPosition( fo->ActiveView, newtop, zo->TopCol );
            }
        }
    else func_CursorDraw();

  END_ERROR:
    if (FP.errmsg > 1) intAZ_SetCmdResult( co, FP.errmsg );
    if (AGP.FObj) UPDATE_STAT(AGP.FObj);
}

//»»»»»» Cmd_SetWriteReplace    ON/S,OFF/S,TOGGLE/S

void CmdFunc_SetWriteReplace_Do( struct CmdObj *co )
{
    if (DoOnOffToggle( &co->ArgRes[0], TST_OVERLAY ? -1 : 0 )) SET_OVERLAY; else CLR_OVERLAY;
    if (AGP.FObj) UPDATE_STAT(AGP.FObj);
}

//»»»»»» Cmd_Abbreviate     DO=REPLACE/S,DEF=DEFINE/S,AB=ABBREV,FT=FULLTEXT/F,LOAD/K,SAVE/K

#define ARGA_REPLACE   co->ArgRes[0]
#define ARGA_DEFINE    co->ArgRes[1]
#define ARGA_ABBREV    co->ArgRes[2]
#define ARGA_FULLTEXT  co->ArgRes[3]
#define ARGA_LOAD      co->ArgRes[4]
#define ARGA_SAVE      co->ArgRes[5]

void CmdFunc_Abbreviate_Do( struct CmdObj *co )
{
}
