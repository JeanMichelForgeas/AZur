/****************************************************************
 *
 *      File:       funcs_div.c     fonctions diverses
 *      Project:    AZur            editeur de texte
 *
 *      Created:    05/08/93        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikobj.h"
#include "uiki:objreqstring.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"
#include "uiki:objstring.h"
#include "uiki:objwfilebut.h"
#include "uiki:objwfontbut.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"
#include "lci:_pool_protos.h"

#define UPBUFPTR(b) ((struct UIKPBuffer *) ((ULONG)b - sizeof(struct UIKPBuffer)))
#define UTABLESIZE(n)   (sizeof(struct UIKNode) * (n))


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern void StrHandler_HexDec();
extern void AutosaveFunc();


/****** Exported ************************************************/


/****** Statics *************************************************/

static UBYTE __far day_tab[2][13] =
   { {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
     {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31} };
static USHORT __far ndays[2] = { 365, 366 };


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

void __asm Text_FreeNodes( register __a0 struct UIKList *L, register __d0 LONG from, register __d1 LONG to )
{
  register struct UIKNode *node, *startnode;
  register int n;
  register LONG ind, nnum;
  register APTR pool = UPBUFPTR(AGP.FObj->Text.UNode)->Pool;
  struct UIKList *sublist;
  UWORD nil;

    if (! (nnum = L->NodeNum)) return;
    if (from < 0) from = nnum - 1;
    if (from >= nnum) return;
    if (to >= nnum || to < 0) to = nnum - 1;
    if (to < from) return;

    startnode = node = &L->UNode[ind=from];
    while (ind <= to)
        {
        if ((node->Flags & UNF_FOLDED) && (sublist = (struct UIKList *) BufGetUDatas( node->Buf, &nil )))
            {
            Text_FreeNodes( sublist, 0, -1 );
            BufFree( (APTR)sublist->UNode );
            AsmFreePooled( pool, sublist, sizeof(struct UIKPList), *(APTR*)4 );
            }
        BufFree( node->Buf );
        node++;
        ind++;
        }
    n = UTABLESIZE(nnum - 1 - to);
    if (n) UIK_MemCopy( (char*)node, (char*)startnode, n );
    L->NodeNum -= (to - from + 1);
}

//-------------------------------------------------

void PrintDebug( UBYTE *p1, UBYTE *p2, UBYTE *p3 )
{
    if (! AGP.DOSConsole)
        AGP.DOSConsole = Open( AGP.Prefs->DOSConsoleDesc, MODE_NEWFILE );

    if (AGP.DOSConsole)
        {
        if (p1) Write( AGP.DOSConsole, p1, StrLen(p1) );
        if (p2) Write( AGP.DOSConsole, p2, StrLen(p2) );
        if (p3) Write( AGP.DOSConsole, p3, StrLen(p3) );
        Write( AGP.DOSConsole, "\n", 1 );
        }
}

void __asm SET_MODIFIED( register __d0 LONG line, register __d1 LONG col )
{
  struct AZObjFile *fo = AGP.FObj;

    fo->Flags |= (AZFILEF_MODIFIED | AZFILEF_MODAUTS);
    fo->LCLine = line;
    fo->LCCol = col;
    fo->ModNum++;
    if (fo->ModNum >= AGP.Prefs->AutsIntMod)
        { fo->Flags |= AZFILEF_DOAUTOSAVE; fo->ModNum = 0; }
}

void __asm CLR_MODIFIED( void )
{
    AGP.FObj->Flags &= ~(AZFILEF_MODIFIED | AZFILEF_MODAUTS);
    AGP.FObj->ModNum = 0;
}

//-------------------------------------------------

void Month_Day( ULONG dat[], UBYTE *result )
{
  USHORT i;
  USHORT fleap, fqqq=dat[0]+1;  /* +1 pour pas avoir 0 janvier */
  USHORT fyear=1978;            /* le temps systeme == 0 pour le 1/1/78 */
  USHORT heu, fmin, tmin=(USHORT)dat[1];

    /*-----------------CALCUL DE LA DATE--------------------------
     *  fleap == 0 si fyear contient 365 jours
     *  fleap == 1 si fyear contient 366 jours
     *  fqqq  == nombre de jours restant. Si le nombre de jours
     *           restant est inferieur au nombre de jours dans
     *           l'annee, c'est le quantieme, fyear == annee:
     */
    heu = (USHORT)( ((USHORT)tmin)/((USHORT)60) );

    fleap = fyear%4 == 0 && fyear%100 !=0 || fyear%400 == 0;
    while (fqqq > ndays[fleap])
        {
        fyear++;
        fqqq -= ndays[fleap];
        fleap = fyear%4 == 0 && fyear%100 !=0 || fyear%400 == 0;
        }

    fmin = (USHORT)(((USHORT)tmin)-(heu*(USHORT)60));

    /*  fqqq == nombre de jours restant. Si le nombre de jours
     *          restant est inferieur au nombre de jours dans
     *          le mois, c'est le jour du mois, i == mois:
     */
    for (i=1; fqqq > day_tab[fleap][i]; i++) fqqq -= day_tab[fleap][i];

    /*-------------conversions ASCII----------------------------------*/
    sprintf( result, "%02ld/%02ld/%02ld %02.2ld:%02.2ld:%02.2ld", fqqq, i, (USHORT)(fyear % (USHORT)100), heu, fmin, dat[2]/50 );
}


//-------------------------------------------------

void SetCmdResultWith( struct CmdObj *co, UWORD errmsg, UBYTE *with )
{
    if (with)
        {
        UBYTE *buf, *ptr = UIK_LangString( AGP.UIK, errmsg );
        ULONG len = StrLen( ptr );
        if (buf = BufAllocP( AGP.Pool, len+2, 200 ))
            {
            buf[0] = ':'; buf[1] = ' '; StrCpy( buf+2, ptr );
            BufPasteS( with, StrLen(with), &buf, 0 );
            func_SetCmdResult( co, intAZ_GetErrorSeverity(errmsg), errmsg, buf );

            errmsg = 0;
            BufFree( buf );
            return;
            }
        }
    intAZ_SetCmdResult( co, errmsg );
}

//-----------------------------------------------------------------

void StoreLastCmd( struct CmdObj *co )
{
    if (co->Flags & AZAFF_OKINMACRO)
        {
        UBYTE *dest, *ptr = co->RegGerm->Names[0];
        ULONG len1, len2;

        len1 = StrLen(ptr);
        len2 = (co->PArg.CmdLine) ? StrLen(co->PArg.CmdLine) : 0;
        if (dest = BufResizeS( &AGP.LastCmd, len1 + len2 + 2 ))
            {
            StrCpy( dest, ptr );
            if (len2)
                {
                dest[len1] = ' ';
                StrCpy( dest + len1 + 1, co->PArg.CmdLine );
                }
            }
        }
}

//-----------------------------------------------------------------

void LayoutString( struct TextFont *font, struct AZLayout *out, ULONG userdata )
{
  UBYTE *ptr = UIK_LangString( AGP.UIK, out->Title );
  UWORD chars, max;

    chars = (out->UserParm >> 16); max = (out->UserParm & 0xffff);

    if (! out->Width)
        {
        if (out->UserData) out->Width = out->UserData; /* visibles*/
        else out->Width = chars;
        out->Width = 4 + (out->Width+1) * UIK_CharWidth( font, '8' );
        }
    if (AGP.WO->WinFlags & UIKW_SYSTEMLOOK) out->Width += 6;
    out->TotW = out->Width;
    if (ptr && *ptr)
        {
        for (; *ptr; ptr++)
            { if (*ptr != '_') out->TotW += UIK_CharWidth( font, *ptr ); }
        out->TotW += 4;
        }
    if (! out->Height) out->Height = font->tf_YSize + 5;
    out->TotH = MAX( out->Height, font->tf_YSize );
    out->UserData = userdata;
}

void LayoutGad( struct TextFont *font, struct AZLayout *out, ULONG userdata )
{
  UBYTE *ptr = UIK_LangString( AGP.UIK, out->Title );

    out->TotW = out->Width;
    if (ptr && *ptr)
        {
        for (; *ptr; ptr++)
            { if (*ptr != '_') out->TotW += UIK_CharWidth( font, *ptr ); }
        out->TotW += 4;
        }
    out->TotH = MAX( out->Height, font->tf_YSize );
    if (out->Id == AZLID_GADCHECK)
        { if (out->TotH > out->Height) out->Height = out->TotH; }
    out->UserData = userdata;
}

struct AZLayout *LayoutFrame( struct TextFont *font, struct AZLayout *out, ULONG userdata )
{
  struct AZLayout *in, *tmp;
  UBYTE *ptr = UIK_LangString( AGP.UIK, out->Title );
  WORD w, maxw=0, maxh, totheight=0, t=0;
  void (*func)();

    if (ptr)
        {
        //------ Calcul des dimensions et de la plus grande largeur
        for (in=out+1; in->Depth > out->Depth; in++)
            {
            switch (in->Id)
                {
                case AZLID_GADSPACE:  func = 0; break;
                case AZLID_FRAME:     func = (void*)LayoutFrame; break;
                case AZLID_INVISIBLE: func = 0; break;
                case AZLID_GADRADIO:  func = LayoutGad; break;
                case AZLID_GADCHECK:  func = LayoutGad; break;
                case AZLID_GADSTRNG:  func = LayoutString; break;
                case AZLID_GADSTNUM:  func = LayoutString; break;
                case AZLID_GADSTWFI:  func = LayoutString; break;
                case AZLID_GADSTFO1:  func = LayoutString; break;
                case AZLID_GADSTFO2:  func = LayoutString; break;
                default: func = 0; break;
                }
            if (func) (*func)( font, in, userdata );

            if (! (in->Flags & AZLF_SAMELINE)) w = in->TotW; // premier de ligne
            else w += (in->TotW + INTERGAD_WIDTH); // suivant sur même ligne
            if (w > maxw) maxw = w;
            }
        //------ Calcul de la hauteur Max par ligne et de la hauteur de la Frame
        t = 0;
        for (in=out+1; in->Depth > out->Depth; )
            {
            if (! (in->Flags & AZLF_SAMELINE))  // premier de ligne
                {
                ULONG diff, count;

                maxh = in->TotH;
                for (tmp=in+1, count=1; tmp->Depth == in->Depth && (tmp->Flags & AZLF_SAMELINE); tmp++)
                    { count++; if (tmp->TotH > maxh) maxh = tmp->TotH; }

                if (t == 0) t = font->tf_YSize;
                for (tmp=in; count--; tmp++)
                    {
                    if (diff = maxh - tmp->TotH) tmp->Top = t + diff / 2;
                    else tmp->Top = t;
                    if (diff = tmp->TotH - tmp->Height) tmp->Top += diff / 2;
                    }

                t += (maxh + 1);
                totheight += (maxh + 1);
                in = tmp; // next in
                }
            }
        //------ Mise à jour des champs de la Frame
        out->Width = out->TotW = MAX( INTERGAD_WIDTH + maxw + INTERGAD_WIDTH, UIK_StrWidth( font, ptr, StrLen(ptr) ) );
        out->TotH = totheight + 2 * font->tf_YSize + (font->tf_YSize / 4);
        out->Height = out->TotH - (font->tf_YSize / 2);
        out->UserData = userdata;
        }
    return( in );
}

struct AZLayout *LayoutPrepare( struct TextFont *font, struct AZLayout *in, ULONG userdata )
{
  struct AZLayout *new, *old;
  ULONG count, size;

    for (old=in, count=1; old->Id; old++, count++) ;
    size = count * sizeof(struct AZLayout);

    if (! (new = (struct AZLayout *) PoolAlloc( size ))) return(0);
    CopyMem( in, new, size );

    for (old=in=new; in->Id; )
        {
        in = LayoutFrame( font, in, userdata );
        if (in == old) return(0);
        old = in;
        }
    return( new );
}

struct AZLayout *LayoutWidthFrame( struct TextFont *font, struct AZLayout *out )
{
  struct AZLayout *in, *tmp;
  UBYTE *ptr = UIK_LangString( AGP.UIK, out->Title );
  WORD left;

    if (ptr)
        {
        for (in=out+1; in->Depth > out->Depth; )
            {
            left = INTERGAD_WIDTH;

            if (in->Flags & AZLF_SUB_EQUAL)
                {
                UWORD gadwidth=in->TotW, interwidth=0, count=0;

                for (tmp=in+1; tmp->Depth == in->Depth && (tmp->Flags & AZLF_SUB_EQUAL) && (tmp->Flags & AZLF_SAMELINE); tmp++)
                    { interwidth += INTERGAD_WIDTH; gadwidth += tmp->TotW; count++; }  // count est inférieur de 1

                if (gadwidth + interwidth < out->Width - 2*INTERGAD_WIDTH) interwidth = out->Width - 2*INTERGAD_WIDTH - gadwidth;
                interwidth /= count;
                for (tmp=in; count--; tmp++)    // on ne fait pas le dernier à cause de count -> tmp pointe sur dernier
                    { tmp->Left = left; left += (tmp->TotW + interwidth); }
                tmp->Left = out->Width - tmp->TotW - INTERGAD_WIDTH;
                in = tmp + 1; // next in
                }
            else{
                if (in->Flags & AZLF_SUB_RIGHT) in->Left = out->Width - in->TotW - INTERGAD_WIDTH;
                else { in->Left = left; left += (in->TotW + INTERGAD_WIDTH); }

                for (tmp=in+1; tmp->Depth == in->Depth && (tmp->Flags & AZLF_SAMELINE); tmp++)
                    {
                    if (tmp->Flags & AZLF_SUB_RIGHT) tmp->Left = out->Width - tmp->TotW - INTERGAD_WIDTH;
                    else { tmp->Left = left; left += (in->TotW + INTERGAD_WIDTH); }
                    }
                in = tmp;
                }
            }
        }
    return( in );
}

void LayoutWidth( struct TextFont *font, struct AZLayout *in )
{
  struct AZLayout *old;

    for (old=in; in->Id; )
        {
        in = LayoutWidthFrame( font, in );
        if (in == old) return;
        old = in;
        }
}

ULONG LayoutAddObjects( struct UIKObjReqString *rs, struct AZLayout *in, ULONG userulong1, ULONG userulong2 )
{
  struct UIKObj *list, *frame;
  ULONG titletag, handlertag;
  WORD left;

    if (! (list = UIK_GetObject( rs->Req, NULL, UIKID_LIST ))) goto END_ERROR;


    for (; in->Id; in++)
        {
        handlertag = TAG_IGNORE;
        titletag = TAG_IGNORE;
        left = in->Left;
        if (in->Flags & AZLF_TITLE_LEFT) { titletag = UIKTAG_OBJ_TitleFl_Left; left += (in->TotW - in->Width); }
        else if (in->Flags & AZLF_TITLE_RIGHT) titletag = UIKTAG_OBJ_TitleFl_Right;

        switch (in->Id)
            {
            case AZLID_GADSPACE:
                break;
            case AZLID_FRAME:
                if (! (frame = UIK_AddObjectTagsB( UIKBase, "UIKObj_Frame", list,
                        UIKTAG_OBJ_ParentRelative,  TRUE,
                        UIKTAG_OBJ_LeftTop,         W2L(left,in->Top),
                        UIKTAG_OBJ_WidthHeight,     W2L(in->Width,in->Height),
                        UIKTAG_OBJ_ActInactPens,    UIKCOLS(UIKCOL_GREY,UIKCOL_GREY,UIKCOL_WHITE,UIKCOL_WHITE),
                        UIKTAG_OBJ_Title,           in->Title,
                        UIKTAG_OBJ_BoxFl_Zoneable,  TRUE,
                        TAG_END ))) goto END_ERROR;
                break;
            case AZLID_INVISIBLE:
                break;
            case AZLID_GADRADIO:
                if (! (in->Obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_RadioExclude", frame,
                        UIKTAG_OBJ_ParentRelative,  TRUE,
                        UIKTAG_OBJ_LeftTop,         W2L(left,in->Top),
                        UIKTAG_OBJ_WidthHeight,     W2L(in->Width,in->Height),
                        UIKTAG_OBJ_Title,           in->Title,
                        titletag,                   TRUE,
                        UIKTAG_OBJ_ActInactPens,    UIKCOLS(UIKCOL_WHITE,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_BLACK),
                        UIKTAG_OBJ_TitleUnderscore, '_',
                        UIKTAG_OBJ_BoxFl_Over,      in->Flags & AZLF_BOX_ACTIVE ? FALSE : TRUE,
                        UIKTAG_OBJ_UserValue1,      userulong1,
                        UIKTAG_OBJ_UserValue2,      userulong2,
                        UIKTAG_OBJ_BoxFl_Disabled,  (in->Flags & AZLF_BOX_DISABLE) ? TRUE : FALSE,
                        TAG_END ))) goto END_ERROR;
                if (in->Func) if (! UIK_AddVector( in->Obj, (void*)in->Func, GADGETDOWN, (ULONG)in )) goto END_ERROR;
                break;
            case AZLID_GADCHECK:
                if (! (in->Obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_CheckToggle", frame,
                        UIKTAG_OBJ_ParentRelative,  TRUE,
                        UIKTAG_OBJ_LeftTop,         W2L(left,in->Top),
                        UIKTAG_OBJ_WidthHeight,     W2L(in->Width,in->Height),
                        UIKTAG_OBJ_Title,           in->Title,
                        titletag,                   TRUE,
                        UIKTAG_OBJ_ActInactPens,    UIKCOLS(UIKCOL_WHITE,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_BLACK),
                        UIKTAG_OBJ_TitleUnderscore, '_',
                        UIKTAG_OBJ_BoxFl_Over,      in->Flags & AZLF_BOX_ACTIVE ? FALSE : TRUE,
                        UIKTAG_OBJ_BoxFl_Filled,    TRUE,
                        UIKTAG_OBJ_UserValue1,      userulong1,
                        UIKTAG_OBJ_UserValue2,      userulong2,
                        UIKTAG_OBJ_BoxFl_NoPush,    TRUE,
                        UIKTAG_OBJ_BoxFl_Disabled,  (in->Flags & AZLF_BOX_DISABLE) ? TRUE : FALSE,
                        TAG_END ))) goto END_ERROR;
                if (in->Func) if (! UIK_AddVector( in->Obj, (void*)in->Func, GADGETUP, (ULONG)in )) goto END_ERROR;
                break;
            case AZLID_GADSTNUM:
                handlertag = UIKTAG_OBJStr_UserHandler;
            case AZLID_GADSTRNG:
                if (! (in->Obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_String", frame,
                        UIKTAG_OBJ_ParentRelative,  TRUE,
                        UIKTAG_OBJ_LeftTop,         W2L(left,in->Top),
                        UIKTAG_OBJ_WidthHeight,     W2L(in->Width,in->Height),
                        UIKTAG_OBJ_Title,           in->Title,
                        titletag,                   TRUE,
                        UIKTAG_OBJ_TitleUnderscore, '_',
                        UIKTAG_OBJ_UserValue1,      userulong1,
                        UIKTAG_OBJ_UserValue2,      (in->UserParm & 0xffff),
                        UIKTAG_OBJ_BoxFl_Disabled,  (in->Flags & AZLF_BOX_DISABLE) ? TRUE : FALSE,
                        UIKTAG_OBJStr_MaxChars,     (in->UserParm >> 16),
                        handlertag,                 StrHandler_HexDec,
                        UIKTAG_OBJStr_InactiveFunc, in->Func,
                        TAG_END ))) goto END_ERROR;
                break;
            case AZLID_GADSTWFI:
                { UBYTE sc, *p = UIK_LangString( AGP.UIK, in->Title );
                for (sc=0; *p; p++) { if (*p == '_') { sc = *(p+1); break; } }
                if (! (in->Obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_WFileButton", frame,
                        UIKTAG_OBJ_ParentRelative,  TRUE,
                        UIKTAG_OBJ_LeftTop,         W2L(left,in->Top),
                        UIKTAG_OBJ_WidthHeight,     W2L(in->Width,in->Height),
                        UIKTAG_OBJ_Title,           in->Title,
                        titletag,                   TRUE,
                        UIKTAG_OBJ_TitleUnderscore, '_',
                        UIKTAG_OBJ_ActInactPens,    UIKCOLS(UIKCOL_WHITE,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_BLACK),
                        UIKTAG_OBJ_UserValue1,      userulong1,
                        UIKTAG_OBJ_UserValue2,      userulong2,
                        UIKTAG_OBJ_BoxFl_Disabled,  (in->Flags & AZLF_BOX_DISABLE) ? TRUE : FALSE,
                        UIKTAG_WFB_SelectorTitle,   in->Title,
                        UIKTAG_WFBFl_DirsOnly,      ((in->UserParm & 0xffff) ? TRUE : FALSE),
                        UIKTAG_WFBFl_ReturnReturn,  TRUE,
                        UIKTAG_WFB_ButtonShortCut,  W2L(B2W(sc,0),0),
                        UIKTAG_WFB_ExtraSpace,      0,
                        TAG_END ))) goto END_ERROR;
                }
                break;
            case AZLID_GADSTFO1:
                if (! (in->Obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_WFontButton", frame,
                        UIKTAG_OBJ_ParentRelative,  TRUE,
                        UIKTAG_OBJ_LeftTop,         W2L(left,in->Top),
                        UIKTAG_OBJ_WidthHeight,     W2L(in->Width,in->Height),
                        UIKTAG_OBJ_Title,           in->Title,
                        titletag,                   TRUE,
                        UIKTAG_OBJ_ActInactPens,    UIKCOLS(UIKCOL_WHITE,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_BLACK),
                        UIKTAG_OBJ_UserValue1,      userulong1,
                        UIKTAG_OBJ_UserValue2,      userulong2,
                        UIKTAG_OBJ_BoxFl_Disabled,  (in->Flags & AZLF_BOX_DISABLE) ? TRUE : FALSE,
                        UIKTAG_WFOB_SelectorTitle,  TEXT_FontInterface,
                        UIKTAG_WFOSFl_FixedWidth,   TRUE,
                        UIKTAG_WFOSFl_PropWidth,    TRUE,
                        TAG_END ))) goto END_ERROR;
                break;
            case AZLID_GADSTFO2:
                if (! (in->Obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_WFontButton", frame,
                        UIKTAG_OBJ_ParentRelative,  TRUE,
                        UIKTAG_OBJ_LeftTop,         W2L(left,in->Top),
                        UIKTAG_OBJ_WidthHeight,     W2L(in->Width,in->Height),
                        UIKTAG_OBJ_Title,           in->Title,
                        titletag,                   TRUE,
                        UIKTAG_OBJ_ActInactPens,    UIKCOLS(UIKCOL_WHITE,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_BLACK),
                        UIKTAG_OBJ_UserValue1,      userulong1,
                        UIKTAG_OBJ_UserValue2,      userulong2,
                        UIKTAG_OBJ_BoxFl_Disabled,  (in->Flags & AZLF_BOX_DISABLE) ? TRUE : FALSE,
                        UIKTAG_WFOB_SelectorTitle,  TEXT_FontView,
                        UIKTAG_WFOSFl_FixedWidth,   TRUE,
                        UIKTAG_WFOSFl_PropWidth,    FALSE,
                        TAG_END ))) goto END_ERROR;
                break;
            }
        }
    return(1);

  END_ERROR:
    return(0);
}

