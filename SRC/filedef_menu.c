/****************************************************************
 *
 *      File:      filedef_menu.c
 *      Project:   AZur
 *
 *      Created:   16-06-93     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:uikbase.h"
#include "uiki:objwindow.h"

#include "lci:objmenu.h"
#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"
#include "lci:_pool_protos.h"

#include "lci:objfile.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern struct UIKObjMenu *ofi_AddMenuObj();


/****** Exported ***********************************************/


/****** Statics ************************************************/

static struct AZMenuEntry __far NullME;

static UBYTE __far defaults_maineng[] =
"MENU \" Project \"\0"
"ITEM \"Other Window\" \"T\" File2NewWin FILEREQ\0"
"ITEM \"Console...\" \"=\" ExeARexxScript ASYNC CONSOLE AZur:Rexx/Console.raz\0"
"BAR\0"
"ITEM \"About\" \"?\" ReqOpen About\0"
"BAR\0"
"ITEM \"Quit AZur\" \"Q\" WinClose ALL FORCE\0"
"#\0";

static UBYTE __far defaults_mainother[] =
"MENU \" Projet \"\0"
"ITEM \"Autre fenêtre\" \"T\" File2NewWin FILEREQ\0"
"ITEM \"Console...\" \"=\" ExeARexxScript ASYNC CONSOLE AZur:Rexx/Console.raz\0"
"BAR\0"
"ITEM \"A Propos\" \"?\" ReqOpen About\0"
"BAR\0"
"ITEM \"Quitter AZur\" \"Q\" WinClose ALL FORCE\0"
"#\0";


static UBYTE __far defaults_eng[] =
"MENU \" Project \"\0"
"ITEM \"New\" \"\" WinClear\0"
"ITEM \"Other Window\" \"T\" File2NewWin FILEREQ\0"
"DIV\0"
"ITEM \"Open...\" \"O\" File2Win SETCURRENT CLEAR NOCURS\0"
"ITEM \"Save\" \"S\" Win2File SAME\0"
"ITEM \"Save As...\" \"A\" Win2File SETCURRENT\0"
"DIV\0"
"ITEM \"Console...\" \"=\" ExeARexxScript ASYNC CONSOLE AZur:Rexx/Console.raz\0"
"BAR\0"
"ITEM \"About\" \"?\" ReqOpen About\0"
"BAR\0"
"ITEM \"Quit AZur\" \"Q\" WinClose ALL\0"

"MENU \" Edit \"\0"
"ITEM \"Cut\" \"X\" Block CLIP=0 ERASE\0"
"ITEM \"Copy\" \"C\" Block CLIP=0\0"
"ITEM \"Paste\" \"V\" CB2Win\0"

"MENU \" Search \"\0"
"ITEM \"Search...\" \"F\" Search CLEAR REQ\0"
"DIV\0"
"ITEM \"Search Next\" \"N\" Search CLEAR FORW\0"
"ITEM \"Search Back\" \"B\" Search CLEAR BACK\0"
"BAR\0"
"ITEM \"Exchange...\" \"E\" Replace CLEAR REQ\0"
"#\0";

static UBYTE __far defaults_other[] =
"MENU \" Projet \"\0"
"ITEM \"Nouveau\" \"\" WinClear\0"
"ITEM \"Autre fenêtre\" \"T\" File2NewWin FILEREQ\0"
"DIV\0"
"ITEM \"Ouvrir...\" \"O\" File2Win SETCURRENT CLEAR NOCURS\0"
"ITEM \"Sauver\" \"S\" Win2File SAME\0"
"ITEM \"Sauver En...\" \"A\" Win2File SETCURRENT\0"
"DIV\0"
"ITEM \"Console...\" \"=\" ExeARexxScript ASYNC CONSOLE AZur:Rexx/Console.raz\0"
"BAR\0"
"ITEM \"A Propos\" \"?\" ReqOpen About\0"
"BAR\0"
"ITEM \"Quitter AZur\" \"Q\" WinClose ALL\0"

"MENU \" Edition \"\0"
"ITEM \"Couper\" \"X\" Block CLIP=0 ERASE\0"
"ITEM \"Copier\" \"C\" Block CLIP=0\0"
"ITEM \"Coller\" \"V\" CB2Win\0"

