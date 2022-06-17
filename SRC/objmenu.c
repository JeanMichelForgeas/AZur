/****************************************************************
 *
 *      Project:   UIK
 *      Function:  objet Menu
 *
 *      Created:   25/02/92     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikbase.h"
#include "uiki:uikglobal.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objscreen.h"
#include "uiki:objwindow.h"

#include <libraries/locale.h>
#include <clib/locale_protos.h>
#include <pragmas/locale_pragmas.h>

#include "lci:objmenu.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#define MOBJ_TITLESEP   20
#define MOBJ_TITLESTART 10
#define MOBJ_TITLEEXTRA -1


/****** Macros **************************************************/


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/


/****** Statics *************************************************/

static int StartMenu( struct AZObjMenu *mobj );
static void StopMenu( struct AZObjMenu *mobj );


/****************************************************************
 *
 *      Utilities
 *
 ****************************************************************/

static ULONG max_item_width( struct AZObjMenu *mobj, struct AZMenuEntry *ume, struct TextFont *font, UBYTE depth, ULONG hires, UWORD checkwidth )
{
  struct UIKObjWindow *wobj = (struct UIKObjWindow *) mobj->Obj.w->UserData;
  struct DrawInfo *di = wobj->DrawInfo;
  ULONG maxwidth=0, titlewidth, key=0, contchar=0, keywidth=0, stdcomwidth, comwidth, comkeylen;

    stdcomwidth = (hires ? COMMWIDTH : LOWCOMMWIDTH);
    if (di && (wobj->Obj.w->Flags & WFLG_NEWLOOKMENUS) && ((struct Library *)GfxBase)->lib_Version >= 39)
        if (di->dri_AmigaKey)
            stdcomwidth = di->dri_AmigaKey->Width;

    ume++;
    while ((ume->Title) && (ume->Depth >= depth))
        {
        if (depth == 1 && ume->Depth == 2) contchar = 1;

        if (((ULONG)ume->Title) != ITEM_SEPARATION && ((ULONG)ume->Title) != ITEM_SYSSEPARATION && ume->Depth == depth)
            {
            /*------ 2 choses séparées : max des titres ------*/
            titlewidth = UIK_StrWidth( font, ume->Title, StrLen(ume->Title) );
            if (ume->Flags & CHECKIT) titlewidth += checkwidth;
            if (titlewidth > maxwidth) maxwidth = titlewidth;

            /*------ 2 choses séparées : max des raccourcis ------*/
            if (ume->ComKey)
                {
                if ((comkeylen = StrLen(ume->ComKey)) == 1)
                    {
                    comwidth = stdcomwidth;
                    }
                else{
                    comwidth = 0;
                    ume->Flags |= AZMEF_CUSTOMSHORTCUT;
                    }
                ume->SCWidth = UIK_StrWidth( font, ume->ComKey, comkeylen );
                key = comwidth + ume->SCWidth;
                if (key > keywidth) keywidth = key;
                }
            }
        ume++;
        }

    /*------ Le max des raccourcis doit être comparé avec la largeur de 'contchar' ------*/
    if (contchar)
        {
        key = UIK_CharWidth( font, '»' );
        if (key > keywidth) keywidth = key;
        }
    if (keywidth) keywidth += UIK_CharWidth( font, '8' );
    keywidth += 4;
    return( maxwidth + keywidth );
}

static ULONG max_item_height( struct AZObjMenu *mobj, struct AZMenuEntry *ume, UWORD height, UBYTE depth )
{
  ULONG maxheight=0;

    ume++;
    while ((ume->Title) && (ume->Depth >= depth))
        {
        if (ume->Depth == depth)
            {
            if (((ULONG)ume->Title) == ITEM_SEPARATION) maxheight += ((height-2)/3+1);  // (height-2)/2+1;
            else if (((ULONG)ume->Title) == ITEM_SYSSEPARATION) maxheight += height-2;
            else maxheight += height;
            }
        ume++;
        }
    return( maxheight );
}