//--------------------------------------------------------------------

struct StoreVar {
    struct SignalSemaphore Sema;
    UBYTE   *Contents;
    UBYTE   Name[1];
    };

static UBYTE ** __asm find_var( register __a0 UBYTE *findname )
{
  struct UIKList *L = &AGP.AZMast->VarList;
  struct UIKNode *node;
  LONG ind = -1;

    node = L->UNode;
    while (++ind < L->NodeNum)
        {
        if (StrNCCmp( findname, ((struct StoreVar *)node->Buf)->Name ) == 0)
            return( &node->Buf );
        node++;
        }
    return(0);
}

//----------------------

void func_FreeVarList()
{
  struct UIKList *L = &AGP.AZMast->VarList;
  struct UIKNode *node;
  LONG ind = -1;

    node = L->UNode;
    while (++ind < L->NodeNum)
        {
        BufFree( ((struct StoreVar *)node->Buf)->Contents );
        node++;
        }
    UIK_nl_FreeList( L );
}

//----------------------

void func_GetVarList( UBYTE **pub )
{
  struct AZSema *sem = AGP.AZMast->VarSema;
  struct UIKList *L = &AGP.AZMast->VarList;
  struct StoreVar *sv;
  struct UIKNode *node;
  LONG len, ind = -1;

    BufTruncate( *pub, 0 );
    intAZ_SemaRead( sem );
    node = L->UNode;
    while (++ind < L->NodeNum)
        {
        sv = (struct StoreVar *)node->Buf;
        //len = StrLen( sv->Name );
        if (! func_BufPrintf( pub, BufLength(*pub), "\"%ls\",", sv->Name )) break;
        node++;
        }
    if (len = BufLength(*pub))
        {
        if ((*pub)[len-1] == ',') BufTruncate( *pub, len-1 );
        }
    intAZ_SemaFreeRead( sem );
}

