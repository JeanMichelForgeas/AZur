/****************************************************************
 *
 *      File:      objzone.c
 *      Project:   gestion d'un fichier
 *
 *      Created:   12-11-92     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <graphics/rpattr.h>

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikdrawauto.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"
#include "uiki:objscreen.h"

#include "lci:azur.h"
#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"
#include "lci:azur_protos.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/

void ozone_Display( struct AZObjZone *zo, LONG startline, LONG nlines, LONG startcol, LONG ncols );
void ozone_ReDisplay( struct AZObjZone *zo );
void ozone_FillWindow( struct AZObjZone *zo );
void ozone_DisplayBlock( struct AZObjZone *zo );
void ozone_DisplayCmd( struct AZObjZone *zo, struct AZDisplayCmd *dcmd );
void ozone_ZoneTopLeft( struct AZObjZone *zo );
void ozone_VScroll( struct AZObjZone *zo, ULONG abslen, LONG len, LONG from );


/****** Statics ************************************************/

static UWORD __far ZoneAuto[] = {
    /*
    COL_BLACK, MOVE_Xy, DRAW_XY, DRAW_xY,
    X_SUB_1, Y_SUB_1,
    COL_WHITE, MOVE_xY, DRAW_xy, DRAW_Xy, DRAW_XY,
    x_ADD_1, DRAW_xY, DRAW_xy,
    x_ADD_1, y_ADD_1, X_SUB_1, Y_SUB_1,
    */
    COL_BLACK, MOVE_xy, DRAW_Xy,
    x_ADD_N,2, y_ADD_N,1, X_SUB_N,2, Y_SUB_N,2,
    END_AUTO
    };


/****************************************************************
 *
 *      Germe
 *
 ****************************************************************/

static void GetRows( struct AZObjZone *zo )
{
  struct Rectangle *rect = &zo->Obj.ObjExt->InnerRect;
  UWORD ysize = zo->Obj.ObjExt->Font->tf_YSize;
  UWORD height = rect->MaxY - rect->MinY + 1;

    zo->Rows = (height + ysize - 1) / ysize;
}

static void VerifPosNum( struct AZObjZone *zo )
{
  LONG pos, n;

    pos = zo->TopLine; n = AGP.FObj->Text.NodeNum + PLUSLINES - (zo->Rows - 1);
    if (pos > n)
        if ((pos = n) < 0)
            pos = 0;
    zo->TopLine = pos;
}

/*
static void ResetPosNum( struct AZObjZone *zo )
{
  struct AZObjView *view = (struct AZObjView *) zo->Obj.Parent;

    VerifPosNum( zo );
    if (view->VProp)
        UIK_OPr_ChangePropGadget( view->VProp, AGP.FObj->Text.NodeNum+PLUSLINES, zo->Rows-1, zo->TopLine );
}
*/

//----------------------------------------------------------------

void HGetMaxLen( struct AZObjZone *zo, LONG topline )
{
  struct AZObjFile *fo = AGP.FObj;
  struct UIKNode *node;
  LONG line, last;
  ULONG linelen;

    zo->MaxLineLen = 0;
    line = topline;
    last = fo->Text.NodeNum;

    if ((last == 0) || ((last - line) <= 0)) return;
    if (last > line + zo->Rows) last = line + zo->Rows;

    for (node=&fo->Text.UNode[line]; line < last; line++, node++) // attention ne pas faire de
        {                                                         // AllocNode dans la boucle
        linelen = intAZ_LineBufLen( node->Buf );
        if (line == fo->Line && fo->Col > linelen) linelen = fo->Col;
        if (linelen > zo->MaxLineLen) zo->MaxLineLen = linelen;
        }
    zo->NewHPropLineLen = MAX(zo->MaxLineLen+PLUSCOLS,zo->TopCol+zo->Cols-1);
}

static void GetCols( struct AZObjZone *zo )
{
  struct Rectangle *rect = &zo->Obj.ObjExt->InnerRect;
  UWORD xsize = zo->Obj.ObjExt->Font->tf_XSize;
  UWORD width = rect->MaxX - rect->MinX + 1;

    zo->Cols = (width + xsize - 1) / xsize;
}

static void HVerifPosNum( struct AZObjZone *zo )
{
  LONG pos, n;

    pos = zo->TopCol; n = zo->MaxLineLen + PLUSCOLS - (zo->Cols - 1);
    if (pos > n)
        if ((pos = n) < 0)
            pos = 0;
    zo->TopCol = pos;
}

/*
static void HResetPosNum( struct AZObjZone *zo )
{
  struct AZObjView *view = (struct AZObjView *) zo->Obj.Parent;

    HVerifPosNum( zo );
    if (view->HProp)
        UIK_OPr_ChangePropGadget( view->HProp, zo->HPropLineLen, zo->Cols-1, zo->TopCol );
}
*/

//----------------------------------------------------------------

