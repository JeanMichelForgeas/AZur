/****************************************************************
 *
 *      File:       arexx.c
 *      Project:    r�ception de commandes ARexx
 *
 *      Created:    12-11-92     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objarexxsim.h"
#include "uiki:objwindow.h"
#include "uiki:objwfilebut.h"

#include "lci:text.h"
#include "lci:eng_obj.h"
#include "lci:azur.h"
#include "lci:azur_protos.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"

#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"

#include <rexx/storage.h>
#include <rexx/errors.h>

#define PUT_PLAIN_WIN   0x01
#define PUT_ICON_WIN    0x02


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern UBYTE *lo_MakeTotalPath( UBYTE *name );


/****** Exported ***********************************************/


/****** Statics ************************************************/

static ULONG __far demo_count;


/****************************************************************
 *
 *      Master Code
 *
 ****************************************************************/

void Mast_CmdComeBack( struct UIKObjARexxSim *aobj, struct RexxMsg *msg )
{
    if (msg == AGP.RexxConsoleMsg)
        {
        Close( AGP.RexxConsole ); AGP.RexxConsole = 0;
        AGP.RexxConsoleMsg = 0;
        }
}

static ULONG locked_port()
{
  UBYTE *ptr = UIK_LangString( AGP.UIK, TEXT_ERR_ARexxLocked );

    BufSetS( ptr, StrLen(ptr), &AGP.RexxResult );
    AGP.RexxError = TEXT_ERR_ARexxLocked;
    CLR_ABORTAREXX;
    return(20);
}

ULONG Mast_ReceiveARexxCmd( struct UIKObjARexxSim *aobj, UBYTE *cmd )
{
  long quiet;

    /*----on re�oit la commande du script ARexx et on l'execute */

    if (TST_AREXXLOCK || TST_ABORTAREXX)
        {
        if (TST_ABORTAREXX || !func_MatchStr( "Lock", cmd, cmd ))
            aobj->ResultCode = locked_port();
        }

    if (aobj->ResultCode == 0)
        {
        quiet = TST_ERRORQUIET;
        SET_ERRORQUIET;
        aobj->ResultCode = eng_ExecuteAZurCmd( cmd, FROMAREXX | FROMMASTER, 0 );
        if (! quiet) CLR_ERRORQUIET;
        }

    /*----il faut retourner soit une erreur soit un r�sultat:
     *
     * si on veut indiquer un r�sultat:
     *      aobj->ResultCode   = RC_OK;
     *      aobj->ResultString = "400 20 10";
     *
     * si on veut indiquer une erreur:
     *      aobj->ResultCode  = RC_WARN ou RC_ERROR ou RC_FATAL;
     *      aobj->ErrorCode   = ERR10_003;
     *      aobj->ErrorString = "no memory available";
     *
     * Dans un script ARexx, on retrouvera ces informations dans
     * les variables suivantes (mettre OPTIONS RESULTS)
     *      aobj->ResultCode   --> RC
     *      aobj->ErrorCode    --> <nom du port>.LASTERROR
     *      aobj->ErrorString  --> <nom du port>.LASTERRORTEXT
     */

    if (aobj->ResultCode == 0xff) // on garde le message pour syst�me d'exclusion !!!
        {
        return(-1);
        }

    if (aobj->ResultCode)
        {
        aobj->ErrorCode = AGP.RexxError;
        aobj->ErrorString = AGP.RexxResult;
        if (TST_DEBUG)
            {
            UBYTE buf[40];
            sprintf( buf, " -> %ld %ld ", aobj->ResultCode, AGP.RexxError );
            PrintDebug( buf, AGP.RexxResult, 0 );
            }
        }
    else{
        aobj->ResultString = AGP.RexxResult;
        if (TST_DEBUG) PrintDebug( " -> 0 ", AGP.RexxResult, 0 );
        }

    return(0);
}

/****************************************************************
 *
 *      Process Code
 *
 ****************************************************************/

void Proc_CmdComeBack( struct UIKObjARexxSim *aobj, struct RexxMsg *msg )
{
    if (msg == AGP.RexxConsoleMsg)
        {
        Close( AGP.RexxConsole ); AGP.RexxConsole = 0;
        AGP.RexxConsoleMsg = 0;
        }
}

static ULONG demo_fold_callback( ULONG *ptotlen, LONG realline, struct UIKPList *ul, LONG ind )
{
    *ptotlen += (1 + BufLength( ul->UNode[ind].Buf ));
    return(0);
}

ULONG Proc_ReceiveARexxCmd( struct UIKObjARexxSim *aobj, UBYTE *cmd )
{
  long quiet;

    if (TST_DEBUG) PrintDebug( "ARexx cmd : ", cmd, 0 );

    /*----on re�oit la commande du script ARexx et on l'execute */

    if (TST_AREXXLOCK || TST_ABORTAREXX)
        {
        if (TST_ABORTAREXX || !func_MatchStr( "Lock", cmd, cmd ))
            aobj->ResultCode = locked_port();
        }

    if (aobj->ResultCode == 0)
        {
        quiet = TST_ERRORQUIET;
        SET_ERRORQUIET;
        aobj->ResultCode = eng_ExecuteAZurCmd( cmd, FROMAREXX, 0 );
        if (! quiet) CLR_ERRORQUIET;
        }

    /*----il faut retourner soit une erreur soit un r�sultat */

    if (aobj->ResultCode == 0xff) // on garde le message pour le syst�me d'exclusion !!!
        {
        return(-1);
        }

    if (aobj->ResultCode)
        {
        aobj->ErrorCode = AGP.RexxError;
        aobj->ErrorString = AGP.RexxResult;
        if (TST_DEBUG)
            {
            UBYTE buf[40];
            sprintf( buf, " -> %ld %ld ", aobj->ResultCode, AGP.RexxError );
            PrintDebug( buf, AGP.RexxResult, 0 );
            }
        }
    else{
        aobj->ResultString = AGP.RexxResult;
        if (TST_DEBUG) PrintDebug( " -> 0 ", AGP.RexxResult, 0 );
        }

    /*----on met � jour �ventuellement le display */

#ifdef AZUR_DEMO
    if (++demo_count > 50)
        {
        ULONG totlen=0;
        BufTruncate( AGP.UBufTmp, 0 );
        func_FoldApplyLines( (void*)demo_fold_callback, &totlen, &AGP.UBufTmp );
        if (totlen > AZURDEMO_MAXSIZE+AZURDEMO_MAXSIZE/2)
            ExecBase->ThisTask->tc_SigRecvd |= (0x00030000 | SIGBREAKF_CTRL_C); /* pour valeur pas reconnaissable */
        demo_count = 0;
        }
#endif

    if (AGP.FObj->WO && AGP.FObj->WO->Obj.w)
        ofi_TickUpdateDisplay( 0, AGP.FObj );

    return(0);
}

