/****************************************************************
 *
 *      File:       funcs_fileio.c  fonctions de fichiers
 *      Project:    AZur            editeur de texte
 *
 *      Created:    06/05/93        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <workbench/workbench.h>

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uiklist.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objtimer.h"
#include "uiki:objwindow.h"
#include "uiki:objfileselector.h"
#include "uiki:objwinfilesel.h"

#include "lci:azur.h"
#include "lci:objfile.h"
#include "lci:objzone.h"
#include "lci:azur_protos.h"
#include "lci:_asyncio.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"

#include "lci:ppbase.h"
#include "lci:powerpacker_protos.h"
#include "lci:powerpacker_pragmas.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/

void func_DeleteFile( STRPTR name );
void func_RenameFile( STRPTR from, STRPTR to );
void func_CopyIcon( STRPTR from, STRPTR to );
void func_SaveIcon( STRPTR name );


/****** Statics *************************************************/

static void save_icon( STRPTR name, struct memfold *mf, struct mempar *mp, ULONG saveflags );

static UWORD __chip def_textI1Data[] =
{
/* Plane 0 */
    0x0000,0x0000,0x00A0,0x0000,0x0140,0x0000,0x0A80,0x0000,
    0x0500,0x03FC,0x0000,0x0000,0x0000,0x0001,0x00F7,0xDBE1,
    0x0000,0x0001,0x07DD,0xFBE1,0x0000,0x0001,0x07FB,0xDFE1,
    0x0000,0x0001,0x05DF,0xD601,0x0000,0x0001,0x01EF,0xDEE1,
    0x0000,0x0001,0x07F7,0xCE01,0x0000,0x0001,0x0000,0x06E1,
    0x0000,0x0001,0x0000,0x0001,0x7FFF,0xFFFF,
/* Plane 1 */
    0xFFFF,0xFA00,0xFEAF,0xFB80,0xFD5F,0xFBE0,0xE2BF,0xFBF8,
    0xF07F,0xF800,0xFFFF,0xFFFE,0xFFFF,0xFFFE,0xFF08,0x241E,
    0xFFFF,0xFFFE,0xF822,0x041E,0xFFFF,0xFFFE,0xF804,0x201E,
    0xFFFF,0xFFFE,0xFA20,0x29FE,0xFFFF,0xFFFE,0xFE10,0x211E,
    0xFFFF,0xFFFE,0xF808,0x31FE,0xFFFF,0xFFFE,0xFFFF,0xF91E,
    0xFFFF,0xFFFE,0xFFFF,0xFFFE,0x0000,0x0000,
};

static struct Image __far def_textI1 =
{
    0, 0,           /* Upper left corner */
    32, 23, 2,      /* Width, Height, Depth */
    def_textI1Data, /* Image data */
    0x0003, 0x0000, /* PlanePick, PlaneOnOff */
    NULL            /* Next image */
};

static UBYTE * __far stdtooltypes[] = { "FILETYPE=TEXT|ASCII", 0 };

struct DiskObject __far def_text =
{
    WB_DISKMAGIC,           /* Magic Number */
    WB_DISKVERSION,         /* Version */
        {                   /* Embedded Gadget Structure */
        NULL,               /* Next Gadget Pointer */
        0, 0, 32, 24,       /* Left,Top,Width,Height */
        GFLG_GADGIMAGE | GADGBACKFILL,  /* Flags */
        GACT_RELVERIFY,     /* Activation Flags */
        BOOLGADGET,         /* Gadget Type */
        (APTR)&def_textI1,  /* Render Image */
        NULL,               /* Select Image */
        NULL,               /* Gadget Text */
        NULL,               /* Mutual Exclude */
        NULL,               /* Special Info */
        0,                  /* Gadget ID */
        NULL,               /* User Data */
        },
    WBPROJECT,              /* Icon Type */
    NULL,                   /* Default Tool */
    stdtooltypes,           /* Tool Type Array */
    NO_ICON_POSITION,       /* Current X */
    NO_ICON_POSITION,       /* Current Y */
    NULL,                   /* Drawer Structure */
    NULL,                   /* Tool Window */
    0                       /* Stack Size */
};

/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

ULONG func_ResetFile()
{
    func_BlockClearAll();
    func_BMRemAll();
    UIK_nl_FreeList( &AGP.FObj->Text );
    if (! UIK_nl_AllocListP( NULL, &AGP.FObj->Text, 256, MEMF_ANY, AGP.Prefs->PuddleSize*1024, AGP.Prefs->PuddleSize*1024 )) goto END_ERROR;
    if (! UIK_nl_AllocNode( &AGP.FObj->Text, -1, 0, 0 )) goto END_ERROR;
    BufSetS( "", 0, &AGP.FObj->OpenFileName );
    text_reset();
    CLR_MODIFIED();
    func_ViewsTopLeft();
    return(1);

  END_ERROR:
    return(0);
}

static BOOL get_password( UBYTE *password, ULONG checksum )
{
  struct Library *PPBase=0;
  ULONG rega4, count=0;
  BOOL rc = FALSE;

    rega4 = __builtin_getreg( 12 );
    __builtin_putreg( 12, *(ULONG*)((ULONG)(FindTask(0)->tc_SPUpper)-6) ); // A4 pointe sur la structure des variables

    if (PPBase = OpenLibrary( "powerpacker.library", 0L ))
        {
        while (rc == FALSE && count < 3)
            {
            BufTruncate( AGP.UBufTmp, 0 );
            if (REQBUT_OK != func_IntSmartRequest( 0, 0,"PowerPacker", TEXT_PowerPacker_Psw,0, 0,&AGP.UBufTmp, 0,0, -1,0,-1, 0, ARF_WINDOW|ARF_ACTIVATE, 0, 0 ))
                break;
            if (checksum == ppCalcChecksum( AGP.UBufTmp ))
                {
                StrCpy( password, AGP.UBufTmp );
                rc = TRUE;
                }
            count++;
            }
        CloseLibrary( PPBase );
        }

    __builtin_putreg( 12, rega4 ); // restore A4
    return( rc );
}

