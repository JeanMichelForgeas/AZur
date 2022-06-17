/****************************************************************
 *
 *      Project:   UIK
 *      Function:  objet Split
 *
 *      Created:   02/06/93     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikbase.h"
#include "uiki:uikglobal.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uikgfx.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objscreen.h"
#include "uiki:objwindow.h"

#include "lci:azur.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"
#include "lci:objsplit.h"


/****** Macros **************************************************/


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/

void ospl_SetImageTitle( struct AZObjSplit *spl );


/****** Statics *************************************************/

static UWORD chip ImageData[5][7][1] =
   {
   0x3800,    /* ..###........... */
   0x7c00,    /* .#####.......... */
   0x9200,    /* #..#..#......... */
   0x1000,    /* ...#............ */
   0x9200,    /* #..#..#......... */
   0x7c00,    /* .#####.......... */
   0x3800,    /* ..###........... */

   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */

   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */

   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */
   0x0000,    /* ................ */

   0x3800,    /* ..###........... */
   0x7c00,    /* .#####.......... */
   0x9200,    /* #..#..#......... */
   0x1000,    /* ...#............ */
   0x9200,    /* #..#..#......... */
   0x7c00,    /* .#####.......... */
   0x3800,    /* ..###........... */
   };
static struct UIKBitMap __far UBM_Image = { &ImageData[4][0][0], UIKBMAP_MASKED|UIKBMAP_CONTIG,
    7, 2,7, 0,4,0, &ImageData[0][0][0], &ImageData[1][0][0], &ImageData[2][0][0], &ImageData[3][0][0], 0, 0, 0, 0 };


static struct UIKBitMap * __far UIKBitMapTable[] = {
    &UBM_Image,
    0
    };

static UWORD __far Pattern[] = { 0x7777, 0xdddd };


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

static WORD but_top( struct AZObjView * vo )
{
    return( vo->Obj.Box.Top + vo->Obj.Box.Height - AGP.FObj->VI.HBorderHeight - 1 );
}

/*------- Object vectors ----------------------------------------*/

static void MoveBar_Start( struct AZObjSplit *spl )
{
  struct AZObjView *vo = (struct AZObjView *) spl->Obj.Parent;
  WORD my = spl->Obj.UIK->IntuiMsg->MouseY;
  WORD x = vo->Obj.Box.Left,
       y = but_top(vo) - 1,          // -1 pour ligne noire
       d = vo->Obj.rp->BitMap->Depth,
       w = vo->Obj.Box.Width,
       h = AGP.FObj->VI.HBorderHeight + 2;  // +2 pour ligne noire

    if (spl->Obj.Title)
        {
        if (spl->BarRPort = UIK_AllocBMapRPort( d, w, h, 0 ))
            {
            if (spl->SaveRPort = UIK_AllocBMapRPort( d, w, h, 0 ))
                {
                if (spl->WorkRPort = UIK_AllocBMapRPort( d, w, 2*h, 0 ))
                    {
                    UBYTE mask = spl->Obj.rp->Mask;

                    spl->Obj.rp->Mask = spl->Mask;
                    ClipBlit( spl->Obj.rp, x, y, spl->BarRPort, 0, 0, w, h, 0xc0 );
                    ClipBlit( spl->Obj.rp, x, y, spl->SaveRPort, 0, 0, w, h, 0xc0 );
                    spl->Obj.rp->Mask = mask;

                    spl->YOffset = my - y;
                    spl->OldY = y;
                    spl->Flags |= SPLF_MOVESTARTED;
                    }
                }
            }
        }
}