/****************************************************************
 *
 *      Commandes purement ARexx
 *
 ****************************************************************/

static void trunc_result( void )
{
  ULONG len;

    if (len = BufLength( AGP.RexxResult ))
        {
        if (AGP.RexxResult[len-1] == ',') BufTruncate( AGP.RexxResult, len-1 );
        }
}

//========================================================== Get

//������ Cmd_GetResident

void CmdFunc_GetResident_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ls", TST_RESIDENT ? "ON" : "OFF" );
}

//������ Cmd_GetBlockInfo       NUM=ID/N

void CmdFunc_GetBlockInfo_Do( struct CmdObj *co )
{
  ULONG num, id = co->ArgRes[0] ? *((ULONG*)co->ArgRes[0]) : func_BlockCurrent2();
  struct AZBlock *azb;

    if ((num = func_BlockExists()) && (id <= num) && (azb = func_BlockPtr( id )))
        {
        func_BufPrintf( &AGP.RexxResult, 0, "ON %ls %ld %ld %ld %ld",
            (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) ? "ON" : "OFF",
            azb->LineStart+1, azb->ColStart+1, azb->LineEnd+1, azb->ColEnd+1 );
        }
    else{
        func_BufPrintf( &AGP.RexxResult, 0, "OFF OFF 0 0 0 0" );
        }
}

//������ Cmd_GetChar

void CmdFunc_GetChar_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  UBYTE *ptr;
  ULONG len;

    len = intAZ_Buf_Len( fo->Line, &ptr );
    if (fo->Line == fo->Text.NodeNum - 1 && fo->Col >= len)
        {
        intAZ_SetCmdResult( co, TEXT_BadPosition );
        }
    else{
        BufResizeS( &AGP.RexxResult, 1 );
        AGP.RexxResult[0] = intAZ_CharCurrent( fo->Col, len, ptr );
        }
}

//������ Cmd_GetCurrentDir

void CmdFunc_GetCurrentDir_Do( struct CmdObj *co )
{
    lo_GetPath( "", &AGP.RexxResult );
}

//������ Cmd_GetFileInfo

void CmdFunc_GetFileInfo_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ld %ls \"%ls\"",
        AGP.FObj->Text.NodeNum,
        TST_MODIFIED ? "YES" : "NO",
        UIK_BaseName( AGP.FObj->OpenFileName ) );
}

//������ Cmd_GetFilePath

void CmdFunc_GetFilePath_Do( struct CmdObj *co )
{
  ULONG len;

    BufTruncate( AGP.UBufTmp, 0 );
    func_BufPrintf( &AGP.UBufTmp, 0, "%ls", AGP.FObj->OpenFileName );
    /*if (BufLength( AGP.RexxResult ) == 0) comment� pour avoir le path complet */
    /* AGP.FObj->OpenFileName peut �tre vide */
    lo_GetPath( AGP.UBufTmp, &AGP.RexxResult );
    if (len = BufLength( AGP.RexxResult ))
        { if (AGP.RexxResult[len-1] == '/') BufTruncate( AGP.RexxResult, len-1 ); }
    else func_BufPrintf( &AGP.RexxResult, 0, "%ls", AGP.FObj->OpenFileName );
}

//������ Cmd_GetFold        NUM=FOLDNUM/A/N,SUB/S,FSL=FIRSTSUBLINE/S

static ULONG getfold_nums( ULONG *pnumfolds )
{
  struct memfold *mf;
  ULONG i, totfolds;

    BufTruncate( AGP.UBufTmp, 0 );
    func_FoldApplyLines( 0, 0, &AGP.UBufTmp );
    totfolds = BufLength(AGP.UBufTmp) / sizeof(struct memfold);
    mf = (struct memfold *) AGP.UBufTmp;
    for (*pnumfolds=i=0; i < totfolds; i++) { if (mf[i].id == 1) (*pnumfolds)++; }
    return( totfolds );
}

struct _getfold {
    LONG  sline;
    LONG  eline;
    ULONG id;
    ULONG preid;
    UWORD sub;
    UWORD firstsubline;
    };

static ULONG getfold_text( struct _getfold *gf, LONG real, struct UIKPList *list, LONG ind, ULONG nil1, ULONG nil2, ULONG foldid )
{
    if (real > gf->eline)
        return(1); // arr�te ici car apr�s la fourchette

    if (real >= gf->sline)
        {
        //if (foldid < gf->id) return(1); // arr�te ici car on remonte : erreur...
        if (foldid == gf->id || (foldid > gf->id && (gf->sub || (gf->preid == gf->id && gf->firstsubline))))  // si �gal ou si sup�rieur et option SUB
            {
            UBYTE *ub = list->UNode[ind].Buf;
            BufPasteS( ub, BufLength(ub), &AGP.RexxResult, BufLength(AGP.RexxResult) );
            BufPasteS( "\n", 1, &AGP.RexxResult, BufLength(AGP.RexxResult) );
            }
        gf->preid = foldid;
        }
    return(0);
}