static int GenericAdd( struct AZObjZone *zo )
{
#ifdef  ZONE_IS_GAD
  struct Rectangle *rect = &zo->Obj.ObjExt->InnerRect;

    zo->Gad.LeftEdge = rect->MinX;
    zo->Gad.TopEdge  = rect->MinY;
    zo->Gad.Width    = rect->MaxX - rect->MinX + 1;
    zo->Gad.Height   = rect->MaxY - rect->MinY + 1;
    zo->Gad.UserData = (APTR) zo;
    zo->Gad.Flags      = GADGHNONE;
    zo->Gad.Activation = FOLLOWMOUSE | RELVERIFY | GADGIMMEDIATE;
    zo->Gad.GadgetType = BOOLGADGET;
#endif

    return(1);
}

static void GenericStop( struct AZObjZone *zo )
{
#ifdef  ZONE_IS_GAD
    UIK_StdGadStop( zo );
#endif
}

static void GenericRemove( struct AZObjZone *zo )
{
}

static void GenericResize( struct AZObjZone *zo )
{
  struct AZObjView *view = (struct AZObjView *) zo->Obj.Parent;

#ifdef  ZONE_IS_GAD
  struct Rectangle *rect = &zo->Obj.ObjExt->InnerRect;
    zo->Gad.LeftEdge = rect->MinX;
    zo->Gad.TopEdge  = rect->MinY;
    zo->Gad.Width    = rect->MaxX - rect->MinX + 1;
    zo->Gad.Height   = rect->MaxY - rect->MinY + 1;
#endif
    GetRows( zo );
    VerifPosNum( zo );
    if (view->VProp)
        UIK_OPr_ChangePropNoDraw( view->VProp, AGP.FObj->Text.NodeNum+PLUSLINES, zo->Rows-1, zo->TopLine );
    GetCols( zo );
    HGetMaxLen( zo, zo->TopLine );
    HVerifPosNum( zo );
    if (view->HProp)
        UIK_OPr_ChangePropNoDraw( view->HProp, (zo->HPropLineLen = zo->NewHPropLineLen), zo->Cols-1, zo->TopCol );
}

static int GenericStart( struct AZObjZone *zo )
{
  /*struct AZurPrefs *p = AGP.Prefs;
  */
    GenericResize( zo );
    return(1);
}

static void GenericRefresh( struct AZObjZone *zo )
{
  struct AZurPrefs *prefs = AGP.Prefs;

    if (zo->Obj.Status == UIKS_STARTED)
        {
        UWORD x = zo->Obj.Box.Left, y = zo->Obj.Box.Top;
        UWORD X = x+zo->Obj.Box.Width-1, Y = y+zo->Obj.Box.Height-1;

        // Pas de "rp->Mask = ..." car le RectFill ne mettrait pas toujours la bonne couleur
        if (zo->OldMask > AGP.FObj->Mask) { SetAPen( zo->Obj.rp, prefs->BackPen ); RectFill( zo->Obj.rp, x,y, X,Y ); }

        //UIK_DrawAuto( zo, ZoneAuto, 0, 0 );
 //        LockLayer( 0, zo->Obj.rp->Layer );

        SetAPen( zo->Obj.rp, UIK_GetColor( zo, UIKCOL_BLACK ) );
        Move( zo->Obj.rp, X,y ); /*Draw( zo->Obj.rp, X,y );*/ Draw( zo->Obj.rp, X,Y ); Draw( zo->Obj.rp, x,Y ); /*y++;*/ X--; Y--;
        SetAPen( zo->Obj.rp, prefs->BackPen );
        Move( zo->Obj.rp, x,Y ); Draw( zo->Obj.rp, x,y ); Draw( zo->Obj.rp, X,y ); Draw( zo->Obj.rp, X,Y ); x++;
        Draw( zo->Obj.rp, x,Y ); Draw( zo->Obj.rp, x,y );

        //UIK_ClipObject( zo );
        ozone_ReDisplay( zo );
        //UIK_UnclipObject( zo );

 //        UnlockLayer( zo->Obj.rp->Layer );

        zo->OldMask = AGP.FObj->Mask;
        }
}

static struct TagItem __far GermTagList[] =
    {
    UIKTAG_GEN_BoxAutomate,     (ULONG)ZoneAuto,
    UIKTAG_GEN_BoxAltAutomate,  (ULONG)ZoneAuto,
    TAG_END
    };

static struct UIKObjGerm __far Germ = {
    "AZURObj_Zone",
    GenericAdd,         /*int     (*AddVector)(); */
    GenericStart,       /*int     (*StartVector)(); */
    GenericStop,        /*void    (*StopVector)(); */
    GenericRemove,      /*void    (*RemoveVector)(); */
    GenericRefresh,     /*void    (*RefreshVector)(); */
    GenericResize,      /*void    (*ResizeVector)(); */
    NULL,               /*void    (*ExceptActVector)(); */
    NULL,               /*void    (*ExceptInaVector)(); */
    NULL,               /*void    (*ExceptMovVector)(); */
    NULL,               /*void    (*ExceptKeyVector)(); */
    NULL,               /*int     (*StartAfterVector)(); */
    NULL,               /*int     (*SetGetValues)(); */
    NULL,               /*ULONG   *ExtentedVectors; */
    GermTagList,        /*ULONG   ExReserved[1]; */
    0,                  /*UWORD   MinWidth; */
    0,                  /*UWORD   MinHeight; */
    0,                  /*ULONG   DefaultWidthHeight; */
    0,                  /*UWORD   ObjId; */
    sizeof(struct AZObjZone),   /*UWORD   ObjSize; */
    BOX_ZONEABLE | BOX_EXTGADUP | BOX_DISPLAYABLE | BOX_INNERZONE | BOX_CLIPPED | BOX_SEMAPHORE | BOX_TRANSLATELEFTTOP, /* flags par defaut */
    };

