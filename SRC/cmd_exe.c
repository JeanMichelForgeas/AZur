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
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include <rexx/storage.h>


/****** Imported ************************************************/

extern void func_Block2File();

extern struct Glob { ULONG toto; } __near GLOB;

extern UBYTE __far CopyrigthCode[];
extern ULONG __far CopyrigthCodeLen;
extern UBYTE __far CopyrigthCode2[];
extern ULONG __far CopyrigthCode2Len;
extern UBYTE __far VersionCode[];
extern ULONG __far VersionCodeLen;
extern UBYTE __far SaveCopyrigthCode[];
extern UBYTE __far SaveCopyrigthCode2[];
extern UBYTE __far SaveVersionCode[];
extern UBYTE * __far LangEnglish[];
extern UBYTE * __far LangOther[];
extern UBYTE __far VersionTag[];


/****** Exported ***********************************************/


/****** Statics ************************************************/

static ULONG dos_OpenConsole();


/****************************************************************
 *
 *      Objets
 *
 ****************************************************************/

static void security()
{
    CopyMem( SaveCopyrigthCode, CopyrigthCode, CopyrigthCodeLen );
    CopyMem( SaveCopyrigthCode2, CopyrigthCode2, CopyrigthCode2Len );
    CopyMem( SaveVersionCode, VersionCode, VersionCodeLen );

    CopyMem( CopyrigthCode, LangEnglish[1], CopyrigthCodeLen );
    func_DecodeText( "yatChovyLMe", LangEnglish[1], CopyrigthCodeLen );
    CopyMem( CopyrigthCode2, LangOther[1], CopyrigthCode2Len );
    func_DecodeText( "yatChovyLMe", LangOther[1], CopyrigthCode2Len );
    CopyMem( VersionCode, &VersionTag[1], VersionCodeLen );
    func_DecodeText( "yatChovyLMe", &VersionTag[1], VersionCodeLen );
}

UBYTE *func_GetPubScreenName()
{
  struct List *sclist;
  struct PubScreenNode *node;
  struct Screen *sc;
  UBYTE *name=0;

    if (AGP.WO && AGP.WO->Obj.w)
        {
        sc = AGP.WO->Obj.w->WScreen;
        if (UIK_IsSystemVersion2())
            {
            if (sclist = LockPubScreenList())
                {
                for (node=(struct PubScreenNode *)sclist->lh_Head; node->psn_Node.ln_Succ; node = (struct PubScreenNode *)node->psn_Node.ln_Succ)
                    {
                    if (node->psn_Screen == sc )
                        { name = node->psn_Node.ln_Name; break; }
                    }
                UnlockPubScreenList();
                }
            }
        if (! name)
            {
            if (! (name = sc->Title)) name = sc->DefaultTitle;
            }
        }
    if (! name) name = "";
    return( name );
}

void extract_filename( UBYTE *src, UBYTE *dst, ULONG dstmaxlen )
{
  UBYTE *p, quoting=0;
  ULONG max = dstmaxlen - 1;

    p = src + skip_char( src, ' ', '\t' );

//    for (start=p, i=0; p[i]; i++)
//        { if (p[i] == ':') { start = p + i; break; } }

    if (*p == 0x22 || *p == 0x27) quoting = *p++;
    while (max--)
        {
        if (quoting && (*p == 0x22 || *p == 0x27))
            {
            break;
            }
        else if ((quoting == 0) /*&& (p > start)*/ && (*p == ' ' || *p == '\t'))
            {
            break;
            }
        *dst++ = *p++;
        }
    *dst = 0;
}

//»»»»»» Cmd_ExeARexxCmd        BLOCK/S,CONSOLE/S,ASYNC/S,BA=BLOCKARG/S,CMD=COMMAND=MACRO/F

void CmdFunc_ExeARexx_SetMenu( struct CmdObj *co )
{
}

