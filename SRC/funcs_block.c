/****************************************************************
 *
 *      File:      funcs_block.c
 *      Project:   AZur
 *
 *      Created:   03-12-93     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:azur_protos.h"
#include "lci:objfile.h"

struct AZBlockInfo
    {
    struct AZBlock  *BlockTable;
    ULONG           Count;
    ULONG           NewId, Current;         // toujours à +1
    LONG            CurSLine, CurSCol;      // départ réel du bloc courant (sans redessement)
    LONG            CurELine, CurECol;      // fin réelle du bloc courant (sans redessement)
    LONG            FirstLine, LastLine;
    WORD            MouseX, MouseY;
    LONG            ClipStartLine, ClipStartCol;
    LONG            ClipEndLine, ClipEndCol;
    ULONG           OldCurrent;
    LONG            OldCurSLine, OldCurSCol;
    LONG            OldCurELine, OldCurECol;
    LONG            FuturSLine, FuturSCol;
    };


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;  // data relative base address pour le compilo


/****** Exported ***********************************************/


/****** Statics ************************************************/


/****************************************************************
 *
 *      code
 *
 ****************************************************************/

static LONG redresse( struct AZBlock *azb )
{
  LONG inv = 0;

    if (azb->LineStart == azb->LineEnd)
        {
        if (azb->ColStart > azb->ColEnd)
            { inv = azb->ColEnd; azb->ColEnd = azb->ColStart; azb->ColStart = inv; inv = 1; }
        }
    else{
        /*if (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL)
            {
            if (azb->ColStart > azb->ColEnd)
                { inv = azb->ColEnd; azb->ColEnd = azb->ColStart; azb->ColStart = inv; inv = 0; }
            if (azb->LineStart > azb->LineEnd)
                { inv = azb->LineEnd; azb->LineEnd = azb->LineStart; azb->LineStart = inv; inv = 1; }
            }
        else*/{
            if (azb->LineStart > azb->LineEnd)
                {
                inv = azb->LineEnd; azb->LineEnd = azb->LineStart; azb->LineStart = inv;
                inv = azb->ColEnd; azb->ColEnd = azb->ColStart; azb->ColStart = inv;
                inv = 1;
                }
            }
        }
    return( inv );
}

static ULONG MarkLines( struct AZBlockInfo *bi, ULONG id )
{
  struct AZBlock *azb = &bi->BlockTable[id];
  LONG last = AGP.FObj->Text.NodeNum - 1;

    if (azb->LineStart <= last)
        intAZ_ORLineFlags( azb->LineStart, UNF_BLKSTART );    // *((UWORD*)(((ULONG)(intAZ_LineBuf(azb->LineStart)))-6)) |= UNF_BLKSTART;
    if (azb->LineEnd <= last)
        intAZ_ORLineFlags( azb->LineEnd, UNF_BLKEND );        // *((UWORD*)(((ULONG)(intAZ_LineBuf(azb->LineEnd)))-6)) |= UNF_BLKEND;
    return(1);
}

static ULONG UnmarkLines( struct AZBlockInfo *bi, ULONG id )
{
  struct AZBlock *azb = &bi->BlockTable[id];
  struct AZBlock *prev = id > 0 ? azb-1 : 0;
  struct AZBlock *next = id+1 < bi->Count ? azb+1 : 0;

    if ((!prev || (prev->LineStart != azb->LineStart)) && (!next || (next->LineStart != azb->LineStart)))
        intAZ_ANDLineFlags( azb->LineStart, ~UNF_BLKSTART );  // *((UWORD*)(((ULONG)(intAZ_LineBuf(azb->LineStart)))-6)) &= ~UNF_BLKSTART;
    if ((!prev || (prev->LineEnd != azb->LineEnd)) && (!next || (next->LineEnd != azb->LineEnd)))
        intAZ_ANDLineFlags( azb->LineEnd, ~UNF_BLKEND );      // *((UWORD*)(((ULONG)(intAZ_LineBuf(azb->LineEnd)))-6)) &= ~UNF_BLKEND;
    return(1);
}

