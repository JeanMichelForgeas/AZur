/****************************************************************
 *
 *      Project:    AZMan
 *      Function:   Documentation en ligne pour AZur
 *
 *      Created:    15/12/93    Patrice TROGNON
 *      Modified:   09/11/94    Jean-Michel Forgeas
 *
 *                  Copyright (C) 1994,
 *          Patrice Trognon & Jean-Michel Forgeas
 *                 Tous Droits Réservés
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <exec/execbase.h>
#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

/****** UIK *******/
#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikglobal.h"
#include "uiki:uikcolors.h"
#include "uiki:uikmacros.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objscreen.h"
#include "uiki:objwindow.h"
#include "uiki:objlist.h"
#include "uiki:objproplift.h"
#include "uiki:objstring.h"
#include "uiki:objreqstring.h"
#include "uiki:objwinhelp.h"
#include "uiki:objhelp.h"

/****** Azur ******/
#include "Azur:DevelKit/Includes/funcs.h"
#include "Azur:DevelKit/Includes/azur.h"
#include "Azur:DevelKit/Includes/obj.h"
#include "Azur:DevelKit/Includes/objfile.h"

#include "AZMan.h"

#define MAINWINDOW_TITLE    1
#define DOCSBUTTON_TITLE    2
#define CMDSLIST_TITLE      3
#define FINBUTTON_TITLE     4
#define SELECTDOC_TITLE     5
#define DOCSLIST_TITLE      6


/****** Imported ************************************************/

void func_EnvFree( APTR pref );
APTR func_EnvGet( STRPTR prefname );
ULONG func_EnvSet( STRPTR prefname, APTR pref, ULONG prefsize );


/****** Exported ************************************************/

struct DOSBase  * __far DOSBase;
struct ExecBase * __far SysBase;
struct UIKBase  * __far UIKBase;
struct IntuitionBase  * __far IntuitionBase;


/****** Statics *************************************************/

static void AZFunc_Do( struct CmdObj *co );
static void AZFunc_Load( struct CmdObj *co );
static void AZFunc_Unload( struct CmdObj *co );

static UWORD __far PatProp[] = { 0xAAAA, 0x5555 };


/****** Germe de l'objet-catalogue UIK **************************/

/* Le fait d'ajouter un objet privé UIK va permettre trois choses :
 *
 * - de bénéficier de la facilité UIK des multiples langages ainsi
 *   que d'un catalogue automatiquement,
 * - de faire du code réentrant en toute simplicité : les variables
 *   susceptibles d'être modifiées sont placées dans la structure de
 *   l'objet privé, et sont accessibles partout à partir du moment
 *   où on passe le pointeur sur l'objet privé à toutes les fonctions.
 * - de supprimer en une seule instruction tous les objets qu'on a
 *   pu ajouter à ceux de AZur, car on leur donne à tous l'objet
 *   privé comme parent plus ou moins éloigné.
 */

static UBYTE * __far LangEnglish[] = {
    "",
    "Choice of Autodoc",    // MAINWINDOW
    "_Autodocs",            // DOCSBUTTON
    "Commands",             // CMDSLIST
    "End",                  // FINBUTTON
    "Selection",            // SELECTDOC
    "Autodocs",             // DOCSLIST
    0
    };

static UBYTE * __far LangOther[] = {
    "",
    "Choix d'une Autodoc",  // MAINWINDOW
    "_Autodocs",            // DOCSBUTTON
    "Commandes",            // CMDSLIST
    "Fin",                  // FINBUTTON
    "Sélection",            // SELECTDOC
    "Autodocs",             // DOCSLIST
    0
    };

static struct TagItem __far GermTL[] = {
    UIKTAG_GEN_LangEnglish, (ULONG)LangEnglish,
    UIKTAG_GEN_LangOther,   (ULONG)LangOther,
    UIKTAG_GEN_Catalog,     (ULONG)"AZur:Catalogs/AZMan.catalog",
    TAG_DONE
    };

struct ObjToto
    {
    struct UIKObj Obj;

    //---- fenêtre générale
    struct UIKObjWindow *MainWindow;
    struct UIKObj       *MainButton;
    struct UIKObjStrGad *MainString;
    struct UIKObjList   *MainList;
    struct UIKObj       *MainProp;
    struct UIKObj       *MainUp;
    struct UIKObj       *MainDown;
    ULONG               MainSelSecs;
    ULONG               MainSelMicros;
    WORD                MainOldSel;
    UBYTE               CurrentDocName[34];

    //---- sous-fenêtre des docs
    struct UIKObjWindow *SubWindow;
    struct UIKObjList   *SubList;
    struct UIKObj       *SubProp;
    struct UIKObj       *SubUp;
    struct UIKObj       *SubDown;
    ULONG               SubSelSecs;
    ULONG               SubSelMicros;
    WORD                SubOldSel;
    };