"MENU \" Recherche \"\0"
"ITEM \"Chercher...\" \"F\" Search CLEAR REQ\0"
"DIV\0"
"ITEM \"Chercher Suivant\" \"N\" Search CLEAR FORW\0"
"ITEM \"Chercher Précédent\" \"B\" Search CLEAR BACK\0"
"BAR\0"
"ITEM \"Remplacer...\" \"E\" Replace CLEAR REQ\0"
"#\0";


/****************************************************************
 *
 *      Keys handling
 *
 ****************************************************************/

ULONG Parm_InitMenu()
{
    return(1);
}
ULONG Parm_InitMainMenu()
{
    return(1);
}


static ULONG cleanup_menu( ULONG quit, ULONG main )  // c'est la commande qui doit faire le stop du menu
{
  struct MenuHandle *mh = (main) ? AGP.AZMast->MainMenuHandle : AGP.AZMast->MenuHandle;
  struct AZMenuEntry *me;
  ULONG i, max;
  APTR pool=0;

    if (mh)
        {
        /*
        if (mh->MenuEntry)
            {
            max = intAZ_LineBufLen( (UBYTE*)mh->MenuEntry ) / sizeof(struct AZMenuEntry);
            for (i=0, me=mh->MenuEntry; i < max; i++, me++)
                {
                if (me->UserData)
                    {
                    eng_DisposeObject( (struct CmdObj *) me->UserData );
                    }
                }
            }
        */
        AsmDeletePool( mh->Pool, ExecBase );
        if (main) AGP.AZMast->MainMenuHandle = 0; else AGP.AZMast->MenuHandle = 0;
        }
    if (! quit)
        {
        if (! (pool = AsmCreatePool( MEMF_ANY|MEMF_CLEAR, 2000, 2000, ExecBase ))) goto END_ERROR;
        if (! (mh = (struct MenuHandle *) BufAllocP( pool, sizeof(struct MenuHandle), sizeof(struct MenuHandle) ))) goto END_ERROR;
        if (! (me = (struct AZMenuEntry *) BufAllocP( pool, sizeof(struct AZMenuEntry), 10*sizeof(struct AZMenuEntry) ))) goto END_ERROR;
        mh->Pool = pool;
        mh->MenuEntry = me;
        if (main) AGP.AZMast->MainMenuHandle = mh; else AGP.AZMast->MenuHandle = mh;
        }
    return(1);

  END_ERROR:
    if (pool) AsmDeletePool( pool, ExecBase );
    return(0);
}

ULONG Parm_CleanupMenu( ULONG quit )  // c'est la commande qui doit faire le stop du menu
{
    return( cleanup_menu( quit, 0 ) );
}

ULONG Parm_CleanupMainMenu( ULONG quit )  // c'est la commande qui doit faire le stop du menu
{
    return( cleanup_menu( quit, 1 ) );
}
//-------------------------------------------------------------

/* fonction lorsque les objets sont pré-ajoutés
static void MenuFunction( struct UIKObjMenu *mo, struct AZMenuEntry *ume, struct MenuItem *item, UWORD menunum, UWORD itemnum, UWORD subnum )
{
    func_ExecuteCmdObj( (struct CmdObj *)ume->UserData, 0 );
    StoreLastCmd( (struct CmdObj *)ume->UserData );
}
*/

static void MenuFunction( struct UIKObjMenu *mo, struct AZMenuEntry *ume )
{
    eng_ExecuteAZurCmd( (UBYTE*)ume->UserData, FROMMENU, 0 );
}