ULONG func_BlockContinue()
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;

    if (bi->Current == 0 && bi->OldCurrent <= bi->Count)
        {
        bi->Current  = bi->OldCurrent;
        bi->CurSLine = bi->OldCurSLine;
        bi->CurSCol  = bi->OldCurSCol;
        bi->CurELine = bi->OldCurELine;
        bi->CurECol  = bi->OldCurECol;
        }
    return( bi->Current );
}

ULONG func_BlockExists()
{
    return( AGP.FObj ? AGP.FObj->BlockInfo->Count : 0 );
}

ULONG func_BlockCurrent()
{
  struct AZBlockInfo *bi;

    if (!AGP.FObj) return(0);
    bi = AGP.FObj->BlockInfo;
    if (bi->CurSLine != bi->FuturSLine || bi->CurSCol != bi->FuturSCol) return(0);
    return( bi->Current );
}

ULONG func_BlockCurrent2()
{
  struct AZBlockInfo *bi;

    if (!AGP.FObj) return(0);
    bi = AGP.FObj->BlockInfo;
    if (bi->Current == 0 || bi->CurSLine != bi->FuturSLine || bi->CurSCol != bi->FuturSCol)
        return( bi->OldCurrent );
    return( bi->Current );
}

void func_BlockGetStart( LONG *pline, LONG *pcol )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;

    if (bi->Count)
        {
        *pline = bi->CurSLine;
        *pcol = bi->CurSCol;
        }
    else{
        *pline = bi->FuturSLine;
        *pcol = bi->FuturSCol;
        }
}

void func_BlockGetLimit( LONG *pline, LONG *pcol )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;

    *pline = bi->CurELine;
    *pcol = bi->CurECol;
}

struct AZBlock *func_BlockPtr( ULONG id )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;

    if (! bi->Count) return(0);
    if (id == -1) id = bi->Current;
    if (! id) return(0);
    return( &bi->BlockTable[id-1] );
}

struct AZBlock *func_BlockGetPrev( LONG line, LONG col )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct AZBlock *azb;
  ULONG id;

    for (id=bi->Count; id > 0; id--)
        {
        azb = &bi->BlockTable[id-1];
        if ((line > azb->LineEnd) || (line == azb->LineEnd && col > azb->ColEnd))
            return( azb );
        }
    return(0);
}

struct AZBlock *func_BlockGetNext( LONG line, LONG col )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct AZBlock *azb;
  ULONG id;

    for (id=1; id <= bi->Count; id++)
        {
        azb = &bi->BlockTable[id-1];
        if ((line < azb->LineStart) || (line == azb->LineStart && col < azb->ColStart))
            return( azb );
        }
    return(0);
}

static ULONG get_newid( LONG line, LONG col )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  ULONG id=0;

    if (bi->Count)
        {
        struct AZBlock *last, *azb = bi->BlockTable;

        last = &azb[bi->Count-1];

        if ((line > last->LineEnd) || (line == last->LineEnd && col > last->ColEnd))
            {
            id = bi->Count;
            }
        else{
            for (id=0; id < bi->Count; id++, azb++)
                {
                if (azb->Mode == BLKMODE_LINE)
                    {
                    //------ Si fin en-dessous du block on insère avant
                    if (line < azb->LineStart)
                        break;
                    //------ Si début avant la fin du block erreur
                    if (line <= azb->LineEnd)
                        return(0);
                    }
                else{
                    //------ Si fin en-dessous du block on insère avant
                    if ((line < azb->LineStart) || (line == azb->LineStart && col < azb->ColStart))
                        break;
                    //------ Si début avant la fin du block erreur
                    if ((line < azb->LineEnd) || (line == azb->LineEnd && (col <= azb->ColEnd || azb->ColEnd >= intAZ_LineLen( azb->LineEnd ))))
                        return(0);
                    }
                //------ On est après ce block, on continue
                }
            }
        }
    return( id + 1 );
}

void func_BlockSetFutur( LONG line, LONG col )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;

    bi->FuturSLine = line; bi->FuturSCol = col;
}