struct UIKObjRegGerm __far ObjZoneRegGerm = { &Germ, 0, 0 ,0 };


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

#define CHARIMAGE_AFTEREND  0x01

static UBYTE __asm CharImage( register __d0 ULONG line, register __d1 LONG col, register __a0 UBYTE *pflags )
{
  ULONG len;
  UBYTE *ptr, ch;

    if (pflags) *pflags = 0;
    len = intAZ_Buf_Len( line, &ptr );

    if (col < len)
        {
        ch = ptr[col];
        }
    else if (col == len)
        {
        struct UIKPList *ul = &AGP.FObj->Text;
        if (pflags) *pflags |= CHARIMAGE_AFTEREND;
        if (line == ul->NodeNum - 1)
            {
            ch = TST_SHOWEOF ? AGP.Prefs->CharEOF : ' ';  /* zéro */
            }
        else{
            ch = (TST_SHOWEOL && !(ul->UNode[line].Flags & UNF_CONTINUE)) ? AGP.Prefs->CharEOL : ' ';  /* ¶ */
            }
        }
    else ch = ' '; /* supérieur */          /* espace */

    return( ch );
}

void ozone_GhostCurs( struct AZObjZone *zo )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZurPrefs *prefs = AGP.Prefs;
  ULONG curspen;

    if (!(fo->Flags & AZFILEF_CURSDISABLED))
      if (fo->Line < fo->Text.NodeNum && fo->Line >= 0 && fo->Col >= 0)
        if (fo->Line >= zo->TopLine && fo->Line <= zo->TopLine + zo->Rows && fo->Col >= zo->TopCol && fo->Col <= zo->TopCol + zo->Cols)
            {
            struct RastPort *rp=zo->Obj.rp;
            struct TextFont *font = zo->Obj.ObjExt->Font;
            struct Rectangle *r = &zo->Obj.ObjExt->InnerRect;
            UBYTE clip=0;
            UWORD x, y, pat[2]={0xAAAA,0x5555};
            UBYTE mask = rp->Mask;

            rp->Mask = fo->Mask;

            x = r->MinX + ((fo->Col - zo->TopCol) * font->tf_XSize);
            y = r->MinY + ((fo->Line - zo->TopLine) * font->tf_YSize);
            if (x + font->tf_XSize > r->MaxX || y + font->tf_YSize > r->MaxY)
                {
                LockLayer( 0, zo->Obj.rp->Layer );
                UIK_ClipObject( zo ); clip = 1;
                }

            if (func_BlockInside( 1, fo->Line, fo->Col, 0, 1 )) curspen = prefs->BlockBPen;
            else if (fo_IsLineFolded( fo->Line )) curspen = prefs->FDI[0].OnBPen;
            else curspen = prefs->BackPen;

            SetDrMd( rp, JAM1 );
            SetAPen( rp, curspen );
            SetAfPt( rp, pat, 1 )
            RectFill( rp, x, y, x + font->tf_XSize - 1, y + font->tf_YSize - 1 );
            SetAfPt( rp, 0, 0 )
            SetDrMd( rp, JAM2 );

            if (clip)
                {
                UIK_UnclipObject( zo );
                UnlockLayer( zo->Obj.rp->Layer );
                }
            rp->Mask = mask;
            }
}