ULONG func_LoadFile2( UBYTE *filename, ULONG *pflags )
{
  UBYTE *start, *ptr, buf[100];
  ULONG code=0, askreq=0, ppdone=0;
  LONG readlen;
  ULONG locks, inputlocked=0;
  struct AsyncFile *file=0;
  //struct AZObjFile *fo = AGP.FObj;

    locks = func_VerifyOff();
    func_SetMainWaitPointer();
    if (TST_INPUTLOCK) inputlocked = 1; else func_ExecuteAZurCmd( "Lock INPUT ON", 0 );

    if (*pflags & LOAF_DECOMPRESS)
        {
        ULONG ppfile, ulb, filelen, ok=1, savres;
        struct Library *PPBase=0;
        UBYTE *filestart;

        if (ppfile = Open( filename, MODE_OLDFILE ))
            {
            if (Read( ppfile, &ulb, 4 ) == 4)
                {
                Close( ppfile ); ppfile = 0;
                if (ulb == B2L('P','P','2','0') || ulb == B2L('P','X','2','0'))
                    {
                    ok = 0;
                    if (PPBase = OpenLibrary( "powerpacker.library", 0L ))
                        {
                        savres = *(ULONG*)((ULONG)AGP.Process->pr_Task.tc_SPUpper-6);
                        *(ULONG*)((ULONG)AGP.Process->pr_Task.tc_SPUpper-6) = __builtin_getreg( 12 );

                        if (ppLoadData( filename, DECR_NONE, MEMF_ANY, &filestart, &filelen, get_password ) == 0)
                            {
                            SET_LOADINGFILE;
                            func_TextPut( filestart, filelen, (TST_STRIPLOAD) ? ARPF_STRIPEOL : ARPF_NOSTRIPEOL, 0 );
                            ok = 1;
                            ppdone = 1;
                            SET_COMPRESSED; // pour qu'il soit sauvé avec PowerPacker
                            FreeMem( filestart, filelen );
                            }
                        *(ULONG*)((ULONG)AGP.Process->pr_Task.tc_SPUpper-6) = savres;
                        CloseLibrary( PPBase );
                        }
                    }
                }
            if (ppfile) Close( ppfile );
            }

        if (! ok)
            {
            BufTruncate( AGP.UBufTmp, 0 );
            func_BufPrintf( &AGP.UBufTmp, 0, UIK_LangString( AGP.UIK, TEXT_Decompress_ReadErr ), filename );
            if (REQBUT_OK != func_IntSmartRequest( 0, 0,0, 0,AGP.UBufTmp, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 ))
                {
                ppdone = 1;
                code = TEXT_ERR_READFILE;
                }
            else *pflags |= LOAF_RES_FILENOICON; // ne pas charger l'icône si pas de décompactage
            }
        }

    if (! ppdone)
        {
        if (! (file = OpenAsync( filename, MODE_READ, AGP.Prefs->IOBufLength*1024 ))) { code = TEXT_ERR_OPENFILE; goto END_ERROR; }

        sprintf( buf, UIK_LangString( AGP.UIK, TEXT_ASQREQ_LoadFile ), UIK_BaseName( filename ) );
        if (lo_FileSize( filename ) > ((AGP.Prefs->IOBufLength*1024)*2))
            {
            askreq = func_AskReqBegin( buf, UIK_LangString( AGP.UIK, TEXT_ASQREQ_LoadTitle ) );
            }

        //------ Lecture du fichier
        SET_LOADINGFILE;
        while (TRUE)
            {
            //------ Lecture d'un bloc
            if ((readlen = ReadAsync( file, &start )) < 0) { code = TEXT_ERR_READFILE; goto END_ERROR; }
            if (readlen == 0) break;

            if (func_AskReqQuery( askreq ) != -1) goto END_ERROR;
            ptr = start;
            if (code = func_TextPut( start, readlen, (TST_STRIPLOAD) ? ARPF_STRIPEOL : ARPF_NOSTRIPEOL, 0 )) goto END_ERROR;
            }
        }

  END_ERROR:
    CLR_LOADINGFILE;
    if (file) CloseAsync( file );
    func_AskReqEnd( askreq );
    if (! inputlocked) func_ExecuteAZurCmd( "Lock INPUT OFF", 0 );
    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    return( code );
}

ULONG func_LoadFile( UBYTE *filename )
{
    return( func_LoadFile2( filename, 0 ) );
}

//-----------------------------------------------------------

static ULONG buildbackupname( UBYTE *filename, ULONG rotation ) // déjà fait UIK_BaseName()
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZurPrefs *prefs = AGP.Prefs;
  ULONG len, errmsg = TEXT_NOMEMORY;
  UBYTE *p=0, rotbuf[20];

    rotbuf[0] = 0;
    if (rotation) sprintf( rotbuf, "%ld", rotation );

    lo_GetPath( prefs->BackupDirName, &fo->BkupFileName );
    len = BufLength( fo->BkupFileName );

    if (TST_BKPFILENAME)
        {
        if (! prefs->BackupName[0]) // si vide : nom intégral
            {
            p = BufPasteS( filename, StrLen(filename), &fo->BkupFileName, len );
            }
        else{
            UBYTE add, *start, *ptr;

            //--- position de la pattern
            for (start=ptr=prefs->BackupName; *ptr && *ptr != '*'; ptr++);

            //--- ajout du début avant la pattern
            p = (APTR)1;
            if (add = ptr - start) p = BufPasteS( start, add, &fo->BkupFileName, len );
            if (p)
                {
                len += add;
                //--- ajout du nom de fichier
                add = StrLen( filename );
                if (p = BufPasteS( filename, add, &fo->BkupFileName, len ))
                    {
                    len += add;
                    //--- ajout du numéro de rotation
                    add = StrLen( rotbuf );
                    if (p = BufPasteS( rotbuf, add, &fo->BkupFileName, len ))
                        {
                        len += add;
                        //--- ajout du reste après la pattern
                        if (*ptr && *(ptr+1))
                            {
                            ptr++; // après la pattern
                            p = BufPasteS( ptr, StrLen(ptr), &fo->BkupFileName, len );
                            }
                        }
                    }
                }
            }
        }
    else{
        p = func_BufPrintf( &fo->BkupFileName, len, "AZur-%lx.bkp", FindTask(0) );
        }
    if (p) errmsg = 0;

    p = UIK_BaseName( fo->BkupFileName );
    if (strlen(p) > 30) BufTruncate( fo->BkupFileName, (ULONG)p - (ULONG)fo->BkupFileName + 30 );

    return( errmsg );
}