static void arrange_menus( struct AZObjMenu *mobj, struct AZMenuEntry *ume, struct TextFont *font, UWORD scwidth )
{
  UWORD num=0, twidth=0, extwidth;
  struct Screen *sc = mobj->Obj.w->WScreen;
  UBYTE cw = (sc->BarHBorder - sc->BarVBorder) * 2; // calcul à cause bug Intuition

    if (mobj->TitleExtra == -1)
        mobj->TitleExtra = cw;

    while (ume->Title)
        {
        if (ume->Depth == 0)
            {
            num++;
            twidth += UIK_StrWidth( font, ume->Title, StrLen(ume->Title) );
            }
        ume++;
        }

    if (num > 1)
        {
        extwidth = mobj->TitleStart + ((num-1) * mobj->TitleSep) + (num * mobj->TitleExtra);
        if (twidth + extwidth > scwidth)
            {
            mobj->TitleStart = 2;
            mobj->TitleExtra = cw;
            extwidth = mobj->TitleStart + (num * mobj->TitleExtra) + 2;
            if (twidth + extwidth < scwidth)
                mobj->TitleSep = (scwidth - (twidth + extwidth)) / (num - 1);
            else
                mobj->TitleSep = 0;
            }
        }
}

static void menu_build( struct AZObjMenu *mobj )
{
  struct AZMenuEntry *ume = mobj->MenuEntry;
  struct Menu *menu=0, *oldmenu=0;
  struct MenuItem *item=0, *olditem;
  struct MenuItem *subitem=0, *oldsubitem=0;
  UBYTE *ptr = (UBYTE *) mobj->Menu;
  UWORD menuleft, itemtop, subitemtop, maxitemwidth, maxsubitemwidth;
  UWORD itemleft, itemmax, subitemleft, subitemmax, maxsubitemheight, subitemY;
  struct IntuiText *itext, *contitext, *sctext;
  struct TextFont *font;
  UWORD fontheight, minheight;
  struct Screen *sc;
  ULONG mode, hires;
  struct UIKObjWindow *wobj = (struct UIKObjWindow *) mobj->Obj.w->UserData;
  struct DrawInfo *di = wobj->DrawInfo;
  UBYTE fpen, bpen;
  struct Image *im;
  UWORD checkwidth;

    sc = mobj->Obj.w->WScreen;

    if ((mobj->Obj.Flags & BOX_OPENEDFONT) && (font = mobj->Obj.ObjExt->Font))
        {
        UIK_Font2TTA( font, &mobj->TTAttr );
        UIK_Font2TTA( font, &mobj->BoldTTAttr );
        }
    else{
        CopyMem( sc->Font, &mobj->TTAttr, sizeof(struct TextAttr) );
        CopyMem( sc->Font, &mobj->BoldTTAttr, sizeof(struct TextAttr) );
        font = sc->RastPort.Font;
        }
    mobj->BoldTTAttr.tta_Style |= FSF_BOLD;
    fontheight = font->tf_YSize;
    minheight = fontheight < 8 ? 8 : fontheight;

    arrange_menus( mobj, ume, font, sc->Width );
    menuleft = mobj->TitleStart;

    fpen = mobj->Obj.ActTitlePen; bpen = mobj->Obj.ActBoxPen;
    if (di && (wobj->Obj.w->Flags & WFLG_NEWLOOKMENUS) && ((struct Library *)GfxBase)->lib_Version >= 39)
        { fpen = di->dri_Pens[BARDETAILPEN]; bpen = di->dri_Pens[BARBLOCKPEN]; }

    mode = INVALID_ID;
    if (UIK_IsSystemVersion2()) mode = GetVPModeID( &sc->ViewPort );
    if (mode == INVALID_ID) mode = sc->ViewPort.Modes;
    hires = (mode & HIRES ? 1 : 0);

    checkwidth = (hires ? CHECKWIDTH : LOWCHECKWIDTH);
    if (di && (wobj->Obj.w->Flags & WFLG_NEWLOOKMENUS) && ((struct Library *)GfxBase)->lib_Version >= 39)
        if (di->dri_CheckMark)
            checkwidth = di->dri_CheckMark->Width;

    while (ume->Title)  /*-----Menu */
        {
        switch (ume->Depth)
            {
            case 0: /*-----Menu */
                {
                ume->IntuiData = (APTR) (menu = (struct Menu *) ptr);
                if (oldmenu) oldmenu->NextMenu = menu;

                menu->LeftEdge  = menuleft;
                menu->Width     = mobj->TitleExtra + UIK_StrWidth( font, ume->Title, StrLen(ume->Title) );
                menu->Height    = fontheight;
                menu->Flags     = ume->Flags;
                menu->MenuName  = ume->Title;

                olditem = (struct MenuItem *) &menu->FirstItem;
                oldmenu = menu;
                itemtop = 0;
                maxitemwidth = max_item_width( mobj, ume, font, 1, hires, checkwidth );
                itemmax = menu->LeftEdge + maxitemwidth + 6 - (hires ? 0 : 3);
                itemleft = (itemmax < sc->Width) ? 0 : sc->Width - itemmax - 1 - 2;
                menuleft += (menu->Width + mobj->TitleSep);
                ptr += sizeof(struct Menu);

                /*-----pour » */
                contitext = (struct IntuiText *) ptr;
                contitext->FrontPen  = fpen;
                contitext->BackPen   = bpen;
                contitext->DrawMode  = JAM1;
                contitext->LeftEdge  = maxitemwidth - (UIK_CharWidth( font, '»' ) + 3);
                contitext->TopEdge   = 1;
                contitext->ITextFont = &mobj->TTAttr;
                contitext->IText     = "»";
                ptr += sizeof(struct IntuiText);
                }
                break;
            case 1: /*-----Item */
                {
                if (((ULONG)ume->Title) == ITEM_SEPARATION)
                    {
                    itemtop += ((minheight / 3) + 1);
                    break;
                    }
                else if (((ULONG)ume->Title) == ITEM_SYSSEPARATION)
                    {
                    im = (struct Image *) ptr;
                    im->LeftEdge   = 0;
                    im->TopEdge    = minheight/2 - 1;
                    im->Width      = maxitemwidth;
                    im->Height     = 2;
                    im->Depth      = 0;
                    im->ImageData  = NULL;
                    im->PlanePick  = 0;
                    im->PlaneOnOff = fpen;
                    /*
                    if (wobj->DrawInfo)
                        if (wobj->DrawInfo->dri_Version >= 2)
                            im->PlaneOnOff = wobj->DrawInfo->dri_Pens[BARDETAILPEN];
                    */
                    ptr += sizeof(struct Image);

                    /*-------*/
                    ume->IntuiData = (APTR) (item = (struct MenuItem *) ptr);
                    olditem->NextItem = item;

                    item->TopEdge       = itemtop;
                    item->LeftEdge      = itemleft;
                    item->Width         = maxitemwidth;
                    item->Height        = minheight;
                    item->ItemFill      = im;
                    item->Flags         = ume->Flags | HIGHNONE;
                    }
                else{
                    itext = (struct IntuiText *) ptr;
                    itext->FrontPen  = fpen;
                    itext->BackPen   = bpen;
                    itext->DrawMode  = JAM1;
                    itext->LeftEdge  = 2 + ((ume->Flags & CHECKIT) ? checkwidth : 0);
                    itext->TopEdge   = 1;
                    itext->ITextFont = &mobj->TTAttr;
                    itext->IText     = ume->Title;
                    ptr += sizeof(struct IntuiText);

                    if (ume->Flags & AZMEF_CUSTOMSHORTCUT)
                        {
                        sctext = (struct IntuiText *) ptr;
                        itext->NextText   = sctext;
                        sctext->FrontPen  = fpen;
                        sctext->BackPen   = bpen;
                        sctext->DrawMode  = JAM1;
                        sctext->LeftEdge  = maxitemwidth - (ume->SCWidth + 3);
                        sctext->TopEdge   = 1;
                        sctext->ITextFont = &mobj->BoldTTAttr;
                        sctext->IText     = ume->ComKey;
                        }
                    ptr += sizeof(struct IntuiText);

                    /*-------*/
                    ume->IntuiData = (APTR) (item = (struct MenuItem *) ptr);
                    olditem->NextItem = item;

                    item->TopEdge       = itemtop;
                    item->LeftEdge      = itemleft;
                    item->Width         = maxitemwidth;
                    item->Height        = minheight + 2;
                    item->ItemFill      = itext;
                    item->MutualExclude = ume->MutualExclude;
                    item->Flags         = ume->Flags | ITEMTEXT|HIGHCOMP;
                    if (ume->ComKey && !(ume->Flags & AZMEF_CUSTOMSHORTCUT))
                        {
                        item->Flags |= COMMSEQ;
                        item->Command = *ume->ComKey;
                        }
                    }

                oldsubitem = (struct MenuItem *) &item->SubItem;
                olditem = item;
                subitemtop = (item->Height * 3) / 4;
                maxsubitemheight = max_item_height( mobj, ume, item->Height, 2 );
                subitemY = fontheight+3 + itemtop + subitemtop + maxsubitemheight + (hires ? 1 : 5);
                if (subitemY >= sc->Height) subitemtop += (sc->Height - subitemY - 1);

                maxsubitemwidth = max_item_width( mobj, ume, font, 2, hires, checkwidth );
                subitemleft = (maxitemwidth * 3)  / 4;
                subitemmax = menu->LeftEdge + itemleft + subitemleft + maxsubitemwidth + 6 - (hires ? 0 : 3);
                if (subitemmax >= sc->Width) subitemleft += sc->Width - subitemmax - 1 - 2;

                itemtop += item->Height;
                ptr += sizeof(struct MenuItem);
                }
                break;
            case 2: /*-----SubItem */
                {
                if (((ULONG)ume->Title) == ITEM_SEPARATION)
                    {
                    subitemtop += ((minheight / 3) + 1);
                    break;
                    }
                else if (((ULONG)ume->Title) == ITEM_SYSSEPARATION)
                    {
                    im = (struct Image *) ptr;
                    im->LeftEdge   = 0;
                    im->TopEdge    = minheight/2 - 1;
                    im->Width      = maxsubitemwidth;
                    im->Height     = 2;
                    im->Depth      = 0;
                    im->ImageData  = NULL;
                    im->PlanePick  = 0;
                    im->PlaneOnOff = fpen;
                    /*
                    if (wobj->DrawInfo)
                        if (wobj->DrawInfo->dri_Version >= 2)
                            im->PlaneOnOff = wobj->DrawInfo->dri_Pens[BARDETAILPEN];
                    */
                    ptr += sizeof(struct Image);

                    /*-------*/
                    ume->IntuiData = (APTR) (subitem = (struct MenuItem *) ptr);
                    oldsubitem->NextItem = subitem;

                    subitem->LeftEdge      = subitemleft;
                    subitem->TopEdge       = subitemtop;
                    subitem->Width         = maxsubitemwidth;
                    subitem->Height        = minheight;
                    subitem->ItemFill      = im;
                    subitem->Flags         = ume->Flags | HIGHNONE;
                    }
                else{
                    /*-------*/
                    ((struct IntuiText *)item->ItemFill)->NextText = contitext;

                    /*-------*/
                    itext = (struct IntuiText *) ptr;
                    itext->FrontPen  = fpen;
                    itext->BackPen   = bpen;
                    itext->DrawMode  = JAM1;
                    itext->LeftEdge  = 2 + ((ume->Flags & CHECKIT) ? checkwidth : 0);
                    itext->TopEdge   = 1;
                    itext->ITextFont = &mobj->TTAttr;
                    itext->IText     = ume->Title;
                    ptr += sizeof(struct IntuiText);

                    if (ume->Flags & AZMEF_CUSTOMSHORTCUT)
                        {
                        sctext = (struct IntuiText *) ptr;
                        itext->NextText   = sctext;
                        sctext->FrontPen  = fpen;
                        sctext->BackPen   = bpen;
                        sctext->DrawMode  = JAM1;
                        sctext->LeftEdge  = maxsubitemwidth - (ume->SCWidth + 3);
                        sctext->TopEdge   = 1;
                        sctext->ITextFont = &mobj->BoldTTAttr;
                        sctext->IText     = ume->ComKey;
                        }
                    ptr += sizeof(struct IntuiText);

                    /*-------*/
                    ume->IntuiData = (APTR) (subitem = (struct MenuItem *) ptr);
                    oldsubitem->NextItem = subitem;

                    subitem->LeftEdge      = subitemleft;
                    subitem->TopEdge       = subitemtop;
                    subitem->Width         = maxsubitemwidth;
                    subitem->Height        = minheight + 2;
                    subitem->ItemFill      = itext;
                    subitem->MutualExclude = ume->MutualExclude;
                    subitem->Flags         = ume->Flags | ITEMTEXT|HIGHCOMP;
                    if (ume->ComKey && !(ume->Flags & AZMEF_CUSTOMSHORTCUT))
                        {
                        subitem->Flags |= COMMSEQ;
                        subitem->Command = *ume->ComKey;
                        }
                    }

                oldsubitem = subitem;
                subitemtop += subitem->Height;
                ptr += sizeof(struct MenuItem);
                }
                break;
            }
        ume++;
        }
}

