/****************************************************************
 *
 *      Project:    AZCharTable
 *      Function:   Fenêtre table des caractères
 *
 *      Created:    27/11/94    Jean-Michel Forgeas
 *
 *      Modified:   (Denis BERNARD) ajout d'options pour le choix
 *                  des couleurs
 *
 *      Modified:   (Jean-Michel Forgeas) ajout des chiffres héxa
 *                  de 0 à F en ligne et en colonne autour de la
 *                  table
 *
 ****************************************************************/

/****************************************************************
 *
 * L'objet a maintenant la template suivante:
 *
 * "FP=FILLPEN/K/N,TP=TEXTPEN/K/N,BP=BACKPEN/K/N,TITLE/F"
 *
 * FILLPEN: Couleur (entre 0 et 3) du fond de la fenêtre
 * TEXTPEN: Couleur du texte
 * BACKPEN: Couleur du fond
 *
 * C'est tellement plus bô: AZCharTable FP 2 TP 1 BP 2 Table ASCII
 *    ou encore plus sobre: AZCharTable FP 0 TP 1 BP 0 Table ASCII
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <exec/execbase.h>
#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <graphics/rastport.h>

#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/exec_pragmas.h>

/****** UIK *******/
#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikdrawauto.h"
#include "uiki:uikglobal.h"
#include "uiki:uikcolors.h"
#include "uiki:uikmacros.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"

/****** Azur ******/
#include "Azur:DevelKit/Includes/funcs.h"
#include "Azur:DevelKit/Includes/azur.h"
#include "Azur:DevelKit/Includes/obj.h"
#include "Azur:DevelKit/Includes/objfile.h"
#include "Azur:DevelKit/Includes/objview.h"
#include "Azur:DevelKit/Includes/objzone.h"

#define INTER_WIDTH  2
#define INTER_HEIGHT 1

struct def_pens {
    UBYTE   FillPen;
    UBYTE   TextPen;
    UBYTE   BackPen;
    UBYTE   Reserved;
    };


/****** Imported ************************************************/


/****** Exported ************************************************/

struct DOSBase  * __far DOSBase;
struct ExecBase * __far SysBase;
struct UIKBase  * __far UIKBase;
struct GfxBase  * __far GfxBase;
struct IntuitionBase  * __far IntuitionBase;


/****** Statics *************************************************/

static void AZFunc_Do( struct CmdObj *co );


/****** Germe de l'objet de commande AZur ***********************/

/* attention à bien donner ici le même nom que son nom de fichier  */
static UBYTE * __far CmdNames[] = { "AZCharTable", 0 };

struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,           (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,    (ULONG) "FP=FILLPEN/K/N,TP=TEXTPEN/K/N,BP=BACKPEN/K/N,TITLE/F",
    AZT_Gen_Flags,          AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionDo,           (ULONG) AZFunc_Do,
    TAG_END
    };

/****************************************************************
 *
 *      Germe UIK
 *
 ****************************************************************/