static ULONG do_backup( UBYTE *filename )
{
  struct AZObjFile *fo = AGP.FObj;
  ULONG errmsg=0, len, filelen=0, i;
  ULONG lock, parentlock, oldlock, blklen = AGP.Prefs->IOBufLength*1024;
  UBYTE *ptr;

    if (filename && filename[0] && (lock = Lock( filename, ACCESS_READ )))
        {
        if (Examine( lock, &AGP.FIB ))
            {
            if (AGP.FIB.fib_DirEntryType < 0) // file
                {
                filelen = AGP.FIB.fib_Size;

                if (TST_KEEPBKP)
                    {
                    struct AsyncFile *fin, *fout;
                    UBYTE savederr = 0, *readbuf;

                    if (parentlock = ParentDir( lock ))
                        {
                        oldlock = CurrentDir( parentlock );
                        ptr = UIK_BaseName( filename );

                        if (! (errmsg = buildbackupname( ptr, AGP.Prefs->BackupRot )))
                            {
                            if (strcmp( fo->BkupFileName, filename ) != 0)
                                {
                                if (AGP.Prefs->BackupRot) func_DeleteFile( fo->BkupFileName );

                                for (i=AGP.Prefs->BackupRot; i > 0; i--)
                                    {
                                    if (! BufSetS( fo->BkupFileName, BufLength(fo->BkupFileName), &fo->AutsFileName )) { errmsg = TEXT_NOMEMORY; break; }
                                    if (errmsg = buildbackupname( ptr, i-1 )) break;
                                    func_RenameFile( fo->BkupFileName, fo->AutsFileName );
                                    }

                                if (! errmsg)
                                    {
                                    if (fin = OpenAsync( ptr, MODE_READ, blklen ))
                                        {
                                        if (fout = OpenAsync( fo->BkupFileName, MODE_WRITE, blklen ))
                                            {
                                            while (filelen)
                                                {
                                                len = (filelen > blklen ? blklen : filelen);
                                                filelen -= len;
                                                if (ReadAsync( fin, &readbuf ) != len) break;
                                                if (WriteAsync( fout, readbuf, len ) != len)
                                                    {
                                                    //CloseAsync( fout ); fout = 0;
                                                    //func_DeleteFile( fo->BkupFileName );
                                                    savederr = 1;
                                                    break;
                                                    }
                                                }
                                            if (fout) CloseAsync( fout );
                                            }
                                        else savederr = 1;
                                        CloseAsync( fin );
                                        }
                                    else savederr = 1;
                                    }
                                else savederr = 1;
                                }
                            else{
                                UBYTE *ub;
                                if (ub = BufAllocP( AGP.Pool, 200, 50 ))
                                    {
                                    if (func_BufPrintf( &ub, 0, "%ls\n\n\"%ls\"", UIK_LangString( AGP.UIK, TEXT_ERR_SameBkpName ), filename ))
                                        func_ReqShowText( ub );
                                    BufFree( ub );
                                    }
                                }
                            }
                        CurrentDir( oldlock );
                        UnLock( parentlock );
                        }

                    if (savederr && !errmsg) errmsg = TEXT_ERR_BackupFile;
                    }
                }
            }
        UnLock( lock );
        }
    return( errmsg );
}

static ULONG replace_auts_name( UBYTE *filename )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZurPrefs *prefs = AGP.Prefs;
  ULONG len, errmsg = TEXT_NOMEMORY;
  UBYTE *p=0;

    filename = UIK_BaseName( filename );

    lo_GetPath( prefs->AutsDirName, &AGP.FObj->AutsFileName );
    len = BufLength( fo->AutsFileName );

    if (! prefs->AutsName[0]) // si vide : nom intégral
        {
        p = BufPasteS( filename, StrLen(filename), &fo->AutsFileName, len );
        }
    else{
        UBYTE add, *start, *ptr;

        //--- position de la pattern
        for (start=ptr=prefs->AutsName; *ptr && *ptr != '*'; ptr++);

        //--- ajout du début avant la pattern
        p = (APTR)1;
        if (add = ptr - start) p = BufPasteS( start, add, &fo->AutsFileName, len );
        if (p)
            {
            len += add;
            //--- ajout du nom de fichier
            add = StrLen( filename );
            if (p = BufPasteS( filename, add, &fo->AutsFileName, len ))
                {
                len += add;
                //--- ajout du reste après la pattern
                if (*ptr && *(ptr+1))
                    {
                    ptr++; // après la pattern
                    p = BufPasteS( ptr, StrLen(ptr), &fo->AutsFileName, len );
                    }
                }
            }
        }
    if (p) errmsg = 0;

    p = UIK_BaseName( fo->AutsFileName );
    if (strlen(p) > 30) BufTruncate( fo->AutsFileName, (ULONG)p - (ULONG)fo->AutsFileName + 30 );

    return( errmsg );
}

static ULONG same_autsname( UBYTE *filename, ULONG request )
{
  UBYTE *ub;

    if (strcmp( AGP.FObj->AutsFileName, filename ) == 0)
        {
        if (request)
            {
            if (ub = BufAllocP( AGP.Pool, 200, 50 ))
                {
                if (func_BufPrintf( &ub, 0, "%ls\n\n\"%ls\"", UIK_LangString( AGP.UIK, TEXT_ERR_SameAutsName ), filename ))
                    func_ReqShowText( ub );
                BufFree( ub );
                }
            }
        return(1);
        }
    return(0);
}

ULONG build_auts_name( UBYTE *filename )
{
  ULONG lock, parentlock, oldlock, rc=TEXT_ERR_BackupFile;

    if (filename[0] && (lock = Lock( filename, ACCESS_READ )))
        {
        if (Examine( lock, &AGP.FIB ))
            {
            if (AGP.FIB.fib_DirEntryType < 0) // file
                {
                if (parentlock = ParentDir( lock ))
                    {
                    oldlock = CurrentDir( parentlock );
                    if (replace_auts_name( filename ) == 0)
                        {
                        rc = 0;
                        }
                    CurrentDir( oldlock );
                    UnLock( parentlock );
                    }
                }
            }
        UnLock( lock );
        }
    return( rc );
}

static ULONG verif_ask_new_icon( UBYTE *filename, ULONG exist )
{
  ULONG len, icon=1;
  struct DiskObject *d;

    if (exist)
        {
        if (d = GetDiskObject( filename )) FreeDiskObject( d );
        if (! d)
            {
            if (TST_NEWICONASKED) icon = 0;
            else if (REQBUT_OK != func_IntSmartRequest( 0, 0,0, TEXT_ConfirmCreateIcon,0, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 ))
                { SET_NEWICONASKED; icon = 0; }
            }
        }
    return( icon );
}

static void verif_delete_icon( UBYTE *filename )
{
/* Pas encore installé cette option. Est-ce utile, car il y a la requête après le test des dates.
  struct DiskObject *d;

    if (d = GetDiskObject( filename ))
        {
        FreeDiskObject( d );
        func_DeleteDiskObject( filename );
        }
*/
}

/* Ici la liste n'est pas forcément FObj->Text, donc attention aux fonctions utilisées !!! pas de intAZ_Buf_Len( line, &ptr )
 */

struct write_info {
    APTR    file;
    UWORD   do_eol;
    UWORD   do_space;
    ULONG   flags;
    UBYTE   **pupar;
    struct mempar *curpar;
    UBYTE   **store;
    };

static ULONG write_store( struct write_info *wi, UBYTE *buf, ULONG len )
{
    if (wi->file)
        { if (WriteAsync( wi->file, buf, len ) < 0) return( TEXT_ERR_WRITEFILE ); }
    else
        { if (! BufPasteS( buf, len, wi->store, BufLength(*wi->store) )) return( TEXT_NOMEMORY ); }
    return(0);
}