static ULONG menu_size( struct AZObjMenu *mobj )
{
  struct AZMenuEntry *ume;
  ULONG size;

    for (size=0, ume=mobj->MenuEntry; ume->Title; ume++)
        if (((ULONG)ume->Title) != ITEM_SEPARATION)
            size += (ume->Depth ?
                        sizeof(struct MenuItem) + 2 * sizeof(struct IntuiText) :
                        sizeof(struct Menu) + sizeof(struct IntuiText));
    return( size );
}

/*
static void printmenu( struct AZObjMenu *mobj )
{
  struct Menu *menu;
  struct MenuItem *menuitem;
  struct MenuItem *subitem;

    menu = mobj->Menu;
    while (menu)
        {
        fprintf( Output(), "MENU %lx\n", menu );
        fprintf( Output(), " NextMenu   = %lx\n", menu->NextMenu );
        fprintf( Output(), " L,T,W,H    = %ld %ld %ld %ld\n", menu->LeftEdge, menu->TopEdge, menu->Width, menu->Height );
        fprintf( Output(), " Flags      = %lx\n", menu->Flags );
        fprintf( Output(), " MenuName   = %ls\n", menu->MenuName );
        fprintf( Output(), " FirstItem  = %lx\n", menu->FirstItem );
        menuitem = menu->FirstItem;
        while (menuitem)
            {
            fprintf( Output(), "  ITEM %lx\n", menuitem );
            fprintf( Output(), "    NextItem   = %lx\n", menuitem->NextItem );
            fprintf( Output(), "    L,T,W,H    = %ld %ld %ld %ld\n", menuitem->LeftEdge, menuitem->TopEdge, menuitem->Width, menuitem->Height );
            fprintf( Output(), "    Flags      = %lx\n", menuitem->Flags );
            fprintf( Output(), "    Texte      = %ls\n", ((struct IntuiText *)menuitem->ItemFill)->IText );
            fprintf( Output(), "    SubItem    = %lx\n", menuitem->SubItem );
            subitem = menuitem->SubItem;
            while (subitem)
                {
                fprintf( Output(), "    SUBITEM %lx\n", subitem );
                fprintf( Output(), "      NextItem   = %lx\n", subitem->NextItem );
                fprintf( Output(), "      L,T,W,H    = %ld %ld %ld %ld\n", subitem->LeftEdge, subitem->TopEdge, subitem->Width, subitem->Height );
                fprintf( Output(), "      Flags      = %lx\n", subitem->Flags );
                fprintf( Output(), "      Texte      = %ls\n", ((struct IntuiText *)subitem->ItemFill)->IText );
                if (SetSignal( 0, SIGBREAKF_CTRL_C ) == SIGBREAKF_CTRL_C) return;
                subitem = subitem->NextItem;
                }
            if (SetSignal( 0, SIGBREAKF_CTRL_C ) == SIGBREAKF_CTRL_C) return;
            menuitem = menuitem->NextItem;
            }
        if (SetSignal( 0, SIGBREAKF_CTRL_C ) == SIGBREAKF_CTRL_C) return;
        menu = menu->NextMenu;
        }
}
*/