static struct UIKObjGerm __far Germ = {
    "",     /* pas de nom */
    NULL,   /* int   (*AddVector)(); */
    NULL,   /* int   (*StartVector)(); */
    NULL,   /* void  (*StopVector)(); */
    NULL,   /* void  (*RemoveVector)(); */
    NULL,   /* void  (*RefreshVector)(); */
    NULL,   /* void  (*ResizeVector)(); */
    NULL,   /* void  (*ExceptActVector)(); */
    NULL,   /* void  (*ExceptInaVector)(); */
    NULL,   /* void  (*ExceptMovVector)(); */
    NULL,   /* void  (*ExceptKeyVector)(); */
    NULL,   /* int   (*StartAfterVector)(); */
    NULL,   /* int   (*SetGetValues)(); */
    NULL,   /* ULONG *ExtentedVectors; */
    GermTL, /* struct TagItem *TagList; */
    0,      /* UWORD MinWidth; */
    0,      /* UWORD MinHeight; */
    0,      /* ULONG DefaultWidthHeight; */
    OBJECTID_USER | 0,  /* UWORD ObjId; */
    sizeof(struct ObjToto), /* UWORD ObjSize; */
    0,      /* ULONG DefaultFlags */
    };

static struct UIKObjRegGerm __far ObjTotoRegGerm = { &Germ, 0, 0, 0 } ;

static UBYTE * __far PathForDoc;
static UBYTE * __far PathForIndex;


/****** Germe de l'objet de commande AZur ***********************/

/* attention à bien donner ici le même nom que son nom de fichier  */
static UBYTE * __far CmdNames[] = { "AZMan", 0 };

static UBYTE __far CmdTmplate[] = "NAME=FUNC,DOC=DOCNAME,PROMPT=REQ=REQUEST/S,CLEAR/S";

struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,           (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmplate,
    AZT_Gen_Flags,          AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionLoad,         (ULONG) AZFunc_Load,
    AZT_ActionDo,           (ULONG) AZFunc_Do,
    TAG_END
    };

/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

static ULONG MIN( ULONG a, ULONG b )
{
    return( a > b ? b : a );
}

static void StrNCpy( UBYTE *Dest, UBYTE *Orig, ULONG Count )
{
  ULONG Ind;

    for(Ind=0; Ind < Count; Ind++) Dest[Ind] = Orig[Ind];
}

static ULONG IsReadDone( UBYTE *ptr, ULONG len )
{
  ULONG i;

    for (i=0; i < len; i++) if (ptr[i] == 0xc) return(1);
    return(0);
}

ULONG func_start( UBYTE *buf, ULONG buflen, ULONG ind )
{
    while (buf[ind] != '/')
        {
        if (buf[ind] == 0xc) return(0);
        if (++ind >= buflen) return(0);
        }
    return( ind + 1 );
}

ULONG func_end( UBYTE *buf, ULONG buflen, ULONG ind )
{
    while (buf[ind] != '\n' && buf[ind] != ' ' && buf[ind] != '\t')
        {
        if (buf[ind] == 0xc) return(0);
        if (++ind >= buflen) return(0);
        }
    return( ind );
}

static void FillListWithDocEntry( UBYTE *buf, ULONG buflen, struct ObjToto *om )
{
  ULONG ind=0, funcstart, len;
  UBYTE tmpbuf[32];

    UIK_OLi_NewTextPtr( om->MainList, "" );  // vide la liste
    UIK_OLi_ActualizeList( om->MainList );

    AZ_SetWaitPointer( om->MainWindow->Obj.w );
    UIK_Stop( om->MainList );
    while (TRUE)
        {
        if (! (ind = func_start( buf, buflen, ind ))) break;
        funcstart = ind;
        if (! (ind = func_end( buf, buflen, ind ))) break;
        len = ind - funcstart;
        StcCpy( tmpbuf, &buf[funcstart], MIN(32,len+1) );
        UIK_OLi_AddLine( om->MainList, tmpbuf, -1, 0 );
        }
    UIK_OLi_ActualizeList( om->MainList );
    UIK_Start( om->MainList );
    AZ_SetSystemPointer( om->MainWindow->Obj.w );
}

