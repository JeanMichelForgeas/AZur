/****************************************************************
 *
 *      File:       cmd_curs.c      commandes curseur
 *      Project:    AZur            editeur de texte
 *
 *      Created:    25/06/93        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objreqstring.h"
#include "uiki:objwinfilesel.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/

ULONG func_Block2CB( ULONG unit );


/****** Statics *************************************************/


/****************************************************************
 *
 *      Common routines
 *
 ****************************************************************/

static ULONG decrypt_position( struct CmdObj *co, LONG *arg, LONG *pline, ULONG *pcol )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG line=fo->Line, col=fo->Col;
  ULONG rc=1;

    if (arg[15])                                            // POINTER
        {
        struct AZObjZone *zo = fo->ActiveView->Zone;
        struct Rectangle *r = &zo->Obj.ObjExt->InnerRect;
        struct TextFont *font = zo->Obj.ObjExt->Font;
        if (AGP.WO && AGP.WO->Obj.w)
            {
            fo->MouseX = AGP.WO->Obj.w->MouseX;
            fo->MouseY = AGP.WO->Obj.w->MouseY;
            }
        line = zo->TopLine + ((fo->MouseY - r->MinY) / font->tf_YSize);
        col  = zo->TopCol  + ((fo->MouseX - r->MinX) / font->tf_XSize);
        if (col < 0) col = 0;
        if (line < 0) line = 0; else if (line >= fo->Text.NodeNum) line = fo->Text.NodeNum - 1;

        fo->CEdCol = col;   // avant WRAP !
        if (arg[11])                                        // WRAP
            {
            ULONG len = intAZ_LineLen( line );
            if (col > len) col = len;
            }
        }
    else{
        if (arg[13])                                        // LINES
            {
            arg[5] = -1;                                    // set RELATIVE
            arg[2] = arg[13];                               // LINE = LINES
            }
        if (arg[14])                                        // CHARS
            {
            arg[5] = -1;                                    // set RELATIVE
            arg[3] = arg[14];                               // COLUMN = CHARS
            }

        if (arg[1])                                         // REQUEST
            {
            if (arg[8])                                     // OFFSET
                {
                if (func_ReqOffset() == REQBUT_CANCEL)
                    { intAZ_SetCmdResult( co, TEXT_CMDCANCELLED ); rc = 0; }
                }
            else{
                if (func_ReqLineColumn() == REQBUT_CANCEL)
                    { intAZ_SetCmdResult( co, TEXT_CMDCANCELLED ); rc = 0; }
                }
            if (rc)
                {
                line = fo->WorkReqLine;
                col = fo->WorkReqCol;
                }
            }
        else{
            if (! func_ComputeLineColFlags( (LONG*) &arg[2], &line, &col, arg[0]?CLCF_FOLDED:0 ))
                { intAZ_SetCmdResult( co, TEXT_BadPosition ); rc = 0; }
            }
        }

    *pline = line;
    *pcol = col;
    return( rc );
}


/****************************************************************
 *
 *      Cmd Functions
 *
 ****************************************************************/

//»»»»»» Cmd_Cursor FOLDS/S,REQUEST/S,LINE/N,COL=COLUMN/N,PAGE/S,REL=RELATIVE/S,WORD/N,TAB/N,OF=OFFSET/N,UP/S,LEFT/S,W=WRAP/S,ENDWORD/S,LINES/N,CHARS/N,PO=POINTER/S,CLEAR=OFF/S,PB=PREVBLOCK/S,NB=NEXTBLOCK/S,PM=PREVMARK/S,NM=NEXTMARK/S,PF=PREVFOLD/S,NF=NEXTFOLD/S