static ULONG writeline_folded( struct write_info *wi, LONG real, struct UIKPList *ul, LONG line )
{
  struct mempar MP;
  ULONG len, CONTINUE_LINE=0, errmsg;
  UBYTE *ptr, *streol;
  struct UIKNode *node;

    if (wi->do_eol)
        {
        if (TST_SAVELF2CR) { streol = "\r"; len = 1; }
        else if (TST_SAVELF2CRLF) { streol = "\r\n"; len = 2; }
        else { streol = "\n"; len = 1; }
        //if (WriteAsync( wi->file, streol, len ) < 0) return( TEXT_ERR_WRITEFILE );
        if (errmsg = write_store( wi, streol, len )) return( errmsg );
        }
    else if (wi->do_space)
        {
        //if (WriteAsync( wi->file, " ", 1 ) < 0) return( TEXT_ERR_WRITEFILE );
        if (errmsg = write_store( wi, " ", 1 )) return( errmsg );
        }

    node = &ul->UNode[line];
    len = func_LineBufLen( (ptr = node->Buf) ); // pas de intAZ_Buf_Len à cause des folds...
    if ((node->Flags & UNF_CONTINUE) && (wi->flags & SAVF_PARAGRAPH)) CONTINUE_LINE = 1;

    if (node->Flags & UNF_CONTINUE) // pour la sauvegarde des paragraphes dans l'icône
        {
        if (wi->curpar == 0) // début de paragraphe
            {
            MP.sline = real;
            MP.eline = -1;
            if (BufPasteS( (APTR)&MP, sizeof(struct mempar), wi->pupar, BufLength(*wi->pupar) ))
                wi->curpar = (struct mempar *) (((ULONG)(*wi->pupar)) + BufLength(*wi->pupar) - sizeof(struct mempar));
            }
        }
    else{
        if (wi->curpar)
            {
            wi->curpar->eline = real - 1;
            wi->curpar = 0;
            }
        }

    if (TST_STRIPSAVE) len = skip_endchar( ptr, ' ', '\t', len );
    if (len)
        {
        if (wi->flags & SAVF_PARAGRAPH)
            {
            ULONG num = skip_char( ptr, ' ', '\t' ); // on vire tous les espaces de début
            len -= num; ptr += num;
            }

        if (len)
            {
            if (wi->flags & SAVF_SPACE2TAB) // conversion espaces --> tabs ?
                {
                LONG ok, c, last=0, col=0, tabcol, lensp;
                while (col < len)
                    {
                    if (ptr[col] == ' ')
                        {
                        ok = 0;
                        if ((tabcol = next_tab(col)) <= len)
                            {
                            for (lensp=0, ok=1, c=tabcol-1; c >= col; c--)
                                {
                                if (ptr[c] != ' ') { ok = 0; break; }
                                lensp++;
                                }
                            }
                        if (ok && lensp > 1)
                            {
                            if (col - last)
                                {
                                // if (WriteAsync( wi->file, &ptr[last], col - last ) < 0) return( TEXT_ERR_WRITEFILE );
                                if (errmsg = write_store( wi, &ptr[last], col - last )) return( errmsg );
                                }
                            // if (WriteAsync( wi->file, "\t", 1 ) < 0) return( TEXT_ERR_WRITEFILE );
                            if (errmsg = write_store( wi, "\t", 1 )) return( errmsg );
                            col = tabcol - 1;
                            last = tabcol;
                            }
                        }
                    col++;
                    }
                if (col - last)
                    {
                    // if (WriteAsync( wi->file, &ptr[last], col - last ) < 0) return( TEXT_ERR_WRITEFILE );
                    if (errmsg = write_store( wi, &ptr[last], col - last )) return( errmsg );
                    }
                }
            else{
                // if (WriteAsync( wi->file, ptr, len ) < 0) return( TEXT_ERR_WRITEFILE );
                if (errmsg = write_store( wi, ptr, len )) return( errmsg );
                }
            }
        }

    if (CONTINUE_LINE) { wi->do_eol = 0; wi->do_space = 1; }
    else { wi->do_eol = 1; wi->do_space = 0; }
    return(0);
}

ULONG func_SaveFile( UBYTE *filename, ULONG nobkp, ULONG append, ULONG noicon, ULONG (*func)(), APTR parm )      // Asynchrone
{
  struct write_info WI;
  ULONG /*num, line, len,*/ errmsg=0, protect=0;
  ULONG blklen = AGP.Prefs->IOBufLength*1024, filesize=0;
  struct AsyncFile *file=0;
  /* UBYTE savech, flag, *ptr; */
  ULONG locks, flags=0, ppdone=0, exist=0;
  UBYTE *ub=0, *upar=0, *store;

    __builtin_memset( &WI, 0, sizeof(struct write_info) );

    if (func == 0) flags = (ULONG)parm;
    if (append && (flags & SAVF_COMPRESS)) { errmsg = TEXT_ERR_AppendCompress; goto END_ERROR; }

    locks = func_VerifyOff();
    func_SetMainWaitPointer();

    ub = BufAllocP( AGP.Pool, 0, 50*sizeof(struct memfold) );
    upar = BufAllocP( AGP.Pool, 0, 50*sizeof(struct mempar) );

    if (lo_FileExists( filename ))
        {
        exist = 1;
        protect = AGP.FIB.fib_Protection & ~FIBF_ARCHIVE;
        filesize = AGP.FIB.fib_Size;
        if (nobkp == 0)
            if (errmsg = do_backup( filename )) goto END_ERROR;
        }

    if (flags & SAVF_COMPRESS)
        {
        struct Library *PPBase=0;
        if (PPBase = OpenLibrary( "powerpacker.library", 35 ))
            {
            ULONG ppfile, crunchedlen;
            APTR crunchinfo;
            if (crunchinfo = ppAllocCrunchInfo( CRUN_VERYGOOD, SPEEDUP_BUFFMEDIUM, 0, 0 ))
                {
                if (! (store = BufAllocP( AGP.Pool, 0, filesize?filesize+10*1024:50*1024 )))
                    store = BufAllocP( AGP.Pool, 0, 20*1024 );
                if (store)
                    {
                    WI.flags = flags;
                    WI.pupar = &upar;
                    WI.store = &store;
                    if ((errmsg = func_FoldApplyLines( writeline_folded, &WI, &ub )) == 0)
                        {
                        crunchedlen = ppCrunchBuffer( crunchinfo, store, BufLength(store) );
                        if (crunchedlen == PP_CRUNCHABORTED || crunchedlen == PP_BUFFEROVERFLOW) errmsg = TEXT_ERR_WRITEFILE;
                        else if (ppfile = Open( filename, MODE_NEWFILE ))
                            {
                            if (! ppWriteDataHeader( ppfile, CRUN_VERYGOOD, 0, 0 )) errmsg = TEXT_ERR_WRITEFILE;
                            else{
                                if (Write( ppfile, store, crunchedlen ) != crunchedlen) errmsg = TEXT_ERR_WRITEFILE;
                                else ppdone = 1;
                                }
                            Close( ppfile );
                            }
                        else errmsg = TEXT_ERR_OPENWRITEFILE;
                        }
                    BufFree( store );
                    }
                else errmsg = TEXT_NOMEMORY;
                ppFreeCrunchInfo( crunchinfo );
                }
            CloseLibrary( PPBase );
            }

        if (errmsg)
            {
            BufTruncate( AGP.UBufTmp, 0 );
            func_BufPrintf( &AGP.UBufTmp, 0, UIK_LangString( AGP.UIK, TEXT_Compress_WriteErr ), filename );
            if (REQBUT_OK != func_IntSmartRequest( 0, 0,0, 0,AGP.UBufTmp, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 )) goto END_ERROR;
            errmsg = 0;
            }
        }

    if (ppdone == 0)
        {
        if (! (file = OpenAsync( filename, append ? MODE_APPEND : MODE_WRITE, blklen ))) { errmsg = TEXT_ERR_OPENWRITEFILE; goto END_ERROR; }

        if (func == 0)
            {
            WI.file = file;
            WI.flags = flags;
            WI.pupar = &upar;
            if (errmsg = func_FoldApplyLines( writeline_folded, &WI, &ub )) goto END_ERROR;
            }
        else{
            errmsg = (*func)( file, parm );
            }
        }

  END_ERROR:
    if (file) CloseAsync( file );
    if (errmsg == 0)
        {
        SetProtection( filename, protect );
        if (func == 0 && noicon == 0) // mettre avant build_auts_name(), sinon problème dans le cas de
            {                         // fichier autosave (filename pointe sur AGP.FObj->AutsFileName)
            if (TST_SAVEICON)
                {
                if ((flags & SAVF_AUTSFILE) || verif_ask_new_icon( filename, exist ))
                    {
                    if (flags & SAVF_PARAGRAPH) func_SaveIcon( filename );  // aucune information
                    else save_icon( filename, (struct memfold *)ub, (struct mempar *)upar, flags );
                    }
                }
            else verif_delete_icon( filename );
            }
        if (build_auts_name( filename ) == 0)  // efface le fichier de sauvegarde automatique associé
            {
            if (! same_autsname( filename, 0 )) func_DeleteFile( AGP.FObj->AutsFileName );
            }
        }
    BufFree( upar );
    BufFree( ub );
    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    return( errmsg );
}