void CmdFunc_GetFold_Do( struct CmdObj *co )
{
  struct _getfold GF;
  struct memfold *mf;
  ULONG getnum, numfolds, totfolds;

    totfolds = getfold_nums( &numfolds );

    getnum = *((ULONG*)co->ArgRes[0]);
    if (getnum == 0 || getnum > totfolds) goto END_ERROR;
    getnum--; // de 0 � n-1

    mf = &((struct memfold *)AGP.UBufTmp)[getnum];
    GF.sline = mf->sline;
    GF.eline = mf->eline;
    GF.id    = mf->id;
    GF.preid = 0;
    GF.sub   = (ULONG)co->ArgRes[1];
    GF.firstsubline = (ULONG)co->ArgRes[2];
    BufTruncate( AGP.UBufTmp, 0 );
    func_FoldApplyLines( getfold_text, &GF, 0 );

    return;

  END_ERROR:
    intAZ_SetCmdResult( co, TEXT_BadPosition );
}

//������ Cmd_GetFoldInfo    FULL/S

void CmdFunc_GetFoldInfo_Do( struct CmdObj *co )
{
  struct memfold *mf;
  ULONG i, numfolds, totfolds;

    totfolds = getfold_nums( &numfolds );
    func_BufPrintf( &AGP.RexxResult, 0, "%ld %ld", numfolds, totfolds );
    if (co->ArgRes[0])
        {
        for (mf=(struct memfold *)AGP.UBufTmp, i=0; i < totfolds; i++, mf++)
            if (! func_BufPrintf( &AGP.RexxResult, BufLength(AGP.RexxResult), ",%ld %ld %ld %ld %ld", i+1, mf->id, mf->folded, mf->sline+1, mf->eline+1 )) break;
        }
}

//������ Cmd_GetNumFolds

void CmdFunc_GetNumFolds_Do( struct CmdObj *co )
{
  ULONG numfolds;

    func_BufPrintf( &AGP.RexxResult, 0, "%ld", getfold_nums( &numfolds ) );
}

//������ Cmd_GetLockInfo

void CmdFunc_GetLockInfo_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ls %ls %ls %ls %ls %ls",
        (TST_DISPLOCK   ? "ON" : "OFF"),
        (TST_INPUTLOCK  ? "ON" : "OFF"),
        (TST_READONLY   ? "ON" : "OFF"),
        (TST_SAVELOCK   ? "ON" : "OFF"),
        (TST_STATUSLOCK ? "ON" : "OFF"),
        (TST_AREXXLOCK  ? "ON" : "OFF") );
}

//������ Cmd_GetMasterPort

void CmdFunc_GetMasterPort_Do( struct CmdObj *co )
{
  UBYTE *ptr=0;

    if (AGP.AZMast->Process == AGP.Process) ptr = (UBYTE*) UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_ARexxName, 0, 0 );
    else ptr = (UBYTE*) IPUIK_CallObjectFunc( AGP.AZMast->Process, AGP.AZMast->ARexx, UIKFUNC_ARexxSim_ARexxName, 0, 0 );
    if (ptr) func_BufPrintf( &AGP.RexxResult, 0, "%ls", ptr );
}

//������ Cmd_GetPort        NAME

void CmdFunc_GetPort_Do( struct CmdObj *co )
{
  UBYTE *ptr;

    if (ptr = (UBYTE*) UIK_Call( AGP.ARexx, UIKFUNC_ARexxSim_ARexxName, 0, 0 ))
        func_BufPrintf( &AGP.RexxResult, 0, "%ls", ptr );
}

//������ Cmd_GetPortList    OI=ONLYICONS/S,NI=NOTICONS/S

void get_win_list( struct CmdObj *co, ULONG getname )
{
  struct AZur *mast = AGP.AZMast;
  struct AZurNode *node;
  struct AZurProc *azp;
  ULONG icflags, iconified;
  UBYTE *name;

    icflags = PUT_PLAIN_WIN | PUT_ICON_WIN;
    if (co->ArgRes[0]) icflags &= ~PUT_PLAIN_WIN;
    if (co->ArgRes[1]) icflags &= ~PUT_ICON_WIN;

    ObtainSemaphore( &mast->ProcSem );
    for (node=(struct AZurNode *)mast->ProcList.lh_Head; node->ln_Succ; node = (struct AZurNode *)node->ln_Succ)
        {
        azp = (struct AZurProc *) (*((ULONG*)&node->ln_Task->tc_Node.ln_Name[8]));
        iconified = 0;
        if (azp->FObj->WO) iconified = (azp->FObj->WO->Obj.Status == UIKS_ICONIFIED) ? 1 : 0;

        if ((!iconified && (icflags & PUT_PLAIN_WIN)) || (iconified && (icflags & PUT_ICON_WIN)))
            {
            if (! getname)
                {
                if (! func_BufPrintf( &AGP.RexxResult, BufLength(AGP.RexxResult), "\"%ls\",", node->ln_PortName )) break;
                }
            else if (name = azp->FObj->OpenFileName)
                {
                if (! func_BufPrintf( &AGP.RexxResult, BufLength(AGP.RexxResult), "\"%ls\",\"%ls\",", UIK_BaseName(name), node->ln_PortName )) break;
                }
            }
        }
    ReleaseSemaphore( &mast->ProcSem );
    trunc_result();
}

