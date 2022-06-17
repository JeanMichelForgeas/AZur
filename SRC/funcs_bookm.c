/****************************************************************
 *
 *      File:      funcs_bookm.c
 *      Project:   AZur
 *
 *      Created:   24-05-94     Jean-Michel Forgeas
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

struct AZBMInfo
    {
    struct AZBM *BMTable;
    ULONG       Count;
    LONG        FirstLine, LastLine;
    };


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;  // data relative base address pour le compilo


/****** Exported ***********************************************/

ULONG func_BMRemove( UBYTE *name, ULONG id );


/****** Statics ************************************************/


/****************************************************************
 *
 *      code
 *
 ****************************************************************/

static ULONG __asm get_exact_name_id( register __a0 UBYTE *name )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  struct AZBM *azb = bi->BMTable;
  ULONG id=0;

    for (id=0; id < bi->Count; id++, azb++)
        {
        if (! StrNCCmp( azb->Name, name ))
            return( id + 1 );
        }
    return(0);
}

static ULONG __asm get_name_id( register __a0 UBYTE *name )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  struct AZBM *azb = bi->BMTable;
  ULONG id=0;

    for (id=0; id < bi->Count; id++, azb++)
        {
        if (func_IsNCSubStr( azb->Name, name ))
            return( id + 1 );
        }
    return(0);
}

static ULONG __asm get_pos_id( register __d0 LONG line, register __d1 LONG col )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  struct AZBM *azb = bi->BMTable;
  ULONG id=0;

    for (id=0; id < bi->Count; id++, azb++)
        {
        if ((line == azb->Line) && (col == azb->Col))
            return( id + 1 );
        }
    return(0);
}

static ULONG __asm get_new_id( register __d0 LONG line, register __d1 LONG col )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  ULONG id=0;

    if (bi->Count)
        {
        struct AZBM *last, *azb = bi->BMTable;

        last = &azb[bi->Count-1];

        if ((line > last->Line) || (line == last->Line && col > last->Col))
            {
            id = bi->Count;
            }
        else{
            for (id=0; id < bi->Count; id++, azb++) //------ Si line en-dessous on insère avant
                {
                if ((line < azb->Line) || (line == azb->Line && col < azb->Col))
                    break;
                }
            }
        }
    return( id );
}

//----------------------------- Actualiseurs

void func_BMAddLines( LONG line, ULONG num )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;

    if (bi->Count)
        {
        struct AZBM *azb = bi->BMTable;
        ULONG id;

        for (id=0; azb->Line < line; azb++) if (++id >= bi->Count) return;
        for (; id < bi->Count; id++, azb++)
            { azb->Line += num; UPDATE_REQBM(AGP.FObj); }
        }
}

void func_BMRemLines( LONG line, ULONG num )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;

    if (bi->Count)
        {
        struct AZBM *azb = bi->BMTable;
        ULONG id, startid, remnum, i;

        for (id=0; azb->Line < line; azb++) if (++id >= bi->Count) return;
        startid = id;
        for (; id < bi->Count && azb->Line >= line && azb->Line < line + num; id++, azb++) ;

        if (remnum = id - startid)
            {
            LONG endline = line;
            if (endline + num - 1 >= AGP.FObj->Text.NodeNum - 1)
                { if (--endline < 0) endline = 0; }

            for (id=startid, i=0; i < remnum; i++)
                {
                azb = &bi->BMTable[id];
                if (! (azb->Flags & AZBMF_DELPROOF))
                    {
                    func_BMRemove( 0, id+1 );
                    if (id >= bi->Count) return;
                    }
                else{
                    azb->Col = 0;
                    azb->Line = endline;
                    id++;
                    UPDATE_REQBM(AGP.FObj);
                    }
                }
            azb = &bi->BMTable[id]; // recharger le nouveau uikbuffer

            /*
            BufCutS( (UBYTE**)&bi->BMTable, startid*sizeof(struct AZBM), remnum*sizeof(struct AZBM) );
            if ((bi->Count -= remnum) == 0)
                {
                bi->FirstLine = MAXSIGNED;
                bi->LastLine = 0xffffffff;
                }
            UPDATE_REQBM(AGP.FObj);
            id -= remnum;           // reprendre au nouveau id
            azb = &bi->BMTable[id]; // recharger le nouveau uikbuffer
            */
            }

        for (; id < bi->Count; id++, azb++)
            { azb->Line -= num; UPDATE_REQBM(AGP.FObj); }
        }
}