void do_autosave()
{
  ULONG done=1;

    if (TST_MODIFIED && TST_AUTOSAVE && (AGP.FObj->Flags & AZFILEF_MODAUTS))
        {
        AGP.FObj->Flags &= ~AZFILEF_MODAUTS; /* évite deux requesters superposés */
        if (build_auts_name( AGP.FObj->OpenFileName ) == 0)
            {
            if (! same_autsname( AGP.FObj->OpenFileName, 1 ))
                {
                ULONG ok=1;

                if (TST_REQAUTOSAVE)
                    {
                    BufTruncate( AGP.UBufTmp, 0 );
                    func_BufPrintf( &AGP.UBufTmp, 0, UIK_LangString( AGP.UIK, TEXT_AutoSaveRequest ), AGP.FObj->OpenFileName );
                    if (REQBUT_OK != func_IntSmartRequest( 0, 0,0, 0,AGP.UBufTmp, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 ))
                        ok = 0;
                    }
                if (ok)
                    {
                    if (func_SaveFile( AGP.FObj->AutsFileName, 1, 0, 0, 0, (APTR)SAVF_AUTSFILE )) done = 0;
                    }
                }
            }
        if (! done) AGP.FObj->Flags |= AZFILEF_MODAUTS; /* remet le flag si erreur */
        }
}

#ifdef AZUR_DEMO
static void verif_timer()
{
    /*------ Démo uniquement ------*/
    ULONG secs = AGP.Timer->TotalTime.tv_secs * 2;
    if (secs / 5 > ((AZURDEMO_MAXTIME*2)/5))  /* pour valeur pas reconnaissable */
        Signal( AGP.AZMast->Process, 0x00020000 | SIGBREAKF_CTRL_C ); /* pour valeur pas reconnaissable */
}
#endif

void AutosaveFunc()
{
#ifdef AZUR_DEMO
    verif_timer();
#endif
    if (AGP.Prefs->AutsIntSec) do_autosave();
}

void AutosaveMod()
{
    if (AGP.Prefs->AutsIntMod) do_autosave();
}

//-----------------------------------------------------------------------------------------

/*
    if(DList->dol_Type == DLT_DEVICE)
    {
        struct  FileSysStartupMsg *FSSM = (struct FileSysStartupMsg *)BADDR(DList->dol_misc.dol_handler.dol_Startup);
        DoGet = FALSE;

        if(FSSM && TypeOfMem(FSSM) && TypeOfMem(BADDR(FSSM->fssm_Device)) && TypeOfMem(BADDR(FSSM->fssm_Environ)))
        {
            if(*((char *)BADDR(FSSM->fssm_Device)) != 255)
            {
                struct  DosEnvec    *DE = (struct DosEnvec *)BADDR(FSSM->fssm_Environ);

                if(DE && TypeOfMem(DE))
                {
                    if(DE->de_Surfaces && DE->de_BlocksPerTrack)
                        DoGet = TRUE;
                }
            }
        }
    }
*/

static UBYTE *set_pattern( struct UIKObjWinFileSel *wfs, UBYTE *pattern )
{
  UBYTE *ptr, *oldpat=0;
  ULONG len;

    if (pattern && pattern[0])
        {
        ptr = UIK_OSt_Get( wfs->FS->StrPatGad, &len );
        if (oldpat = PoolAlloc( len + 1 ))
            {
            StrCpy( oldpat, ptr ); // sauve ancien contenu
            UIK_Call( wfs->FS, UIKFUNC_FS_ChangePattern, (ULONG)pattern, 0 );
            }
        }
    return( oldpat );
}

static void reset_pattern( struct UIKObjWinFileSel *wfs, UBYTE *oldpat )
{
    if (oldpat)
        {
        UIK_Call( wfs->FS, UIKFUNC_FS_ChangePattern, (ULONG)oldpat, 0 );
        PoolFree( oldpat );
        }
}

static UBYTE * __far devlist[] = { "NIL:", "GMC:", "CON:", "NEWCON:", "RAW:", "SER:", "PAR:", "PRT:", 0 };

ULONG IsNotFileSystem( UBYTE *name )
{
  ULONG save, i, pseudo=0;
  UBYTE *dev;

    save = UIK_DisableDOSReq();
    if (UIK_IsSystemVersion2())
        {
        if (! IsFileSystem( name )) pseudo = 1;
        }
    else{
        for (i=0; dev = devlist[i]; i++)
            if (func_IsNCSubStr( dev, name )) { pseudo = 1; break; }
        }
    UIK_EnableDOSReq( 0, save );
    return( pseudo );
}