APTR func_VarNew( UBYTE *name, ULONG size )
{
  struct AZSema *sem = AGP.AZMast->VarSema;
  struct UIKList *L = &AGP.AZMast->VarList;
  ULONG len = StrLen( name );
  UBYTE **pptr, *ptr=0;
  struct StoreVar *sv;

    intAZ_SemaWrite( sem );
    if (pptr = find_var( name ))
        {
        ptr = (APTR)-1; // existe déjà
        }
    else{
        if (ptr = BufAlloc( 10, size, MEMF_PUBLIC|MEMF_ANY ))
            {
            if (sv = (struct StoreVar *) UIK_nl_AllocNode( L, -1, sizeof(struct StoreVar) + len, 0 ))
                {
                InitSemaphore( &sv->Sema );
                ObtainSemaphore( &sv->Sema );
                StrCpy( sv->Name, name ); // après le Obtain pour éviter qu'on la réserve avant
                sv->Contents = ptr;
                }
            else BufFreeS( &ptr ); // ptr = 0;
            }
        }
    intAZ_SemaFreeWrite( sem );
    return( ptr );
}

APTR func_VarLock( UBYTE *name )
{
  struct AZSema *sem = AGP.AZMast->VarSema;
  struct StoreVar *sv;
  UBYTE **pptr, *ptr=0;

    intAZ_SemaRead( sem );
    if (pptr = find_var( name ))
        {
        sv = (struct StoreVar *) *pptr;
        ObtainSemaphore( &sv->Sema );
        if (! (ptr = sv->Contents)) ReleaseSemaphore( &sv->Sema ); // à cause de func_VarDispose()
        else StrCpy( sv->Name, name ); // après le Obtain pour éviter qu'on la réserve avant
        }
    intAZ_SemaFreeRead( sem );
    return( ptr );
}

