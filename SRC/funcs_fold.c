/****************************************************************
 *
 *      File:      funcs_fold.c
 *      Project:   AZur
 *
 *      Created:   08/09/94     Jean-Michel Forgeas
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
#include "lci:_pool_protos.h"

#define UTABLESIZE(n)   (sizeof(struct UIKNode) * (n))

#define UPBUFPTR(b) ((struct UIKPBuffer *) ((ULONG)b - sizeof(struct UIKPBuffer)))


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;  // data relative base address pour le compilo


/****** Exported ***********************************************/

ULONG func_FoldApplyLines( ULONG (*func)(), APTR parm, UIKBUF **pub );


/****** Statics ************************************************/


/****************************************************************
 *
 *      code
 *
 ****************************************************************/

static LONG nl_paste_list( struct UIKPList *L, LONG afterind, struct UIKPList *il, UWORD delta_id )
{
  LONG nnum, subnum, i, ind = afterind + 1;
  UWORD base_id;

    base_id = L->UNode[afterind].Reserved + 1;
    nnum = L->NodeNum;

    if (il == 0) // une seule ligne foldée
        {
        L->UNode[afterind].Reserved = base_id + delta_id;
        L->UNode[afterind].Flags &= ~UNF_FOLDED;
        return(0);
        }
    if (subnum = il->NodeNum)
        {
        if (nnum + subnum >= L->MaxNodes) // if (! IntUIK_ExtendList( L )) goto END_ERROR;
            {
            struct UIKNode *nodes;
            LONG missing, size;

            missing = nnum + subnum - L->MaxNodes + 1;
            if (missing <= L->BlockSize) size = L->BlockSize;
            else size = ((missing / L->BlockSize) + 1) * L->BlockSize;
            if (! (nodes = (struct UIKNode *) BufResize( (UBYTE *) L->UNode, UTABLESIZE(L->MaxNodes+size) ))) goto END_ERROR;
            L->UNode = nodes;
            L->MaxNodes += size;
            }
        UIK_MemCopy( (char*)&L->UNode[ind], (char*)&L->UNode[ind+subnum], UTABLESIZE(nnum - ind) );
        L->NodeNum += subnum;
        L->UNode[afterind].Reserved = base_id + delta_id;
        L->UNode[afterind].Flags &= ~UNF_FOLDED;

        for (i=0; i < subnum; i++) { il->UNode[i].Reserved += base_id; }
        UIK_MemCopy( (char*)il->UNode, (char*)&L->UNode[ind], UTABLESIZE(subnum) );
        func_BMAddLines( afterind+1, subnum );

        for (ind=0; ind < subnum; ind++) il->UNode[ind].Buf = 0; // pas de Free
        }
    UIK_nl_FreeList( il );
    AsmFreePooled( UPBUFPTR(AGP.FObj->Text.UNode)->Pool, il, sizeof(struct UIKPList), *(APTR*)4 );
    return( subnum );

  END_ERROR:
    return(0);
}

static ULONG unfold_nodes( struct UIKPList *list, LONG line, ULONG subparse )
{
  ULONG numlines=0, add, retlines=0;
  struct UIKPList *sublist;
  LONG ind, examine;
  UWORD delta_id;
  UBYTE *ub;

    if (line < list->NodeNum)
        {
        ub = list->UNode[line].Buf; // si sublist==0 exécuter quand même nl_paste_list() pour les folds de 1 ligne
        sublist = (struct UIKPList *) BufGetUDatas( ub, &delta_id );
        BufSetUDatas( ub, 0, 0 );
        retlines = nl_paste_list( list, line, sublist, delta_id );
        if (retlines && subparse)
            {
            for (numlines=retlines, examine=line+1, ind=0; ind < numlines; ind++, examine++)
                {
                if (list->UNode[examine].Flags & UNF_FOLDED)
                    {
                    if ((add = unfold_nodes( list, examine, subparse )) == -1) goto END_ERROR;
                    examine += add;
                    retlines += add;
                    }
                }
            }
        }
  END_ERROR:
    return( retlines );
}

//-----------------

