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

ULONG func_TextPrep( ULONG startline, ULONG startcol, ULONG flags, ULONG maxlines, ULONG indtype );


/****** Statics *************************************************/


/****************************************************************
 *
 *      Utils
 *
 ****************************************************************/

static void move_foldinfo( LONG fromline, LONG toline )
{
    if (fo_IsLineFolded( fromline ))
        {
        struct UIKPList *li = &AGP.FObj->Text;
        struct UIKNode *n1 = &li->UNode[fromline];
        struct UIKNode *n2 = &li->UNode[toline];
        UWORD delta_id;
        struct UIKPList *sublist = (struct UIKPList *) BufGetUDatas( n1->Buf, &delta_id );
        BufSetUDatas( n1->Buf, 0, 0 );                     n1->Flags &= ~UNF_FOLDED;
        BufSetUDatas( n2->Buf, delta_id, (ULONG)sublist ); n2->Flags |= UNF_FOLDED;
        }
}

ULONG func_intSplitLine( LONG line, LONG col, ULONG dobm, ULONG nil /*newline*/ )
{
  struct UIKPList *l = &AGP.FObj->Text;
  UBYTE *ptr, **pptr;
  ULONG len = intAZ_LineLen( line );

    //func_BlockClearAll();    ne pas mettre car appelée depuis DelText()

    if (col < len)
        {
        if (col == 0 /*&& newline*/) // on troue le fold et on ajoute un newline
            {
            if (! UIK_nl_AllocNode( l, line, 0, 0 )) return(TEXT_NOMEMORY);
            if (line > 0 && l->UNode[line-1].Reserved) // on garde le fold si ligne précédente est dans un fold
                l->UNode[line].Reserved = l->UNode[line+1].Reserved; // reste dans le fold
            }
        else{         // on garde le fold de la ligne précédente
            len -= col;
            if (! (ptr = UIK_nl_AllocNode( l, line+1, len, 0 ))) return(TEXT_NOMEMORY);

            l->UNode[line+1].Reserved = l->UNode[line].Reserved; // reste dans le fold
            l->UNode[line+1].Flags |= (l->UNode[line].Flags & (UNF_CONTINUE|UNF_ENDSPACE));
            l->UNode[line].Flags &= ~(UNF_CONTINUE|UNF_ENDSPACE);

            pptr = &l->UNode[line].Buf; // !!!! car AllocNode réalloue la table des nodes !!!!
            CopyMem( *pptr + col, ptr, len );
            BufTruncate( *pptr, col );
            }
        if (dobm) func_BMSplitLine( line, col );
        return(0);
        }
    return(-1);
}

ULONG func_SplitLine( LONG line, LONG col )
{
    func_BlockClearAll();
    SET_MODIFIED( line, col );
    return( func_intSplitLine( line, col, 1, 0 ) );
}

ULONG func_intJoinLine( LONG line, LONG col, ULONG dobm, ULONG skipspace, ULONG delfold )
{
  struct UIKPList *l = &AGP.FObj->Text;
  UBYTE *p, *ub2, **pptr = &l->UNode[line].Buf;  // !!!! car AllocNode réalloue la table des nodes !!!!
  ULONG i, lenub2, start2=0, len = intAZ_LineBufLen( *pptr ), flags1, flags2;

    if (line >= l->NodeNum - 1) return(-1);
    flags1 = l->UNode[line].Flags;
    flags2 = l->UNode[line+1].Flags;

    // func_BlockClearAll(); ne pas mettre car appelée depuis DelText()

    if (col == 0 && len == 0) // pour garder les folds
        {
        if ((flags1 & UNF_FOLDED) && (delfold == 0)) return(-1);
        if (dobm) func_BMJoinLines( line, len );
        Text_FreeNodes( l, line, line );
        }
    else{
        if ((flags1 & UNF_FOLDED) && (flags2 & UNF_FOLDED)) return(-1);

        lenub2 = intAZ_Buf_Len( line+1, &ub2 );
        if (lenub2)
            {
            if (col > len)
                {
                if (! BufResizeS( pptr, col )) return( TEXT_NOMEMORY );
                for (p=*pptr, i=len; i < col; i++) p[i] = ' ';
                len = col;
                }
            if (skipspace)
                {
                start2 = skip_char( ub2, ' ', '\t' );
                if (start2 && dobm) func_BMRemChars( line+1, 0, start2 );
                }
            if (! BufPasteS( ub2+start2, intAZ_LineBufLen(ub2)-start2, pptr, len )) return( TEXT_NOMEMORY );
            }
        l->UNode[line].Flags &= ~(UNF_CONTINUE|UNF_ENDSPACE);
        l->UNode[line].Flags |= (flags2 & (UNF_CONTINUE|UNF_ENDSPACE));

        if (dobm) func_BMJoinLines( line, len );
        if (flags2 & UNF_FOLDED) move_foldinfo( line+1, line );
        Text_FreeNodes( l, line+1, line+1 );
        }
    return(0);
}

ULONG func_JoinLine( LONG line, LONG col )
{
    func_BlockClearAll();
    SET_MODIFIED( line, col );
    return( func_intJoinLine( line, col, 1, 0, 0 ) );
}

ULONG func_LinePadSpace( LONG line, LONG col )  // retourne la longueur de la ligne
{
  ULONG len, i;
  UBYTE **pptr, *p;
                      // !!!! car AllocNode réalloue la table des nodes !!!!
    pptr = &AGP.FObj->Text.UNode[line].Buf; len=intAZ_LineBufLen(*pptr);
    if (col > len)
        {
        if (! BufResizeS( pptr, col )) return( len );
        for (p=*pptr, i=len; i < col; i++) p[i] = ' ';
        SET_MODIFIED( line, col );
        return( (ULONG)col );
        }
    return( len );
}

ULONG func_LineTruncateSpace( LONG line ) // ne doit pas changer l'UIKBuffer
{
  UBYTE *ptr;
  ULONG lineend, len; // = intAZ_LineBufLen(ptr);

    len = intAZ_Buf_Len( line, &ptr );
    lineend = skip_endchar( ptr, ' ', '\t', len );
    if (lineend < len) { BufTruncate( ptr, lineend ); SET_MODIFIED( line, lineend ); }
    return( lineend );
}

