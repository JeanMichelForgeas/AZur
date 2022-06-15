extern UBYTE * __asm AZ_nl_AllocNode( register __a0 struct UIKList *L, register __d0 LONG ind, register __d1 LONG len, register __a1 WORD flags );
extern struct CmdGerm * __asm FindRegGerm( register __a1 UBYTE *objname );
extern struct AZEqual * __asm FindEqual( register __a1 UBYTE *cmdstr );

extern void eng_ShowRequest( UBYTE *fmt, WORD error, UBYTE *p1, UBYTE *p2, UBYTE *p3 );
extern ULONG eng_RegisterInternalObjects( void );
extern void eng_UnregisterAllObjects( void );
extern void __asm eng_DisposeObject( register __a1 struct CmdObj *co );
extern struct AZObj * __asm eng_NewPrivateObject( register __a0 struct CmdGerm *rg, register __a1 struct TagItem *objtaglist );
extern struct AZObj * __asm eng_NewObject( register __a0 UBYTE *name, register __a1 struct TagItem *objtaglist );
extern struct AZObj *eng_NewObjectTags( UBYTE *name, ... );
extern struct CmdGerm * __asm eng_NewGerm( register __a0 UBYTE *name );
extern ULONG eng_ExecuteAZurCmd( UBYTE *cmd, ULONG from, ULONG parm );

extern ULONG __asm eng_DoAction(            register __a1 struct CmdObj *co, register __d0 UWORD type, register __d1 ULONG parm );
extern ULONG __asm eng_RGNewAction(         register __a1 struct CmdGerm *rg );
extern void  __asm eng_RGDeleteAction(      register __a1 struct CmdGerm *rg, register __d0 UWORD type );
extern ULONG __asm eng_RGNewAllActions(     register __a1 struct CmdGerm *rg );
extern void  __asm eng_RGDeleteAllActions(  register __a1 struct CmdGerm *rg );
extern ULONG __asm eng_RGNewActionFunc(     register __a1 struct CmdGerm *rg, register __d0 UWORD type, register __d1 struct CmdFunc *cf, register __a0 ULONG end );

extern ULONG __asm eng_Tags2Struct( register __a0 struct TagItem *tl, register __a1 APTR rgobj );

extern LONG __asm pos_tab( register __d0 LONG col );
extern LONG __asm next_tab( register __d0 LONG col );
extern LONG __asm prev_tab( register __d0 LONG col );
extern ULONG __asm skip_endchar( register __a0 UBYTE *p, register __d0 UBYTE c1, register __d1 UBYTE c2, register __a1 ULONG len );
extern ULONG __asm find_endchar( register __a0 UBYTE *p, register __d0 UBYTE c1, register __d1 UBYTE c2, register __a1 ULONG len );
extern ULONG __asm skip_char( register __a0 UBYTE *p, register __d0 UBYTE c1, register __d1 UBYTE c2 );
extern ULONG __asm find_char( register __a0 UBYTE *p, register __d0 UBYTE c1, register __d1 UBYTE c2 );

extern UBYTE *func_BufPadS( UBYTE ch, ULONG numch, UBYTE **pub, ULONG start );

extern ULONG __asm DoOnOffToggle( register __a0 APTR parms, register __d0 ULONG initial );
extern ULONG __asm AZMapANSI( register __a0 UBYTE *string, register __d0 ULONG count, register __a1 UBYTE *buffer, register __d1 ULONG length, register __a2 struct KeyMap *keyMap );
extern ULONG __asm AZRawKeyConvert( register __a0 APTR ev, register __a1 UBYTE *buf, register __d1 ULONG len );
extern UBYTE * __asm FindTabName( register __a0 APTR Tab, register __a1 UBYTE *name, register __d0 ULONG len, register __d1 APTR *ptr );
extern APTR __asm PoolAlloc( register __d0 ULONG memSize );
extern void __asm PoolFree( register __a1 void *memory );
extern void __asm Text_FreeNodes( register __a0 struct UIKList *L, register __d0 LONG from, register __d1 LONG to );

/*
extern ULONG __asm AllocNewStack( register __d0 ULONG stacksize, register __a0 ULONG *b );
extern void __asm RestoreStack( register __a0 ULONG *b );
*/
LONG __asm AllocNewStack( register __a0 ULONG *, register __d0 ULONG );
void __asm RestoreStack( register __a0 ULONG * );