void func_BMAddChars( LONG line, LONG col, ULONG chars )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;

    if (bi->Count)
        {
        struct AZBM *azb = bi->BMTable;
        ULONG id=0;

        for (id=0; azb->Line < line; id++, azb++)
            if (id >= bi->Count) return;
        for (; id < bi->Count && azb->Line == line; id++, azb++)
            if (azb->Col >= col) { azb->Col += chars; UPDATE_REQBM(AGP.FObj); }
        }
}

void func_BMRemChars( LONG line, LONG col, ULONG chars )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;

    if (bi->Count)
        {
        struct AZBM *azb = bi->BMTable;
        ULONG id=0;

        for (id=0; azb->Line < line; id++, azb++) if (id >= bi->Count) return;

        for (; id < bi->Count && azb->Line == line; )
            {
            if (azb->Col >= col)
                {
                if ((chars != MAXSIGNED) && (azb->Col >= col + chars))  // attention car chars peut == MAXSIGNED
                    {
                    azb->Col -= chars;
                    id++; azb++;
                    UPDATE_REQBM(AGP.FObj);
                    }
                else{
                    if (! (azb->Flags & AZBMF_DELPROOF))
                        {
                        func_BMRemove( 0, id+1 );
                        if (id >= bi->Count) return;
                        azb = &bi->BMTable[id];
                        }
                    else{
                        azb->Col = col;
                        id++; azb++;
                        UPDATE_REQBM(AGP.FObj);
                        }
                    }
                }
            else { id++; azb++; }
            }
        }
}

void func_BMSwapLines( LONG line1, LONG line2 )
{
 /*
  struct AZBMInfo *bi = AGP.FObj->BMInfo;

    if (bi->Count)
        {
        struct AZBM *azb = bi->BMTable;
        ULONG id1, startid1, num1;
        ULONG id2, startid2, num2;

        for (id1=0; azb->Line < line1; azb++) if (++id1 >= bi->Count) return;
        startid1 = id1;
        for (; id1 < bi->Count && azb->Line == line1; id1++, azb++) ;
        num1 = id1 - startid1;

        for (id2=0; azb->Line < line2; azb++) if (++id2 >= bi->Count) return;
        startid2 = id2;
        for (; id2 < bi->Count && azb->Line == line2; id2++, azb++) ;
        num2 = id2 - startid2;
        }
 */
}

void func_BMJoinLines( LONG line, ULONG linelen )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;

    if (bi->Count)
        {
        struct AZBM *azb = bi->BMTable;
        ULONG id;

        for (id=0; azb->Line <= line; azb++) if (++id >= bi->Count) return;
        for (; azb->Line == line+1; azb++)
            {
            azb->Col += linelen;
            azb->Line--;
            UPDATE_REQBM(AGP.FObj);
            if (++id >= bi->Count) return;
            }
        for (; id < bi->Count; id++, azb++)
            {
            azb->Line--; UPDATE_REQBM(AGP.FObj);
            }
        }
}

void func_BMSplitLine( LONG line, ULONG col )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;

    if (bi->Count)
        {
        struct AZBM *azb = bi->BMTable;
        ULONG id;

        for (id=0; azb->Line < line; azb++) if (++id >= bi->Count) return;
        for (; azb->Line == line; azb++)
            {
            if (azb->Col >= col) { azb->Col -= col; azb->Line++; UPDATE_REQBM(AGP.FObj); }
            if (++id >= bi->Count) return;
            }
        for (; id < bi->Count; id++, azb++)
            { azb->Line++; UPDATE_REQBM(AGP.FObj); }
        }
}

//-----------------------------

struct AZBM *func_BMGetPrev( LONG line, LONG col )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  struct AZBM *azb, *found=0;

    if (bi->Count)
        {
        ULONG id;
        azb = bi->BMTable;
        for (id=0; (id < bi->Count) && (azb->Line < line) || (azb->Line == line && azb->Col < col); id++, azb++)
            found = azb;
        }
    return( found );
}