ULONG func_CharFirst( UBYTE *ptr, ULONG len )
{
  ULONG lineend, i = 0;

    lineend = skip_endchar( ptr, ' ', '\t', len );
    for (i=0; i < lineend && ptr[i] == ' '; i++);
    return( i );
}

/****************************************************************
 *
 *      Utils
 *
 ****************************************************************/

void text_sync()
{
  struct AZObjFile *fo = AGP.FObj;
  struct UIKPList *l = &fo->Text;

    if (fo->Line >= l->NodeNum) fo->Line = l->NodeNum - 1;
    if (fo->Col == -1) fo->Col = intAZ_LineLen( fo->Line );
}

void text_reset()
{
  struct AZObjFile *fo = AGP.FObj;

    fo->Line = fo->Col = fo->OldLine = fo->OldCol = 0;
    //if (TST_USEMARGIN) { if (fo->Col > fo->RightMargin) fo->Col = fo->RightMargin; }
    fo->FoundCol = -1;
    func_BMRemAll();
    func_BlockRemAll( 0, 0 );
    func_BlockStart( 0, 0, 0 );
    func_TextPrep( 0, 0, 0, 0, 0 );
}

/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

LONG func_FindIndentPos( LONG line, ULONG indtype )   // 0:début, 1:left, 2:indent, 3:right, 4:ttx
{
  LONG col;
  UBYTE *ptr;

    if (indtype == INDENT_TTX)
        {
        ptr = intAZ_LineBuf( line );
        col = skip_char( ptr, ' ', '\t' );
        if (ptr[col] == 0) col = AGP.FObj->Col;
        }
    else{
        while (TRUE)
            {
            ULONG len = intAZ_Buf_Len( line, &ptr );
            col = skip_endchar( ptr, ' ', '\t', len );
            if (col || (line == 0)) break;
            line--;
            }
        if (col) col = skip_char( ptr, ' ', '\t' );
        }

    switch (indtype)
        {
        case 0: col = 0; break;
        case INDENT_LEFT: col = prev_tab( col ); break;
        case INDENT_STAY: /*col = pos_tab( col );*/ break;
        case INDENT_RIGHT: col = next_tab( col ); break;
        }
    return( col );
}

UBYTE *func_CharPrev( UBYTE *ptr, LONG *plen, LONG *pline, LONG *pcol, UBYTE *pch )
{
  LONG line=*pline, col=*pcol;

    if (col == 0)
        {
        if (line <= 0) return( (APTR)-1 );
        col = *plen = intAZ_Buf_Len( --line, &ptr );
        *pch = '\n';
        }
    else *pch = intAZ_CharCurrent( --col, *plen, ptr );

    *pline = line; *pcol = col;
    return( ptr );
}

UBYTE *func_CharNext( UBYTE *ptr, LONG *plen, LONG *pline, LONG *pcol, UBYTE *pch )
{
  LONG col = *pcol;

    if (++col < *plen)
        {
        *pch = ptr[col];
        *pcol = col;
        }
    else{
        struct AZObjFile *fo = AGP.FObj;
        LONG line = *pline;

        if (line == fo->Text.NodeNum - 1) // cas spécial car la dernière ligne n'a pas de \n
            {
            col = *plen;
            *pch = ' ';
            ptr = (APTR)-1;
            }
        else if (col == *plen)
            {
            *pch = '\n';
            }
        else{
            col = 0; *plen = intAZ_Buf_Len( ++line, &ptr );
            *pline = line;
            *pch = intAZ_CharCurrent( col, *plen, ptr );
            }
        *pcol = col;
        }

    return( ptr );
}

UBYTE *func_PosPrev( LONG *pline, LONG *pcol )
{
  ULONG len;
  UBYTE *ptr, ch;

    len = intAZ_Buf_Len( *pline, &ptr );
    return( func_CharPrev( ptr, &len, pline, pcol, &ch ) );
}

UBYTE *func_PosNext( LONG *pline, LONG *pcol )
{
  ULONG len;
  UBYTE *ptr, ch;

    len = intAZ_Buf_Len( *pline, &ptr );
    return( func_CharNext( ptr, &len, pline, pcol, &ch ) );
}

LONG func_CountTabs( LONG col, LONG tabs )
{
  ULONG i;

    if (tabs > 0)
        {
        for (i=0; i < tabs; i++) col = next_tab( col );
        }
    else{
        tabs = -tabs;
        for (i=0; i < tabs && col > 0; i++) col = prev_tab( col );
        }
    return( col );
}

ULONG func_CountWords( LONG line, LONG col, LONG words, LONG *resline, LONG *rescol, ULONG real )
{
  ULONG chars=0, numwords=0, len;
  LONG x=col, y=line, savex, savey;
  UBYTE *ptr, ch, status_word=0;

    len = intAZ_Buf_Len( y, &ptr );
    ch = intAZ_CharCurrent( col, len, ptr );

    if (words <= 0)
        {
        words = -words;
        savex = x; savey = y;
        if ((ptr = func_CharPrev( ptr, &len, &y, &x, &ch )) != (APTR)-1)
            {
            status_word = func_CharIsWord( ch ) ? 1 : 0;
            if (words == 0 && status_word == 0) // on reste au début du même mot
                {
                x = savex; y = savey;
                }
            else{
                --chars;
                if (real)
                    {
                    savex = x; savey = y;
                    while ((ptr = func_CharPrev( ptr, &len, &y, &x, &ch )) != (APTR)-1)
                        {
                        if (! func_CharIsWord( ch ))
                            {
                            if (status_word == 1)
                                {
                                if (++numwords >= words) break;
                                status_word = 0;
                                }
                            }
                        else { if (status_word == 0) status_word = 1; }
                        savex = x; savey = y;
                        --chars;
                        }
                    x = savex; y = savey;
                    }
                else{
                    while ((ptr = func_CharPrev( ptr, &len, &y, &x, &ch )) != (APTR)-1)
                        {
                        if (func_CharIsWord( ch ))
                            { if (status_word == 0) { numwords++; status_word = 1; } }
                        else{ if (status_word == 1) { numwords++; status_word = 0; } }

                        if (numwords >= words) break;
                        --chars;
                        }
                    }
                }
            }
        }
    else{
        if (col > len) ch = '\n'; // fin de ligne compte pour un mot

        if (ptr != (APTR)-1)
            {
            status_word = func_CharIsWord( ch ) ? 1 : 0;
            while ((ptr = func_CharNext( ptr, &len, &y, &x, &ch )) != (APTR)-1)
                {
                if (func_CharIsWord( ch ))
                    { if (status_word == 0) { numwords++; status_word = 1; } }
                else{ if (status_word == 1) { numwords++; status_word = 0; } }

                ++chars;
                if (numwords >= words)
                    {
                    if (real) { if (status_word == 1) break; }
                    else break;
                    }
                }
            }
        }

    if (resline) *resline = y;
    if (rescol) *rescol = x;
    if (real && status_word == 0) return(0);
    return( chars );
}

