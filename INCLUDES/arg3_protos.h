extern long __asm ARG_StrToLong( register __a0 UBYTE *str, register __a1 long *pval );
extern ULONG __asm ARG_StrLen( register __a0 UBYTE *str );
extern UBYTE * __asm AllocSpMem( register __d0 long size );
extern void __asm FreeSpMem( register __a1 UBYTE *mem );
extern UBYTE * __stdargs ResizeSpMem( long newsize, UBYTE *old );
extern int __asm SpMemLen( register __a0 UBYTE *mem );
extern void __asm BuildWBArgs( register __a0 UBYTE *cmdbuf, register __a1 UBYTE **tab );
extern struct DiskObject * __asm my_GetDiskObject( register __a0 UBYTE *name, register __a6 ULONG iconbase );
extern void __asm my_FreeDiskObject( register __a0 struct DiskObject *diskobj, register __a6 ULONG iconbase );
extern UBYTE * __asm my_FindToolType( register __a0 UBYTE **toolTypeArray, register __a1 UBYTE *typeName, register __a6 ULONG iconbase );
extern UBYTE *ARG_BuildCmdLine( ULONG pargc, UBYTE *pargv[], ULONG *pCurDirLock, UBYTE *template );
extern void __asm ARG_FreeCmdLine( register __a1 UBYTE *wbargs );
extern APTR __asm ARG_Parse( register __d1 UBYTE *Template, register __d2 APTR result,
        register __d3 APTR PArg, register __d4 ULONG argc, register __a6 ULONG DOSBase );
extern void * __asm ARG_FreeParse( register __d1 APTR parg );
extern ULONG __asm ARG_MatchArg( register __d1 APTR pattern, register __d2 APTR keyword );
