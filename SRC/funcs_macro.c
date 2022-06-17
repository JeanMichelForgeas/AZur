/****************************************************************
 *
 *      File:      funcs_macro.c
 *      Project:   AZur
 *
 *      Created:   03-12-93     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:objwindow.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:azur.h"
#include "lci:eng_obj.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:azur_protos.h"
#include "lci:objfile.h"

struct AZMacroInfo
    {
    struct CmdObj   **COTable;
    ULONG           Count;
    };


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;  // data relative base address pour le compilo


/****** Exported ***********************************************/

void func_MacroReset( void );
void func_MacroEndRecord( void );


/****** Statics ************************************************/


/****************************************************************
 *
 *      code
 *
 ****************************************************************/

ULONG func_MacroExists( void )
{
    return( AGP.FObj->MacInfo->Count );
}

void func_MacroReset( void )
{
  struct AZMacroInfo *mi = AGP.FObj->MacInfo;
  ULONG i;

    if (mi->Count)
        {
        for (i=0; i < mi->Count; i++) eng_DisposeObject( mi->COTable[i] );
        BufSetS( "", 0, (APTR)&mi->COTable );
        mi->Count = 0;
        }
}

void func_MacroDelete( void )
{
  struct AZMacroInfo *mi = AGP.FObj->MacInfo;

    if (mi->Count)
        {
        mi->Count--;
        BufResizeS( (APTR)&mi->COTable, mi->Count * 4 );
        }
}

ULONG func_MacroAdd( UBYTE *cmdname, UBYTE *cmdline )
{
  struct AZMacroInfo *mi = AGP.FObj->MacInfo;
  UBYTE *ptr;
  ULONG quiet, errmsg=0, len2;
  struct CmdObj *co;

    if (! cmdline) cmdline = "";
    if (BufResizeS( (APTR)&mi->COTable, (mi->Count + 1) * 4 ))
        {
        len2 = StrLen( cmdline );
        if (ptr = BufAllocP( AGP.Pool, len2, 4 ))
            {
            StrCpy( ptr, cmdline );
            quiet = TST_ERRORQUIET;
            SET_ERRORQUIET;
            if (co = eng_NewObjectTags( cmdname, AZT_CmdLine, ptr, AZT_ExtraBuf, ptr, AZT_ExeFrom, FROMMACRO, TAG_END ))
                {
                if (co->Flags & AZAFF_OKINMACRO)
                    {
                    mi->COTable[mi->Count] = co;
                    mi->Count++;
                    }
                else{
                    eng_DisposeObject( co );
                    errmsg = TEXT_ExeObj_Macro;
                    }
                }
            else{
                errmsg = AGP.RexxError;
                BufFree( ptr ); // libérer seulement si objet pas créé
                }
            if (! quiet) CLR_ERRORQUIET;
            }
        else errmsg = TEXT_NOMEMORY;

        // (pas utile car mi->Count) if (errmsg) BufResizeS( (APTR)&mi->COTable, mi->Count * 4 );
        }
    else errmsg = TEXT_NOMEMORY;
    return( errmsg );
}

