/****************************************************************
 *
 *      Project:   UIK
 *      Function:  objet Stat
 *
 *      Created:   06/06/93     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikbase.h"
#include "uiki:uikglobal.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uikdrawauto.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objscreen.h"

#include "lci:azur.h"
#include "lci:azur_protos.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"

#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"


/****** Macros **************************************************/


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/


/****** Statics *************************************************/

static UWORD __far StatAuto[] = {
    x_ADD_N,1, y_ADD_N,2, X_SUB_N,2, Y_SUB_N,2,
    END_AUTO
    };


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

void ostat_Display( struct UIKObj *st, ULONG title )
{
  struct AZObjFile *fo = AGP.FObj;
  struct KeyHandle *kh = (struct KeyHandle *) AGP.AZMast->KeyHandle;
  struct AZObjView *vo = fo->ActiveView;
  struct AZObjZone *zo = vo->Zone;
  UBYTE *ptr, buf[100], black, grey, white;
  LONG line, col, w, dw;
  ULONG len, x, offs;
  struct TextFont *font = st->ObjExt->Font;
  struct RastPort *rp = st->rp;
  struct Rectangle *r = &st->ObjExt->InnerRect;
  UBYTE clip=0;
  UBYTE mask = rp->Mask;

    rp->Mask = st->sc ? ((struct UIKObjScreen *)st->sc->UserData)->Mask : UIKBase->UIKPrefs->Mask;

    if (TST_DISPLOCK) line = fo->Line;
    else line = (fo->Line < zo->TopLine || fo->Line > zo->TopLine + zo->Rows) ? zo->TopLine : fo->Line;
    col = fo->Col;          // (fo->Col < zo->TopCol || fo->Col > zo->TopCol + zo->Cols) ? zo->TopCol : fo->Col;
    sprintf( buf, " %ld/%ld  %ld  $%02.2lx  %lc%lc%lc%lc%lc%lc %lc%lc%lc%lc%lc %lc%lc%lc ",
        line+1,
        fo->Text.NodeNum,
        col+1,
        (UBYTE)intAZ_CharCurrent( col, intAZ_Buf_Len( fo->Line, &ptr ), ptr ),
        (UBYTE)(TST_OVERLAY ? fo->Ind[3] : fo->Ind[2]),
        (UBYTE)(fo->SeRep->SearchFlags & SEF_CASE ? fo->Ind[0] : fo->Ind[1]),
        (UBYTE)(TST_AUTOFORMAT ? fo->Ind[6] : fo->Ind[7]),
        (UBYTE)(TST_DEBUG      ? fo->Ind[8] : fo->Ind[9]),
        (UBYTE)(TST_COMPRESSED ? 'C' : '-'),
        (UBYTE)(TST_RESIDENT   ? 'R' : '-'),
        (UBYTE)(TST_DISPLOCK   ? 'D' : '-'),
        (UBYTE)(TST_INPUTLOCK  ? 'I' : '-'),
        (UBYTE)(TST_READONLY   ? 'M' : '-'),
        (UBYTE)(TST_SAVELOCK   ? 'S' : '-'),
        (UBYTE)(TST_AREXXLOCK  ? 'A' : '-'),
        (UBYTE)(kh->AZQual & SQUAL_LOCK1 ? '1' : '-'),
        (UBYTE)(kh->AZQual & SQUAL_LOCK2 ? '2' : '-'),
        (UBYTE)(kh->AZQual & SQUAL_LOCK3 ? '3' : '-')
        );
    len = StrLen( buf );

    black = UIK_GetColor( st, UIKCOL_BLACK );
    grey = UIK_GetColor( st, UIKCOL_GREY );
    white = UIK_GetColor( st, UIKCOL_WHITE );
    if (font != rp->Font) SetFont( rp, font );

    if (((len+7) * font->tf_XSize) > r->MaxX)
        {
        LockLayer( 0, st->rp->Layer );
        UIK_ClipObject( st ); clip = 1;
        }

    Move( rp, r->MinX, r->MinY + font->tf_Baseline );
    SetAPen( rp, black );
    SetBPen( rp, grey );
    //SetDrMd( rp, JAM2 );
    Text( rp, buf, len );

    if (fo->Flags & AZFILEF_MODIFIED)
        {
        SetAPen( rp, white );
        Text( rp, &fo->Ind[4], 1 );
        SetAPen( rp, black );
        }
    else Text( rp, &fo->Ind[5], 1 );

    if (rp->cp_x <= r->MaxX)
        {
        x = rp->cp_x + 2*font->tf_XSize;
        SetAPen( rp, grey ); RectFill( rp, rp->cp_x, r->MinY, x, r->MaxY );

        ptr = fo->OpenFileName;
        if (*ptr)
            {
            w = font->tf_XSize * (len = BufLength( ptr )) ;
            dw = r->MaxX - x;
            SetAPen( rp, black );
            Move( rp, x, rp->cp_y );
            if ((w -= dw) <= 0)
                { Text( rp, ptr, len ); }
            else{
                Text( rp, "...", 3 );
                offs = 3 + ((w + font->tf_XSize-1) / font->tf_XSize);
                if (offs < len) Text( rp, ptr + offs, len - offs );
                }
            }
        if (rp->cp_x <= r->MaxX)
            { SetAPen( rp, grey ); RectFill( rp, rp->cp_x, r->MinY, r->MaxX, r->MaxY ); }
        }

    if (clip)
        {
        UIK_UnclipObject( st );
        UnlockLayer( st->rp->Layer );
        }

    rp->Mask = mask;
}