/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

static void DoMenu( struct AZObjMenu *mo, struct UIKObjWindow *wo, struct MenuItem *item, UWORD code )
{
  UWORD menunum, itemnum, subnum, i;
  struct AZMenuEntry *ume = mo->MenuEntry;

    menunum = MENUNUM( code );
    itemnum = ITEMNUM( code );
    subnum  = SUBNUM( code );

    for (i=0; i < menunum; )
        { ume++; if (ume->Depth == 0) i++; }
    if (itemnum != NOITEM)
        for (ume++, i=0; i < itemnum; )
            { ume++; if (((ULONG)ume->Title) != ITEM_SEPARATION /*&& ((ULONG)ume->Title) != ITEM_SYSSEPARATION*/ && ume->Depth == 1) i++; }
    if (subnum != NOSUB)
        for (ume++, i=0; i < subnum; )
            { ume++; if (((ULONG)ume->Title) != ITEM_SEPARATION /*&& ((ULONG)ume->Title) != ITEM_SYSSEPARATION*/ && ume->Depth == 2) i++; }

    if (item->Flags & CHECKED) ume->Flags |= CHECKED; else ume->Flags &= ~CHECKED;
    if (ume->Func) (*ume->Func)( mo, ume, item, menunum, itemnum, subnum );
}

