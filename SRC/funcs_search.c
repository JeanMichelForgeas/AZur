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

static ULONG lfverif_found( struct SearchReplace *sr )
{
  UBYTE *ptr;
  LONG line, col, len;
  UBYTE ch;

    if (sr->Flags & SEF_RECT)
        {
        if ((sr->Col < sr->StartCol) || (sr->ECol > sr->EndCol)) return(0);
        }
    else{
        if ((sr->Line == sr->StartLine && sr->Col < sr->StartCol) ||
            (sr->ELine == sr->EndLine && sr->ECol > sr->EndCol)) return(0);
        }

    if (sr->Flags & SEF_ENDOFLINE)
        {
        line = sr->ELine; col = sr->ECol;
        len = intAZ_Buf_Len( line, &ptr );
        if (func_CharNext( ptr, &len, &line, &col, &ch ) != (APTR)-1) // si -1, ok, car à la fin de la dernière ligne
            if (ch != '\n') return(0);
        }

    if (sr->Flags & SEF_WORD)
        {
        if (sr->Flags & SEF_STARTOFWORD)
            {
            line = sr->Line; col = sr->Col;
            len = intAZ_Buf_Len( line, &ptr );
            if (! func_CharIsWord( ptr[col] )) return(0);
            if (func_CharPrev( ptr, &len, &line, &col, &ch ) != (APTR)-1)
                if (func_CharIsWord( ch )) return(0);
            }

        if (sr->Flags & SEF_ENDOFWORD)
            {
            line = sr->ELine; col = sr->ECol;
            len = intAZ_Buf_Len( line, &ptr );
            if (! func_CharIsWord( ptr[col] )) return(0);
            if (func_CharNext( ptr, &len, &line, &col, &ch ) != (APTR)-1)
                if (func_CharIsWord( ch )) return(0);
            }
        }

    return(1);
}

static ULONG te_LFStringEqual( struct SearchReplace *sr, UBYTE *ptr )
{
  UBYTE c1, c2;
  D_S(ULONG,aligne)
  ULONG i=0, len;
  UBYTE *p1=sr->SStr;

    len = intAZ_LineBufLen(ptr);
    sr->ELine = sr->Line;
    sr->ECol = sr->Col;
    c2 = intAZ_CharCurrent( sr->Col, len, ptr );

    while (TRUE)
        {
        c1 = *(p1++);
        if (c1 != '?' || !(sr->Flags & SEF_PATTERN))
            {
            if (! (sr->Flags & SEF_ACCENT))
                {
                if (c1 & 0x80) c1 = func_CharStripAccent( c1, 0 );
                if (c2 & 0x80) c2 = func_CharStripAccent( c2, 0 );
                }
            if (c1 != c2)
                {
                if (sr->Flags & SEF_CASE) return(0); /* not equal */
                if (UIK_ToUpper( c1 ) != UIK_ToUpper( c2 )) return(0); /* not equal */
                }
            }
        if (++i >= sr->SLen) return(1); /* equal (n'incrémente pas ELine et ECol) */
        if ((ptr = func_CharNext( ptr, &len, &sr->ELine, &sr->ECol, &c2 )) == (APTR)-1) return(0); // fin du texte atteinte
        if (sr->Flags & SEF_RECT)
            if ((sr->ECol < sr->StartCol) || (sr->ECol > sr->EndCol)) return(0);
        }
}

static ULONG Search_LFNext( void )
{
  UBYTE ch;
  D_S(ULONG,aligne)
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  UBYTE *ptr;
  ULONG found=0, len;

    len = intAZ_Buf_Len( sr->Line, &ptr );
    ch = intAZ_CharCurrent( sr->Col, len, ptr );
    while (TRUE)
        {
        if (!(sr->Flags & SEF_STARTOFLINE) || (sr->Col == 0))
            {
            if (! ((sr->Flags & SEF_RECT) && ((sr->Col < sr->StartCol) || (sr->Col > sr->EndCol))))
                if (te_LFStringEqual( sr, ptr )) { if (found = lfverif_found( sr )) break; }
            }
        if ((ptr = func_CharNext( ptr, &len, &sr->Line, &sr->Col, &ch )) == (APTR)-1) break;
        if (sr->Line > sr->EndLine) break;
        }
    return( found );
}

static ULONG Search_LFPrev( void )
{
  UBYTE ch;
  D_S(ULONG,aligne)
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  UBYTE *ptr;
  ULONG found=0, len;

    len = intAZ_Buf_Len( sr->Line, &ptr );
    ch = intAZ_CharCurrent( sr->Col, len, ptr );
    while (TRUE)
        {
        if (!(sr->Flags & SEF_STARTOFLINE) || (sr->Col == 0))
            {
            if (! ((sr->Flags & SEF_RECT) && ((sr->Col < sr->StartCol) || (sr->Col > sr->EndCol))))
                if (te_LFStringEqual( sr, ptr )) { if (found = lfverif_found( sr )) break; }
            }
        if ((ptr = func_CharPrev( ptr, &len, &sr->Line, &sr->Col, &ch )) == (APTR)-1) break;
        if (sr->Line < sr->StartLine) break;
        }
    return( found );
}