static void MoveBar( struct AZObjSplit *spl )
{
  struct AZObjFile *fo = AGP.FObj;

    if (spl->Flags & SPLF_MOVESTARTED)
        {
        struct AZObjView *vo = (struct AZObjView *) spl->Obj.Parent;
        struct AZObjZone *zo = vo->Zone;
        struct UIKObjWindow *wo = fo->WO;
        WORD my = spl->Obj.UIK->IntuiMsg->MouseY,
             x = vo->Obj.Box.Left,
             y = my - spl->YOffset,
             w = vo->Obj.Box.Width,
             h = fo->VI.HBorderHeight + 2, // +2 pour ligne noire
             diff = fo->VI.HBorderHeight + 1 - ((vo->Obj.Box.Top + vo->Obj.Box.Height) - (zo->Obj.Box.Top + zo->Obj.Box.Height)),
             topy = vo->Obj.Box.Top + fo->VI.MinHeight - fo->VI.HBorderHeight - 2, // -1 pour ligne noire,
             boty = wo->Obj.Box.Height - wo->BottomBorderHeight - fo->VI.HBorderHeight - 2; // -1 pour ligne noire

        if (fo->CmdLine) boty -= fo->CmdLine->Box.Height;

        if (y < topy) y = topy;
        if (y > boty) y = boty;

        if (y != spl->OldY)
            {
            WORD offs;
            UBYTE mask = spl->Obj.rp->Mask;

            spl->Obj.rp->Mask = spl->Mask;

            if ((offs = y - spl->OldY) > 0 && y < spl->OldY + h)
                {
                ClipBlit( spl->Obj.rp,    x, y,    spl->WorkRPort, 0, offs,      w, h, 0xc0 );
                ClipBlit( spl->SaveRPort, 0, 0,    spl->WorkRPort, 0, 0,         w, h, 0xc0 );
                if (diff)
                    ClipBlit( spl->WorkRPort, 0, offs,   spl->BarRPort, 0, 0,  w-fo->VI.VBorderWidth, diff, 0xc0 );
                ClipBlit( spl->WorkRPort, 0, offs, spl->SaveRPort, 0, 0,         w, h, 0xc0 );
                ClipBlit( spl->BarRPort,  0, 0,    spl->WorkRPort, 0, offs,      w, h, 0xc0 );
                ClipBlit( spl->WorkRPort, 0, 0,    spl->Obj.rp,    x, spl->OldY, w, h+offs, 0xc0 );
                }
            else if ((offs = spl->OldY - y) > 0 && y + h > spl->OldY)
                {
                ClipBlit( spl->Obj.rp,    x, y,    spl->WorkRPort, 0, 0,         w, h, 0xc0 );
                ClipBlit( spl->SaveRPort, 0, 0,    spl->WorkRPort, 0, offs,      w, h, 0xc0 );
                if (diff)
                    ClipBlit( spl->Obj.rp, x, y,   spl->BarRPort, 0, 0,  w-fo->VI.VBorderWidth, diff, 0xc0 );
                ClipBlit( spl->WorkRPort, 0, 0,    spl->SaveRPort, 0, 0,         w, h, 0xc0 );
                ClipBlit( spl->BarRPort,  0, 0,    spl->WorkRPort, 0, 0,         w, h, 0xc0 );
                ClipBlit( spl->WorkRPort, 0, 0,    spl->Obj.rp,    x, y,         w, h+offs, 0xc0 );
                }
            else{
                //WaitBOVP( &spl->Obj.w->WScreen->ViewPort );
                ClipBlit( spl->SaveRPort, 0, 0, spl->Obj.rp, x, spl->OldY, w, h, 0xc0 );
                ClipBlit( spl->Obj.rp, x, y, spl->SaveRPort, 0, 0, w, h, 0xc0 );
                ClipBlit( spl->BarRPort, 0, 0, spl->Obj.rp, x, y, w, h, 0xc0 );
                }
            spl->OldY = y;

            spl->Obj.rp->Mask = mask;
            }
        }
}