static void MenuFunc( struct AZObjMenu *mo, struct UIKObjWindow *wo, ULONG class, UWORD code )
{
    if (class == IDCMP_MENUPICK)
        {
        while (code != MENUNULL && mo->Obj.Status == UIKS_STARTED)
            {
            struct MenuItem *item;
            UWORD verif_life = mo->LifeId;
            item = (struct MenuItem *) ItemAddress( mo->Menu, code );
            DoMenu( mo, wo, item, code );
            if (mo->Obj.Status != UIKS_STARTED) return; /* plus de menu: stopped ou removing !!! */
            if (mo->LifeId != verif_life) return;  /* menu changé */
            code = item->NextSelect;
            }
        }
}

/*
static ULONG alloc_string( struct AZObjMenu *mobj, struct AZMenuEntry *ume )
{
    if ((ume->Flags & MENUTEXTADDR) && !(mobj->Flags & TITLETEXTSINGLE))
        {
        UBYTE *ptr;
        if (! (ptr = UIK_nl_AllocNode( &mobj->Strings, -1, StrLen( (UBYTE*) ume->Title ), 0 ))) return(0);
        StrCpy( ptr, (UBYTE*) ume->Title );
        ume->Title = (ULONG) ptr;
        }
    return(1);
}

static void free_string( struct AZObjMenu *mobj, struct AZMenuEntry *ume )
{
    if ((ume->Flags & MENUTEXTADDR) && !(mobj->Flags & TITLETEXTSINGLE))
        {
        LONG line;
        if ((line = UIK_nl_FindNode( &mobj->Strings, (UBYTE *) ume->Title )) != -1)
            {
            UIK_nl_FreeNodes( &mobj->Strings, line, line );
            ume->Title = 0;
            ume->Flags &= ~MENUTEXTADDR;
            }
        }
}
*/

