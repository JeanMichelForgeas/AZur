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

#include "uiki:objwinfilesel.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"

#define DBUG(a)


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/


/****** Statics *************************************************/


/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

//»»»»»» Cmd_Del            NUM/N,CHARS/S,LINES/S,WORDS/S,PREV=BACK=PREVIOUS/S,BOUND/S,LINE/N,COL=COLUMN/N,PAGE/S,REL=RELATIVE/S,WORD/K/N,TAB/K/N,OF=OFFSET/N,UP/S,LEFT/S,W=WRAP/S,ENDWORD/S,NC=NOCURS/S,NOCLIP/S

void CmdFunc_Del_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG num, line, col, chars=0, lines=0, words=0;
  LONG startline = fo->Line, startcol = fo->Col;

    if (! co->ArgRes[18]) func_Block2CB( AGP.Prefs->DelClip ); // NOCLIP
    if (func_BlockEraseAll()) return;

    //------ ligne et colonne de départ
    if (! func_ComputeLineCol( (LONG*) &co->ArgRes[6], &line, &col ))
        { intAZ_SetCmdResult( co, TEXT_BadPosition ); return; }

    //------ Nombre de caractères/mots ou de lignes
    num = co->ArgRes[0] ? *(LONG*)co->ArgRes[0] : 1;

    if (co->ArgRes[2])                      // LINES
        {
        if (co->ArgRes[5])                  // BOUND
            {
            lines = (co->ArgRes[4] ? 0x80000000 : 0x7fffffff);
            }
        else{
            if (! (lines = num)) return;
            //------ ajustement si en arrière
            if (co->ArgRes[4] && lines > 0) lines = -lines; // PREV
            }
        }

    else if (co->ArgRes[3])                 // WORDS
        {
        if (co->ArgRes[5]) goto CHARBOUNDS;

        if (! (words = num)) return;

        //------ ajustement si en arrière
        if (co->ArgRes[4] && words > 0) words = -words;

        //------ transformation mots ==> caractères
        chars = func_CountWords( line, col, words, 0, 0, 0 );
        }

    else /*if (co->ArgRes[1])*/             // CHARS
        {
        CHARBOUNDS:
        if (co->ArgRes[5])                  // BOUND
            {
            chars = (co->ArgRes[4] ? -col : intAZ_LineLen(line) - col);
            }
        else{
            if (! (chars = num)) ; //return; // defaut à 1, ne pas faire return car il faut placer le curseur
            //------ ajustement si en arrière
            if (co->ArgRes[4] && (chars > 0)) chars = -chars; // PREV
            }
        }

    //------ efface tout ça
    if (chars || lines)
        {
        struct AZDisplayCmd dispinfo;

        func_CursorErase();
        if (((ABS(chars) > 1) || lines) && !co->ArgRes[18]) // NOCLIP
            func_Text2CB( line, col, chars, lines );
        if (func_TextDel( line, col, chars, lines, &dispinfo ))
            {
            if (AGP.FObj->Text.NodeNum == 0) UIK_nl_AllocNode( &AGP.FObj->Text, -1, 0, 0 );
            func_DisplayCmd( &dispinfo );
            }
        }
    else{
        fo->WorkLine = line;
        fo->WorkCol = col;
        }

    if (co->ArgRes[17]) func_CursorDraw();
    else { func_CursorPos( fo->WorkLine, fo->WorkCol ); fo->CEdCol = fo->WorkCol; }

    if (TST_USEMARGIN && TST_AUTOFORMAT)
        {
        if (chars < 0 && startline == fo->Line && fo->Col < fo->LeftMargin && fo->Line > 0)
            {
            if ((fo->Line > 0) && (fo->Text.UNode[fo->Line-1].Flags & UNF_CONTINUE)) // ligne précédente terminée par un LF ?
                func_CursorPos( fo->Line-1, startcol=intAZ_LineLen(fo->Line-1) ); fo->CEdCol = startcol;
            }
        func_AutoFormat();
        }
}

//»»»»»» Cmd_Text2Win       LINE/K/N,COL=COLUMN/K/N,PAGE/S,REL=RELATIVE/S,WORD/N,TAB/N,OF=OFFSET/N,UP/S,LEFT/S,W=WRAP/S,ENDWORD/S,IP=INDENTPREV/S,I=INDENT/S,IN=INDENTNEXT/S,PREFS/S,NC=NOCURS/S,RECT=VERTICAL/S,REPL=OVERLAY/S,CC=CONVERT/S,NF=NOFORMAT/S,TEXT/F

UBYTE commun_text_2( struct CmdObj *co, UBYTE *ptr, ULONG len, APTR nocurs, ULONG flags, ULONG noformat )
{
  struct AZObjFile *fo = AGP.FObj;
  UBYTE code=0;

    if (len)
        {
        func_CursorErase();
        if (nocurs) flags |= ARPF_NOCURS; else flags &= ~ARPF_NOCURS;
        if (ptr) code = func_TextPut( ptr, len, flags, 1 );
        if (nocurs == 0)
            {
            func_CursorPos( fo->WorkLine, fo->WorkCol );
            // func_BlockStart( fo->WorkLine, fo->WorkCol, 0 );
            fo->CEdCol = fo->WorkCol;
            }
        else func_CursorDraw();
        if (code == 0 && noformat == 0) func_AutoFormat();
        if (code && co) intAZ_SetCmdResult( co, code ); // co peut être à 0 si appelée depuis RawKeys()
        }
    return( code );
}

void CmdFunc_Text2Win_Do( struct CmdObj *co )
{
  ULONG line; // = co->ArgRes[0] ? *(ULONG*)co->ArgRes[0] - 1 : AGP.FObj->Line;
  ULONG col;  // = co->ArgRes[1] ? *(ULONG*)co->ArgRes[1] - 1 : AGP.FObj->Col;
  ULONG code, indtype, flags;
  UBYTE *ptr;

    //------ nettoie (à faire au début car peut changer la position courante)
    if (TST_CHARDELBLOCK)
        {
        func_Block2CB( AGP.Prefs->DelClip );
        func_BlockEraseAll();
        }
    else{
        func_BlockClearAll();
        }

    //------ ligne et colonne de départ
    if (! func_ComputeLineCol( (LONG*) &co->ArgRes[0], &line, &col ))
        { intAZ_SetCmdResult( co, TEXT_BadPosition ); return; }

    //------ indentation
    if (co->ArgRes[11]) indtype = INDENT_LEFT;
    else if (co->ArgRes[12]) indtype = INDENT_STAY;
    else if (co->ArgRes[13]) indtype = INDENT_RIGHT;
    else if (co->ArgRes[14]) indtype = AGP.Prefs->IndentType;
    else indtype = 0;

    //------ flags
    flags = 0;
    if (co->ArgRes[16]) flags |= ARPF_RECT;         // RECT=VERTICAL
    if (co->ArgRes[17]) flags |= ARPF_OVERLAY;      // REPL=OVERLAY
    if (co->ArgRes[18]) flags |= ARPF_CONVERT;      // CC=CONVERT

    if (! (ptr = co->ArgRes[20])) ptr = "";

    if (code = func_TextPrep( line, col, flags, 0, indtype )) intAZ_SetCmdResult( co, code );
    else commun_text_2( co, ptr, StrLen( ptr ), co->ArgRes[15], 0, (ULONG)co->ArgRes[19] );
}