APTR func_VarLockResize( UBYTE *name, ULONG newsize )
{
  struct AZSema *sem = AGP.AZMast->VarSema;
  struct StoreVar *sv;
  UBYTE **pptr, *ptr=(APTR)-1;

    intAZ_SemaRead( sem );
    if (pptr = find_var( name ))
        {
        sv = (struct StoreVar *) *pptr;
        ObtainSemaphore( &sv->Sema );
        if (ptr = sv->Contents)     // à cause de func_VarDispose()
            ptr = BufResizeS( &sv->Contents, newsize );
        if (! ptr)
            ReleaseSemaphore( &sv->Sema );
        else StrCpy( sv->Name, name ); // après le Obtain pour éviter qu'on la réserve avant
        }

    intAZ_SemaFreeRead( sem );
    return( ptr );
}

void func_VarUnlock( UBYTE *name )
{
  struct AZSema *sem = AGP.AZMast->VarSema;
  struct StoreVar *sv;
  UBYTE **pptr;

    intAZ_SemaRead( sem );
    if (pptr = find_var( name ))
        {
        sv = (struct StoreVar *) *pptr;
        ReleaseSemaphore( &sv->Sema );
        }
    intAZ_SemaFreeRead( sem );
}

static void cleanup_varlist()
{
  struct UIKList *L = &AGP.AZMast->VarList;
  LONG ind = L->NodeNum;

    while (--ind >= 0)
        {
        if (((struct StoreVar *)L->UNode[ind].Buf)->Name[0] == 0)
            UIK_nl_FreeNode( L, ind );
        }
}