void CmdFunc_GetPortList_Do( struct CmdObj *co )
{
    get_win_list( co, 0 );
}

//������ Cmd_GetWinList     OI=ONLYICONS/S,NI=NOTICONS/S

void CmdFunc_GetWinList_Do( struct CmdObj *co )
{
    get_win_list( co, 1 );
}

//������ Cmd_GetPos         LINE/S,COL=COLUMN/S,IN=INFOLD/S,FOLDS/S

static LONG fold_getline( UBYTE *lineptr, LONG realline, struct UIKPList *ul, LONG ind )
{
    if (lineptr == ul->UNode[ind].Buf) return( realline + 1 );
    return(0);
}

void CmdFunc_GetPos_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  struct UIKNode *node = &fo->Text.UNode[fo->Line];
  ULONG len;
  LONG line = fo->Line + 1;
  STRPTR foldptr = "OFF";

    if (co->ArgRes[3]) // FOLDS
        line = func_FoldApplyLines( (void*)fold_getline, node->Buf, 0 );
    if ((node->Flags & UNF_FOLDED) || node->Reserved)
        foldptr = "ON";

    if (co->ArgRes[2]) func_BufPrintf( &AGP.RexxResult, 0, "%ls", foldptr );
    if (co->ArgRes[1]) func_BufPrintf( &AGP.RexxResult, 0, "%ld ", fo->Col+1 );
    if (co->ArgRes[0]) func_BufPrintf( &AGP.RexxResult, 0, "%ld ", line );
    if (len = BufLength( AGP.RexxResult ))
        {
        if (AGP.RexxResult[len-1] == ' ') BufTruncate( AGP.RexxResult, len-1 );
        }
    else func_BufPrintf( &AGP.RexxResult, 0, "%ld %ld %ls", line, fo->Col+1, foldptr );
}

//������ Cmd_GetPri

void CmdFunc_GetPri_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ld", (ULONG)AGP.Process->pr_Task.tc_Node.ln_Pri );
}

//������ Cmd_GetReadonly

void CmdFunc_GetReadonly_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ls", (TST_READONLY ? "ON" : "OFF") );
}

//������ Cmd_GetError       ERROR/N,LAST/S

void CmdFunc_GetError_Do( struct CmdObj *co )
{
  ULONG num=0;

    if (co->ArgRes[0]) num = *((ULONG*)co->ArgRes[0]);
    else if (co->ArgRes[1]) num = (ULONG)AGP.LastError;
    if (num && num < LAST_TEXT_INDEX && num > 0)
        func_BufPrintf( &AGP.RexxResult, 0, "(%ld) %ls", intAZ_GetErrorSeverity(num), UIK_LangString( AGP.UIK, num ) );
}

//������ Cmd_GetBlock       NUM=ID/N,ALL/S

extern ULONG bloc_2_buf( struct AZBlock *azb, ULONG id, UIKBUF **pub );

void CmdFunc_GetBlock_Do( struct CmdObj *co )
{
  ULONG num, id = co->ArgRes[0] ? *((ULONG*)co->ArgRes[0]) : func_BlockCurrent();
  struct AZBlock *azb;

    if ((num = func_BlockExists()) && (id <= num) && (azb = func_BlockPtr( id )))
        {
        if (co->ArgRes[1]) // ALL
            {
            func_BlockApply( bloc_2_buf, 1, -1, 0, &AGP.RexxResult );
            }
        else{
            bloc_2_buf( azb, id, &AGP.RexxResult );
            }
        }
}

//������ Cmd_GetFontInfo

void CmdFunc_GetFontInfo_Do( struct CmdObj *co )
{
  struct AZurPrefs *p = AGP.Prefs;
  struct Screen *sc;

    if (AGP.WO && AGP.WO->Obj.w) sc = AGP.WO->Obj.w->WScreen;

    func_BufPrintf( &AGP.RexxResult, 0, "%ls %ld %ls %ld %ls %ld",
        p->ViewFontName, p->ViewFontHeight,
        p->FontName, p->FontHeight,
        sc ? sc->Font->ta_Name : "OFF",
        sc ? sc->Font->ta_YSize : 0 );
}

//������ Cmd_GetLine        STARTCOL/N,ENDCOL/N,LINE/N

void CmdFunc_GetLine_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG startcol, endcol, line;
  UBYTE *ptr;
  ULONG len;

    line = fo->Line;
    if (co->ArgRes[2])
        {
        line = *((ULONG*)co->ArgRes[2]);
        if (line == -1) line = fo->Text.NodeNum;
        line--;  // car user = +1
        }

    if (line < fo->Text.NodeNum)
        {
        len = intAZ_Buf_Len( line, &ptr );

        if (len)
            {
            startcol = 0;
            if (co->ArgRes[0])
                {
                startcol = *((ULONG*)co->ArgRes[0]);
                if (startcol == -1) startcol = len;
                startcol--;
                }
            if (startcol < len)
                {
                endcol = len - 1;
                if (co->ArgRes[1])
                    {
                    endcol = *((ULONG*)co->ArgRes[1]);
                    if (endcol == -1) endcol = len;
                    endcol--;
                    }

                if (startcol >= len) startcol = len - 1;
                if (endcol >= len) endcol = len - 1;
                if (endcol < startcol) endcol = startcol;
                len = endcol - startcol + 1;
                if (BufResizeS( &AGP.RexxResult, len ))
                    CopyMem( &ptr[startcol], AGP.RexxResult, len );
                }
            }
        }
}

//������ Cmd_GetLineInfo    LINE/N