//-------------------------------------------------------------------------

static ULONG verif_found( struct SearchReplace *sr )
{
    sr->ELine = sr->Line; sr->ECol = sr->Col + sr->SLen - 1;

    if (sr->Flags & SEF_RECT)
        {
        if (sr->Col < sr->StartCol || sr->ECol > sr->EndCol) return(0);
        }

    if (sr->Flags & SEF_WORD)
        {
        UBYTE ch, *ptr;
        LONG line, col, len;

        if (sr->Flags & SEF_STARTOFWORD)
            {
            line = sr->Line; col = sr->Col;
            len = intAZ_Buf_Len( line, &ptr );
            if (! func_CharIsWord( ptr[col] )) return(0);
            if (func_CharPrev( ptr, &len, &line, &col, &ch ) != (APTR)-1)
                if (func_CharIsWord( ch )) return(0);
            }

        if (sr->Flags & SEF_ENDOFWORD)
            {
            line = sr->ELine; col = sr->ECol;
            len = intAZ_Buf_Len( line, &ptr );
            if (! func_CharIsWord( ptr[col] )) return(0);
            if (func_CharNext( ptr, &len, &line, &col, &ch ) != (APTR)-1)
                if (func_CharIsWord( ch )) return(0);
            }
        }

    return(1);
}

static ULONG te_StringEqual( struct SearchReplace *sr, UBYTE *ptr )
{
  register ULONG i;
  register UBYTE c1, c2, *p1=sr->SStr, *p2=ptr+sr->Col;

    for (i=0; i < sr->SLen; i++)
        {
        c1 = *(p1++); c2 = *(p2++);
        if (c1 != '?' || !(sr->Flags & SEF_PATTERN))
            {
            if (! (sr->Flags & SEF_ACCENT))
                {
                if (c1 & 0x80) c1 = func_CharStripAccent( c1, 0 );
                if (c2 & 0x80) c2 = func_CharStripAccent( c2, 0 );
                }
            if (c1 != c2)
                {
                if (sr->Flags & SEF_CASE) return(0); /* not equal */
                if (UIK_ToUpper( c1 ) != UIK_ToUpper( c2 )) return(0); /* not equal */
                }
            }
        }
    return(1); /* equal */
}

static ULONG Search_NoLFNext( void )
{
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  UBYTE *ptr;
  struct UIKNode *node = &fo->Text.UNode[sr->Line]; // !!!! car AllocNode réalloue la table des nodes !!!!
  ULONG found=0, llen;

    for (; ; node++)
        {
        ptr = node->Buf; llen = intAZ_LineBufLen(ptr);
        if (sr->Line == sr->EndLine) llen = sr->EndCol;

        if (sr->Col + sr->SLen <= llen + 1)
            {
            if (sr->Flags & (SEF_STARTOFLINE|SEF_ENDOFLINE))
                {
                if ((sr->Flags & (SEF_STARTOFLINE|SEF_ENDOFLINE)) == (SEF_STARTOFLINE|SEF_ENDOFLINE))
                    {
                    if (sr->Col == 0 && sr->SLen == intAZ_LineBufLen(ptr))
                        {
                        if (te_StringEqual( sr, ptr )) found = verif_found( sr );
                        }
                    }
                else{
                    if ((sr->Flags & SEF_STARTOFLINE) && sr->Col == 0)
                        {
                        if (te_StringEqual( sr, ptr )) found = verif_found( sr );
                        }
                    else if ((sr->Flags & SEF_ENDOFLINE) && llen == intAZ_LineBufLen(ptr)) // llen peut être < BUFLEN si endcol
                        {
                        if (sr->Col < llen - sr->SLen) sr->Col = llen - sr->SLen; // ne pas rester au même ebdroit
                        if (te_StringEqual( sr, ptr )) found = verif_found( sr );
                        }
                    }
                }
            else while (sr->Col + sr->SLen <= llen + 1)
                {
                if (te_StringEqual( sr, ptr )) { if (found = verif_found( sr )) break; }
                ++sr->Col;
                }

            if (found) return(1);
            }
        if (++sr->Line > sr->EndLine) break;
        sr->Col = 0;
        }
    return(0);
}