void ozone_DisplayCurs( struct AZObjZone *zo, LONG line, LONG col )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZurPrefs *prefs = AGP.Prefs;
  ULONG blockid, setstyle=0, linelen;
  struct UIKNode *node;
  UBYTE textpen, backpen, charflags, nil;

    if (! (fo->Flags & AZFILEF_CURSDISABLED))
      if (line < fo->Text.NodeNum && line >= 0 && col >= 0)
        if (line >= zo->TopLine && line <= zo->TopLine + zo->Rows && col >= zo->TopCol && col <= zo->TopCol + zo->Cols)
            {
            struct RastPort *rp=zo->Obj.rp;
            struct TextFont *font = zo->Obj.ObjExt->Font;
            struct Rectangle *r = &zo->Obj.ObjExt->InnerRect;
            UBYTE clip=0, ch;
            UWORD x, y, Y;
            UBYTE ch2, mask = rp->Mask;

            rp->Mask = fo->Mask;

            ch = CharImage( line, col, &charflags );
            x = r->MinX + ((col - zo->TopCol) * font->tf_XSize);
            y = r->MinY + ((line - zo->TopLine) * font->tf_YSize);

            if (x + font->tf_XSize > r->MaxX || y + font->tf_YSize > r->MaxY)
                {
                LockLayer( 0, zo->Obj.rp->Layer );
                UIK_ClipObject( zo ); clip = 1;
                }

            Move( rp, x, y + font->tf_Baseline );
            SetDrMd( rp, JAM2 );
            blockid = func_BlockInside( 1, line, col, 0, 1 );
            if ((fo->Flags & AZFILEF_CURSVISIBLE) && (line == fo->Line && col == fo->Col))
                {
                if (blockid) { textpen = prefs->CursBlockTPen; backpen = prefs->CursBlockBPen; }
                else { textpen = prefs->CursTextPen; backpen = prefs->CursBackPen; }
                }
            else{
                if (blockid) { textpen = prefs->BlockTPen; backpen = prefs->BlockBPen; }
                else if (fo_IsLineFolded( line ))
                    {
                    textpen = prefs->FDI[0].OnAPen; backpen = prefs->FDI[0].OnBPen;
                    linelen = intAZ_LineLen( line );
                    if (col <= linelen) /* le = est pour col-1 */
                        {
                        SetSoftStyle( rp, prefs->FDI[0].OnFlags, prefs->FDI[0].OnFlags );
                        setstyle = 1;
                        }
                    }
                else { textpen = prefs->TextPen; backpen = prefs->BackPen; }
                }
            SetAPen( rp, prefs->TextPen ); SetBPen( rp, prefs->BackPen );
            if (font != rp->Font) SetFont( rp, font );
            SetAPen( rp, textpen ); SetBPen( rp, backpen );

            if (charflags & CHARIMAGE_AFTEREND) // ne pas bolder les fin de ligne ou fin de fichier
                {
                if (setstyle) SetSoftStyle( rp, 0, prefs->FDI[0].OnFlags );
                Text( rp, &ch, 1 );
                if (setstyle) SetSoftStyle( rp, prefs->FDI[0].OnFlags, prefs->FDI[0].OnFlags );
                }
            else Text( rp, &ch, 1 );

            SetDrMd( rp, JAM1 ); // pour USEMARGIN et setstyle

            if (setstyle) // tracer le caractère suivant en JAM1 sinon le bold ou italique en bouffe une partie
                {         // quand on dessine en normal (le curseur n'est pas boldé)
                if (prefs->FDI[0].OnFlags & FSF_ITALIC)
                    {
                    if ((col > 0) && ((col-1) < linelen))
                        {
                        if (!clip && (x - font->tf_XSize < r->MinX))
                            {
                            LockLayer( 0, zo->Obj.rp->Layer );
                            UIK_ClipObject( zo ); clip = 1;
                            }
                        Move( rp, x - font->tf_XSize, y + font->tf_Baseline );
                        ch2 = CharImage( line, col-1, &nil );
                        Text( rp, &ch2, 1 );
                        }
                    }

                ch = CharImage( line, col+1, &charflags );
                if ((charflags & CHARIMAGE_AFTEREND) // ne pas bolder les fin de ligne ou fin de fichier
                    || (col+1 >= linelen))
                    { if (setstyle) SetSoftStyle( rp, 0, prefs->FDI[0].OnFlags ); setstyle = 0; }
                if (!clip && (x + 2*font->tf_XSize > r->MaxX))
                    {
                    LockLayer( 0, zo->Obj.rp->Layer );
                    UIK_ClipObject( zo ); clip = 1;
                    }
                Move( rp, x + font->tf_XSize, y + font->tf_Baseline );
                Text( rp, &ch, 1 );

                if (setstyle) SetSoftStyle( rp, 0, prefs->FDI[0].OnFlags );
                }

            node = &fo->Text.UNode[line];
            Y = y + font->tf_YSize - 1;
            SetAPen( rp, prefs->TextPen );
            if (node->Reserved && col < node->Reserved)
                {
                Move( rp, x, y ); Draw( rp, x, Y );
                }
            if (TST_USEMARGIN && (col == fo->LeftMargin-1 || col == fo->RightMargin))
                {
                SetDrPt( rp, 0x1111 );
                x += (font->tf_XSize / 2);
                Move( rp, x, y ); Draw( rp, x, Y );
                SetDrPt( rp, 0xffff );
                }

            if (clip)
                {
                UIK_UnclipObject( zo );
                UnlockLayer( zo->Obj.rp->Layer );
                }
            SetDrMd( rp, JAM2 );
            rp->Mask = mask;
            }
}

struct draw_text_info
    {
    struct AZObjZone *zo;
    struct RastPort *rp;
    struct TextFont *font;
    struct UIKNode *node;
    ULONG inblock;
    LONG  clip, line, linelen, clipline, clipcol;
    LONG  startcol, ncols;
    UBYTE *lineptr;
    UWORD XX, x, y, X, Y, left, top;
    };