ULONG func_Curs2FileName( UBYTE **pub )
{
  ULONG errmsg=0, alloc=0, len;
  LONG scol, ecol;
  UBYTE *ptr;

    ptr = func_LineBuf( AGP.FObj->Line );
    len = func_LineBufLen( ptr );
    scol = AGP.FObj->Col;
    if (! func_CharIsFile( ptr[scol] ))
        if (scol > 0) scol--;
    if (func_CharIsFile( ptr[scol] ))
        {
        ecol = scol;
        for (; scol > 0; scol--) if (! func_CharIsFile( ptr[scol-1] )) break;
        for (; ecol < len; ecol++) if (! func_CharIsFile( ptr[ecol+1] )) break;
        if (len = ecol - scol + 1)
            {
            if (! *pub) // buffer peut être pré-alloué
                {
                if (! (*pub = BufAlloc( 16, len, MEMF_ANY ))) errmsg = TEXT_NOMEMORY;
                else alloc = 1;
                }
            if (*pub)
                {
                if (! BufSetS( &ptr[scol], len, pub ))
                    {
                    errmsg = TEXT_NOMEMORY;
                    if (alloc) BufFree( *pub );
                    }
                }
            }
        }
    return( errmsg );
}

//---------------------------------------------------------------------------

static ULONG clip_buf( APTR clip, UBYTE *buf, ULONG len, LONG line )
{
    if (len)
        {
        if (! func_CBWrite( clip, buf, len )) return( TEXT_ERR_WriteClip );
        AGP.Tmp += len;
        }
    return(0);
}

ULONG func_Text2CBRect( ULONG unit, LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect )
{
  ULONG error;
  APTR clip;

    if (unit > 255) return( TEXT_ERR_BadClipUnit );
    if (clip = func_CBWriteStart( unit ))
        {
        AGP.Tmp = 0; // totlen
        error = func_TextApply( sline, scol, eline, ecol, rect, 0, clip_buf, clip );
        func_CBWriteEnd( clip, AGP.Tmp );
        }
    else error = TEXT_ERR_OpenClip;
#ifdef AZUR_DEMO
    if (AGP.Tmp > AZURDEMO_MAXSIZE+1) SET_READONLY;
#endif

    return( error );
}

ULONG func_Text2CB( LONG line, LONG col, LONG chars, LONG lines )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG diff;
  ULONG len, errmsg=0;

    if (line < 0 || line >= fo->Text.NodeNum) return(0);

    if (chars)
        {
        //------ Ajustement pour valeur négative
        if (chars < 0)
            {
            col += chars;
            if (col >= 0)    // évite de ralonger la ligne
                {
                len = intAZ_LineLen( line );
                if (col >= len) return(0);
                else{
                    chars = -chars;     // positif
                    if (col + chars > len) chars = len - col;  // enlève juste le bout de la ligne
                    }
                }
            else{
                while (col < 0)
                    {
                    if (--line < 0)
                        {
                        chars -= col; // diminue le nombre de chars à enlever
                        line = col = 0;
                        }
                    else{
                        len = intAZ_LineLen( line );    // virtuel fin de ligne
                        col = len + 1 + col; // +1 pour fins de lignes
                        }
                    }
                chars = -chars;
                }
            }
        if (chars)
            {
            LONG eline=line, ecol, len=intAZ_LineLen( line );
            if (col > len) col = len; // différent de la routine de Del car elle fait func_LinePadSpace()
            ecol = col + chars - 1;

            while (ecol >= (len + 1))    // +1 pour fin de ligne
                {
                ecol -= (len + 1);      // +1 pour fin de ligne

                if (++eline >= fo->Text.NodeNum)
                    {
                    eline = fo->Text.NodeNum - 1;
                    ecol = intAZ_LineLen( eline ) - 1; // -1 car nombre de lignes + 1 si \n
                    break;
                    }
                else{
                    len = intAZ_LineLen( eline );
                    }
                }
            errmsg = func_Text2CBRect( AGP.Prefs->DelClip, line, col, eline, ecol, 0 );
            }
        }
    else if (lines)
        {
        //------ Ajustement pour valeur négative
        if (lines < 0)
            {
            if ((diff = line + lines) < 0)
                {
                line = 0;
                lines -= diff;
                }
            else{
                line += lines;
                }
            lines = -lines;
            }

        //------ Suppression des lignes
        if (lines)
            {
            LONG eline = line + lines - 1;

            if (eline >= fo->Text.NodeNum) eline = fo->Text.NodeNum - 1;
            if (line <= eline)
                errmsg = func_Text2CBRect( AGP.Prefs->DelClip, line, 0, eline, -1, 0 );
            }
        }
    return( errmsg );
}

//---------------------------------------------------------------------------