static void DoString2List( struct ObjToto *om )
{
  ULONG len, file, lock, oldlock, tablesize, readlen, str;
  UBYTE *name, *ubuf;

    UIK_LockObjVars( om->MainString ); /* Intuition ne doit pas modifier le buffer pendant le travail */
    str = 1;
    name = UIK_OSt_Get( om->MainString, &len ); /* name = buffer dynamique de l'objet UIKObj_String */
    StcCpy( om->CurrentDocName, name, 40 );
    if (len)
        {
        if (lock = Lock( PathForDoc, SHARED_LOCK ))
            {
            oldlock = CurrentDir( lock );
            if (file = Open( name, MODE_OLDFILE ))
                {
                UIK_UnlockObjVars( om->MainString ); str = 0; /* plus besoin de verrouiller le buffer */

                if (ubuf = BufAlloc( 512, 0, MEMF_PUBLIC ))
                    {
                    while (TRUE)
                        {
                        tablesize = BufLength( ubuf );
                        if (! BufResizeS( &ubuf, 512 + tablesize )) break;
                        if ((readlen = Read( file, ubuf + tablesize, 512 )) != 512)
                            {
                            if (readlen != -1) tablesize += readlen;
                            break;
                            }
                        if (IsReadDone( ubuf, tablesize )) break;
                        }
                    FillListWithDocEntry( ubuf, tablesize, om );
                    BufFree( ubuf );
                    }
                Close( file );
                }
            CurrentDir( oldlock );
            UnLock( lock );
            }
        }
    if (str) UIK_UnlockObjVars( om->MainString );
}

//-------------------------------------------------------------------------------

static void DocChoice( struct ObjToto *om )
{
  UBYTE *ptr, locklist;
  WORD sel;

    UIK_LockObjVars( om->SubList ); locklist = 1;
    if ((sel = om->SubList->Sel) != -1)
        {
        if (ptr = UIK_OLi_LineBuffer( om->SubList , sel ))
            {
            UIK_OSt_Set( om->MainString, ptr, StrLen( ptr ) );
            UIK_UnlockObjVars( om->SubList ); locklist = 0;
            UIK_Remove( om->SubWindow );
            DoString2List( om );
            }
        }
    if (locklist) UIK_UnlockObjVars( om->SubList );
}

static void SelectListEntry( struct UIKObjList *objlist, struct ObjToto *om )
{
  struct IntuiMessage *imsg = objlist->Obj.UIK->IntuiMsg;
  ULONG doubleclick = 0;
  UBYTE docname[40];
  WORD sel;

    if (imsg)
        {
        doubleclick = DoubleClick( om->SubSelSecs, om->SubSelMicros, imsg->Seconds, imsg->Micros );
        om->SubSelSecs = imsg->Seconds; om->SubSelMicros = imsg->Micros;
        }
    else doubleclick = 1; // sélection par joystick

    docname[0] = 0;

    UIK_LockObjVars( objlist );
    sel = objlist->Sel;
    if (sel != -1 && doubleclick && sel == om->SubOldSel)
        StcCpy( docname, UIK_OLi_LineBuffer( objlist, sel ), 39 );
    om->SubOldSel = sel;
    UIK_UnlockObjVars( objlist );

    if (docname[0])
        {
        UIK_OSt_Set( om->MainString, docname, StrLen( docname ) );
        UIK_Remove( om->SubWindow );
        DoString2List( om );
        }
}

void ActualizeSubList( struct ObjToto *om )
{
  struct FileInfoBlock *fib;
  ULONG lock;

    AZ_SetWaitPointer( om->SubWindow->Obj.w );
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
                                if (! UIK_OLi_AddLine( om->SubList, fib->fib_FileName, -1, 0 )) break;
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
    UIK_OLi_ActualizeList( om->SubList );
    AZ_SetSystemPointer( om->SubWindow->Obj.w );
}

static void DocSelectorKeys( struct UIKObj *win, struct ObjToto *om )
{
  struct IntuiMessage *imsg = win->UIK->IntuiMsg;
  struct UIKObjList *olist = om->SubList;
  UWORD alt, shift;

    if (imsg->Code & 0x80) return;
    alt = imsg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT);
    shift = imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT);

    switch (imsg->Code)
        {
        case 0x43:                                              /* ent */
        case 0x44:                                              /* ret */
            DocChoice( om );
            break;
        case 0x4c:                                              /* haut */
            if (alt) UIK_OLi_SetListTop( olist, 0 );
            else if (shift) UIK_OLi_MoveListFrame( olist, -1 );
            else UIK_OLi_MoveListSel( olist, -1 );
            break;
        case 0x4d:                                              /* bas */
            if (alt) UIK_OLi_SetListTop( olist, -1 );
            else if (shift) UIK_OLi_MoveListFrame( olist, 1 );
            else UIK_OLi_MoveListSel( olist, 1 );
            break;
        }
}

static void RemoveSubWindow( int nil, struct ObjToto *om )
{
    UIK_Remove( om->SubWindow );
}