ULONG func_BlockStart( LONG line, LONG col, ULONG pointer )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZBlockInfo *bi = fo->BlockInfo;

    if (! (bi->NewId = get_newid( line, col ))) return(0);
    if (bi->Current)
        {
        bi->OldCurrent  = bi->Current;
        bi->OldCurSLine = bi->CurSLine;
        bi->OldCurSCol  = bi->CurSCol;
        bi->OldCurELine = bi->CurELine;
        bi->OldCurECol  = bi->CurECol;
        }
    bi->CurSLine = line; bi->CurSCol = col;     // position de départ des blocs
    bi->FuturSLine = line; bi->FuturSCol = col; // position de départ des blocs
    bi->Current = 0;                            // pas de bloc courant
    bi->CurELine = bi->CurECol = -1;            // marque la première fois
    if (pointer) { bi->MouseX = fo->MouseX; bi->MouseY = fo->MouseY; } else bi->MouseX = -1;
    return(1);
}

static void get_clip( ULONG id )  // id n'existe pas encore, on cherche son espace
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct AZBlock *prev, *next;

    if (id > 0)
        {
        prev = &bi->BlockTable[id-1];
        if (prev->Mode == BLKMODE_LINE || prev->ColEnd >= intAZ_LineLen( prev->LineEnd ))
            {
            bi->ClipStartLine = prev->LineEnd + 1;
            bi->ClipStartCol = 0;
            }
        else{
            bi->ClipStartLine = prev->LineEnd;
            bi->ClipStartCol = prev->ColEnd + 1;
            }
        }
    else{
        bi->ClipStartLine = 0;
        bi->ClipStartCol = 0;
        }

    if (id < bi->Count)
        {
        next = &bi->BlockTable[id];
        if (next->Mode == BLKMODE_LINE)
            {
            bi->ClipEndLine = next->LineStart - 1;
            bi->ClipEndCol = MAXSIGNED;
            }
        else{
            bi->ClipEndLine = next->LineStart;
            bi->ClipEndCol = next->ColStart - 1;
            }
        }
    else{
        bi->ClipEndLine = AGP.FObj->Text.NodeNum - 1;
        bi->ClipEndCol = MAXSIGNED;
        }
}

