/****************************************************************
 *
 *      File:       funcs_text.c    manipule le texte
 *      Project:    AZur            editeur de texte
 *
 *      Created:    17/06/93        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"

/* this macro lets us long-align structures on the stack */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((LONG)(a_##name+3) & ~3);

#define DBUG(a)


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/


/****** Statics *************************************************/


/****************************************************************
 *
 *      Utils
 *
 ****************************************************************/

struct interval {
    ULONG   start;
    ULONG   len;
    };

#define INTADD(a,b) { ITV.start=(a); ITV.len=(b); if (! BufPasteS( (APTR)&ITV, sizeof(struct interval), pub, BufLength(*pub) )) break; }
#define INTNUM() (BufLength(*pub)/sizeof(struct interval))
#define INTPTR(a) ((struct interval*)(ublist+(a)*sizeof(struct interval)))

static ULONG build_interval( UBYTE **pub, UBYTE **pptr, ULONG start, ULONG len )
{
  struct interval ITV;
  ULONG actlen, num;
  UBYTE *p;

    for (actlen=0, p=*pptr+start; ; )
        {
        if ((num = find_char( p, ' ', ' ' )) == 0) break;
        if ((actlen += num) > len) break;
        p += num;
        if ((num = skip_char( p, ' ', ' ' )) == 0) break;
        if ((actlen += num) > len) break;
        INTADD( p-*pptr, num );
        p += num;
        }
    return( INTNUM() );
}

ULONG func_TextJustify( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG funcflags, struct AZDisplayCmd *dinfo )
{
  struct AZObjFile *fo = AGP.FObj;
  struct UIKList *list = &fo->Text;
  ULONG numstart, numend, errmsg=0, numspace, numinter, i, xx, cut;
  LONG line, start, len;
  struct interval *it;
  UBYTE **pptr, *ublist=0;

    if (! (ublist = BufAllocP( AGP.Pool, 0, 50*sizeof(struct interval) ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }

    for (line=sline; line <= eline; line++)     // ecol est la colonne juste après le texte
        {
        pptr = &list->UNode[line].Buf;
        len = intAZ_LineBufLen( *pptr );
        BufTruncate( ublist, 0 );

        start = scol;
        if (len > ecol) len = ecol;

        if (funcflags & FMTF_KEEPALINEA) // conserver les alineas
            {
            if ((line == 0) || !(list->UNode[line-1].Flags & UNF_CONTINUE)) // ligne précédente terminée par un LF ?
                start += skip_char( *pptr+start, ' ', '\t' );
            }

        if (len <= start) continue;
        numstart = skip_char( *pptr+start, ' ', '\t' );
        if (numstart >= len) continue;
        if (func_LinePadSpace( line, ecol ) < ecol) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        len = ecol - start;
        numend = len - skip_endchar( *pptr+start, ' ', '\t', len );
        /* Ici la zone est encadrée entre start et len,
         * et le texte encadré entre numstart et numend.
         */

        if (numend) BufCutS( pptr, start+len-numend, numend );
        if (numstart) BufCutS( pptr, start, numstart );
        numspace = numstart + numend;
        len -= numspace;
        if (len <= 0) continue;
        if (numinter = build_interval( &ublist, pptr, start, len ))
            {
            for (cut=0, i=0; i < numinter; i++)
                {
                it = INTPTR(i);
                it->start -= cut;
                if (it->len > 1)
                    {
                    BufCutS( pptr, it->start, xx = it->len-1 );
                    cut += xx;
                    len -= xx;
                    numspace += xx;
                    it->len = 1;
                    }
                }
            }
        if (numspace)
            {
            switch (funcflags & JUS_MASK)
                {
                case JUS_CENTER:
                    func_BufPadS( ' ', numspace/2, pptr, start+len );
                    numspace -= (numspace / 2);
                    break;

                case JUS_LEFT:
                    start += len;
                    break;

                case JUS_RIGHT:
                    break;

                case JUS_FLUSH:
                    if (numinter)
                        {
                        ULONG numadd, every, pred;

                        //--- Ajout des espaces partout
                        if (numadd = numspace / numinter)
                            {
                            for (pred=0, i=0; i < numinter; i++)
                                {
                                it = INTPTR(i);
                                it->start += pred;
                                if (! func_BufPadS( ' ', numadd, pptr, it->start )) break;
                                it->len += numadd;
                                pred += numadd;
                                numspace -= numadd;
                                }
                            }
                        if (numspace)
                            {
                            every = numinter / numspace;
                            for (pred=0, i=0; i < numinter && numspace; i++)
                                {
                                if (i % every == 0)
                                    {
                                    it = INTPTR(i);
                                    it->start += pred;
                                    if (! func_BufPadS( ' ', 1, pptr, it->start )) break;
                                    it->len++;
                                    pred++;
                                    numspace--;
                                    }
                                }

                            }
                        if (numspace) start = (INTPTR(0)->start);
                        }
                    else start += len;
                    break;
                }
            if (numspace) func_BufPadS( ' ', numspace, pptr, start );
            if (TST_STRIPCHANGE) func_LineTruncateSpace( line );
            }
        }

  END_ERROR:
    if (dinfo)
        {
        dinfo->DisplayNum  = eline - sline + 1;
        dinfo->DisplayFrom = sline;
        dinfo->ScrollNum   = 0;
        dinfo->Line        = eline;
        }
    SET_MODIFIED( sline, scol );
    BufFree( ublist );
    return( errmsg );
}

//--------------------------------------------------------------------------------

ULONG func_TextFormat( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG funcflags, struct AZDisplayCmd *dinfo )
{
  struct AZObjFile *fo = AGP.FObj;
  struct UIKList *list = &fo->Text;
  UBYTE **pbl, *ptr, *pnext, *bmname = "___$$$TmpTextFormat";
  LONG lscol, oldeline, oteline, lineend, len, cutlen, start, num, i, oldlen, nextlen;
  ULONG errmsg=0, modified=0, do_alinea;
  struct AZBM *bookmark = 0;

    oldeline = eline;
    fo->WorkLine = fo->Line;
    fo->WorkCol = fo->Col;

    oteline = -1;
    if (funcflags & FMTF_ONLYTWO) oteline = sline + 2;

    for (i=sline; i <= eline; i++)
        {
        if (i == oteline && modified == 0) break;

        pbl = &list->UNode[i].Buf;  // Laisser : UNode peut changer quand une ligne est ajoutée
        len = intAZ_LineBufLen( ptr = *pbl );
        lscol = scol;

        do_alinea = 0;
        if (funcflags & FMTF_KEEPALINEA)
            {
            if ((i == 0) || !(list->UNode[i-1].Flags & UNF_CONTINUE)) // ligne précédente terminée par un LF ?
                {
                do_alinea = 1;
                }
            }

        //--- Rien à faire si que des espaces ou si ligne vide
        if (0 == (lineend = skip_endchar( ptr, ' ', '\t', len ))) continue;

        //--- Coupe les espaces de fin de ligne
        if (lineend < len) { BufTruncate( ptr, lineend ); len = lineend; }

        //--- Conserver les alineas
        if (do_alinea)
            {
            lscol += skip_char( ptr+lscol, ' ', '\t' );
            }

        //--- Insère des espaces pour arriver à la bonne marge
        if ((start = skip_char( ptr, ' ', '\t' )) < lscol)
            {
            if (! do_alinea)
                {
                num = lscol - start;
                if (ptr = func_BufPadS( ' ', num, pbl, 0 ))
                    {
                    len += num;
                    modified = 1; if (!bookmark) bookmark = (struct AZBM *) func_BMAdd( bmname, fo->Line, fo->Col );
                    func_BMAddChars( i, 0, num );
                    }
                else ptr = *pbl;
                }
            }
        else if (num = start - lscol)
            {
            modified = 1; if (!bookmark) bookmark = (struct AZBM *) func_BMAdd( bmname, fo->Line, fo->Col );
            ptr = BufCutS( pbl, lscol, num ); func_BMRemChars( i, lscol, num );
            len -= num;
            }

        //--------------- Enlève la fin de ligne si JOINPARA
        if (i < eline && !(list->UNode[i].Flags & (UNF_CONTINUE+UNF_ENDSPACE)) && (funcflags & FMTF_JOINPARA))
            {
            modified = 1;
            list->UNode[i].Flags |= (UNF_CONTINUE+UNF_ENDSPACE);
            }

        //--------------- Coupe la ligne si trop longue
        if (len > ecol)
            {
            for (cutlen=lineend=len; lineend > ecol && lineend && lineend > lscol; )
                {
                oldlen  = cutlen;
                cutlen  = find_endchar( ptr, ' ', '\t', lineend );
                lineend = skip_endchar( ptr, ' ', '\t', cutlen  );
                }
            if (lineend == 0)   // prend le mot même si trop long
                if (oldlen != len)
                    lineend = skip_endchar( ptr, ' ', '\t', cutlen = oldlen  );

            //--- Si ce sont des espaces jusqu'au début : on ne coupe pas
            if (lineend && lineend > lscol)
                {
                //--- Coupe la ligne
                modified = 1; if (!bookmark) bookmark = (struct AZBM *) func_BMAdd( bmname, fo->Line, fo->Col );
                if (errmsg = func_intSplitLine( i, cutlen, 1, 0 )) goto END_ERROR; // peut changer list->UNode !!!
                BufTruncate( list->UNode[i].Buf, lineend );
                list->UNode[i].Flags |= (UNF_CONTINUE+UNF_ENDSPACE);
                eline++;  // ««««««««««««««« Reprend la boucle entière sur la ligne ajoutée
                }
            }
        //--------------- Colle la ligne du dessous si trop courte (++len car ajout d'espace)
        else if (++len < ecol && i < eline && ((list->UNode[i].Flags & UNF_CONTINUE) || (funcflags & FMTF_JOINPARA)))
            {
            // Pour rapidité : est-ce que la ligne du dessous a un mot qui pourrait combler ?
            num = ecol - len;
            pnext = intAZ_LineBuf( i+1 );
            pnext += skip_char( pnext, ' ', '\t' );
            nextlen = find_char( pnext, ' ', '\t' );
            if (/*nextlen &&*/ nextlen <= num)
                {
                modified = 1; if (!bookmark) bookmark = (struct AZBM *) func_BMAdd( bmname, fo->Line, fo->Col );
                func_intJoinLine( i, len, 1, 1, 0 );
                eline--;
                i--;  // ««««««««««««««« Reprend la boucle entière sur la même ligne
                }
            }
        }
  END_ERROR:
    if (dinfo)
        {
        if (modified)
            {
            dinfo->DisplayNum  = eline - sline + 1;
            dinfo->DisplayFrom = sline;
            dinfo->ScrollNum   = oldeline - eline;
            dinfo->ScrollFrom  = eline + 1;
            }
        else{
            dinfo->DisplayNum  = 0;
            dinfo->ScrollNum   = 0;
            }
        dinfo->Line = eline;
        }
    if (bookmark)
        {
        fo->WorkLine = bookmark->Line;
        fo->WorkCol = bookmark->Col;
        func_BMRemove( bmname, 0 );
        }
    if (modified) SET_MODIFIED( sline, scol );
    return( errmsg );
}

void func_AutoFormat( void )
{
  struct AZDisplayCmd DI;
  struct AZObjFile *fo = AGP.FObj;
  LONG sline, eline, scol, ecol, max, len;
  UBYTE *ptr;

    if (TST_AUTOFORMAT && TST_USEMARGIN)
        {
        sline = eline = fo->Line;
        if (sline > 0 && (fo->Text.UNode[sline-1].Flags & UNF_CONTINUE))
            sline--;     // pour reprendre la fin de ligne si on tape contre un autre mot

        ecol = intAZ_Buf_Len( sline, &ptr );
        scol = skip_char( ptr, ' ', '\t' );     // cherche la plus longue du paragraphe
        max = fo->Text.NodeNum - 1;
        while ((fo->Text.UNode[eline].Flags & UNF_CONTINUE) && (eline < max))
            {
            eline++;
            if ((len = intAZ_LineLen( eline )) > ecol) ecol = len;
            }
        if (TST_USEMARGIN) { scol = fo->LeftMargin; ecol = fo->RightMargin; }

        func_TextFormat( sline, scol, eline, ecol, FMTF_KEEPALINEA | FMTF_ONLYTWO, &DI );
        if (DI.DisplayNum)
            {
            func_DisplayCmd( &DI );
            func_CursorPos( fo->WorkLine, fo->WorkCol );
            fo->CEdCol = fo->WorkCol;
            }
        }
}