//»»»»»» Cmd_Text2CB        CLIP=UNIT/K/N,CC=CONVERT/S,TEXT/F

ULONG func_Buf2CB( ULONG unit, UBYTE *buf, ULONG len )
{
  APTR clip;
  ULONG errmsg = 0;

    if (unit <= 255)
        {
        if (clip = func_CBWriteStart( unit ))
            {
            if (! func_CBWrite( clip, buf, len )) errmsg = TEXT_ERR_WriteClip;
            func_CBWriteEnd( clip, len );
            }
        else errmsg = TEXT_ERR_OpenClip;
        }
    else errmsg = TEXT_ERR_BadClipUnit;

    return( errmsg );
}

void CmdFunc_Text2CB_Do( struct CmdObj *co )
{
  ULONG errmsg = 0, unit = 0;
  UBYTE *ptr, *cnv = 0;

    if (co->ArgRes[0])                  // UNIT ?
        unit = *(ULONG*)co->ArgRes[0];

    if (ptr = co->ArgRes[2])            // TEXT ?
        {
        if (co->ArgRes[1])              // CONVERT ?
            {
            if (cnv = func_C2Bin( ptr, StrLen( ptr ) ))
                ptr = cnv;
            }
        }

    if (ptr)
        {
        errmsg = func_Buf2CB( unit, ptr, StrLen( ptr ) );
        BufFree( cnv );
        }

  END_ERROR:
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_Text2File      NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,NAME,PAT=PATTERN/K,CC=CONVERT/S,TEXT/F

static ULONG func_Text2File( struct AsyncFile *fout, UBYTE *str )
{
  ULONG len;

    if (len = StrLen( str ))
        {
        if (WriteAsync( fout, str, len ) != len) return( TEXT_ERR_WRITEFILE );
        }
    return(0);
}

void CmdFunc_Text2File_Do( struct CmdObj *co )
{
  UBYTE *filename, *ptr, *cnv = 0;
  ULONG errmsg = 0;
  ULONG locks;

    locks = func_VerifyOff();
    func_SetMainWaitPointer();
                                        // NAME ?
    if (! (filename = (UBYTE*)func_IntReqFileNameS( co->ArgRes[3], co->ArgRes[4], TEXT_REQ_Text2File, &errmsg, 1 ))) goto END_ERROR;

    if (ptr = co->ArgRes[6])            // TEXT ?
        {
        if (co->ArgRes[5])              // CONVERT ?
            {
            if (cnv = func_C2Bin( ptr, StrLen( ptr ) ))
                ptr = cnv;
            }
        }

    if (ptr)
        {                                                                        // pas d'icône
        errmsg = func_SaveFile( filename, (ULONG)co->ArgRes[0], (ULONG)co->ArgRes[2], 1, func_Text2File, ptr );
        if (errmsg==0 && co->ArgRes[1]==0 && TST_SAVEICON) func_SaveIcon( filename );
        BufFree( cnv );
        }

  END_ERROR:
    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    if (! errmsg)
        { if (co->ArgRes[6]) update_file2fs( filename, 0, 1 ); } // TEXT ?
    else intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_LineClone

void CmdFunc_LineClone_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  UBYTE *ptr, *dst;
  ULONG len;
  struct AZDisplayCmd DC;

    func_BlockClearAll();

    len = intAZ_Buf_Len( fo->Line, &ptr );

    if (! (dst = UIK_nl_AllocNode( &fo->Text, fo->Line+1, len, 0 ))) { intAZ_SetCmdResult( co, TEXT_NOMEMORY ); return; }
    func_BMAddLines( fo->Line+1, 1 );
    SET_MODIFIED( fo->Line, fo->Col );
    CopyMem( ptr, dst, len );
    fo->Text.UNode[fo->Line+1].Reserved = fo->Text.UNode[fo->Line].Reserved; // clone le id de fold

    func_CursorErase();
    DC.DisplayFrom = fo->Line + 1;
    DC.DisplayNum = 1;
    DC.ScrollFrom = fo->Line;
    DC.ScrollNum = -1;
    func_DisplayCmd( &DC );
    func_CursorDraw();
}

//»»»»»» Cmd_LineJoin

void CmdFunc_LineJoin_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZDisplayCmd DC;
  ULONG len, joinerr;

    len = func_LineTruncateSpace( fo->Line );
    if (len && fo->Col <= len) len++; else len = fo->Col;
    joinerr = func_intJoinLine( fo->Line, len, 1, 1, 0 );
    SET_MODIFIED( fo->Line, len );
    if (joinerr == 0)
        {
        DC.DisplayNum = 2;  // +1 car si en bas, pas de scroll
        DC.DisplayFrom = fo->Line;
        DC.ScrollNum = 1;
        DC.ScrollFrom = fo->Line + 1;
        func_DisplayCmd( &DC );
        func_AutoFormat();
        }
}

//»»»»»» Cmd_CharConvert    CHAR/S,WORD/S,LINE/S,ALL/S,LOWER/S,UPPER/S,TOGGLE/S,NOACCENT/S,ASCII7/S,VDX2AM/S,AM2VDX/S,MSDOS2AM/S,AM2MSDOS/S,ROT=ROTATE/S,XORKEY/K,NOCURS/S

#define ARG_CHAR       0
#define ARG_WORD       1
#define ARG_LINE       2
#define ARG_ALL        3
#define ARG_LOWER      4
#define ARG_UPPER      5
#define ARG_TOGGLE     6
#define ARG_NOACCENT   7
#define ARG_ASCII7     8
#define ARG_VDX2AM     9
#define ARG_AM2VDX    10
#define ARG_MSDOS2AM  11
#define ARG_AM2MSDOS  12
#define ARG_ROTATE    13
#define ARG_XORKEY    14
#define ARG_NOCURS    15

static UBYTE * __far rot_table = "NOPQRSTUVWXYZABCDEFGHIJKLM[\\]^_`nopqrstuvwxyzabcdefghijklm";