void CmdFunc_GetLineInfo_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG line;
  UBYTE *ptr;
  ULONG len, num;

    line = fo->Line;
    if (co->ArgRes[0])
        {
        line = *((ULONG*)co->ArgRes[0]);
        if (line == -1) line = fo->Text.NodeNum;
        line--;  // car user = +1
        }

    if (line < fo->Text.NodeNum)
        {
        len = intAZ_Buf_Len( line, &ptr );
        num = skip_char( ptr, '\x20', '\t' );
        func_BufPrintf( &AGP.RexxResult, 0, "%ld %ld %ld %ls %ls",
            len,
            num,
            fo_GetLineId( line ),
            fo_IsLineFolded( line ) ? "ON" : "OFF",
            ptr );
        }
    else intAZ_SetCmdResult( co, TEXT_BadPosition );
}

//������ Cmd_GetWord        LINE/N,COL=COLUMN/N,PREV/S,NEXT/S,MC=MOVECURS/S

void CmdFunc_GetWord_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG scol, ecol, line;
  UBYTE *ptr, ch;
  ULONG len;

    line = fo->Line;
    if (co->ArgRes[0])
        {
        line = *((ULONG*)co->ArgRes[0]);
        if (line == -1) line = fo->Text.NodeNum;
        line--;  // car user = +1
        }
    len = intAZ_Buf_Len( line, &ptr );
    ecol = fo->Col;
    if (co->ArgRes[1])
        {
        ecol = *((ULONG*)co->ArgRes[1]);
        if (ecol == -1) ecol = len;
        else ecol--;  // car user = +1
        }

    if (! func_CharIsWord( intAZ_CharCurrent( ecol, len, ptr ) ))
        {
        UBYTE *(*func)();
        if (co->ArgRes[2]) func = func_CharPrev; else if (co->ArgRes[3]) func = func_CharNext; else return;
        while (TRUE)
            {
            if  ((ptr = func( ptr, &len, &line, &ecol, &ch )) == (APTR)-1) return;
            if (func_CharIsWord( ch )) break;
            }
        }
    scol = ecol;
    for (; scol>0; scol--) if (! func_CharIsWord( ptr[scol-1] )) break;
    for (; ecol<len; ecol++) if (! func_CharIsWord( ptr[ecol+1] )) break;
    len = ecol - scol + 1;
    /*
    if (ResizeS( AGP.RexxResult, len ))
        CopyMem( &ptr[scol], AGP.RexxResult, len );
    */
    BufSetS( &ptr[scol], len, &AGP.RexxResult );
    if (co->ArgRes[4]) func_CursorPos( line, scol ); // MOVECURS
}

//������ Cmd_GetModified

void CmdFunc_GetModified_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ls", TST_MODIFIED ? "ON" : "OFF" );
}

//������ Cmd_GetNumBlocks

void CmdFunc_GetNumBlocks_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ld", func_BlockExists() );
}

//������ Cmd_GetNumLines

void CmdFunc_GetNumLines_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ld", AGP.FObj->Text.NodeNum );
}

//������ Cmd_GetVarList     PAT=PATTERN/S

void CmdFunc_GetVarList_Do( struct CmdObj *co )
{
    func_GetVarList( &AGP.RexxResult );
}

//������ Cmd_GetVar         NAME/A

void CmdFunc_GetVar_Do( struct CmdObj *co )
{
  APTR var;
  UBYTE *name = co->ArgRes[0];

    if (! name[0]) return;

    if (var = func_VarLock( name ))
        {
        BufSetS( var, BufLength( var ), &AGP.RexxResult );
        func_VarUnlock( name );
        }
    else BufTruncate( AGP.RexxResult, 0 );
}

//������ Cmd_GetScreenInfo      NOBAR/S

void CmdFunc_GetScreenInfo_Do( struct CmdObj *co )
{
  struct Window *w;
  struct Screen *sc;
  LONG cw, ch, depth;

    if (get_winicon_ptrs( AGP.FObj->WO, &w ))
        {
        sc = w->WScreen;
        cw = sc->Width; ch = sc->Height;
        depth = sc->RastPort.BitMap->Depth;

        if (UIK_IsSystemVersion2())
            {
            struct DimensionInfo DI;
            ULONG modeid;

            if ((modeid = GetVPModeID( &sc->ViewPort )) != INVALID_ID)
                if (GetDisplayInfoData( NULL, (UBYTE *) &DI, sizeof(struct DimensionInfo), DTAG_DIMS, modeid ))
                    {
                    cw = DI.TxtOScan.MaxX - DI.TxtOScan.MinX + 1;
                    ch = DI.TxtOScan.MaxY - DI.TxtOScan.MinY + 1;
                    }
            }

        if (((struct Library *)GfxBase)->lib_Version >= 39)
            depth = GetBitMapAttr( sc->RastPort.BitMap, BMA_DEPTH );

        if (co->ArgRes[0])
            func_BufPrintf( &AGP.RexxResult, 0, "%ld %ld %ld %ld %ld %ld %ld",
                sc->LeftEdge, sc->TopEdge, sc->Width, sc->Height, depth, cw, ch );
        else
            func_BufPrintf( &AGP.RexxResult, 0, "%ld %ld %ld %ld %ld %ld %ld %ld",
                sc->BarHeight+1, sc->LeftEdge, sc->TopEdge, sc->Width, sc->Height, depth, cw, ch );
        }
}

//������ Cmd_GetPubScreen

void CmdFunc_GetPubScreen_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ls", func_GetPubScreenName() );  /* cmd_exe.c */
}

//������ Cmd_GetScreenList      PUBLIC/S

static void __asm get_scr_title( register __a0 struct Screen *sc, register __a1 UBYTE *title )
{
    if (! title)
        if (! (title = sc->Title))
            title = sc->DefaultTitle;
    if (title)
        func_BufPrintf( &AGP.RexxResult, BufLength(AGP.RexxResult), "\"%ls\",", title );
}