static void MoveBar_End( struct AZObjSplit *spl, ULONG cancel )
{
  struct AZObjFile *fo = AGP.FObj;

    if (spl->Flags & SPLF_MOVESTARTED)
        {
        struct AZObjView *nvo, *vo = (struct AZObjView *) spl->Obj.Parent;
        struct UIKObjWindow *wo = fo->WO;
        WORD x = vo->Obj.Box.Left,
             w = vo->Obj.Box.Width,
             h = fo->VI.HBorderHeight + 2, // +2 pour ligne noire
             Y = spl->OldY,
             maxy,
             WinBotY = wo->Obj.Box.Height - wo->BottomBorderHeight - fo->VI.HBorderHeight - 2, // -1 pour ligne noire
             WinMaxY,
             success = FALSE;

        if (fo->CmdLine) WinBotY -= fo->CmdLine->Box.Height;
        WinMaxY = WinBotY - fo->VI.MinHeight;

        spl->Flags &= ~SPLF_MOVESTARTED;

        //------ Ajout ou suppression de View?
        if (! cancel)
            {
            if (Y+1 < but_top(vo))
                //------ Remonte (resize en petit et ajout éventuel)
                {
                if (nvo = vo->NextView) //------ resize en grand celle du dessous
                    {
                    oview_ChangeView( vo, 0/*topchange*/, (Y + h) - vo->Obj.Box.Top );
                    oview_ChangeView( nvo, 1/*topchange*/, but_top(nvo) - Y - 1 );
                    success = TRUE;
                    }                   //------ ajoute
                else{
                    WORD height = vo->Obj.Box.Height;

                    if (Y > WinMaxY) Y = WinMaxY;

                    oview_ChangeView( vo, 0/*topchange*/, (Y+h) - vo->Obj.Box.Top );
                    if (nvo = (struct AZObjView *) ofi_AddView( fo, &fo->VI ))
                        {
                        if (UIK_Start( nvo )) success = TRUE;
                        else ofi_RemView( fo, nvo );
                        }
                    if (success == FALSE) oview_ChangeView( vo, 0/*topchange*/, height );
                    }
                }
            else if (Y+1 > but_top(vo))
                //------ Descend (resize en grand et suppression éventuelle)
                {
                while (TRUE)
                    {
                    if (nvo = vo->NextView)
                        {
                        if (Y+h >= but_top(nvo))
                            {
                            ofi_RemView( fo, nvo ); // supprime et relink les views restantes
                            }
                        else{ // recentre
                            maxy = but_top(nvo) - fo->VI.MinHeight - 1;
                            if (Y > maxy) Y = maxy;
                            break;
                            }
                        }
                    else{
                        Y = WinBotY;
                        break;
                        }
                    }

                oview_ChangeView( vo, 0/*topchange*/, (Y+h) - vo->Obj.Box.Top);
                if (nvo)
                    oview_ChangeView( nvo, 1/*topchange*/, but_top(nvo) - Y - 1 );
                success = TRUE;
                }
            }

        if (success == FALSE)   //------ restore affichage
            {
            UBYTE mask = spl->Obj.rp->Mask;

            spl->Obj.rp->Mask = spl->Mask;
            ClipBlit( spl->SaveRPort, 0, 0, spl->Obj.rp, x, Y, w, h, 0xc0 );
            spl->Obj.rp->Mask = mask;

            UIK_Refresh( spl, 1 );
            }

        ofi_ActualizeViewLimits( fo );
        }
    UIK_FreeBMapRPort( spl->WorkRPort ); spl->WorkRPort = 0;
    UIK_FreeBMapRPort( spl->SaveRPort ); spl->SaveRPort = 0;
    UIK_FreeBMapRPort( spl->BarRPort ); spl->BarRPort = 0;
}

static void MoveBar_EndMB( int nil, struct AZObjSplit *spl )
{
  struct IntuiMessage *im = spl->Obj.UIK->IntuiMsg;

    if (spl->Flags & SPLF_MOVESTARTED)
        {
        if (im->Code == SELECTUP)
            {
            MoveBar_End( spl, (im->Qualifier & IEQUALIFIER_RBUTTON) ? 1 : 0 );
            }
        else if (im->Code == MENUDOWN) // Ne marche pas : Intuition n'envoie pas de MOUSEBUTTON
            {                          // quand le gadget est déjà séletionné
            MoveBar_End( spl, 1 );
            }
        }
}

/*------- Object entries ----------------------------------------*/

static int GenericAdd( struct AZObjSplit *spl, struct TagItem *taglist)
{
    UIK_InvertBlackWhite( spl->Obj.UIK, UIKBitMapTable, 1 );

    spl->AZurProc = __builtin_getreg(12);
    spl->Obj.TitleFlags = TITLE_IMAGE;
    ospl_SetImageTitle( spl );

    spl->Gad.LeftEdge = spl->Obj.Box.Left;
    spl->Gad.TopEdge  = spl->Obj.Box.Top;
    spl->Gad.Width    = spl->Obj.Box.Width;
    spl->Gad.Height   = spl->Obj.Box.Height;
    spl->Gad.Flags      = GADGHNONE;
    spl->Gad.Activation = FOLLOWMOUSE | RELVERIFY | GADGIMMEDIATE;
    spl->Gad.GadgetType = BOOLGADGET;
    spl->Gad.UserData = (APTR) spl; /* le champ UserData doit toujours pointer */
                                    /* vers l'objet auquel appartient ce gadget */

    if (! (UIK_AddVector( spl, (void*)MoveBar_Start, IDCMP_GADGETDOWN, 0 ))) goto END_ERROR;
    if (! (UIK_AddVector( spl, (void*)MoveBar,       IDCMP_MOUSEMOVE,  0 ))) goto END_ERROR;
    if (! (UIK_AddVector( spl, (void*)MoveBar_End,   IDCMP_GADGETUP, 0 ))) goto END_ERROR;
    if (! (spl->MBHook = UIK_AddVector( AGP.FObj->WO, (void*)MoveBar_EndMB, IDCMP_MOUSEBUTTONS, (ULONG)spl ))) goto END_ERROR;

    spl->Mask = spl->Obj.sc ? ((struct UIKObjScreen *)spl->Obj.sc->UserData)->Mask : UIKBase->UIKPrefs->Mask;

   return(1);

 END_ERROR:
   return(0);
}