static ULONG arrange_limits( struct AZBlock *bl )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  ULONG lenstart, lenend, status_word;

    switch (bl->Mode)
        {
        case BLKMODE_CHAR:
            if (bl->LineStart < bi->ClipStartLine) bl->LineStart = bi->ClipStartLine;
            if (bl->LineStart == bi->ClipStartLine && bl->ColStart < bi->ClipStartCol) bl->LineStart = bi->ClipStartLine + 1;
            lenstart = intAZ_LineLen( bl->LineStart );
            if (bl->ColStart > lenstart) bl->ColStart = lenstart;

            if (bl->LineEnd > bi->ClipEndLine) bl->LineEnd = bi->ClipEndLine;
            if (bl->LineEnd == bi->ClipEndLine && bl->ColEnd > bi->ClipEndCol) bl->LineEnd = bi->ClipEndLine - 1;
            lenend = intAZ_LineLen( bl->LineEnd );
            if (bl->ColEnd > lenend) bl->ColEnd = lenend;
            break;

        case BLKMODE_WORD:
            {
            UBYTE ch, *ptr;
            LONG line, col;

            //---------------------------------
            if (bl->LineStart < bi->ClipStartLine) bl->LineStart = bi->ClipStartLine;
            if (bl->LineStart == bi->ClipStartLine && bl->ColStart < bi->ClipStartCol) bl->LineStart = bi->ClipStartLine + 1;
            lenstart = intAZ_LineLen( bl->LineStart );
            if (bl->ColStart > lenstart) bl->ColStart = lenstart;

            ptr = func_LineBuf( bl->LineStart );
            line = bl->LineStart; col = bl->ColStart;
            ch = intAZ_CharCurrent( bl->ColStart, lenstart, ptr );
            if (status_word = func_CharIsWord( ch ) ? 1 : 0)
                {
                while ((ptr = func_CharPrev( ptr, &lenstart, &line, &col, &ch )) != (APTR)-1)
                    {
                    if (func_CharIsWord( ch )) { if (status_word == 0) break; }
                    else { if (status_word == 1) break; }

                    if (line > bi->ClipStartLine || (line == bi->ClipStartLine && col >= bi->ClipStartCol))
                        { bl->LineStart = line; bl->ColStart = col; }
                    else break;
                    }
                }

            //---------------------------------
            if (bl->LineEnd > bi->ClipEndLine) bl->LineEnd = bi->ClipEndLine;
            if (bl->LineEnd == bi->ClipEndLine && bl->ColEnd > bi->ClipEndCol) bl->LineEnd = bi->ClipEndLine - 1;
            lenend = intAZ_LineLen( bl->LineEnd );
            if (bl->ColEnd > lenend) bl->ColEnd = lenend;

            ptr = func_LineBuf( bl->LineEnd );
            line = bl->LineEnd; col = bl->ColEnd;
            ch = intAZ_CharCurrent( bl->ColEnd, lenend, ptr );
            if (status_word = func_CharIsWord( ch ) ? 1 : 0)
                {
                while ((ptr = func_CharNext( ptr, &lenend, &line, &col, &ch )) != (APTR)-1)
                    {
                    if (func_CharIsWord( ch )) { if (status_word == 0) break; }
                    else { if (status_word == 1) break; }

                    if (line < bi->ClipEndLine || (line == bi->ClipEndLine && col <= bi->ClipEndCol))
                        { bl->LineEnd = line; bl->ColEnd = col; }
                    else break;
                    }
                }
            }
            break;

        case BLKMODE_LINE:
            if (bl->LineStart < bi->ClipStartLine) bl->LineStart = bi->ClipStartLine;
            bl->ColStart = 0;
            if (bl->LineStart == bi->ClipStartLine && bl->ColStart < bi->ClipStartCol) bl->LineStart = bi->ClipStartLine + 1;

            if (bl->LineEnd > bi->ClipEndLine) bl->LineEnd = bi->ClipEndLine;
            bl->ColEnd = intAZ_LineLen( bl->LineEnd );
            if (bl->LineEnd == bi->ClipEndLine && bl->ColEnd > bi->ClipEndCol)
                {
                bl->LineEnd = bi->ClipEndLine - 1;
                bl->ColEnd = intAZ_LineLen( bl->LineEnd );
                }
            break;

        case BLKMODE_RECT:
        case BLKMODE_FULL:
            if (bl->LineStart < bi->ClipStartLine) bl->LineStart = bi->ClipStartLine;
            if (bl->LineStart == bi->ClipStartLine && (bl->ColStart < bi->ClipStartCol || bl->ColEnd < bi->ClipStartCol)) bl->LineStart = bi->ClipStartLine + 1;

            if (bl->LineEnd > bi->ClipEndLine) bl->LineEnd = bi->ClipEndLine;
            if (bl->LineEnd == bi->ClipEndLine && (bl->ColStart > bi->ClipEndCol || bl->ColEnd > bi->ClipEndCol)) bl->LineEnd = bi->ClipEndLine - 1;
            break;
        }

    if (bl->LineStart < bl->LineEnd || (bl->LineStart == bl->LineEnd && bl->ColStart <= bl->ColEnd))
        return(1);
    return(0);
}

struct AZBlock *func_BlockModify( LONG lineend, LONG colend, LONG *pstartline, LONG *pendline )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct AZBlock bl, *azb;
  ULONG id, inv;
  LONG num;

    if (! (id = bi->Current)) return(0);
    id--;
    azb = &bi->BlockTable[id];

    bl.LineStart = bi->CurSLine;
    bl.ColStart  = bi->CurSCol;
    bl.LineEnd   = lineend;
    bl.ColEnd    = colend;
    bl.Mode      = azb->Mode;
    inv = redresse( &bl );

    if (! arrange_limits( &bl )) return(0);

    if (inv) { lineend = bl.LineStart; colend = bl.ColStart; }
    else { lineend = bl.LineEnd; colend = bl.ColEnd; }

    /* CurELine et CurECol doivent contenir les valeurs après traitement car
     * les lignes et colonnes peuvent être différentes et donc nécessiter de
     * dessiner au-delà des positions entrées.
     */
    if (lineend == bi->CurELine && colend == bi->CurECol) // même place ?
        {
        *pstartline = lineend;
        *pendline = lineend - 1; /* pour ne dessiner aucune ligne... */
        }
    else{
        UnmarkLines( bi, id );

        //------ Nombre de lignes à redessiner
        if ((num = lineend - bi->CurELine) < 0)
            {
            *pstartline = lineend;
            *pendline = bi->CurELine;
            }
        else{
            *pstartline = bi->CurELine;
            *pendline = lineend;
            }
        if (bl.Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL)
            {
            if (*pstartline > bl.LineStart) *pstartline = bl.LineStart;
            if (*pendline < bl.LineEnd) *pendline = bl.LineEnd;
            }
        bi->CurELine = lineend;
        bi->CurECol  = colend;

        //------ modification du bloc
        if ((bl.Mode == BLKMODE_RECT || bl.Mode == BLKMODE_FULL) && bl.ColStart > bl.ColEnd)
            { LONG tmp; tmp = bl.ColStart; bl.ColStart = bl.ColEnd; bl.ColEnd = tmp; }
        azb->LineStart = bl.LineStart;
        azb->ColStart  = bl.ColStart;
        azb->LineEnd   = bl.LineEnd;
        azb->ColEnd    = bl.ColEnd;

        if (bi->FirstLine > azb->LineStart) bi->FirstLine = azb->LineStart;
        if (bi->LastLine < azb->LineEnd) bi->LastLine = azb->LineEnd;

        MarkLines( bi, id );
        }
    return( azb );
}