static LONG nl_cut_list( struct UIKPList *L, LONG afterind, LONG eind, struct UIKPList **pil, ULONG base_id )
{
  LONG i, numind, sind;
  struct UIKPList *il;

    sind = afterind + 1;
    numind = eind - sind + 1;
    if (numind == 0)
        {
        L->UNode[afterind].Flags |= UNF_FOLDED;
        L->UNode[afterind].Reserved = base_id - 1;
        *pil = 0;
        return(0);
        }
    if (il = (struct UIKPList *) AsmAllocPooled( UPBUFPTR(AGP.FObj->Text.UNode)->Pool, sizeof(struct UIKPList), *(APTR*)4 ))
        {
        __builtin_memset( il, 0, sizeof(struct UIKPList) );
        if (UIK_nl_AllocListP( UPBUFPTR(AGP.FObj->Text.UNode)->Pool, il, numind, 0, 0, 0 ))
            {
            UIK_MemCopy( (char*)&L->UNode[sind], (char*)il->UNode, UTABLESIZE(numind) );
            il->NodeNum = numind;
            for (i=0; i < numind; i++) { il->UNode[i].Reserved -= base_id; } // pas la première

            UIK_MemCopy( (char*)&L->UNode[sind+numind], (char*)&L->UNode[sind], UTABLESIZE(L->NodeNum - sind) );
            L->NodeNum -= numind;
            L->UNode[afterind].Flags |= UNF_FOLDED;
            L->UNode[afterind].Reserved = base_id - 1;
            *pil = il;
            func_BMRemLines( sind, numind );
            return(numind);
            }
        AsmFreePooled( UPBUFPTR(AGP.FObj->Text.UNode)->Pool, il, sizeof(struct UIKPList), *(APTR*)4 );
        }
    return(0);
}

static LONG fold_nodes( UWORD base_id, LONG sline, LONG eline )
{
  struct UIKPList *list;
  struct UIKPList *sublist;
  LONG sub=0;
  UWORD delta_id;

    // Line de début du fold : différence des ids si différente de la ligne de prise du fold
    // On range le delta de la première ligne dans le UData, quand aux autres lignes on
    // soustraira le base_id de leur id à chacune.
    list = &AGP.FObj->Text;

    // delta = nombre de ids pour partir du id foldé jusqu'au id de la tête de liste
    delta_id = list->UNode[sline].Reserved - base_id;
    if (sub = nl_cut_list( list, sline, eline, &sublist, base_id ))
        {
        BufSetUDatas( list->UNode[sline].Buf, delta_id, (ULONG)sublist );
        }
    return( sub );
}

/****************************************************************
 *
 *      Simple Utilities
 *
 ****************************************************************/

void fo_SetLineId( LONG line, UWORD id )
{
    AGP.FObj->Text.UNode[line].Reserved = id;
}

ULONG fo_GetLineId( LONG line )
{
    return( (ULONG)AGP.FObj->Text.UNode[line].Reserved );
}

ULONG fo_IsLineFolded( LONG line )
{
    return( (ULONG)((AGP.FObj->Text.UNode[line].Flags & UNF_FOLDED) ? 1 : 0) );
}

ULONG fo_GetIndentLimit( LONG line, LONG *peline, ULONG addsame )
{
  ULONG found = 0;
  LONG startcol, col, last = AGP.FObj->Text.NodeNum - 1;
  UBYTE *ptr;

    startcol = skip_char( intAZ_LineBuf( line ), ' ', '\t' );
    *peline = line;

    //------ Recherche début indentation
    for (; line <= last; line++)
        {
        col = skip_char( ptr = intAZ_LineBuf( line ), ' ', '\t' );
        if (*ptr == 0 || col < startcol) break;  // Si ligne vide ou à gauche : terminé
        if (col > startcol)
            {
            //------ Saute l'indentation
            for (; line <= last; line++)
                {
                col = skip_char( ptr = intAZ_LineBuf( line ), ' ', '\t' );
                if (*ptr && col <= startcol) break; // ici on accepte les lignes vides
                if (*ptr) *peline = line;
                }

            //------ Attend la fin (on n'incrémente pas line car peut-être < startcol)
            for (; line <= last; line++)
                {
                col = skip_char( ptr = intAZ_LineBuf( line ), ' ', '\t' );
                if (*ptr == 0 || col < startcol) break; // Si ligne vide ou inférieure : terminé
                if (addsame) *peline = line; // Laisser la ligne de dernière indentation
                if (col == startcol) break;  // Si égale : terminé dans les deux cas
                *peline = line;
                }
            break;
            }
        if (line > last) line = last;
        *peline = line;     // Au moins la première ligne est bonne
        found++;
        }
    return( found );
}