/*
/****************************************************************
 *
 *      Methodes (pas à tenir compte de la compatibilité)
 *
 ****************************************************************/

static ULONG pos2offset( struct AZObjMenu *mobj, ULONG pos )
{
  ULONG max;
  struct AZMenuEntry *m;

    for (max=0, m=mobj->MenuEntry; m->Title; m++) max++;
    if (pos == -1 || pos > max) pos = max;
    return( sizeof(struct AZMenuEntry) * pos );
}

static int me_AddEntry( struct AZObjMenu *mobj, struct AZMenuEntry *ume, ULONG pos )
{
  ULONG offset = pos2offset( mobj, pos );

    //------ alloc string
    if (! alloc_string( mobj, ume )) return(0);

    //------ ajout
    if (BufPasteS( (UBYTE*)ume, sizeof(struct AZMenuEntry), (UBYTE**)&mobj->MenuEntry, offset ))
        {
        mobj->OtherMenu = mobj->EnglishMenu = mobj->MenuEntry;
        if (mobj->Obj.Status == UIKS_STARTED)
            {
            StopMenu( mobj );
            return( StartMenu( mobj ) );
            }
        else return(1);
        }
    return(0);
}

static void me_RemoveEntry( struct AZObjMenu *mobj, ULONG pos )
{
  ULONG max, offset, started;
  struct AZMenuEntry *m;

    //------ pos
    for (max=0, m=mobj->MenuEntry; m->Title; m++) max++;
    if (pos == -1 || pos > max) pos = max;
    if (max == 0 || pos >= max+1) return;
    offset = sizeof(struct AZMenuEntry) * pos;

    //-------------------------------------------
    started = (mobj->Obj.Status == UIKS_STARTED) ? 1 : 0;
    if (started) ClearMenuStrip( mobj->Obj.w );

    //------ free string
    free_string( mobj, mobj->MenuEntry + pos );

    //------ remove
    BufCutS( (UBYTE**)&mobj->MenuEntry, offset, sizeof(struct AZMenuEntry) );
    mobj->OtherMenu = mobj->EnglishMenu = mobj->MenuEntry;
    if (mobj->Obj.Status == UIKS_STARTED)
        {
        StopMenu( mobj );
        StartMenu( mobj );
        }

    if (started) SetMenuStrip( mobj->Obj.w, mobj->Menu );
}

