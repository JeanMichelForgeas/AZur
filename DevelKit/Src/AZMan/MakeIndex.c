/****************************************************************
 *
 *      Project:    MakeAutoDocsIndex
 *      Function:   Création des index pour la
 *                  Documentation en ligne de AZur
 *
 *      Created:    12/02/94    Patrice TROGNON
 *
 *                  Copyright (C) 1994,
 *          Patrice Trognon & Jean-Michel Forgeas
 *                 Tous Droits Réservés
 *
 *      Remarques:  Dans ce source les lignes de codes pour l'intégration
 *                  du traitement des documentations compactées sont
 *                  présentes.
 *                  Nous n'avons pas jugé utile de les laisser actives
 *                  pour la présente version, si vous voulez utiliser des
 *                  AutoDocs compressées les lignes de codes UIK sont
 *                  prévues.
 *                  Nous laissons ainsi au programmeur éventuel le choix
 *                  de la méthode de compression a utiliser.
 *
 ****************************************************************/

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikglobal.h"
#include "uiki:uikcolors.h"
#include "uiki:uikmacros.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objscreen.h"
#include "uiki:objwindow.h"
#include "uiki:objstring.h"
#include "uiki:objchecktoggle.h"
#include "uiki:objprogress.h"
#include "uiki:objlist.h"
#include "uiki:objproplift.h"
#include "uiki:objreqstring.h"

#include "AZMan.h"

#define MAINWINDOW_TITLE         1
#define DOCSPRESENT_TITLE        2
#define DOCSTOPERFORM_TITLE      3
#define AJOUTER_TITLE            4
#define SUPPRIMER_TITLE          5
#define COMPRESSION_TITLE        6
#define MAKEINDEX_TITLE          7
#define PROGRESSWINDOW_TITLE     8
#define CURRENTDOC_TITLE         9
#define CURRENTFNCT_TITLE       10
#define CURRENTCOMPRESS_TITLE   11

#define TEXT_NOTADIRECTORY      15
#define TEXT_CANNOTEXAMINE      16
#define TEXT_CANNOTLOCK         17
#define TEXT_NOTAFILE           18
#define TEXT_ERRORREADINDEX     19
#define TEXT_NOMEMORY           20
#define TEXT_ERROROPENINDEX     21
#define TEXT_WRITEERROR         22
#define TEXT_CREATEERROR        23
#define TEXT_READERROR          24
#define TEXT_BYTESREADERROR     25
#define TEXT_ERRORPROCESSING    26
#define TEXT_ERRORINTERFACE     27
#define TEXT_ERRORLOCKINDEX     28
#define TEXT_ERRORLOCKDOCS      29
#define TEXT_ERRORCREATEINDEX   30
#define TEXT_INDEXOK            31
#define TEXT_DOCSNOTDEFINED     32
#define TEXT_AZMANDIR           33
#define TEXT_FORMATERROR        34