/****************************************************************
 *
 *      Public Tools
 *
 ****************************************************************/

ULONG func_FoldGetPrev( LONG *pline, LONG *pcol )
{
  LONG foundline, line = *pline;
  struct UIKNode *node = &AGP.FObj->Text.UNode[line];
  UWORD current;

    if (line == 0) { current = node->Reserved; goto END_ERROR; }

    current = (node-1)->Reserved;
    foundline = line - 1;
    for (--line, --node; line >= 0; line--, node--)
        {
        if (node->Reserved < current) goto FOUND_NORMAL; // un fold collé fait aussi office de limite
        if (node->Flags & UNF_FOLDED) { foundline = line; goto FOUND_FOLDED; }
        /*if (node->Reserved == 0 || node->Reserved > current)*/ current = node->Reserved;
        foundline = line;
        if (line == 0 && current) goto FOUND_NORMAL;
        }

  END_ERROR:
    return(0);

  FOUND_FOLDED:
    current = node->Reserved + 1;
  FOUND_NORMAL:
    *pline = foundline; *pcol = current - 1;
    return( (ULONG)current );
}

ULONG func_FoldGetNext( LONG *pline, LONG *pcol )
{
  LONG line = *pline, last = AGP.FObj->Text.NodeNum - 1;
  struct UIKNode *node = &AGP.FObj->Text.UNode[line];
  UWORD current;

    if (line == last) goto END_ERROR;

    current = node->Reserved; // si folded pas de différence : il faut le début du premier qui remonte

    //------ Chercher le premier qui remonte ou le premier folded
    for (++line, ++node; line <= last; line++, node++)
        {
        if (node->Flags & UNF_FOLDED) goto FOUND_FOLDED;
        if ((LONG)node->Reserved > (LONG)current) goto FOUND_NORMAL;
        current = node->Reserved;
        }

  END_ERROR:
    return(0);

  FOUND_FOLDED:
    current = node->Reserved + 1;
  FOUND_NORMAL:
    *pline = line; *pcol = current;
    return( (ULONG)current+1 );
}

/* Tient compte si besoin si la ligne est FOLDED
 */
UWORD fo_GetLimits( LONG line, UWORD id, LONG *psline, LONG *peline, ULONG takefolded )
{
  LONG i, found=0;
  LONG last = AGP.FObj->Text.NodeNum - 1;
  struct UIKNode *node = &AGP.FObj->Text.UNode[line];

    *psline = *peline = line; // toujours qlq chose de valide

    if (id == 0xffff) id = fo_GetLineId( line );
    if (takefolded && (node->Flags & UNF_FOLDED)) return( (ULONG)id+1 );
    if (id == 0) return(0);

    for (i=line; i >= 0; i--, node--)
        {
        if (node->Reserved < id) break;
        found = 1;
        *psline = i;
        }
    if (found)
        {
        node = &AGP.FObj->Text.UNode[line+1];
        for (i=line+1; i <= last; i++, node++)
            {
            if (node->Reserved < id) break;
            *peline = i;
            }
        }
    return( (ULONG)(found ? id : 0) );
}

//------------------------------------------------------------------------------------

static LONG do_fold_off( LONG line, struct AZDisplayCmd *dinfo, ULONG subparse )
{
  LONG add=0;

    if (dinfo)
        {
        dinfo->DisplayNum  = 1;
        dinfo->DisplayFrom = line;
        dinfo->ScrollFrom  = line + 1;
        dinfo->ScrollNum   = 0;
        }

    if (fo_IsLineFolded( line ))
        {
        if (add = unfold_nodes( &AGP.FObj->Text, line, subparse ))
            {
            if (dinfo)
                {
                dinfo->DisplayNum += add;
                dinfo->ScrollNum = -add;
                }
            }
        }

    return( add );
}