void func_VarDispose( UBYTE *name )
{
  struct AZSema *sem = AGP.AZMast->VarSema;
  struct StoreVar *sv;
  UBYTE **pptr;

    intAZ_SemaRead( sem );
    if (pptr = find_var( name ))
        {
        sv = (struct StoreVar *) *pptr;
        ObtainSemaphore( &sv->Sema );
        BufFreeS( &sv->Contents );
        sv->Name[0] = 0;
        ReleaseSemaphore( &sv->Sema );
        intAZ_SemaFreeRead( sem );

        intAZ_SemaWrite( sem );
        cleanup_varlist();
        intAZ_SemaFreeWrite( sem );
        }
    else intAZ_SemaFreeRead( sem );
}

//--------------------------------------------------------------------

void func_EnvFree( APTR getresult )
{
    UIK_MemSpFree( getresult );
}

APTR func_EnvGet( STRPTR prefname )
{
  ULONG lock, oldlock, ok, file, prefsize;
  APTR pref;

    if (! (lock = Lock( "ENV:AzurEditor/Commands", ACCESS_READ ))) goto END_ERROR;

    oldlock = CurrentDir( lock );
    ok = 0;
    if (prefsize = lo_FileSize( prefname ))
        if (pref = UIK_MemSpAlloc( prefsize, MEMF_PUBLIC ))
            if (file = Open( prefname, MODE_OLDFILE ))
                {
                if (Read( file, pref, prefsize ) == prefsize) ok = 1;
                Close( file );
                }
    CurrentDir( oldlock );
    UnLock( lock );
    if (! ok) goto END_ERROR;

    return( pref );

  END_ERROR:
    return(0);
}