void CmdFunc_Cursor_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZBlock *azb;
  struct AZBM *azm;
  LONG line, col, startline, endline;
  UBYTE *foldptr;

    if (decrypt_position( co, (LONG*)&co->ArgRes[0], &line, &col ) == 0) return;
    // on place le curseur même si hors limites
    if (col < 0) col = 0;
    if (line < 0) line = 0;
    if (line >= fo->Text.NodeNum) line = fo->Text.NodeNum - 1;

    if (co->ArgRes[17])         // PREVBLOCK
        {
        if (! (azb = (struct AZBlock *) func_BlockGetPrev( line, col ))) return;
        line = azb->LineStart; col = azb->ColStart;
        func_PosPrev( &line, &col );
        }
    else if (co->ArgRes[18])    // NEXTBLOCK
        {
        if (! (azb = (struct AZBlock *) func_BlockGetNext( line, col ))) return;
        line = azb->LineEnd; col = azb->ColEnd;
        func_PosNext( &line, &col );
        }
    if (co->ArgRes[19])         // PREVMARK
        {
        if (! (azm = (struct AZBM *) func_BMGetPrev( line, col ))) return;
        line = azm->Line; col = azm->Col;
        }
    else if (co->ArgRes[20])    // NEXTMARK
        {
        if (! (azm = (struct AZBM *) func_BMGetNext( line, col ))) return;
        line = azm->Line; col = azm->Col;
        }
    if (co->ArgRes[21])         // PREVFOLD
        {
        if (! func_FoldGetPrev( &line, &col )) return;
        }
    else if (co->ArgRes[22])    // NEXTFOLD
        {
        if (! func_FoldGetNext( &line, &col )) return;
        }

    func_CursorPos( line, col );
    if (co->ArgRes[16])             // CLEAR
        {
        if (func_BlockRemAll( &startline, &endline ))
            func_Display( startline, endline - startline + 1, 0, -1 );
        }
    //func_BlockStart( line, col, (ULONG)co->ArgRes[15] /*POINTER*/ );
    fo->CurBlockMode = AGP.Prefs->BlockMode;

    foldptr = "OFF";
    if ((fo->Text.UNode[line].Flags & UNF_FOLDED) || fo->Text.UNode[line].Reserved) foldptr = "ON";
    func_BufPrintf( &AGP.RexxResult, 0, "%ld %ld %ls", line+1, col+1, foldptr );
}

//»»»»»» Cmd_CursorOld

void CmdFunc_CursorOld_Do( struct CmdObj *co )
{
    func_CursorPos( AGP.FObj->OldLine, AGP.FObj->OldCol );
}

//»»»»»» Cmd_Block
// CLIP/K/N,    CLEAR=OFF/S, ERASE=DEL/S,   BC=BCHAR/S,  BW=BWORD/S,
// BL=BLINE/S,  BR=BRECT/S,  BF=BFULLRECT/S,ALL/S,       MC=MOVECURS/S,
// START/S,     STARTDO/S,   FOLDS/S,       REQUEST/S,   LINE/N,
// COL=COLUMN/N,PAGE/S,      REL=RELATIVE/S,WORD/N,      TAB/N,
// OF=OFFSET/N, UP/S,        LEFT/S,        W=WRAP/S,    ENDWORD/S,
// LINES/N,     CHARS/N,     PO=POINTER/S,  EC=EXTCURS/S,CONT=CONTINUE/S

void CmdFunc_Block_New( struct CmdObj *co )
{
    if (! co->ArgRes[2])    // ERASE
        {
        co->Flags &= ~AZAFF_DOMODIFY;
        }
}