LONG func_FoldOffSub( LONG line, struct AZDisplayCmd *dinfo )
{
    return( do_fold_off( line, dinfo, 1 ) );
}

LONG func_FoldOff( LONG line, struct AZDisplayCmd *dinfo )
{
    return( do_fold_off( line, dinfo, 0 ) );
}

LONG func_FoldOn( LONG line, struct AZDisplayCmd *dinfo )
{
  LONG sline, eline, sub=0, loadmode=0;
  UWORD id;

    if (dinfo == (APTR)-1)
        {
        loadmode = 1;
        dinfo = 0;
        }

    if (dinfo)
        {
        dinfo->DisplayNum  = 1;
        dinfo->DisplayFrom = line;
        dinfo->ScrollNum   = 0;
        }

    if (line >= AGP.FObj->Text.NodeNum) return(0); // si fold collés, il y a danger

    if (! fo_IsLineFolded( line ))
        {
        if (id = fo_GetLimits( line, (UWORD)-1, &sline, &eline, 0 ))
            {
            if (loadmode) sline = line;

            if (sub = fold_nodes( id, sline, eline ))
                {
                if (dinfo)
                    {
                    dinfo->DisplayFrom = sline;
                    dinfo->ScrollNum = sub;
                    dinfo->ScrollFrom  = sline + 1;
                    }
                }
            }
        }

    return( -sub );
}

LONG func_FoldSet( LONG sline, LONG eline, struct AZDisplayCmd *dinfo )
{
  LONG i, l1, l2, last = AGP.FObj->Text.NodeNum - 1;
  struct UIKNode *node;
  UWORD startfold;

    if (dinfo)
        {
        dinfo->DisplayNum = 0;
        dinfo->ScrollNum  = 0;
        }
    if (sline == -1) sline = last; if (sline < 0 || sline > last) goto END_ERROR;
    if (eline == -1) eline = last; if (eline < 0 || eline > last) goto END_ERROR;
    if (eline < sline) goto END_ERROR;

    node = &AGP.FObj->Text.UNode[sline];
    startfold = node->Reserved;
    if (startfold != 0xffff)
        {
        /* On vérifie que la ligne n'est pas déjà un début de fold
         */
        if (fo_GetLimits( sline, (UWORD)-1, &l1, &l2, 1 ))
            { if (l1 == sline) goto END_ERROR; }

        /* On vérifie qu'un sous-fold n'est pas égal au maximum 0xffff.
         */
        for (i=sline; i <= eline; i++, node++)
            { if (node->Reserved == 0xffff) goto END_ERROR; }

        /* On incrémente le marqueur de fold
         */
        node = &AGP.FObj->Text.UNode[sline];
        for (i=sline; i <= eline; i++, node++)
            {
            //if (node->Reserved != startfold) break;
            node->Reserved++;
            }
        if (dinfo)
            {
            dinfo->DisplayFrom = sline;
            dinfo->DisplayNum  = eline - sline + 1;
            }
        }
  END_ERROR:
    return(0);
}

LONG func_FoldUnset( LONG line, struct AZDisplayCmd *dinfo )
{
  ULONG i;
  UWORD dinfodone=0;
  LONG sline, eline, last = AGP.FObj->Text.NodeNum - 1, add=0;
  struct UIKNode *node;

    if (dinfo)
        {
        dinfo->DisplayNum = 0;
        dinfo->ScrollNum  = 0;
        }
    if (line == -1) line = last; if (line < 0 || line > last) return(0);

    if (fo_IsLineFolded( line ))
        {
        add = func_FoldOff( line, dinfo );
        dinfodone = 1;
        }

    if (fo_GetLimits( line, (UWORD)-1, &sline, &eline, 0 ))
        {
        node = &AGP.FObj->Text.UNode[sline];
        for (i=sline; i <= eline; i++, node++) node->Reserved--;
        if (dinfo && !dinfodone)
            {
            dinfo->DisplayFrom = sline;
            dinfo->DisplayNum  = eline - sline + 1;
            }
        }

    return( add );
}

//-------------------------------------------------------------------------------------

struct list_fold
    {
    ULONG errmsg;
    ULONG (*func)();
    APTR parm;
    LONG real;
    LONG startfoldindex;
    LONG startfoldreal;
    UIKBUF **pub;
    struct UIKPList *startlist;
    WORD nil;
    };