static int GenericStart( struct AZObjSplit *spl )
{
    UIK_StdGadStart( spl );
    return(1);
}

static void GenericStop( struct AZObjSplit *spl )
{
    UIK_StdGadStop( spl );
}

static void GenericRemove( struct AZObjSplit *spl )
{
    UIK_RemoveVector( AGP.FObj->WO, spl->MBHook );
}

static void GenericRefresh( struct AZObjSplit *spl )
{
    UIK_StdGadRefresh( spl );
}

static void GenericResize( struct AZObjSplit *spl )
{
    UIK_StdGadResize( spl );
    ospl_SetImageTitle( spl );
}

static void __asm GenericAct( register __a1 struct AZObjSplit *spl )
{
    __builtin_putreg( 12, spl->AZurProc ); // A4 pointe sur la structure des variables
    UIK_StdGadAct( spl );
}

static void __asm GenericInact( register __a1 struct AZObjSplit *spl )
{
    __builtin_putreg( 12, spl->AZurProc ); // A4 pointe sur la structure des variables
    UIK_StdGadInact( spl );
}

static void __asm GenericMove( register __a1 struct AZObjSplit *spl, register __a3 struct IntuiMessage *imsg )
{
    /*
    __builtin_putreg( 12, spl->AZurProc ); // A4 pointe sur la structure des variables
    UIK_StdGadMove( spl, imsg );
    */
}

/*------ Objects functions --------------------------------------*/

void ospl_SetImageTitle( struct AZObjSplit *spl )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZObjView *vo = (struct AZObjView *) spl->Obj.Parent;
  struct UIKObjWindow *wo = fo->WO;
  WORD WinBotY = wo->Obj.Box.Height - wo->BottomBorderHeight - fo->VI.HBorderHeight - 1;

    if (fo->CmdLine) WinBotY -= fo->CmdLine->Box.Height;

    if ((but_top(vo) < WinBotY) || (vo->Obj.Box.Height >= (2 * fo->VI.MinHeight)))
        {
        spl->Obj.Title = (ULONG) &UBM_Image;
        spl->Obj.ActBoxPen = UIKCOL_WHITE;
        spl->Obj.Pattern = 0;
        spl->Obj.PatternPower = 0;
        }
    else{
        spl->Obj.Title = 0;
        spl->Obj.ActBoxPen = UIKCOL_GREY;
        spl->Obj.Pattern = (ULONG) Pattern;
        spl->Obj.PatternPower = 1;
        }
}

/*------ Germ ---------------------------------------------------*/

static struct TagItem __far GermTagList[] =
{
   { UIKTAG_GEN_ActInactPens,   UIKCOLS(UIKCOL_GREY,UIKCOL_GREY,UIKCOL_WHITE,UIKCOL_BLACK) },
   { TAG_END }
};

static struct UIKObjGerm __far Germ = {
    "",
    GenericAdd,         /*int     (*AddVector)(); */
    GenericStart,       /*int     (*StartVector)(); */
    GenericStop,        /*void    (*StopVector)(); */
    GenericRemove,      /*void    (*RemoveVector)(); */
    GenericRefresh,     /*void    (*RefreshVector)(); */
    GenericResize,      /*void    (*ResizeVector)(); */
    GenericAct,         /*void    (*ExceptActVector)(); */
    GenericInact,       /*void    (*ExceptInaVector)(); */
    GenericMove,        /*void    (*ExceptMovVector)(); */
    NULL,               /*void    (*ExceptKeyVector)(); */
    NULL,               /*int     (*StartAfterVector)(); */
    NULL,               /*int     (*SelectVector)(); */
    NULL,               /*ULONG   *ExtentedVectors; */
    GermTagList,        /*struct TagItem *GermTagList; */
    8,                  /*UWORD   MinWidth; */
    4,                  /*UWORD   MinHeight; */
    SETL(10,10), /*ULONG DefaultWidthHeight; */
    0,                 /*UWORD   ObjId; */
    sizeof(struct AZObjSplit),   /*UWORD   ObjSize; */
    BOX_DISPLAYABLE|BOX_INNERZONE | BOX_NOPUSH | BOX_OVER | BOX_FILLED | BOX_SEMAPHORE | BOX_TRANSLATELEFTTOP,
    };

struct UIKObjRegGerm __far ObjSplitRegGerm = { &Germ, 0, 0 ,0 };
