extern void intAZ_SetCmdResult( struct CmdObj *co, ULONG msg_index );
extern UBYTE __asm intAZ_CharCurrent( register __d0 LONG col, register __d1 ULONG len, register __a0 UBYTE *ptr );
extern ULONG __asm intAZ_GetErrorSeverity( register __d0 WORD error );
extern UBYTE * __asm intAZ_LineBuf( register __d0 LONG line );
extern ULONG __asm intAZ_LineLen( register __d0 LONG line );
extern ULONG __asm intAZ_LineBufLen( register __a0 UBYTE *ubuf );
extern ULONG __asm intAZ_Buf_Len( register __d0 LONG line, register __a0 UBYTE **pptr );
extern ULONG __asm intAZ_Node_Buf_Len( register __d0 LONG line, register __a0 struct UIKNode *node, register __a1 UBYTE **pptr );
extern ULONG __asm intAZ_IsSubStr( register __a0 UBYTE *sub, register __a1 UBYTE *str );
extern ULONG __asm intAZ_IsNCSubStr( register __a0 UBYTE *sub, register __a1 UBYTE *str );
extern ULONG __asm intAZ_StrNCEqual( register __a0 UBYTE *s1, register __a1 UBYTE *s2 );

extern UWORD __asm intAZ_GetLineFlags( register __d0 LONG line );
extern void __asm intAZ_SetLineFlags( register __d0 LONG line, register __d1 ULONG flags );
extern void __asm intAZ_ORLineFlags( register __d0 LONG line, register __d1 ULONG flags );
extern void __asm intAZ_ANDLineFlags( register __d0 LONG line, register __d1 ULONG flags );

extern UWORD __asm intAZ_GetBufFlags( register __a0 UBYTE *ubuf );
extern void __asm intAZ_SetBufFlags( register __a0 UBYTE *ubuf, register __d1 ULONG flags );
extern void __asm intAZ_ORBufFlags( register __a0 UBYTE *ubuf, register __d1 ULONG flags );
extern void __asm intAZ_ANDBufFlags( register __a0 UBYTE *ubuf, register __d1 ULONG flags );

extern LONG func_CBWrite( struct IOClipReq *clip, char *ptr, LONG len );
extern struct IOClipReq *func_CBWriteStart( LONG unit );
extern void func_CBWriteEnd( struct IOClipReq *clip, LONG len );
extern LONG func_CBRead( struct IOClipReq *clip, char *ptr, LONG len );
extern struct IOClipReq *func_CBReadStart( LONG unit, ULONG *plen );
extern void func_CBReadEnd( struct IOClipReq *clip );

extern struct AZSema * __asm intAZ_SemaCreate( void );
extern void __asm intAZ_SemaWrite( register __a0 struct AZSema *as );
extern void __asm intAZ_SemaFreeWrite( register __a0 struct AZSema *as );
extern void __asm intAZ_SemaRead( register __a0 struct AZSema *as );
extern void __asm intAZ_SemaFreeRead( register __a0 struct AZSema *as );

extern void __asm SET_MODIFIED( register __d0 LONG line, register __d1 LONG col );
extern void __asm CLR_MODIFIED( void );
