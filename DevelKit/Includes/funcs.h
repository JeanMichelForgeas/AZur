#ifndef  AZUR_JUMP_H
#define  AZUR_JUMP_H
/*
 *      Copyright (C) 1993 Jean-Michel Forgeas
 *              Tous Droits Réservés
 */

struct _jump
    {
    ULONG Dummy;
    ULONG (*LoadFile)( STRPTR filename );
    ULONG (*ResetFile)( void );
    ULONG (*SaveFile)( STRPTR filename, ULONG nobackup, ULONG append, ULONG noicon, ULONG (*res1)(), APTR res2 );
    void (*CursorPos)( LONG line, LONG col  );
    LONG (*FindIndentPos)( LONG line, ULONG indtype );
    ULONG (*Buf2Text)( void );
    LONG (*TextDel)( LONG line, LONG col, LONG chars, LONG lines, struct AZDisplayCmd *dinfo );
    LONG (*TextDelRect)( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, struct AZDisplayCmd *dinfo );
    ULONG (*SplitLine)( LONG line, LONG col );
    ULONG (*JoinLine)( LONG line, LONG col );
    STRPTR (*C2Bin)( STRPTR src, ULONG len );
    UBYTE (*CharCurrent)( LONG col, ULONG len, STRPTR ptr );
    STRPTR (*CharPrev)( STRPTR ptr, LONG *plen, LONG *pline, LONG *pcol, STRPTR pch );
    STRPTR (*CharNext)( STRPTR ptr, LONG *plen, LONG *pline, LONG *pcol, STRPTR pch );
    STRPTR (*CharIsWord)( UBYTE ch );
    ULONG (*NumLines)( void );
    STRPTR (*LineBuf)( LONG line );
    ULONG (*LineLen)( LONG line );
    ULONG (*LineBufLen)( STRPTR buf );
    STRPTR (*BufAlloc)( ULONG len );
    void (*BufFree)( STRPTR buf );
    STRPTR (*BufPrintf)( STRPTR *pbuf, ULONG start, STRPTR fmt, ... );
    ULONG (*BlockApply)( ULONG (*func)(), ULONG from, ULONG to, ULONG reverse, APTR parm );
    ULONG (*BlockCurrent)( void );
    struct AZBlock * (*BlockDefine)( LONG lineend, LONG colend, ULONG mode, ULONG pointer );
    ULONG (*BlockExists)( void );
    ULONG (*BlockInside)( ULONG fromid, LONG line, LONG col, struct AZBlock **pazb, ULONG rect );
    ULONG (*BlockLineInside)( ULONG fromid, LONG line, struct AZBlock **pazb );
    struct AZBlock * (*BlockModify)( LONG lineend, LONG colend, LONG *pstartline, LONG *pendline );
    struct AZBlock * (*BlockPtr)( ULONG id );
    ULONG (*BlockRemAll)( LONG *pfirstline, LONG *plastline );
    ULONG (*BlockRemove)( ULONG id, LONG *pfirstline, LONG *plastline );
    ULONG (*BlockStart)( LONG line, LONG col, ULONG pointer );
    ULONG (*BlockEraseAll)( void );
    ULONG (*BlockClearAll)( void );
    ULONG (*TextApply)( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, ULONG reverse, ULONG (*func)(), APTR parm );
    ULONG (*TextPrep)( ULONG startline, ULONG startcol, ULONG flags, ULONG maxlines, ULONG indtype );
    ULONG (*TextPut)( STRPTR src, ULONG srclen, ULONG flags, ULONG refresh );
    ULONG (*TextSearch)( struct SearchRepQuery *query );
    ULONG (*TextReplace)( struct SearchRepQuery *query );
    STRPTR (*PosPrev)( LONG *pline, LONG *pcol );
    STRPTR (*PosNext)( LONG *pline, LONG *pcol );
    ULONG (*TextApplyReal)( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, ULONG reverse, ULONG (*func)(), APTR parm );
    ULONG (*TextSort)( LONG sline, LONG eline, LONG scol, LONG ecol, ULONG grplines, ULONG grpoffset, int (*cmpfunc)(), ULONG flags, struct AZDisplayCmd *dinfo );
    ULONG (*Block2Buf)( STRPTR *puikbuf, ULONG clear );
    ULONG (*BMExists)( void );
    struct AZBM * (*BMGetPtr)( ULONG id );
    struct AZBM * (*BMAdd)( STRPTR name, LONG line, LONG col );
    ULONG (*BMInside)( ULONG fromid, LONG line, struct AZBM **pazb );
    ULONG (*BMApply)( ULONG (*func)(), ULONG from, ULONG to, ULONG reverse, APTR parm );
    ULONG (*BMGetId)( STRPTR name, LONG line, LONG col );
    ULONG (*BMRemove)( STRPTR name, ULONG id );
    ULONG (*BMRemAll)( void );
    ULONG Reserved1[4];
    void (*SetWaitPointer)( struct Window *w );
    void (*SetSystemPointer)( struct Window *w );
    void (*ReDisplay)( void );
    void (*DisplayStats)( ULONG title );
    void (*DisplayBlock)( void );
    void (*Display)( LONG startline, LONG nlines, LONG startcol, LONG ncols );
    void (*ResetProps)( void );
    void (*DisplayCmd)( struct AZDisplayCmd *dcmd );
    void (*Scroll)( ULONG vert, LONG len, LONG from );
    void (*CursorErase)( void );
    void (*CursorDraw)( void );
    ULONG Reserved2[21];
    ULONG (*AskReqBegin)( STRPTR text, STRPTR title );
    ULONG (*AskReqQuery)( ULONG handle );
    void (*AskReqEnd)( ULONG handle );
    void (*ReqShowText)( STRPTR text );
    ULONG (*SmartRequest)( STRPTR titleptr, STRPTR textptr, STRPTR title1, STRPTR *buf1, STRPTR title2, STRPTR *buf2, STRPTR ok, STRPTR mid, STRPTR can, ULONG (*AddFunc)(), ULONG rsflags );
    ULONG (*ReqNumber)( ULONG *presult );
    ULONG (*ReqString)( STRPTR title, STRPTR text, STRPTR *presult );
    STRPTR (*ReqFileNameO)( STRPTR givenname, STRPTR pattern, STRPTR wintitle, ULONG *errcode );
    STRPTR (*ReqFileNameS)( STRPTR givenname, STRPTR pattern, STRPTR wintitle, ULONG *errcode );
    ULONG (*ReqList)( struct AZRequest *ar, ULONG tag, ... );
    ULONG (*ReqOpen)( STRPTR reqname, ULONG *button );
    void (*ReqClose)( STRPTR reqname, struct AZRequest *ar );
    ULONG Reserved3[25];
    struct CmdGerm * (*LoadGerm)( STRPTR name );
    ULONG (*UnloadGerm)( STRPTR name );
    struct CmdObj * (*NewObject)( STRPTR name, struct TagItem *objtaglist );
    void (*DisposeObject)( struct CmdObj *co );
    ULONG (*DoAction)( struct CmdObj *co, UWORD type, ULONG parm );
    ULONG (*NewAllActions)( struct CmdGerm *cg );
    ULONG (*NewAction)( struct CmdGerm *cg );
    void (*DeleteAction)( struct CmdGerm *cg, UWORD type );
    ULONG (*NewActionFunc)( struct CmdGerm *cg, UWORD type, struct CmdFunc *cf, ULONG end );
    ULONG (*LaunchProcess)( ULONG tag, ... );
    void (*SetCmdResult)( struct CmdObj *co, ULONG rc, WORD error, STRPTR msg );
    void (*SetQualifier)( ULONG qual, ULONG on );
    LONG (*ComputeLineCol)( LONG *arg, LONG *pline, LONG *pcol );
    ULONG (*ExecuteAZurCmd)( STRPTR cmd, ULONG actionparm );
    ULONG (*ExecuteCmdObj)( struct CmdObj *co, ULONG actionparm );
    APTR (*VarNew)( STRPTR name, ULONG size );
    APTR (*VarLock)( STRPTR name );
    void (*VarUnlock)( STRPTR name );
    void (*VarDispose)( STRPTR name );
    ULONG (*CodeText)( STRPTR key, STRPTR string );
    void (*DecodeText)( STRPTR key, STRPTR string, ULONG length );
    struct CmdGerm * (*FindRegGerm)( STRPTR name );
    struct AZurProc * (*GetAZur)( void );
    APTR (*VarLockResize)( STRPTR name, ULONG newsize );
    void (*EnvFree)( APTR getresult );
    APTR (*EnvGet)( STRPTR prefname );
    ULONG (*EnvSet)( STRPTR prefname, APTR pref, ULONG prefsize );
    ULONG Reserved4[1];
    ULONG (*MacroExists)( void );
    void (*MacroReset)( void );
    void (*MacroDelete)( void );
    ULONG (*MacroAdd)( STRPTR cmdname, STRPTR cmdline );
    ULONG (*MacroPlay)( ULONG num, ULONG chgptr );
    ULONG (*MacroApply)( ULONG (*func)(), APTR parm );
    ULONG (*MacroRecord)( ULONG quiet, ULONG chgptr );
    void (*MacroEndRecord)( void );
    ULONG (*Macro2Buf)( STRPTR *pub );
    ULONG (*Buf2Macro)( STRPTR *pub, STRPTR *errorname );
    void (*SetModified)( LONG line, LONG col );
    };