/*------- Object entries ----------------------------------------*/

static void GenericRefresh( struct UIKObj *st )
{
    if (st->Status == UIKS_STARTED)
        {
        UWORD x = st->Box.Left, y = st->Box.Top;
        UWORD X = x+st->Box.Width-1, Y = y+st->Box.Height-1;
        UBYTE black, grey;
        struct RastPort *rp = st->rp;

//        LockLayer( 0, rp->Layer );

        black = UIK_GetColor( st, UIKCOL_BLACK );
        grey = UIK_GetColor( st, UIKCOL_GREY );
        SetAPen( rp, black );
        Move( rp, x,y ); Draw( rp, X,y ); y++; Draw( rp, X,Y ); X--; Draw( rp, x,Y ); Y--;
        SetAPen( rp, grey );
        Move( rp, x,Y ); Draw( rp, x,y ); Draw( rp, X,y ); Draw( rp, X,Y ); x++;
        Draw( rp, x,Y ); //Draw( rp, x,y );

        ostat_Display( st, 1 );

//        UnlockLayer( rp->Layer );
        }
}

/*------ Objects functions --------------------------------------*/


/*------ Germ ---------------------------------------------------*/

static struct TagItem __far GermTagList[] =
{
    UIKTAG_GEN_ActInactPens,    UIKCOLS(UIKCOL_GREY,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_BLACK),
    UIKTAG_GEN_BoxAutomate,     (ULONG)StatAuto,
    UIKTAG_GEN_BoxAltAutomate,  (ULONG)StatAuto,
    TAG_END
};

static struct UIKObjGerm __far Germ = {
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
    NULL,               /*int     (*SelectVector)(); */
    NULL,               /*ULONG   *ExtentedVectors; */
    GermTagList,        /*struct TagItem *GermTagList; */
    8,                  /*UWORD   MinWidth; */
    4,                  /*UWORD   MinHeight; */
    SETL(10,10), /*ULONG DefaultWidthHeight; */
    0,                 /*UWORD   ObjId; */
    sizeof(struct UIKObj),   /*UWORD   ObjSize; */
    BOX_DISPLAYABLE | BOX_CLIPPED | BOX_TRANSLATELEFTTOP,
    };

struct UIKObjRegGerm __far ObjStatRegGerm = { &Germ, 0, 0 ,0 };