struct AZBlock *func_BlockDefine( LONG lineend, LONG colend, ULONG mode, ULONG pointer )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct AZBlock bl; //, *azb = bi->BlockTable;
  ULONG inv, id;

    if (bi->CurSLine != bi->FuturSLine || bi->CurSCol != bi->FuturSCol)
        {
        if (! func_BlockStart( bi->FuturSLine, bi->FuturSCol, pointer )) return(0);
        }
    id = bi->NewId - 1;

    if (pointer && bi->MouseX >= 0) // éloigné du point de clic ?
        {
        if (ABS(AGP.FObj->MouseX - bi->MouseX) <= AGP.Prefs->BlockXLim && ABS(AGP.FObj->MouseY - bi->MouseY) <= AGP.Prefs->BlockYLim)
            return((APTR)-1);
        }

    get_clip( id );

    if (! mode) mode = BLKMODE_CHAR;

    bl.LineStart = bi->CurSLine = bi->FuturSLine;
    bl.ColStart  = bi->CurSCol  = bi->FuturSCol ;
    bl.LineEnd   = lineend;
    bl.ColEnd    = colend;
    bl.Mode      = mode;
    inv = redresse( &bl );

    if (! arrange_limits( &bl )) return(0);

    if (inv) { lineend = bl.LineStart; colend = bl.ColStart; }
    else { lineend = bl.LineEnd; colend = bl.ColEnd; }
//kprintf( "4: bl.LineStart=%ld, bl.ColStart=%ld, bl.LineEnd=%ld, bl.ColEnd=%ld, bl.Mode=%ld\n", bl.LineStart, bl.ColStart, bl.LineEnd, bl.ColEnd, bl.Mode );

    if ((bl.Mode == BLKMODE_RECT || bl.Mode == BLKMODE_FULL) && bl.ColStart > bl.ColEnd)
        { LONG tmp; tmp = bl.ColStart; bl.ColStart = bl.ColEnd; bl.ColEnd = tmp; }
//kprintf( "5: bl.LineStart=%ld, bl.ColStart=%ld, bl.LineEnd=%ld, bl.ColEnd=%ld, bl.Mode=%ld\n", bl.LineStart, bl.ColStart, bl.LineEnd, bl.ColEnd, bl.Mode );

    if (! BufPasteS( (APTR)&bl, sizeof(struct AZBlock), (UBYTE **)&bi->BlockTable, id*sizeof(struct AZBlock) )) return(0);
    MarkLines( bi, id );

    if (bi->FirstLine > bl.LineStart) bi->FirstLine = bl.LineStart;
    if (bi->LastLine < bl.LineEnd) bi->LastLine = bl.LineEnd;
    bi->Count++;
    bi->Current = id+1;
    bi->CurELine = lineend;
    bi->CurECol  = colend;

    return( &bi->BlockTable[id] );
}

ULONG func_BlockLineInside( ULONG fromid, LONG line, struct AZBlock **pazb )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct AZBlock *azb;

    if (fromid) fromid--;
    if (fromid >= bi->Count) return(0);

    for (azb = &bi->BlockTable[fromid]; fromid < bi->Count; fromid++, azb++)
        {
        if (line < azb->LineStart) return(0);

        if ((line >= azb->LineStart) && (line <= azb->LineEnd))
            {
            if (pazb) *pazb = azb;
            return( fromid+1 );
            }
        }
    return(0);
}