struct AZBM *func_BMGetNext( LONG line, LONG col )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  struct AZBM *azb, *found=0;

    if (bi->Count)
        {
        ULONG id;
        azb = &bi->BMTable[bi->Count-1];
        for (id=bi->Count; (id > 0) && (azb->Line > line) || (azb->Line == line && azb->Col > col); id--, azb--)
            found = azb;
        }
    return( found );
}

ULONG func_BMExists()
{
    return( AGP.FObj->BMInfo->Count );
}

struct AZBM *func_BMGetPtr( ULONG id )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;

    if (! bi->Count) return(0);
    if (id == -1) id = bi->Count;
    if (! id) return(0);
    return( &bi->BMTable[id-1] );
}

struct AZBM *func_BMAdd( UBYTE *name, LONG line, LONG col )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  struct AZBM bm;
  ULONG id;

    id = get_new_id( line, col );

    StcCpy( bm.Name, name, sizeof(bm.Name) );
    bm.Line = line;
    bm.Col  = col;
    if (! BufPasteS( (APTR)&bm, sizeof(struct AZBM), (UBYTE **)&bi->BMTable, id*sizeof(struct AZBM) )) return(0);

    if (bi->FirstLine > line) bi->FirstLine = line;
    if (bi->LastLine < line) bi->LastLine = line;
    bi->Count++;
    UPDATE_REQBM(AGP.FObj);
    return( &bi->BMTable[id] );
}

ULONG func_BMInside( ULONG fromid, LONG line, struct AZBM **pazb )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  struct AZBM *azb;

    if (fromid) fromid--;
    if (fromid >= bi->Count) return(0);

    for (azb = &bi->BMTable[fromid]; fromid < bi->Count; fromid++, azb++)
        {
        if (line == azb->Line)
            {
            if (pazb) *pazb = azb;
            return( fromid+1 );
            }
        }
    return(0);
}

ULONG func_BMApply( ULONG (*func)(), ULONG from, ULONG to, ULONG reverse, APTR parm )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  ULONG rc;

    if (! AGP.FObj->BMInfo->Count) return( TEXT_NoBMDefined );
    if (from == 0) from = 1;
    if (to > bi->Count) to = bi->Count;
    if (reverse)
        {
        for (; to >= from; to--)    // bi->BMTable peut changer si des blocs sont libérés
            if (rc = (*func)( &bi->BMTable[to-1], to, parm )) return( rc );
        }
    else{
        for (; from <= to; from++)
            if (rc = (*func)( &bi->BMTable[from-1], from, parm )) return( rc );
        }
    return(0);
}

ULONG func_BMGetId( UBYTE *name, LONG line, LONG col )
{
  ULONG id=0;

    if (name)
        {
        id = get_name_id( name );
        }
    else{
        id = get_pos_id( line, col );
        }
    return(id);
}

ULONG func_BMRemove( UBYTE *name, ULONG id )
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;

    if (name) if (! (id = get_exact_name_id( name ))) return(0);

    if (id) id--;
    if (id >= bi->Count) return(0);

    BufCutS( (UBYTE**)&bi->BMTable, id*sizeof(struct AZBM), sizeof(struct AZBM) );
    if (--bi->Count == 0)
        {
        bi->FirstLine = MAXSIGNED;
        bi->LastLine = 0xffffffff;
        }
    UPDATE_REQBM(AGP.FObj);
    return(1);
}

ULONG func_BMRemAll()
{
  struct AZBMInfo *bi = AGP.FObj->BMInfo;
  ULONG num;

    if (! (num = bi->Count)) return(0);

    BufResizeS( (UBYTE**)&bi->BMTable, 0 );
    bi->Count = 0;
    bi->FirstLine = MAXSIGNED;
    bi->LastLine = 0xffffffff;
    UPDATE_REQBM(AGP.FObj);
    return( num );
}

//----------------------------------------------------------------

ULONG func_BMInit()
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZBMInfo *bi;

    if (bi = PoolAlloc( sizeof(struct AZBMInfo) ))
        {
        if (bi->BMTable = (struct AZBM *) BufAllocP( AGP.Pool, 0, 10*sizeof(struct AZBM) ))
            {
            fo->BMInfo = bi;
            bi->Count = 0;
            bi->FirstLine = MAXSIGNED;
            bi->LastLine = 0xffffffff;
            return(1);
            }
        }
    return(0);
}

void func_BMCleanup()
{
    /* on ne fait rien car tout est pooled */
}