static void GenericRefresh( struct UIKObj *box )
{
  struct AZurProc *ap = (struct AZurProc *)box->UserULong1;
  struct TextFont *vfont = box->ObjExt->Font;
  struct Window *w = box->w;
  struct def_pens *dp = (struct def_pens *)&box->UserULong2;
  LONG x, y, X, Y, offs;
  UBYTE ascii;

    UIK_LockObjVars( box );

    SetFont( w->RPort, vfont );
    SetDrMd( w->RPort, JAM1 );
    SetAPen( w->RPort, UIK_GetColor(box,UIKCOL_BLACK) );

    SetSoftStyle( w->RPort, FSF_BOLD, FSF_BOLD );
    x = box->Box.Left - (vfont->tf_XSize + 2);
    offs = box->Box.Top + INTER_HEIGHT + vfont->tf_Baseline + 1;
    for (y=0; y < 16; y++)
        {
        Move( w->RPort, x, offs + y * (vfont->tf_YSize + 2 * INTER_HEIGHT) );
        ascii = y < 10 ? y + '0' : y + 'A' - 10;
        Text( w->RPort, &ascii, 1 );
        }
    SetSoftStyle( w->RPort, 0, FSF_BOLD );
    y = box->Box.Top - (vfont->tf_YSize + 1) + vfont->tf_Baseline + 1;
    offs = box->Box.Left + INTER_WIDTH;
    for (x=0; x < 16; x++)
        {
        Move( w->RPort, offs + x * (vfont->tf_XSize + 2 * INTER_WIDTH), y );
        ascii = x < 10 ? x + '0' : x + 'A' - 10;
        Text( w->RPort, &ascii, 1 );
        }

    x = box->Box.Left; y = box->Box.Top; X = x + box->Box.Width - 1; Y = y + box->Box.Height - 1;
    SetAPen( w->RPort, UIK_GetColor(box,UIKCOL_BLACK) );
    Move( w->RPort, x,y ); Draw( w->RPort, X,y ); Draw( w->RPort, X,Y ); Draw( w->RPort, x,Y ); Draw( w->RPort, x,y );
    SetAPen( w->RPort, dp->FillPen );
    RectFill( w->RPort, x+1,y+1, X-1,Y-1 );

    SetAPen( w->RPort, dp->TextPen );
    SetBPen( w->RPort, dp->BackPen );
    SetDrMd( w->RPort, JAM2 );

    offs = box->Box.Top + INTER_HEIGHT + vfont->tf_Baseline + 1;
    for (y=0; y < 16; y++)
        {
        for (x=0; x < 16; x++)
            {
            ascii = y * 16 + x;
            Move( w->RPort,
                box->Box.Left + INTER_WIDTH + x * (vfont->tf_XSize + 2 * INTER_WIDTH),
                offs + y * (vfont->tf_YSize + 2 * INTER_HEIGHT) );
            Text( w->RPort, &ascii, 1 );
            }
        }
    UIK_UnlockObjVars( box );
}

static struct UIKObjGerm __far BoxGerm = {
    "",
    NULL,               /*int     (*AddVector)(); */
    NULL,               /*int     (*StartVector)(); */
    NULL,               /*void    (*StopVector)(); */
    NULL,               /*void    (*RemoveVector)(); */
    GenericRefresh,     /*void    (*RefreshVector)(); */
    NULL,               /*void    (*ResizeVector)(); */
    NULL,               /*void    (*ExceptActVector)(); */
    NULL,               /*void    (*ExceptInaVector)(); */
    NULL,               /*void    (*ExceptMovVector)(); */
    NULL,               /*void    (*ExceptKeyVector)(); */
    NULL,               /*int     (*StartAfterVector)(); */
    NULL,               /*int     (*SetGetValues)(); */
    NULL,               /*ULONG   *ExtentedVectors; */
    NULL,               /*ULONG   ExReserved[1]; */
    0,                  /*UWORD   MinWidth; */
    0,                  /*UWORD   MinHeight; */
    0,                  /*ULONG   DefaultWidthHeight; */
    -1,                 /*UWORD   ObjId; */
    sizeof(struct UIKObj),   /*UWORD   ObjSize; */
    BOX_DISPLAYABLE | BOX_TRANSLATELEFTTOP, /* flags par defaut */
    };

struct UIKObjRegGerm __far BoxRegGerm = { &BoxGerm, 0, 0 ,0 };


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

static void do_mousebuttons( struct UIKObjWindow *wo, struct AZurProc *ap )
{
  struct TextFont *vfont;
  WORD x, y, low, high, code;
  UBYTE ascii;
  struct UIKObj *box;

    if (!TST_READONLY && (ap->UIK->IntuiMsg->Code == SELECTDOWN))
        {
        if (box = UIK_GetObject( wo, 0, (UWORD)-1 ))
            {
            x = ap->UIK->IntuiMsg->MouseX - (box->Box.Left + INTER_WIDTH/2) + 1;
            y = ap->UIK->IntuiMsg->MouseY - (box->Box.Top + INTER_HEIGHT/2) - 1;
            if (x > 0 && y > 0 && x < box->Box.Width && y < box->Box.Height)
                {
                vfont = ap->FObj->ActiveView->Zone->Obj.ObjExt->Font;
                low = x / (vfont->tf_XSize + 2 * INTER_WIDTH);
                high = y / (vfont->tf_YSize + 2 * INTER_HEIGHT);
                code = high * 16 + low;
                if (code < 256)
                    {
                    ascii = code;
                    AZ_TextPrep( ap->FObj->Line, ap->FObj->Col, 0, 0, 0 );
                    AZ_TextPut( &ascii, 1, 0, 1 );
                    AZ_DisplayStats(0);
                    AZ_CursorPos( ap->FObj->WorkLine, ap->FObj->WorkCol );  /* remet le curseur */
                    }
                }
            }
        }
}