/*
ULONG func_MacroAdd( UBYTE *cmdname, UBYTE *cmdline )
{
  struct AZMacroInfo *mi = AGP.FObj->MacInfo;
  UBYTE *ptr;
  ULONG quiet, errmsg=0, len1, len2;
  struct CmdObj *co;

    if (! cmdline) cmdline = "";
    if (BufResizeS( (APTR)&mi->COTable, (mi->Count + 1) * 4 ))
        {
        len1 = StrLen( cmdname );
        len2 = StrLen( cmdline );
        if (ptr = BufAllocP( AGP.Pool, len1+len2+2, 4 ))
            {
            StrCpy( ptr, cmdname );
            StrCpy( ptr+len1+1, cmdline );

            quiet = TST_ERRORQUIET;
            SET_ERRORQUIET;
            if (co = eng_NewObjectTags( ptr, AZT_CmdLine, ptr+len1+1, AZT_ExtraBuf, ptr, AZT_ExeFrom, FROMMACRO, TAG_END ))
                {
                if (co->Flags & AZAFF_OKINMACRO)
                    {
                    mi->COTable[mi->Count] = co;
                    mi->Count++;
                    }
                else{
                    eng_DisposeObject( co );
                    errmsg = TEXT_ExeObj_Macro;
                    }
                }
            else{
                errmsg = AGP.RexxError;
                BufFree( ptr ); // libérer seulement si objet pas créé
                }
            if (! quiet) CLR_ERRORQUIET;
            }
        else errmsg = TEXT_NOMEMORY;

        // (pas utile car mi->Count) if (errmsg) BufResizeS( (APTR)&mi->COTable, mi->Count * 4 );
        }
    else errmsg = TEXT_NOMEMORY;
    return( errmsg );
}
*/

//---------------------------------------------------------

static void ReplyIntuiMsg( struct Message *msg )
{
    if (msg->mn_ReplyPort) ReplyMsg( msg );
    else UIK_MemSpFree( msg );
}

ULONG StripWindowEventsMacro()
{
  struct List *list;
  struct Node *succ, *node;
  struct Window *w;
  ULONG escape=0, cont=1;

    if (AGP.WO)
        {
        list = (struct List *) &AGP.UIK->IMSList;
        w = AGP.WO->Obj.w;

        Disable();
        for (node=list->lh_Head; cont && (succ = node->ln_Succ); node = succ)
            {
            if (((struct IntuiMessage *)node)->IDCMPWindow == w)
                {
                if (((struct IntuiMessage *)node)->Class == RAWKEY)
                    {
                    if (((struct IntuiMessage *)node)->Code == (0x45)) { escape = 1; cont=0; }
                    Remove( node );
                    ReplyIntuiMsg( (struct Message *)node );
                    }
                }
            }
        Enable();
        }
    return( escape );
}

ULONG func_MacroPlay( ULONG num, ULONG chgptr )
{
  struct AZMacroInfo *mi = AGP.FObj->MacInfo;
  ULONG errmsg=0, n, i;

    if (chgptr) { func_SetPlayPointer(); AGP.WorkFlags |= WRF_MAC_PTRLOCK; }
    AGP.WorkFlags |= WRF_MAC_PLAYING;
    for (n=0; num==0 || n < num; n++)
        {
        if (StripWindowEventsMacro()) goto END_ERROR;

        for (i=0; i < mi->Count; i++)
            {
            if ((SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D) & SetSignal( 0, 0 )) goto END_ERROR;
            if (func_ExecuteCmdObj( mi->COTable[i], 0 ))
                {                              // Le intAZ_SetCmdResult est déjà fait par la commande
                errmsg = (ULONG)AGP.RexxError; // mais MacPlay à son propre ResCode, donc remettre l'erreur
                goto END_ERROR;
                }
            }
        }
  END_ERROR:
    if (chgptr) { AGP.WorkFlags &= ~(WRF_MAC_PTRLOCK); func_SetNormalPointer(); }
    AGP.WorkFlags &= ~(WRF_MAC_PLAYING);
    return( errmsg ); // erreur déjà mise par la commande
}

ULONG func_MacroApply( ULONG (*func)(), APTR parm )
{
  struct AZMacroInfo *mi = AGP.FObj->MacInfo;
  ULONG i, errmsg=0;

    for (i=0; i < mi->Count; i++)
        if (errmsg = (*func)( mi->COTable[i], parm )) break;
    return( errmsg );
}

//---------------------------------------------------------

ULONG func_MacroRecord( ULONG quiet, ULONG chgptr )
{
    func_MacroReset();
    AGP.WorkFlags |= WRF_MAC_RECORDING;
    if (quiet) AGP.WorkFlags |= WRF_MAC_QUIET;
    if (chgptr) { func_SetRecordPointer(); AGP.WorkFlags |= WRF_MAC_PTRLOCK; }
    return(0);
}