static void draw_text( struct draw_text_info *dti )
{
  LONG len, EOLX=-1, EOLC;
  UBYTE textpen, backpen, ch, setstyle=0;
  struct AZurPrefs *prefs = AGP.Prefs;

    dti->x = dti->left + (dti->startcol - dti->zo->TopCol) * dti->font->tf_XSize;
    dti->X = dti->x + dti->ncols * dti->font->tf_XSize - 1; if (dti->X > dti->XX) dti->X = dti->XX;

    if (dti->inblock) { textpen = prefs->BlockTPen; backpen = prefs->BlockBPen; }
    else if (dti->node->Flags & UNF_FOLDED)
        {
        textpen = prefs->FDI[0].OnAPen; backpen = prefs->FDI[0].OnBPen;
        SetSoftStyle( dti->rp, prefs->FDI[0].OnFlags, prefs->FDI[0].OnFlags );
        setstyle = 1;
        }
    else { textpen = prefs->TextPen; backpen = prefs->BackPen; }

    if ((len = dti->linelen - dti->startcol) <= 0) //-------------------------- Ligne vide
        {
        //--- Efface ligne
        SetAPen( dti->rp, backpen ); RectFill( dti->rp, dti->x, dti->y, dti->X, dti->Y );

        if (len == 0)
            {
            EOLX = dti->x;
            EOLC = dti->startcol;
            }
        }
    else//--------------------------------------------------------- Tracé de texte
        {
        if (len > dti->ncols) len = dti->ncols;
        if (dti->startcol < dti->zo->TopCol + dti->zo->Cols)
            if (dti->startcol + len > dti->zo->TopCol + dti->zo->Cols)
                len = dti->zo->TopCol + dti->zo->Cols - dti->startcol;

        //--- Efface fin de ligne (avant Text() en cas d'italique ou bold)
        if (setstyle && (dti->x+dti->font->tf_XSize <= dti->X))
            {
            SetAPen( dti->rp, backpen ); RectFill( dti->rp, dti->x+dti->font->tf_XSize, dti->y, dti->X, dti->Y );
            }

        //--- Trace ligne
        if (dti->clip == 0 && ((dti->linelen >= dti->clipcol) || (dti->line >= dti->clipline)))
            {
            LockLayer( 0, dti->zo->Obj.rp->Layer );
            UIK_ClipObject( dti->zo ); dti->clip = 1;
            }
        Move( dti->rp, dti->x, dti->top );
        SetAPen( dti->rp, textpen ); SetBPen( dti->rp, backpen );
        Text( dti->rp, dti->lineptr+dti->startcol, len );

        if (dti->rp->cp_x <= dti->X)
            {
            if (! setstyle) { SetAPen( dti->rp, backpen ); RectFill( dti->rp, dti->rp->cp_x, dti->y, dti->X, dti->Y ); }
            EOLX = dti->rp->cp_x;
            EOLC = dti->startcol + len;
            }
        }
    if (setstyle) SetSoftStyle( dti->rp, 0, prefs->FDI[0].OnFlags );

    //----------------- trace caractères de fin de ligne ou de fichier
    if (EOLX != -1)
        {
        if (dti->clip == 0 && ((dti->linelen+1 >= dti->clipcol) || (dti->line >= dti->clipline)))
            {
            LockLayer( 0, dti->zo->Obj.rp->Layer );
            UIK_ClipObject( dti->zo ); dti->clip = 1;
            }
        ch = CharImage( dti->line, EOLC, 0 );
        Move( dti->rp, EOLX, dti->top );
        SetAPen( dti->rp, textpen ); SetBPen( dti->rp, backpen );
        SetDrMd( dti->rp, JAM1 );
        Text( dti->rp, &ch, 1 );
        SetDrMd( dti->rp, JAM2 );
        }
}

static void draw_rect( struct AZBlock *azb, LONG curcol, LONG curncols, struct draw_text_info *dti )
{
  LONG num, tmp, cs = azb->ColStart, ce = azb->ColEnd;

    if (cs > ce) { tmp = cs; cs = ce; ce = tmp; }
    while (curncols)
        {
        if (curcol < cs)
            {
            if ((num = cs - curcol) > curncols) num = curncols;
            dti->inblock = 0;
            }
        else if (curcol <= ce)
            {
            if ((num = ce - curcol + 1) > curncols) num = curncols;
            dti->inblock = 1;
            }
        else{
            num = curncols;
            dti->inblock = 0;
            }
        dti->startcol = curcol;
        dti->ncols = num;
        draw_text( dti );  // trace le début de ligne en blanc
        curcol += num;
        curncols -= num;
        }
}