static void AZFunc_Do( struct CmdObj *co )
{
  struct AZurProc *ap = AZ_GetAZur();
  struct UIKObjWindow *wo=0;
  struct AZObjZone *zone;
  struct TextFont *vfont;
  WORD xoffs, yoffs, width, height;
  ULONG DP=0;

    UIKBase = ap->UIKBase;
    SysBase = ap->ExecBase;
    DOSBase = ap->DOSBase;
    GfxBase = ap->GfxBase;
    IntuitionBase = ap->IntuitionBase;

    if (!ap->WO) goto END_ERROR;
    if (!ap->WO->Obj.w) goto END_ERROR;

    ((struct def_pens *)&DP)->FillPen = UIK_GetColor( ap->WO, UIKCOL_WHITE );
    ((struct def_pens *)&DP)->TextPen = UIK_GetColor( ap->WO, UIKCOL_BLACK );
    ((struct def_pens *)&DP)->BackPen = ((struct def_pens *)&DP)->FillPen;
    if (co->ArgRes[0]) ((struct def_pens *)&DP)->FillPen = (UBYTE)*((ULONG*)co->ArgRes[0]);
    if (co->ArgRes[1]) ((struct def_pens *)&DP)->TextPen = (UBYTE)*((ULONG*)co->ArgRes[1]);
    if (co->ArgRes[2]) ((struct def_pens *)&DP)->BackPen = (UBYTE)*((ULONG*)co->ArgRes[2]);

    zone = ap->FObj->ActiveView->Zone;
    vfont = zone->Obj.ObjExt->Font;
    width = (vfont->tf_XSize + 2 * INTER_WIDTH)  * 16;
    height = (vfont->tf_YSize + 2 * INTER_HEIGHT) * 16;
    xoffs = vfont->tf_XSize + 2;
    yoffs = vfont->tf_YSize + 1;

    if (! (wo = (struct UIKObjWindow *) UIK_AddObjectTags( "UIKObj_Window", ap->WO,
            UIKTAG_OBJ_WidthHeight,             W2L(xoffs+width+8,yoffs+height+8),
            UIKTAG_OBJ_Title,                   co->ArgRes[3] ? co->ArgRes[3] : "ASCII",
            UIKTAG_OBJ_TitleFl_Addr,            TRUE,
            UIKTAG_OBJWindowFl_CloseStop,       FALSE,
            UIKTAG_OBJWindowFl_CloseRemove,     TRUE,
            UIKTAG_OBJWindowFl_AutoCenter,      TRUE,
            UIKTAG_OBJWindowFl_With_Size,       FALSE,
            UIKTAG_OBJWindowFl_With_Icon,       FALSE,
            TAG_END ))) goto END_ERROR;
    if (! UIK_AddObjectTags( "UIKObj_Box", wo,
            UIKTAG_OBJ_WidthHeight,             W2L(xoffs+width+8,yoffs+height+8),
            UIKTAG_OBJ_BoxFl_Over,              TRUE,
            TAG_END )) goto END_ERROR;
    if (! UIK_AddPrivateObjectTags( &BoxRegGerm, wo,
            UIKTAG_OBJ_LeftTop,                 W2L(4+xoffs,3+yoffs),
            UIKTAG_OBJ_WidthHeight,             W2L(width,height+2),
            UIKTAG_OBJ_TTextAttr,               &zone->Obj.ObjExt->TTA,
            UIKTAG_OBJ_UserValue1,              ap,
            UIKTAG_OBJ_UserValue2,              DP,
            TAG_END )) goto END_ERROR;
    if (! UIK_AddVector( wo, do_mousebuttons, IDCMP_MOUSEBUTTONS, (ULONG)ap )) goto END_ERROR;
    if (! UIK_Start( wo )) goto END_ERROR;

    return;

  END_ERROR:
    UIK_Remove( wo );
}