static ULONG convert_chars( ULONG type, UBYTE **pptr, ULONG start, ULONG len, LONG line )
{
  struct AZObjFile *fo = AGP.FObj;
  ULONG oldlen, newlen;
  LONG i;
  UBYTE *new, buf[4], *ptr = *pptr + start;

    fo->WorkLine = line;
    fo->WorkCol = start + len;

    oldlen = BufLength( *pptr );
    switch (type)
        {
        case ARG_LOWER:
            for (i=0; i<len; i++, ptr++) *ptr = UIK_ToLower( *ptr );
            break;
        case ARG_UPPER:
            for (i=0; i<len; i++, ptr++) *ptr = UIK_ToUpper( *ptr );
            break;
        case ARG_TOGGLE:
            for (i=0; i<len; i++, ptr++) *ptr = (*ptr & 0x20) ? UIK_ToUpper( *ptr ) : UIK_ToLower( *ptr );
            break;
        case ARG_NOACCENT:
            for (i=len-1; i >= 0; i--)
                {
                (*pptr)[start+i] = func_CharStripAccent( (*pptr)[start+i], &buf[0] );
                if (buf[0] > 1)
                    {
                    if (! BufPasteS( &buf[2], buf[0]-1, pptr, start+i+1 )) break;
                    fo->WorkCol += buf[0]-1;
                    }
                }
            break;
        case ARG_ASCII7:
            for (i=len-1; i >= 0; i--)
                {
                (*pptr)[start+i] = func_CharStripBit7( (*pptr)[start+i], &buf[0] );
                if (buf[0] > 1)
                    {
                    if (! BufPasteS( &buf[2], buf[0]-1, pptr, start+i+1 )) break;
                    fo->WorkCol += buf[0]-1;
                    }
                }
            break;
        case ARG_VDX2AM:
            if (new = (UBYTE*) func_Vdx2Amiga( pptr, start, len ))
                {
                BufFree( *pptr );
                *pptr = new;
                fo->WorkCol = intAZ_LineBufLen(new);
                }
            break;
        case ARG_AM2VDX:
            func_Amiga2Vdx( pptr, start, len );
            break;
        case ARG_MSDOS2AM:
            func_MSDOS2Amiga( pptr, start, len );
            if (start+len == oldlen)
                {
                if ((*pptr)[oldlen-1] == 0x0d) BufTruncate( *pptr, oldlen - 1 );
                }
            break;
        case ARG_AM2MSDOS:
            func_Amiga2MSDOS( pptr, start, len );
            if (start+len == oldlen)
                {
                BufPasteS( "\x0d", 1, pptr, oldlen );
                }
            break;
        case ARG_ROTATE:
            for (i=0; i<len; i++, ptr++) { if (*ptr >= 'A' && *ptr <= 'z') *ptr = rot_table[*ptr-'A']; }
            break;
        case ARG_XORKEY:
            func_DecodeText( AGP.UBufTmp, ptr, len );
            break;
        }
    newlen = BufLength( *pptr );
    if (newlen < oldlen) func_BMRemChars( line, newlen, oldlen - newlen );
    return(0);
}

void CmdFunc_CharConvert_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG sline, scol, eline, ecol, rect;
  ULONG type, len;
  UBYTE *ptr;
  ULONG block, numblocks;
  struct AZBlock *azb;

         if (co->ArgRes[ARG_LOWER   ]) type = ARG_LOWER   ;
    else if (co->ArgRes[ARG_UPPER   ]) type = ARG_UPPER   ;
    else if (co->ArgRes[ARG_TOGGLE  ]) type = ARG_TOGGLE  ;
    else if (co->ArgRes[ARG_NOACCENT]) type = ARG_NOACCENT;
    else if (co->ArgRes[ARG_ASCII7  ]) type = ARG_ASCII7  ;
    else if (co->ArgRes[ARG_VDX2AM  ]) type = ARG_VDX2AM  ;
    else if (co->ArgRes[ARG_AM2VDX  ]) type = ARG_AM2VDX  ;
    else if (co->ArgRes[ARG_MSDOS2AM]) type = ARG_MSDOS2AM;
    else if (co->ArgRes[ARG_AM2MSDOS]) type = ARG_AM2MSDOS;
    else if (co->ArgRes[ARG_ROTATE])   type = ARG_ROTATE;
    else if (co->ArgRes[ARG_XORKEY]) { type = ARG_XORKEY; ptr = (STRPTR)co->ArgRes[ARG_XORKEY]; BufSetS( ptr, StrLen(ptr), &AGP.UBufTmp ); }
    else { intAZ_SetCmdResult( co, TEXT_ERR_NoConversion ); return; }

    fo->WorkCol = -1;
    if (numblocks = func_BlockExists())
        {
        for (block=numblocks; block > 0; block--) // on part du haut
            {
            azb = func_BlockPtr( block );
            sline = azb->LineStart;
            scol  = azb->ColStart;
            eline = azb->LineEnd;
            ecol  = azb->ColEnd;
            rect  = (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) ? azb->Mode : 0;

            func_BlockRemove( block, 0, 0 ); // avant toute modif
            func_TextApplyReal( sline, scol, eline, ecol, rect, 0, convert_chars, (APTR)type );
            if (sline != eline) scol = 0;
            func_Display( sline, eline-sline+1, scol, -1 );
            if (! co->ArgRes[ARG_NOCURS]) func_CursorPos( eline, fo->WorkCol );
            SET_MODIFIED( eline, fo->WorkCol );
            }
        }
    else{
        len = intAZ_Buf_Len( fo->Line, &ptr );

        if (co->ArgRes[ARG_WORD])
            {
            sline = eline = fo->Line; scol = ecol = fo->Col;
            if (! func_CharIsWord( ptr[scol] )) return;
            for (; scol>0; scol--) if (! func_CharIsWord( ptr[scol-1] )) break;
            for (; ecol<len; ecol++) if (! func_CharIsWord( ptr[ecol+1] )) break;
            }
        else if (co->ArgRes[ARG_LINE])
            {
            sline = fo->Line; scol = 0; eline = fo->Line; ecol = len-1;
            }
        else if (co->ArgRes[ARG_ALL])
            {
            sline = 0; scol = 0; eline = fo->Text.NodeNum-1;
            ecol = intAZ_LineLen( eline ) - 1;
            }
        else // if (co->ArgRes[ARG_CHAR]) c'est le défaut
            {
            sline = eline = fo->Line; scol = ecol = fo->Col;
            }
        func_TextApplyReal( sline, scol, eline, ecol, 0, 0, convert_chars, (APTR)type );
        if (sline != eline) scol = 0;
        func_Display( sline, eline-sline+1, scol, -1 );
        if (fo->WorkCol != -1)
            {
            ULONG len = intAZ_LineLen( fo->WorkLine );
            if (fo->WorkCol > len) fo->WorkCol = len;
            if (! co->ArgRes[ARG_NOCURS])
                {
                func_CursorPos( eline, fo->WorkCol );
                fo->CEdCol = fo->WorkCol;
                }
            SET_MODIFIED( eline, fo->WorkCol );
            }
        }
    if (AGP.FObj) UPDATE_STAT(fo);
}