void CmdFunc_Block_Do( struct CmdObj *co )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG line, col, endline, mode;
  ULONG error, id, pointer = (ULONG)co->ArgRes[27];
  struct AZBlock *azb;

    fo->Flags &= ~AZFILEF_CURSVISIBLE;

    if (decrypt_position( co, (LONG*)&co->ArgRes[12], &line, &col ))
        {
        LONG ext=0, startline, startcol, bline=line, bcol=col; // pour EXTCURS

        if (co->ArgRes[0])                      // CLIP
            {
            if (error = func_Block2CB( *(LONG*)co->ArgRes[0] ))
                { intAZ_SetCmdResult( co, error ); return; }
            }

        if (co->ArgRes[2])                      // ERASE
            {
            func_BlockEraseAll();
            }
        else if (co->ArgRes[1])                 // CLEAR
            {
            func_BlockClearAll();
            }

        if (co->ArgRes[8])                      // ALL
            {
            func_BlockClearAll();
            func_BlockStart( 0, 0, 0 );
            func_BlockDefine( fo->Text.NodeNum-1, 0, BLKMODE_LINE, 0 );
            func_ReDisplay();
            }
        else if ((co->ArgRes[10] || co->ArgRes[11]) && !co->ArgRes[2])     // START ou STARTDO mais pas ERASE
            {
            if (co->ArgRes[3]) fo->CurBlockMode = BLKMODE_CHAR;         // BCHAR
            else if (co->ArgRes[4]) fo->CurBlockMode = BLKMODE_WORD;    // BWORD
            else if (co->ArgRes[5]) fo->CurBlockMode = BLKMODE_LINE;    // BLINE
            else if (co->ArgRes[6]) fo->CurBlockMode = BLKMODE_RECT;    // BRECT
            else if (co->ArgRes[7]) fo->CurBlockMode = BLKMODE_FULL;    // BFULLRECT
            else fo->CurBlockMode = AGP.Prefs->BlockMode;
            if (TST_DEFAULTBLM && fo->CurBlockMode != BLKMODE_RECT && fo->CurBlockMode != BLKMODE_FULL)
                fo->CurBlockMode = BLKMODE_LINE;

            if (co->ArgRes[11])                 // STARTDO
                {
                id = func_BlockCurrent();
                if (func_BlockInside( id, line, col, 0, 0 ))
                    if (func_BlockRemove( id, &startline, &endline ))
                        func_Display( startline, endline - startline + 1, 0, -1 );
                }

            if (! func_BlockStart( line, col, pointer ))
                { intAZ_SetCmdResult( co, TEXT_BadPosition ); goto END_ERROR; }

            if (co->ArgRes[11])                 // STARTDO
                {
                if (!co->ArgRes[28] || (fo->CurBlockMode != BLKMODE_CHAR)) // ! EXTCURS
                    {
                    if (! (azb = func_BlockDefine( line, col, fo->CurBlockMode, 0/*pointer*/ )))
                        { intAZ_SetCmdResult( co, TEXT_BadPosition ); goto END_ERROR; }
                    if (azb == (APTR)-1) goto END_ERROR;
                    func_Display( azb->LineStart, azb->LineEnd - azb->LineStart + 1, 0, -1 );
                    func_BlockGetLimit( &line, &col );
                    }
                }

            if (co->ArgRes[9])                  // MOVECURS
                {
                func_CursorPosReal( line, col, (co->ExeFrom & FROMMOUSE)?0:1, 0 ); // à cause du déplacement intempestif de droite à gauche quand on trace des blocs
                }
            }
        else if (! (co->ArgRes[1] || co->ArgRes[2]))  // ni CLEAR ni ERASE
            {
            ULONG cont=0;

            func_BlockGetStart( &startline, &startcol );
            if (co->ArgRes[28] && (fo->CurBlockMode == BLKMODE_CHAR) && (line > startline || (line == startline && col > startcol)))
                { ext = 1; }                    // EXTCURS

            if (func_BlockCurrent()) cont = 1;
            else if (co->ArgRes[29] && func_BlockExists()) // CONTINUE
                {
                if (func_BlockContinue()) cont = 1;
                }

            if (cont)
                {
                if (ext) func_PosPrev( &bline, &bcol );
                if (! (azb = func_BlockModify( bline, bcol, &startline, &endline )))
                    { intAZ_SetCmdResult( co, TEXT_BadPosition ); goto END_ERROR; }

                func_Display( startline, endline - startline + 1, 0, -1 );
                }
            else{
                if (co->ArgRes[3]) mode = BLKMODE_CHAR;       // BCHAR
                else if (co->ArgRes[4]) mode = BLKMODE_WORD;  // BWORD
                else if (co->ArgRes[5]) mode = BLKMODE_LINE;  // BLINE
                else if (co->ArgRes[6]) mode = BLKMODE_RECT;  // BRECT
                else if (co->ArgRes[7]) mode = BLKMODE_FULL;  // BFULLRECT
                else mode = fo->CurBlockMode;
                if (! mode) mode = AGP.Prefs->BlockMode;
                if (TST_DEFAULTBLM && mode != BLKMODE_RECT && mode != BLKMODE_FULL)
                    mode = BLKMODE_LINE;

                if (mode != BLKMODE_CHAR) ext = 0;
                if (ext) func_PosPrev( &bline, &bcol );

                if (! (azb = func_BlockDefine( bline, bcol, mode, pointer )))
                    { intAZ_SetCmdResult( co, TEXT_BadPosition ); goto END_ERROR; }
                if (azb == (APTR)-1) goto END_ERROR;
                func_Display( azb->LineStart, azb->LineEnd - azb->LineStart + 1, 0, -1 );
                }

            if (co->ArgRes[9])                  // MOVECURS
                {
                if (! ext) func_BlockGetLimit( &line, &col );
                func_CursorPosReal( line, col, (co->ExeFrom & FROMMOUSE)?0:1, 0 ); // à cause du déplacement intempestif de droite à gauche quand on trace des blocs
                }
            }
        }
  END_ERROR:
    fo->Flags |= AZFILEF_CURSVISIBLE;
}