void CmdFunc_GetScreenList_Do( struct CmdObj *co )
{
  ULONG ilock;
  struct Screen *sc;
  struct List *sclist;
  struct PubScreenNode *node;

    if (co->ArgRes[0] && UIK_IsSystemVersion2())
        {
        sclist = LockPubScreenList();
        for (node=(struct PubScreenNode *)sclist->lh_Head; node->psn_Node.ln_Succ; node = (struct PubScreenNode *)node->psn_Node.ln_Succ)
            {
            get_scr_title( node->psn_Screen, node->psn_Node.ln_Name );
            }
        UnlockPubScreenList();
        }
    else{
        ilock = LockIBase( 0 );
        sc = IntuitionBase->FirstScreen;
        while (sc)
            {
            get_scr_title( sc, 0 );
            sc = sc->NextScreen;
            }
        UnlockIBase( ilock );
        }
    trunc_result();
}

//������ Cmd_GetTaskInfo        HEX/S

void CmdFunc_GetTaskInfo_Do( struct CmdObj *co )
{
  UBYTE dest[100], *name, *fmt = co->ArgRes[0] ? "%ls \"%ls\" %08.8lx %ld" : "%ls \"%ls\" %ld %ld";
  struct Process *me = AGP.Process;
  struct CommandLineInterface *cli;

    name = AGP.Process->pr_Task.tc_Node.ln_Name;

    if (me->pr_TaskNum)
        {
        if (cli = (struct CommandLineInterface *) BADDR(me->pr_CLI))
            {
            StrB2C( (ULONG)BADDR(cli->cli_CommandName), dest );
            name = dest;
            }
        }
    func_BufPrintf( &AGP.RexxResult, 0, fmt,
        (me == AGP.AZMast->Process) ? "MASTER" : "EDIT",
        name,
        me,
        (ULONG)AGP.Process->pr_Task.tc_Node.ln_Pri
        );
}

//������ Cmd_GetText            SL=STARTLINE/A/N,SC=STARTCOL/A/N,EL=ENDLINE/A/N,EC=ENDCOL/A/N,RECT=VERT/S

#define ARG_STARTLINE   co->ArgRes[0]
#define ARG_STARTCOL    co->ArgRes[1]
#define ARG_ENDLINE     co->ArgRes[2]
#define ARG_ENDCOL      co->ArgRes[3]
#define ARG_RECT        co->ArgRes[4]

static ULONG text_buf( int nil, UBYTE *buf, ULONG len )
{
    if (len)
        if (! BufPasteS( buf, len, &AGP.RexxResult, BufLength(AGP.RexxResult) )) return( TEXT_NOMEMORY );
    return(0);
}

void CmdFunc_GetText_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG sline, scol, eline, ecol;
  ULONG errmsg = 0;

    sline = *((LONG*)ARG_STARTLINE);
    eline = *((LONG*)ARG_ENDLINE);
    scol  = *((LONG*)ARG_STARTCOL);
    ecol  = *((LONG*)ARG_ENDCOL);

    sline--;
    if (eline != -1) eline--;
    if (scol != -1) scol--;
    if (ecol != -1) ecol--;

    if (eline == -1) eline = fo->Text.NodeNum;
    if (eline > fo->Text.NodeNum - 1) eline = fo->Text.NodeNum - 1;

    if ((scol < 0 && scol != -1) || (ecol < 0 && ecol != -1) || sline < 0 || eline < 0 || sline > eline)
        {
        errmsg = TEXT_ERR_BadValue;
        }
    else{
        if (!ARG_RECT || scol <= ecol)
            errmsg = func_TextApply( sline, scol, eline, ecol, ARG_RECT?1:0, 0, text_buf, 0 );
        }

  END_ERROR:
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//������ Cmd_GetViewInfo    NUM=VIEWNUM/N