//»»»»»» Cmd_Search
// PREFS/S,RESET/S,FB=FROMBLOCK/S,CASE/S,ACC=ACCENT/S,ALL/S,SINGLE/S,BACK=BACKWARD=PREV=PREVIOUS/S,
// FORW=FORWARD=NEXT/S,FULL/S,FC=FROMCURS/S,IB=INTOBLOCK/S,PAT=PATTERN/S,CONV=CONVERT/S,CLEAR=OFF/S,
// TB=TOBLOCKS/S,NOBLOCK/S,SOL/S,EOL/S,SOW/S,EOW/S,REQ=REQUEST=PROMPT/S,NC=NOCURS/S,SEARCH=FIND=TEXT

#define ARGS_PREFS       0
#define ARGS_RESET       1
#define ARGS_FROMBLOCK   2
#define ARGS_CASE        3
#define ARGS_ACCENT      4
#define ARGS_ALL         5
#define ARGS_SINGLE      6
#define ARGS_BACK        7
#define ARGS_FORW        8
#define ARGS_FULL        9
#define ARGS_FROMCURS   10
#define ARGS_INTOBLOCK  11
#define ARGS_PATTERN    12
#define ARGS_CONVERT    13
#define ARGS_CLEAR      14
#define ARGS_TOBLOCKS   15
#define ARGS_NOBLOCK    16
#define ARGS_SOL        17
#define ARGS_EOL        18
#define ARGS_SOW        19
#define ARGS_EOW        20
#define ARGS_REQUEST    21
#define ARGS_NOCURS     22
#define ARGS_TEXT       23
#define ARGS_WITH       24
#define ARGS_NOVERIFY   25

ULONG DoRealSearch( struct CmdObj *co, ULONG repl, ULONG convert1, ULONG convert2, ULONG clear, ULONG docursor )
{
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  struct SearchRepQuery query_;
  UBYTE *ubuf, *ub;
  ULONG errmsg, (*do_func)();
  struct AZBlock *azb;

    //------ Si une chaîne de recherche est donnée ou REQUEST on converti
    if (convert1)
        {
        sr->SearchFlags &= ~SEF_VERIFYOK;
        if (sr->SearchFlags & SEF_CONVERT)
            {
            if (! (ubuf = func_C2Bin( sr->SearchStr, intAZ_LineBufLen(sr->SearchStr) ))) return( TEXT_NOMEMORY );
            BufFree( sr->ConvSearchStr );
            sr->ConvSearchStr = ubuf;
            }
        }
    //------ Si une chaîne de remplacement est donnée ou REQUEST on converti
    if (repl && convert2)
        {
        if (sr->SearchFlags & SEF_CONVERT)
            {
            if (! (ubuf = func_C2Bin( sr->ReplaceStr, intAZ_LineBufLen(sr->ReplaceStr) ))) return( TEXT_NOMEMORY );
            BufFree( sr->ConvReplaceStr );
            sr->ConvReplaceStr = ubuf;
            }
        }

    query_.StartLine   = -1;
    query_.StartCol    = -1;
    query_.EndLine     = -1;
    query_.EndCol      = -1;
    query_.SStr        = (APTR)-1;
    query_.SLen        = -1;
    query_.RStr        = (APTR)-1;
    query_.RLen        = -1;
    query_.SearchFlags = -1;
    query_.DoDisplay   = 1;
    query_.DoCursor    = docursor;
    do_func = repl ? func_TextReplace : func_TextSearch;

    //------ Si simple recherche part sur un block
    if (!repl && !(sr->SearchFlags & SEF_ALL))
        {
        if (fo->FoundCol == -1)
            {//------ la recherche commence juste : si le curseur est dans un bloc -> on se cadre dedans
            if (azb = func_BlockPtr( func_BlockCurrent() ))
                {
                if (((fo->Line > azb->LineStart) || (fo->Line == azb->LineStart && fo->Col >= azb->ColStart))
                 && ((fo->Line < azb->LineEnd) || (fo->Line == azb->LineEnd && fo->Col <= azb->ColEnd)))
                    {
                    if (sr->SearchFlags & SEF_BACKWARD)
                        {
                        query_.EndLine = azb->LineStart;
                        query_.EndCol  = azb->ColStart;
                        }
                    else{
                        query_.StartLine = azb->LineEnd;
                        query_.StartCol  = azb->ColEnd+1;
                        }
                    }
                }
            }
        }

    //------ Si Verif, on cherche au lieu de remplacer
    if (repl && !(sr->SearchFlags & SEF_ALL))
        {
        if (fo->FoundCol == -1)
            {//------ la recherche commence juste : si le curseur est dans un bloc -> pas de verification
            if (azb = func_BlockPtr( func_BlockCurrent() ))
                {
                if (((fo->Line > azb->LineStart) || (fo->Line == azb->LineStart && fo->Col >= azb->ColStart))
                 && ((fo->Line < azb->LineEnd) || (fo->Line == azb->LineEnd && fo->Col <= azb->ColEnd)))
                    {
                    if (sr->SearchFlags & SEF_BACKWARD)
                        {
                        query_.EndLine = azb->LineEnd;
                        query_.EndCol  = azb->ColEnd;
                        }
                    else{
                        query_.StartLine = azb->LineStart;
                        query_.StartCol  = azb->ColStart;
                        }
                    sr->SearchFlags |= SEF_VERIFYOK;
                    }
                }
            }
        else{//------ on continue une recherche déjà commencée : doit-on vérifier? Est-ce déjà fait?
            if (fo->Line == fo->FoundLine && fo->Col == fo->FoundCol)
                {
                if (sr->SearchFlags & SEF_BACKWARD)
                    {
                    query_.EndLine = fo->FoundELine;
                    query_.EndCol  = fo->FoundECol;
                    }
                sr->SearchFlags |= SEF_VERIFYOK;
                }
            else sr->SearchFlags &= ~SEF_VERIFYOK;
            }

         if (! (sr->SearchFlags & SEF_NOVERIFY))
             {
             if (sr->SearchFlags & SEF_VERIFYOK)
                 {
                 sr->SearchFlags &= ~SEF_VERIFYOK;
                 }
             else{
                 do_func = func_TextSearch;
                 sr->SearchFlags |= SEF_VERIFYOK;
                 }
            }
        }
    else sr->SearchFlags &= ~SEF_VERIFYOK; // clear pour prochaine fois

    //------ On efface les blocs ?
    if ((clear || repl) && !(sr->SearchFlags & SEF_BLOCK)) // si SEF_BLOCK clear dans func_TextSearch()
        {
        func_BlockClearAll();
        }

  //============================
    errmsg = do_func( &query_ );
  //============================

    if (errmsg)
        {
        sr->SearchFlags &= ~SEF_VERIFYOK;
        if (errmsg == TEXT_Search_NotFound)
            {
            if (ub = BufAllocP( AGP.Pool, 0, 100 ))
                {
                func_BufPrintf( &ub, 0, "\"%ls\"\n\n%ls", sr->SearchStr, UIK_LangString( AGP.UIK, errmsg ) );
                if (co) func_SetCmdResult( co, 5, errmsg, ub ); else func_ReqShowText( ub );
                BufFree( ub );
                return(-1);
                }
            }
        return( errmsg );
        }
    else{
        UBYTE buf[200];

        if (co && (sr->Flags & SEF_ALL) && !(co->ExeFrom & FROMAREXX))
            {
            sprintf( buf, UIK_LangString( AGP.UIK, repl ? TEXT_Search_ReplaceTimes : TEXT_Search_FoundTimes ), query_.FoundCount );
            func_ReqShowText( buf );
            }
        sprintf( buf, "%ld", query_.FoundCount );
        if (co) func_SetCmdResult( co, 0, 0, buf );
        }

    return(0);
}