static void ResizeSubUpDown( struct UIKObjWindow *wo, struct ObjToto *om )
{
  int size;

    size = om->SubList->Obj.Box.Width;
    om->SubDown->Box.Left = om->SubUp->Box.Left + size/2;
    om->SubUp->Box.Width = om->SubDown->Box.Left - om->SubUp->Box.Left;
    om->SubDown->Box.Width = size - om->SubUp->Box.Width;

    (*om->SubDown->ResizeVector)( om->SubDown );
    (*om->SubUp->ResizeVector)( om->SubUp );
}

static void OpenSubWindow( APTR nil, struct ObjToto *om )
{
    if (! (om->SubWindow = (struct UIKObjWindow *) UIK_AddObjectTags( "UIKObj_Window", om->MainWindow,
            UIKTAG_OBJ_WidthHeight,   SETL(199,159),
            UIKTAG_OBJ_Title,         SELECTDOC_TITLE,
            UIKTAG_OBJ_AltTitle,      SELECTDOC_TITLE,
            UIKTAG_OBJ_ActInactPens,  UIKCOLS(UIKCOL_LIGHT,UIKCOL_GREY,UIKCOL_WHITE,UIKCOL_BLACK),
            UIKTAG_OBJWindow_MinimumWidthHeight,SETL(140,80),
            UIKTAG_OBJWindowFl_CloseStop,  FALSE,
            UIKTAG_OBJWindowFl_AutoCenter, TRUE,
            UIKTAG_OBJ_LangMaster,    om,
            TAG_END ))) goto END_SELECT ;

    if (! (om->SubList = (struct UIKObjList *)UIK_AddObjectTags( "UIKObj_List", om->SubWindow,
            UIKTAG_OBJ_LeftTop,             SETL(16,14),
            UIKTAG_OBJ_WidthHeight,         SETL(150,124),
            UIKTAG_OBJ_Title,               DOCSLIST_TITLE,
            UIKTAG_OBJ_TitleFl_Left,        TRUE,
            UIKTAG_OBJ_TitleFl_Above,       TRUE,
            UIKTAG_OBJListFl_ClickSel,      TRUE,
            UIKTAG_OBJListFl_SelDispl,      TRUE,
            UIKTAG_OBJListFl_Sorted,        TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJ_LangMaster,          om,
            TAG_END ))) goto END_SELECT;

    if (! (om->SubProp = UIK_AddObjectTags( "UIKObj_PropLift", om->SubWindow,
            UIKTAG_OBJ_LeftTop,             SETL(167,14),
            UIKTAG_OBJ_WidthHeight,         SETL(20,124),
            UIKTAG_OBJ_BoxFl_Over,          FALSE,
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJProp_Vertical,        TRUE,
            UIKTAG_OBJ_Pattern,             (ULONG)PatProp,
            UIKTAG_OBJ_PatternPower,        1,
            UIKTAG_OBJ_LangMaster,          om,
            TAG_END ))) goto END_SELECT;

    if (! (om->SubUp = UIK_AddObjectTags( "UIKObj_Up", om->SubWindow,
            UIKTAG_OBJ_LeftTop,             SETL(16,139),
            UIKTAG_OBJ_WidthHeight,         SETL(75,12),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJ_LangMaster,          om,
            TAG_END ))) goto END_SELECT;

    if (! (om->SubDown = UIK_AddObjectTags( "UIKObj_Down", om->SubWindow,
            UIKTAG_OBJ_LeftTop,             SETL(91,139),
            UIKTAG_OBJ_WidthHeight,         SETL(75,12),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJ_LangMaster,          om,
            TAG_END ))) goto END_SELECT;

    /* On lie l'ascenceur a la liste */
    if (! UIK_Link( om->SubProp, om->SubList, 0, MOUSEMOVE|GADGETDOWN, 0 )) goto END_SELECT;

    /* On lie les boutons Up et Down à l'ascenceur */
    if (! UIK_Link( om->SubUp, om->SubProp, 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) goto END_SELECT;
    if (! UIK_Link( om->SubDown, om->SubProp, 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) goto END_SELECT;

    ActualizeSubList( om );

    if ( ! UIK_AddVector( om->SubWindow , (void *)ResizeSubUpDown, NEWSIZE, (ULONG)om )) goto END_SELECT;
    if ( ! UIK_AddVector( om->SubWindow , (void *)RemoveSubWindow, CLOSEWINDOW, (ULONG)om )) goto END_SELECT;
    if ( ! UIK_AddVector( om->SubList, (void *)SelectListEntry, GADGETDOWN, (ULONG)om )) goto END_SELECT;
    if ( ! UIK_AddVector( om->SubWindow, (void*)DocSelectorKeys, RAWKEY, (ULONG)om )) goto END_SELECT;

    if (! UIK_Start( om->SubWindow )) goto END_SELECT;

  END_SELECT:
    return; // contre warnings
}

//-----------------------------------------------------------------------------------------------

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

static LONG SearchIndex( REFERENCE *ref, UBYTE *funcname, UBYTE *docname, ULONG *pfrom, ULONG *ptotal )
{                                                           // ref est en sortie, docname est facultatif
  ULONG lock, oldlock, file, filesize, i, refnum, from=0;
  LONG found=-1;
  UBYTE filename[2], *ptr;
  REFERENCE *reftable;

    if (pfrom) from = *pfrom;

    if (ptr = funcname)
        {
        while (*ptr)
            {
            if (*ptr == '(') *ptr = 0;
            else ptr++;
            }
        }

    if (valid_filename( funcname, filename ))
        {
        if (lock = Lock( PathForIndex, SHARED_LOCK ))
            {
            oldlock = CurrentDir( lock );
            if (file = Open( filename, MODE_OLDFILE ))
                {
                filesize = Seek( file, Seek( file, 0, OFFSET_END ), OFFSET_BEGINNING );
                if (reftable = (REFERENCE*) BufAlloc( 512, filesize, MEMF_PUBLIC ))
                    {
                    if (Read( file, reftable, filesize ) == filesize)
                        {
                        refnum = filesize / sizeof(REFERENCE);
                        if (ptotal) *ptotal = refnum;
                        for (i=from; i < refnum; i++)
                            {
                            if (StrNCCmp( reftable[i].FnctName, funcname ) == 0)
                                {
                                if (docname && docname[0])  // les docs contiennent des noms en
                                    {                       // double comme --Overview--, etc...
                                    if (StrNCCmp( reftable[i].DocFileName, docname ) == 0)
                                        {
                                        found = i;
                                        break;
                                        }
                                    else{
                                        found = -2;
                                        break;
                                        }
                                    }
                                else{
                                    found = -2;
                                    break;
                                    }
                                }
                            }
                        if (found != -1)
                            {
                            *ref = reftable[i];
                            if (pfrom) *pfrom = i;
                            /*
                            StrCpy( ref->FnctName, reftable[i].FnctName );
                            StrCpy( ref->Owner, reftable[i].Owner );
                            StrCpy( ref->DocFileName, reftable[i].DocFileName );
                            ref->Offset = reftable[i].Offset;
                            ref->Size   = reftable[i].Size;
                            */
                            }
                        }
                    BufFree( (APTR)reftable );
                    }
                Close( file );
                }
            CurrentDir( oldlock );
            UnLock( lock );
            }
        }
    return( found );
}

static ULONG twh( UBYTE *buf )
{
  UBYTE *str, *ptr;
  UWORD maxw=0, w, h=1;

    ptr = str = buf;
    while (*ptr)
        {
        while (*ptr && *ptr != '\n') ptr++;
        w = ptr - str; if (w > maxw) maxw = w;
        h++;
        if (*ptr) ptr++; // skip newline
        str = ptr;
        }
    return( W2L(maxw+1,h) );
}

static ULONG DisplayAmigaManFonction( REFERENCE *ref )
{
  ULONG lock, oldlock, file, tmpfile, writelen, ok=0;
  UBYTE *ubuf, TmpFileName[40];

    if (lock = Lock( PathForDoc, SHARED_LOCK ))
        {
        oldlock = CurrentDir( lock );
        if (file = Open( ref->DocFileName, MODE_OLDFILE ))
            {
            if (ubuf = BufAlloc( 512, ref->Size, MEMF_PUBLIC ))
                {
                Seek( file, ref->Offset, OFFSET_BEGINNING );
                if (Read( file, ubuf, ref->Size ) == ref->Size)
                    {
                    sprintf( TmpFileName, TMPFILE_FMT, ref->FnctName );
                    if (tmpfile = Open( TmpFileName, MODE_NEWFILE ))
                        {
                        writelen = Write( tmpfile, ubuf, ref->Size );
                        Close( tmpfile );
                        if (writelen == ref->Size)
                            {
                            ok = 1;
                            AZ_LaunchProcess( // fonction synchrone si on donne le nom de fichier
                                    AZTAG_Launch_TextWidthHeight,   twh( ubuf ),
                                    AZTAG_Launch_FileName,          TmpFileName,
                                    AZTAG_LaunchF_ReadOnly,         TRUE,
                                    TAG_END);
                            }
                        DeleteFile( TmpFileName );
                        }
                    }
                BufFree( ubuf );
                }
            Close( file );
            }
        CurrentDir( oldlock );
        UnLock( lock );
        }
    return( ok );
}

static void SelectMainList( struct UIKObjList *objlist, struct ObjToto *om )
{
  struct IntuiMessage *imsg = objlist->Obj.UIK->IntuiMsg;
  ULONG doubleclick = 0;
  UBYTE funcname[40];
  WORD sel;

    if (imsg)
        {
        doubleclick = DoubleClick( om->MainSelSecs, om->MainSelMicros, imsg->Seconds, imsg->Micros );
        om->MainSelSecs = imsg->Seconds; om->MainSelMicros = imsg->Micros;
        }
    else doubleclick = 1; // sélection par joystick

    funcname[0] = 0;

    UIK_LockObjVars( objlist );
    sel = objlist->Sel;
    if (sel != -1 && doubleclick && sel == om->MainOldSel)
        StcCpy( funcname, UIK_OLi_LineBuffer( objlist, sel ), 39 );
    om->MainOldSel = sel;
    UIK_UnlockObjVars( objlist );

    if (funcname[0])
        {
        REFERENCE IndexReference;
        if (SearchIndex( &IndexReference, funcname, om->CurrentDocName, 0, 0 ) >= 0)
            DisplayAmigaManFonction( &IndexReference );
        }
}

static void MainString2List( struct UIKObjStrGad *ostr, struct ObjToto *om )
{
  struct IntuiMessage *imsg = ostr->Obj.UIK->IntuiMsg;

    if (imsg && imsg->Code & 0x80) return; /* touche relevée */

    if (ostr->Obj.UIK->KeySupp.Length == 1 && ostr->Obj.UIK->KeySupp.KeyBuffer[0] == 0x0d)  /* touche RETURN */
        {
        DoString2List( om );
        }
    return;
}

static void ResizeMainUpDown( struct UIKObjWindow *wo, struct ObjToto *om )
{
  int size;

    size = om->MainList->Obj.Box.Width;
    om->MainDown->Box.Left = om->MainUp->Box.Left + size/2;
    om->MainUp->Box.Width = om->MainDown->Box.Left - om->MainUp->Box.Left;
    om->MainDown->Box.Width = size - om->MainUp->Box.Width;

    (*om->MainDown->ResizeVector)( om->MainDown );
    (*om->MainUp->ResizeVector)( om->MainUp );
}

static void MainRawkeys( struct UIKObj *win, struct ObjToto *om )
{
  struct IntuiMessage *imsg = win->UIK->IntuiMsg;
  struct UIKObjList *olist = om->MainList;
  UBYTE funcname[40];
  WORD alt, shift;

    if (imsg->Code & 0x80) return;
    alt = imsg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT);
    shift = imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT);

    switch (imsg->Code)
        {
        case 0x43:                                              /* ent */
        case 0x44:                                              /* ret */
            funcname[0] = 0;
            UIK_LockObjVars( olist );
            if (olist->Sel != -1) StcCpy( funcname, UIK_OLi_LineBuffer( olist, olist->Sel ), 39 );
            UIK_UnlockObjVars( olist );
            if (funcname[0])
                {
                REFERENCE IndexReference;
                if (SearchIndex( &IndexReference, funcname, om->CurrentDocName, 0, 0 ) >= 0)
                    DisplayAmigaManFonction( &IndexReference );
                }
            break;
        case 0x4c:                                              /* haut */
            if (alt) UIK_OLi_SetListTop( olist, 0 );
            else if (shift)
                UIK_OLi_MoveListFrame( olist, -1 );
            else UIK_OLi_MoveListSel( olist, -1 );
            break;
        case 0x4d:                                              /* bas */
            if (alt) UIK_OLi_SetListTop( olist, -1 );
            else if (shift)
                UIK_OLi_MoveListFrame( olist, 1 );
            else UIK_OLi_MoveListSel( olist, 1 );
            break;
        case 0x5f:                                              /* help */
            OpenSubWindow( 0, om );
            break;
        case 0x40:                                              /* space */
            UIK_ActivateKeyTank( om->MainString );
            break;
        default:
            break;
        }
}