ULONG MayBeFileName( UBYTE *name, ULONG create )
{
    if (IsNotFileSystem( name )) return(1);
    if (lo_GetFIB( name ))
        {
        if (AGP.FIB.fib_DirEntryType < 0) return(1);
        }
    else{  // pas si directory
        ULONG len = StrLen( name );
        if (create && len && name[len-1] != ':') return(1);
        }
    return(0);
}

static UBYTE *ReqFileName( struct UIKObjWinFileSel *wfs1, struct UIKObjWinFileSel *wfs2, UBYTE *givenname, UBYTE *pattern, UBYTE *wintitle, ULONG *errcode, ULONG create )
{
  ULONG path=0;
  UBYTE *oldpat=0;

    if (givenname && givenname[0])
        {
        if (MayBeFileName( givenname, create )) return( givenname );
        path = 1;
        }

    if (wfs1)
        {
        if (wfs1->Obj.Status == UIKS_STARTED) goto END_CANCEL;
        }
    if (wfs2)
        {
        if (path) UIK_Call( wfs2->FS, UIKFUNC_FS_ChangePath, (ULONG)givenname, 0 );
        UIK_Call( wfs2->WO, UIKFUNC_Window_ChangeTitle, (ULONG)wintitle, 0 );
        oldpat = set_pattern( wfs2, pattern );

        if (! UIK_Start( wfs2 )) goto END_MEMORY;
        if (sync_request( wfs2 ) != REQBUT_OK) goto END_CANCEL;
        givenname = AGP.FileSelResult;
        reset_pattern( wfs2, oldpat );
        }

    return( givenname );

  END_CANCEL:
    if (errcode) *errcode = TEXT_CMDCANCELLED;
    return(0);
  END_MEMORY:
    if (errcode) *errcode = TEXT_NOMEMORY;
    return(0);
}

UBYTE *func_ReqFileNameO( UBYTE *givenname, UBYTE *pattern, UBYTE *wintitle, ULONG *errcode )
{
    return( ReqFileName( AGP.FObj->SaveFS, AGP.FObj->OpenFS, givenname, pattern, wintitle, errcode, 0 ) );
}

UBYTE *func_IntReqFileNameO( UBYTE *givenname, UBYTE *pattern, ULONG titleindex, ULONG *errcode, ULONG create )
{
    return( ReqFileName( AGP.FObj->SaveFS, AGP.FObj->OpenFS, givenname, pattern, UIK_LangString( AGP.UIK, titleindex ), errcode, create ) );
}

UBYTE *func_ReqFileNameS( UBYTE *givenname, UBYTE *pattern, UBYTE *wintitle, ULONG *errcode )
{
    return( ReqFileName( AGP.FObj->OpenFS, AGP.FObj->SaveFS, givenname, pattern, wintitle, errcode, 0 ) );
}

UBYTE *func_IntReqFileNameS( UBYTE *givenname, UBYTE *pattern, ULONG titleindex, ULONG *errcode, ULONG create )
{
    return( ReqFileName( AGP.FObj->OpenFS, AGP.FObj->SaveFS, givenname, pattern, UIK_LangString( AGP.UIK, titleindex ), errcode, create ) );
}

//-----------------------------------------------------------------------------------------

struct DiskObject *func_GetDiskObject( STRPTR name ) // Si intêressé par l'icône par défaut
{
    return( (struct DiskObject *) (UIK_IsSystemVersion2() ? GetDiskObjectNew( name ) : GetDiskObject( name )) );
}

void func_DeleteDiskObject( STRPTR name )
{
    if (UIK_IsSystemVersion2())
        {
        DeleteDiskObject( name );
        }
    else{
        ULONG len = StrLen( name );
        UBYTE *mem;
        if (len && (len+5) <= 30)
            if (mem = UIK_MemSpAlloc( len + 5, MEMF_ANY ))
                {
                StrCpy( mem, name ); StrCpy( mem + len, ".info" );
                DeleteFile( mem );
                UIK_MemSpFree( mem );
                }
        }
}

void func_CopyIcon( STRPTR from, STRPTR to )
{
  struct DiskObject *diskobj;

    if (diskobj = GetDiskObject( from )) // Ne pas prendre l'icône par défaut, donc pas my_GetDiskObject()
        {
        PutDiskObject( to, diskobj );
        FreeDiskObject( diskobj );
        }
}

void func_RenameFile( STRPTR from, STRPTR to )
{
    Rename( from, to );
    func_CopyIcon( from, to );
}

void func_DeleteFile( STRPTR name )
{
    DeleteFile( name );
    func_DeleteDiskObject( name );
}

//----------------------------------------------------------------------

static STRPTR icon_add_attrs( APTR pool )
{
  STRPTR ub=0, attrtab[] = { "RO", "UM", "AF", "EOF", "EOL", "T2S", "S2T", "RM", "LM", "Tab", "SCRLF", "SCR", "LCRLF", "LCR", "SL", "SS", "SC", 0 };
  ULONG i;

    if (ub = (APTR) BufAllocP( pool, 0, 50 ))
        {
        if (func_BufPrintf( &ub, 0, "AZURATTR=" ))
            {
            for (i=0; attrtab[i]; i++)
                {
                BufTruncate( AGP.RexxResult, 0 );
                if (GetSet_OnePref( attrtab[i], 0, 0 ) == 0)
                    {
                    if (AGP.RexxResult[0])
                        {
                        func_BufPrintf( &ub, BufLength(ub), "%ls %ls", attrtab[i], AGP.RexxResult );
                        }
                    }
                }
            }
        }
    return( ub );
}

static STRPTR icon_add_curpos( APTR pool )
{
  struct AZObjFile *fo=AGP.FObj;
  struct AZObjZone *zo=0;
  STRPTR ub=0;

    if (AGP.WO) zo = fo->ActiveView->Zone;
    if (ub = (APTR) BufAllocP( pool, 0, 50 ))
        func_BufPrintf( &ub, 0, "AZURPOS=%ld %ld/%ld %ld/%ld %ld",
            (zo ? zo->TopLine : 0), (zo ? zo->TopCol : 0),
            fo->Line, fo->Col,
            fo->LCLine, fo->LCCol
            );
    return( ub );
}

static ULONG bloc_2_tt( struct AZBlock *azb, ULONG id, UIKBUF **pub )
{
    if (func_BufPrintf( pub, BufLength(*pub), "%ld %ld %ld %ld %ld %ld/", azb->LineStart, azb->ColStart, azb->LineEnd, azb->ColEnd, azb->Mode, azb->Flags ))
        return(0);
    return(1);
}
static STRPTR icon_add_blocks( APTR pool )
{
  STRPTR ub=0;

    if (func_BlockExists())
        if (ub = (APTR) BufAllocP( pool, StrLen("AZURBLOCK="), 50 ))
            {
            StrCpy( ub, "AZURBLOCK=" );
            func_BlockApply( bloc_2_tt, 1, -1, 0, &ub );
            }
    return( ub );
}