static void me_EnableEntry( struct AZObjMenu *mobj, ULONG pos, struct AZMenuEntry *ume )
{
  ULONG offset;

    if (pos != -1)
        {
        offset = pos2offset( mobj, pos );
        ume = (struct AZMenuEntry *) (((UBYTE *)mobj->MenuEntry) + offset);
        }
    if (ume->Depth == 0) ume->Flags |= MENUENABLED;
    else ume->Flags |= ITEMENABLED;

    if (mobj->Obj.Status == UIKS_STARTED)
        {
        struct MenuItem *it = ume->IntuiData;
        ClearMenuStrip( mobj->Obj.w );

        if (ume->Depth == 0) it->Flags |= MENUENABLED;
        else it->Flags |= ITEMENABLED;

        SetMenuStrip( mobj->Obj.w, mobj->Menu );
        }
}

static void me_DisableEntry( struct AZObjMenu *mobj, ULONG pos, struct AZMenuEntry *ume )
{
  ULONG offset;

    if (pos != -1)
        {
        offset = pos2offset( mobj, pos );
        ume = (struct AZMenuEntry *) (((UBYTE *)mobj->MenuEntry) + offset);
        }
    if (ume->Depth == 0) ume->Flags &= ~MENUENABLED;
    else ume->Flags &= ~ITEMENABLED;

    if (mobj->Obj.Status == UIKS_STARTED)
        {
        struct MenuItem *it = ume->IntuiData;
        ClearMenuStrip( mobj->Obj.w );

        if (ume->Depth == 0) it->Flags &= ~MENUENABLED;
        else it->Flags &= ~ITEMENABLED;

        SetMenuStrip( mobj->Obj.w, mobj->Menu );
        }
}

static ULONG me_GetPos( struct AZObjMenu *mobj, struct AZMenuEntry *ume )
{
    return( (ULONG)(((ULONG)ume - (ULONG)mobj->MenuEntry)) / sizeof(struct AZMenuEntry) );
}

static ULONG me_GetEntry( struct AZObjMenu *mobj, ULONG pos )
{
  ULONG max;
  struct AZMenuEntry *m;

    for (max=0, m=mobj->MenuEntry; m->Title; m++) max++;
    if (pos >= max) return(0);
    return( ((ULONG)mobj->MenuEntry) + (sizeof(struct AZMenuEntry) * pos) );
}