static ULONG Search_NoLFPrev( void )
{
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  UBYTE *ptr;
  struct UIKNode *node = &fo->Text.UNode[sr->Line]; // !!!! car AllocNode réalloue la table des nodes !!!!
  ULONG found=0, llen;
  LONG ldeb;

    ptr = node->Buf; ldeb = 0;
    llen = sr->EndCol+1; if (llen > intAZ_LineBufLen(ptr)) llen = intAZ_LineBufLen(ptr);// position de départ
    for (; ;)
        {
        if (sr->Col > llen - sr->SLen) sr->Col = llen - sr->SLen;  // pour ne pas comparer par-dessus la fin
        if (sr->Line == sr->StartLine) ldeb = sr->StartCol;

        if (sr->Col >= ldeb)   // 0 ou startcol : si la ligne est assez grande
            {
            if (sr->Flags & (SEF_STARTOFLINE|SEF_ENDOFLINE))
                {
                if ((sr->Flags & (SEF_STARTOFLINE|SEF_ENDOFLINE)) == (SEF_STARTOFLINE|SEF_ENDOFLINE))
                    {
                    if (sr->Col == 0 && sr->SLen == intAZ_LineBufLen(ptr))
                        {
                        if (te_StringEqual( sr, ptr )) found = verif_found( sr );
                        }
                    }
                else{                               // vérifs pour startcol et endcol
                    if ((sr->Flags & SEF_ENDOFLINE) && llen == intAZ_LineBufLen(ptr) && (sr->Col == llen - sr->SLen))
                        {
                        if (te_StringEqual( sr, ptr )) found = verif_found( sr );
                        }
                    else if ((sr->Flags & SEF_STARTOFLINE) && ldeb == 0) // pas pour startline si startcol > 0
                        {
                        sr->Col = 0;
                        if (te_StringEqual( sr, ptr )) found = verif_found( sr );
                        }
                    }
                }
            else while (sr->Col >= ldeb)
                {
                if (te_StringEqual( sr, ptr )) { if (found = verif_found( sr )) break; }
                --sr->Col;
                }

            if (found) return(1);
            }
        if (--sr->Line < sr->StartLine) break;
        node--;
        ptr = node->Buf; llen = intAZ_LineBufLen(ptr);
        sr->Col = llen;
        }
    return(0);
}

//--------------------------------------------------------------

static ULONG SearchInto( struct SearchRepQuery *q, ULONG *pfirstline, ULONG *plastline, LONG *pcursline, LONG *pcurscol )
{
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  ULONG len, errmsg=0, found;
  UBYTE *ptr;

    /*** On peut donner ( 0, 0, -1, -1, ... ) pour tout le texte
     *** ou ( fo->Line, fo->Col, -1, -1, ... ) pour depuis le curseur
     ***/

    if (sr->Flags & SEF_BACKWARD)
        {
        if (! (sr->Flags & SEF_BLOCK))
            {
            if (sr->Flags & SEF_FULL)
                {
                q->StartLine = q->StartCol = 0;
                q->EndLine = fo->Text.NodeNum - 1;
                q->EndCol = intAZ_LineLen(q->EndLine);
                sr->Line = q->EndLine;
                sr->Col = q->EndCol;
                }
            else{
                if (q->StartLine == -1) q->StartLine = 0;
                if (q->StartCol == -1) q->StartCol = 0;
                if (fo->Line == fo->FoundLine && fo->Col == fo->FoundCol)
                    {
                    if (q->EndLine == -1) q->EndLine = fo->FoundELine;
                    if (q->EndCol == -1) q->EndCol = fo->FoundECol;
                    sr->Line = fo->FoundLine;
                    sr->Col = fo->FoundCol;
                    }
                else{
                    if (q->EndLine == -1) q->EndLine = fo->Line;
                    if (q->EndCol == -1) q->EndCol = fo->Col;
                    len = intAZ_Buf_Len( q->EndLine, &ptr );
                    if (q->EndCol > len) q->EndCol = len;
                    sr->Line = q->EndLine;
                    sr->Col = q->EndCol;
                    }
                }
            }
        }
    else{
        if (! (sr->Flags & SEF_BLOCK))
            {
            if (sr->Flags & SEF_FULL) { q->StartLine = q->StartCol = 0; q->EndLine = q->EndCol = -1; }
            else{
                if (q->StartLine == -1) q->StartLine = fo->Line;
                if (q->StartCol == -1) q->StartCol = fo->Col;
                }
            if (q->EndLine == -1 || q->EndLine > fo->Text.NodeNum - 1) q->EndLine = fo->Text.NodeNum - 1;
            len = intAZ_Buf_Len( q->EndLine, &ptr );
            if (q->EndCol == -1 || q->EndCol > len) q->EndCol = len;
            }
        sr->Line = q->StartLine;
        sr->Col = q->StartCol;
        }
    sr->StartLine = q->StartLine;
    sr->StartCol = q->StartCol;
    sr->EndLine = q->EndLine;
    sr->EndCol = q->EndCol;

    //-----------------------------------

    while (TRUE)
        {
        if (sr->Flags & SEF_BACKWARD)
            {
            if (sr->Line == fo->FoundLine && sr->Col == fo->FoundCol)
                {
                if (func_PosPrev( &sr->Line, &sr->Col ) == (APTR)-1) break; // évite de rester sur le même
                if (sr->EndLine == fo->FoundLine && sr->EndCol == fo->FoundCol)
                    { sr->EndLine = sr->Line; sr->EndCol = sr->Col; } // si on change de ligne
                }
            if (sr->Flags & (SEF_DOLF|SEF_DOPATTERN)) found = Search_LFPrev();
            else found = Search_NoLFPrev();
            }
        else{
            if (sr->Line == fo->FoundLine && sr->Col == fo->FoundCol)
                {
                sr->Line = fo->FoundELine; sr->Col = fo->FoundECol; // se place à la fin de la zone trouvée
                if (func_PosNext( &sr->Line, &sr->Col ) == (APTR)-1) break;   // évite de rester sur le même
                }

            if (sr->Flags & (SEF_DOLF|SEF_DOPATTERN)) found = Search_LFNext();
            else found = Search_NoLFNext();
            }

        if (found)
            {
            if (sr->Line < *pfirstline) *pfirstline = sr->Line;
            if (sr->Line > *plastline) *plastline = sr->Line;
            if (sr->ELine < *pfirstline) *pfirstline = sr->ELine;
            if (sr->ELine > *plastline) *plastline = sr->ELine;

            if (sr->Flags & SEF_TOBLOCK)
                {
                if (func_BlockStart( sr->Line, sr->Col, 0/*pointer*/ ))
                    func_BlockDefine( sr->ELine, sr->ECol, BLKMODE_CHAR, 0/*pointer*/ );
                }

            q->FoundCount++;
            fo->FoundLine = sr->Line; fo->FoundCol = sr->Col;
            fo->FoundELine = sr->ELine; fo->FoundECol = sr->ECol;

            if (sr->Flags & SEF_BACKWARD)
                {
                if (sr->Line < *pcursline || (sr->Line == *pcursline && sr->Col < *pcurscol)) { *pcursline = sr->Line; *pcurscol = sr->Col; }
                }
            else{
                if (sr->Line > *pcursline || (sr->Line == *pcursline && sr->Col > *pcurscol)) { *pcursline = sr->Line; *pcurscol = sr->Col; }
                }
            }

        if (!(sr->Flags & SEF_ALL) || !found) break;
        }

    //-----------------------------------

    return( errmsg );
}