static void Cleanup( int nil, struct ObjToto *om )
{
    UIK_Remove( om );
}

static void AddMainWindow( struct CmdObj *co, struct AZurProc *ap )
{
  struct ObjToto *om;

    if (! (om = (struct ObjToto *) UIK_AddPrivateObject( &ObjTotoRegGerm, ap->WO , NULL ))) goto END_ERROR;

    if (! (om->MainWindow = (struct UIKObjWindow *) UIK_AddObjectTags( "UIKObj_Window", om,
            UIKTAG_OBJ_WidthHeight,             SETL(285,168),
            UIKTAG_OBJ_Title,                   MAINWINDOW_TITLE,
            UIKTAG_OBJ_AltTitle,                MAINWINDOW_TITLE,
            UIKTAG_OBJ_ActInactPens,            UIKCOLS(UIKCOL_LIGHT,UIKCOL_GREY,UIKCOL_WHITE,UIKCOL_BLACK),
            UIKTAG_OBJWindow_MinimumWidthHeight,SETL(140,80),
            UIKTAG_OBJ_LangMaster,              om,
            UIKTAG_OBJWindowFl_CloseStop,       FALSE,
            UIKTAG_OBJWindowFl_AutoCenter,      TRUE,
            TAG_END ))) goto END_ERROR;

    if (! (om->MainButton = UIK_AddObjectTags( "UIKObj_Boolean", om->MainWindow,
            UIKTAG_OBJ_LeftTop,         SETL(15,8),
            UIKTAG_OBJ_WidthHeight,     SETL(95,16),
            UIKTAG_OBJ_Title,           DOCSBUTTON_TITLE,
            UIKTAG_OBJ_BoxFl_Filled,    TRUE,
            UIKTAG_OBJ_LangMaster,      om,
            UIKTAG_OBJ_TitleUnderscore, '_',
            TAG_END ))) goto END_ERROR;

    if (! (om->MainString = (struct UIKObjStrGad *) UIK_AddObjectTags( "UIKObj_String", om->MainWindow,
            UIKTAG_OBJ_LeftTop,             SETL(112,9),
            UIKTAG_OBJ_WidthHeight,         SETL(159,0),
            UIKTAG_OBJStr_TextPointer,      "",
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_LangMaster,          om,
            TAG_END ))) goto END_ERROR;

    if (! (om->MainList  = (struct UIKObjList *)UIK_AddObjectTags( "UIKObj_List", om->MainWindow,
            UIKTAG_OBJ_LeftTop,             SETL(15,38),
            UIKTAG_OBJ_WidthHeight,         SETL(236,110),
            UIKTAG_OBJ_Title,               CMDSLIST_TITLE,
            UIKTAG_OBJ_TitleFl_Left,        TRUE,
            UIKTAG_OBJ_TitleFl_Above,       TRUE,
            UIKTAG_OBJListFl_ClickSel,      TRUE,
            UIKTAG_OBJListFl_SelDispl,      TRUE,
            UIKTAG_OBJListFl_Sorted,        TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Left,  TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJ_LangMaster,          om,
            TAG_END ))) goto END_ERROR;

    if (! (om->MainProp = UIK_AddObjectTags( "UIKObj_PropLift", om->MainList,
            UIKTAG_OBJ_LeftTop,             SETL(252,38),
            UIKTAG_OBJ_WidthHeight,         SETL(20,110),
            UIKTAG_OBJ_BoxFl_Over,          FALSE,
            UIKTAG_OBJ_Sz_AttachedFl_Top,   TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Right, TRUE,
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJProp_Vertical,        TRUE,
            UIKTAG_OBJ_Pattern,             (ULONG)PatProp,
            UIKTAG_OBJ_PatternPower,        1,
            UIKTAG_OBJ_LangMaster,          om,
            TAG_END ))) goto END_ERROR;

    if (! (om->MainUp = UIK_AddObjectTags( "UIKObj_Up", om->MainList,
            UIKTAG_OBJ_LeftTop,             SETL(15,149),
            UIKTAG_OBJ_WidthHeight,         SETL(118,12),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJ_LangMaster,          om,
            TAG_END ))) goto END_ERROR;

    if (! (om->MainDown = UIK_AddObjectTags( "UIKObj_Down", om->MainList,
            UIKTAG_OBJ_LeftTop,             SETL(134,149),
            UIKTAG_OBJ_WidthHeight,         SETL(118,12),
            UIKTAG_OBJ_Sz_AttachedFl_Bot,   TRUE,
            UIKTAG_OBJ_LangMaster,          om,
            TAG_END ))) goto END_ERROR;

    /* On lie l'ascenceur à la liste */
    if (! UIK_Link( om->MainProp, om->MainList, 0, MOUSEMOVE|GADGETDOWN, 0 )) goto END_ERROR;

    /* On lie les boutons Up et Down à l'ascenceur */
    if (! UIK_Link( om->MainUp, om->MainProp , 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) goto END_ERROR;
    if (! UIK_Link( om->MainDown, om->MainProp , 0, GADGETDOWN|INTUITICKS|MOUSEMOVE, 0 )) goto END_ERROR;

    /* On ajoute un vecteur à la fenêtre pour retailler les 2 boutons */
    if ( ! UIK_AddVector( om->MainWindow , (void *)ResizeMainUpDown, NEWSIZE, (ULONG)om )) goto END_ERROR;
    if ( ! UIK_AddVector( om->MainWindow , (void *)Cleanup, CLOSEWINDOW, (ULONG)om )) goto END_ERROR;
    if ( ! UIK_AddVector( om->MainButton, (void *)OpenSubWindow, GADGETUP, (ULONG)om )) goto END_ERROR;
    if ( ! UIK_AddVector( om->MainString , (void*)MainString2List, RAWKEY, (ULONG)om )) goto END_ERROR;
    if ( ! UIK_AddVector( om->MainList, (void *)SelectMainList , GADGETDOWN , (ULONG)om )) goto END_ERROR;
    if ( ! UIK_AddVector( om->MainWindow, (void*)MainRawkeys, RAWKEY, (ULONG)om )) goto END_ERROR;

    if (! UIK_Start( om )) goto END_ERROR;

    return;

  END_ERROR:
    UIK_Remove( om );
}

#define SCANARG_NAME    0
#define SCANARG_DOCNAME 1
#define SCANARG_PROMPT  2
#define SCANARG_CLEAR   3

static ULONG get_myenv()
{
    if (PathForDoc = AZ_EnvGet( "AZMan" ))
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
            AZ_EnvFree( PathForDoc );
            PathForDoc = 0;
            }
        }
    return( (ULONG)PathForDoc );
}