ULONG func_EnvSet( STRPTR prefname, APTR pref, ULONG prefsize )
{
  ULONG old, lock, oldlock, ok, file;

    old = UIK_DisableDOSReq();
    if (! lo_DirCreate( "ENV:AzurEditor/Commands" )) goto END_ERROR;
    if (! lo_DirCreate( "ENVARC:AzurEditor/Commands" )) goto END_ERROR;
    UIK_EnableDOSReq( 0, old );

    if (! (lock = Lock( "ENV:AzurEditor/Commands", ACCESS_READ ))) goto END_ERROR;
    oldlock = CurrentDir( lock );
    ok = 0;
    if (file = Open( prefname, MODE_NEWFILE ))
        {
        if (Write( file, pref, prefsize ) == prefsize) ok = 1;
        Close( file );
        }
    CurrentDir( oldlock );
    UnLock( lock );
    if (! ok) goto END_ERROR;

    if (! (lock = Lock( "ENVARC:AzurEditor/Commands", ACCESS_READ ))) goto END_ERROR;
    oldlock = CurrentDir( lock );
    ok = 0;
    if (file = Open( prefname, MODE_NEWFILE ))
        {
        if (Write( file, pref, prefsize ) == prefsize) ok = 1;
        Close( file );
        }
    CurrentDir( oldlock );
    UnLock( lock );
    if (! ok) goto END_ERROR;

    return(1);

  END_ERROR:
    return(0);
}