//»»»»»» Cmd_Block2CB       CLIP=UNIT/N

static ULONG clip_buf( APTR clip, UBYTE *buf, ULONG len, LONG line )
{
    if (len)
        {
        if (! func_CBWrite( clip, buf, len )) return( TEXT_ERR_WriteClip );
        AGP.Tmp += len;
        }
    return(0);
}

static ULONG bloc_clip( struct AZBlock *azb, ULONG id, APTR clip )
{
    return( func_TextApply( azb->LineStart, azb->ColStart, azb->LineEnd, azb->ColEnd,
                            (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) ? azb->Mode : 0,
                            0, clip_buf, clip ) );
}

ULONG func_Block2CB( ULONG unit )
{
  ULONG error;
  APTR clip;
  ULONG locks;

    if (! func_BlockExists()) return( TEXT_NoBlockDefined );
    if (unit > 255) return( TEXT_ERR_BadClipUnit );

    locks = func_VerifyOff();
    func_SetMainWaitPointer();
    if (clip = func_CBWriteStart( unit ))
        {
        AGP.Tmp = 0; // totlen
        error = func_BlockApply( bloc_clip, 1, -1, 0, clip );
        func_CBWriteEnd( clip, AGP.Tmp );
        }
    else error = TEXT_ERR_OpenClip;
    func_SetMainSystemPointer();
    func_VerifyOn( locks );

    return( error );
}

void CmdFunc_Block2CB_Do( struct CmdObj *co )
{
  ULONG error, unit = 0;

    if (co->ArgRes[0])
        unit = *(ULONG*)co->ArgRes[0];
    if (error = func_Block2CB( unit ))
        intAZ_SetCmdResult( co, error );
}

//»»»»»» Cmd_Win2CB       CLIP=UNIT/N

ULONG func_Win2CB( ULONG unit )
{
  ULONG error;
  APTR clip;
  ULONG locks;

    if (unit > 255) return( TEXT_ERR_BadClipUnit );

    locks = func_VerifyOff();
    func_SetMainWaitPointer();
    if (clip = func_CBWriteStart( unit ))
        {
        AGP.Tmp = 0; // totlen
        error = func_TextApply( 0, 0, -1, -1, 0, 0, clip_buf, clip );
        func_CBWriteEnd( clip, AGP.Tmp );
        }
    else error = TEXT_ERR_OpenClip;
    func_SetMainSystemPointer();
    func_VerifyOn( locks );

    return( error );
}