static ULONG interpret_item( struct read_file *rf, struct AZMenuEntry *pm, UBYTE *p, ULONG main )
{
  struct MenuHandle *mh = (main) ? AGP.AZMast->MainMenuHandle : AGP.AZMast->MenuHandle;
  UBYTE *key=0, *cmdname=0, *parm="", *mem=0;
  struct CmdObj *co=0;
  ULONG len;

    p += skip_char( p, ' ', '\t' );  // saute espaces entre title et key
    if (*p)
        {
        if (*p == '"') // raccourcis clavier ?
            {
            p++; // skip 1er "
            if (*p) // premier de key
                {
                if ((len = find_char( p, '"', '"' )) > 0)
                    {
                    if (len == 1) /* raccourcis Intuition */
                        {
                        mh->ComKeys[*p]++;
                        if (Keys_AddShortCut( p, 1, IEQUALIFIER_LCOMMAND ) == 0) goto END_ERROR;
                        }
                    else{ /* raccourcis Maison */
                        if (Keys_AddShortCut( p, 1, 0 ) == 0) goto END_ERROR;
                        }
                    if (! (key = AsmAllocPooled( mh->Pool, len+1, ExecBase ))) goto END_ERROR;
                    StcCpy( key, p, len+1 );
                    p += len; // skip name
                    }
                p++; // skip 2ème "
                }
            }

        if (*p)
            {
            p += skip_char( p, ' ', '\t' );     // saute espaces entre key et cmdname
            if (*p)
                {
                if (! (mem = (UBYTE*)EqualReplace( mh->Pool, p )))
                    {
                    //if (! (mem = BufAllocP( mh->Pool, StrLen(p), 4 ))) goto END_ERROR;
                    if (! (mem = AsmAllocPooled( mh->Pool, StrLen(p)+1, ExecBase ))) goto END_ERROR;
                    StrCpy( mem, p );
                    }
                cmdname = p = mem;
                if (StrNCCmp( cmdname, "nop" ) == 0) cmdname = 0;
                /*
                p += find_char( p, ' ', '\t' ); // cherche espace de fin cmdname
                if (*p)
                    {
                    *p++ = 0;                   // cmdname OK
                    parm = p + skip_char( p, ' ', '\t' ); // saute espaces entre cmdname et parms
                    }
                */
                }

            if (cmdname == 0)
                {
                cmdname = "NOp";
                pm->Flags &= ~ITEMENABLED;
                }

            pm->ComKey = key;
            pm->Func = (void*) MenuFunction;
            pm->UserData = (ULONG)cmdname;
            /*
            if (co = eng_NewObjectTags( cmdname, AZT_CmdLine, parm, AZT_ExeFrom, FROMMENU, TAG_END ))
                {
                pm->ComKey = key;
                pm->Func = (void*) MenuFunction;
                pm->UserData = (ULONG) co;
                eng_DoAction( co, AZA_INITMENU, (ULONG)pm );
                }
            else goto END_ERROR;
            */
            }
        else pm->Flags &= ~ITEMENABLED;
        }
    return(1);

  END_ERROR:
    pm->Flags &= ~ITEMENABLED;
    return(0);
}

static UBYTE *find_title( struct read_file *rf, struct AZMenuEntry *pm, UBYTE *p, struct MenuHandle *mh )
{
  UBYTE *title;

    p += skip_char( p, ' ', '\t' );
    title = ++p;            // skip 1er "
    p += find_char( p, '"', '"' );
    if (*p) *p++ = 0;       // 2eme " ok

    if (! (pm->Title = AsmAllocPooled( mh->Pool, p - title + 1, ExecBase ))) return(0);
    StrCpy( pm->Title, title );
    return(p);
}