LONG func_TextDel( LONG line, LONG col, LONG chars, LONG lines, struct AZDisplayCmd *dinfo )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG diff, draw=0;
  ULONG len;

    if (line < 0 || line >= fo->Text.NodeNum) return(0);

    fo->WorkLine = line;
    fo->WorkCol = col;

    if (chars)
        {
        //------ Ajustement pour valeur négative
        if (chars < 0)
            {
            col += chars;
            if (col >= 0)    // évite de ralonger la ligne
                {
                len = intAZ_LineLen( line );
                if (col >= len)
                    {
                    chars = 0; // pas besoin de faire quoi que ce soit
                    fo->WorkLine = line;
                    fo->WorkCol = col;
                    if (dinfo)
                        {
                        dinfo->DisplayNum  = 1;
                        dinfo->DisplayFrom = line;
                        dinfo->ScrollNum   = 0;
                        dinfo->Line = line; dinfo->Col = col;
                        }
                    return(1);
                    }
                else{
                    chars = -chars;     // positif
                    if (col + chars > len) chars = len - col;  // enlève juste le bout de la ligne
                    }
                }
            else{
                while (col < 0)
                    {
                    if (--line < 0)
                        {
                        chars -= col; // diminue le nombre de chars à enlever
                        line = col = 0;
                        }
                    else{
                        len = intAZ_LineLen( line );
                        col = len + 1 + col; // +1 pour fins de lignes
                        }
                    }
                chars = -chars;
                }
            }
        else{
            //------ Ajuste la longueur de la ligne de départ pour le join
            if (func_LinePadSpace( line, col ) < col) return(0);
            }

        //------ Suppression des caractères
        if (chars)
            {
            LONG eline=line, ecol=col+chars-1, len=intAZ_LineLen( line );

            while (ecol >= (len + 1))    // +1 pour fin de ligne
                {
                ecol -= (len + 1);      // +1 pour fin de ligne

                if (++eline >= fo->Text.NodeNum)
                    {
                    eline = fo->Text.NodeNum - 1;
                    ecol = intAZ_LineLen( eline ) - 1; // -1 car nombre de lignes + 1 si \n
                    break;
                    }
                else{
                    len = intAZ_LineLen( eline );
                    }
                }

            draw = func_TextDelRect( line, col, eline, ecol, 0, dinfo );
            }
        }
    else if (lines)
        {
        //------ Ajustement pour valeur négative
        if (lines < 0)
            {
            if ((diff = line + lines) < 0)
                {
                line = 0;
                lines -= diff;
                }
            else{
                line += lines;
                }
            lines = -lines;
            }

        //------ Suppression des lignes
        if (lines)
            {
            LONG eline = line + lines - 1;

            if (eline >= fo->Text.NodeNum) eline = fo->Text.NodeNum - 1;
            if (line <= eline)
                {
                Text_FreeNodes( &fo->Text, line, eline );
                func_BMRemLines( line, eline - line + 1 );
                fo->WorkLine = line;
                fo->WorkCol = 0;
                if (dinfo)
                    {
                    dinfo->DisplayNum  = (line >= fo->Text.NodeNum) ? 1 : 0; // pour afficher le fin de texte
                    dinfo->DisplayFrom = line - 1;                           // si supprime dernière ligne
                    dinfo->ScrollNum   = eline + 1 - line;
                    dinfo->ScrollFrom  = line;
                    dinfo->Line = line; dinfo->Col = 0; //col
                    }
                SET_MODIFIED( line, 0 );
                draw = 1;
                }
            }
        }
    return( draw );
}

LONG func_TextDelRect( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, struct AZDisplayCmd *dinfo )
{
  struct AZObjFile *fo = AGP.FObj;
  UBYTE **pptr=&fo->Text.UNode[sline].Buf; // !!!! car AllocNode réalloue la table des nodes !!!!
  ULONG len=intAZ_LineBufLen(*pptr);

    fo->WorkLine = sline;
    fo->WorkCol = scol;

    if (rect)
        {
        LONG num, line, cutlen = ecol - scol + 1;
        for (num=0, line=sline; line <= eline; line++)
            {
            pptr = &fo->Text.UNode[line].Buf; // !!!! car AllocNode réalloue la table des nodes !!!!
            if ((len = intAZ_LineBufLen(*pptr)) > scol)
                {
                if (len <= ecol + 1) BufTruncate( *pptr, scol );
                else BufCutS( pptr, scol, cutlen );
                }
            func_BMRemChars( line, scol, cutlen );
            num++;
            }
        fo->WorkLine = sline;
        fo->WorkCol = scol;
        if (dinfo)
            {
            dinfo->DisplayNum  = num;
            dinfo->DisplayFrom = sline;
            dinfo->ScrollNum   = 0;
            dinfo->Line = sline; dinfo->Col = scol;
            }
        SET_MODIFIED( sline, scol );
        return( num );
        }
    else{
        //------ Ajuste la longueur de la ligne de départ
        if (func_LinePadSpace( sline, scol ) < scol) return(0);

        //------ Delete
        if (sline == eline)
            {
            ULONG join=0;

            if (ecol >= len) { ecol = len - 1; join = 1; }
            if (scol <= ecol)
                {
                BufCutS( pptr, scol, ecol - scol + 1 );
                func_BMRemChars( sline, scol, ecol - scol + 1 );
                }

            if (join) { if (func_intJoinLine( sline, scol, 1, 0, 1 )) join = 0; }

            fo->WorkLine = sline;
            fo->WorkCol = scol;
            if (dinfo)
                {
                dinfo->DisplayNum  = 1;
                dinfo->DisplayFrom = sline;
                dinfo->ScrollNum   = join;
                dinfo->ScrollFrom  = sline + 1;
                dinfo->Line = sline; dinfo->Col = scol;
                }
            SET_MODIFIED( sline, scol );
            return(1);
            }
        else{
            LONG join=0, num=0;

            if (sline + 1 < eline)
                {
                num = eline - sline - 1;
                Text_FreeNodes( &fo->Text, sline+1, eline-1 );
                func_BMRemLines( sline+1, eline - sline + 1 - 2 );
                eline = sline + 1;
                }

            BufTruncate( *pptr, scol );
            func_BMRemChars( sline, scol, MAXSIGNED );

            // !!!! car AllocNode réalloue la table des nodes !!!!
            pptr=&fo->Text.UNode[eline].Buf; len=intAZ_LineBufLen(*pptr);
            if (ecol >= len) { join = 1; ecol = len - 1; }  // peut nécessiter de joindre 3 lignes en 1 seule
            BufCutS( pptr, 0, ecol+1 ); // +1 car ecol comprise
            func_BMRemChars( eline, 0, ecol+1 );
            if (join) { if (func_intJoinLine( eline, 0, 1, 0, 1 )) join--; }

            if (func_intJoinLine( sline, 0, 1, 0, 1 )) join--;

            fo->WorkLine = sline;
            fo->WorkCol = scol;
            if (dinfo)
                {
                dinfo->DisplayNum  = 1;
                dinfo->DisplayFrom = sline;
                dinfo->ScrollNum   = num + join + 1;
                dinfo->ScrollFrom  = sline + 1;
                dinfo->Line = sline; dinfo->Col = scol;
                }
            SET_MODIFIED( sline, scol );
            return(1);
            }
        }
}