void ozone_Display( struct AZObjZone *zo, LONG startline, LONG nlines, LONG startcol, LONG ncols )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZurPrefs *prefs = AGP.Prefs;
  struct RastPort *rp = zo->Obj.rp;
  struct Rectangle *r = &zo->Obj.ObjExt->InnerRect;
  UWORD x, X, lineflags, maxx, maxy;
  ULONG searchblock, curssearchblock, plainline;
  struct AZBlock *azb;
  LONG curcol, curncols, num;
  UBYTE mask = rp->Mask;

  struct draw_text_info DTI;

    DTI.zo = zo;
    DTI.rp = rp;
    DTI.font = zo->Obj.ObjExt->Font;
    DTI.clip = 0;
    DTI.clipline = zo->TopLine + zo->Rows - 1;
    DTI.clipcol = zo->TopCol + zo->Cols;

    rp->Mask = fo->Mask;

    if (DTI.font != rp->Font) SetFont( rp, DTI.font );
    SetDrMd( rp, JAM2 );

    DTI.left = r->MinX; DTI.top = r->MinY; maxx = r->MaxX; maxy = r->MaxY;

    //------ Vérifications
    if ((fo->Text.NodeNum == 0) || ((fo->Text.NodeNum - zo->TopLine) <= 0))
        {
        zo->MaxLineLen = 0;
        if (DTI.left <= maxx && DTI.top <= maxy) { SetAPen( rp, prefs->BackPen ); RectFill( rp, DTI.left, DTI.top, maxx, maxy ); }
        goto END_PROP;
        }

    if (nlines == -1) nlines = fo->Text.NodeNum;
    if (startline < zo->TopLine)
        {
        if ((nlines -= (zo->TopLine - startline)) <= 0) goto END_PROP; // rajouté...
        startline = zo->TopLine;
        }

    if (startcol > zo->TopCol + zo->Cols) goto END_PROP;
    if (ncols == -1) ncols = zo->Cols - (startcol - zo->TopCol);
    if (startcol < zo->TopCol)
        {
        if ((ncols -= (zo->TopCol - startcol)) < 0) goto END_PROP;
        startcol = zo->TopCol;
        }

    //------ Saute les premières lignes
    DTI.line = zo->TopLine;
    DTI.node = &fo->Text.UNode[DTI.line]; // !!!!! ne pas faire AllocNode plus loin !!!!!!!!

    DTI.y = DTI.top;
    DTI.top += rp->TxBaseline;
    maxy += rp->TxBaseline;

    //------ colonne
    zo->MaxLineLen = 0;
    x = DTI.left + (startcol - zo->TopCol) * DTI.font->tf_XSize;
    X = x + ncols * DTI.font->tf_XSize - 1; if (X > r->MaxX) X = r->MaxX;
    DTI.x = x; DTI.XX = DTI.X = X;
    DTI.startcol = startcol; DTI.ncols = ncols;

    //------ Dessine les lignes
    searchblock = 1;
    while (TRUE)
        {
        if (DTI.top > maxy)
            {
            break;
            }
        if (DTI.line >= fo->Text.NodeNum)
            {
            if ((DTI.line < startline + nlines) && (DTI.left <= maxx && DTI.y <= maxy))
                { SetAPen( rp, prefs->BackPen ); RectFill( rp, DTI.left, DTI.y, r->MaxX, r->MaxY ); }
            break;
            }

        DTI.lineptr = DTI.node->Buf;
        DTI.linelen = intAZ_LineBufLen( DTI.lineptr ); if (DTI.linelen > zo->MaxLineLen) zo->MaxLineLen = DTI.linelen;
        lineflags = DTI.node->Flags;

        if (DTI.line >= startline && DTI.line < startline + nlines)
            {
            if (DTI.inblock = func_BlockLineInside( searchblock, DTI.line, &azb )) searchblock = DTI.inblock;

            DTI.Y = DTI.y + DTI.font->tf_YSize - 1; if (DTI.Y > r->MaxY) DTI.Y = r->MaxY;

            //******************************************************************* Tracé du texte

            plainline = 1;
            curssearchblock = searchblock;
            if (lineflags & (UNF_BLKSTART | UNF_BLKEND))
                {
                if (DTI.inblock) // = func_BlockLineInside( DTI.inblock, DTI.line, &azb )
                    {
                    //---------------------------------------------- Type du block ?
                    if (azb->Mode != BLKMODE_LINE)
                        {
                        LONG tmp, cs = azb->ColStart, ce = azb->ColEnd;
                        if (cs > ce) { tmp = cs; cs = ce; ce = tmp; }

                        plainline = 0;
                        curcol = startcol; curncols = ncols;
                        searchblock = DTI.inblock;

                        //------------------------------------------ Le 1er char est-il dans un block?
    ANOTHERONE:
                        DTI.inblock = func_BlockInside( searchblock, DTI.line, curcol, &azb, 0 );

                        if (DTI.inblock)//------------------------------- ON attend un BLKEND
                            {                            // (le 1er char peut être égal à ColStart)
                            searchblock = DTI.inblock;
                            if (azb->LineEnd == DTI.line)
                                {
                                if (azb->ColEnd >= DTI.linelen) num = curncols;
                                else if ((num = azb->ColEnd - curcol + 1) > curncols) num = curncols;

                                if (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) draw_rect( azb, curcol, num, &DTI );
                                else{
                                    DTI.startcol = curcol; DTI.ncols = num;
                                    draw_text( &DTI );      // trace la fin de bloc
                                    }   // il faut maintenant tracer la fin de ligne...
                                searchblock++; // on a fini avec ce bloc, suivant...
                                curcol += num;
                                if (curncols -= num)   // blocks après la fin ?
                                    {
                                    if (DTI.inblock = func_BlockLineInside( searchblock, DTI.line, &azb ))
                                        {   // on a un bloc derrière...
                                        goto ANOTHERONE;
                                        }
                                    else{   // plus rien : on termine la ligne en blanc
                                        /*if (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) draw_rect( azb, startcol, ncols, &DTI );
                                        else*/{
                                            DTI.startcol = curcol; DTI.ncols = curncols;
                                            draw_text( &DTI ); // trace fin de ligne en blanc. Fin.
                                            }
                                        }
                                    }
                                }
                            else{  // début de bloc
                                if (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) draw_rect( azb, curcol, curncols, &DTI );
                                else{
                                    DTI.startcol = curcol; DTI.ncols = curncols;
                                    draw_text( &DTI );
                                    }
                                }
                            }
                        else            //------------------------------- OFF attend un BLKSTART si début
                            {
                            if (azb->LineStart == DTI.line && azb->ColStart > curcol)
                                {
                                if ((num = azb->ColStart - curcol) > curncols) num = curncols;
                                DTI.startcol = curcol; DTI.ncols = num;
                                draw_text( &DTI );  // trace le début de ligne en blanc

                                curcol += num;
                                if (curncols -= num)
                                    {     // s'il reste des caractères, c'est un début de bloc
                                    goto ANOTHERONE;
                                    }
                                }
                            else        // sinon, on est sur LineEnd après un BLKEND
                                {
                                searchblock++;
                                if (DTI.inblock = func_BlockLineInside( searchblock, DTI.line, &azb ))
                                    {   // on a un bloc derrière...
                                    goto ANOTHERONE;
                                    }
                                else{   // plus rien : on termine la ligne en blanc
                                    num = curncols;
                                    DTI.startcol = curcol; DTI.ncols = num;
                                    draw_text( &DTI ); // trace fin de ligne en blanc. Fin.
                                    }
                                }
                            }
                        DTI.x = x; DTI.X = X; DTI.startcol = startcol; DTI.ncols = ncols; // restore
                        }
                    }
                }

            //================================================================== Ligne pleine

            if (plainline)
                {
                if (DTI.inblock && (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL)) draw_rect( azb, startcol, ncols, &DTI );
                else draw_text( &DTI );
                }

            SetAPen( rp, prefs->TextPen );
            if (DTI.node->Reserved)
                {
                ULONG i, max=MIN(startcol+ncols,DTI.node->Reserved);
                for (i=startcol; i < max; i++)
                    {
                    ULONG xx = x + (i - zo->TopCol) * DTI.font->tf_XSize;
                    Move( rp, xx, DTI.y ); Draw( rp, xx, DTI.Y );
                    }
                }
            if (TST_USEMARGIN)
                {
                LONG xx, margcol = fo->LeftMargin - 1;
                SetDrMd( rp, JAM1 );
                SetDrPt( rp, 0x1111 );
                if (margcol >= zo->TopCol && margcol < zo->TopCol + zo->Cols)
                    {
                    xx = r->MinX + DTI.font->tf_XSize / 2 + ((margcol - zo->TopCol) * DTI.font->tf_XSize);
                    if (xx < r->MaxX) { Move( rp, xx, DTI.y ); Draw( rp, xx, DTI.Y ); }
                    }
                margcol = fo->RightMargin;
                if (margcol >= zo->TopCol && margcol < zo->TopCol + zo->Cols)
                    {
                    xx = r->MinX + DTI.font->tf_XSize / 2 + ((margcol - zo->TopCol) * DTI.font->tf_XSize);
                    if (xx < r->MaxX) { Move( rp, xx, DTI.y ); Draw( rp, xx, DTI.Y ); }
                    }
                SetDrPt( rp, 0xffff );
                SetDrMd( rp, JAM2 );
                }

            //******************************************************************* Tracé du curseur

            if (DTI.line == fo->Line && AGP.FObj->ActiveView->Zone == zo)
                {
                if ((fo->Flags & AZFILEF_CURSVISIBLE) && !(fo->Flags & AZFILEF_CURSDISABLED))
                    {
                    if (fo->Col >= startcol && fo->Col < startcol + ncols)
                        {
                        UBYTE ch = CharImage( fo->Line, fo->Col, 0 ); //(fo->Col >= DTI.linelen) ? ' ' : DTI.lineptr[fo->Col];
                        UWORD xc = DTI.left + (fo->Col-zo->TopCol) * DTI.font->tf_XSize;
                        if (DTI.clip == 0 && (xc + DTI.font->tf_XSize > r->MaxX || DTI.top + DTI.font->tf_YSize > r->MaxY))
                            {
    LockLayer( 0, zo->Obj.rp->Layer );
                            UIK_ClipObject( zo ); DTI.clip = 1;
                            }
                        Move( rp, xc, DTI.top );
                        if (func_BlockInside( curssearchblock, fo->Line, fo->Col, &azb, 1 ))
                            { SetAPen( rp, prefs->CursBlockTPen ); SetBPen( rp, prefs->CursBlockBPen ); }
                        else
                            { SetAPen( rp, prefs->CursTextPen ); SetBPen( rp, prefs->CursBackPen ); }
                        Text( rp, &ch, 1 );
                        }
                    }
                }
            }

        //--- Suivante...
        DTI.line++; DTI.y += DTI.font->tf_YSize; DTI.top += DTI.font->tf_YSize; DTI.node++;
        }

    //------ met à jour le prop du bas
  END_PROP:
    zo->NewHPropLineLen = MAX(zo->MaxLineLen+PLUSCOLS,zo->TopCol+zo->Cols-1);
    if (DTI.clip)
        {
        UIK_UnclipObject( zo );
    UnlockLayer( zo->Obj.rp->Layer );
        }

    rp->Mask = mask;
}