ULONG func_TextSearch( struct SearchRepQuery *q )
{
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  ULONG i, errmsg=0;
  UBYTE *ubuf=0;
  LONG firstline=0x7fffffff, lastline=0, cursline, curscol;

    q->FoundCount = 0;

    /*** Le CONVERT éventuel à été déjà fait dans l'objet de commande si q->SStr == -1.
     *** Sinon on doit le faire ici et mettre un flag pour désallouer.
     ***/

    //------ Si flags -1 on prend les défauts
    if (q->SearchFlags == -1)
        {
        sr->Flags = sr->SearchFlags;
        }
    else{
        sr->Flags = q->SearchFlags;
        }
    cursline = (sr->Flags & SEF_BACKWARD) ? 0x7fffffff : -1;

    //------ Si q->SStr -1 on prend les défauts
    if (q->SStr == (APTR)-1)
        {
        sr->SStr = (sr->Flags & SEF_CONVERT) ? sr->ConvSearchStr : sr->SearchStr;
        sr->SLen = intAZ_LineBufLen(sr->SStr);
        if (! sr->SLen) { errmsg = TEXT_ERR_Nothing2Search; goto END_ERROR; }
        }
    else{
        //------ On prend la chaine passée en argument
        if (!q->SLen || !q->SStr) { errmsg = TEXT_ERR_Nothing2Search; goto END_ERROR; }
        if (sr->Flags & SEF_CONVERT)
            {
            if (! (sr->SStr = ubuf = func_C2Bin( q->SStr, q->SLen ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            sr->SLen = intAZ_LineBufLen(sr->SStr);
            if (! sr->SLen) { errmsg = TEXT_ERR_Nothing2Search; goto END_ERROR; }
            }
        else{
            sr->SStr = q->SStr;
            sr->SLen = q->SLen;
            }
        }

    //------ WORD ?
    if (sr->Flags & SEF_WORD)
        {
        if (sr->Flags & SEF_STARTOFWORD)
            if (!func_CharIsWord( sr->SStr[0] ) && !(sr->SStr[0] == '?' && (sr->Flags & SEF_PATTERN))) { errmsg = TEXT_ERR_FirstCharNotWord; goto END_ERROR; }
        if (sr->Flags & SEF_ENDOFWORD)
            if (!func_CharIsWord( sr->SStr[sr->SLen-1] ) && !(sr->SStr[sr->SLen-1] == '?' && (sr->Flags & SEF_PATTERN))) { errmsg = TEXT_ERR_LastCharNotWord; goto END_ERROR; }
        }

    /*** Cherche les linefeed pour changer éventuellement de routine
     ***
     ***/

    sr->Flags &= ~SEF_DOLF;
    for (i=0; i < sr->SLen; i++)
        {
        if (sr->SStr[i] == '\n') { sr->Flags |= SEF_DOLF; break; }
        }

    /*** Cherche les '?' pour le remplacement de pattern
     ***
     ***/

    sr->Flags &= ~SEF_DOPATTERN;
    if (sr->Flags & SEF_PATTERN)
        {
        for (i=0; i < sr->SLen; i++)
            {
            if (sr->SStr[i] == '?') { sr->Flags |= SEF_DOPATTERN; break; }
            }
        }

    /*** Effectue la recherche
     ***
     ***/

    func_SetMainWaitPointer();

    if (sr->Flags & SEF_BLOCK)
        {
        struct AZBlock *azb;
        ULONG block, numblocks;

        if (! (numblocks = func_BlockExists())) { errmsg = TEXT_NoBlockDefined; goto END_ERROR; }

        for (block=numblocks; block > 0; block--) // on part du dernier
            {
            azb = func_BlockPtr( block );
            q->StartLine = azb->LineStart;
            q->StartCol = azb->ColStart;
            q->EndLine = azb->LineEnd;
            q->EndCol = azb->ColEnd;
            if (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) sr->Flags |= SEF_RECT;
            else sr->Flags &= ~SEF_RECT;
            func_BlockRemove( block, 0, 0 );

            if (q->StartLine < firstline) firstline = q->StartLine;
            if (q->StartLine > lastline) lastline = q->StartLine;
            if (q->EndLine < firstline) firstline = q->EndLine;
            if (q->EndLine > lastline) lastline = q->EndLine;
            if (errmsg = SearchInto( q, &firstline, &lastline, &cursline, &curscol )) break;
            }
        }
    else{
        sr->Flags &= ~SEF_RECT;

        errmsg = SearchInto( q, &firstline, &lastline, &cursline, &curscol );
        }

    //-------------------------------------------------------------------------------

    q->DC.DisplayNum = q->DC.ScrollNum = 0;
    if (firstline <= lastline)
        {
        q->DC.DisplayNum = lastline - firstline + 1;
        q->DC.DisplayFrom = firstline;
        if (q->DoDisplay)
            {
            func_DisplayCmd( &q->DC );
            q->DC.DisplayNum = 0;
            }
        }

    if (q->FoundCount)
        {
        LONG fline = fo->FoundLine, fcol = fo->FoundCol;
        fo->WorkLine = cursline; fo->WorkCol = curscol;
        if (q->DoCursor) { func_CursorPos( fo->WorkLine, fo->WorkCol ); fo->CEdCol = fo->WorkCol; }
        fo->FoundLine = fline; fo->FoundCol = fcol;  // car func_CursorPos() détruit les Found...
        }
    else{
        if (errmsg == 0) errmsg = TEXT_Search_NotFound;
        fo->FoundCol = -1;
        }

  END_ERROR:
    func_SetMainSystemPointer();
    if (ubuf) BufFree( ubuf );
    return( errmsg );
}

//---------------------------------------------------------------------------------------------

static ULONG ReplaceInto( struct SearchRepQuery *q, ULONG *pfirstline, ULONG *plastline, LONG *pcursline, LONG *pcurscol, UBYTE *patstr )
{
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  ULONG len, errmsg=0, found;
  UBYTE *ptr, *cont=0;
  struct AZDisplayCmd DC = { 0,0,0,0 };
  struct AZBM *bookmark = 0;

    /*** On peut donner ( 0, 0, -1, -1, ... ) pour tout le texte
     *** ou ( fo->Line, fo->Col, -1, -1, ... ) pour depuis le curseur
     ***/

    if (sr->Flags & SEF_BACKWARD)
        {
        if (! (sr->Flags & SEF_BLOCK))
            {
            if (sr->Flags & SEF_FULL)
                {
                q->StartLine = q->StartCol = 0;
                q->EndLine = fo->Text.NodeNum - 1;
                q->EndCol = intAZ_LineLen(q->EndLine);
                }
            else{
                if (q->StartLine == -1) q->StartLine = 0;
                if (q->StartCol == -1) q->StartCol = 0;
                if (q->EndLine == -1) q->EndLine = (fo->FoundCol != -1) ? fo->FoundELine : fo->Line;
                if (q->EndCol == -1) q->EndCol = (fo->FoundCol != -1) ? fo->FoundECol : fo->Col;
                len = intAZ_Buf_Len( q->EndLine, &ptr );
                if (q->EndCol > len) q->EndCol = len;
                }
            }
        sr->Line = q->EndLine;
        sr->Col = q->EndCol;
        }
    else{
        if (! (sr->Flags & SEF_BLOCK))
            {
            if (sr->Flags & SEF_FULL)
                {
                q->StartLine = q->StartCol = 0;
                q->EndLine = q->EndCol = -1;
                }
            else{
                if (q->StartLine == -1) q->StartLine = fo->Line;
                if (q->StartCol == -1) q->StartCol = fo->Col;
                }
            if (q->EndLine == -1 || q->EndLine > fo->Text.NodeNum - 1) q->EndLine = fo->Text.NodeNum - 1;
            len = intAZ_Buf_Len( q->EndLine, &ptr );
            if (q->EndCol == -1 || q->EndCol > len) q->EndCol = len;
            }
        sr->Line = q->StartLine;
        sr->Col = q->StartCol;
        }
    sr->StartLine = q->StartLine;
    sr->StartCol = q->StartCol;
    sr->EndLine = q->EndLine;
    sr->EndCol = q->EndCol;

    //-----------------------------------

    if ((sr->Flags & (SEF_DOLF|SEF_DOPATTERN)) && (sr->Flags & SEF_ALL))
        bookmark = (struct AZBM *) func_BMAdd( "___$$$TmpReplace", sr->EndLine, sr->EndCol );

    while (cont != (APTR)-1)
        {
        if (sr->Flags & SEF_BACKWARD)
            {
            //if (func_PosPrev( &sr->Line, &sr->Col ) == (APTR)-1) break; --> curseur déjà à -1

            if (sr->Flags & (SEF_DOLF|SEF_DOPATTERN)) found = Search_LFPrev();
            else found = Search_NoLFPrev();
            }
        else{
            if (sr->Line == fo->FoundLine && sr->Col == fo->FoundCol) // pas utilisé première fois car mis à -1, utile si ALL
                {
                sr->Line = fo->FoundELine; sr->Col = fo->FoundECol; // se place à la fin de la zone trouvée
                if (func_PosNext( &sr->Line, &sr->Col ) == (APTR)-1) break;   // évite de rester sur le même
                }

            if (sr->Flags & (SEF_DOLF|SEF_DOPATTERN)) found = Search_LFNext();
            else found = Search_NoLFNext();
            }

        if (found)
            {
            UBYTE *putptr = sr->RStr;

            //-------------------------- Traite le pattern de remplacement
            if ((sr->Flags & SEF_DOPATTERN) && sr->RLen && patstr)
                {
                ULONG sx, rx, len, snum;
                LONG line, col;
                UBYTE *ptr, ch, sc;

                line = sr->Line; col = sr->Col;
                len = intAZ_Buf_Len( line, &ptr );
                ch = intAZ_CharCurrent( col, len, ptr );
                CopyMem( sr->RStr, patstr, sr->RLen );

                for (snum=sr->SPatNum, sx=0, rx=0; snum && sx < sr->SLen && rx < sr->RLen; rx++)
                    {
                    if (patstr[rx] == '?')
                        {
                        while (sx < sr->SLen)
                            {
                            sc = sr->SStr[sx];
                            if (sc =='?') patstr[rx] = ch;

                            if ((ptr = func_CharNext( ptr, &len, &line, &col, &ch )) == (APTR)-1) goto FINI;
                            if ((line > sr->ELine) || (line == sr->ELine && col > sr->ECol)) goto FINI;
                            sx++;
                            if (sc == '?') { snum--; break; }
                            }
                        }
                    }

                for (; sx < sr->SLen && rx < sr->RLen; rx++)
                    {
                    if (patstr[rx] == '?')
                        {
                        patstr[rx] = ch;

                        if ((ptr = func_CharNext( ptr, &len, &line, &col, &ch )) == (APTR)-1) goto FINI;
                        if ((line > sr->ELine) || (line == sr->ELine && col > sr->ECol)) goto FINI;
                        sx++;
                        }
                    }

                FINI: putptr = patstr;
                }

            //-------------------------- Efface l'ancien texte
            func_TextDelRect( sr->Line, sr->Col, sr->ELine, sr->ECol, 0, &DC );
            sr->ELine = fo->WorkLine; sr->ECol = fo->WorkCol;
            if (q->DoDisplay && (sr->Flags & (SEF_DOLF|SEF_DOPATTERN))) func_DisplayCmd( &DC );

            //-------------------------- Insère le nouveau texte
            if (sr->RLen)
                {
                func_TextPrep( sr->Line, sr->Col, ARPF_NOCONVERT|ARPF_NOOVERLAY|(sr->Flags&SEF_RDOLF?ARPF_NORECT:ARPF_RECT), 0, NOINDENT );
                func_TextPut( putptr, sr->RLen, 0, 0 );
                sr->ELine = fo->WorkLine; sr->ECol = fo->WorkCol;
                if (bookmark) { sr->EndLine = bookmark->Line; sr->EndCol = bookmark->Col; }

                func_PosPrev( &sr->ELine, &sr->ECol );
                if ((sr->SearchFlags & SEF_TOBLOCK) /*&& !(sr->Flags & SEF_DOLF)*/)
                    {
                    if (func_BlockStart( sr->Line, sr->Col, 0/*pointer*/ ))
                        func_BlockDefine( sr->ELine, sr->ECol, BLKMODE_CHAR, 0/*pointer*/ );
                    }
                // refresh
                if (q->DoDisplay && (sr->Flags & (SEF_DOLF|SEF_DOPATTERN))) func_TextPut( 0, 0, 0, 1 );
                }

            if (sr->Line < *pfirstline) *pfirstline = sr->Line;
            if (sr->Line > *plastline) *plastline = sr->Line;
            if (sr->ELine < *pfirstline) *pfirstline = sr->ELine;
            if (sr->ELine > *plastline) *plastline = sr->ELine;

            q->FoundCount++;
            fo->FoundLine = sr->Line; fo->FoundCol = sr->Col;
            fo->FoundELine = sr->ELine; fo->FoundECol = sr->ECol;

            if (sr->Flags & SEF_BACKWARD)
                {
                cont = func_PosPrev( &sr->Line, &sr->Col );
                if (sr->Line < *pcursline) { *pcursline = sr->Line; *pcurscol = sr->Col; }
                }
            else{
                sr->Line = sr->ELine;
                sr->Col = sr->ECol;
                cont = func_PosNext( &sr->Line, &sr->Col );
                if (sr->Line > *pcursline) { *pcursline = sr->Line; *pcurscol = sr->Col; }
                }
            }

        if (!(sr->Flags & SEF_ALL) || !found) break;
        }

    if (bookmark) func_BMRemove( "___$$$TmpReplace", 0 );

    //-----------------------------------

    return( errmsg );
}

void verif_searchflags( struct SearchReplace *sr, ULONG repl )
{
    if ((sr->SearchFlags & (SEF_FULL|SEF_BLOCK)) == (SEF_FULL|SEF_BLOCK))
        sr->SearchFlags &= ~SEF_BLOCK;

    if (sr->SearchFlags & SEF_BLOCK)
        {
        sr->SearchFlags |= SEF_ALL;
        }

    if (repl && (sr->SearchFlags & SEF_ALL))
        {
        sr->SearchFlags &= ~SEF_BACKWARD;
        }
}

ULONG func_TextReplace( struct SearchRepQuery *q )
{
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  ULONG i, errmsg=0;
  UBYTE *ubuf=0, *patstr=0;
  LONG firstline=0x7fffffff, lastline=0, cursline, curscol;

    q->FoundCount = 0;

    /*** Le CONVERT éventuel à été déjà fait dans l'objet de commande si q->SStr == -1.
     *** Sinon on doit le faire ici et mettre un flag pour désallouer.
     ***/

    //------ Pour remplacer une chaine juste trouvée
    fo->FoundLine = -1; fo->FoundCol = -1;

    //------ Si flags -1 on prend les défauts
    if (q->SearchFlags == -1)
        {
        sr->Flags = sr->SearchFlags;
        }
    else{
        sr->Flags = q->SearchFlags;
        }
    cursline = (sr->Flags & SEF_BACKWARD) ? 0x7fffffff : -1;

    //------ Verifs
    verif_searchflags( sr, 1 );

    //------ Si q->SStr -1 on prend les défauts
    if (q->SStr == (APTR)-1)
        {
        sr->SStr = (sr->Flags & SEF_CONVERT) ? sr->ConvSearchStr : sr->SearchStr;
        sr->SLen = intAZ_LineBufLen(sr->SStr);
        if (! sr->SLen) { errmsg = TEXT_ERR_Nothing2Search; goto END_ERROR; }
        }
    else{
        //------ On prend la chaine passée en argument
        if (!q->SLen || !q->SStr) { errmsg = TEXT_ERR_Nothing2Search; goto END_ERROR; }
        if (sr->Flags & SEF_CONVERT)
            {
            if (! (sr->SStr = ubuf = func_C2Bin( q->SStr, q->SLen ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
            sr->SLen = intAZ_LineBufLen(sr->SStr);
            if (! sr->SLen) { errmsg = TEXT_ERR_Nothing2Search; goto END_ERROR; }
            }
        else{
            sr->SStr = q->SStr;
            sr->SLen = q->SLen;
            }
        }
    if (q->RStr == (APTR)-1)
        {
        sr->RStr = (sr->Flags & SEF_CONVERT) ? sr->ConvReplaceStr : sr->ReplaceStr;
        sr->RLen = intAZ_LineBufLen(sr->RStr);
        }
    else{
        //------ On prend la chaine passée en argument
        if (/*!q->RLen ||*/ !q->RStr) { q->RStr = ""; q->RLen = 0; }
        if (sr->Flags & SEF_CONVERT)
            {
            if (q->RLen)
                {
                if (! (sr->RStr = ubuf = func_C2Bin( q->RStr, q->RLen ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
                sr->RLen = intAZ_LineBufLen(sr->RStr);
                }
            else { sr->RStr = ""; sr->RLen = 0; }
            }
        else{
            sr->RStr = q->RStr;
            sr->RLen = q->RLen;
            }
        }

    //------ WORD ?
    if (sr->Flags & SEF_WORD)
        {
        if (sr->Flags & SEF_STARTOFWORD)
            if (!func_CharIsWord( sr->SStr[0] ) && !(sr->SStr[0] == '?' && (sr->Flags & SEF_PATTERN))) { errmsg = TEXT_ERR_FirstCharNotWord; goto END_ERROR; }
        if (sr->Flags & SEF_ENDOFWORD)
            if (!func_CharIsWord( sr->SStr[sr->SLen-1] ) && !(sr->SStr[sr->SLen-1] == '?' && (sr->Flags & SEF_PATTERN))) { errmsg = TEXT_ERR_LastCharNotWord; goto END_ERROR; }
        }

    /*** Cherche les linefeed pour changer éventuellement de routine
     ***
     ***/

    sr->Flags &= ~(SEF_DOLF|SEF_RDOLF);
    for (i=0; i < sr->SLen; i++)
        {
        if (sr->SStr[i] == '\n') { sr->Flags |= SEF_DOLF; break; }
        }
    for (i=0; i < sr->RLen; i++)
        {
        if (sr->RStr[i] == '\n') { sr->Flags |= (SEF_DOLF|SEF_RDOLF); break; }
        }

    /*** Cherche les '?' pour le remplacement de pattern
     ***
     ***/

    if (sr->Flags & SEF_PATTERN)
        {
        sr->Flags &= ~SEF_DOPATTERN;
        for (sr->SPatNum=i=0; i < sr->SLen; i++) { if (sr->SStr[i] == '?') sr->SPatNum++; }
        for (sr->RPatNum=i=0; i < sr->RLen; i++) { if (sr->RStr[i] == '?') sr->RPatNum++; }
        if (sr->RPatNum)
            {
            sr->Flags |= SEF_DOPATTERN;
            patstr = PoolAlloc( sr->RLen );
            }
        }

    /*** Effectue la recherche
     ***
     ***/

    func_SetMainWaitPointer();

    if (sr->Flags & SEF_BLOCK)
        {
        struct AZBlock *azb;
        ULONG block, numblocks;

        if (! (numblocks = func_BlockExists())) { errmsg = TEXT_NoBlockDefined; goto END_ERROR; }

        for (block=numblocks; block > 0; block--) // on part du haut
            {
            azb = func_BlockPtr( block );
            q->StartLine = azb->LineStart;
            q->StartCol = azb->ColStart;
            q->EndLine = azb->LineEnd;
            q->EndCol = azb->ColEnd;
            if (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) sr->Flags |= SEF_RECT;
            else sr->Flags &= ~SEF_RECT;
            func_BlockRemove( block, 0, 0 );

            if (q->StartLine < firstline) firstline = q->StartLine;
            if (q->StartLine > lastline) lastline = q->StartLine;
            if (q->EndLine < firstline) firstline = q->EndLine;
            if (q->EndLine > lastline) lastline = q->EndLine;
            if (errmsg = ReplaceInto( q, &firstline, &lastline, &cursline, &curscol, patstr )) break;
            }
        }
    else{
        sr->Flags &= ~SEF_RECT;

        errmsg = ReplaceInto( q, &firstline, &lastline, &cursline, &curscol, patstr );
        }

    //-------------------------------------------------------------------------------

    q->DC.DisplayNum = q->DC.ScrollNum = 0;
    if (firstline <= lastline)
        {
        q->DC.DisplayNum = lastline - firstline + 1;
        q->DC.DisplayFrom = firstline;
        if (q->DoDisplay && !(sr->Flags & (SEF_DOLF|SEF_DOPATTERN))) // --> si DOLF refresh fait au fur et à mesure
            {
            func_DisplayCmd( &q->DC );
            q->DC.DisplayNum = 0;
            }
        }

    if (q->FoundCount)
        {
        LONG fline = fo->FoundLine, fcol = fo->FoundCol;
        fo->WorkLine = cursline; fo->WorkCol = curscol;
        if (q->DoCursor) { func_CursorPos( fo->WorkLine, fo->WorkCol ); fo->CEdCol = fo->WorkCol; }
        fo->FoundLine = fline; fo->FoundCol = fcol;  // car func_CursorPos() détruit les Found...
        }
    else{
        if (errmsg == 0) errmsg = TEXT_Search_NotFound;
        fo->FoundCol = -1;
        }

  END_ERROR:
    func_SetMainSystemPointer();
    if (patstr) PoolFree( patstr );
    if (ubuf) BufFree( ubuf );
    return( errmsg );
}