#endif   /* AZUR_JUMP_H */

#ifndef  AZUR_FUNCS_H
#define  AZUR_FUNCS_H
/*
 *      Copyright (C) 1993 Jean-Michel Forgeas
 *              Tous Droits Réservés
 */

#ifdef _DCC
extern struct _jump * __stkargs _linkazur(void);
#else
extern struct _jump * __stdargs _linkazur(void);
#endif

#define AZFuncs ((struct _jump *)_linkazur())

#define AZ_LoadFile AZFuncs->LoadFile
#define AZ_ResetFile AZFuncs->ResetFile
#define AZ_SaveFile AZFuncs->SaveFile
#define AZ_CursorPos AZFuncs->CursorPos
#define AZ_FindIndentPos AZFuncs->FindIndentPos
#define AZ_Buf2Text AZFuncs->Buf2Text
#define AZ_TextDel AZFuncs->TextDel
#define AZ_TextDelRect AZFuncs->TextDelRect
#define AZ_SplitLine AZFuncs->SplitLine
#define AZ_JoinLine AZFuncs->JoinLine
#define AZ_C2Bin AZFuncs->C2Bin
#define AZ_CharCurrent AZFuncs->CharCurrent
#define AZ_CharPrev AZFuncs->CharPrev
#define AZ_CharNext AZFuncs->CharNext
#define AZ_CharIsWord AZFuncs->CharIsWord
#define AZ_NumLines AZFuncs->NumLines
#define AZ_LineBuf AZFuncs->LineBuf
#define AZ_LineLen AZFuncs->LineLen
#define AZ_LineBufLen AZFuncs->LineBufLen
#define AZ_BufAlloc AZFuncs->BufAlloc
#define AZ_BufFree AZFuncs->BufFree
#define AZ_BufPrintf AZFuncs->BufPrintf
#define AZ_BlockApply AZFuncs->BlockApply
#define AZ_BlockCurrent AZFuncs->BlockCurrent
#define AZ_BlockDefine AZFuncs->BlockDefine
#define AZ_BlockExists AZFuncs->BlockExists
#define AZ_BlockInside AZFuncs->BlockInside
#define AZ_BlockLineInside AZFuncs->BlockLineInside
#define AZ_BlockModify AZFuncs->BlockModify
#define AZ_BlockPtr AZFuncs->BlockPtr
#define AZ_BlockRemAll AZFuncs->BlockRemAll
#define AZ_BlockRemove AZFuncs->BlockRemove
#define AZ_BlockStart AZFuncs->BlockStart
#define AZ_BlockEraseAll AZFuncs->BlockEraseAll
#define AZ_BlockClearAll AZFuncs->BlockClearAll
#define AZ_TextApply AZFuncs->TextApply
#define AZ_TextPrep AZFuncs->TextPrep
#define AZ_TextPut AZFuncs->TextPut
#define AZ_TextSearch AZFuncs->TextSearch
#define AZ_TextReplace AZFuncs->TextReplace
#define AZ_PosPrev AZFuncs->PosPrev
#define AZ_PosNext AZFuncs->PosNext
#define AZ_TextApplyReal AZFuncs->TextApplyReal
#define AZ_TextSort AZFuncs->TextSort
#define AZ_Block2Buf AZFuncs->Block2Buf
#define AZ_BMExists AZFuncs->BMExists
#define AZ_BMGetPtr AZFuncs->BMGetPtr
#define AZ_BMAdd AZFuncs->BMAdd
#define AZ_BMInside AZFuncs->BMInside
#define AZ_BMApply AZFuncs->BMApply
#define AZ_BMGetId AZFuncs->BMGetId
#define AZ_BMRemove AZFuncs->BMRemove
#define AZ_BMRemAll AZFuncs->BMRemAll
#define AZ_SetWaitPointer AZFuncs->SetWaitPointer
#define AZ_SetSystemPointer AZFuncs->SetSystemPointer
#define AZ_ReDisplay AZFuncs->ReDisplay
#define AZ_DisplayStats AZFuncs->DisplayStats
#define AZ_DisplayBlock AZFuncs->DisplayBlock
#define AZ_Display AZFuncs->Display
#define AZ_ResetProps AZFuncs->ResetProps
#define AZ_DisplayCmd AZFuncs->DisplayCmd
#define AZ_Scroll AZFuncs->Scroll
#define AZ_CursorErase AZFuncs->CursorErase
#define AZ_CursorDraw AZFuncs->CursorDraw
#define AZ_AskReqBegin AZFuncs->AskReqBegin
#define AZ_AskReqQuery AZFuncs->AskReqQuery
#define AZ_AskReqEnd AZFuncs->AskReqEnd
#define AZ_ReqShowText AZFuncs->ReqShowText
#define AZ_SmartRequest AZFuncs->SmartRequest
#define AZ_ReqNumber AZFuncs->ReqNumber
#define AZ_ReqString AZFuncs->ReqString
#define AZ_ReqFileNameO AZFuncs->ReqFileNameO
#define AZ_ReqFileNameS AZFuncs->ReqFileNameS
#define AZ_ReqList AZFuncs->ReqList
#define AZ_ReqOpen AZFuncs->ReqOpen
#define AZ_ReqClose AZFuncs->ReqClose
#define AZ_LoadGerm AZFuncs->LoadGerm
#define AZ_UnloadGerm AZFuncs->UnloadGerm
#define AZ_NewObject AZFuncs->NewObject
#define AZ_DisposeObject AZFuncs->DisposeObject
#define AZ_DoAction AZFuncs->DoAction
#define AZ_NewAllActions AZFuncs->NewAllActions
#define AZ_NewAction AZFuncs->NewAction
#define AZ_DeleteAction AZFuncs->DeleteAction
#define AZ_NewActionFunc AZFuncs->NewActionFunc
#define AZ_LaunchProcess AZFuncs->LaunchProcess
#define AZ_SetCmdResult AZFuncs->SetCmdResult
#define AZ_SetQualifier AZFuncs->SetQualifier
#define AZ_ComputeLineCol AZFuncs->ComputeLineCol
#define AZ_ExecuteAZurCmd AZFuncs->ExecuteAZurCmd
#define AZ_ExecuteCmdObj AZFuncs->ExecuteCmdObj
#define AZ_VarNew AZFuncs->VarNew
#define AZ_VarLock AZFuncs->VarLock
#define AZ_VarUnlock AZFuncs->VarUnlock
#define AZ_VarDispose AZFuncs->VarDispose
#define AZ_CodeText AZFuncs->CodeText
#define AZ_DecodeText AZFuncs->DecodeText
#define AZ_FindRegGerm AZFuncs->FindRegGerm
#define AZ_GetAZur AZFuncs->GetAZur
#define AZ_VarLockResize AZFuncs->VarLockResize
#define AZ_EnvFree AZFuncs->EnvFree
#define AZ_EnvGet AZFuncs->EnvGet
#define AZ_EnvSet AZFuncs->EnvSet
#define AZ_MacroExists AZFuncs->MacroExists
#define AZ_MacroReset AZFuncs->MacroReset
#define AZ_MacroDelete AZFuncs->MacroDelete
#define AZ_MacroAdd AZFuncs->MacroAdd
#define AZ_MacroPlay AZFuncs->MacroPlay
#define AZ_MacroApply AZFuncs->MacroApply
#define AZ_MacroRecord AZFuncs->MacroRecord
#define AZ_MacroEndRecord AZFuncs->MacroEndRecord
#define AZ_Macro2Buf AZFuncs->Macro2Buf
#define AZ_Buf2Macro AZFuncs->Buf2Macro
#define AZ_SetModified AZFuncs->SetModified

#endif   /* AZUR_FUNCS_H */