void ozone_ReDisplay( struct AZObjZone *zo )
{
    ozone_Display( zo, zo->TopLine, zo->Rows, zo->TopCol, zo->Cols );
    UPDATE_STAT_PROP(AGP.FObj);
}

void ozone_DisplayBlock( struct AZObjZone *zo )
{
}

void ozone_DisplayCmd( struct AZObjZone *zo, struct AZDisplayCmd *dcmd )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG topline=zo->TopLine;

    VerifPosNum( zo );
    if (zo->TopLine - topline)
        {
        ozone_ReDisplay( zo );
        }
    else{
        HGetMaxLen( zo, zo->TopLine );
        if (dcmd->DisplayNum && dcmd->ScrollNum >= 0)
            {
            ozone_Display( zo, dcmd->DisplayFrom, dcmd->DisplayNum, 0, -1 );
            }
        if (dcmd->ScrollNum)
            {
            ozone_VScroll( zo, ABS(dcmd->ScrollNum), dcmd->ScrollNum, dcmd->ScrollFrom );
            }
        if (dcmd->DisplayNum && dcmd->ScrollNum < 0)
            {
            ozone_Display( zo, dcmd->DisplayFrom, dcmd->DisplayNum, 0, -1 );
            }
        }
    UPDATE_STAT_PROP(fo);
}