static ULONG __far FuncTable[] =
    {
    (ULONG) me_AddEntry,
    (ULONG) me_RemoveEntry,
    (ULONG) me_EnableEntry,
    (ULONG) me_DisableEntry,
    (ULONG) me_GetPos,
    (ULONG) me_GetEntry,
    };
*/

/****************************************************************
 *
 *      Init/Cleanup
 *
 ****************************************************************/

static int StartMenu( struct AZObjMenu *mobj )
{
  ULONG menusize;
  struct UIKObjWindow *wo;

    wo = (struct UIKObjWindow *) mobj->Obj.w->UserData;
    if (wo->WMenu) return(0); /* déjà un menu sur cette fenêtre*/

    menusize = menu_size( mobj );
    if (! (mobj->Menu = (struct Menu *) UIK_MemSpAlloc( menusize, MEMF_ANY | MEMF_CLEAR ))) goto END_ERROR;
    menu_build( mobj );
    /* printmenu( mobj ); */

    if (! SetMenuStrip( mobj->Obj.w, mobj->Menu )) goto END_ERROR;
    mobj->Flags |= MENUSTRIP_SET;
    mobj->Obj.w->Flags &= ~RMBTRAP;
    wo->WMenu = mobj;
    ((struct AZObjMenu *)wo->WMenu)->MenuFunc = MenuFunc;
    return(1);

  END_ERROR:
    return(0);
}

static void StopMenu( struct AZObjMenu *mobj )
{
    if (mobj->Menu)
        {
        struct UIKObjWindow *wo;

        mobj->Obj.w->Flags |= RMBTRAP;
        if (mobj->Flags & MENUSTRIP_SET)
            {
            ClearMenuStrip( mobj->Obj.w );
            mobj->Flags &= ~MENUSTRIP_SET;
            mobj->LifeId++;
            wo = (struct UIKObjWindow *) mobj->Obj.w->UserData;
            if (wo->WMenu == mobj) wo->WMenu = 0;
            }
        UIK_MemSpFree( mobj->Menu ); mobj->Menu = 0;
        }
}

static int AddMenu( struct AZObjMenu *mobj, struct TagItem *taglist )
{
    mobj->TitleSep   = MOBJ_TITLESEP;
    mobj->TitleStart = MOBJ_TITLESTART;
    mobj->TitleExtra = MOBJ_TITLEEXTRA;
    *((ULONG*)&mobj->Obj.ActBoxPen) = B2L(1,1,2,2);
    if (! (mobj->MenuEntry = (struct AZMenuEntry *) UIK_GetTagData( UIKTAG_Menu_Entry, 0, taglist ))) return(0);
    return(1);
}

static struct UIKObjGerm __far Germ = {
    "",
    AddMenu,            /*int     (*AddVector)(); */
    StartMenu,          /*int     (*StartVector)(); */
    StopMenu,           /*void    (*StopVector)(); */
    NULL, /*RemoveMenu, */ /*void    (*RemoveVector)(); */
    NULL,               /*void    (*RefreshVector)(); */
    NULL,               /*void    (*ResizeVector)(); */
    NULL,               /*void    (*ExceptActVector)(); */
    NULL,               /*void    (*ExceptInaVector)(); */
    NULL,               /*void    (*ExceptMovVector)(); */
    NULL,               /*void    (*ExceptKeyVector)(); */
    NULL,               /*int     (*StartAfterVector)(); */
    NULL,               /*int     (*SelectVector)(); */
    NULL, /*FuncTable,*/  /*ULONG   *ExtentedVectors; */
    NULL, /*GermTagList,*/ /*struct TagItem *GermTagList; */
    0,                  /*UWORD   MinWidth; */
    0,                  /*UWORD   MinHeight; */
    0,                  /*ULONG   DefaultWidthHeight; */
    0,                  /*UWORD   ObjId; */
    sizeof(struct AZObjMenu),   /*UWORD   ObjSize; */
    0, /* flags par defaut */
    };

struct UIKObjRegGerm __far ObjMenuRegGerm = { &Germ, 0, 0 ,0 };