static ULONG mark_2_tt( struct AZBM *azb, ULONG id, UIKBUF **pub )
{
    if (func_BufPrintf( pub, BufLength(*pub), "%ld %ld %ld %ls\n", azb->Line, azb->Col, azb->Flags, azb->Name ))
        return(0);
    return(1);
}
static STRPTR icon_add_marks( APTR pool )
{
  STRPTR ub=0;

    if (func_BMExists())
        if (ub = (APTR) BufAllocP( pool, StrLen("AZURMARK="), 50 ))
            {
            StrCpy( ub, "AZURMARK=" );
            func_BMApply( mark_2_tt, 1, -1, 0, &ub );
            }
    return( ub );
}

static STRPTR icon_add_paras( APTR pool, struct mempar *mp )
{
  ULONG i, num;
  STRPTR ub=0;

    if (mp && (num = BufLength((APTR)mp)/sizeof(struct mempar)) && (ub = (APTR) BufAllocP( pool, 0, 200 )))
        {
        for (i=0; i < num; i++, mp++)
            if (! func_BufPrintf( &ub, BufLength(ub), "%ld %ld/", mp->sline, mp->eline )) break;
        if (ub[0])
            ub = BufPaste( "AZURPARA=", StrLen("AZURPARA="), ub, 0 );
        }
    return( ub );
}

static STRPTR icon_add_folds( APTR pool, struct memfold *mf )
{
  ULONG i, num;
  STRPTR ub=0;

    if (mf && (num = BufLength((APTR)mf)/sizeof(struct memfold)) && (ub = (APTR) BufAllocP( pool, 0, 200 )))
        {
        for (i=0; i < num; i++, mf++)
            if (! func_BufPrintf( &ub, BufLength(ub), "%ld %ld %ld %ld/", mf->sline, mf->eline, mf->folded, mf->id )) break;
        if (ub[0])
            ub = BufPaste( "AZURFOLD=", StrLen("AZURFOLD="), ub, 0 );
        }
    return( ub );
}

static void save_icon( STRPTR name, struct memfold *mf, struct mempar *mp, ULONG saveflags )

