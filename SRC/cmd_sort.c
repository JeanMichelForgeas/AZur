/****************************************************************
 *
 *      Project:   AZUR
 *      Function:  objets
 *
 *      Created:   17/04/93 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:objfile.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"

/* this macro lets us long-align structures on the stack */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((LONG)(a_##name+3) & ~3);


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/


/****************************************************************
 *
 *      Objets
 *
 ****************************************************************/

//»»»»»» Cmd_Sort   SL=STARTLINE/N,EL=ENDLINE/N,SC=STARTCOL/N,EC=ENDCOL/N,GL=GROUPLINES/N,GO=GROUPOFFSET/N,ALL/S,CASE/S,ACCENT/S,DATE/S,DS=DESCENDING/S

/**   1    s1 > s2
 **   0    s1 = s2
 **  -1    s2 > s1
 **/

static int cmp_noaccent_case( UBYTE *ptr1, UBYTE *ptr2, ULONG len )
{
  register UBYTE c1, c2;

    for (; len; len--)
        {
        c1 = *(ptr1++); c2 = *(ptr2++);
        if (c1 & 0x80) c1 = func_CharStripAccent( c1, 0 );
        if (c2 & 0x80) c2 = func_CharStripAccent( c2, 0 );
        if (c1 > c2) return(1); else if (c1 < c2) return(-1);
        }
    return(0); /* equal */
}

static int cmp_accent_case( UBYTE *ptr1, UBYTE *ptr2, ULONG len )
{
  register UBYTE c1, c2;

    for (; len; len--)
        {
        c1 = *(ptr1++); c2 = *(ptr2++);
        if (c1 > c2) return(1); else if (c1 < c2) return(-1);
        }
    return(0); /* equal */
}

static int cmp_noaccent_nocase( UBYTE *ptr1, UBYTE *ptr2, ULONG len )
{
  register UBYTE c1, c2;

    for (; len; len--)
        {
        c1 = *(ptr1++); c2 = *(ptr2++);
        if (c1 & 0x80) c1 = func_CharStripAccent( c1, 0 );
        if (c2 & 0x80) c2 = func_CharStripAccent( c2, 0 );
        c1 = UIK_ToUpper( c1 );
        c2 = UIK_ToUpper( c2 );
        if (c1 > c2) return(1); else if (c1 < c2) return(-1);
        }
    return(0); /* equal */
}

static int cmp_accent_nocase( UBYTE *ptr1, UBYTE *ptr2, ULONG len )
{
  register UBYTE c1, c2;

    for (; len; len--)
        {
        c1 = *(ptr1++); c2 = *(ptr2++);
        c1 = UIK_ToUpper( c1 );
        c2 = UIK_ToUpper( c2 );
        if (c1 > c2) return(1); else if (c1 < c2) return(-1);
        }
    return(0); /* equal */
}

static int cmp_date( UBYTE *ptr1, UBYTE *ptr2, ULONG len )
{
  ULONG day1, month1, year1;
  ULONG day2, month2, year2;

    day1   = Str2Num( ptr1 ); ptr1 = (UBYTE *) __builtin_getreg( 8 );
    month1 = Str2Num( ptr1 ); ptr1 = (UBYTE *) __builtin_getreg( 8 );
    year1  = Str2Num( ptr1 ); ptr1 = (UBYTE *) __builtin_getreg( 8 );
    day2   = Str2Num( ptr2 ); ptr2 = (UBYTE *) __builtin_getreg( 8 );
    month2 = Str2Num( ptr2 ); ptr2 = (UBYTE *) __builtin_getreg( 8 );
    year2  = Str2Num( ptr2 ); ptr2 = (UBYTE *) __builtin_getreg( 8 );

    if (year1 == year2)
        {
        if (month1 == month2)
            {
            if (day1 == day2) return(0);
            else return( (day1 > day2) ? 1 : -1 );
            }
        else return( (month1 > month2) ? 1 : -1 );
        }
    else return( (year1 > year2) ? 1 : -1 );
}

/** Combsort, by Richard Box and Stephen Lacey, Byte 4/91, p.315
 **/

#define SORT_ACCENT 1
#define SORT_CASE   2
#define SORT_DATE   4
#define SORT_DESC   8

static ULONG CombSort( struct UIKNode *tab, ULONG size, LONG startcol, LONG endcol, ULONG grplines, ULONG grpoffset, int (*cmpfunc)(), ULONG flags )
{
  ULONG gap, flag, i, j, max;
  struct UIKNode ah;

    size = size / grplines;
    gap = size;
    flag = -1;
    while (flag || (gap != 1))
        {                       // int(gap/1.3);
        if (gap < (1<<16)) gap = (gap << 15) / 42598;  // 1.3 * (1<<15)
        else if (gap < (1<<20)) gap = (gap << 11) / 2662;  // 1.3 * (1<<11)
        else if (gap < (1<<24)) gap = (gap << 7) / 166;  // 1.3 * (1<<7)
        else if (gap < (1<<28)) gap = (gap << 3) / 10;  // 1.3 * (1<<3)
        else gap = gap / 2;

        if (gap < 1) gap = 1;
        if (gap == 9 || gap == 10) gap = 11;
        flag = 0;
        max = size - gap;
        for (i=0; i < max; i++)
            {
            UBYTE *ptr1, *ptr2;
            ULONG len, len1, len2, k;
            LONG scol=startcol, ecol=endcol, linei, linej;
            int res;

            j = i + gap;
            linei = i * grplines;
            linej = j * grplines;

            len1 = intAZ_Node_Buf_Len( linei+grpoffset, tab, &ptr1 );
            len2 = intAZ_Node_Buf_Len( linej+grpoffset, tab, &ptr2 );

            len = MIN(len1,len2);
            if (scol > len) scol = len;
            if (ecol == -1 || ecol > len) ecol = len;
            len = ecol - scol + 1;

            res = (*cmpfunc)( ptr1+scol, ptr2+scol, len );
            if ((res > 0 && !(flags & SORT_DESC)) || (res < 0 && (flags & SORT_DESC)))
                {
                flag = -1;
                for (k=0; k < grplines; k++)
                    {
                    func_BMSwapLines( linei+k, linej+k );
                    ah = tab[linei+k]; tab[linei+k] = tab[linej+k]; tab[linej+k] = ah;
                    }
                }
            }
        }
    return( flag );
}

ULONG func_TextSort( LONG sline, LONG eline, LONG scol, LONG ecol, ULONG grplines, ULONG grpoffset, int (*cmpfunc)(), ULONG flags, struct AZDisplayCmd *dinfo )
{
  struct AZObjFile *fo = AGP.FObj;
  ULONG numlines;

    if (dinfo)
        {
        dinfo->DisplayNum  = 0;
        dinfo->ScrollNum   = 0;
        }
    if (eline == -1) eline = fo->Text.NodeNum;
    if (eline > fo->Text.NodeNum - 1) eline = fo->Text.NodeNum - 1;

    if (scol<0 || (ecol<0 && ecol != -1) || sline<0 || eline<0)
        return( TEXT_ERR_BadValue );

    numlines = eline - sline + 1;

    if (cmpfunc == 0)
        {
        if (flags & SORT_DATE)        cmpfunc = cmp_date;
        else if (flags & SORT_ACCENT) cmpfunc = (flags & SORT_CASE) ? cmp_accent_case : cmp_accent_nocase;
        else                          cmpfunc = (flags & SORT_CASE) ? cmp_noaccent_case : cmp_noaccent_nocase;
        }

    if (grplines > 1)   // vérifications sur startline et endline
        {
        if (numlines % grplines) return( TEXT_ERR_BadValue );
        }

    if (CombSort( &fo->Text.UNode[sline], numlines, scol, ecol, grplines, grpoffset, cmpfunc, flags ))
        SET_MODIFIED( sline + numlines - 1, ecol );

    if (dinfo)
        {
        dinfo->DisplayNum  = numlines;
        dinfo->DisplayFrom = sline;
        }
    return(0); // no error
}

#define ARG_STARTLINE   0
#define ARG_ENDLINE     1
#define ARG_STARTCOL    2
#define ARG_ENDCOL      3
#define ARG_GROUPLINES  4
#define ARG_GROUPOFFS   5
#define ARG_ALL         6
#define ARG_CASE        7
#define ARG_ACCENT      8
#define ARG_DATE        9
#define ARG_DESCENDING 10

void CmdFunc_Sort_Do( struct CmdObj *co )
{
  struct AZDisplayCmd dinfo;
  LONG scol, ecol, sline, eline;
  ULONG grplines, grpoffset, block, numblocks, errmsg, flags;
  struct AZBlock *azb;

    if (co->ArgRes[ARG_GROUPLINES]) grplines = *((ULONG*)co->ArgRes[ARG_GROUPLINES]);
    else grplines = 1;
    if (co->ArgRes[ARG_GROUPOFFS]) grpoffset = *((ULONG*)co->ArgRes[ARG_GROUPOFFS]);
    else grpoffset = 0;

    flags = 0;
    if (co->ArgRes[ARG_DATE])       flags |= SORT_DATE;
    if (co->ArgRes[ARG_ACCENT])     flags |= SORT_ACCENT;
    if (co->ArgRes[ARG_CASE])       flags |= SORT_CASE;
    if (co->ArgRes[ARG_DESCENDING]) flags |= SORT_DESC;

    if (!co->ArgRes[ARG_ALL] && (numblocks = func_BlockExists()))
        {
        for (block=numblocks; block > 0; block--) // on part du haut
            {
            azb = func_BlockPtr( block );
            sline = azb->LineStart;
            scol  = azb->ColStart;
            eline = azb->LineEnd;
            ecol  = azb->ColEnd;

            if (azb->Mode != BLKMODE_RECT && azb->Mode != BLKMODE_FULL) { scol = 0; ecol = -1; }

            func_BlockRemove( block, 0, 0 ); // avant toute modif
            if (errmsg = func_TextSort( sline, eline, scol, ecol, grplines, grpoffset, 0, flags, &dinfo ))
                { intAZ_SetCmdResult( co, errmsg ); return; }
            func_DisplayCmd( &dinfo );
            }
        }
    else{
        sline = eline = 0;

        if (co->ArgRes[ARG_ALL])
            { sline = 1; eline = -1; scol = 1; ecol = -1; }

        if (co->ArgRes[ARG_STARTCOL])  scol  = *((ULONG*)co->ArgRes[ARG_STARTCOL]);
        else scol = 1;
        if (co->ArgRes[ARG_ENDCOL])    ecol  = *((ULONG*)co->ArgRes[ARG_ENDCOL]);
        else ecol = -1;
        if (co->ArgRes[ARG_STARTLINE]) sline = *((ULONG*)co->ArgRes[ARG_STARTLINE]);
        if (co->ArgRes[ARG_ENDLINE])   eline = *((ULONG*)co->ArgRes[ARG_ENDLINE]);

        scol--;
        if (ecol != -1) ecol--;
        sline--;
        if (eline != -1) eline--;

        if (errmsg = func_TextSort( sline, eline, scol, ecol, grplines, grpoffset, 0, flags, &dinfo ))
            intAZ_SetCmdResult( co, errmsg );
        func_DisplayCmd( &dinfo );
        }
}

//»»»»»» Cmd_Match  FULL/S,NC=NOCURS/S,PAIRS/A/F

#define ARG_FULL    0
#define ARG_NOCURS  1
#define ARG_PAIRS   2

#define NOMODE     -1
#define BACKMODE    0
#define NEXTMODE    1

struct PairElement {
    ULONG ElLen;
    UBYTE *ElPtr;
    };
typedef struct PairElement PE;

struct matchinfo {
    LONG  Line, Col;
    UBYTE *Ptr;
    UBYTE *PtrEnd;
    UBYTE *PtrStart;
    struct PairElement *Pairs;
    ULONG Count;
    struct UIKPList *UL;
    };

ULONG CharMatch_Next( struct matchinfo *mi, PE *pe1, PE *pe2 )
{
  ULONG count=0, rc=0;

    while (TRUE)
        {
        while (mi->Ptr < mi->PtrEnd)
            {
            if (func_MatchStr( pe1->ElPtr, mi->Ptr, mi->PtrStart ))  // if (*mi->Ptr == c1)
                {
                count++;
                }
            else if (func_MatchStr( pe2->ElPtr, mi->Ptr, mi->PtrStart ))  // if (*mi->Ptr == c2)
                {
                if (--count == 0) return(1);
                }
            else{
                ULONG num;
                PE *pe;
                for (rc=1, num=0, pe=mi->Pairs; num < mi->Count; num++, pe+=2)
                    {
                    if (func_MatchStr( pe[0].ElPtr, mi->Ptr, mi->PtrStart ))
                        {
                        rc = CharMatch_Next( mi, pe, pe+1 );
                        break;
                        }
                    else if (func_MatchStr( pe[1].ElPtr, mi->Ptr, mi->PtrStart ))
                        { rc = 0; break; }
                    }
                if (! rc) return(0);
                }
            mi->Ptr++;
            }
        if (mi->Line+1 >= mi->UL->NodeNum) return( (ULONG)(count ? 0 : 1) );
        mi->Line++;
        mi->PtrStart = mi->Ptr = intAZ_LineBuf(mi->Line);
        mi->PtrEnd = mi->Ptr + intAZ_LineBufLen(mi->Ptr);
        }
}

ULONG CharMatch_Back( struct matchinfo *mi, PE *pe1, PE *pe2 )
{
  ULONG count=0, rc=0;

    while (TRUE)
        {
        while (mi->Ptr >= mi->PtrEnd)
            {
            if (func_MatchStr( pe1->ElPtr, mi->Ptr, mi->PtrEnd ))  // if (*mi->Ptr == c1)
                {
                count++;
                }
            else if (func_MatchStr( pe2->ElPtr, mi->Ptr, mi->PtrEnd ))  // if (*mi->Ptr == c2)
                {
                if (--count == 0) return(1);
                }
            else{
                ULONG num;
                PE *pe;
                for (rc=1, num=0, pe=mi->Pairs; num < mi->Count; num++, pe+=2)
                    {
                    if (func_MatchStr( pe[1].ElPtr, mi->Ptr, mi->PtrEnd )) { rc = CharMatch_Back( mi, pe+1, pe ); break; }
                    else if (func_MatchStr( pe[0].ElPtr, mi->Ptr, mi->PtrEnd )) { rc = 0; break; }
                    }
                if (! rc) return(0);
                }
            mi->Ptr--;
            }
        if (mi->Line-1 < 0) return( (ULONG)(count ? 0 : 1) );
        mi->Line--;
        mi->PtrEnd = intAZ_LineBuf(mi->Line);
        mi->Ptr = mi->PtrEnd + intAZ_LineBufLen(mi->PtrEnd) - 1;
        }
}

static ULONG MatchOne( struct matchinfo *mi )
{
  struct AZObjFile *fo = AGP.FObj;
  UBYTE *str;
  ULONG mode, rc=0, num, len;
  PE *pe;

    mi->Line = fo->Line;
    mi->Col = fo->Col;
    mi->PtrStart = mi->Ptr = intAZ_LineBuf( mi->Line );     /* start */
    len = intAZ_LineBufLen( mi->Ptr );
    mi->PtrEnd = mi->Ptr + len;

    str = &mi->Ptr[mi->Col];
    for (num=0, pe=mi->Pairs; num < mi->Count; num++, pe+=2)
        {
        if (func_IsNCSubStr( pe[0].ElPtr, str )) { mode = NEXTMODE; break; }
        if (func_IsNCSubStr( pe[1].ElPtr, str )) { mode = BACKMODE; break; }
        }
    if (num >= mi->Count && fo->Col)
        {
        mi->Col = fo->Col - 1;
        str = &mi->Ptr[mi->Col];
        for (num=0, pe=mi->Pairs; num < mi->Count; num++, pe+=2)
            {
            if (func_IsNCSubStr( pe[0].ElPtr, str )) { mode = NEXTMODE; break; }
            if (func_IsNCSubStr( pe[1].ElPtr, str )) { mode = BACKMODE; break; }
            }
        }
    if (num >= mi->Count) { mi->Col = fo->Col; return( rc ); }

    if (mode == NEXTMODE)
        {
        mi->PtrEnd = mi->Ptr + intAZ_LineLen( mi->Line );   /* end   */
        mi->Ptr += mi->Col;                                 /* from  */
        rc = CharMatch_Next( mi, pe, pe+1 );
        mi->Col = mi->Ptr - intAZ_LineBuf( mi->Line );
        }
    else{
        mi->PtrEnd = mi->Ptr;                               /* end   */
        mi->Ptr += mi->Col;                                 /* from  */
        rc = CharMatch_Back( mi, pe+1, pe );
        mi->Col = mi->Ptr - mi->PtrEnd;
        }
    if (! rc)
        {
        if (mi->Line == fo->Text.NodeNum-1 && mi->Col == intAZ_LineLen( mi->Line ))
            { mi->Line = fo->Line; mi->Col = fo->Col; }
        DisplayBeep( NULL );
        }
    return( rc );
}

static ULONG MatchFull( struct matchinfo *mi )
{
  struct AZObjFile *fo = AGP.FObj;
  ULONG rc=0;
  PE imp={1,"\n"};

    mi->Line = mi->Col = 0;
    mi->Ptr = intAZ_LineBuf( 0 );             /* start */
    mi->PtrEnd  = mi->Ptr + intAZ_LineLen( 0 );  /* end   */
    rc = CharMatch_Next( mi, &imp, &imp ); // chaînes impossibles à rencontrer
    if (! rc)
        {
        if (mi->Line == fo->Text.NodeNum-1 && mi->Col == intAZ_LineLen( mi->Line ))
            { mi->Line = fo->Line; mi->Col = fo->Col; }
        else mi->Col = mi->Ptr - intAZ_LineBuf( mi->Line );
        DisplayBeep( NULL );
        }
    return( rc );
}

void CmdFunc_Match_Do( struct CmdObj *co )
{
  struct PairElement pel;
  ULONG len, count, rc=0;
  UBYTE *start, *end, *p, *ubuf=0, *ulp=0, quoting;
  D_S(struct matchinfo,mi);

    p = co->ArgRes[ARG_PAIRS];
    len = StrLen( p );
    ubuf = BufAlloc( 4, len, MEMF_ANY );
    ulp = BufAlloc( 4*6, 0, MEMF_ANY );
    if (!ulp || !ubuf) { intAZ_SetCmdResult( co, TEXT_NOMEMORY ); goto END_ERROR; }
    CopyMem( p, ubuf, len );
    UIK_StrToUpper( ubuf );

    p = ubuf;
    count = 0;
    while (*p)        // boucle de recherche de mots
        {
        //--- cherche début
        p += skip_char( p, ' ', '\t' ); if (! *p) break;
        if (quoting = ((*p == '\"' || *p == 0x27) ? *p : 0)) { if (! *++p) break; }
        start = p;

        //--- cherche fin
        if (quoting)
            {
            p += find_char( p, quoting, quoting );
            end = p;
            if (*p) { *p++ = 0; p += find_char( p, ' ', '\t' ); }
            }
        else{
            p += find_char( p, ' ', '\t' );
            end = p;
            if (*p) *p++ = 0;
            }

        //--- range un parm de plus
        if (end > start)
            {
            pel.ElLen = end - start;
            pel.ElPtr = start;
            if (! BufPasteS( (APTR)&pel, sizeof(struct PairElement), &ulp, count * sizeof(struct PairElement) )) break;
            count++;
            }
        }
    if (count == 0 || count & 1) { intAZ_SetCmdResult( co, TEXT_ERR_MatchPairs ); goto END_ERROR; }

/*
mi->Pairs = (struct PairElement *)ulp;
for (len=0; len<count; len++)
    {
    kprintf( "count=%ld : ", count );
    kprintf( "(%ld)'%ls', ", mi->Pairs->ElLen, mi->Pairs->ElPtr );
    kprintf( "\n" );
    mi->Pairs++;
    }
*/
    mi->Pairs = (struct PairElement *)ulp;
    mi->Count = count / 2;
    mi->UL = &AGP.FObj->Text;
    if (co->ArgRes[ARG_FULL]) rc = MatchFull( mi ); else rc = MatchOne( mi );
    if (! rc) intAZ_SetCmdResult( co, TEXT_ERR_MatchNotFound );

    if (! co->ArgRes[ARG_NOCURS]) { func_CursorPos( mi->Line, mi->Col ); AGP.FObj->CEdCol = mi->Col; }

  END_ERROR:
    BufFree( ulp );
    BufFree( ubuf );
}

//»»»»»» Cmd_Indent BACK/S,SPACE/S

static LONG get_frontcol( LONG line, LONG eline )
{
  LONG frontcol, col;
  UBYTE *ptr;

    for (frontcol=0x7fffffff; line <= eline; line++)
        {
        col = skip_char( ptr = intAZ_LineBuf( line ), ' ', '\t' );
        if (*ptr)
            if (frontcol > col) frontcol = col;
        }
    if (frontcol == 0x7fffffff) frontcol = 0;
    return( frontcol );
}

void CmdFunc_Indent_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG line, col, prev, frontcol, diff;
  ULONG i, len, max, numblocks, errmsg = 0;
  struct AZBlock *azb;
  UBYTE *ptr, *mem=0;

    if (mem = PoolAlloc( AGP.Prefs->TabLen ))
        {
        for (i=0; i < AGP.Prefs->TabLen; i++) mem[i] = ' ';

        if (numblocks = func_BlockExists())
            {
            if (numblocks == 1)
                {
                azb = func_BlockPtr( 1 );
                if (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL)
                    {
                    frontcol = azb->ColStart;
                    if (co->ArgRes[0])  // BACK
                        {
                        prev = co->ArgRes[1] ? frontcol - 1 : prev_tab( frontcol );
                        for (line=azb->LineStart; line <= azb->LineEnd; line++)
                            {
                            len = intAZ_Buf_Len( line, &ptr );
                            if (len > prev)
                                {
                                max = MIN(len,frontcol);
                                for (i=max-1; i >= prev; i--)
                                    { if (ptr[i] != ' ' && ptr[i] != '\t') { prev = i + 1; break; } }
                                }
                            }
                        if ((diff = frontcol - prev) > 0)
                            {
                            func_TextDelRect( azb->LineStart, prev, azb->LineEnd, frontcol-1, 1, 0 );
                            azb->ColStart -= diff;
                            azb->ColEnd -= diff;
                            }
                        }
                    else{
                        len = co->ArgRes[1] ? 1 : next_tab( frontcol ) - frontcol;
                        for (line=azb->LineStart; line <= azb->LineEnd; line++)
                            {
                            func_TextPrep( line, frontcol, ARPF_NOCONVERT|ARPF_RECT|ARPF_NOOVERLAY, 0, 0 );
                            if (errmsg = func_TextPut( mem, len, 0, 0 )) break;
                            }
                        azb->ColStart += len;
                        azb->ColEnd += len;
                        }
                    }
                else{
                    frontcol = get_frontcol( azb->LineStart, azb->LineEnd );
                    if (co->ArgRes[0])  // BACK
                        {
                        prev = co->ArgRes[1] ? frontcol - 1 : prev_tab( frontcol );
                        if (frontcol - prev > 0)
                            func_TextDelRect( azb->LineStart, prev, azb->LineEnd, frontcol-1, 1, 0 );
                        }
                    else{
                        len = co->ArgRes[1] ? 1 : next_tab( frontcol ) - frontcol;
                        for (line=azb->LineStart; line <= azb->LineEnd; line++)
                            {
                            func_TextPrep( line, frontcol, ARPF_NOCONVERT|ARPF_RECT|ARPF_NOOVERLAY, 0, 0 );
                            if (errmsg = func_TextPut( mem, len, 0, 0 )) break;
                            }
                        }
                    diff = get_frontcol( azb->LineStart, azb->LineEnd ) - frontcol;
                    if (azb->ColStart >= frontcol) azb->ColStart += diff;
                    if (azb->ColEnd >= frontcol)   azb->ColEnd += diff;
                    }

                func_Display( azb->LineStart, azb->LineEnd-azb->LineStart+1, 0, -1 );
                }
            else errmsg = TEXT_ERR_TooManyBlocks;
            }
        else{
            if (co->ArgRes[0])  // BACK
                {
                if (fo->Col)
                    {
                    prev = co->ArgRes[1] ? fo->Col - 1 : prev_tab( fo->Col );
                    len = intAZ_Buf_Len( fo->Line, &ptr );
                    col = fo->Col;
                    if (col > len)
                        {
                        if (len < prev) col = prev;
                        else col = len;
                        }
                    for (; col > prev; )
                        { if (ptr[col-1] == ' ' || ptr[col-1] == '\t') col--; else break; }
                    if (fo->Col - col > 0)
                        {
                        func_TextDelRect( fo->Line, col, fo->Line, fo->Col-1, 1, 0 );
                        func_Display( fo->WorkLine, 1, 0, -1 );
                        func_CursorPos( fo->WorkLine, fo->WorkCol );
                        fo->CEdCol = fo->WorkCol;
                        if (fo->CEdCol > len) fo->CEdCol = len;
                        }
                    }
                }
            else{
                len = co->ArgRes[1] ? 1 : next_tab( fo->Col ) - fo->Col;
                errmsg = func_TextPut( mem, len, ARPF_NOCONVERT|ARPF_RECT|ARPF_NOOVERLAY, 1 );
                func_CursorPos( fo->WorkLine, fo->WorkCol );
                fo->CEdCol = fo->WorkCol;
                }
            }
        PoolFree( mem );
        }
    else errmsg = TEXT_NOMEMORY;

    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}