/* this macro lets us long-align structures on the stack */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((LONG)(a_##name+3) & ~3);


/****** Imported ************************************************/

extern ULONG              DOSBase;
extern struct ExecBase *  SysBase;


/****** Exported ***********************************************/

struct UIKBase *  UIKBase;

/****** Statics ************************************************/

struct UIKGlobal *  UIK;

static  struct  UIKObjWindow *  MainWindow;
static  struct  UIKObjWindow *  ProgressWindow;

static struct FileInfoBlock  FIB;
static ULONG  IndexDirLock;

static UBYTE *  PathForDoc;
static UBYTE *  PathForIndex;


/****** Textes *************************************************/

UBYTE *  LangEnglish[] =
    {
    "",
    "Creation of index",
    "AutoDocs found",
    "AutoDocs to install",
    "Insert all",
    "Remove all",
    "Compress",
    "Creating Index",
    "Install in progress ...",
    "Creating index for   : ",
    "Fonction in progress : ",
    "Compress of          : ",
    "Enter directory names where are Autodocs\nand where you want the index to be created",
    "_Autodocs",
    "_Index",
    "*** %ls is not a directory",
    "*** Cannot examine %ls",
    "*** Cannot take a lock on %ls",
    "%ls is not a file but a directory",
    "Error reading index %ls",
    "Not enough memory",
    "Error opening index %ls",
    "Write error on %ls",
    "Create error on %ls",
    "Read error on %ls",
    "Cannot read the %ld bytes of file %ls",
    "Cannot process %ls",
    "Error while adding user interface",
    "Cannot lock index directory",
    "Cannot lock autodocs directory",
    "Index creation interrupted",
    "Index created OK.",
    "Autodocs directory not defined",
    "AZMan directories",
    "This file is not formated as an 'Autodoc'",
     0
    };

 UBYTE *  LangOther[] =
    {
    "",
    "Création des index",
    "AutoDocs présentes",
    "AutoDocs à installer",
    "Tout ajouter",
    "Tout supprimer",
    "Compression",
    "Création index",
    "Installation en cours ...",
    "Création des index pour : ",
    "Fonction en traitement  : ",
    "Compression de          : ",
    "Entrez les noms de répertoires où sont les autodocs\net où les index doivent être créés",
    "_Autodocs",
    "_Index",
    "*** %ls n'est pas un répertoire",
    "*** Ne peux examiner %ls",
    "*** Ne peux prendre un Lock sur %ls",
    "%ls n'est pas un fichier mais un REPERTOIRE",
    "Erreur de lecture de l'index %ls",
    "Manque de mémoire",
    "Erreur d'ouverture de l'index %ls",
    "Erreur d'écriture sur %ls",
    "Erreur sur création de %ls",
    "Erreur durant la lecture de %ls",
    "Impossible de lire les %ld octets du fichier %ls",
    "Impossible de traiter %ls",
    "Erreur à l'ajout de l'interface utilisateur",
    "Erreur sur lock du répertoire des index",
    "Erreur sur lock du répertoire des autodocs",
    "Création d'index interrompue",
    "Index créés OK.",
    "Répertoire des autodocs non défini",
    "Répertoire AZMan",
    "Ce fichier n'est pas au format 'Autodoc'",
    0
    };

UWORD PatProp[] = { 0xAAAA, 0x5555 };


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

static void ShowMsg( ULONG fmt, UBYTE *p1, UBYTE *p2, UBYTE *p3 )
{
  struct Library *IntuitionBase;
  UBYTE buf[200];

    if (UIK) fmt = (ULONG) UIK_LangString( UIK, fmt );
    sprintf( buf, (STRPTR)fmt, p1, p2, p3 );
    if (IntuitionBase = OpenLibrary( "intuition.library", 0 ))
        {
        struct IntuiText msg = { -1, -1, JAM1, 10, 10, NULL, NULL, NULL },
                         ok  = { -1, -1, JAM1, 6, 3, NULL, "OK", NULL };
        msg.IText = buf;
        AutoRequest( NULL, &msg, 0, &ok, 0, 0, (__builtin_strlen( buf ) + 13) * 8, 50 );
        CloseLibrary( IntuitionBase );
        }
}

static ULONG LockDirectory( UBYTE *name )
{
  ULONG lock, ok=0;

    if (lock = Lock( name, SHARED_LOCK ))
        {
        if (Examine( lock, &FIB ))
            {
            if (FIB.fib_DirEntryType > 0) ok = 1;
            else ShowMsg( TEXT_NOTADIRECTORY, name, 0, 0 );
            }
        else ShowMsg( TEXT_CANNOTEXAMINE, name, 0, 0 );
        }
    else ShowMsg( TEXT_CANNOTLOCK, name, 0, 0 );

    if (!ok && lock) { UnLock( lock ); lock = 0; }
    return( lock );
}

static void CleanUp(void)
{
    if (UIK)     UIK_Remove( UIK );
    if (UIKBase) CloseLibrary( UIKBase );
    exit(0);
}

static void ProgressCleanup(void)
{
    UIK_Remove(ProgressWindow);
    ProgressWindow = 0;
}

static  void    ActualizeDocPresentList(struct UIKObjList *DocsList)
{
struct FileInfoBlock *fib;
ULONG lock;

    if (fib = (struct FileInfoBlock *) AllocMem( sizeof(struct FileInfoBlock), MEMF_PUBLIC|MEMF_CLEAR ))
        {
        if (lock = Lock( PathForDoc, SHARED_LOCK ))
            {
            if (Examine( lock, fib ))
                {
                if (fib->fib_DirEntryType > 0)
                    {
                    while (TRUE)
                        {
                        if (ExNext( lock, fib ) != FALSE)
                            {
                            if (fib->fib_DirEntryType < 0)
                                {
                                UBYTE *ptr = fib->fib_FileName;
                                ULONG len = StrLen( ptr );
                                if (len >= 4)
                                    if (! StrNCCmp( ptr+len-4, ".doc" ))
                                        if (! UIK_OLi_AddLine( DocsList , ptr, -1, 0 )) break;
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
        FreeMem( fib, sizeof(struct FileInfoBlock) );
        }
     UIK_OLi_ActualizeList( DocsList);
    return;
}

static LONG  SizeOfFile(UBYTE *FileName)
{
  ULONG Lck;
  D_S(struct FileInfoBlock,FileInfo);

    if (! ( Lck = Lock( FileName , ACCESS_READ ) ) )
        {
        ShowMsg( TEXT_CANNOTLOCK,FileName, 0, 0);
        return(0L);
        }
    if ( ! Examine(Lck,FileInfo) )
        {
        ShowMsg( TEXT_CANNOTEXAMINE,FileName, 0, 0 );
        UnLock(Lck);
        return(0L);
        }
    if ( FileInfo->fib_DirEntryType >= 0 )
        {
        ShowMsg( TEXT_NOTAFILE, FileName, 0, 0 );
        UnLock(Lck);
        return(0L);
        }
    UnLock(Lck);
    return(FileInfo->fib_Size);
}

static ULONG Find0A0C( UBYTE *buf, ULONG ind, ULONG max )
{
    while (ind < max)
        {
        while (buf[ind] != 0xC) { if (++ind >= max) return( max-1 ); }
        if (ind && buf[ind-1] == '\n') break; else ind++;
        }
    return( ind < max ? ind : max-1 );
}

static ULONG SearchNextEntry( UBYTE *buf, ULONG fileindex, REFERENCE *Ref, ULONG filesize )
{
  ULONG ind, offset=0;

    if (fileindex >= filesize) return(0);

    //  On est bien au début d'une rubrique
    //  Extraction des champs Offset Owner FnctName
    //
    //  OFFSET
    Ref->Offset = fileindex;

    //  OWNER
    for (ind=0; ind < SIZE_AMIGA_FILENAME-1 && buf[offset] != '/'; ind++)
        {
        if (fileindex + offset > filesize) return(0);
        Ref->Owner[ind] = buf[offset++];
        }
    Ref->Owner[ind] = 0;

    //  FNCTNAME
    offset++;
    for (ind=0; (ind < SIZE_AMIGA_FILENAME-1) && (buf[offset] != ' ') && (buf[offset] != '\t') && (buf[offset] != '\n') && (buf[offset] != '('); ind++)
        {
        if (fileindex + offset > filesize) return(0);
        Ref->FnctName[ind] = buf[offset++];
        }
    Ref->FnctName[ind] = 0;

    //  Recherche de la taille de la rubrique
    //
    offset = Find0A0C( buf, offset, filesize - fileindex );
    Ref->Size = offset;
    return( offset );
}

static ULONG valid_filename( UBYTE *ptr, UBYTE *filename )
{
    while (*ptr)
        {
        if (*ptr != '*')
            {
            filename[0] = UpperChar( *ptr );
            filename[1] = 0;
            return(1);
            }
        ptr++;
        }
    return(0);
}

static ULONG IndexFileExist( REFERENCE *Ref )
{
  UBYTE filename[2];
  ULONG indexlock, found=0;

    //filename[0] = UpperChar( Ref->FnctName[0] ); filename[1] = 0;
    if (valid_filename( Ref->FnctName, filename ))
        {
        if (indexlock = Lock( filename, ACCESS_READ )) { UnLock( indexlock ); found = 1; }
        }
    return( found );
}

static ULONG UpdateExistingIndexFile( REFERENCE *Ref )
{
  UBYTE filename[2], *buf;
  ULONG i, numindex, ok=0, indexfile, filesize, copylen;
  REFERENCE *ir;

    //filename[0] = UpperChar( Ref->FnctName[0] ); filename[1] = 0;
    if (valid_filename( Ref->FnctName, filename ))
        {
        if (indexfile = Open( filename, MODE_OLDFILE ))
            {
            filesize = Seek( indexfile, Seek( indexfile, 0, OFFSET_END ), OFFSET_BEGINNING );
            numindex = filesize / sizeof(REFERENCE);
            if (buf = (UBYTE*) AllocMem( filesize + sizeof(REFERENCE), MEMF_PUBLIC ))
                {
                if (Read( indexfile, buf, filesize ) == filesize)
                    {
                    LONG notequal=-10;
                    for (i=0, ir=(REFERENCE*)buf; i < numindex; i++, ir++)
                        {
                        if ((notequal = strcmp( ir->FnctName, Ref->FnctName )) >= 0) /* Si Place Alpha Atteinte */
                            {
                            if (notequal != 0) // on va insérer
                                break;
                            else{ // on va remplacer, attention aux noms en doubles comme --Overview--, etc...
                                if ((notequal = strcmp( ir->DocFileName, Ref->DocFileName )) == 0)
                                    break;
                                }
                            }
                        }
                    // si notequal == 0 on écrit par-dessus
                    if (notequal != 0)
                        {
                        if ((notequal > 0) && (copylen = ((ULONG)buf + filesize - (ULONG)ir) / sizeof(ULONG)))
                            {
                            ULONG *lpd = (ULONG*)(buf + filesize + sizeof(REFERENCE) - sizeof(ULONG));
                            ULONG *lps = (ULONG*)(((ULONG)lpd) - sizeof(REFERENCE));
                            for (i=0; i < copylen; i++) *lpd-- = *lps--;
                            }
                        numindex++;
                        }
                    *ir = *Ref;
                    Seek( indexfile, 0, OFFSET_BEGINNING );
                    if (Write( indexfile, buf, numindex * sizeof(REFERENCE) ) == numindex * sizeof(REFERENCE))
                        ok = 1;
                    else ShowMsg( TEXT_ERRORREADINDEX, filename, 0, 0 );
                    }
                else ShowMsg( TEXT_ERRORREADINDEX, filename, 0, 0 );
                FreeMem( buf, filesize + sizeof(REFERENCE) );
                }
            else ShowMsg( TEXT_NOMEMORY, 0, 0, 0 );
            Close( indexfile );
            }
        else ShowMsg( TEXT_ERROROPENINDEX, filename, 0, 0 );
        }
    return( ok );
}

static ULONG CreateIndexFile( REFERENCE *Ref )
{
  UBYTE filename[2];
  ULONG indexfile, ok=0;

    //filename[0] = UpperChar( Ref->FnctName[0] ); filename[1] = 0;
    if (valid_filename( Ref->FnctName, filename ))
        {
        if (indexfile = Open( filename, MODE_NEWFILE ))
            {
            if (Write( indexfile, Ref, sizeof(REFERENCE) ) == sizeof(REFERENCE)) ok = 1;
            else ShowMsg( TEXT_WRITEERROR, filename, 0, 0 );
            Close( indexfile );
            }
        else ShowMsg( TEXT_CREATEERROR, filename, 0, 0 );
        }
    return( ok );
}

static ULONG verif_header( UBYTE *buf, ULONG max )
{
  ULONG i, j, hlen;
  UBYTE *hstr;

    hstr = "TABLE OF CONTENTS\n";
    hlen = strlen( hstr );
    for (i=0; i < max; i++)
        {
        if (buf[i] == 0x0c)
            {
            if (i > hlen && buf[i-1] == 0x0a)
                {
                if (strncmp( buf, hstr, hlen ) == 0) return(1);
                }
            break;
            }
        }
    return(0);
}

static ULONG CreateIndexEntries( UBYTE *docname, LONG filesize , struct UIKObjStrGad *FnctString)
{
  UBYTE *readbuf;
  ULONG file, oldlock, readlen, fileindex, offset, ok=0;
  REFERENCE CurrentRef;

    if (readbuf = (UBYTE*) AllocMem( filesize, MEMF_PUBLIC|MEMF_CLEAR ))
        {
        if (file = Open( docname, MODE_OLDFILE ))
            {
            readlen = Read( file, readbuf, filesize );
            Close( file );
            if ((readlen == filesize) && filesize)
                {
                if (verif_header( readbuf, filesize ))
                    {
                    strcpy( CurrentRef.DocFileName, docname );
                    fileindex = Find0A0C( readbuf, 0, filesize ) + 1;
                    ok = 1;  // ne pas mettre après car en cas d'autoc vide il faut continuer
                    if (fileindex < filesize)
                        {
                        oldlock = CurrentDir( IndexDirLock );
                        while (offset = SearchNextEntry( &readbuf[fileindex], fileindex, &CurrentRef, filesize ))
                            {
                            if (SIGBREAKF_CTRL_C & SetSignal(0,0))
                                {
                                ok = 0;
                                break;
                                }
                            UIK_ProcessPendingEvents( ProgressWindow, 0L);
                            if (ProgressWindow->Obj.Status != UIKS_STARTED && ProgressWindow->Obj.Status != UIKS_ICONIFIED) { ok = 0; break; }

                            UIK_OSt_Set( FnctString, CurrentRef.FnctName, (ULONG)StrLen(CurrentRef.FnctName));
                            if (IndexFileExist( &CurrentRef ))
                                {
                                if (! UpdateExistingIndexFile( &CurrentRef ))
                                    {
                                    ok = 0;
                                    break;
                                    }
                                }
                            else{
                                if (! CreateIndexFile( &CurrentRef ))
                                    {
                                    ok = 0;
                                    break;
                                    }
                                }
                            fileindex += offset + 1;
                            }
                        CurrentDir( oldlock );
                        }
                    }
                else ShowMsg( TEXT_FORMATERROR, docname, 0, 0 );
                }
            else ShowMsg( TEXT_READERROR, docname, 0, 0 );
            }
        else ShowMsg( TEXT_BYTESREADERROR, (APTR)filesize, docname, 0 );
        FreeMem( readbuf, filesize );
        }
    else ShowMsg( TEXT_NOMEMORY, 0, 0, 0 );
    return( ok );
}



static  void    FonctionMakeIndex(struct UIKObj *Button)
{
//  struct  UIKObjStrGad        *CurrentCompressString;
//  struct  UIKObjProgress      *CompressProgress;
//  struct  UIKObj              *CompressBoxProgress;
//  struct  UIKObjCheckToggle   *CrunchIndicator;
struct  UIKObjStrGad        *CurrentDocString;
struct  UIKObjStrGad        *CurrentFnctString;
struct  UIKObjProgress      *MakeIndexProgress;
struct  UIKObj              *MakeIndexBoxProgress;
struct  UIKObjList          *ListToPerform;
LONG                        NbrOfLines;
LONG                        CurrentLine;
UBYTE                       CurrentDoc[32];
ULONG                       lock=0;
ULONG                       oldlock=0;
int                         offs, CurrentPosition=0;
ULONG msg=0;

    if (ProgressWindow) return;

    IndexDirLock = 0;

    ListToPerform=(struct UIKObjList *)Button->UserULong1;
//      CrunchIndicator=(struct UIKObjCheckToggle *)Button->UserULong2;

    NbrOfLines = UIK_OLi_TotLines(ListToPerform);
    if ( NbrOfLines <= 0 )
        return;
    if (! (ProgressWindow = (struct UIKObjWindow *) UIK_AddObjectTags ( "UIKObj_Window", MainWindow,
            UIKTAG_OBJ_LeftTop,       SETL(40,30) ,
//            UIKTAG_OBJ_WidthHeight,   SETL(480,110) ,
            UIKTAG_OBJ_WidthHeight,   SETL(480,70),
            UIKTAG_OBJ_Title,         PROGRESSWINDOW_TITLE ,
            UIKTAG_OBJ_AltTitle,      PROGRESSWINDOW_TITLE ,
            UIKTAG_OBJ_ActInactPens,  UIKCOLS(UIKCOL_LIGHT,UIKCOL_GREY,UIKCOL_WHITE,UIKCOL_BLACK) ,
//            UIKTAG_OBJWindow_MinimumWidthHeight, SETL(480,110) ,
//            UIKTAG_OBJWindow_MaximumWidthHeight, SETL(480,110) ,
            UIKTAG_OBJWindow_MinimumWidthHeight, SETL(480,70) ,
            UIKTAG_OBJWindow_MaximumWidthHeight, SETL(480,70) ,
            /*UIKTAG_OBJWindowFl_CloseStop, FALSE,*/
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    offs = ((ProgressWindow->Obj.AutoRatio == 1) && (UIKBase->UIKPrefs->PrefsFlags & UIKF_SPLOOK)) ? 1 : 0;

    if (! (CurrentDocString = (struct UIKObjStrGad *) UIK_AddObjectTags( "UIKObj_String", ProgressWindow ,
            UIKTAG_OBJ_LeftTop,             SETL(250,10),
            UIKTAG_OBJ_WidthHeight,         SETL(200,0),
            UIKTAG_OBJ_Title,               CURRENTDOC_TITLE,
            UIKTAG_OBJ_AltTitle,            CURRENTDOC_TITLE,
            UIKTAG_OBJ_TitleFl_Left ,       TRUE,
            UIKTAG_OBJStr_TextPointer,      (ULONG)"",
            UIKTAG_OBJStr_TextPens,         UIKCOLS(UIKCOL_BLACK,UIKCOL_BLACK,UIKCOL_GREY,UIKCOL_GREY),
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJ_BoxFl_NotDrawn,      TRUE,
            UIKTAG_OBJ_BoxFl_NoAct,         TRUE,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    if (! (CurrentFnctString = (struct UIKObjStrGad *) UIK_AddObjectTags( "UIKObj_String", ProgressWindow ,
            UIKTAG_OBJ_LeftTop,             SETL(250,25),
            UIKTAG_OBJ_WidthHeight,         SETL(200,0),
            UIKTAG_OBJ_Title,               CURRENTFNCT_TITLE,
            UIKTAG_OBJ_AltTitle,            CURRENTFNCT_TITLE,
            UIKTAG_OBJ_TitleFl_Left ,       TRUE,
            UIKTAG_OBJStr_TextPointer,      (ULONG)"",
            UIKTAG_OBJStr_TextPens,         UIKCOLS(UIKCOL_BLACK,UIKCOL_BLACK,UIKCOL_GREY,UIKCOL_GREY),
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJ_BoxFl_NotDrawn,      TRUE,
            UIKTAG_OBJ_BoxFl_NoAct,         TRUE,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
//
//    if (! (CurrentCompressString = (struct UIKObjStrGad *) UIK_AddObjectTags( "UIKObj_String", ProgressWindow ,
//            UIKTAG_OBJ_LeftTop,             SETL(250,70),
//            UIKTAG_OBJ_WidthHeight,         SETL(200,0),
//            UIKTAG_OBJ_Title,               CURRENTCOMPRESS_TITLE,
//            UIKTAG_OBJ_AltTitle,            CURRENTCOMPRESS_TITLE,
//            UIKTAG_OBJ_TitleFl_Left ,       TRUE,
//            UIKTAG_OBJStr_TextPointer,      (ULONG)"",
//            UIKTAG_OBJStr_TextPens,         UIKCOLS(UIKCOL_BLACK,UIKCOL_BLACK,UIKCOL_GREY,UIKCOL_GREY),
//            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
//            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
//            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
//            UIKTAG_OBJ_BoxFl_NotDrawn,        TRUE,
//            UIKTAG_OBJ_BoxFl_NoAct,         TRUE,
//            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
//
    if ( NbrOfLines > 1 )
        {
        if (! (MakeIndexBoxProgress = UIK_AddObjectTags("UIKObj_Box", ProgressWindow ,
                UIKTAG_OBJ_LeftTop,         SETL(38,43),
                UIKTAG_OBJ_WidthHeight,     SETL(404+offs,19),
                UIKTAG_OBJ_BoxFl_Over,      TRUE,
                UIKTAG_OBJ_VisualEraseColor,UIKCOL_GREY,
                TAG_END))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

        if (! (MakeIndexProgress = (struct UIKObjProgress *) UIK_AddObjectTags ("UIKObj_Progress", ProgressWindow ,
                UIKTAG_OBJ_LeftTop,            SETL(40,45) ,
                UIKTAG_OBJ_WidthHeight,        SETL(400,13+offs) ,
                UIKTAG_OBJ_VisualEraseColor,   UIKCOL_GREY,
                UIKTAG_OBJProgress_Total,      10,
                UIKTAG_OBJProgress_Pos,        0 ,
                UIKTAG_OBJProgress_ToDoPen,    UIKCOL_GREY ,
                UIKTAG_OBJProgress_DonePen,    UIKCOL_LIGHT ,
                UIKTAG_OBJProgress_RenderMode, PROG_RenderFromLeft ,
                TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
        }
//
//    if (! (CompressBoxProgress = UIK_AddObjectTags("UIKObj_Box", ProgressWindow ,
//            UIKTAG_OBJ_LeftTop,         SETL(38,88),
//            UIKTAG_OBJ_WidthHeight,     SETL(404,19),
//            UIKTAG_OBJ_BoxFl_Over,      TRUE,
//            UIKTAG_OBJ_VisualEraseColor,UIKCOL_GREY,
//            TAG_END))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
//
//    if (! (CompressProgress = (struct UIKObjProgress *) UIK_AddObjectTags ("UIKObj_Progress", ProgressWindow ,
//            UIKTAG_OBJ_LeftTop,            SETL(40,90) ,
//            UIKTAG_OBJ_WidthHeight,        SETL(400,15) ,
//            UIKTAG_OBJ_VisualEraseColor,   UIKCOL_GREY,
//            UIKTAG_OBJProgress_Total,      10,
//            UIKTAG_OBJProgress_Pos,        0 ,
//            UIKTAG_OBJProgress_ToDoPen,    UIKCOL_GREY ,
//            UIKTAG_OBJProgress_DonePen,    UIKCOL_LIGHT ,
//            UIKTAG_OBJProgress_RenderMode, PROG_RenderFromLeft ,
//            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
//
    UIK_Start(ProgressWindow);

    if (! (IndexDirLock = LockDirectory( PathForIndex ))) { msg = TEXT_ERRORLOCKINDEX; goto END_ERROR; }
    if (! (lock = LockDirectory( PathForDoc ))) { msg = TEXT_ERRORLOCKDOCS; goto END_ERROR; }
    oldlock = CurrentDir( lock );

    //
    //  On boucle sur les lignes de la liste
    //  et pour chaque ligne on cree les index.
    //
    if ( NbrOfLines > 1 ) MakeIndexProgress->Total = (ULONG)NbrOfLines;

    for (CurrentLine=NbrOfLines-1 ; CurrentLine >= 0 ; CurrentLine--)
        {
        UIK_ProcessPendingEvents( ProgressWindow, 0L);
        if (ProgressWindow->Obj.Status != UIKS_STARTED && ProgressWindow->Obj.Status != UIKS_ICONIFIED) { msg = TEXT_ERRORCREATEINDEX; goto END_ERROR; }

        StcCpy(CurrentDoc,UIK_OLi_LineBuffer(ListToPerform,CurrentLine),32);
        UIK_OSt_Set( CurrentDocString, CurrentDoc, (ULONG)StrLen(CurrentDoc));

        if (! CreateIndexEntries( CurrentDoc , SizeOfFile(CurrentDoc), CurrentFnctString))
            { ShowMsg( TEXT_ERRORPROCESSING, CurrentDoc, 0, 0); goto END_ERROR; }

        if ( NbrOfLines > 1 )
            {
            CurrentPosition++;
            UIK_CallObjectFunc( MakeIndexProgress, UIKFUNC_Progress_ChangeProgress, NbrOfLines, CurrentPosition);
            }
        }
    msg = TEXT_INDEXOK;

  END_ERROR:
    if (oldlock) CurrentDir( oldlock );
    if (lock) UnLock( lock );
    if (IndexDirLock) UnLock( IndexDirLock );
    if (msg) ShowMsg( msg, 0, 0, 0 );
    ProgressCleanup();
}

static void ClearList(struct UIKObjList *ListToClear)
{
LONG    NbrOfLines;
LONG    CurrentLine;

    NbrOfLines = UIK_OLi_TotLines(ListToClear);
    for(CurrentLine=NbrOfLines-1 ; CurrentLine >= 0 ; CurrentLine--)
        UIK_OLi_DelLine(ListToClear,CurrentLine);
    UIK_OLi_ActualizeList(ListToClear);
    return;
}


static void AddEntry(struct UIKObjList *PresentList,struct UIKObjList *PerformList)
{
UBYTE DocSelect[32];
WORD sel;

    UIK_LockObjVars(PresentList);
    sel=PresentList->Sel;
    if ( sel != - 1 )
        {
        StcCpy(DocSelect,UIK_OLi_LineBuffer(PresentList,sel),32);
        UIK_OLi_AddLine( PerformList, DocSelect , -1, 0 );
        UIK_OLi_DeleteLine(PresentList,sel);
        }
    UIK_UnlockObjVars(PresentList);
    return;
}

static void SubEntry(struct UIKObjList *PerformList,struct UIKObjList *PresentList)
{
UBYTE DocSelect[32];
WORD sel;

    UIK_LockObjVars(PerformList);
    sel=PerformList->Sel;
    if ( sel != -1 )
        {
        StcCpy(DocSelect,UIK_OLi_LineBuffer(PerformList,sel),32);
        UIK_OLi_AddLine( PresentList, DocSelect, -1, 0 );
        UIK_OLi_DeleteLine(PerformList,sel);
        }
    UIK_UnlockObjVars(PerformList);
    return;
}

static void ToutAjouter(struct UIKObj *Gad)
{
struct UIKObjList *PresentList;
struct UIKObjList *PerformList;
LONG    NbrOfLines;
LONG    CurrentLine;
UBYTE   DocSelect[32];

    PresentList=(struct UIKObjList *)Gad->UserULong1;
    PerformList=(struct UIKObjList *)Gad->UserULong2;
    NbrOfLines = UIK_OLi_TotLines(PresentList);
    for(CurrentLine=NbrOfLines-1 ; CurrentLine >= 0 ; CurrentLine--)
        {
        StcCpy(DocSelect,UIK_OLi_LineBuffer(PresentList,CurrentLine),32);
        UIK_OLi_DeleteLine(PresentList,CurrentLine);
        UIK_OLi_AddLine( PerformList , DocSelect, -1, 0 );
        }
    UIK_OLi_ActualizeList(PerformList);
//  ClearList(PresentList);
    return;
}

static void ToutSupprimer(struct UIKObj *Gad)
{
struct UIKObjList *PresentList;
struct UIKObjList *PerformList;
LONG    NbrOfLines;
LONG    CurrentLine;
UBYTE   DocSelect[32];


    PresentList=(struct UIKObjList *)Gad->UserULong1;
    PerformList=(struct UIKObjList *)Gad->UserULong2;
    NbrOfLines = UIK_OLi_TotLines(PerformList);
    for(CurrentLine=NbrOfLines-1 ; CurrentLine >= 0 ; CurrentLine--)
        {
        StcCpy(DocSelect,UIK_OLi_LineBuffer(PerformList,CurrentLine),32);
        UIK_OLi_DeleteLine(PerformList,CurrentLine);
        UIK_OLi_AddLine( PresentList, DocSelect, -1, 0 );
        }
    UIK_OLi_ActualizeList(PresentList);
//  ClearList(PerformList);
    return;
}

static ULONG get_myenv()
{
    if (PathForDoc = (UBYTE*) func_EnvGet( "AZMan" ))
        {
        UBYTE *p = PathForDoc;
        while (*p && *p != 0x0A) p++;
        if (*p)
            {
            *p++ = 0;
            if (*p)
                {
                PathForIndex = p;
                while (*p && *p != 0x0A) p++;
                if (*p == 0x0A) *p = 0;
                }
            }
        if (PathForIndex == 0)
            {
            func_EnvFree( PathForDoc );
            PathForDoc = 0;
            }
        }
    return( (ULONG)PathForDoc );
}

static void sm_okfunc( int nil, struct UIKObjReqString *rs )
{
  ULONG len1, len2, ok;
  UBYTE *ptr1, *ptr2, *mem;

    ptr1 = UIK_OSt_Get( rs->Str, &len1 );
    ok = lo_GetFIB( ptr1, &FIB );
    if (!len1 || !ok || FIB.fib_DirEntryType < 0) { ShowMsg( TEXT_NOTADIRECTORY, ptr1, 0, 0 ); return; }

    ptr2 = UIK_OSt_Get( rs->Str2, &len2 );
    ok = lo_GetFIB( ptr2, &FIB );
    if (!len2 || !ok || FIB.fib_DirEntryType < 0) { ShowMsg( TEXT_NOTADIRECTORY, ptr2, 0, 0 ); return; }

    if (mem = UIK_MemSpAlloc( len1+len2+2, MEMF_PUBLIC ))
        {
        StrCpy( mem, ptr1 );
        mem[len1] = '\n';
        StrCpy( mem+len1+1, ptr2 );
        mem[len1+1+len2] = '\n';
        func_EnvSet( "AZMan", mem, len1+len2+2 );
        UIK_MemSpFree( mem );
        }
    rs->Obj.UserULong1 = 1;
}

static void sm_cancelfunc( int nil, struct UIKObjReqString *rs )
{
    rs->Obj.UserULong1 = 0;
}

static struct TagItem __far smartreq_tags1[] =
    { { UIKTAG_OBJ_Title, 13 }, { UIKTAG_OBJ_TitleUnderscore, '_' }, { TAG_END } };
static struct TagItem __far smartreq_tags2[] =
    { { UIKTAG_OBJ_Title, 14 }, { UIKTAG_OBJ_TitleUnderscore, '_' }, { TAG_END } };

static struct TagItem __far smartreq_tags[] = {
    { UIKTAG_OBJ_Title,                 TEXT_AZMANDIR },
    { UIKTAG_ReqString_TextPtr,         0 },
    { UIKTAG_ReqString_OKFunc,          sm_okfunc },
    { UIKTAG_ReqString_CancelFunc,      sm_cancelfunc },
    { UIKTAG_ReqString_StringPtr,       0 },
    { UIKTAG_ReqString_StringPtr2,      0 },
    { UIKTAG_ReqString_StringTagList,   (ULONG)smartreq_tags1 },
    { UIKTAG_ReqString_StringTagList2,  (ULONG)smartreq_tags2 },
    { UIKTAG_ReqStringFl_Window,        TRUE },
    { UIKTAG_ReqStringFl_StringQuiet,   TRUE },
    { TAG_END } };

void main(int argc)
{
struct  UIKObjList          *DocsPresentList;
struct  UIKObjPropGad       *DocsPresentAscenceur;
struct  UIKObjGad           *DocsPresentListUp;
struct  UIKObjGad           *DocsPresentListDown;
struct  UIKObjList          *DocsToPerformList;
struct  UIKObjPropGad       *DocsToPerformAscenceur;
struct  UIKObjGad           *DocsToPerformListUp;
struct  UIKObjGad           *DocsToPerformListDown;
struct  UIKObjBoolean       *Ajouter;
struct  UIKObjBoolean       *Supprimer;
//  struct  UIKObjCheckToggle   *Compression;
struct  UIKObjBoolean       *MakeIndex;
ULONG msg=0;
struct UIKObjReqString *rs;
struct Library *IntuitionBase;


    if (! (UIKBase = (struct UIKBase *) OpenLibrary( UIKNAME, 0 ))) { msg = (ULONG)"Cannot Open uik.library"; goto END_ERROR; }
    if (! (UIK = (struct UIKGlobal *) UIK_AddObjectTags( "UIK", 0,
             UIKTAG_GEN_LangEnglish,    (ULONG) LangEnglish,
             UIKTAG_GEN_LangOther,      (ULONG) LangOther,
             TAG_END ))) { msg = (ULONG)"Can not Add UIK"; goto END_ERROR; }
    UIK_Start( UIK );

    get_myenv();
    smartreq_tags[1].ti_Data = (ULONG) UIK_LangString( UIK, 12 );
    smartreq_tags[4].ti_Data = (ULONG) (PathForDoc ? PathForDoc : "");
    smartreq_tags[5].ti_Data = (ULONG) (PathForIndex ? PathForIndex : "");
    if (rs = (struct UIKObjReqString *) UIK_AddObject( "UIKObj_ReqString", UIK, smartreq_tags ))
        {
        if (UIK_Start( rs ))
            {
            if (IntuitionBase = OpenLibrary( "intuition.library", 0 ))
                {
                ScreenToFront( rs->Req->w->WScreen );
                CloseLibrary( IntuitionBase );
                }
            rs->Obj.UserULong1 = -1;
            while (rs->Obj.UserULong1 == -1)
                {
                ULONG events;
                events = UIK_WaitFirstEvents( UIK, SIGBREAKF_CTRL_C );
                if (events & SIGBREAKF_CTRL_C)
                    {
                    Signal( FindTask(0), SIGBREAKF_CTRL_C );
                    rs->Obj.UserULong1 = 0;
                    }
                }
            }
        UIK_Remove( rs );
        get_myenv();
        }
    if (PathForDoc == 0) { msg = TEXT_DOCSNOTDEFINED; goto END_ERROR; }


    if (! (MainWindow = (struct UIKObjWindow *) UIK_AddObjectTags ( "UIKObj_Window", UIK,
            UIKTAG_OBJ_LeftTop,       SETL(20,10) ,
            UIKTAG_OBJ_WidthHeight,   SETL(529,227) ,
            UIKTAG_OBJ_Title,         MAINWINDOW_TITLE ,
            UIKTAG_OBJ_AltTitle,      MAINWINDOW_TITLE ,
            UIKTAG_OBJ_ActInactPens,  UIKCOLS(UIKCOL_LIGHT,UIKCOL_GREY,UIKCOL_WHITE,UIKCOL_BLACK) ,
            UIKTAG_OBJWindowFl_With_Size, FALSE,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }


    if (! (DocsPresentList  = (struct UIKObjList *)UIK_AddObjectTags( "UIKObj_List", MainWindow ,
            UIKTAG_OBJ_LeftTop,             SETL(17,28),
            UIKTAG_OBJ_WidthHeight,         SETL(150,124),
            UIKTAG_OBJ_Title,               DOCSPRESENT_TITLE,
            UIKTAG_OBJ_TitleFl_Left,        TRUE,
            UIKTAG_OBJ_TitleFl_Above,       TRUE,
            UIKTAG_OBJListFl_ClickSel,      TRUE,
            UIKTAG_OBJListFl_SelDispl,      TRUE,
            UIKTAG_OBJListFl_Sorted,        TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }


    if (! (DocsPresentAscenceur = ( struct UIKObj_PropGad *)UIK_AddObjectTags( "UIKObj_PropLift", MainWindow,
            UIKTAG_OBJ_LeftTop,             SETL(168,28),
            UIKTAG_OBJ_WidthHeight,         SETL(20,124),
            UIKTAG_OBJ_BoxFl_Over,          FALSE,
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJProp_Vertical,        TRUE,
            UIKTAG_OBJ_Pattern,             (ULONG)PatProp,
            UIKTAG_OBJ_PatternPower,        1,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }


    if (! (DocsPresentListUp = (struct UIKObjGad *)UIK_AddObjectTags( "UIKObj_Up", MainWindow ,
            UIKTAG_OBJ_LeftTop,             SETL(17,154),
            UIKTAG_OBJ_WidthHeight,         SETL(74,11),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }


    if (! (DocsPresentListDown = (struct UIKObjGad *)UIK_AddObjectTags( "UIKObj_Down", MainWindow ,
            UIKTAG_OBJ_LeftTop,             SETL(91,154),
            UIKTAG_OBJ_WidthHeight,         SETL(74,11),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }


    if (! (DocsToPerformList  = (struct UIKObjList *)UIK_AddObjectTags( "UIKObj_List", MainWindow ,
            UIKTAG_OBJ_LeftTop,             SETL(348,28),
            UIKTAG_OBJ_WidthHeight,         SETL(150,124),
            UIKTAG_OBJ_Title,               DOCSTOPERFORM_TITLE,
            UIKTAG_OBJ_TitleFl_Left,        TRUE,
            UIKTAG_OBJ_TitleFl_Above,       TRUE,
            UIKTAG_OBJListFl_ClickSel,      TRUE,
            UIKTAG_OBJListFl_SelDispl,      TRUE,
            UIKTAG_OBJListFl_Sorted,        TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }


    if (! (DocsToPerformAscenceur  = ( struct UIKObj_PropGad *)UIK_AddObjectTags( "UIKObj_PropLift", MainWindow,
            UIKTAG_OBJ_LeftTop,             SETL(499,28),
            UIKTAG_OBJ_WidthHeight,         SETL(20,124),
            UIKTAG_OBJ_BoxFl_Over,          FALSE,
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJProp_Vertical,        TRUE,
            UIKTAG_OBJ_Pattern,             (ULONG)PatProp,
            UIKTAG_OBJ_PatternPower,        1,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }


    if (! (DocsToPerformListUp  = (struct UIKObjGad *)UIK_AddObjectTags( "UIKObj_Up", MainWindow ,
            UIKTAG_OBJ_LeftTop,             SETL(348,154),
            UIKTAG_OBJ_WidthHeight,         SETL(74,11),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }


    if (! (DocsToPerformListDown  = (struct UIKObjGad *)UIK_AddObjectTags( "UIKObj_Down", MainWindow ,
            UIKTAG_OBJ_LeftTop,             SETL(422,154),
            UIKTAG_OBJ_WidthHeight,         SETL(74,11),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }


    if (! (Ajouter  = (struct UIKObjBoolean *)UIK_AddObjectTags( "UIKObj_Boolean", MainWindow ,
            UIKTAG_OBJ_LeftTop,         SETL(200,42),
            UIKTAG_OBJ_WidthHeight,     SETL(130,14),
            UIKTAG_OBJ_Title,           AJOUTER_TITLE,
            UIKTAG_OBJ_BoxFl_Filled,    TRUE,
            UIKTAG_OBJ_ShortCutKey,     B2L('A',0,0,0),
            UIKTAG_OBJ_UserValue1,      (ULONG)DocsPresentList,
            UIKTAG_OBJ_UserValue2,      (ULONG)DocsToPerformList,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    if (! (Supprimer  = (struct UIKObjBoolean *)UIK_AddObjectTags( "UIKObj_Boolean", MainWindow ,
            UIKTAG_OBJ_LeftTop,         SETL(200,81),
            UIKTAG_OBJ_WidthHeight,     SETL(130,14),
            UIKTAG_OBJ_Title,           SUPPRIMER_TITLE,
            UIKTAG_OBJ_BoxFl_Filled,    TRUE,
            UIKTAG_OBJ_ShortCutKey,     B2L('S',0,0,0),
            UIKTAG_OBJ_UserValue1,      (ULONG)DocsPresentList,
            UIKTAG_OBJ_UserValue2,      (ULONG)DocsToPerformList,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

//    if (! ( Compression  = ( struct UIKObjCheckToggle * )UIK_AddObjectTags( "UIKObj_CheckToggle", MainWindow,
//            UIKTAG_OBJ_LeftTop,         SETL(219,120),
//            UIKTAG_OBJ_WidthHeight,     SETL(30,10),
//            UIKTAG_OBJ_Title,           COMPRESSION_TITLE,
//            UIKTAG_OBJ_TitleFl_Right,   TRUE,
//            UIKTAG_OBJ_ActInactPens,    UIKCOLS(UIKCOL_GREY,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_BLACK),
//            UIKTAG_OBJ_ShortCutKey,     B2L('C',0,0,0),
//            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
//

    if (! (MakeIndex  = (struct UIKObjBoolean *)UIK_AddObjectTags( "UIKObj_Boolean", MainWindow ,
            UIKTAG_OBJ_LeftTop,         SETL(192,178),
            UIKTAG_OBJ_WidthHeight,     SETL(158,30),
            UIKTAG_OBJ_Title,           MAKEINDEX_TITLE,
            UIKTAG_OBJ_BoxFl_Filled,    TRUE,
            UIKTAG_OBJ_ShortCutKey,     B2L('I',0,0,0),
            UIKTAG_OBJ_UserValue1,      (ULONG)DocsToPerformList,
//            UIKTAG_OBJ_UserValue2,        (ULONG)Compression,
            TAG_END ))) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    /* On lie les ascenceurs a leur liste    */
    if (! UIK_Link( DocsPresentAscenceur , DocsPresentList, 0, MOUSEMOVE|GADGETDOWN, 0 )) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    if (! UIK_Link( DocsToPerformAscenceur , DocsToPerformList, 0, MOUSEMOVE|GADGETDOWN, 0 )) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    /* On lie les boutons Up et Down à leur ascenceur */
    if (! UIK_Link( DocsPresentListUp, DocsPresentAscenceur , 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
    if (! UIK_Link( DocsPresentListDown, DocsPresentAscenceur , 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    if (! UIK_Link( DocsToPerformListUp, DocsToPerformAscenceur , 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
    if (! UIK_Link( DocsToPerformListDown, DocsToPerformAscenceur , 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    if (! UIK_Start( MainWindow )) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    ActualizeDocPresentList(DocsPresentList);

    if ( ! UIK_AddVector( MakeIndex , (void*)FonctionMakeIndex, GADGETUP , (ULONG) NULL ) ) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
    if ( ! UIK_AddVector( DocsPresentList, (void *)AddEntry,GADGETDOWN,(ULONG)DocsToPerformList)) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
    if ( ! UIK_AddVector( DocsToPerformList,(void *)SubEntry,GADGETDOWN,(ULONG)DocsPresentList)) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
    if ( ! UIK_AddVector( Ajouter , (void *)ToutAjouter, GADGETUP , (ULONG) NULL )) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }
    if ( ! UIK_AddVector( Supprimer , (void *)ToutSupprimer,GADGETUP,(ULONG)NULL )) { msg = TEXT_ERRORINTERFACE; goto END_ERROR; }

    UIK_Do( UIK, SIGBREAKF_CTRL_C);

  END_ERROR:
    if (msg) ShowMsg( msg, 0, 0, 0 );
    CleanUp();
}