/*
static ULONG cadre_recherche_block( struct SearchRepQuery *q, ULONG flags )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZBlock *azb;

    if (azb = func_BlockPtr( func_BlockCurrent() ))
        {
        if (((fo->Line > azb->LineStart) || (fo->Line == azb->LineStart && fo->Col >= azb->ColStart))
         && ((fo->Line < azb->LineEnd) || (fo->Line == azb->LineEnd && fo->Col <= azb->ColEnd)))
            {
            if (flags & SEF_BACKWARD)
                {
                q->EndLine = azb->LineEnd;
                q->EndCol  = azb->ColEnd;
                }
            else{
                q->StartLine = azb->LineStart;
                q->StartCol  = azb->ColStart;
                }
            return(1);
            }
        }
    return(0);
}
*/

ULONG search_common( struct CmdObj *co, ULONG *pres, ULONG repl )
{
  struct AZObjFile *fo = AGP.FObj;
  struct SearchReplace *sr = fo->SeRep;
  ULONG errmsg; // backw;

//    backw = sr->SearchFlags & SEF_BACKWARD;

    if (co->ArgRes[ARGS_PREFS])     { sr->SearchFlags = AGP.Prefs->SearchFlags; }
    if (co->ArgRes[ARGS_RESET])     { sr->SearchFlags = 0; }
    if (co->ArgRes[ARGS_CASE])      { sr->SearchFlags |= SEF_CASE; }
    if (co->ArgRes[ARGS_ACCENT])    { sr->SearchFlags |= SEF_ACCENT; }
    if (co->ArgRes[ARGS_SINGLE])    { sr->SearchFlags &=~SEF_ALL; }
    if (co->ArgRes[ARGS_ALL])       { sr->SearchFlags |= SEF_ALL; }
    if (co->ArgRes[ARGS_FORW])      { sr->SearchFlags &=~SEF_BACKWARD; }
    if (co->ArgRes[ARGS_BACK])      { sr->SearchFlags |= SEF_BACKWARD; }
    if (co->ArgRes[ARGS_FROMCURS])  { sr->SearchFlags &=~SEF_FULL; }
    if (co->ArgRes[ARGS_FULL])      { sr->SearchFlags |= SEF_FULL; }
    if (co->ArgRes[ARGS_INTOBLOCK]) { sr->SearchFlags |= SEF_BLOCK; }
    if (co->ArgRes[ARGS_PATTERN])   { sr->SearchFlags |= SEF_PATTERN; }
    if (co->ArgRes[ARGS_CONVERT])   { sr->SearchFlags |= SEF_CONVERT; }
    if (co->ArgRes[ARGS_NOBLOCK])   { sr->SearchFlags &=~SEF_TOBLOCK; }
    if (co->ArgRes[ARGS_TOBLOCKS])  { sr->SearchFlags |= SEF_TOBLOCK; }
    if (co->ArgRes[ARGS_SOL])       { sr->SearchFlags |= SEF_STARTOFLINE; }
    if (co->ArgRes[ARGS_EOL])       { sr->SearchFlags |= SEF_ENDOFLINE; }
    if (co->ArgRes[ARGS_SOW])       { sr->SearchFlags |= SEF_STARTOFWORD; }
    if (co->ArgRes[ARGS_EOW])       { sr->SearchFlags |= SEF_ENDOFWORD; }
    if (repl && co->ArgRes[ARGS_NOVERIFY])  { sr->SearchFlags |= SEF_NOVERIFY; }

    if (co->ArgRes[ARGS_FROMBLOCK])
        {
        BufTruncate( sr->SearchStr, 0 );
        if (errmsg = func_Block2Buf( &sr->SearchStr, 0 )) return( errmsg );
        if (repl) BufCopyS( sr->SearchStr, &sr->ReplaceStr, 0, BufLength(sr->SearchStr) );
        }

    //------ Verifs
    verif_searchflags( sr, repl );

    //------ Si une chaîne de recherche est donnée on remplace
    if (co->ArgRes[ARGS_TEXT])
        {
        if (! BufSetS( co->ArgRes[ARGS_TEXT], StrLen( co->ArgRes[ARGS_TEXT] ), &sr->SearchStr )) return( TEXT_NOMEMORY );
        }
    //------ Si une chaîne de remplacement est donnée on remplace
    if (repl && co->ArgRes[ARGS_WITH])
        {
        if (! BufSetS( co->ArgRes[ARGS_WITH], StrLen( co->ArgRes[ARGS_WITH] ), &sr->ReplaceStr )) return( TEXT_NOMEMORY );
        }

    //------ Si REQUEST on affiche la requête
    if (co->ArgRes[ARGS_REQUEST])
        {
        sr->SearchFlags &= ~SEF_VERIFYOK;
        *pres = repl ? func_ReqSearchReplace() : func_ReqSearch();
        if (*pres == REQBUT_CANCEL) return( TEXT_CMDCANCELLED );
        if (*pres == REQBUT_MIDDLE) repl = 0;
        }

//    if (backw != (sr->SearchFlags & SEF_BACKWARD)) // si changement de sens...
//        { fo->FoundLine = -1; fo->FoundCol = -1; }

    return( DoRealSearch(
                    co,
                    repl,
                    (co->ArgRes[ARGS_REQUEST] || co->ArgRes[ARGS_TEXT]),
                    (co->ArgRes[ARGS_REQUEST] || co->ArgRes[ARGS_WITH]),
                    (ULONG)co->ArgRes[ARGS_CLEAR],
                    co->ArgRes[ARGS_NOCURS] ? 0 : 1
                    ) );
}