struct TextApplyParms {
    UBYTE   *ptr;
    UBYTE   **pbuffer;
    LONG    line;
    LONG    len;
    LONG    ecol;
    ULONG   (*func)();
    APTR    parm;
    UBYTE   *buf;
    ULONG   rect;
    ULONG   lastline;
    UBYTE   real;
    UBYTE   padb;
    UWORD   padw;
    };

static ULONG put_str_rect( struct TextApplyParms *atp )
{
  ULONG rc=0, i, copy, total;
  UBYTE *dst;

    total = atp->ecol + 1;
    copy = (atp->len > total) ? total : atp->len;

    if (atp->real)
        {
        if (copy)
            rc = (*atp->func)( atp->parm, atp->pbuffer, atp->ptr - *atp->pbuffer, copy, atp->line );
        }
    else{
        dst = atp->buf;

        if (copy) { CopyMem( atp->ptr, dst, copy ); dst += copy; }
        if (atp->rect == BLKMODE_FULL) { for (i=copy; i < total; i++) *dst++ = ' '; }
        *dst++ = '\n';
        rc = (*atp->func)( atp->parm, atp->buf, dst - atp->buf, atp->line );
        }
    return( rc );
}

static ULONG put_str( struct TextApplyParms *atp )
{
  ULONG rc=0, copy, total;

    if (atp->real)
        {
        total = atp->ecol + 1;
        copy = (atp->len > total) ? total : atp->len;
        if (copy)
            rc = (*atp->func)( atp->parm, atp->pbuffer, atp->ptr - *atp->pbuffer, copy, atp->line );
        }
    else{
        if (atp->ecol < atp->len)
            {
            rc = (*atp->func)( atp->parm, atp->ptr, atp->ecol + 1, atp->line );
            }
        else{
            LONG len = atp->len;
            if (atp->line != atp->lastline)
                //if (! (AGP.FObj->Text.UNode[atp->line].Flags & UNF_CONTINUE))
                    atp->ptr[len++] = '\n';
            rc = (*atp->func)( atp->parm, atp->ptr, len, atp->line );
            atp->ptr[atp->len] = 0;
            }
        }
    return( rc );
}

static ULONG text_apply( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, ULONG reverse, ULONG (*func)(), APTR parm, ULONG real )
{
  struct AZObjFile *fo = AGP.FObj;
  struct UIKNode *unode = fo->Text.UNode;
  struct TextApplyParms ATP;
  ULONG len, rc;
  LONG line;
  UBYTE *ptr;

    if (eline == -1 || eline >= fo->Text.NodeNum) eline = fo->Text.NodeNum - 1;

    if (!rect)
        {
        len = intAZ_LineLen( eline );
        if (ecol == -1 || ecol > len) ecol = len;
        len = intAZ_LineLen( sline );
        if (scol == -1 || scol > len) scol = len;
        }

    ATP.lastline = fo->Text.NodeNum - 1;
    ATP.func = func;
    ATP.parm = parm;
    ATP.rect = rect;
    ATP.real = real;
    ATP.buf  = 0;
    if (rect || sline == eline)
        {
        if ((ATP.ecol = ecol - scol) < 0) return( TEXT_BadPosition );
        if (! real)
            {
            if (! (ATP.buf = UIK_MemSpAlloc( ATP.ecol + 3, MEMF_ANY ))) return( TEXT_NOMEMORY );
            }
        if (reverse)
            {
            for (line=eline; sline <= line; line--)
                {
                ATP.pbuffer = &unode[line].Buf; ptr = unode[line].Buf;
                ATP.line = line;
                if (rect)
                    {
                    if ((ATP.len = intAZ_LineBufLen(ptr) - scol) >= 0) ATP.ptr = ptr + scol;
                    else ATP.len = 0;
                    rc = put_str_rect( &ATP );
                    }
                else{
                    if ((ATP.len = intAZ_LineBufLen(ptr) - scol) >= 0)
                        {
                        ATP.ptr = ptr + scol;
                        rc = put_str( &ATP );
                        }
                    }
                if (rc) { UIK_MemSpFree( ATP.buf ); return( rc ); }
                }
            }
        else{
            for (line=sline; line <= eline; line++)
                {
                ATP.pbuffer = &unode[line].Buf; ptr = unode[line].Buf;
                ATP.line = line;
                if (rect)
                    {
                    if ((ATP.len = intAZ_LineBufLen(ptr) - scol) >= 0) ATP.ptr = ptr + scol;
                    else ATP.len = 0;
                    rc = put_str_rect( &ATP );
                    }
                else{
                    if ((ATP.len = intAZ_LineBufLen(ptr) - scol) >= 0)
                        {
                        ATP.ptr = ptr + scol;
                        rc = put_str( &ATP );
                        }
                    }
                if (rc) { UIK_MemSpFree( ATP.buf ); return( rc ); }
                }
            }
        UIK_MemSpFree( ATP.buf );
        }
    else{
        if (reverse)
            {
            //------ dernière ligne
            ATP.pbuffer = &unode[eline].Buf; ATP.ptr = unode[eline].Buf;
            ATP.len = intAZ_LineBufLen(ATP.ptr);
            ATP.line = eline;
            ATP.ecol = ecol;
            if (rc = put_str( &ATP )) return( rc );

            //------ milieu
            for (line=eline-1; sline < line; line--)
                {
                ATP.pbuffer = &unode[line].Buf; ATP.ptr = unode[line].Buf;
                ATP.len = intAZ_LineBufLen(ATP.ptr);
                ATP.line = line;
                ATP.ecol = ATP.len;
                if (rc = put_str( &ATP )) return( rc );
               }

            //------ première ligne
            ATP.pbuffer = &unode[sline].Buf; ptr = unode[sline].Buf;
            if ((ATP.len = intAZ_LineBufLen(ptr) - scol) >= 0)
                {
                ATP.line = sline;
                ATP.ecol = ATP.len;
                ATP.ptr = ptr + scol;
                if (rc = put_str( &ATP )) return( rc );
                }
            }
        else{
            //------ première ligne
            ATP.pbuffer = &unode[sline].Buf; ptr = unode[sline].Buf;
            if ((ATP.len = intAZ_LineBufLen(ptr) - scol) >= 0)
                {
                ATP.line = sline;
                ATP.ecol = ATP.len;
                ATP.ptr = ptr + scol;
                if (rc = put_str( &ATP )) return( rc );
                }

            //------ milieu
            for (line=sline+1; line < eline; line++)
                {
                ATP.pbuffer = &unode[line].Buf; ATP.ptr = unode[line].Buf;
                ATP.len = intAZ_LineBufLen(ATP.ptr);
                ATP.line = line;
                ATP.ecol = ATP.len;
                if (rc = put_str( &ATP )) return( rc );
                }

            //------ dernière ligne
            ATP.pbuffer = &unode[eline].Buf; ATP.ptr = unode[eline].Buf;
            ATP.len = intAZ_LineBufLen(ATP.ptr);
            ATP.line = eline;
            ATP.ecol = ecol;
            if (rc = put_str( &ATP )) return( rc );
            }
        }
    return(0);
}

