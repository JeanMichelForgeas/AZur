/****************************************************************
 *
 *      File:      lock.c       routines de lock
 *      Project:   AZ           editeur de texte
 *
 *      Created:   07-12-89     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objtimer.h"

#include "lci:azur.h"
#include "lci:azur_protos.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/

static ULONG __far elapsed1, __far elapsed2;


/****************************************************************
 *
 *      Debug
 *
 ****************************************************************/

/*
static ULONG dodo( struct timeval *t )
{   /* tient dans un long car l'interval choisi est inférieur à 4 heures */
    return( t->tv_secs * ONESECOND + t->tv_micro );
}

void time1()
{
    elapsed1 = dodo( (struct timeval *) UIK_CallObjectFunc( AGP.Timer, UIKFUNC_Timer_GetElapsed, 0, 0 ) );
}

void time2( UBYTE *str )
{
  ULONG diff, v1, v2;

    elapsed2 = dodo( (struct timeval *) UIK_CallObjectFunc( AGP.Timer, UIKFUNC_Timer_GetElapsed, 0, 0 ) );

    diff = elapsed2 - elapsed1;
    v1 = diff / ONESECOND;
    v2 = diff % ONESECOND;
    kprintf( "%ls time=%ld.%06.6ld\n", str, v1, v2 );
    elapsed1 = elapsed2;
}

ULONG time3()
{
    elapsed2 = dodo( (struct timeval *) UIK_CallObjectFunc( AGP.Timer, UIKFUNC_Timer_GetElapsed, 0, 0 ) );
    return( elapsed2 - elapsed1 );
}

void time4( UBYTE *str, ULONG tot )
{
  ULONG v1, v2;

    v1 = tot / ONESECOND;
    v2 = tot % ONESECOND;
    kprintf( "%ls time=%ld.%06.6ld\n", str, v1, v2 );
}
*/

/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

ULONG lo_GetFIB( UBYTE *name )
{
  ULONG lock, rc=0;

    if (lock = Lock( name, ACCESS_READ ))
        {
        rc = Examine( lock, &AGP.FIB );
        UnLock( lock );
        }
    return( rc );
}

ULONG lo_FileExists( UBYTE *name )
{
    if (lo_GetFIB( name ))
        {
        if (AGP.FIB.fib_DirEntryType < 0)
            return(1);  /* it is a file */
        }
    return(0);
}

ULONG lo_FileSize( UBYTE *name )
{
    if (lo_FileExists( name ))
        {
        return( (ULONG) AGP.FIB.fib_Size );
        }
    return(0);
}

ULONG lo_NameType( UBYTE *name )
{
    if (name == 0) return( NAME_IS_NULL );
    if (lo_GetFIB( name ))
        {
        if (AGP.FIB.fib_DirEntryType < 0) return( NAME_IS_FILE );
        return( NAME_IS_DIR );
        }
    return( NAME_IS_VOID );
}

ULONG lo_DirCreate( UBYTE *name )
{
  ULONG lock;

    if (lo_GetFIB( name ))
        {
        if (AGP.FIB.fib_DirEntryType > 0) return(1);  /* it is a directory */
        }
    if (lock = CreateDir( name ))
        {
        UnLock( lock );
        return(1);
        }
    return(0);
}

ULONG lo_NewerFile( UBYTE *name1, UBYTE *name2, struct DateStamp *pd1, struct DateStamp *pd2, ULONG *ps1, ULONG *ps2 )
{
  struct DateStamp d1, d2;

    if (lo_GetFIB( name1 ))
        {
        d1 = AGP.FIB.fib_Date;
        if (ps1) *ps1 = AGP.FIB.fib_Size;

        if (lo_GetFIB( name2 ))
            {
            d2 = AGP.FIB.fib_Date;
            if (ps2) *ps2 = AGP.FIB.fib_Size;

            if (pd1) *pd1 = d1; if (pd2) *pd2 = d2;
            if (d1.ds_Days > d2.ds_Days || (d1.ds_Days == d2.ds_Days && d1.ds_Minute > d2.ds_Minute) || (d1.ds_Days == d2.ds_Days && d1.ds_Minute == d2.ds_Minute && d1.ds_Tick > d2.ds_Tick))
                return(1);
            return(2);
            }
        }
    return(0);
}

void lo_GetPath( UBYTE *name, UIKBUF **pubuf )
{
  LONG save, lock, i, l, oldlock, ins;

    BufTruncate( *pubuf, 0 );
    save = UIK_DisableDOSReq();
    if (lock = Lock( name, ACCESS_READ ))
        {
        while (lock)
            {
            if (Examine( lock, &AGP.FIB ))
                {
                BufPasteS( "/", 1, pubuf, 0 );
                BufPasteS( AGP.FIB.fib_FileName, StrLen( AGP.FIB.fib_FileName ), pubuf, 0 );
                }
            oldlock = lock;
            lock    = ParentDir( lock );
            UnLock( oldlock );
            }

        for(i=0, ins=1, l=BufLength(*pubuf); i < l; i++)
            if ((*pubuf)[i] == '/') { ins = 0; break; }

        if (! ins) (*pubuf)[i] = ':';
        else BufPasteS( ":", 1, pubuf, BufLength(*pubuf) );
        }
    UIK_EnableDOSReq( 0, save );
}

ULONG lo_SetCurDir( UBYTE *name )
{
  ULONG lock, dirlock, rc=0;

    if (! (dirlock = Lock( name, ACCESS_READ )))
        {
        UBYTE *ub;
        if (ub = BufAlloc( 300, 0, MEMF_ANY ))
            {
            if (BufSetS( name, StrLen(name), &ub ))
                {
                *(UIK_BaseName( ub )) = 0;
                dirlock = Lock( ub, ACCESS_READ );
                }
            BufFree( ub );
            }
        }
    if (dirlock)
        {
        if (Examine( dirlock, &AGP.FIB ))
            {
            if (AGP.FIB.fib_DirEntryType < 0)
                {
                lock = dirlock;
                dirlock = ParentDir( lock );
                UnLock( lock );
                }
            CurrentDir( dirlock );
            if (AGP.CurDirLock) UnLock( AGP.CurDirLock );
            AGP.CurDirLock = dirlock; dirlock = 0;
            rc = 1;
            }
        if (dirlock) UnLock( dirlock );
        }
    return( rc );
}

UBYTE *lo_MakeTotalPath( UBYTE *name ) // crée un uikbuffer sans le libérer
{
  ULONG len, i, found;
  UBYTE *ub = 0;

    for (len=strlen(name), i=0, found=0; i < len; i++)
        { if (name[i] == ':') { found = 1; break; } }
    if (found == 0) // si pas de path absolu
        {
        if (ub = BufAlloc( 300, 0, MEMF_ANY ))
            {
            lo_GetPath( "", &ub );
            BufPasteS( name, len, &ub, BufLength(ub) );
            }
        }
    return( ub );
}