//--------------------------------------------------------------------

/*
ULONG func_VerifAllMem()
{
  struct MemHeader *header;
  struct MemChunk *chunk;
  ULONG totsize, chunksize, headersize;

    Forbid();

    //while (header = (struct MemHeader *) header->mh_Node.ln_Succ)
    for (header = (struct MemHeader *)ExecBase->MemList.lh_Head; header->mh_Node.ln_Succ; header = (struct MemHeader *)header->mh_Node.ln_Succ)
        {
        if (chunk = header->mh_First)
            {
            totsize = 0;
            headersize = (ULONG)header->mh_Upper - (ULONG)header->mh_Lower;

            while (chunk)
                {
                if ((ULONG)chunk < (ULONG)header->mh_Lower)
                    { kprintf( "*** %08.8lx - Chunk addr < mh_Lower : %lx %lx\n", (ULONG)header->mh_Attributes, chunk, header->mh_Lower ); goto END_ERROR; }
                if ((ULONG)chunk > (ULONG)header->mh_Upper)
                    { kprintf( "*** %08.8lx - Chunk addr > mh_Upper : %lx %lx\n", (ULONG)header->mh_Attributes, chunk, header->mh_Upper ); goto END_ERROR; }

                chunksize = chunk->mc_Bytes;
                if (chunksize > headersize)
                    { kprintf( "*** %08.8lx - Chunk size > header size : %ld %ld\n", (ULONG)header->mh_Attributes, chunksize, headersize ); goto END_ERROR; }

                totsize += chunksize;
                chunk = chunk->mc_Next;
                }

            if (totsize != header->mh_Free)
                { kprintf( "*** %08.8lx - Sizes do not match: %ld %ld\n", (ULONG)header->mh_Attributes, header->mh_Free, totsize ); goto END_ERROR; }
            }
        else { kprintf( "*** %08.8lx - Empty Header list...\n", (ULONG)header->mh_Attributes ); goto END_ERROR; }
        }

    Permit();
    return(1);

  END_ERROR:
    Permit();
    DisplayBeep( NULL );
    return(0);
}
*/