void func_MacroEndRecord( void )
{
    if (! (AGP.WorkFlags & WRF_MAC_RECORDING)) return;
    AGP.WorkFlags &= ~(WRF_MAC_RECORDING | WRF_MAC_QUIET | WRF_MAC_PTRLOCK);
    func_SetNormalPointer();
}

//================================================================

ULONG func_MacroInit()
{
  struct AZMacroInfo *mi;

    if (AGP.FObj->MacInfo = mi = PoolAlloc( sizeof(struct AZMacroInfo) ))
        {
        if (mi->COTable = (struct CmdObj **) BufAllocP( AGP.Pool, 0, 5*4 ))
            {
            return(1);
            }
        }
    return(0);
}

void func_MacroCleanup()
{
    func_MacroReset();
    /* rien d'autre car pooled */
}

//================================================================

static ULONG macro2buf( struct CmdObj *co, UBYTE **pub )
{
  ULONG totlen, len;

    totlen = BufLength( *pub ); len = StrLen( co->RegGerm->Names[0] );
    if (! BufPasteS( co->RegGerm->Names[0], len, pub, totlen )) goto END_ERROR;

    totlen += len; len = 1;
    if (! BufPasteS( " ", len, pub, totlen )) goto END_ERROR;

    totlen += len; len = StrLen( co->PArg.CmdLine );
    if (! BufPasteS( co->PArg.CmdLine, len, pub, totlen )) goto END_ERROR;

    totlen += len; len = 1;
    if (! BufPasteS( "\n", len, pub, totlen )) goto END_ERROR;

    return(0);

  END_ERROR:
    return( TEXT_NOMEMORY );
}

ULONG func_Macro2Buf( UBYTE **pub )
{
  ULONG errmsg=0, alloc=0;

    if (func_MacroExists())
        {
        if (! *pub) // buffer peut être pré-alloué
            {
            if (! (*pub = BufAlloc( 16, 0, MEMF_ANY ))) errmsg = TEXT_NOMEMORY;
            alloc = 1;
            }
        if (*pub)
            {
            if (errmsg = func_MacroApply( macro2buf, pub ))
                {
                if (alloc) BufFreeS( pub ); // si erreur on libère si on a alloué
                }
            }
        }
    else errmsg = TEXT_ERR_NoRecMacro;

    return( errmsg );
}

//---------------

ULONG func_Buf2Macro( UBYTE **pbuf, UBYTE **errorname )
{
  ULONG errmsg=0, len;
  struct read_file rf = {0,0,0,0,0,0,0,0,0};
  UBYTE *equalmem=0, *p, *cmdname, *cmdline;

    rf.mem = *pbuf;
    rf.max = rf.mem + BufLength(*pbuf);
    strip_comments( &rf, 0, 0 );
    *pbuf = rf.mem;

    rf.ptr = rf.mem;
    while (rf.ptr < rf.max)
        {
        if (len = StrLen( rf.ptr ))
            {
            p = rf.ptr + skip_char( rf.ptr, ' ', '\t' ); // saute espaces entre début et cmdname
            if (*p)
                {
                if (equalmem = (UIKBUF *) EqualReplace( AGP.Pool, p )) p = equalmem;
                cmdname = p;
                cmdline = "";
                p += find_char( p, ' ', '\t' ); // cherche espace de fin cmdname
                if (*p)
                    {
                    *p++ = 0;                  // fin cmdname
                    cmdline = p + skip_char( p, ' ', '\t' ); // saute espaces entre cmdname et parms
                    }
                errmsg = func_MacroAdd( cmdname, cmdline );
                BufFreeS( &equalmem );
                if (errmsg) { *errorname = cmdname; break; }
                }
            }
        rf.ptr += len + 1;
        }
    return( errmsg );
}