ULONG func_BlockInside( ULONG fromid, LONG line, LONG col, struct AZBlock **pazb, ULONG rect )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct AZBlock *azb;
  LONG colend;

    if (fromid) fromid--;
    if (fromid >= bi->Count) return(0);

    for (azb = &bi->BlockTable[fromid]; fromid < bi->Count; fromid++, azb++)
        {
        if (line < azb->LineStart) return(0);

        if (azb->Mode == BLKMODE_LINE)
            {
            if ((line >= azb->LineStart) && (line <= azb->LineEnd))
                {
                if (pazb) *pazb = azb;
                return( fromid+1 );
                }
            }
        else if (rect && (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL))
            {
            if ((line >= azb->LineStart) && (line <= azb->LineEnd) && (col >= azb->ColStart) && (col <= azb->ColEnd))
                {
                if (pazb) *pazb = azb;
                return( fromid+1 );
                }
            }
        else{
            colend = (azb->ColEnd >= intAZ_LineLen( line )) ? MAXSIGNED : azb->ColEnd;
            if (((line > azb->LineStart) || (line == azb->LineStart && col >= azb->ColStart)) &&
                ((line < azb->LineEnd) || (line == azb->LineEnd && col <= colend)))
                {
                if (pazb) *pazb = azb;
                return( fromid+1 );
                }
            }
        }
    return(0);
}

ULONG func_BlockApply( ULONG (*func)(), ULONG from, ULONG to, ULONG reverse, APTR parm )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  ULONG rc;

    if (! AGP.FObj->BlockInfo->Count) return( TEXT_NoBlockDefined );
    if (from == 0) from = 1;
    if (to > bi->Count) to = bi->Count;
    if (reverse)
        {
        for (; to >= from; to--)    // bi->BlockTable peut changer si des blocs sont libérés
            if (rc = (*func)( &bi->BlockTable[to-1], to, parm )) return( rc );
        }
    else{
        for (; from <= to; from++)
            if (rc = (*func)( &bi->BlockTable[from-1], from, parm )) return( rc );
        }
    return(0);
}

ULONG func_BlockRemove( ULONG id, LONG *pfirstline, LONG *plastline )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct AZBlock *azb;

    if (id) id--;
    if (id >= bi->Count) return(0);

    UnmarkLines( bi, id );
    azb = &bi->BlockTable[id];
    if (pfirstline) *pfirstline = azb->LineStart;
    if (plastline) *plastline = azb->LineEnd;

    BufCutS( (UBYTE**)&bi->BlockTable, id*sizeof(struct AZBlock), sizeof(struct AZBlock) );
    if (--bi->Count == 0)
        {
        bi->FirstLine = MAXSIGNED;
        bi->LastLine = 0xffffffff;
        //bi->FirstLine = -1;
        }
    if (bi->Current-1 == id)
        {
        bi->OldCurrent = 0;
        bi->Current = 0;
        bi->CurELine = bi->CurECol = -1;
        }
    return(1);
}

ULONG func_BlockRemAll( LONG *pfirstline, LONG *plastline )
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct UIKNode *unode = AGP.FObj->Text.UNode;
  ULONG i, num;
  LONG last = AGP.FObj->Text.NodeNum - 1;

    if (! (num = bi->Count)) return(0);

    for (i=0; i<bi->Count; i++)
        {
        struct AZBlock *azb = &bi->BlockTable[i];
        if (azb->LineStart <= last) *((UWORD*)(((ULONG)(unode[azb->LineStart].Buf))-6)) &= ~UNF_BLKSTART;
        if (azb->LineEnd <= last) *((UWORD*)(((ULONG)(unode[azb->LineEnd].Buf))-6)) &= ~UNF_BLKEND;
        }
    if (pfirstline) *pfirstline = bi->FirstLine;
    if (plastline) *plastline = bi->LastLine;

    BufResizeS( (UBYTE**)&bi->BlockTable, 0 );
    bi->Count = 0;
    bi->OldCurrent = bi->Current = 0;
    bi->FirstLine = MAXSIGNED;
    bi->LastLine = 0xffffffff;
    bi->CurELine = bi->CurECol = -1;
    return( num );
}