{
  struct DiskObject defdobj = def_text;
  struct DiskObject *d=0, *dobj;
  APTR pool;
  STRPTR *tooltypes, *oldtooltypes, olddefaulttool, *tt, str, ub;
  ULONG len, overtool=1;

    if (! (pool = AsmCreatePool( MEMF_PUBLIC|MEMF_CLEAR, 1024, 1024, *(APTR*)4 ))) return;

    if (! (d = GetDiskObject( name )))
        if (IoErr() == ERROR_OBJECT_NOT_FOUND)
            if (! (d = GetDiskObject( "ENV:Sys/def_text" )))
                d = GetDiskObject( "AZur:Icons/def_text" );

    if (! (tooltypes = (APTR) BufAllocP( pool, 0, 4*20 ))) goto END_ERROR;

    str = "FILETYPE=TEXT|ASCII";
    if (! BufPasteS( (APTR)&str, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;
    if (TST_READONLY)
        {
        str = "READONLY";
        if (! BufPasteS( (APTR)&str, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;
        }

    if (d)
        {
        dobj = d;
        if (tt = dobj->do_ToolTypes)
            {
            for (; *tt; tt++)
                {
                if (*tt[0] && ARG_MatchArg( "READONLY,FILETYPE,AZURATTR,AZURPOS,AZURBLOCK,AZURMARK,AZURFOLD,AZURPARA", *tt ) == -1)
                    {
                    len = StrLen( *tt );
                    if (! (ub = BufAllocP( pool, len, len ))) goto END_ERROR;
                    StrCpy( ub, *tt );
                    if (! BufPasteS( (APTR)&ub, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;
                    }
                }
            }
        if (d->do_DefaultTool) // demander avant de changer le DefaultTool de l'icône ?
            {
            if (! (saveflags & SAVF_AUTSFILE)) // à cause des utilitaires qui changent les icônes, pour
                {                              // ne pas avoir la requête à chaque autosave...
                UBYTE *ptrdef, *ptrprefs;
                ptrdef = UIK_BaseName( d->do_DefaultTool );
                ptrprefs = UIK_BaseName( AGP.Prefs->IconTool );
                if (StrNCCmp( ptrdef, ptrprefs ) && StrNCCmp( ptrdef, "AZur" ) && StrNCCmp( ptrdef, "AZurQuick" ))
                    {
                    /* on doit demander l'écrasement du Default Tool */
                    if (TST_TOOLASKED) overtool = 0;
                    else{
                        BufTruncate( AGP.UBufTmp, 0 );
                        func_BufPrintf( &AGP.UBufTmp, 0, UIK_LangString( AGP.UIK, TEXT_OverDefaultTool ), d->do_DefaultTool, AGP.Prefs->IconTool );
                        if (REQBUT_OK != func_IntSmartRequest( 0, 0,0, 0,AGP.UBufTmp, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 ))
                            { overtool = 0; SET_TOOLASKED; }
                        }
                    }
                }
            }
        }
    else dobj = &defdobj;

    if (ub = icon_add_attrs( pool ))
        if (! BufPasteS( (APTR)&ub, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;
    if (ub = icon_add_curpos( pool ))
        if (! BufPasteS( (APTR)&ub, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;
    if (ub = icon_add_blocks( pool ))
        if (! BufPasteS( (APTR)&ub, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;
    if (ub = icon_add_marks( pool ))
        if (! BufPasteS( (APTR)&ub, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;
    if (ub = icon_add_paras( pool, mp ))
        if (! BufPasteS( (APTR)&ub, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;
    if (ub = icon_add_folds( pool, mf ))
        if (! BufPasteS( (APTR)&ub, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;

    ub = 0;   // insère le pointeur nul de fin
    if (! BufPasteS( (APTR)&ub, 4, (APTR)&tooltypes, BufLength((APTR)tooltypes) )) goto END_ERROR;

    oldtooltypes   = dobj->do_ToolTypes;
    olddefaulttool = dobj->do_DefaultTool;
    dobj->do_ToolTypes   = tooltypes;
    if (overtool) dobj->do_DefaultTool = AGP.Prefs->IconTool;

    PutDiskObject( name, dobj );

    dobj->do_ToolTypes   = oldtooltypes;
    dobj->do_DefaultTool = olddefaulttool;

  END_ERROR:
    if (d) FreeDiskObject( d );
    AsmDeletePool( pool, *(APTR*)4 );
}

void func_SaveIcon( STRPTR name )
{
  struct DiskObject defdobj = def_text;
  struct DiskObject *d=0, *dobj;
  STRPTR olddefaulttool;

    if (d = GetDiskObject( name )) goto END_ERROR; // on écrase pas une icône pré-existante
    if (IoErr() != ERROR_OBJECT_NOT_FOUND) goto END_ERROR; // idem

    if (! (d = GetDiskObject( "ENV:Sys/def_text" )))
        d = GetDiskObject( "AZur:Icons/def_text" );

    if (! (dobj = d)) dobj = &defdobj;

    olddefaulttool = dobj->do_DefaultTool;
    dobj->do_DefaultTool = AGP.Prefs->IconTool;

    PutDiskObject( name, dobj );

    dobj->do_DefaultTool = olddefaulttool;

  END_ERROR:
    if (d) FreeDiskObject( d );
}

//--------------------------------------------

#define MAXREADSTRLEN 100

static ULONG read_string( STRPTR ptr, STRPTR res, STRPTR *pptr )
{
  ULONG i;

    for (i=0; *ptr && *ptr != '\n' && (i < MAXREADSTRLEN); i++) *res++ = *ptr++;
    *res = 0;
    if (*ptr) ptr++;
    if (pptr) *pptr = ptr;
    return( i );
}

static ULONG read_longs( STRPTR ptr, ULONG n, ULONG *res, STRPTR *pptr )
{
  ULONG i;

    for (i=0; *ptr && i < n; i++)
        {
        res[i] = Str2Num( ptr ); ptr = (STRPTR)__builtin_getreg( 8 );
        }
    if (pptr) *pptr = ptr;
    return( i );
}

static ULONG tt_doparas( STRPTR desc )
{
  struct AZObjFile *fo=AGP.FObj;
  struct mempar MP;
  LONG i, last = fo->Text.NodeNum - 1;
  struct UIKNode *node;

    while (2 == read_longs( desc, 2, (ULONG*)&MP, &desc ))
        {
        if (MP.eline == -1 || MP.eline > last) MP.eline = last;
        node = &fo->Text.UNode[MP.sline];
        for (i=MP.sline; i <= MP.eline; i++, node++)
            { node->Flags |= (UNF_CONTINUE+UNF_ENDSPACE); }
        }
    return(0);
}

static ULONG tt_dofolds( STRPTR desc )
{
  struct memfold MF, *mf;
  LONG num, i, lastid;
  STRPTR ub;

    if (ub = BufAllocP( AGP.Pool, 0, 50*sizeof(struct memfold) ))
        {
        while (4 == read_longs( desc, 4, (ULONG*)&MF, &desc ))
            if (! BufPasteS( (APTR)&MF, sizeof(struct memfold), &ub, BufLength(ub) )) break;

        if (num = BufLength(ub) / sizeof(struct memfold))
            {
            for (lastid=0, mf=(struct memfold *)ub, i=0; i < num; i++, mf++)
                {
                if (mf->id > lastid+1) goto ERROR_FOLD;
                func_FoldSet( mf->sline, mf->eline, 0 );
                lastid = mf->id;
                }
            for (mf=&((struct memfold *)ub)[num-1], i=num-1; i >= 0; i--, mf--)
                {
                if (mf->folded) func_FoldOn( mf->sline, -1 );
                }
            }
        ERROR_FOLD:
        BufFree( ub );
        }
    return(0);
}

static ULONG tt_domarks( STRPTR desc )
{
  struct AZBM *azb;
  UBYTE name[100];
  ULONG res[3];

    while (3 == read_longs( desc, 3, res, &desc ))
        {
        if (read_string( desc, name, &desc ))
            if (azb = func_BMAdd( name, res[0], res[1] ))
                azb->Flags = res[2];
        }
    return(0);
}

static ULONG tt_doblocks( STRPTR desc )
{
  struct AZBlock *azb;
  ULONG res[6];

    while (6 == read_longs( desc, 6, res, &desc ))
        {
        if (func_BlockStart( res[0], res[1], 0 ))
            if (azb = func_BlockDefine( res[2], res[3], res[4], 0 ))
                azb->Flags = res[5];
        }
    return(0);
}

static ULONG tt_dopos( STRPTR desc )
{
  struct AZObjFile *fo=AGP.FObj;
  struct AZObjZone *zo=0;
  ULONG res[6];

    if (6 == read_longs( desc, 6, res, 0 ))
        {
        if (AGP.WO && (zo = fo->ActiveView->Zone))
            {
            zo->TopLine = res[0];
            zo->TopCol  = res[1];
            fo->Line    = res[2];
            fo->Col     = res[3];
            fo->LCLine  = res[4];
            fo->LCCol   = res[5];
            }
        }
    return(0);
}

static ULONG tt_doattrs( STRPTR desc )
{
    if (desc && *desc) GetSetPrefs( desc, 1, 0 );
    return(0);
}

static ULONG tt_search_do( STRPTR template, struct DiskObject *dobj, ULONG (*func)() )
{
  STRPTR *tt, ptr;

    if (tt = dobj->do_ToolTypes)
        {
        for (; (ptr = *tt); tt++)
            {
            if (*ptr && (ARG_MatchArg( template, ptr ) == 0))
                {
                while (*ptr && *ptr != '=') ptr++;
                if (*ptr == 0) break;
                (*func)( ptr + 1 );
                break;
                }
            }
        }
    return(0);
}

void load_icon( STRPTR filename )
{
  struct DiskObject *dobj;

    if (! (dobj = GetDiskObject( filename ))) return;

    {
    struct DateStamp date;
    UBYTE buf[300];

    StrCpy( buf, filename ); StrCat( buf, ".info" );
    if (lo_NewerFile( filename, buf, &date, 0, 0, 0 ) == 1)
        {
        if (REQBUT_OK != func_IntSmartRequest( 0, 0,0, TEXT_OldIcon,0, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 ))
            {
            if (REQBUT_OK == func_IntSmartRequest( 0, 0,0, TEXT_DeleteIcon,0, 0,0,0,0, -1,0,-1, 0, 0, 0, 0 ))
                func_DeleteDiskObject( filename );
            goto END_ERROR;
            }
        else if (UIK_IsSystemVersion2())
            {
            ULONG save = UIK_DisableDOSReq();
            SetFileDate( buf, &date );
            UIK_EnableDOSReq( 0, save );
            }
        }
    }

    tt_search_do( "AZURPARA",  dobj, tt_doparas  );
    tt_search_do( "AZURFOLD",  dobj, tt_dofolds  );
    tt_search_do( "AZURMARK",  dobj, tt_domarks  );
    tt_search_do( "AZURBLOCK", dobj, tt_doblocks );
    tt_search_do( "AZURPOS",   dobj, tt_dopos    );
    tt_search_do( "AZURATTR",  dobj, tt_doattrs  );

  END_ERROR:
    FreeDiskObject( dobj );
}