void CmdFunc_GetViewInfo_Do( struct CmdObj *co )
{
  struct UIKObjWindow *wo;
  struct Window *w;
  struct Node *node;
  struct AZObjView *vo;
  struct AZObjZone *zo;
  LONG viewnum, num;

    if (wo = (struct UIKObjWindow *) get_winicon_ptrs( AGP.FObj->WO, &w ))
        {
        viewnum = (co->ArgRes[0] ? *((ULONG*)co->ArgRes[0]) : 0);
        if (viewnum)
            {   // cherche la vue correspondant au num�ro donn� (1 � n)
            for (num=1, node=AGP.FObj->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
                {
                if (num > viewnum) break;
                vo = (struct AZObjView *)node->ln_Name;
                num++;
                }
            }
        else{   // cherche le num�ro de la vue active
            vo = AGP.FObj->ActiveView;
            for (viewnum=1, node=AGP.FObj->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
                {
                if (vo == (struct AZObjView *)node->ln_Name) break;
                viewnum++;
                }
            }
        zo = vo->Zone;
        func_BufPrintf( &AGP.RexxResult, 0, "%ld %ld %ld %ld %ld",
            viewnum, zo->Rows, zo->Cols, zo->TopLine+1, zo->TopCol+1 );
        }
}

//������ Cmd_GetWinBorders

void CmdFunc_GetWinBorders_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  struct UIKObjWindow *wo;
  struct Window *w;

    if (wo = (struct UIKObjWindow *) get_winicon_ptrs( fo->WO, &w ))
        {
        func_BufPrintf( &AGP.RexxResult, 0, "%ld %ld %ld %ld %ld %ld %ld %ld",
            wo->LeftBorderWidth, wo->RightBorderWidth, wo->TopBorderHeight, wo->BottomBorderHeight,
            fo->Stat ? fo->Stat->Box.Height : 0,
            fo->CmdLine ? fo->CmdLine->Box.Height : 0,
            (AGP.Prefs->EleMask & BUMF_RIGHTBORDER) ? fo->VI.VBorderWidth : 0,
            (AGP.Prefs->EleMask & BUMF_BOTTOMBORDER) ? fo->VI.HBorderHeight : 0 );
        }
}

//������ Cmd_GetWinInfo     WINDOW/S

void CmdFunc_GetWinInfo_Do( struct CmdObj *co )
{
  struct UIKObjWindow *wo;
  struct Window *w;
  struct Node *node;
  LONG num;
  WORD left, top, width, height;

    if (wo = AGP.WO) // = (struct UIKObjWindow *) get_winicon_ptrs( AGP.FObj->WO, &w ))
        {
        for (num=0, node=AGP.FObj->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ) num++;

        if (w = wo->Obj.w)
            {
            left = w->LeftEdge;
            top = w->TopEdge;
            width = w->Width;
            height = w->Height;
            }
        else{
            left = wo->NW.LeftEdge;
            top = wo->NW.TopEdge;
            width = wo->NW.Width;
            height = wo->NW.Height;
            }

        if (co->ArgRes[0] == 0)
            {
            if (wo = (struct UIKObjWindow *) get_winicon_ptrs( AGP.FObj->WO, &w ))
                {
                if (w = wo->Obj.w)
                    {
                    left = w->LeftEdge;
                    top = w->TopEdge;
                    width = w->Width;
                    height = w->Height;
                    }
                else{
                    left = wo->NW.LeftEdge;
                    top = wo->NW.TopEdge;
                    width = wo->NW.Width;
                    height = wo->NW.Height;
                    }
                }
            }

        if (wo)
            {
            func_BufPrintf( &AGP.RexxResult, 0, "%ls %ld %ld %ld %ld %ld %ld %ld",
                (AGP.WO->Obj.Status == UIKS_ICONIFIED) ? "ON" : "OFF",
                left, top, width,height,
                wo->NW.MinWidth, wo->NW.MinHeight, num );
            }
        }
}

//������ Cmd_GetNumBM

void CmdFunc_GetNumBM_Do( struct CmdObj *co )
{
    func_BufPrintf( &AGP.RexxResult, 0, "%ld", func_BMExists() );
}

//������ Cmd_GetBMInfo          NAME=BOOKMARK,NUM=ID/N/K

void CmdFunc_GetBMInfo_Do( struct CmdObj *co )
{
  struct AZBM *azb;
  ULONG id;

    if (! func_BMExists()) { intAZ_SetCmdResult( co, TEXT_NoBMDefined ); return; }

    if (co->ArgRes[0])  // NAME
        {
        id = func_BMGetId( co->ArgRes[0], 0, 0 );
        }
    else if (co->ArgRes[1]) // NUM
        {
        id = *((ULONG*)co->ArgRes[1]);
        }
    else id = 1;

    if (azb = (struct AZBM *) func_BMGetPtr( id ))
        {
        func_BufPrintf( &AGP.RexxResult, 0, "\"%ls\",%ld,%ld,%ld,", azb->Name, id, azb->Line+1, azb->Col+1  );
        }
}

//������ Cmd_GetBMList

ULONG bm_2_buf( struct AZBM *azb, ULONG id, UIKBUF **pub )
{
  ULONG errmsg = 0;

    if (! func_BufPrintf( pub, BufLength(*pub), "\"%ls\",", azb->Name ))
        errmsg = TEXT_NOMEMORY;
    return( errmsg );
}

void CmdFunc_GetBMList_Do( struct CmdObj *co )
{
  ULONG errmsg;


    if (errmsg = func_BMApply( bm_2_buf, 1, -1, 0, &AGP.RexxResult ))
        intAZ_SetCmdResult( co, errmsg );
    trunc_result();
}

//========================================================== Set

//������ Cmd_SetVar         NAME/A,WITH=CONTENTS/F

void CmdFunc_SetVar_Do( struct CmdObj *co )
{
  APTR var;
  UBYTE *name = co->ArgRes[0];
  UBYTE *with = co->ArgRes[1];
  ULONG len;

    if (! name[0]) return;

    if (! with) with = "";
    if (len = StrLen( with ))   //------ Si qlq chose � mettre...
        {
        if ((var = func_VarLockResize( name, len )) == (APTR)-1)
            {                                   //------ si pas dans la liste
            Forbid();
            if ((var = func_VarLockResize( name, len )) == (APTR)-1)
                var = func_VarNew( name, len ); //------ si pas dans la liste on la cr�e
            Permit();
            }

        if (var)
            {
            CopyMem( with, var, len );
            func_VarUnlock( name );
            }
        else intAZ_SetCmdResult( co, TEXT_NOMEMORY );
        }
    else func_VarDispose( name ); //------ Si rien � y mettre on la supprime
}

//--------------------------------------------------------------------------------------

//������ Cmd_SetModified        ON/S,OFF/S,TOGGLE/S

void CmdFunc_SetModified_Do( struct CmdObj *co )
{
    if (DoOnOffToggle( &co->ArgRes[0], TST_MODIFIED ? -1 : 0 )) AGP.FObj->Flags |= AZFILEF_MODIFIED; else CLR_MODIFIED();
    func_DisplayStats(0);
}

//������ Cmd_SetCompress        ON/S,OFF/S,TOGGLE/S

void CmdFunc_SetCompress_Do( struct CmdObj *co )
{
    if (DoOnOffToggle( &co->ArgRes[0], TST_COMPRESSED ? -1 : 0 )) SET_COMPRESSED; else CLR_COMPRESSED;
    func_DisplayStats(0);
}

//������ Cmd_SetResident        ON/S,OFF/S,TOGGLE/S

void CmdFunc_SetResident_Do( struct CmdObj *co )
{
    if (DoOnOffToggle( &co->ArgRes[0], TST_RESIDENT ? -1 : 0 )) SET_RESIDENT; else CLR_RESIDENT;
    func_DisplayStats(0);
}

//������ Cmd_SetCurrentDir      NAME

extern void ofi_FS_OKFunc();
extern void ofi_FS_CancelFunc();

void CmdFunc_SetCurrentDir_Do( struct CmdObj *co )
{
  UBYTE *name = co->ArgRes[0];
  ULONG errmsg = 0;
  struct UIKObj *wfs=0;

    if (name == 0 && AGP.WO)
        {
        if (wfs = (struct UIKObjWinFileSel *) UIK_AddObjectTagsB( UIKBase, "UIKObj_WinFileSel", AGP.WO,
                UIKTAG_OBJ_Title,               TEXT_Title_SelectDir,
                UIKTAG_OBJ_FontName,            AGP.Prefs->ViewFontName,
                UIKTAG_OBJ_FontHeight,          AGP.Prefs->ViewFontHeight,
                UIKTAG_WFS_OKFunc,              ofi_FS_OKFunc,
                UIKTAG_WFS_CancelFunc,          ofi_FS_CancelFunc,
                UIKTAG_WFS_InitPath,            "",
                UIKTAG_WFSFl_ReturnReturn,      TRUE,
                UIKTAG_WFSFl_DirsOnly,          TRUE,
                UIKTAG_WFSFl_LeavePath,         FALSE,
                TAG_END ))
            {
            if (UIK_Start( wfs ))
                {
                if (sync_request( wfs ) == REQBUT_OK)
                    name = AGP.FileSelResult;
                else errmsg = TEXT_CMDCANCELLED;
                }
            else errmsg = TEXT_NOMEMORY;
            }
        else errmsg = TEXT_CMDCANCELLED;
        }

    if (!errmsg)
        {
        if (! name) errmsg = TEXT_ERR_InvalidDir;
        else if (! lo_SetCurDir( name )) errmsg = TEXT_ERR_InvalidDir;
        }

    if (errmsg) intAZ_SetCmdResult( co, errmsg );
    else if (AGP.WO && AGP.FObj)
        {
        UIK_Call( AGP.FObj->OpenFS->FS, UIKFUNC_FS_ChangePath, (ULONG)name, 0 );
        UIK_Call( AGP.FObj->SaveFS->FS, UIKFUNC_FS_ChangePath, (ULONG)name, 0 );
        }
}

//������ Cmd_SetDebug           ON/S,OFF/S,TOGGLE/S

void CmdFunc_SetDebug_Do( struct CmdObj *co )
{
    if (DoOnOffToggle( &co->ArgRes[0], TST_DEBUG ? -1 : 0 )) SET_DEBUG; else CLR_DEBUG;
    if (TST_DEBUG && !AGP.DOSConsole)
        AGP.DOSConsole = Open( AGP.Prefs->DOSConsoleDesc, MODE_NEWFILE );
    else if (AGP.DOSConsole) { Close( AGP.DOSConsole ); AGP.DOSConsole = 0; }
    func_DisplayStats(0);
}

//������ Cmd_SetFilePath        NAME/A

void CmdFunc_SetFilePath_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  UBYTE *ub = 0, *name = co->ArgRes[0];
  ULONG errmsg = 0;

    if (ub = lo_MakeTotalPath( name )) name = ub;
    if (lo_SetCurDir( name ))
        {
        if (! BufSetS( name, StrLen(name), &fo->OpenFileName )) errmsg = TEXT_NOMEMORY;
        }
    else errmsg = TEXT_ERR_InvalidDir;

    if (!errmsg && AGP.WO)
        {
        UIK_Call( fo->WO, UIKFUNC_Window_ChangeTitle, (ULONG)UIK_BaseName( name ), 0 );
        func_DisplayStats(1);
        UIK_Call( fo->OpenFS->FS, UIKFUNC_FS_ChangePath, (ULONG)name, 0 );
        UIK_Call( fo->SaveFS->FS, UIKFUNC_FS_ChangePath, (ULONG)name, 0 );
        }

    BufFree( ub );
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//������ Cmd_SetPri         PRIORITY/N

void CmdFunc_SetPri_Do( struct CmdObj *co )
{
  ULONG errmsg=0;
  LONG pri;

    if (co->ArgRes[0])
        {
        pri = *((LONG*)co->ArgRes[0]);
        }
    else{
        if (AGP.WO)
            {
            BufTruncate( AGP.UBufTmp, 0 );
            BufTruncate( AGP.FObj->ColString, 0 );
            func_BufPrintf( &AGP.UBufTmp, 0, UIK_LangString( AGP.UIK, TEXT_PriorityText ), (ULONG)AGP.Process->pr_Task.tc_Node.ln_Pri );

            if (REQBUT_OK == func_IntSmartRequest( 0, TEXT_Title_Priority,0, 0,AGP.UBufTmp, 0,&AGP.FObj->ColString, 0,0, -1,0,-1, 0, ARF_ACTIVATE, 0, 0 ))
                {
                if (ARG_StrToLong( AGP.FObj->ColString, &pri ) <= 0) errmsg = TEXT_ERR_InvalidPri;
                }
            else errmsg = TEXT_CMDCANCELLED;
            }
        else errmsg = TEXT_ERR_InvalidPri;
        }

    if (! errmsg)
        {
        if (pri < -128 || pri > 127) errmsg = TEXT_ERR_InvalidPri;
        else SetTaskPri( FindTask(0), pri );
        }

    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