void CmdFunc_Win2CB_Do( struct CmdObj *co )
{
  ULONG error, unit = 0;

    if (co->ArgRes[0])
        unit = *(ULONG*)co->ArgRes[0];
    if (error = func_Win2CB( unit ))
        intAZ_SetCmdResult( co, error );
}

//»»»»»» Cmd_Block2File     NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,NAME,PAT=PATTERN/K

struct block2fileinfo {
    APTR file;
    ULONG flags;
    };

static ULONG write_buf( struct block2fileinfo *bfi, UBYTE *buf, ULONG len, LONG line )
{
    if (len)
        if (WriteAsync( bfi->file, buf, len ) < 0) return( TEXT_ERR_WRITEFILE );
    return(0);
}

static ULONG bloc_file( struct AZBlock *azb, ULONG id, APTR bfi )
{
    return( func_TextApply( azb->LineStart, azb->ColStart, azb->LineEnd, azb->ColEnd,
                            (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) ? azb->Mode : 0,
                            0, write_buf, bfi ) );
}

ULONG func_Block2File( APTR file, APTR parm )
{
  struct block2fileinfo BFI;
  ULONG error;

    if (! func_BlockExists()) return( TEXT_NoBlockDefined );
    BFI.file = file;
    BFI.flags = (ULONG)parm;
    if (! (error = func_BlockApply( bloc_file, 1, -1, 0, &BFI )))
    return( error );
}