//----------------------------------------------------------------

ULONG func_BlockInit()
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZBlockInfo *bi;

    if (bi = PoolAlloc( sizeof(struct AZBlockInfo) ))
        {
        if (bi->BlockTable = (struct AZBlock *) BufAllocP( AGP.Pool, 0, 10*sizeof(struct AZBlock) ))
            {
            fo->BlockInfo = bi;
            bi->FirstLine = MAXSIGNED;
            bi->LastLine = 0xffffffff;
            func_BlockStart( 0, 0, 0 );
            return(1);
            }
        }
    return(0);
}

void func_BlockCleanup()
{
    /* on ne fait rien car tout est pooled */
}

//================================================================

static ULONG bloc_del( struct AZBlock *azb, ULONG id, struct AZDisplayCmd *dinfo )
{
  LONG ls, cs, le, ce, rect;

    ls = azb->LineStart;
    cs = azb->ColStart;
    le = azb->LineEnd;
    ce = azb->ColEnd;
    rect = (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) ? azb->Mode : 0;

    func_BlockRemove( id, 0, 0 );
    func_TextDelRect( ls, cs, le, ce, rect, dinfo );
    func_DisplayCmd( dinfo );
    return(0);
}

ULONG func_BlockEraseAll()
{
  struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  struct AZDisplayCmd DInfo;
  ULONG num;

    if (! (num = bi->Count)) return(0);
    func_CursorErase();
    func_BlockApply( bloc_del, 1, -1, 1, &DInfo );
    func_CursorPos( DInfo.Line, DInfo.Col );
    func_AutoFormat();
    return( num );
}

ULONG func_BlockClearAll()
{
  //struct AZBlockInfo *bi = AGP.FObj->BlockInfo;
  LONG startline, endline;
  ULONG num;

    /* il faudrait pousser jusqu'au bout à droite si endcol est égal à sa longueur
       mais on ne gagne pas beaucoup de temps...
    if ((num = bi->Count) == 1)  // optimisation pour Search
        {
        struct AZBlock *azb = bi->BlockTable;
        LONG startcol, endcol;
        startline = azb->LineStart;
        endline = azb->LineEnd;
        if (startline == endline)
            {
            startcol = azb->ColStart;
            endcol = azb->ColEnd;
            if (num = func_BlockRemAll( 0, 0 ))
                func_Display( startline, endline - startline + 1, startcol, endcol - startcol + 1 );
            return(1);
            }
        }
    */
    if (num = func_BlockRemAll( &startline, &endline ))
        {
        SETCURSOR_VISIBLE(AGP.FObj);
        func_Display( startline, endline - startline + 1, 0, -1 );
        }
    return( num );
}

//--------------------------------------------------------------------------

static ULONG write_buf( UIKBUF **pub, UBYTE *buf, ULONG len )
{
    if (len)
        if (! BufPasteS( buf, len, pub, intAZ_LineBufLen(*pub) )) return( TEXT_NOMEMORY );
    return(0);
}

ULONG bloc_2_buf( struct AZBlock *azb, ULONG id, UIKBUF **pub )
{
    return( func_TextApply( azb->LineStart, azb->ColStart, azb->LineEnd, azb->ColEnd,
                            (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) ? azb->Mode : 0,
                            0, write_buf, pub ) );
}

ULONG func_Block2Buf( UBYTE **pub, ULONG clear )
{
  ULONG errmsg=0, alloc=0;

    if (func_BlockExists())
        {
        if (! *pub) // buffer peut être pré-alloué
            {
            if (! (*pub = BufAlloc( 16, 0, MEMF_ANY ))) errmsg = TEXT_NOMEMORY;
            alloc = 1;
            }
        if (*pub)
            {
            if ((errmsg = func_BlockApply( bloc_2_buf, 1, -1, 0, pub )) == 0)
                {
                if (clear) func_BlockClearAll();
                }
            else{ // si erreur on libère si on a alloué
                if (alloc) BufFreeS( pub );
                }
            }
        }
    else errmsg = TEXT_NoBlockDefined;

    return( errmsg );
}