//--------------------------------------------------------------------

ULONG func_VerifyOff()
{
  struct AZObjFile *fo = AGP.FObj;
  struct Window *w;
  ULONG locks=0;

    if (fo && fo->WO)
        {
        if (w = fo->WO->Obj.w)
            {
            struct UIKObj *size_obj;
            if (size_obj = UIK_GetObject( fo->WO, NULL, UIKID_SIZEGAD ))
                UIK_Stop( size_obj );
            /*
            if (w->Flags & RMBTRAP) locks |= 1;
            if (TST_INPUTLOCK) locks |= 2;
            w->Flags |= RMBTRAP;
            SET_INPUTLOCK;
            */
            ModifyIDCMP( w, w->IDCMPFlags & ~(/*IDCMP_RAWKEY|*/IDCMP_REQVERIFY|IDCMP_MENUVERIFY|IDCMP_SIZEVERIFY) );
            }
        }
    return( locks );
}

void func_VerifyOn( ULONG locks )
{
  struct AZObjFile *fo = AGP.FObj;
  struct Window *w;

    if (fo && fo->WO)
        {
        if (w = fo->WO->Obj.w)
            {
            struct UIKObj *size_obj;

            if (size_obj = UIK_GetObject( fo->WO, NULL, UIKID_SIZEGAD ))
                UIK_Start( size_obj );
            /*
            if (! (locks & 1)) w->Flags &= ~RMBTRAP;
            if (! (locks & 2)) CLR_INPUTLOCK;
            */
            ModifyIDCMP( w, w->IDCMPFlags | (/*IDCMP_RAWKEY|*/IDCMP_REQVERIFY|IDCMP_MENUVERIFY|IDCMP_SIZEVERIFY) );
            }
        }
}
