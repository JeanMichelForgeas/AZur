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

#include "uiki:objreqstring.h"
#include "uiki:objwinfilesel.h"
#include "uiki:objmemory.h"
#include "uiki:objstring.h"

#include "lci:aps.h"
#include "lci:aps_protos.h"
#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:_asyncio.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern ULONG build_auts_name( UBYTE *filename );

extern UBYTE *lo_MakeTotalPath( UBYTE *name );


/****** Exported ***********************************************/


/****** Statics ************************************************/


/****************************************************************
 *
 *      Utilities
 *
 ****************************************************************/

static ULONG AskModified( ULONG textinfo, ULONG okbut )
{
    if (TST_MODIFIED)
        {
        UBYTE buf[250];
        sprintf( buf, "%ls%ls", UIK_LangString( AGP.UIK, TEXT_MODIFIED ), UIK_LangString( AGP.UIK, textinfo ) );
        return( (ULONG)func_IntSmartRequest( 0, 0,0, 0,buf, 0,0,0,0, okbut,0,TEXT_CANCEL, 0, 0, 0, 0 ) );
        }
    return( REQBUT_OK );
}

void err_cancel( struct CmdObj *co ) { intAZ_SetCmdResult( co, TEXT_CMDCANCELLED ); }
void err_memory( struct CmdObj *co ) { intAZ_SetCmdResult( co, TEXT_NOMEMORY ); }

void update_file2fs( UBYTE *filename, ULONG chgopen, ULONG chgsave )
{
  struct UIKObjWinFileSel *wfs;

    if (wfs = AGP.FObj->OpenFS)
        {
        UIK_Call( wfs->FS, UIKFUNC_FS_UpdateFile, (ULONG)filename, 0 );
        if (chgopen) UIK_Call( wfs->FS, UIKFUNC_FS_ChangePath, (ULONG)filename, 0 );
        }
    if (wfs = AGP.FObj->SaveFS)
        {
        UIK_Call( wfs->FS, UIKFUNC_FS_UpdateFile, (ULONG)filename, 0 );
        if (chgsave) UIK_Call( wfs->FS, UIKFUNC_FS_ChangePath, (ULONG)filename, 0 );
        }
}

/****************************************************************
 *
 *      Objets
 *
 ****************************************************************/

//»»»»»» Cmd_WinClear »»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»» QUIET=FORCE/S

void CmdFunc_WinClear_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;

    if (co->ArgRes[0] == 0) // QUIET ?
        if (! AskModified( TEXT_CLEARVERIFY, TEXT_NEW )) { err_cancel( co ); return; }

    UIK_Call( fo->WO, UIKFUNC_Window_ChangeTitle, (ULONG)UIK_LangString( AGP.UIK, TEXT_UnTitled ), 0 );
    if (fo->SaveFS)
        {
        BufTruncate( fo->OpenFileName, UIK_BaseName( fo->OpenFileName ) - fo->OpenFileName );
        UIK_Call( fo->SaveFS->FS, UIKFUNC_FS_ChangePath, (ULONG)fo->OpenFileName, 0 );
        }
    if (! func_ResetFile()) err_memory( co );
    CLR_TOOLASKED; /* le Default Tool est de nouveau inconnu */
    CLR_COMPRESSED; /* le type du fichier est de nouveau inconnu */
    CLR_NEWICONASKED; /* la décision pour l'icône est de nouveau inconnue */
    func_DisplayStats(1);
    func_ReDisplay();
    func_ResetProps();
}

//»»»»»» Cmd_ File2Win »»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»  QUIET=FORCE/S,CLEAR/S,SAME/S,
// CREATE=SC=SETCURRENT/S,NAME=FILENAME,RECT=VERTICAL/S,REPL=OVERLAY/S,CC=CONVERT/S,
// NC=NOCURS/S,RO=READONLY/S,BLOCK/S,PAT=PATTERN/K,NI=NOICON/S,CURSNAME/S,ND=NODECOMPRESS/S

static ULONG cmd_loadfile( UBYTE *filename, ULONG *arg );

void patch_lf( UBYTE *ptr )
{
    while (*ptr)
        {
        if (*ptr == '\n') *ptr = 0;
        else ptr++;
        }
}

static void do_lock_modify( ULONG on )
{
  UBYTE buf[32];

    sprintf( buf, "Lock MODIFY %ls", (on ? "ON" : "OFF") );
    eng_ExecuteAZurCmd( buf, -1, 0 );
}