void ozone_ZoneTopLeft( struct AZObjZone *zo )
{
    zo->TopLine = zo->TopCol = 0;
}

void ozone_VScroll( struct AZObjZone *zo, ULONG abslen, LONG len, LONG from )
{
  struct RastPort *rp = zo->Obj.rp;
  struct Rectangle *r = &zo->Obj.ObjExt->InnerRect;
  LONG start, x, a=0, b=0, offs, miny, height;
  UWORD fh = zo->Obj.ObjExt->TTA.tta_YSize;
  UBYTE mask = rp->Mask;

    rp->Mask = AGP.FObj->Mask;
    if (abslen)
        {
        if (from == -1) from = AGP.FObj->Text.NodeNum - 1;

        offs = from - zo->TopLine;
        if (offs < 0) offs = 0;

        miny = r->MinY + offs * fh;
        x = abslen * fh;
        if ((height = r->MaxY - miny + 1 - x) > 0)
            {
            if (len > 0) a = x; else b = x;
            ClipBlit( rp, r->MinX, miny + a, rp, r->MinX, miny + b, r->MaxX-r->MinX+1, height, 0xC0 );
            }
        if (len > 0) abslen++; // pour écrire la dernière ligne coupée
        start = len > 0 ? zo->TopLine + (zo->Rows - 1) - len : zo->TopLine + offs;
        ozone_Display( zo, start, abslen, zo->TopCol, zo->Cols );
        }
    rp->Mask = mask;
}

void ozone_HScroll( struct AZObjZone *zo, ULONG abslen, LONG len, LONG from )
{
  struct RastPort *rp = zo->Obj.rp;
  struct Rectangle *r = &zo->Obj.ObjExt->InnerRect;
  LONG start, x, a=0, b=0, offs, minx, width;
  UWORD fw = zo->Obj.ObjExt->Font->tf_XSize;
  UBYTE mask = rp->Mask;

    rp->Mask = AGP.FObj->Mask;
    if (abslen)
        {
        if (from == -1) from = zo->MaxLineLen - 1;

        offs = from - zo->TopCol;
        if (offs < 0) offs = 0;

        minx = r->MinX + offs * fw;
        x = abslen * fw;
        if ((width = r->MaxX - minx + 1 - x) > 0)
            {
            if (len > 0) a = x; else b = x;
            ClipBlit( rp, minx + a, r->MinY, rp, minx + b, r->MinY, width, r->MaxY-r->MinY+1, 0xC0 );
            }
        if (len > 0) abslen++; // pour écrire la dernière ligne coupée
        start = len > 0 ? zo->TopCol + (zo->Cols-1) - len : zo->TopCol + offs;
        ozone_Display( zo, zo->TopLine, zo->Rows, start, abslen );
        }
    rp->Mask = mask;
}