void CmdFunc_Search_Do( struct CmdObj *co )
{
  ULONG errmsg, butres;

    errmsg = search_common( co, &butres, 0 );
    if (errmsg && errmsg != -1) intAZ_SetCmdResult( co, errmsg );

}

//»»»»»» Cmd_Replace

void CmdFunc_Replace_Do( struct CmdObj *co )
{
  ULONG errmsg, butres;

    errmsg = search_common( co, &butres, 1 );
    if (errmsg && errmsg != -1) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_Surround       F=FIRST/A,L=LAST,M=MIDDLE,C=CORNER,LINE/S,RECT/S,SL=STARTLINE/N,SC=STARTCOL/N,EL=ENDLINE/N,EC=ENDCOL/N,OV=OVERLAY/S,NC=NOCURS/S

#define ARGSU_FIRST        0
#define ARGSU_LAST         1
#define ARGSU_MIDDLE       2
#define ARGSU_CORNER       3
#define ARGSU_LINE         4
#define ARGSU_RECT         5
#define ARGSU_STARTLINE    6
#define ARGSU_STARTCOL     7
#define ARGSU_ENDLINE      8
#define ARGSU_ENDCOL       9
#define ARGSU_OVERLAY     10
#define ARGSU_NOCURS      11
#define ARGSU_CONVERT     12

struct SurroundQuery
    {
    //------ arguments : initialiser tous ces champs
    LONG    FirstLine;
    LONG    LastLine;
    LONG    CursLine;
    LONG    CursCol;
    LONG    StartLine;
    LONG    StartCol;
    LONG    EndLine;
    LONG    EndCol;
    UBYTE   *First;
    ULONG   LenFirst;
    UBYTE   *Last;
    ULONG   LenLast;
    UBYTE   *Midd;
    ULONG   LenMidd;
    UBYTE   *Corner;
    ULONG   HLen;
    UBYTE   LineMode;
    UBYTE   RectMode;
    UWORD   Flags;

    //------ results : pas besoin d'initialiser
    struct AZDisplayCmd DC;
    };

static ULONG print( UBYTE *buf, ULONG len, LONG line, LONG col, UWORD flags )
{
  ULONG errmsg=0;

    if (buf)
        {
        if (errmsg = func_TextPrep( line, col, flags, 0, 0 )) goto END_ERROR;
        if (errmsg = func_TextPut( buf, len, 0, 0 )) goto END_ERROR;
        }
  END_ERROR:
    return( errmsg );
}

static ULONG draw_horiz( struct SurroundQuery *sq, LONG line )
{
  ULONG i, errmsg=0;

    if (errmsg = func_TextPrep( line, sq->StartCol, ARPF_OVERLAY, 0, 0 )) goto END_ERROR;
    if (errmsg = func_TextPut( sq->Corner, 1, 0, 0 )) goto END_ERROR;
    for (i=0; i < sq->HLen; i++)
        if (errmsg = func_TextPut( sq->Midd, 1, 0, 0 )) goto END_ERROR;
    if (errmsg = func_TextPut( sq->Corner, 1, 0, 0 )) goto END_ERROR;

  END_ERROR:
    return( errmsg );
}

static ULONG DoSurround( struct SurroundQuery *sq )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG i;
  ULONG errmsg=0;

    if (sq->RectMode)
        {
        sq->EndCol++;

        if (sq->Flags & ARPF_OVERLAY)
            {
            if (sq->StartCol > (sq->LenFirst-1)) sq->StartCol -= (sq->LenFirst-1); // -1 car \n à la fin de First
            else sq->StartCol = 0;
            }

        //-------- Tracé du haut du tableau
        if (sq->Corner)
            {
            sq->HLen = (sq->EndCol + (sq->LenLast-1)) - sq->StartCol - 2;
            if (! (sq->Flags & ARPF_OVERLAY))
                {
                if (errmsg = print( "\n", 1, sq->EndLine, -1, ARPF_NOOVERLAY )) goto END_ERROR;
                if (errmsg = print( "\n", 1, sq->StartLine, 0, ARPF_NOOVERLAY )) goto END_ERROR;
                sq->StartLine++;
                sq->EndLine++;
                sq->HLen += (sq->LenFirst-1);
                }
            if (sq->StartLine > 0)
                {
                if (errmsg = draw_horiz( sq, sq->StartLine-1 )) goto END_ERROR;
                }
            }

        //-------- Tracé des bords
        for (i=sq->EndLine; i >= sq->StartLine; i--)
            {
            if (errmsg = print( sq->Last, sq->LenLast, i, sq->EndCol, sq->Flags )) goto END_ERROR;
            if (fo->WorkLine >= sq->CursLine) { sq->CursLine = fo->WorkLine; sq->CursCol = fo->WorkCol; }
            if (errmsg = print( sq->First, sq->LenFirst, i, sq->StartCol, sq->Flags )) goto END_ERROR;
            }

        //-------- Tracé du bas du tableau
        if (sq->Corner)
            {
            if (errmsg = draw_horiz( sq, sq->EndLine+1 )) goto END_ERROR;
            }
        }
    else if (sq->LineMode)
        {
        for (i=sq->EndLine; i >= sq->StartLine; i--)
            {
            if (errmsg = print( sq->Last, sq->LenLast, i, intAZ_LineLen( i ), sq->Flags )) goto END_ERROR;
            if (fo->WorkLine >= sq->CursLine) { sq->CursLine = fo->WorkLine; sq->CursCol = fo->WorkCol; }
            if (errmsg = print( sq->First, sq->LenFirst, i, sq->StartCol, sq->Flags )) goto END_ERROR;
            }
        }
    else{
        if (sq->StartLine == sq->EndLine) sq->EndCol++;
        else sq->EndCol = sq->StartCol;

        if (errmsg = print( sq->Last, sq->LenLast, sq->EndLine, sq->EndCol, sq->Flags )) goto END_ERROR;
        if (fo->WorkLine >= sq->CursLine) { sq->CursLine = fo->WorkLine; sq->CursCol = fo->WorkCol; }
        if (sq->EndLine == sq->StartLine)
            {
            if (errmsg = print( sq->First, sq->LenFirst, sq->StartLine, sq->StartCol, sq->Flags )) goto END_ERROR;
            }
        else{
            if (sq->EndCol > sq->StartCol)
                {
                if (errmsg = print( sq->Midd, sq->LenMidd, sq->StartLine, sq->StartCol, sq->Flags )) goto END_ERROR;
                }
            for (i=sq->EndLine-1; i > sq->StartLine; i--)
                {
                if (errmsg = print( sq->Midd, sq->LenMidd, i, sq->StartCol, sq->Flags )) goto END_ERROR;
                }
            if (errmsg = print( sq->First, sq->LenFirst, sq->StartLine, sq->StartCol, sq->Flags )) goto END_ERROR;
            }
        }

  END_ERROR:
    return( errmsg );
}

