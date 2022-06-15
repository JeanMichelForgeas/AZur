/****************************************************************
 *
 *      Project:    DicServ
 *
 *      Created:    21/12/94 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "dicserv.h"
#include "_pool_protos.h"

#include "Projects:UIK/INCLUDES/uik_protosint.h"


/****** Imported ************************************************/

extern ULONG dic_create( UBYTE *dicname );
extern ULONG dic_load( UBYTE *dicname );
extern ULONG dic_unload( struct DicNode *node );
extern ULONG dic_unloadall( ULONG force );
extern ULONG dic_save( struct DicNode *node );
extern ULONG dic_addentry( struct DicNode *node, UBYTE *str, UBYTE *str2, ULONG flags );
extern ULONG dic_deleteentry( struct DicNode *node, UBYTE *str );
extern ULONG dic_searchentry( struct DicNode *node, UBYTE *str, ULONG cmd, UBYTE **ubuf );
extern ULONG dic_list( struct DicNode *node, UBYTE *filename );
extern struct DicNode *find_dicname( UBYTE *dicname );

extern struct Library *DOSBase;
extern struct Library *SysBase;

extern struct SignalSemaphore DicSem;
extern struct List DicList;
extern APTR Pool;
extern ULONG UseCount, StopWait;
extern struct Task *Process;
extern ULONG TotalFound;


/****** Statics ************************************************/


/****** Exported ***********************************************/


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

static ULONG do_multiple( ULONG (*func)(), UBYTE *tab[], APTR arg1, APTR arg2, APTR arg3 )
{
  struct DicNode *node, *succ;
  UBYTE buf[300];
  ULONG i, errmsg=0;

    if (((struct Library *)SysBase)->lib_Version >= 37)
        {
        for (i=0; tab[i]; i++)
            {
            asm_StrToUpper( tab[i] ); // bug Sys < V39
            ParsePatternNoCase( tab[i], buf, 300 );
            for (node=(struct DicNode *)DicList.lh_Head; node->ln_Succ; )
                {
                succ = (struct DicNode *)node->ln_Succ; // avant à cause de unload possible
                if (MatchPatternNoCase( buf, node->ln_Name ))
                    { if (errmsg = func( node, arg1, arg2, arg3 )) break; }
                node = succ;
                }
            }
        }
    else{
        for (i=0; tab[i]; i++)
            {
            for (node=(struct DicNode *)DicList.lh_Head; node->ln_Succ; )
                {
                succ = (struct DicNode *)node->ln_Succ; // avant à cause de unload possible
                if (StrNCCmp( tab[i], node->ln_Name ) == 0)
                    { errmsg = func( node, arg1, arg2, arg3 ); break; }
                node = succ;
                }
            }
        }
    return( errmsg );
}

//-----------------------------------------------------------

ULONG direct_Lock( void )
{
  ULONG errmsg=0;

    Forbid();
    if (FindPort( PORTNAME ))
        {
        ObtainSemaphore( &DicSem );
        UseCount++;
        ReleaseSemaphore( &DicSem );
        }
    else errmsg = ERR_NODICSERV;
    Permit();
    return( errmsg );
}

void direct_Unlock( void )
{
    ObtainSemaphore( &DicSem );
    if (UseCount > 0) UseCount--;
    if (StopWait) Signal( Process, SIGBREAKF_CTRL_C );
    ReleaseSemaphore( &DicSem );
}

//-------------------------

struct DicNode *direct_Find( UBYTE *dicname )
{
  struct DicNode *node;

    ObtainSemaphore( &DicSem );
    node = find_dicname( dicname );
    ReleaseSemaphore( &DicSem );
    return( node );
}

ULONG direct_Create( UBYTE *tab[] )
{
  ULONG i, errmsg=0;

    ObtainSemaphore( &DicSem );

    for (i=0; tab[i]; i++)
        { errmsg = dic_create( tab[i] ); }

    ReleaseSemaphore( &DicSem );
    return( errmsg );
}

static verif_icon( UBYTE *name )
{
    ULONG len = StrLen( name );
    if (len >= 5) return( StrNCCmp( name+len-5, ".info" ) );
    return(1);
}

ULONG direct_Load( UBYTE *tab[] )
{
  struct AnchorPath *ma;
  ULONG i, errmsg=0;

    ObtainSemaphore( &DicSem );

    if (ma = AsmAllocPooled( Pool, sizeof(struct AnchorPath) + 256, *((APTR*)4) ))
        {
        ma->ap_BreakBits = SIGBREAKF_CTRL_C;
        ma->ap_Strlen = 256;

        for (i=0; tab[i] && errmsg==0; i++)
            {
            if (((struct Library *)SysBase)->lib_Version >= 37)
                {
                if (MatchFirst( tab[i], ma ) == 0)
                    {
                    do { if (verif_icon( ma->ap_Buf )) errmsg = dic_load( ma->ap_Buf ); }
                        while (errmsg==0 && MatchNext( ma )==0);
                    MatchEnd( ma );
                    if (ma->ap_FoundBreak & SIGBREAKF_CTRL_C) break;
                    }
                else errmsg = dic_load( tab[i] );
                }
            else errmsg = dic_load( tab[i] );
            }
        AsmFreePooled( Pool, ma, sizeof(struct AnchorPath) + 256, *((APTR*)4) );
        }
    else errmsg = ERR_NOMEMORY;

    ReleaseSemaphore( &DicSem );
    return( errmsg );
}

ULONG direct_Unload( UBYTE **name )
{
  ULONG errmsg;

    ObtainSemaphore( &DicSem );
    errmsg = do_multiple( dic_unload, name, 0, 0, 0 );
    ReleaseSemaphore( &DicSem );
    return( errmsg );
}

ULONG direct_UnloadAll( ULONG force )
{
    ObtainSemaphore( &DicSem );
    dic_unloadall( force );
    ReleaseSemaphore( &DicSem );
    return(0);
}

ULONG direct_Save( UBYTE **name )
{
  ULONG errmsg;

    ObtainSemaphore( &DicSem );
    errmsg = do_multiple( dic_save, name, 0, 0, 0 );
    ReleaseSemaphore( &DicSem );
    return( errmsg );
}

ULONG direct_AddEntry( UBYTE **name, UBYTE *str, UBYTE *str2, ULONG flags )
{
  ULONG errmsg;

    ObtainSemaphore( &DicSem );
    errmsg = do_multiple( dic_addentry, name, str, str2, 0 );
    ReleaseSemaphore( &DicSem );
    return( errmsg );
}

ULONG direct_DeleteEntry( UBYTE **name, UBYTE *str )
{
  ULONG errmsg;

    ObtainSemaphore( &DicSem );
    errmsg = do_multiple( dic_deleteentry, name, str, 0, 0 );
    ReleaseSemaphore( &DicSem );
    return( errmsg );
}

ULONG direct_SearchEntry( UBYTE **name, UBYTE *str, ULONG cmd, UBYTE **ubuf )
{
  ULONG found;

    ObtainSemaphore( &DicSem );
    TotalFound = 0;
    do_multiple( dic_searchentry, name, str, (APTR)cmd, ubuf );
    found = TotalFound;
    ReleaseSemaphore( &DicSem );
    return( found );
}

ULONG direct_List( UBYTE **name, UBYTE *filename )
{
  ULONG file, errmsg;

    ObtainSemaphore( &DicSem );
    if (file = Open( filename, MODE_NEWFILE )) Close( file );
    errmsg = do_multiple( dic_list, name, filename, 0, 0 );
    ReleaseSemaphore( &DicSem );
    return( errmsg );
}