ULONG interpret_menus( struct read_file *rf, ULONG append, ULONG main )
{
  struct MenuHandle *mh = (main) ? AGP.AZMast->MainMenuHandle : AGP.AZMast->MenuHandle;
  ULONG key, len, found;
  APTR oldmenu=0;

    if (main == 0 && AGP.FObj && (oldmenu = (APTR)AGP.FObj->Menu)) { UIK_Remove( oldmenu ); AGP.FObj->Menu = 0; }

    rf->offs = 0;

    if (append == 0 || mh == 0)
        {
        if (! cleanup_menu( 0, main )) return(0);
        mh = (main) ? AGP.AZMast->MainMenuHandle : AGP.AZMast->MenuHandle;
        }
    else{
        rf->offs = intAZ_LineBufLen( (UBYTE*)mh->MenuEntry ) - sizeof(struct AZMenuEntry); // avant la dernière nulle
        }

    if (append == 0)
        {
        for (key=0; key < 256; key++) mh->ComKeys[key] = 0;
        }

    while (!rf->diese && rf->ptr < rf->max)
        {
        struct AZMenuEntry me=NullME;

        rf->ptr += skip_char( rf->ptr, ' ', '\t' );
        if (len = StrLen( rf->ptr ))
            {
            UBYTE *p = rf->ptr;
            ULONG ltext = B2L(p[0],p[1],p[2],p[3]);

            found = 0;
            switch (*p)
                {
                case 'M':
                    if (ltext = B2L('M','E','N','U'))
                        {
                        me.Flags = MENUENABLED;
                        if (! find_title( rf, &me, p+4, mh )) goto END_ERROR;
                        found = 1;
                        }
                    break;
                case 'I':
                    if (ltext = B2L('I','T','E','M'))
                        {
                        me.Depth = 1;
                        me.Flags = ITEMENABLED;
                        if (! (p = find_title( rf, &me, p+4, mh ))) goto END_ERROR;
                        if (! interpret_item( rf, &me, p, main )) ; //goto END_ERROR;
                        else found = 1;
                        }
                    break;
                case 'D':
                    if (ltext = B2L('D','I','V',0))
                        {
                        me.Depth = 1;
                        me.Title = (UBYTE*)ITEM_SEPARATION;
                        found = 1;
                        }
                    break;
                case 'B':
                    if (ltext = B2L('B','A','R',0))
                        {
                        me.Depth = 1;
                        me.Title = (UBYTE*)ITEM_SYSSEPARATION;
                        found = 1;
                        }
                    break;
                case 'S':
                    switch (*(p+1))
                        {
                        case 'U':
                            if (ltext = B2L('S','U','B',0))
                                {
                                me.Depth = 2;
                                me.Flags = ITEMENABLED;
                                if (! (p = find_title( rf, &me, p+3, mh ))) goto END_ERROR;
                                if (! interpret_item( rf, &me, p, main )) ; //goto END_ERROR;
                                else found = 1;
                                }
                            break;
                        case 'D':
                            if (ltext = B2L('S','D','I','V'))
                                {
                                me.Depth = 2;
                                me.Title = (UBYTE*)ITEM_SEPARATION;
                                found = 1;
                                }
                            break;
                        case 'B':
                            if (ltext = B2L('S','B','A','R'))
                                {
                                me.Depth = 2;
                                me.Title = (UBYTE*)ITEM_SYSSEPARATION;
                                found = 1;
                                }
                            break;
                        }
                    break;
                case '#':
                    rf->diese = 1;
                    break;
                }

            if (found)
                {
                if (BufPasteS( (UBYTE*)&me, sizeof(struct AZMenuEntry), (UBYTE**)&mh->MenuEntry, rf->offs ))
                    {
                    rf->offs += sizeof(struct AZMenuEntry);
                    }
                }
            else { if (!rf->diese) goto END_ERROR; }
            }
        rf->ptr += len + 1;
        }

    for (key=0; key < 256; key++)
        {
        if (mh->ComKeys[key] > 1)
            {
            UBYTE keystr[2];
            keystr[0] = key; keystr[1] = 0;
            eng_ShowRequest( UIK_LangString( AGP.UIK, TEXT_ERR_MenuKeyTwice ), TEXT_ERR_MenuKeyTwice, keystr, (UBYTE*)((ULONG)mh->ComKeys[key]), 0 );
            }
        }

    if (oldmenu) AGP.FObj->Menu = ofi_AddMenuObj();
    return(1);

  END_ERROR:
    return(0);
}

//-------------------------------------------------------------

static ULONG verif_menu( ULONG defaut, ULONG main )
{
  struct MenuHandle *mh = (main) ? AGP.AZMast->MainMenuHandle : AGP.AZMast->MenuHandle;

    if (mh == 0 || intAZ_LineBufLen( (UBYTE*)mh->MenuEntry ) <= sizeof(struct AZMenuEntry))
        {
        if (defaut)
            {
            struct read_file rf = {0,0,0,0,0,0,0,0,0,0};
            UBYTE *ptr, *mem;
            ULONG size, rc=0;

            if (! (UIKBase->UIKPrefs->PrefsFlags & UIKLANG_OTHER))
                {
                ptr  = (main) ? defaults_maineng : defaults_eng;
                size = (main) ? sizeof(defaults_maineng) : sizeof(defaults_eng);
                }
            else{
                ptr  = (main) ? defaults_mainother : defaults_other;
                size = (main) ? sizeof(defaults_mainother) : sizeof(defaults_other);
                }
            if (mem = UIK_MemSpAlloc( size, MEMF_PUBLIC ))
                {
                CopyMem( ptr, mem, size );
                rf.ptr = mem;
                rf.max = mem + size;
                rc = interpret_menus( &rf, 0, main );
                UIK_MemSpFree( mem );
                }
            return( rc );
            }
        else return( cleanup_menu( 0, main ) );
        }
    return(1);
}

ULONG Parm_VerifMenu( ULONG defaut )
{
    return( verif_menu( defaut, 0 ) );
}
ULONG Parm_VerifMainMenu( ULONG defaut )
{
    return( verif_menu( defaut, 1 ) );
}