ULONG func_TextApply( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, ULONG reverse, ULONG (*func)(), APTR parm )
{
    return( text_apply( sline, scol, eline, ecol, rect, reverse, func, parm, 0 ) );
}

ULONG func_TextApplyReal( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, ULONG reverse, ULONG (*func)(), APTR parm )
{
    return( text_apply( sline, scol, eline, ecol, rect, reverse, func, parm, 1 ) );
}

//-------------------------------------------------------------

ULONG func_TextPrep( ULONG startline, ULONG startcol, ULONG flags, ULONG maxlines, ULONG indtype )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZTextRP *v = &fo->TRP;
  struct UIKPList *l = &fo->Text;
  ULONG lineend, errmsg=0, linelen;
  UBYTE *ptr;

    v->line = startline;
    v->col = startcol;
    v->flags = flags;
    if (TST_STRIPCHANGE) v->flags |= ARPF_STRIPEOL; else v->flags &= ~ARPF_STRIPEOL;
    v->maxlines = maxlines; if (v->maxlines == 0) v->maxlines = fo->MaxLines;

    v->totlines = 0;
    v->tablen = AGP.Prefs->TabLen;
    v->cr_replaced = 0;

    //------ ajoute une ligne à la fin si demandé
    if (v->line == -1 || v->line >= l->NodeNum)     // changé > en >=
        {
        v->line = l->NodeNum;
        if (l->NodeNum >= v->maxlines) { errmsg = TEXT_MaxLinesReached; goto END_ERROR; }
        if (! (ptr = UIK_nl_AllocNode( l, v->line, 0, 0 ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        linelen = 0;
        v->totlines++;
        }
    else{
        linelen = intAZ_Buf_Len( v->line, &ptr );
        }

    //------ va à la fin de la ligne si demandé
    if (v->col == -1) v->col = linelen;

    //------ indentation de départ
    if (v->flags & ARPF_RECT) v->indcol = v->col;
    else if (indtype)
        {
        v->indcol = func_FindIndentPos( v->line, indtype );
        lineend = skip_endchar( ptr, ' ', '\t', linelen );  // éviter le resize en grand après
        if ((lineend < v->indcol) && (v->flags & ARPF_STRIPEOL))
            { BufTruncate( ptr, v->indcol ); linelen = v->indcol; }
        if (v->col > linelen) v->col = linelen;
        if (v->indcol > v->col) v->col = v->indcol;
        }
    else v->indcol = 0;

    /*
    if (TST_USEMARGIN)
        {
        LONG min = fo->LeftMargin; max = fo->RightMargin;
        if (v->col < min) v->col = min;
        if (v->indcol > min v->indcol = min;
        if (v->col > max) v->col = max;
        if (v->indcol > max) v->indcol = max;
        }
    */
    fo->WorkLine = v->line;
    fo->WorkCol = v->col;

    //------ init pour display éventuel
    v->DC.DisplayNum  = v->totlines;
    v->DC.DisplayFrom = v->line;
    v->DC.ScrollNum   = -v->totlines;
    v->DC.ScrollFrom  = v->line + 1;

  END_ERROR:
    return( errmsg );
}

static ULONG store_std_line_norect( UBYTE *start, UBYTE *ptr )
{
  struct AZTextRP *v = &AGP.FObj->TRP;
  UBYTE *dest;
  ULONG i, num, cols, errmsg=0;

    num = v->col;

    if (v->flags & ARPF_CONTINUE)
        {
        struct UIKNode *node = &AGP.FObj->Text.UNode[v->line]; // !!!! car AllocNode réalloue la table des nodes !!!!
        ULONG valids = intAZ_LineBufLen( node->Buf );

        if (*start == '\n') num = valids;

        if (num > valids) /* rempli le trou entre fin de ligne et début d'insertion */
            {
            if (! BufResizeS( &node->Buf, num )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            dest = node->Buf + valids;
            if (node->Buf == 0) kprintf( "!!! pointeur node->Buf==0 dans store_std_line_norect() !!!\nprévenir JMF merci\n" );
            else{
                /*if (v->indcol > valids)*/ for (; valids < v->indcol; valids++) *dest++ = ' ';
                }
            }
        num -= valids; // rien à copier si CR au début d'un bloc en continuation
        v->flags &= ~ARPF_CONTINUE;
        }
    else{
        struct UIKList *li = &AGP.FObj->Text;


        if (li->NodeNum >= v->maxlines) return( TEXT_MaxLinesReached );
        if (! (dest = UIK_nl_AllocNode( li, ++v->line, num, 0 ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        li->UNode[v->line].Reserved = li->UNode[v->line-1].Reserved; // reste dans le fold

        func_BMAddLines( v->line, 1 );
        v->totlines++;
        if (v->indcol)
            {
            for (i=0; i < v->indcol; i++) *dest++ = ' ';
            num -= v->indcol;
            }
        /*
        else // on insère en début de ligne, alors il faut déplacer le fold vers le bas
            {
kprintf( "---\n" );
            move_foldinfo( v->line-1, v->line );
            }
        */
        }

    func_BMAddChars( v->line, v->storecol, num );

    if (num)
        {
        if (TST_TAB2SPACE)
            {
            for (; start < ptr;)
                {
                UBYTE ch;
                if ((ch = *start++) == '\t') { cols = v->tablen - (v->storecol % v->tablen); v->storecol += cols; for (i=0; i < cols; i++) *dest++ = ' '; }
                else { v->storecol++; *dest++ = ch; }
                }
            }
        else CopyMem( start, dest, num );
        }

    //------ Si les caractères se terminent par un saut de ligne, *ptr == '\n'
    if (*ptr == '\n' && (v->flags & ARPF_STRIPEOL)) func_LineTruncateSpace( v->line );

  END_ERROR:
    return( errmsg );
}

static ULONG store_std_line_rect( UBYTE *start, UBYTE *ptr )
{
  struct AZTextRP *v = &AGP.FObj->TRP;
  struct UIKList *li = &AGP.FObj->Text;
  struct UIKNode *node;
  ULONG i, valids, instart, inslen, errmsg=0;
  UBYTE *dest;

    instart = v->storecol;
    inslen = v->col - instart;

    if (v->flags & ARPF_CONTINUE)
        {
        node = &li->UNode[v->line];
        }
    else{
        if (v->line + 1 >= li->NodeNum)
            {
            if (v->line + 1 >= v->maxlines) return( TEXT_MaxLinesReached );
            if (! UIK_nl_AllocNode( li, -1, 0, 0 )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            valids = 0;
            }
        v->line++;
        v->totlines++;
        node = &li->UNode[v->line];
        }
    v->flags &= ~ARPF_CONTINUE;

    if (*start == '\n') goto END_ERROR; // laisser ici car il faut quand même incrémenter la ligne et insérer une nouvelle

    valids = intAZ_LineBufLen( node->Buf );

    if (instart < valids) // on va insérer au milieu d'un buffer
        {
        if (v->flags & ARPF_OVERLAY)
            {
            LONG dif = inslen - (valids - instart);
            if (dif > 0)
                if (! BufResizeS( &node->Buf, valids+dif )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            }
        else{
            if (! BufResizeS( &node->Buf, valids+inslen )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }

            UIK_MemCopy( node->Buf+instart, node->Buf+instart+inslen, valids-instart );
            }
        dest = node->Buf + instart;
        }
    else{
        if (! BufResizeS( &node->Buf, instart+inslen )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
        dest = node->Buf + valids;
        for (; valids < instart; valids++) *dest++ = ' ';
        }

    if (! (v->flags & ARPF_OVERLAY)) func_BMAddChars( v->line, v->storecol, inslen );

    if (TST_TAB2SPACE)
        {
        for (; start < ptr;)
            {
            UBYTE ch;
            if ((ch = *start++) == '\t') { ULONG cols = v->tablen - (v->storecol % v->tablen); v->storecol += cols; for (i=0; i < cols; i++) *dest++ = ' '; }
            else { v->storecol++; *dest++ = ch; }
            }
        }
    else CopyMem( start, dest, inslen );

    //------ Si les caractères se terminent par un saut de ligne, *ptr == '\n'
    if (*ptr == '\n' && (v->flags & ARPF_STRIPEOL)) func_LineTruncateSpace( v->line );

  END_ERROR:
    return( errmsg );
}

ULONG func_TextPut( UBYTE *src, ULONG srclen, ULONG flags, ULONG refresh )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZTextRP *v = &fo->TRP;
  struct UIKPList *l = &fo->Text;
  ULONG code, errmsg=0, linelen;
  LONG origline, origcol, split = 0;
  ULONG (*func)(), joinfailed=0;
  UBYTE *ub=0, *start, *ptr;
  UBYTE last, pad0, pad1, pad2;

    v->totlines = 0;
    origline = v->line; origcol = v->col;

    if (src && srclen)
        {
         //------ Conversion retour de ligne ?
        if (*src == '\r' && TST_LOADINGFILE && (AGP.Prefs->Flags2 & (AZP_LOADCR2LF|AZP_LOADCRLF2LF)))
            {
            *src = '\n';
            v->cr_replaced = 1;
            }

         //------ flags modifiés ?
        if (flags)
            {
            if (flags & ARPF_CONVERT)        v->flags |= ARPF_CONVERT;
            else if (flags & ARPF_NOCONVERT) v->flags &= ~ARPF_CONVERT;
            if (flags & ARPF_RECT)           v->flags |= ARPF_RECT;
            else if (flags & ARPF_NORECT)    v->flags &= ~ARPF_RECT;
            if (flags & ARPF_OVERLAY)        v->flags |= ARPF_OVERLAY;
            else if (flags & ARPF_NOOVERLAY) v->flags &= ~ARPF_OVERLAY;
            if (flags & ARPF_STRIPEOL)        v->flags |= ARPF_STRIPEOL;
            else if (flags & ARPF_NOSTRIPEOL) v->flags &= ~ARPF_STRIPEOL;
            }

        //------ converti si demandé
        if (v->flags & ARPF_CONVERT)
            {
            if (! (ub = func_C2Bin( src, srclen ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            src = ub; srclen = intAZ_LineBufLen(ub);
            }

        //------ ajoute une ligne à la fin si demandé
        if (v->line == -1 || v->line >= l->NodeNum)     // changé > en >=
            {
            v->line = l->NodeNum;
            if (l->NodeNum >= v->maxlines) { errmsg = TEXT_MaxLinesReached; goto END_ERROR; }
            if (! (ptr = UIK_nl_AllocNode( l, v->line, 0, 0 ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            linelen = 0;
            v->totlines++;
            }
        else{
            linelen = intAZ_Buf_Len( v->line, &ptr );
            }

        //------ mode rectangle ou non
        if (v->flags & (ARPF_OVERLAY | ARPF_RECT))
            {
            func = store_std_line_rect;
            }
        else{
            //------ coupe en deux si pas rectangle
            if (l->NodeNum >= v->maxlines) { errmsg = TEXT_MaxLinesReached; goto END_ERROR; }
            func = store_std_line_norect;
            if (v->col < linelen)
                {
                if ((code = func_intSplitLine( v->line, v->col, 1, /*(*src=='\n')?1:*/0 )) == 0)
                    { split = 1; linelen = intAZ_LineLen( v->line ); }
                else{ if ((code) != -1) { errmsg = code; goto END_ERROR; } }
                }
            }

        //------ rempli le début de ligne
        if (linelen < v->col && *src != '\n')
            {
            if (! (ptr = BufResizeS( &l->UNode[v->line].Buf, v->col ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            for (; linelen < v->col; linelen++) ptr[linelen] = ' ';
            }

        v->flags |= ARPF_CONTINUE;
        v->storecol = v->col;
        ptr = start = src;
        last = start[srclen-1];
        while (srclen)
            {
            //------ Lecture d'une ligne
            while (srclen)
                    {
                    //------ Conversion CR > LF
                    if (*ptr == '\r' && TST_LOADINGFILE && (AGP.Prefs->Flags2 & (AZP_LOADCR2LF|AZP_LOADCRLF2LF)))
                        {
                        *ptr = '\n';
                        v->cr_replaced = 1;
                        }

                    //------ Tabs
                    if (*ptr == '\t')
                        {
                        if (TST_TAB2SPACE) v->col += (v->tablen - (v->col % v->tablen)); else v->col++;
                        srclen--;
                        ptr++;
                        v->cr_replaced = 0;
                        }
                    //------ NewLine
                    else if (*ptr == '\n')
                        {
                        if (v->cr_replaced == 2)
                            {
                            srclen--;
                            ptr++;
                            v->cr_replaced = 0;
                            }
                        else{
                            UBYTE *end = ptr;
                            if (v->cr_replaced == 1)
                                {
                                v->cr_replaced++;
                                if (srclen > 1 && *(ptr+1) == '\n')
                                    {
                                    srclen--;
                                    ptr++;
                                    v->cr_replaced = 0;
                                    }
                                }
                            if (l->NodeNum >= 0x7ffffff0) { errmsg = TEXT_MaxLinesReached; goto END_ERROR; }
                            if (errmsg = func( start, end )) goto END_ERROR;
                            if (l->NodeNum >= v->maxlines) { errmsg = TEXT_MaxLinesReached; goto END_ERROR; }
                            //if (v->totlines >= v->maxlines) { errmsg = TEXT_MaxLinesReached; goto END_ERROR; }
                            v->col = v->storecol = v->indcol;
                            srclen--;
                            start = ++ptr;
                            break;
                            }
                        }
                    //------ Char
                    else{
                        /*
                        if (/*!(v->flags & (ARPF_OVERLAY | ARPF_RECT)) &&*/ TST_USEMARGIN && (v->col >= fo->RightMargin))
                            {
                            UBYTE *streol = "\n";
                            if (fo->Text.NodeNum >= 0x7ffffff0) { errmsg = TEXT_MaxLinesReached; goto END_ERROR; }
                            if (errmsg = func( start, ptr )) goto END_ERROR;
                            if (!(v->flags & (ARPF_OVERLAY | ARPF_RECT))) AGP.FObj->Text.UNode[v->line].Flags |= UNF_CONTINUE;
                            v->flags |= ARPF_CONTINUE;
                            if (errmsg = func( streol, streol )) goto END_ERROR;
                            if (l->NodeNum >= v->maxlines) { errmsg = TEXT_MaxLinesReached; goto END_ERROR; }
                            //if (v->totlines >= v->maxlines) { errmsg = TEXT_MaxLinesReached; goto END_ERROR; }
                            v->col = v->storecol = v->indcol;
                            start = ptr;
                            break;
                            }
                        else
                        */
                            {
                            v->col++;
                            srclen--;
                            ptr++;
                            v->cr_replaced = 0;
                            }
                        }
                    }

            if (srclen == 0 && v->col > v->storecol)
                    {
                    if (errmsg = func( start, ptr )) goto END_ERROR;
                    }
            }

        if (last == '\n')
            {
            if (v->flags & (ARPF_OVERLAY | ARPF_RECT))
                {
                func( &last, &last );
                }
            else if ((split || fo->Text.NodeNum < v->maxlines) && (errmsg == 0)) // TEXT_MaxLinesReached
                {
                ULONG i, col;   // garder pour incrémentation et remplissage
                UBYTE *ptr;
                col = (split) ? v->col : 0;
                if (ptr = UIK_nl_AllocNode( &fo->Text, ++v->line, col, 0 ))
                    {
                    fo->Text.UNode[v->line].Reserved = fo->Text.UNode[v->line-1].Reserved; // reste dans le fold
                    func_BMAddLines( v->line, 1 );
                    for (i=0; i < col; i++) *ptr++ = ' ';
                    v->totlines++;
                    }
                }
            }

        //------ remet en un si coupé en deux
        if (split) { joinfailed = func_intJoinLine( v->line, 0, 1, 0, 0 ); }

        SET_MODIFIED( v->line, v->col );
        }

  END_ERROR:
    if (joinfailed && v->totlines) v->totlines++;
    v->DC.Line = fo->WorkLine = v->line;
    v->DC.Col = fo->WorkCol = v->col;
    v->DC.DisplayNum += v->totlines;
    if (! (v->flags & (ARPF_OVERLAY | ARPF_RECT))) v->DC.ScrollNum  -= v->totlines;
    if (refresh)
        {
        if (! (v->flags & (ARPF_OVERLAY | ARPF_RECT))) v->DC.DisplayNum += 1;
        else if (last != '\n') v->DC.DisplayNum += 1;
        func_DisplayCmd( &v->DC );

        v->DC.DisplayNum  = 0;
        v->DC.DisplayFrom = v->line;
        v->DC.ScrollNum   = 0;
        v->DC.ScrollFrom  = v->line + 1;
        }
    if (flags & ARPF_NOCURS)
        {
        v->line = v->DC.Line = fo->WorkLine = origline;
        v->col = v->DC.Col = fo->WorkCol = origcol;
        }
    if (ub) BufFree( ub );
    return( errmsg );
}

ULONG func_Buf2Text( void )
{
    return( TEXT_IllegalFunc );
}