/*
static void list_one_fold_byline( struct UIKPList *ul, struct list_fold *lf )
{
  register LONG ind;
  register struct UIKNode *node;

    for (node=ul->UNode, ind=0; ind < ul->NodeNum; ind++, node++)
        {
        lf->real++;
        if (lf->errmsg = (*lf->func)( lf->parm, lf->real, ul, ind )) return;

        if (node->Flags & UNF_FOLDED)
            {
            struct UIKPList *sub = (struct UIKPList *) BufGetUDatas( node->Buf, &lf->nil );
            list_one_fold_byline( sub, lf );
            if (lf->errmsg) return;
            }
        }
}
*/

static void open_id( struct list_fold *lf, UWORD from, UWORD to, ULONG folded, ULONG foldedid )
{
    if (lf->pub)
        {
        struct memfold MF;

        for (; from <= to; from++)
            {
            MF.sline  = lf->real;
            MF.eline  = -1;
            MF.folded = 0;
            MF.id     = from;
            if (folded && from == foldedid) MF.folded = 1;
            if (! BufPasteS( (APTR)&MF, sizeof(struct memfold), lf->pub, BufLength(*lf->pub) )) break;
            }
        }
}

static void close_id( struct list_fold *lf, LONG endline, UWORD close_id )
{
    if (lf->pub)
        {
        struct memfold *mf;
        LONG i, num = BufLength(*lf->pub) / sizeof(struct memfold);

        for (mf=&((struct memfold *)(*lf->pub))[num-1], i=num-1; i >= 0; i--, mf--)
            {
            if (mf->id >= close_id) {
            if (mf->eline == -1) mf->eline = endline; }
            else break;
            }
        }
}

// le delta n'est valable que pour la ligne de tête de liste

static void list_one_fold_byline( struct UIKPList *ul, struct list_fold *lf, UWORD base_id, UWORD delta_id )
{
  register LONG ind;
  register struct UIKNode *node;
  register UWORD curr_id, line_id, pass_id;

    curr_id = base_id + delta_id;

    for (node=ul->UNode, ind=0; ind < ul->NodeNum; ind++, node++)
        {
        lf->real++;
        if (ul == lf->startlist)
            {
            lf->startfoldindex = ind;       // index dans le texte principal
            lf->startfoldreal = lf->real;   // index dans le texte entier
            }

        line_id = pass_id = base_id + node->Reserved;

        if (lf->func)
            {
            if (node->Flags & UNF_FOLDED) pass_id++;
            if (lf->errmsg = (*lf->func)( lf->parm, lf->real, ul, ind, lf->startfoldindex, lf->startfoldreal, pass_id )) return;
            }

        if (line_id < curr_id) close_id( lf, lf->real-1, line_id+1 ); // ferme tout ce qui est supérieur à line_id

        if (node->Flags & UNF_FOLDED)
            {
            UWORD newbase_id, subdelta_id;
            struct UIKPList *sub = (struct UIKPList *) BufGetUDatas( node->Buf, &subdelta_id );

            newbase_id = line_id + 1;
            open_id( lf, curr_id+1, newbase_id+subdelta_id, 1, newbase_id );

            if (sub) list_one_fold_byline( sub, lf, newbase_id, subdelta_id );
            else close_id( lf, lf->real, newbase_id+subdelta_id ); // si une seule ligne, pas de liste

            if (lf->errmsg) return;
            }
        else{
            if (line_id > curr_id)
                {
                open_id( lf, curr_id+1, line_id, 0, 0 );
                }
            }
        curr_id = line_id;
        }
    close_id( lf, lf->real, base_id ); // ferme tous les folds ouverts dans une sous-liste
}

ULONG func_FoldApplyLines( ULONG (*func)(), APTR parm, UIKBUF **pub )
{
  struct list_fold LF;

    LF.errmsg = 0;
    LF.func = func;
    LF.parm = parm;
    LF.real = -1;
    LF.startlist = &AGP.FObj->Text;
    LF.pub  = pub;
    list_one_fold_byline( &AGP.FObj->Text, &LF, 0, 0 );
    return( LF.errmsg );
}