static void AZFunc_Do( struct CmdObj *co )
{
  struct AZurProc *ap = AZ_GetAZur();
  REFERENCE IndexReference;

    UIKBase = ap->UIKBase;
    SysBase = ap->ExecBase;
    DOSBase = ap->DOSBase;
    IntuitionBase = ap->IntuitionBase;

    if (PathForDoc == 0)
        {
        if (! get_myenv())
            {
            AZ_ReqShowText( "Les répertoires d'autodocs et d'index n'ont\n"
                            "pas encore été définis par le programme\n"
                            "           \"AZMan_MakeIndex\"\n"
                            "(ou le menu \"Aide/Créer des index\")\n" );
            return;
            }
        }

    if (co->ArgRes[SCANARG_NAME])
        {
        if (co->ArgRes[SCANARG_DOCNAME])
            {
            if (SearchIndex( &IndexReference, co->ArgRes[SCANARG_NAME], co->ArgRes[SCANARG_DOCNAME], 0, 0 ) >= 0)
                DisplayAmigaManFonction( &IndexReference );
            }
        else{
            LONG from, total=1, found;
            for (from=0; from < total; from++)
                {
                if ((found = SearchIndex( &IndexReference, co->ArgRes[SCANARG_NAME], 0, &from, &total )) != -1)
                    {   // si -2 : fonction ok mais éventuellement pas bonne doc, donc continue
                    if (DisplayAmigaManFonction( &IndexReference )) break;
                    }
                else break;
                }
            }
        }
    else if (co->ArgRes[SCANARG_PROMPT])
        {
        AddMainWindow( co, ap );
        }
    else{
        struct AZBlock *azb;
        LONG line, scol, ecol, len;
        UBYTE *ptr, name[40];

        name[0] = 0;
        line = ap->FObj->Line; scol = ap->FObj->Col;

        if (azb = AZ_BlockPtr( AZ_BlockCurrent() ))
            {
            line = azb->LineStart; scol = ecol = azb->ColStart;
            if (line == azb->LineEnd)
                {
                ptr = AZ_LineBuf( line );
                ecol = azb->ColEnd;
                if ((len = ecol - scol + 1) > 0)
                    {
                    if (++len > 40) len = 40;
                    if (AZ_CharIsWord( ptr[scol] ) && AZ_CharIsWord( ptr[ecol] ))
                        StcCpy( name, &ptr[scol], MIN(40,len) );

                    if (co->ArgRes[SCANARG_CLEAR])
                        AZ_BlockClearAll();
                    }
                }
            }

        if (name[0] == 0)
            {
            ptr = AZ_LineBuf( line );
            len = AZ_LineBufLen( ptr );
            if (! AZ_CharIsWord( ptr[scol] ))
                if (scol > 0) scol--;
            if (AZ_CharIsWord( ptr[scol] ))
                {
                ecol = scol;
                for (; scol > 0; scol--) if (! AZ_CharIsWord( ptr[scol-1] )) break;
                for (; ecol < len; ecol++) if (! AZ_CharIsWord( ptr[ecol+1] )) break;
                if ((len = ecol - scol + 2) > 40) len = 40;
                StcCpy( name, &ptr[scol], MIN(40,len) );
                }
            }

        if (name[0])
            {
            LONG from, total=1, found, disp;
            for (from=0; from < total; from++)
                {
                if ((found = SearchIndex( &IndexReference, name, 0, &from, &total )) != -1)
                    {   // si -2 : fonction ok mais éventuellement pas bonne doc, donc continue
                    if (disp = DisplayAmigaManFonction( &IndexReference )) break;
                    }
                else break;
                }
            }
        }
}

static void AZFunc_Load( struct CmdObj *co )
{
    get_myenv();
    /* Si les préférences n'existent pas elles pourront être créées
     * plus tard par le programme AZMan_MakeIndex.
     */
}

static void AZFunc_Unload( struct CmdObj *co )
{
    AZ_EnvFree( PathForDoc );
    PathForDoc = 0;
    PathForIndex = 0;
}