void CmdFunc_Block2File_Do( struct CmdObj *co )
{
  UBYTE *filename;
  ULONG errmsg;
  ULONG locks, flags=0;

    if (TST_SAVELOCK) { err_cancel( co ); return; }
    locks = func_VerifyOff();
    func_SetMainWaitPointer();
                                                        // NAME ?
    if (! (filename = (UBYTE*)func_IntReqFileNameS( co->ArgRes[3], co->ArgRes[4], TEXT_REQ_Block2File, &errmsg, 1 ))) goto END_ERROR;
                                                        // NOBACKUP ? APPEND ?  pas d'icône
    errmsg = func_SaveFile( filename, (ULONG)co->ArgRes[0], (ULONG)co->ArgRes[2], 1, func_Block2File, (APTR)flags );
    if (errmsg==0 && co->ArgRes[1]==0 && TST_SAVEICON) func_SaveIcon( filename );

  END_ERROR:
    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    if (! errmsg) { update_file2fs( filename, 0, 1 ); }
    else intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_CB2File        CLIP=UNIT/K/N,NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,NAME,PAT=PATTERN/K

static ULONG func_CB2File( APTR file, ULONG unit )
{
  ULONG error=0, totlen, len;
  APTR mem, clip;
  ULONG blklen = AGP.Prefs->IOBufLength*1024;

    if (unit > 255) return( TEXT_ERR_BadClipUnit );
    if (mem = AllocMem( blklen, MEMF_ANY ))
        {
        if (clip = func_CBReadStart( unit, &totlen ))
            {
            while (totlen)
                {
                if (totlen > blklen) len = blklen; else len = totlen;
                if (func_CBRead( clip, mem, len ) != len) { error = TEXT_ERR_ReadClip; break; }
                if (WriteAsync( file, mem, len ) < 0) { error = TEXT_ERR_WRITEFILE; break; }
                totlen -= len;
                }
            func_CBReadEnd( clip );
            }
        else error = TEXT_ERR_OpenClip;
        FreeMem( mem, blklen );
        }
    else error = TEXT_NOMEMORY;

    return( error );
}

void CmdFunc_CB2File_Do( struct CmdObj *co )
{
  UBYTE *filename;
  ULONG errmsg, unit = 0;
  ULONG locks;

    if (TST_SAVELOCK) { err_cancel( co ); return; }
    locks = func_VerifyOff();
    func_SetMainWaitPointer();

    if (co->ArgRes[0])                                  // UNIT
        unit = *(ULONG*)co->ArgRes[0];
                                                        // NAME ?
    if (! (filename = (UBYTE*)func_IntReqFileNameS( co->ArgRes[4], co->ArgRes[5], TEXT_REQ_CB2File, &errmsg, 1 ))) goto END_ERROR;
                                                        // NOBACKUP ? APPEND ?  pas d'icône
    errmsg = func_SaveFile( filename, (ULONG)co->ArgRes[1], (ULONG)co->ArgRes[3], 1, func_CB2File, (APTR)unit );
    if (errmsg==0 && co->ArgRes[2]==0 && TST_SAVEICON) func_SaveIcon( filename );

  END_ERROR:
    func_SetMainSystemPointer();
    func_VerifyOn( locks );
    if (! errmsg) { update_file2fs( filename, 0, 1 ); }
    else intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_CB2Win         CLIP=UNIT/N,RECT=VERTICAL/S,REPL=OVERLAY/S,CC=CONVERT/S,NOCURS/S

void CmdFunc_CB2Win_Do( struct CmdObj *co )
{
  ULONG error = 0, totlen, len, unit = 0;
  APTR mem, clip;
  ULONG flags, blklen = AGP.Prefs->IOBufLength*1024;

    if (co->ArgRes[0])                  // UNIT
        unit = *(ULONG*)co->ArgRes[0];

    if (unit <= 255)
        {
        if (mem = AllocMem( blklen, MEMF_ANY ))
            {
            if (unit != AGP.Prefs->DelClip) func_Block2CB( AGP.Prefs->DelClip );
            func_BlockEraseAll();
            func_SetMainWaitPointer();
            if (clip = func_CBReadStart( unit, &totlen ))
                {
                flags = 0;
                if (co->ArgRes[1]) flags |= ARPF_RECT;         // RECT=VERTICAL
                if (co->ArgRes[2]) flags |= ARPF_OVERLAY;      // REPL=OVERLAY
                if (co->ArgRes[3]) flags |= ARPF_CONVERT;      // CC=CONVERT
                if (! (error = func_TextPrep( AGP.FObj->Line, AGP.FObj->Col, flags, 0, 0 )))
                    {
                    while (totlen)
                        {
                        if (totlen > blklen) len = blklen; else len = totlen;
                        if (func_CBRead( clip, mem, len ) != len) { error = TEXT_ERR_ReadClip; break; }
                        if (error = func_TextPut( mem, len, 0, 0 )) break;
                        totlen -= len;
                        }
                    func_TextPut( 0, 0, 0, 1 ); // dessine l'ensemble
                    }
                func_CBReadEnd( clip );
                if (! co->ArgRes[4])
                    {
                    func_CursorPos( AGP.FObj->WorkLine, AGP.FObj->WorkCol );
                    // func_BlockStart( AGP.FObj->WorkLine, AGP.FObj->WorkCol, 0 );
                    AGP.FObj->CEdCol = AGP.FObj->WorkCol;
                    }
                else func_TextPrep( AGP.FObj->Line, AGP.FObj->Col, flags, 0, 0 );
                }
            else error = TEXT_ERR_OpenClip;
            func_SetMainSystemPointer();
            FreeMem( mem, blklen );
            }
        else error = TEXT_NOMEMORY;
        }
    else error = TEXT_ERR_BadClipUnit;

    if (error) intAZ_SetCmdResult( co, error );
    else func_AutoFormat();
}

void CmdFunc_CursLastChange_Do( struct CmdObj *co )
{
    func_CursorPos( AGP.FObj->LCLine, AGP.FObj->LCCol );
}

//»»»»»» Cmd_BMAdd      RESET/S,BLOCK/S,NAME=BOOKMARK,CLEAR=OFF/S,MC=MOVECURS/S,DELPROOF/S,FOLDS/S,REQ=REQUEST/S,LINE/N,COL=COLUMN/N,PAGE/S,REL=RELATIVE/S,WORD/N,TAB/N,OF=OFFSET/N,UP/S,LEFT/S,W=WRAP/S,LINES/N,CHARS/N,PO=POINTER/S

// static void BM_copy_name( int nil, UBYTE *selected )
// {
//     BufSetS( selected, StrLen(selected), &AGP.UBufTmp );
//}

ULONG BM_apply_2_buftmp( struct AZBM *azb, ULONG id, UIKBUF **pub ) // utilisée dans funcs_reqs.c
{
  ULONG errmsg = 0;

    if (! func_BufPrintf( pub, BufLength(*pub), "%ls    (%ld,%ld)\n", azb->Name, azb->Line+1, azb->Col+1 ))
        errmsg = TEXT_NOMEMORY;
    return( errmsg );
}

static ULONG trunc_BM_name( int nil, UBYTE *buf )
{
  LONG len, i;

    if (len = StrLen( buf ))
        {
        for (i=len-1; i >= 2; i--)
            if (buf[i] == '(' && buf[i-1] == ' ' && buf[i-2] == ' ') { len = i-2; break; }
        }
    return( (ULONG)len );
}

ULONG BM_sel_2_buftmp( struct AZRequest *ar, UBYTE *name )
{
  ULONG len;

    BufTruncate( AGP.UBufTmp, 0 );
    if (len = trunc_BM_name( 0, name ))
        {
        if (BufSetS( name, len, &AGP.UBufTmp ))
            {
            return(1);
            }
        }
    return(0);
}

static void BM_ok( int nil, UBYTE *name, struct AZRequest *ar )
{
    BM_sel_2_buftmp( ar, name );
    func_ReqClose( 0, ar );
}

void BM_cancel( int nil, struct AZRequest *ar )
{
    func_ReqClose( 0, ar );
}

void BM_update_palette( void )
{
  struct AZRequest *ar = &AGP.AZReqBookmark;
  ULONG errmsg;

    if (ar->Obj)
        {
        BufTruncate( AGP.UBufTmp, 0 );
        errmsg = func_BMApply( BM_apply_2_buftmp, 1, -1, 0, &AGP.UBufTmp );
        if (errmsg == 0 || errmsg == TEXT_NoBMDefined)
            {
            struct UIKObjList *list = (APTR)ar->Obj->UserULong1;
            WORD pos = list->Pos;
            UIK_Stop( list );
            UIK_OLi_NewTextPtr( list, AGP.UBufTmp );
            UIK_OLi_SetListTop( list, pos );
            UIK_Start( list );
            }
        }
}

/* dans le cas de BMAdd, le BM n'existe pas encore donc le id retourné
 * sera égal à 0 sans indiquer d'erreur.
 */
static UBYTE *get_BM_name_id( UBYTE *name, ULONG *pid, ULONG *resid, ULONG liststrgad )
{
  struct AZBM *azb;
  ULONG id, errmsg;

    if (name && *name)  // NAME
        {
        id = func_BMGetId( name, 0, 0 );
        }
    else if (pid) // NUM
        {
        id = *pid;
        if (azb = (struct AZBM *) func_BMGetPtr( id )) name = azb->Name;
        }
    else // if (list || func_BMExists()) // (option LIST) ou (liststrgad))
        {
        struct AZRequest AR;
        __builtin_memset( &AR, 0, sizeof(struct AZRequest) );
        BufTruncate( AGP.UBufTmp, 0 );
        errmsg = func_BMApply( BM_apply_2_buftmp, 1, -1, 0, &AGP.UBufTmp );
        if (errmsg == 0 || errmsg == TEXT_NoBMDefined)
            {                 // synchrone
            name = 0;
            errmsg = func_ReqList( &AR,
                ART_ReqTitle,       UIK_LangString( AGP.UIK, (liststrgad ? TEXT_Title_BMName : TEXT_Title_Bookmarks) ),
                ART_ReqFlags,       ARF_WINDOW | (liststrgad ? ARF_STRGAD|ARF_ACTIVATE : 0), // synchrone
                ART_OKFunc,         BM_ok,
                ART_CancelFunc,     BM_cancel,
                ART_ListInit,       AGP.UBufTmp,
                ART_List2StrFunc,   trunc_BM_name,
                TAG_END );
            if (AR.ErrMsg == 0 && AR.Button == REQBUT_OK)
                {
                id = func_BMGetId( name = AGP.UBufTmp, 0, 0 );
                }
            }
        }
    if (resid) *resid = (name ? id : 0);
    return( name );
}

void CmdFunc_BMAdd_Do( struct CmdObj *co )
{
  LONG line, col;
  ULONG errmsg=0;
  UBYTE *name;
  struct AZBM *azb;


    if (co->ArgRes[0]) func_BMRemAll(); // RESET

    if (co->ArgRes[1])      // BLOCK
        {
        BufTruncate( AGP.UBufTmp, 0 );
        if (errmsg = func_Block2Buf( &AGP.UBufTmp, 1 )) goto END_ERROR;
        name = AGP.UBufTmp;
        }
    else{
        if (! (name = get_BM_name_id( co->ArgRes[2], 0, 0, 1 ))) { errmsg = TEXT_CMDCANCELLED; goto END_ERROR; }
        }

    if (decrypt_position( co, (LONG*)&co->ArgRes[6], &line, &col ) == 0) return;
    func_BMRemove( name, 0 ); // efface si le nom existe déjà
    if (! (azb = func_BMAdd( name, line, col ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }

    if (co->ArgRes[3]) func_BlockClearAll();
    if (co->ArgRes[4]) func_CursorPos( line, col );
    if (co->ArgRes[5]) azb->Flags |= AZBMF_DELPROOF;

  END_ERROR:
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_BMCursor   NAME=BOOKMARK,NUM=ID/N/K,CLEAR=OFF/S

void CmdFunc_BMCursor_Do( struct CmdObj *co )
{
  struct AZBM *azb;
  ULONG id, errmsg=0;

    if (! func_BMExists()) { errmsg = TEXT_NoBMDefined; goto END_ERROR; }

    get_BM_name_id( co->ArgRes[0], co->ArgRes[1], &id, 0 );
    if (! id) { errmsg = TEXT_CMDCANCELLED; goto END_ERROR; }

    if (azb = (struct AZBM *) func_BMGetPtr( id ))
        {
        if (co->ArgRes[2]) func_BlockClearAll();
        func_CursorPos( azb->Line, azb->Col );
        }

  END_ERROR:
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_BMRemove   NAME=BOOKMARK,NUM=ID/N/K,ALL/S,PAT=PATTERN/K

static ULONG BM_remove_pat( struct AZBM *azb, ULONG id, UBYTE *pattern )
{
  UBYTE buf[210];

    UIK_CmplPattern( pattern, buf, 200 );
    if (UIK_Match( pattern, buf, azb->Name )) func_BMRemove( 0, id );
    return(0);
}

void CmdFunc_BMRemove_Do( struct CmdObj *co )
{
  ULONG id, errmsg=0;

    if (! func_BMExists()) { errmsg = TEXT_NoBMDefined; goto END_ERROR; }

    if (co->ArgRes[2])      // ALL
        {
        func_BMRemAll();
        }
    else if (co->ArgRes[3]) // PATTERN
        {
        errmsg = func_BMApply( BM_remove_pat, 0, -1, 1, co->ArgRes[3] );
        }
    else{
        get_BM_name_id( co->ArgRes[0], co->ArgRes[1], &id, 0 );
        if (! id) { errmsg = TEXT_CMDCANCELLED; goto END_ERROR; }
        func_BMRemove( 0, id );
        }
  END_ERROR:
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}