static ULONG arexx_OpenConsole( ULONG *pfile )
{
  ULONG len, errmsg=0;
  UBYTE *ub=0, *screenname;

    /*
    if (&AGP == (struct AZurProc *) AGP.AZMast) // pas de console pour les Master Process
        { errmsg = TEXT_ExeObj_Master; goto END_ERROR; }
    */
    if (AGP.RexxConsole) { errmsg = TEXT_ERR_ConsoleInUse; goto END_ERROR; }

    len = StrLen( AGP.Prefs->ARexxConsoleDesc );

    if (! (ub = BufAlloc( 8, 0, MEMF_ANY ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
    if (! BufSetS( AGP.Prefs->ARexxConsoleDesc, len, &ub )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }

    if (UIK_IsSystemVersion2())
        {
        screenname = func_GetPubScreenName();
        if (screenname[0])
            if (! BufPrintfB( UIKBase, &ub, len, "/SCREEN%ls", screenname )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        }
    if (! (*pfile = Open( ub, MODE_NEWFILE ))) { errmsg = TEXT_ERR_OpenConsole; goto END_ERROR; }

  END_ERROR:
    BufFree( ub ); ub = 0;
    return( errmsg );
}

static void arexx_FS_OKFunc( struct UIKObjWinFileSel *wfs, UBYTE *filename )
{
  struct UIKRexxCustom custom;
  ULONG events, errmsg = 0, opened = 0, async = wfs->Obj.UserULong2;
  APTR rmsg = 0;

    //------ Paramètres
    BufSetS( filename, StrLen(filename), &AGP.ARexxFileName );
    BufSetS( filename, StrLen(filename), &AGP.UBufTmp );
    UIK_Stop( wfs );

    if (REQBUT_OK != func_IntSmartRequest( 0, TEXT_ARexxScript2Exe,0, TEXT_ExeARexx_Parms,0, 0,&AGP.UBufTmp, 0,0, TEXT_GADOK,0,TEXT_CANCEL, 0, ARF_ACTIVATE, 0, 0 )) goto END_ERROR;

    //------ Doit-on ouvrir une console ?
    custom.Action = RXCOMM | RXFF_RESULT;
    custom.Stdin = 0;
    custom.Stdout = 0;

    if (wfs->Obj.UserULong1)
        {
        if (async)
            {
            if (errmsg = arexx_OpenConsole( &custom.Stdin )) goto END_ERROR;
            opened = 1;
            AGP.RexxConsole = custom.Stdout = custom.Stdin;
            }
        else{
            if (errmsg = dos_OpenConsole()) goto END_ERROR;
            custom.Stdout = custom.Stdin = AGP.DOSConsole;
            }
        security();
        }

    //------ envoie la commande
    if (! async)
        {
        events = UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_SendSyncCmdCustom, (ULONG)AGP.UBufTmp, (ULONG)&custom );
        //--- la commande est finie.
        if (! events) errmsg = TEXT_ERR_CantSendARexx;
        else if (events & (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D)) errmsg = TEXT_CMDCANCELLED;
        goto END_ERROR;
        }
    else{
        if (rmsg = (APTR) UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_SendARexxCmdCustom, (ULONG)AGP.UBufTmp, (ULONG)&custom ))
            {
            //--- commande partie OK, elle reviendra plus tard
            if (opened) AGP.RexxConsoleMsg = rmsg;
            }
        else { errmsg = TEXT_NOMEMORY; }
        }

  END_ERROR:
    if (!async || errmsg)
        {
        if (opened && AGP.RexxConsole) { Close( AGP.RexxConsole ); AGP.RexxConsole = 0; }
        if (errmsg && !TST_ERRORQUIET) func_ReqShowText( UIK_LangString( AGP.UIK, errmsg ) );
        }
    UIK_Remove( wfs );
}
static void arexx_FS_CancelFunc( int nil, struct UIKObjWinFileSel *wfs )
{
    UIK_Remove( wfs );
}

static void ARexxCommon( struct CmdObj *co, ULONG isstr, UBYTE *pattern, UBYTE *name )
{
  struct UIKRexxCustom custom;
  ULONG errmsg=0, opened=0, sync=0, events, file=0;
  APTR rmsg=0;
  UIKBUF *ub=0;
  UBYTE *hd="/* */\n", *string, bufname[200];
  struct UIKObj *req=0;

    if (AGP.ARexx == 0) return;
    if (! AGP.ARexxFileName[0]) BufSetS( "AZur:Rexx", 9, &AGP.ARexxFileName );

    //------ Faut-il envoyer le FileSelector ?
    if (name && name[0] && !isstr)  // si pas script, pas tester si nom de fichier (requête pour ":")
        {
        extract_filename( name, bufname, 200 );
        if (MayBeFileName( bufname, 1 )) file = 1; // create mode car scripts souvent abrégés
        BufSetS( name, StrLen(name), &AGP.ARexxFileName );
        }

    if (!isstr && !file && !co->ArgRes[0])   // si script et pas de nom et pas de block
        {
        if (! AGP.WO) return;
        if (!name || !name[0])
            {
            extract_filename( AGP.ARexxFileName, bufname, 200 );
            name = bufname;
            }

        req = (struct UIKObjWinFileSel *) UIK_AddObjectTagsB( UIKBase, "UIKObj_WinFileSel", AGP.WO,
            UIKTAG_OBJ_Title,               UIK_LangString( AGP.UIK, TEXT_ARexxScript2Exe ),
            UIKTAG_OBJ_TitleFl_Addr,        TRUE,
            UIKTAG_OBJ_FontName,            AGP.Prefs->ViewFontName,
            UIKTAG_OBJ_FontHeight,          AGP.Prefs->ViewFontHeight,
            UIKTAG_OBJ_UserValue1,          co->ArgRes[1],  // console ?
            UIKTAG_OBJ_UserValue2,          co->ArgRes[2],  // async ?
            UIKTAG_WFS_OKFunc,              arexx_FS_OKFunc,
            UIKTAG_WFS_CancelFunc,          arexx_FS_CancelFunc,
            UIKTAG_WFS_InitPath,            name,
            UIKTAG_WFS_Pattern,             pattern,
            UIKTAG_WFSFl_ReturnReturn,      TRUE,
            UIKTAG_WFSFl_OK_IfNotExists,    FALSE,
            UIKTAG_WFSFl_LeavePath,         FALSE,
            TAG_END );
        if (!req || !UIK_Start( req )) { UIK_Remove( req ); errmsg = TEXT_NOMEMORY; goto END_ERROR; }

        return; // interface asynchrone
        }

    //------ Non, on va envoyer une commande d'ici...
    custom.Action = RXCOMM | RXFF_RESULT | (isstr ? RXFF_STRING : 0);
    custom.Stdin = 0;
    custom.Stdout = 0;

    if (!co->ArgRes[2]) sync = 1;  // Si pas ASYNC

    //------ Doit-on ouvrir une console ?
    if (co->ArgRes[1])  // CONSOLE
        {
        if (sync == 0)
            {
            if (errmsg = arexx_OpenConsole( &custom.Stdin )) goto END_ERROR;
            opened = 1;
            AGP.RexxConsole = custom.Stdout = custom.Stdin;
            }
        else{
            if (errmsg = dos_OpenConsole()) goto END_ERROR;
            custom.Stdout = custom.Stdin = AGP.DOSConsole;
            }
        security();
        }

    //------ trouve ce qu'il faut envoyer
    if (! (string = name))
        {
        if (co->ArgRes[0])              // BLOCK ?
            {
            if (errmsg = func_Block2Buf( &ub, 0 )) goto END_ERROR;
            if (isstr) { if (! BufPasteS( hd, 6, &ub, 0 )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; } }
            string = ub;
            }
        }
    else{
        if (co->ArgRes[3] && func_BlockExists()) // BLOCKARG ?
            {
            if (! (ub = BufAlloc( 100, StrLen(string)+1, MEMF_ANY ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            StrCpy( ub, string );
            StrCat( ub, " " );
            if (errmsg = func_Block2Buf( &ub, 0 )) goto END_ERROR;
            string = ub;
            }
        }

    //------ envoie la commande
    if (string)
        {
        if (sync)
            {
            events = UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_SendSyncCmdCustom, (ULONG)string, (ULONG)&custom );
            //--- la commande est finie.
            if (! events) errmsg = TEXT_ERR_CantSendARexx;
            else if (events & (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D)) errmsg = TEXT_CMDCANCELLED;
            goto END_ERROR;
            }
        else{
            if (rmsg = (APTR) UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_SendARexxCmdCustom, (ULONG)string, (ULONG)&custom ))
                {
                //--- commande partie OK, elle reviendra plus tard
                if (opened) AGP.RexxConsoleMsg = rmsg;
                }
            else { errmsg = TEXT_ERR_CantSendARexx; goto END_ERROR; }
            }
        }

  END_ERROR:
    if (ub) BufFree( ub );
    if (errmsg || sync)
        {
        if (req) UIK_Remove( req );
        if (opened) { Close( AGP.RexxConsole ); AGP.RexxConsole = 0; }
        if (errmsg) intAZ_SetCmdResult( co, errmsg ); // if() pour ne pas modifier le résultat d'une cmd
        }
}

void CmdFunc_ExeARexxCmd_Do( struct CmdObj *co )
{
    ARexxCommon( co, 1, 0, co->ArgRes[4] );
}

//»»»»»» Cmd_ExeARexxScript     BLOCK/S,CONSOLE/S,ASYNC/S,BA=BLOCKARG/S,PAT=PATTERN/K,SCRIPT=NAME/F

void CmdFunc_ExeARexxScript_Do( struct CmdObj *co )
{
    CopyProt();
    ARexxCommon( co, 0, co->ArgRes[4], co->ArgRes[5] );
}

//---------------------------------------------------------------------------------------

//»»»»»» Cmd_ExeDOSCmd          BLOCK/S,CONSOLE/S,ASYNC/S,BA=BLOCKARG/S,PORT/S,SCREEN/S,CMD=COMMAND=NAME/F

#define EXEDOS_BLOCK        0x0001
#define EXEDOS_CONSOLE      0x0002
#define EXEDOS_ASYNC        0x0004
#define EXEDOS_PORT         0x0008
#define EXEDOS_SCREEN       0x0010
#define EXEDOS_FROMAREXX    0x0020
#define EXEDOS_ISCMD        0x0040

static ULONG dos_execute( UBYTE **pcmd, ULONG flags )  // cmd est un UIKBuffer
{
  ULONG errmsg = 0, out, oldout;
  APTR contask;

    out = (flags & EXEDOS_CONSOLE) ? AGP.DOSConsole : AGP.NilFh;

    if (flags & EXEDOS_PORT)
        {
        if (! func_BufPrintf( pcmd, intAZ_LineBufLen(*pcmd), " PORT %ls",
            UIK_Call( AGP.ARexx, UIKFUNC_ARexxSim_ARexxName, 0, 0 ) )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        }

    if (flags & EXEDOS_SCREEN)
        {
        UBYTE *name = func_GetPubScreenName();
        if (name[0])
            if (! func_BufPrintf( pcmd, intAZ_LineBufLen(*pcmd), " PUBSCREEN \"%ls\"", name )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        }

    if (! (flags & EXEDOS_ISCMD))
        {
        if (! func_BufPrintf( pcmd, 0, "Execute " )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        }

    if (flags & EXEDOS_ASYNC)
        {
        if (! func_BufPrintf( pcmd, 0, "Run <NIL: %ls ", (flags & EXEDOS_CONSOLE) ? "" : ">NIL:" )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        }

    if (UIK_IsSystemVersion2())
        {
        contask = SetConsoleTask( ((struct FileHandle *)(out << 2))->fh_Type );
        oldout = SelectOutput( out );
        }
    else{
        contask = AGP.Process->pr_ConsoleTask;
        oldout = AGP.Process->pr_COS;
        AGP.Process->pr_ConsoleTask = ((struct FileHandle *)(out << 2))->fh_Type;
        AGP.Process->pr_COS = out;
        }

    if (FALSE == Execute( *pcmd, 0, out )) errmsg = TEXT_ERR_CantSendDOS;

    if (UIK_IsSystemVersion2())
        {
        SetConsoleTask( contask );
        SelectOutput( oldout );
        }
    else{
        AGP.Process->pr_ConsoleTask = contask;
        AGP.Process->pr_COS = oldout;
        }

  END_ERROR:
    return( errmsg );
}

static ULONG dos_OpenConsole()
{
  ULONG len, errmsg=0;
  UBYTE *ub=0, *screenname;

    /*
    if (&AGP == (struct AZurProc *) AGP.AZMast) // pas de console pour les Master Process
        { errmsg = TEXT_ExeObj_Master; goto END_ERROR; }
    */
    if (AGP.DOSConsole) return(0); //{ errmsg = TEXT_ERR_DOSConsoleInUse; goto END_ERROR; }

    len = StrLen( AGP.Prefs->DOSConsoleDesc );

    if (! (ub = BufAlloc( 8, 0, MEMF_ANY ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
    if (! BufSetS( AGP.Prefs->DOSConsoleDesc, len, &ub )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }

    if (UIK_IsSystemVersion2())
        {
        screenname = func_GetPubScreenName();
        if (screenname[0])
            if (! BufPrintfB( UIKBase, &ub, len, "/SCREEN%ls", screenname )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        }

    if (! (AGP.DOSConsole = Open( ub, MODE_NEWFILE ))) { errmsg = TEXT_ERR_DOSOpenConsole; goto END_ERROR; }

  END_ERROR:
    BufFree( ub ); ub = 0;
    return( errmsg );
}

static void dos_FS_OKFunc( struct UIKObjWinFileSel *wfs, UBYTE *filename )
{
  ULONG errmsg = 0, opened = 0;
  ULONG flags = wfs->Obj.UserULong1;

    //------ Paramètres
    BufSetS( filename, StrLen(filename), &AGP.DOSFileName );
    BufSetS( filename, StrLen(filename), &AGP.UBufTmp );
    UIK_Stop( wfs );

    if (REQBUT_OK != func_IntSmartRequest( 0, TEXT_DOSScript2Exe,0, TEXT_ExeDOS_Parms,0, 0,&AGP.UBufTmp, 0,0, TEXT_GADOK,0,TEXT_CANCEL, 0, ARF_ACTIVATE, 0, 0 )) goto END_ERROR;

    //------ Doit-on ouvrir une console ?
    if (flags & EXEDOS_CONSOLE)
        {
        if (! AGP.DOSConsole)
            {
            if (errmsg = dos_OpenConsole()) goto END_ERROR;
            opened = 1;
            }
        security();
        }

    //------ envoie la commande
    errmsg = dos_execute( &AGP.UBufTmp, flags );

  END_ERROR:
    if (errmsg || !(flags & EXEDOS_ASYNC))
        {
        //if (opened && AGP.DOSConsole) { Close( AGP.DOSConsole ); AGP.DOSConsole = 0; }
        if (errmsg && !TST_ERRORQUIET) func_ReqShowText( UIK_LangString( AGP.UIK, errmsg ) );
        }
    UIK_Remove( wfs );
}
static void dos_FS_CancelFunc( int nil, struct UIKObjWinFileSel *wfs )
{
    UIK_Remove( wfs );
}

#define ARG_BLOCK   co->ArgRes[0]
#define ARG_CONSOLE co->ArgRes[1]
#define ARG_ASYNC   co->ArgRes[2]
#define ARG_BLKARG  co->ArgRes[3]
#define ARG_PORT    co->ArgRes[4]
#define ARG_SCREEN  co->ArgRes[5]

static void DOSCommon( struct CmdObj *co, ULONG iscmd, UBYTE *pattern, UBYTE *name )
{
  ULONG flags=0, errmsg=0, opened=0, file=0;
  UBYTE *string, delname[36], bufname[200];
  UIKBUF *ub=0;
  struct UIKObj *req=0;

    if (ARG_BLOCK)   flags |= EXEDOS_BLOCK;
    if (ARG_CONSOLE) flags |= EXEDOS_CONSOLE;
    if (ARG_ASYNC)   flags |= EXEDOS_ASYNC;
    if (ARG_PORT)    flags |= EXEDOS_PORT;
    if (ARG_SCREEN)  flags |= EXEDOS_SCREEN;
    if (co->ExeFrom==FROMAREXX) flags |= EXEDOS_FROMAREXX;
    if (iscmd)       flags |= EXEDOS_ISCMD;

    delname[0] = 0;
    if (! AGP.DOSFileName[0]) BufSetS( "S:", 2, &AGP.DOSFileName );

    //------ Faut-il envoyer le FileSelector ?
    if (name && name[0] && !iscmd)  // si pas script, pas tester si nom de fichier (requête pour ":")
        {
        extract_filename( name, bufname, 200 );
        if (MayBeFileName( bufname, 0 )) file = 1;
        BufSetS( name, StrLen(name), &AGP.DOSFileName );
        }

    if (!iscmd && !file && !ARG_BLOCK)   // si script et pas de nom et pas de block
        {
        if (! AGP.WO) return;
        if (!name || !name[0])
            {
            extract_filename( AGP.DOSFileName, bufname, 200 );
            name = bufname;
            }

        req = (struct UIKObjWinFileSel *) UIK_AddObjectTagsB( UIKBase, "UIKObj_WinFileSel", AGP.WO,
            UIKTAG_OBJ_Title,               UIK_LangString( AGP.UIK, TEXT_DOSScript2Exe ),
            UIKTAG_OBJ_TitleFl_Addr,        TRUE,
            UIKTAG_OBJ_FontName,            AGP.Prefs->ViewFontName,
            UIKTAG_OBJ_FontHeight,          AGP.Prefs->ViewFontHeight,
            UIKTAG_OBJ_UserValue1,          flags,  // arguments
            UIKTAG_WFS_OKFunc,              dos_FS_OKFunc,
            UIKTAG_WFS_CancelFunc,          dos_FS_CancelFunc,
            UIKTAG_WFS_InitPath,            name,
            UIKTAG_WFS_Pattern,             pattern,
            UIKTAG_WFSFl_ReturnReturn,      TRUE,
            UIKTAG_WFSFl_OK_IfNotExists,    FALSE,
            TAG_END );
        if (!req || !UIK_Start( req )) { UIK_Remove( req ); errmsg = TEXT_NOMEMORY; goto END_ERROR; }

        return; // interface asynchrone
        }

    //------ Doit-on ouvrir une console ?
    if (ARG_CONSOLE)    // si CONSOLE
        {
        if (! AGP.DOSConsole)
            {
            if (errmsg = dos_OpenConsole()) goto END_ERROR;
            opened = 1;
            }
        security();
        }

    //------ trouve ce qu'il faut envoyer
    if (! (ub = BufAlloc( 100, 0, MEMF_ANY ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }

    if (! (string = name))  // NAME ?
        {
        if (ARG_BLOCK)          // BLOCK ?
            {
            if (func_BlockExists())
                {
                if (flags & EXEDOS_ISCMD)
                    {
                    //--- sauve le bloc (script) dans un fichier, et le nom dans ub
                    if (! func_BufPrintf( &ub, 0, "T:AZur-%lx.cmd", AGP.Process )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
                    if (errmsg = func_SaveFile( ub, 1, 0, 1, (void*)func_Block2File, 0 )) goto END_ERROR;
                    StrCpy( delname, ub );
                    flags &= ~EXEDOS_ISCMD;
                    }
                else{
                    //--- met le bloc (nom de script) directement dans ub
                    if (errmsg = func_Block2Buf( &ub, 0 )) goto END_ERROR;
                    }
                }
            else { errmsg = TEXT_NoBlockDefined; goto END_ERROR; }
            }
        }
    else{
        BufSetS( string, StrLen(string), &ub );
        if (ARG_BLKARG && func_BlockExists())
            {
            if (! BufPasteS( " ", 1, &ub, intAZ_LineBufLen(ub) )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            if (errmsg = func_Block2Buf( &ub, 0 )) goto END_ERROR;
            }
        }

    //------ envoie la commande
    errmsg = dos_execute( &ub, flags );

  END_ERROR:          // attend que
    if (delname[0])
        {
        if (ARG_ASYNC) Delay(10);
        DeleteFile( delname ); // avant le BufFree( ub )
        }
    if (ub) BufFree( ub );
    if (errmsg || !ARG_ASYNC)
        {
        if (req) UIK_Remove( req );
        //if (opened) { Close( AGP.DOSConsole ); AGP.DOSConsole = 0; }
        if (errmsg) intAZ_SetCmdResult( co, errmsg ); // if() pour ne pas modifier le résultat d'une cmd
        }
}

void CmdFunc_ExeDOSCmd_Do( struct CmdObj *co )
{
    DOSCommon( co, 1, 0, co->ArgRes[6] );
}

//»»»»»» Cmd_ExeDOSScript       BLOCK/S,CONSOLE/S,ASYNC/S,BA=BLOCKARG/S,PORT/S,SCREEN/S,PAT=PATTERN/K,SCRIPT=NAME/F

void CmdFunc_ExeDOSScript_Do( struct CmdObj *co )
{
    CopyProt();
    DOSCommon( co, 0, co->ArgRes[6], co->ArgRes[7] );
}

//»»»»»» Cmd_ExeLastCmd

void CmdFunc_ExeLastCmd_Do( struct CmdObj *co )
{
  UBYTE *ub, *ptr = AGP. LastCmd;
  ULONG len = BufLength(ptr);

    if (len)
        {
        if (ub = BufAllocP( AGP.Pool, len, 4 ))
            {
            StrCpy( ub, ptr );
            eng_ExecuteAZurCmd( ub, FROMPGM, 0 );
            BufFree( ub );
            }
        }
}
