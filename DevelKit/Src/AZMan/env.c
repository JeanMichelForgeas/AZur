#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

ULONG lo_GetFIB( UBYTE *name, struct FileInfoBlock *fib )
{
  ULONG lock, rc=0;

    if (lock = Lock( name, ACCESS_READ ))
        {
        rc = Examine( lock, fib );
        UnLock( lock );
        }
    return( rc );
}

ULONG lo_FileExists( UBYTE *name, struct FileInfoBlock *fib )
{
    if (lo_GetFIB( name, fib ))
        {
        if (fib->fib_DirEntryType < 0)
            return(1);  /* it is a file */
        }
    return(0);
}

ULONG lo_FileSize( UBYTE *name, struct FileInfoBlock *fib )
{
    if (lo_FileExists( name, fib ))
        {
        return( (ULONG) fib->fib_Size );
        }
    return(0);
}

ULONG lo_DirCreate( UBYTE *name, struct FileInfoBlock *fib )
{
  ULONG lock;

    if (lo_GetFIB( name, fib ))
        {
        if (fib->fib_DirEntryType > 0) return(1);  /* it is a directory */
        }
    if (lock = CreateDir( name ))
        {
        UnLock( lock );
        return(1);
        }
    return(0);
}

void func_EnvFree( APTR pref )
{
    UIK_MemSpFree( pref );
}

APTR func_EnvGet( STRPTR prefname )
{
  struct FileInfoBlock *fib=0;
  ULONG lock=0, oldlock, ok, file, prefsize;
  APTR pref=0;

    if (! (fib = (struct FileInfoBlock *) AllocMem( sizeof(struct FileInfoBlock), MEMF_PUBLIC|MEMF_CLEAR ))) goto END_ERROR;
    if (! (lock = Lock( "ENV:AzurEditor/Commands", ACCESS_READ ))) goto END_ERROR;

    oldlock = CurrentDir( lock );
    ok = 0;
    if (prefsize = lo_FileSize( prefname, fib ))
        if (pref = (APTR) UIK_MemSpAlloc( prefsize, MEMF_PUBLIC ))
            if (file = Open( prefname, MODE_OLDFILE ))
                {
                if (Read( file, pref, prefsize ) == prefsize) ok = 1;
                Close( file );
                }
    CurrentDir( oldlock );

    if (! ok)
        { UIK_MemSpFree( pref ); pref = 0; }

  END_ERROR:
    if (lock) UnLock( lock );
    if (fib) FreeMem( fib, sizeof(struct FileInfoBlock) );
    return( pref );
}

ULONG func_EnvSet( STRPTR prefname, APTR pref, ULONG prefsize )
{
  struct FileInfoBlock *fib=0;
  ULONG old, lock=0, oldlock, ok, file;

    if (! (fib = (struct FileInfoBlock *) AllocMem( sizeof(struct FileInfoBlock), MEMF_PUBLIC|MEMF_CLEAR ))) goto END_ERROR;

    old = UIK_DisableDOSReq();
    if (! lo_DirCreate( "ENV:AzurEditor/Commands", fib )) goto END_ERROR;
    if (! lo_DirCreate( "ENVARC:AzurEditor/Commands", fib )) goto END_ERROR;
    UIK_EnableDOSReq( 0, old );

    if (! (lock = Lock( "ENV:AzurEditor/Commands", ACCESS_READ ))) goto END_ERROR;
    oldlock = CurrentDir( lock );
    ok = 0;
    if (file = Open( prefname, MODE_NEWFILE ))
        {
        if (Write( file, pref, prefsize ) == prefsize) ok = 1;
        Close( file );
        }
    CurrentDir( oldlock );
    UnLock( lock ); lock = 0;
    if (! ok) goto END_ERROR;

    if (! (lock = Lock( "ENVARC:AzurEditor/Commands", ACCESS_READ ))) goto END_ERROR;
    oldlock = CurrentDir( lock );
    ok = 0;
    if (file = Open( prefname, MODE_NEWFILE ))
        {
        if (Write( file, pref, prefsize ) == prefsize) ok = 1;
        Close( file );
        }
    CurrentDir( oldlock );
    UnLock( lock ); lock = 0;
    if (! ok) goto END_ERROR;

    return(1);

  END_ERROR:
    if (lock) UnLock( lock );
    if (fib) FreeMem( fib, sizeof(struct FileInfoBlock) );
    return(0);
}