void CmdFunc_Surround_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  struct SurroundQuery SQ;
  ULONG errmsg=0, len;
  UBYTE *ub=0, *ptr, *c1=0, *c2=0, *c3=0, *c4=0;

    /*----------------- Valeurs par défaut */
    SQ.FirstLine = 0x7fffffff;
    SQ.LastLine = -1;
    SQ.StartLine = fo->OldLine;
    SQ.StartCol = fo->OldCol;
    SQ.EndLine = fo->Line;
    SQ.EndCol = fo->Col;
    SQ.Flags = ARPF_RECT | ARPF_CONVERT | ARPF_NOOVERLAY;

    /*----------------- Valeurs données en argument */
    SQ.First = (UBYTE*)co->ArgRes[ARGSU_FIRST ];
    SQ.Last  = (UBYTE*)co->ArgRes[ARGSU_LAST  ];
    SQ.Midd  = (UBYTE*)co->ArgRes[ARGSU_MIDDLE];
    SQ.Corner= (UBYTE*)co->ArgRes[ARGSU_CORNER];
    if (co->ArgRes[ARGSU_CONVERT])
        {
        if (c1 = func_C2Bin( SQ.First, StrLen( SQ.First ) )) SQ.First = c1;
        if (c2 = func_C2Bin( SQ.Last,  StrLen( SQ.Last  ) )) SQ.Last  = c2;
        if (c3 = func_C2Bin( SQ.Midd,  StrLen( SQ.Midd  ) )) SQ.Midd  = c3;
        if (c4 = func_C2Bin( SQ.Corner,StrLen( SQ.Corner) )) SQ.Corner= c4;
        }
    SQ.LineMode = (UBYTE)co->ArgRes[ARGSU_LINE];
    SQ.RectMode = (UBYTE)co->ArgRes[ARGSU_RECT];
    if (co->ArgRes[ARGSU_STARTLINE]) SQ.StartLine = *(ULONG*)co->ArgRes[ARGSU_STARTLINE] - 1;
    if (co->ArgRes[ARGSU_STARTCOL ]) SQ.StartCol  = *(ULONG*)co->ArgRes[ARGSU_STARTCOL ] - 1;
    if (co->ArgRes[ARGSU_ENDLINE  ]) SQ.EndLine   = *(ULONG*)co->ArgRes[ARGSU_ENDLINE  ] - 1;
    if (co->ArgRes[ARGSU_ENDCOL   ]) SQ.EndCol    = *(ULONG*)co->ArgRes[ARGSU_ENDCOL   ] - 1;
    if (co->ArgRes[ARGSU_OVERLAY  ]) SQ.Flags    |= ARPF_OVERLAY;

    if (SQ.EndLine == -2) SQ.EndLine = func_NumLines() - 1; // si ENDLINE=-1

    SQ.LenFirst = (SQ.First) ? strlen( SQ.First ) : 0;
    SQ.LenLast  = (SQ.Last)  ? strlen( SQ.Last  ) : 0;
    SQ.LenMidd  = (SQ.Midd)  ? strlen( SQ.Midd  ) : 0;
    len = SQ.LenFirst + SQ.LenLast + SQ.LenMidd + 1 + 4;
    if (! (ptr = ub = BufAllocP( AGP.Pool, len, len ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
    if (SQ.First) { CopyMem( SQ.First, ptr, SQ.LenFirst ); SQ.First = ptr; ptr += SQ.LenFirst; SQ.LenFirst++; }
    *ptr++ = '\n';
    if (SQ.Last) { CopyMem( SQ.Last, ptr, SQ.LenLast ); SQ.Last = ptr; ptr += SQ.LenLast; SQ.LenLast++; }
    *ptr++ = '\n';
    if (SQ.Midd) { CopyMem( SQ.Midd, ptr, SQ.LenMidd ); SQ.Midd = ptr; ptr += SQ.LenMidd; SQ.LenMidd++; }
    *ptr++ = '\n';
    if (SQ.Corner) { CopyMem( SQ.Corner, ptr, 1 ); SQ.Corner = ptr; ptr += 1; }
    *ptr++ = '\n';

    /*----------------- On y va... */
    func_SetMainWaitPointer();

    SQ.CursLine = SQ.CursCol = 0;
    if (func_BlockExists())
        {
        struct AZBlock *azb;
        ULONG block, numblocks = func_BlockExists();

        for (block=numblocks; block > 0; block--) // on part du haut
            {
            azb = func_BlockPtr( block );
            SQ.StartLine = azb->LineStart;
            SQ.StartCol = azb->ColStart;
            SQ.EndLine = azb->LineEnd;
            SQ.EndCol = azb->ColEnd;
            SQ.RectMode = (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) ? 1 : 0;
            SQ.LineMode = (azb->Mode == BLKMODE_LINE) ? 1 : 0;
            //if (SQ.RectMode) SQ.EndCol++;
            func_BlockRemove( block, 0, 0 );

            if (SQ.StartLine < SQ.FirstLine) SQ.FirstLine = SQ.StartLine;
            if (SQ.StartLine > SQ.LastLine) SQ.LastLine = SQ.StartLine;
            if (SQ.EndLine < SQ.FirstLine) SQ.FirstLine = SQ.EndLine;
            if (SQ.EndLine > SQ.LastLine) SQ.LastLine = SQ.EndLine;
            if (errmsg = DoSurround( &SQ )) break;
            }
        }
    else{
        if (SQ.StartLine > SQ.EndLine || (SQ.StartLine == SQ.EndLine && SQ.StartCol > SQ.EndCol))
            errmsg = TEXT_BadPosition;
        else{
            if (SQ.StartLine < SQ.FirstLine) SQ.FirstLine = SQ.StartLine;
            if (SQ.StartLine > SQ.LastLine) SQ.LastLine = SQ.StartLine;
            if (SQ.EndLine < SQ.FirstLine) SQ.FirstLine = SQ.EndLine;
            if (SQ.EndLine > SQ.LastLine) SQ.LastLine = SQ.EndLine;
            errmsg = DoSurround( &SQ );
            }
        }

    /*----------------- Affichage des modifications */
    if (SQ.FirstLine <= SQ.LastLine)
        {
        SQ.DC.ScrollNum = 0;
        SQ.DC.DisplayFrom = SQ.FirstLine;
        SQ.DC.DisplayNum = SQ.LastLine - SQ.FirstLine + 1;
        if (SQ.RectMode && SQ.Corner)
            {
            if (SQ.DC.DisplayFrom) { SQ.DC.DisplayFrom--; SQ.DC.DisplayNum += 2; }
            if (! (SQ.Flags & ARPF_OVERLAY)) SQ.DC.DisplayNum = -1;
            }
        func_DisplayCmd( &SQ.DC );
        SQ.DC.DisplayNum = 0;
        fo->WorkLine = SQ.CursLine;  fo->WorkCol = SQ.CursCol;
        }
    func_SetMainSystemPointer();
    func_DisplayStats(0);

    if (! co->ArgRes[ARGSU_NOCURS])
        { func_CursorPos( fo->WorkLine, fo->WorkCol ); fo->CEdCol = fo->WorkCol; } /* remet le curseur */
  END_ERROR:
    BufFree( ub );
    BufFree( c4 );
    BufFree( c3 );
    BufFree( c2 );
    BufFree( c1 );
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_TextJustify    C=CENTER/S,F=FLUSH/S,L=LEFT/S,R=RIGHT/S

#define ARGJ_CENTER    0
#define ARGJ_FLUSH     1
#define ARGJ_LEFT      2
#define ARGJ_RIGHT     3

void CmdFunc_TextJustify_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZDisplayCmd DI;
  LONG sline, scol, eline, ecol, len, max;
  ULONG errmsg, flags;
  UBYTE *ptr;

    if (co->ArgRes[ARGJ_CENTER])     flags = JUS_CENTER;
    else if (co->ArgRes[ARGJ_FLUSH]) flags = JUS_FLUSH ;
    else if (co->ArgRes[ARGJ_LEFT])  flags = JUS_LEFT  ;
    else if (co->ArgRes[ARGJ_RIGHT]) flags = JUS_RIGHT ;
    else return;

    if (func_BlockExists())
        {
        struct AZBlock *azb;
        ULONG block, numblocks = func_BlockExists();

        for (block=numblocks; block > 0; block--) // on part du haut
            {
            azb = func_BlockPtr( block );
            sline = azb->LineStart;
            scol  = azb->ColStart;
            eline = azb->LineEnd;
            ecol  = azb->ColEnd + 1;

            flags &= JUS_MASK; // vide les flags
            if (azb->Mode != BLKMODE_RECT && azb->Mode != BLKMODE_FULL) flags |= FMTF_KEEPALINEA;

            if (/*sline != eline &&*/ azb->Mode != BLKMODE_RECT && azb->Mode != BLKMODE_FULL)
                {
                if (TST_USEMARGIN) { scol = fo->LeftMargin; ecol = fo->RightMargin; }
                }
            //func_BlockRemove( block, 0, 0 );
            if (errmsg = func_TextJustify( sline, scol, eline, ecol, flags, &DI )) break;
            if (DI.DisplayNum == 0)
                { DI.DisplayNum = eline - sline +1; DI.DisplayFrom = sline; } // si ligne untouchées, effacer le bloc
            func_DisplayCmd( &DI );
            //func_CursorPos( DI.Line, intAZ_LineLen( DI.Line ) );
            }
        }
    else{
        flags |= FMTF_KEEPALINEA;
        sline = eline = fo->Line;

        ecol = intAZ_Buf_Len( sline, &ptr );
        scol = skip_char( ptr, ' ', '\t' );     // cherche la plus longue du paragraphe
        max = fo->Text.NodeNum - 1;
        while ((fo->Text.UNode[eline].Flags & UNF_CONTINUE) && (eline < max))
            {
            if ((flags & JUS_MASK) == JUS_FLUSH && !(fo->Text.UNode[eline+1].Flags & UNF_CONTINUE)) break;
            eline++;
            if ((len = intAZ_LineLen( eline )) > ecol) ecol = len;
            }
        if (TST_USEMARGIN) { scol = fo->LeftMargin; ecol = fo->RightMargin; }

        errmsg = func_TextJustify( sline, scol, eline, ecol, flags, &DI );
        func_DisplayCmd( &DI );
        //func_CursorPos( fo->WorkLine, fo->WorkCol );
        }

    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_TextFormat     JP=JOINPARA/S

void CmdFunc_TextFormat_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZDisplayCmd DI;
  ULONG errmsg;
  LONG flags, sline, scol, eline, ecol, len, max, doline;
  UBYTE *ptr;

    if (func_BlockExists())
        {
        struct AZBlock *azb;
        ULONG block, numblocks = func_BlockExists();

        for (block=numblocks; block > 0; block--) // on part du haut
            {
            azb = func_BlockPtr( block );
            sline = azb->LineStart;
            scol  = azb->ColStart;
            eline = azb->LineEnd;
            ecol  = azb->ColEnd + 1;

            flags = 0;
            if (co->ArgRes[0]) flags |= FMTF_JOINPARA;
            if (azb->Mode != BLKMODE_RECT && azb->Mode != BLKMODE_FULL) flags |= FMTF_KEEPALINEA;

            if (/*sline != eline &&*/ azb->Mode != BLKMODE_RECT && azb->Mode != BLKMODE_FULL)
                {
                if (TST_USEMARGIN) { scol = fo->LeftMargin; ecol = fo->RightMargin; }
                }
            func_BlockRemove( block, 0, 0 );
            if (errmsg = func_TextFormat( sline, scol, eline, ecol, flags, &DI )) break;
            if (DI.DisplayNum == 0)
                { DI.DisplayNum = eline - sline + 1; DI.DisplayFrom = sline; } // si ligne untouchées, effacer le bloc
            func_DisplayCmd( &DI );
            //func_CursorPos( DI.Line, intAZ_LineLen( DI.Line ) );
            }
        }
    else{
        flags = FMTF_KEEPALINEA;
        if (co->ArgRes[0]) flags |= FMTF_JOINPARA;
        sline = eline = fo->Line;

        ecol = intAZ_Buf_Len( sline, &ptr );
        scol = skip_char( ptr, ' ', '\t' );     // cherche la plus longue du paragraphe
        max = fo->Text.NodeNum - 1;

        if (flags & FMTF_JOINPARA) doline = 1;
        while (eline < max)
            {
            if (! (fo->Text.UNode[eline].Flags & UNF_CONTINUE))
                {
                if (! (flags & FMTF_JOINPARA)) break;
                else{
                    if (doline == 1) doline = 0;
                    else break;
                    }
                }
            eline++;
            if ((len = intAZ_LineLen( eline )) > ecol) ecol = len;
            }
        if (TST_USEMARGIN) { scol = fo->LeftMargin; ecol = fo->RightMargin; }

        errmsg = func_TextFormat( sline, scol, eline, ecol, flags, &DI );
        func_DisplayCmd( &DI );
        //func_CursorPos( fo->WorkLine, fo->WorkCol );
        }

    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}
