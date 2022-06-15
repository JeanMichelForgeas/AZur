#ifndef  AZUR_PROTOS_H
#define  AZUR_PROTOS_H
/*
 *      Copyright (C) 1993 Jean-Michel Forgeas
 *              Tous Droits R�serv�s
 */

ULONG func_LoadFile( STRPTR filename );
ULONG func_ResetFile( void );
ULONG func_SaveFile( STRPTR filename, ULONG nobackup, ULONG append, ULONG noicon, ULONG (*res1)(), APTR res2 );
void func_CursorPos( LONG line, LONG col  );
LONG func_FindIndentPos( LONG line, ULONG indtype );
ULONG func_Buf2Text( void );
LONG func_TextDel( LONG line, LONG col, LONG chars, LONG lines, struct AZDisplayCmd *dinfo );
LONG func_TextDelRect( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, struct AZDisplayCmd *dinfo );
ULONG func_SplitLine( LONG line, LONG col );
ULONG func_JoinLine( LONG line, LONG col );
STRPTR func_C2Bin( STRPTR src, ULONG len );
UBYTE func_CharCurrent( LONG col, ULONG len, STRPTR ptr );
STRPTR func_CharPrev( STRPTR ptr, LONG *plen, LONG *pline, LONG *pcol, STRPTR pch );
STRPTR func_CharNext( STRPTR ptr, LONG *plen, LONG *pline, LONG *pcol, STRPTR pch );
STRPTR func_CharIsWord( UBYTE ch );
ULONG func_NumLines( void );
STRPTR func_LineBuf( LONG line );
ULONG func_LineLen( LONG line );
ULONG func_LineBufLen( STRPTR buf );
STRPTR func_BufAlloc( ULONG len );
void func_BufFree( STRPTR buf );
STRPTR func_BufPrintf( STRPTR *pbuf, ULONG start, STRPTR fmt, ... );
ULONG func_BlockApply( ULONG (*func)(), ULONG from, ULONG to, ULONG reverse, APTR parm );
ULONG func_BlockCurrent( void );
struct AZBlock * func_BlockDefine( LONG lineend, LONG colend, ULONG mode, ULONG pointer );
ULONG func_BlockExists( void );
ULONG func_BlockInside( ULONG fromid, LONG line, LONG col, struct AZBlock **pazb, ULONG rect );
ULONG func_BlockLineInside( ULONG fromid, LONG line, struct AZBlock **pazb );
struct AZBlock * func_BlockModify( LONG lineend, LONG colend, LONG *pstartline, LONG *pendline );
struct AZBlock * func_BlockPtr( ULONG id );
ULONG func_BlockRemAll( LONG *pfirstline, LONG *plastline );
ULONG func_BlockRemove( ULONG id, LONG *pfirstline, LONG *plastline );
ULONG func_BlockStart( LONG line, LONG col, ULONG pointer );
ULONG func_BlockEraseAll( void );
ULONG func_BlockClearAll( void );
ULONG func_TextApply( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, ULONG reverse, ULONG (*func)(), APTR parm );
ULONG func_TextPrep( ULONG startline, ULONG startcol, ULONG flags, ULONG maxlines, ULONG indtype );
ULONG func_TextPut( STRPTR src, ULONG srclen, ULONG flags, ULONG refresh );
ULONG func_TextSearch( struct SearchRepQuery *query );
ULONG func_TextReplace( struct SearchRepQuery *query );
STRPTR func_PosPrev( LONG *pline, LONG *pcol );
STRPTR func_PosNext( LONG *pline, LONG *pcol );
ULONG func_TextApplyReal( LONG sline, LONG scol, LONG eline, LONG ecol, ULONG rect, ULONG reverse, ULONG (*func)(), APTR parm );
ULONG func_TextSort( LONG sline, LONG eline, LONG scol, LONG ecol, ULONG grplines, ULONG grpoffset, int (*cmpfunc)(), ULONG flags, struct AZDisplayCmd *dinfo );
ULONG func_Block2Buf( STRPTR *puikbuf, ULONG clear );
ULONG func_BMExists( void );
struct AZBM * func_BMGetPtr( ULONG id );
struct AZBM * func_BMAdd( STRPTR name, LONG line, LONG col );
ULONG func_BMInside( ULONG fromid, LONG line, struct AZBM **pazb );
ULONG func_BMApply( ULONG (*func)(), ULONG from, ULONG to, ULONG reverse, APTR parm );
ULONG func_BMGetId( STRPTR name, LONG line, LONG col );
ULONG func_BMRemove( STRPTR name, ULONG id );
ULONG func_BMRemAll( void );
void func_SetWaitPointer( struct Window *w );
void func_SetSystemPointer( struct Window *w );
void func_ReDisplay( void );
void func_DisplayStats( ULONG title );
void func_DisplayBlock( void );
void func_Display( LONG startline, LONG nlines, LONG startcol, LONG ncols );
void func_ResetProps( void );
void func_DisplayCmd( struct AZDisplayCmd *dcmd );
void func_Scroll( ULONG vert, LONG len, LONG from );
void func_CursorErase( void );
void func_CursorDraw( void );
ULONG func_AskReqBegin( STRPTR text, STRPTR title );
ULONG func_AskReqQuery( ULONG handle );
void func_AskReqEnd( ULONG handle );
void func_ReqShowText( STRPTR text );
ULONG func_SmartRequest( STRPTR titleptr, STRPTR textptr, STRPTR title1, STRPTR *buf1, STRPTR title2, STRPTR *buf2, STRPTR ok, STRPTR mid, STRPTR can, ULONG (*AddFunc)(), ULONG rsflags );
ULONG func_ReqNumber( ULONG *presult );
ULONG func_ReqString( STRPTR title, STRPTR text, STRPTR *presult );
STRPTR func_ReqFileNameO( STRPTR givenname, STRPTR pattern, STRPTR wintitle, ULONG *errcode );
STRPTR func_ReqFileNameS( STRPTR givenname, STRPTR pattern, STRPTR wintitle, ULONG *errcode );
ULONG func_ReqList( struct AZRequest *ar, ULONG tag, ... );
ULONG func_ReqOpen( STRPTR reqname, ULONG *button );
void func_ReqClose( STRPTR reqname, struct AZRequest *ar );
struct CmdGerm * func_LoadGerm( STRPTR name );
ULONG func_UnloadGerm( STRPTR name );
struct CmdObj * func_NewObject( STRPTR name, struct TagItem *objtaglist );
void func_DisposeObject( struct CmdObj *co );
ULONG func_DoAction( struct CmdObj *co, UWORD type, ULONG parm );
ULONG func_NewAllActions( struct CmdGerm *cg );
ULONG func_NewAction( struct CmdGerm *cg );
void func_DeleteAction( struct CmdGerm *cg, UWORD type );
ULONG func_NewActionFunc( struct CmdGerm *cg, UWORD type, struct CmdFunc *cf, ULONG end );
ULONG func_LaunchProcess( ULONG tag, ... );
void func_SetCmdResult( struct CmdObj *co, ULONG rc, WORD error, STRPTR msg );
void func_SetQualifier( ULONG qual, ULONG on );
LONG func_ComputeLineCol( LONG *arg, LONG *pline, LONG *pcol );
ULONG func_ExecuteAZurCmd( STRPTR cmd, ULONG actionparm );
ULONG func_ExecuteCmdObj( struct CmdObj *co, ULONG actionparm );
APTR func_VarNew( STRPTR name, ULONG size );
APTR func_VarLock( STRPTR name );
void func_VarUnlock( STRPTR name );
void func_VarDispose( STRPTR name );
ULONG func_CodeText( STRPTR key, STRPTR string );
void func_DecodeText( STRPTR key, STRPTR string, ULONG length );
struct CmdGerm * func_FindRegGerm( STRPTR name );
struct AZurProc * func_GetAZur( void );
APTR func_VarLockResize( STRPTR name, ULONG newsize );
void func_EnvFree( APTR getresult );
APTR func_EnvGet( STRPTR prefname );
ULONG func_EnvSet( STRPTR prefname, APTR pref, ULONG prefsize );
ULONG func_MacroExists( void );
void func_MacroReset( void );
void func_MacroDelete( void );
ULONG func_MacroAdd( STRPTR cmdname, STRPTR cmdline );
ULONG func_MacroPlay( ULONG num, ULONG chgptr );
ULONG func_MacroApply( ULONG (*func)(), APTR parm );
ULONG func_MacroRecord( ULONG quiet, ULONG chgptr );
void func_MacroEndRecord( void );
ULONG func_Macro2Buf( STRPTR *pub );
ULONG func_Buf2Macro( STRPTR *pub, STRPTR *errorname );
void func_SetModified( LONG line, LONG col );

#endif   /* AZUR_PROTOS_H */