void CmdFunc_File2Win_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  ULONG create, errmsg=0, clear=(ULONG)co->ArgRes[1], quiet=(ULONG)co->ArgRes[0], oldmod=TST_MODIFIED;
  UBYTE *filename=0;
  UBYTE *ub=0;

    create = co->ArgRes[3] ? 1 : 0;

    //------ si pas CLEAR et pas CREATE on ne doit pas charger de fichier car on est en READONLY
    if (TST_READONLY)
        {
        if (!clear && !create) { errmsg = TEXT_ExeObj_Modify; goto END_ERROR; }
        }

    //------ si CLEAR et pas QUIET on demande confirmation pour modification
    if (clear && (quiet == 0))
        if (! AskModified( TEXT_OPENVERIFY, TEXT_OPEN )) { err_cancel( co ); return; }

    //------ obtention du nom de fichier
    if (co->ArgRes[2] && fo->OpenFileName[0])      // SAME ?
        {
        if (! (ub = BufAlloc( 300, 0, MEMF_ANY ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        BufSetS( fo->OpenFileName, StrLen(fo->OpenFileName), &ub );
        filename = ub;
        }
    else if (co->ArgRes[13])  // CURSNAME
        {
        if (errmsg = func_Curs2FileName( &ub )) goto END_ERROR;
        filename = ub;
        }
    else if (co->ArgRes[10])    // BLOCK ?
        {
        if (errmsg = func_Block2Buf( &ub, 1 )) goto END_ERROR;
        patch_lf( ub );
        filename = ub;
        }
    else{
        if (! (filename = (UBYTE*)func_IntReqFileNameO( co->ArgRes[4], co->ArgRes[11], TEXT_REQ_OpenFile, &errmsg, create )))
            {
            if (fo->OpenFileName[0]) UIK_Call( fo->WO, UIKFUNC_Window_ChangeTitle, (ULONG)UIK_BaseName( fo->OpenFileName ), 0 );
            goto END_ERROR;
            }
        if (clear && (quiet == 0) && (oldmod == 0)) // si CLEAR et pas QUIET et pas modifié avant
            if (! AskModified( TEXT_OPENVERIFY, TEXT_OPEN )) { err_cancel( co ); return; }
        }

    //------ chargement
    do_lock_modify(0);
    errmsg = cmd_loadfile( filename, (APTR)&co->ArgRes[0] );

  END_ERROR:
    BufFree( ub );
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
    else{
        if (create || clear)
            {
            lo_SetCurDir( fo->OpenFileName );
            if (co->ArgRes[9] && !TST_READONLY) /* a pu être mis par AZURATTR dans l'icône */
                do_lock_modify(1);
            }
        if (create) // si CREATE/SETCURRENT
            {
            if (fo->SaveFS)
                UIK_Call( fo->SaveFS->FS, UIKFUNC_FS_ChangePath, (ULONG)fo->OpenFileName, 0 );
            }
        }
    func_DisplayStats(1);
}

static ULONG cmd_loadfile( UBYTE *filename, ULONG *arg )
{
  ULONG loadflags, flags, askdelete=0, errmsg=0, line, col, zline, zcol, clear=arg[1], create=arg[3];
  UBYTE *autname=0, *loadname, *ub=0, *ubuf=0; /*buf[200],*/
  struct AZObjFile *fo = AGP.FObj;

    if (! (ubuf = BufAllocP( AGP.Pool, 200, 20 ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }

    line = fo->Line; col = fo->Col;
    if (AGP.WO) { zline = fo->ActiveView->Zone->TopLine; zcol = fo->ActiveView->Zone->TopCol; }

    if (clear || create)
        {
        CLR_COMPRESSED; /* le type du fichier est de nouveau inconnu */
        }

    //------ effacement ancien texte
    if (clear)
        {
        UIK_Call( fo->WO, UIKFUNC_Window_ChangeTitle, (ULONG)UIK_LangString( AGP.UIK, TEXT_UnTitled ), 0 );
        if (! func_ResetFile()) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        func_ReDisplay();
        }

    //------ flags de lecture
    loadflags = LOAF_DECOMPRESS;
    if (arg[14]) loadflags &= ~LOAF_DECOMPRESS; // ND=NODECOMPRESS/S

    //------ flags d'écriture
    flags = 0;
    if (arg[5]) flags |= ARPF_RECT;         // RECT=VERTICAL
    if (arg[6]) flags |= ARPF_OVERLAY;      // REPL=OVERLAY
    if (arg[7]) flags |= ARPF_CONVERT;      // CC=CONVERT
    if (errmsg = func_TextPrep( fo->Line, fo->Col, flags, 0, 0 )) goto END_ERROR;

    //------ requête autosave
    loadname = filename;
    if (build_auts_name( loadname ) == 0)
        {
        struct DateStamp d1, d2;
        UBYTE date1[30], date2[30], ss1[15], ss2[15], *spaces, *spaces1, *spaces2, *name1, *name2, *spacestr="                                ";
        LONG diff, size1, size2, len = StrLen( spacestr );

        autname = fo->AutsFileName;
        if (lo_NewerFile( autname, loadname, &d2, &d1, &size2, &size1 ) == 1)
            {
            Month_Day( &d1, date1 ); Month_Day( &d2, date2 );

            name1 = UIK_BaseName( loadname ); name2 = UIK_BaseName( autname );
            if ((diff = StrLen( name2 ) - StrLen( name1 )) < 0) diff = 0;
            if (diff > 30) diff = 30;
            spaces = &spacestr[len - diff];

            sprintf( ss1, "%ld", size1 ); size1 = StrLen( ss1 );
            sprintf( ss2, "%ld", size2 ); size2 = StrLen( ss2 );
            if (size1 > size2) { spaces1 = &spacestr[len]; spaces2 = &spacestr[len - (size1 - size2)]; }
            else { spaces2 = &spacestr[len]; spaces1 = &spacestr[len - (size2 - size1)]; }

            func_BufPrintf( &ubuf, 0, UIK_LangString( AGP.UIK, TEXT_AutoSaveNewer ),
                name1, spaces, spaces1, ss1, date1,
                name2,         spaces2, ss2, date2
                );
            if (REQBUT_OK == func_IntSmartRequest( 0, 0,0, 0,ubuf, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 ))
                {
                loadname = autname;
                }
            else{ // REQBUT_CANCEL
                askdelete = 1;
                }
            }
        }

    //------ en mode création le fichier peut ne pas exister
    if (lo_FileExists( loadname ) || !create) // ... if (!lo_FileExists() && create) c'est OK quand même
        {
        if (errmsg = func_LoadFile2( loadname, &loadflags )) goto END_ERROR;
        }
    if (clear && (arg[12] == 0/*NOICON*/) && TST_LOADICON && !(loadflags & LOAF_RES_FILENOICON))
        load_icon( loadname );                                             // résultat de func_LoadFile2()

    //------ remet les anciennes positions
    if (clear && arg[2]) // SAME
        {
        fo->Line = line; fo->Col = col;
        if (AGP.WO) { fo->ActiveView->Zone->TopLine = zline; fo->ActiveView->Zone->TopCol = zcol; }
        }

    //------ affiche le texte
    func_TextPut( 0, 0, 0, 1 ); // affiche le tout

    //--- Met le titre à la fenêtre
    if (clear || create)
        {
        CLR_TOOLASKED; /* le Default Tool est de nouveau inconnu */
        CLR_NEWICONASKED; /* la décision pour l'icône est de nouveau inconnue */
        if (clear) CLR_MODIFIED();
        if (create) // peut être un simple nom : rajouter le path courant
            { if (ub = lo_MakeTotalPath( filename )) filename = ub; }
        if (! BufSetS( filename, StrLen( filename ), &fo->OpenFileName )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        UIK_Call( fo->WO, UIKFUNC_Window_ChangeTitle, (ULONG)UIK_BaseName( filename ), 0 );
        }

    //--- Efface le fichier autosauve
    if (errmsg==0 && askdelete && autname)
        {
        BufTruncate( ubuf, 0 );
        func_BufPrintf( &ubuf, 0, UIK_LangString( AGP.UIK, TEXT_AutoSaveDelete ), UIK_BaseName( filename ), UIK_BaseName( autname ) );
        if (REQBUT_OK == func_IntSmartRequest( 0, 0,0, 0,ubuf, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 ))
            {
            func_DeleteFile( autname );
            }
        }

  END_ERROR:
    BufFree( ub );
    BufFree( ubuf );
    if (arg[8]) func_CursorPos( fo->Line, fo->Col );  // NOCURS
    else { func_CursorPos( fo->WorkLine, fo->WorkCol ); fo->CEdCol = fo->WorkCol; }
    if (! fo->Text.NodeNum) UIK_nl_AllocNode( &fo->Text, -1, 0, 0 );
    // func_DisplayStats(1);
    if (errmsg) func_ReDisplay();
    func_ResetProps();
    return( errmsg );
}

//»»»»»» Cmd_Win2File »»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»  NB=NOBACKUP/S,NI=NOICON/S,SAME/S,AP=APPEND/S,SC=SETCURRENT/S,MODIFIED/S,NAME,PAT=PATTERN/K,PAR=PARAGRAPH/S,NT=NOSPACE2TAB/S,CF=CONFIRM/S

/*
void CmdFunc_Win2File_New( struct CmdObj *co )
{
    if (co->ArgRes[6] && IsNotFileSystem( co->ArgRes[6] ))
        co->Flags &= ~AZAFF_DOMODIFY; // évite "Le texte ne peut être modifié" pour prt:
}
*/

void CmdFunc_Win2File_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  UBYTE *filename;
  ULONG errmsg=0, filesystem=0, flags=0;

    if (co->ArgRes[5] && !TST_MODIFIED) { err_cancel( co ); return; }

    if (co->ArgRes[2]) filename = fo->OpenFileName;   // SAME ?
    else filename = co->ArgRes[6];                          // NAME ?
    if (! (filename = (UBYTE*)func_IntReqFileNameS( filename, co->ArgRes[7], TEXT_REQ_SaveFile, &errmsg, 1 ))) goto END_ERROR;

    if (TST_SAVELOCK && (StrNCCmp( filename, fo->OpenFileName ) == 0)) { err_cancel( co ); return; }
    filesystem = 1;
    if (IsNotFileSystem( filename )) filesystem = 0;

    if (filesystem && co->ArgRes[10] && lo_FileExists(filename)) // CONFIRM
        {
        BufTruncate( AGP.UBufTmp, 0 );
        func_BufPrintf( &AGP.UBufTmp, 0, UIK_LangString( AGP.UIK, TEXT_ConfirmSave ), filename );
        if (REQBUT_OK != func_IntSmartRequest( 0, 0,0, 0,AGP.UBufTmp, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 ))
            { err_cancel( co ); return; }
        }

    if (co->ArgRes[8]) flags |= SAVF_PARAGRAPH;
    if (TST_SPACE2TAB && !co->ArgRes[9]) flags |= SAVF_SPACE2TAB;
    if (TST_COMPRESSED && filesystem) flags |= SAVF_COMPRESS;
    errmsg = func_SaveFile( filename, (ULONG)co->ArgRes[0], (ULONG)co->ArgRes[3], (ULONG)co->ArgRes[1], 0, (APTR)flags );  // NOBACKUP ? APPEND ?
    if (filesystem)
        {
        if (! errmsg)
            {
            CLR_MODIFIED();
            update_file2fs( filename, 0, 0 );
            }
        if (! BufSetS( filename, StrLen( filename ), &fo->SaveFileName )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        if (fo->SaveFS)
            UIK_Call( fo->SaveFS->FS, UIKFUNC_FS_ChangePath, (ULONG)fo->SaveFileName, 0 );
        }

  END_ERROR:
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
    else if (filesystem && (fo->OpenFileName[0] == 0 || co->ArgRes[4])) // si pas encore de titre, ou SETCURRENT
        {
        if (TST_SAVELOCK && (StrNCCmp( filename, fo->OpenFileName ) != 0)) CLR_SAVELOCK;

        if (! BufSetS( filename, StrLen( filename ), &fo->OpenFileName )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        lo_SetCurDir( fo->OpenFileName );
        UIK_Call( fo->WO, UIKFUNC_Window_ChangeTitle, (ULONG)UIK_BaseName( filename ), 0 );
        if (co->ArgRes[4] && fo->OpenFS)                  // si SETCURRENT
            {
            UIK_Call( fo->OpenFS->FS, UIKFUNC_FS_ChangePath, (ULONG)filename, 0 );
            }
        }
    func_DisplayStats(1);
    return;
}

//»»»»»» Cmd_File2CB            NAME,CLIP=UNIT/K/N,PAT=PATTERN/K

static ULONG func_File2CB( UBYTE *filename, ULONG unit )
{
  ULONG errmsg=0, filelen, len, writelen;
  ULONG blklen = AGP.Prefs->IOBufLength*1024;
  APTR clip = 0;
  struct AsyncFile *fin;
  UBYTE *readbuf;

    if (unit <= 255)
        {
        if (lo_FileExists( filename ))
            {
            filelen = AGP.FIB.fib_Size;
            if (fin = OpenAsync( filename, MODE_READ, blklen ))
                {
                if (clip = func_CBWriteStart( unit ))
                    {
                    writelen = 0;
                    while (filelen)
                        {
                        len = (filelen > blklen ? blklen : filelen);
                        filelen -= len;
                        if (ReadAsync( fin, &readbuf ) != len) { errmsg = TEXT_ERR_READFILE; break; }
                        if (! func_CBWrite( clip, readbuf, len )) { errmsg = TEXT_ERR_WriteClip; break; }
                        writelen += len;
                        }
                    func_CBWriteEnd( clip, writelen );
                    }
                else errmsg = TEXT_ERR_OpenClip;
                CloseAsync( fin );
                }
            else errmsg = TEXT_ERR_OPENFILE;
            }
        else errmsg = TEXT_ERR_OPENFILE;
        }
    else errmsg = TEXT_ERR_BadClipUnit;

    return( errmsg );
}

void CmdFunc_File2CB_Do( struct CmdObj *co )
{
  UBYTE *filename;
  ULONG errmsg, unit = 0;
  ULONG locks;

    if (co->ArgRes[1])                  // UNIT ?
        unit = *(ULONG*)co->ArgRes[1];

    locks = func_VerifyOff();
    func_SetMainWaitPointer();
                                        // NAME ?
    if (! (filename = (UBYTE*)func_IntReqFileNameO( co->ArgRes[0], co->ArgRes[2], TEXT_REQ_File2CB, &errmsg, 0 ))) goto END_ERROR;

    errmsg = func_File2CB( filename, unit );

  END_ERROR:
    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_File2File          NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,NAME,DEST,PAT=PATTERN/K,DESTPAT=DESTPATTERN/K

static ULONG func_File2File( struct AsyncFile *fout, UBYTE *srcfilename )
{
  ULONG errmsg=0, filelen, len;
  ULONG blklen = AGP.Prefs->IOBufLength*1024;
  struct AsyncFile *fin;
  UBYTE *readbuf;

    if (lo_FileExists( srcfilename ))
        {
        filelen = AGP.FIB.fib_Size;
        if (fin = OpenAsync( srcfilename, MODE_READ, blklen ))
            {
            while (filelen)
                {
                len = (filelen > blklen ? blklen : filelen);
                filelen -= len;
                if (ReadAsync( fin, &readbuf ) != len) { errmsg = TEXT_ERR_READFILE; break; }
                if (WriteAsync( fout, readbuf, len ) != len) { errmsg = TEXT_ERR_WRITEFILE; break; }
                }
            CloseAsync( fin );
            }
        else errmsg = TEXT_ERR_OPENFILE;
        }
    else errmsg = TEXT_ERR_OPENFILE;

    return( errmsg );
}

void CmdFunc_File2File_Do( struct CmdObj *co )
{
  UBYTE srcfilename[256], *dstfilename;
  ULONG errmsg=0;
  ULONG locks;

    locks = func_VerifyOff();
    func_SetMainWaitPointer();
                                        // NAME ?
    if (! (dstfilename = (UBYTE*)func_IntReqFileNameO( co->ArgRes[3], co->ArgRes[5], TEXT_REQ_File2FileSrc, &errmsg, 0 ))) goto END_ERROR;
    StrCpy( srcfilename, dstfilename );
                                        // DEST ?
    if (! (dstfilename = (UBYTE*)func_IntReqFileNameS( co->ArgRes[4], co->ArgRes[6], TEXT_REQ_File2FileDst, &errmsg, 1 ))) goto END_ERROR;
                                        // NOBACKUP ? APPEND ?                     pas d'icône
    errmsg = func_SaveFile( dstfilename, (ULONG)co->ArgRes[0], (ULONG)co->ArgRes[2], 1, func_File2File, (APTR)srcfilename );
    if (errmsg==0 && co->ArgRes[1]==0 && TST_SAVEICON) func_CopyIcon( srcfilename, dstfilename );

  END_ERROR:
    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    if (! errmsg) update_file2fs( dstfilename, 0, 0 );
    else intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_Prefs2File     DEFAULT/S,NAME  »»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»

static UBYTE *make_dirlist()
{
  struct APSHandle *apsh;
  ULONG lock=0, oldlock=0, userlock=0;
  UBYTE *list=0;

    if (list = BufAllocP( AGP.Pool, 0, 64 ))
        {
        if (apsh = APS_RegisterAppl( "AZurEditor", NULL, NULL, NULL, NULL, NULL, NULL ))
            {
            if (apsh->APSLock)
                {
                oldlock = CurrentDir( apsh->APSLock );
                if (userlock = Lock( apsh->UserName, SHARED_LOCK )) CurrentDir( userlock );
                }
            else{
                ULONG save = UIK_DisableDOSReq();
                if (! (userlock = Lock( "ENVARC:", SHARED_LOCK ))) userlock = Lock( "S:", SHARED_LOCK );
                UIK_EnableDOSReq( 0, save );
                if (userlock) oldlock = CurrentDir( userlock );
                }

            if (userlock)
                {
                if (! (lock = Lock( "AZurEditor", SHARED_LOCK )))
                    lock = CreateDir( "AZurEditor" );

                if (lock)
                    {
                    if (Examine( lock, &AGP.FIB ))
                        {
                        if (AGP.FIB.fib_DirEntryType > 0)
                            {
                            while (TRUE)
                                {
                                if (ExNext( lock, &AGP.FIB ) != FALSE)
                                    {
                                    if (AGP.FIB.fib_DirEntryType < 0)
                                        {
                                        if (! func_BufPrintf( &list, BufLength(list), "%ls\n", AGP.FIB.fib_FileName )) break;
                                        }
                                    }
                                else{
                                    // err = IoErr();
                                    // if (err != ERROR_NO_MORE_ENTRIES) // doit être testé juste après ExNext()
                                    break;
                                    }
                                }
                            }
                        }
                    UnLock( lock );
                    }
                if (oldlock) { CurrentDir( oldlock ); oldlock = 0; }
                UnLock( userlock );
                }

            if (oldlock) CurrentDir( oldlock );
            APS_UnregisterAppl( apsh );
            }
        }
    return( list );
}

static void setenvname( UBYTE **pub, UBYTE *envname, UBYTE *name )
{
  ULONG len;

    if (*pub = BufAllocP( AGP.Pool, 0, 4 ))
        {
        if (len = UIK_BaseName( envname ) - envname)
            BufPasteS( envname, len, pub, 0 );
        BufPasteS( name, StrLen(name), pub, len );
        }
}

static UBYTE *setallnames( UBYTE *name, UBYTE **ubenv, UBYTE **ubarc )
{
  struct AZur *mast = AGP.AZMast;

    name = UIK_BaseName( name );
    setenvname( ubenv, mast->EnvFileName, name );
    setenvname( ubarc, mast->ArcFileName, name );
    return( name );
}

//-----------------------------------------------------------------------------------------------

static void update_mast_prefs()
{
  struct AZurPrefs *mp = AGP.AZMast->Prefs;
  ULONG or1, or2, or3, ors;

    if (AGP.AZMast != (APTR)&AGP)
        {
        Forbid();
        or1 = mp->Flags1 & AZPM_NOTSAVED1;
        or2 = mp->Flags2 & AZPM_NOTSAVED2;
        or3 = mp->Flags3 & AZPM_NOTSAVED3;
        ors = mp->SearchFlags & AZPM_NOTSAVEDSEARCH;

        CopyMem( AGP.Prefs, AGP.AZMast->Prefs, sizeof(struct AZurPrefs) );

        mp->Flags1 &= ~AZPM_NOTSAVED1;
        mp->Flags2 &= ~AZPM_NOTSAVED2;
        mp->Flags3 &= ~AZPM_NOTSAVED3;
        mp->SearchFlags &= ~AZPM_NOTSAVEDSEARCH;

        mp->Flags1 |= or1;
        mp->Flags2 |= or2;
        mp->Flags3 |= or3;
        mp->SearchFlags |= ors;
        Permit();
        }
}

static void prefs_cancel( int nil, struct AZRequest *ar )
{
    func_ReqClose( 0, ar );
}

static ULONG do_saveprefs( int nil, UBYTE *name, struct AZRequest *ar )
{
  ULONG errmsg=0;
  struct APSHandle *apsh;
  struct AZurPrefs *curprefs = AGP.Prefs;
  APTR prefs;
  UBYTE *ubenv=0, *ubarc=0;


    if (name = setallnames( name, &ubenv, &ubarc ))
        {
        if (ubenv && ubarc && (apsh = APS_RegisterAppl( "AZurEditor", NULL, NULL, NULL, NULL, NULL, NULL )))
            {
            if (prefs = APS_OpenPrefs( apsh, ubenv, name, sizeof(struct AZurPrefs), 0, curprefs, sizeof(struct AZurPrefs) ))
                {
                if (AGP.AZMast != (APTR)&AGP)
                    {
                    curprefs->PosBookmarks = *((struct IBox *) &AGP.AZReqBookmark.Left);
                    curprefs->PosLineCol   = *((struct IBox *) &AGP.AZReqLine.Left);
                    curprefs->PosOffset    = *((struct IBox *) &AGP.AZReqOffset.Left);
                    curprefs->PosSearch    = *((struct IBox *) &AGP.AZReqSearch.Left);
                    curprefs->PosReplace   = *((struct IBox *) &AGP.AZReqReplace.Left);
                    }

                CopyMem( curprefs, prefs, sizeof(struct AZurPrefs) );
                if (APS_ERROR == APS_SavePrefs( apsh, ubarc,  ubenv, name, 0 ))
                    errmsg = TEXT_ERR_WRITEFILE;
                else{
                    StrCpy( AGP.DefPrefsName, name );
                    if (StrNCCmp( AGP.AZMast->DefPrefsName, name ) == 0) update_mast_prefs();
                    }
                APS_ClosePrefs( apsh, name );
                }
            APS_UnregisterAppl( apsh );
            }
        }

  END_ERROR:
    BufFree( ubenv );
    BufFree( ubarc );
    if (ar)
        {
        if (errmsg && !TST_ERRORQUIET) func_ReqShowText( UIK_LangString( AGP.UIK, errmsg ) );
        func_ReqClose( 0, ar );
        }
    return( errmsg );
}

void CmdFunc_Prefs2File_Do( struct CmdObj *co )
{
  UBYTE *name, *list;
  ULONG errmsg;

    if (co->ArgRes[0])  // DEFAULT
        name = AGP.DefPrefsName;
    else name = co->ArgRes[1]; // name ou request si 0

    if (name)
        {
        if (errmsg = do_saveprefs( 0, name, 0 ))
            intAZ_SetCmdResult( co, errmsg );
        }
    else if (list = make_dirlist())
        {
        func_ReqList( 0,
            ART_ReqTitle,       UIK_LangString( AGP.UIK, TEXT_Title_SavePrefs ),
            ART_ReqFlags,       ARF_WINDOW | ARF_ASYNC | ARF_STRGAD,
            ART_OKFunc,         do_saveprefs,
            ART_CancelFunc,     prefs_cancel,
            ART_ListInit,       list,
            ART_ListSelect,     AGP.DefPrefsName,
            TAG_END );
        BufFree( list );
        }
}

//»»»»»» Cmd_File2Prefs     DEFAULT/S,NAME

ULONG do_openprefs( int nil, UBYTE *name, struct AZRequest *ar ) // appelée aussi depuis ProcessMain()
{
  ULONG errmsg=0;
  struct APSHandle *apsh;
  struct AZurPrefs *curprefs = AGP.Prefs;
  APTR prefs;
  UBYTE *ubenv=0, *ubarc=0;

    if (name = setallnames( name, &ubenv, &ubarc ))
        {
        if (ubenv && ubarc && (apsh = APS_RegisterAppl( "AZurEditor", NULL, NULL, NULL, NULL, NULL, NULL )))
            {
            if (prefs = APS_OpenPrefs( apsh, ubenv, name, sizeof(struct AZurPrefs), 0, curprefs, sizeof(struct AZurPrefs) ))
                {
                CopyMem( curprefs, AGP.TmpPrefs, sizeof(struct AZurPrefs) );

                CopyMem( prefs, curprefs, sizeof(struct AZurPrefs) );
                APS_ClosePrefs( apsh, name );

                curprefs->Flags1 &= ~AZPM_NOTSAVED1;
                curprefs->Flags2 &= ~AZPM_NOTSAVED2;
                curprefs->Flags3 &= ~AZPM_NOTSAVED3;
                curprefs->SearchFlags &= ~AZPM_NOTSAVEDSEARCH;
                //update_mast_prefs();

                *((struct IBox *) &AGP.AZReqBookmark.Left) = curprefs->PosBookmarks;
                *((struct IBox *) &AGP.AZReqLine.Left)     = curprefs->PosLineCol  ;
                *((struct IBox *) &AGP.AZReqOffset.Left)   = curprefs->PosOffset   ;
                *((struct IBox *) &AGP.AZReqSearch.Left)   = curprefs->PosSearch   ;
                *((struct IBox *) &AGP.AZReqReplace.Left)  = curprefs->PosReplace  ;

                StrCpy( AGP.DefPrefsName, name );
                ActualizePrefs(1);
                }
            else errmsg = TEXT_ERR_READFILE;
            APS_UnregisterAppl( apsh );
            }
        }

  END_ERROR:
    BufFree( ubenv );
    BufFree( ubarc );
    if (ar) // donc si asynchrone
        {
        if (errmsg && !TST_ERRORQUIET) func_ReqShowText( UIK_LangString( AGP.UIK, errmsg ) );
        func_ReqClose( 0, ar );
        }
    return( errmsg );
}

void CmdFunc_File2Prefs_Do( struct CmdObj *co )
{
  UBYTE *name, *list;
  ULONG errmsg;

    if (co->ArgRes[0])  // DEFAULT
        name = AGP.DefPrefsName;
    else name = co->ArgRes[1]; // name ou request si 0

    if (name)
        {
        if (errmsg = do_openprefs( 0, name, 0 ))
            intAZ_SetCmdResult( co, errmsg );
        }
    else if (list = make_dirlist())
        {
        func_ReqList( 0,
            ART_ReqTitle,       UIK_LangString( AGP.UIK, TEXT_Title_LoadPrefs ),
            ART_ReqFlags,       ARF_WINDOW | ARF_ASYNC,
            ART_OKFunc,         do_openprefs,
            ART_CancelFunc,     prefs_cancel,
            ART_ListInit,       list,
            ART_ListSelect,     AGP.DefPrefsName,
            TAG_END );
        BufFree( list );
        }
}

//»»»»»» Cmd_File2Parms         // NAME,PAT=PATTERN/K

void CmdFunc_File2Parms_Do( struct CmdObj *co )
{
/*
  ULONG errmsg=0;
  UBYTE *srcfilename;
  ULONG locks;

    locks  func_VerifyOff();
    func_SetMainWaitPointer();

    if (srcfilename = (UBYTE*)func_IntReqFileNameS( co->ArgRes[0], co->ArgRes[1], TEXT_Title_LoadParms, &errmsg, 0 ))
        {
        errmsg = func_ReadParms( srcfilename );
        }

    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
*/
}

//»»»»»» Cmd_File2Mac       AP=APPEND/S,NAME,PAT=PATTERN/K

void CmdFunc_File2Mac_Do( struct CmdObj *co )
{
  ULONG file, size, errmsg=0;
  UBYTE *name, *ub=0, *errorname=0;
  ULONG locks;

    locks = func_VerifyOff();
    func_SetMainWaitPointer();
                                        // NAME ?
    if (! (name = (UBYTE*)func_IntReqFileNameS( co->ArgRes[1], co->ArgRes[2], TEXT_Macro2Load, &errmsg, 0 ))) goto END_ERROR;

    if (file = Open( name, MODE_OLDFILE ))
        {
        if (size = lo_FileSize( name ))
            {
            if (ub = BufAllocP( AGP.Pool, size, 4 ))
                {
                if (Read( file, ub, size ) == size)
                    {
                    if (! co->ArgRes[0]) func_MacroReset(); // APPEND
                    if (! (errmsg = func_Buf2Macro( &ub, &errorname )))
                        if (! func_MacroExists()) errmsg = TEXT_ERR_NoRecMacro;
                    }
                else errmsg = TEXT_ERR_READFILE;
                }
            else errmsg = TEXT_NOMEMORY;
            }
        Close( file );
        }
    else errmsg = TEXT_ERR_OPENFILE;

  END_ERROR:
    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    if (errmsg) SetCmdResultWith( co, errmsg, errorname );
    BufFree( ub ); // libérer après utilisation de *errorname
}

//»»»»»» Cmd_Mac2File       NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,NAME,PAT=PATTERN/K

static ULONG macro2file( struct AsyncFile *fout )
{
  ULONG errmsg=0, len;

    if (WriteAsync( fout, "/* AZur - Macro */\n\n", 20 ) == 20)
        {
        UBYTE *ub = 0;
        if (! (errmsg = func_Macro2Buf( &ub )))
            {
            len = BufLength( ub );
            if (WriteAsync( fout, ub, len ) != len) errmsg = TEXT_ERR_WRITEFILE;
            }
        BufFree( ub );
        }
    else errmsg = TEXT_ERR_WRITEFILE;

    return( errmsg );
}

void CmdFunc_Mac2File_Do( struct CmdObj *co )
{
  UBYTE *dstfilename;
  ULONG errmsg=0;
  ULONG locks;

    locks = func_VerifyOff();
    func_SetMainWaitPointer();

    if (! func_MacroExists()) { errmsg = TEXT_ERR_NoRecMacro; goto END_ERROR; }
                                        // NAME ?
    if (! (dstfilename = (UBYTE*)func_IntReqFileNameS( co->ArgRes[3], co->ArgRes[4], TEXT_Macro2Save, &errmsg, 1 ))) goto END_ERROR;
                                        // NOBACKUP ? APPEND ?                     pas d'icône
    errmsg = func_SaveFile( dstfilename, (ULONG)co->ArgRes[0], (ULONG)co->ArgRes[2], 1, macro2file, 0 );
    if (errmsg==0 && co->ArgRes[1]==0 && TST_SAVEICON) func_SaveIcon( dstfilename );

  END_ERROR:
    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    if (! errmsg) update_file2fs( dstfilename, 0, 0 );
    else intAZ_SetCmdResult( co, errmsg );
}
