#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:azur.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

struct CmdGerm __far rg;
struct CmdFunc __far cf;

AZUR __far GLOB;

UBYTE * __far Names[] = { "test", 0 };

static struct TagItem __far GermTags[] = {
    AZT_Gen_Name,           (ULONG)Names,
    AZT_Gen_Size,           450,
    AZT_Gen_Id,             30,
    AZT_Gen_VersionRevision, W2L(1,2),
    AZT_Gen_UserData,       33333,
    AZT_Gen_ArgTemplate,    (ULONG)"AAA,BBB,CCC",
    AZT_ActionLoad,     1, // 0
    AZT_ActionNew,      2, // 2
    AZT_ActionDispose,  3, // 3
    AZT_ActionSetMenu,  4, // 5
    AZT_ActionUndo,     5, // 7
    TAG_END,
    };

/*******************************************************************/

static void Cleanup()
{
    if (DOSBase) CloseLibrary( DOSBase );
    if (UIKBase) CloseLibrary( UIKBase );
    exit(0);
}

void main()
{
  ULONG i, j, num, act, a4 = __builtin_getreg(12);
  struct CmdFunc *rf;

    __builtin_putreg( 12, &GLOB );
    if (! (UIKBase = (struct Library *) OpenLibrary( "uik.library", 0 ))) goto END_ERROR;
    if (! (DOSBase = (struct Library *) OpenLibrary( "dos.library", 0 ))) goto END_ERROR;

    GLOB.AZMast = &GLOB;
    InitSemaphore( &GLOB.RGSem );

    if (! func_NewAllActions( &rg )) goto END_ERROR;
    kprintf( "total=%ld\n", BufLength( (UBYTE*)rg.Actions ) / 4 );

    func_NewAction( &rg );
    func_NewAction( &rg );
    func_NewAction( &rg );
    func_NewAction( &rg );
    func_NewAction( &rg );
    func_NewAction( &rg );
    kprintf( "total=%ld\n", BufLength( (UBYTE*)rg.Actions ) / 4 );

    eng_Tags2Struct( GermTags, &rg );
    kprintf( "\tName     : %ls\n", rg.Names[0] );
    kprintf( "\tSize     : %ld\n", rg.ObjSize );
    kprintf( "\tId       : %ld\n", rg.Id );
    kprintf( "\tVersion  : %ld\n", rg.Version );
    kprintf( "\tRevision : %ld\n", rg.Revision );
    kprintf( "\tItems    : %ld\n", rg.Items );
    kprintf( "\tTemplate : %ls\n", rg.CmdTemplate );
    kprintf( "\tUserData : %ld\n", rg.UserData );

    cf.Parm = (APTR)1; func_NewActionFunc( &rg, 1, &cf, 1 );
    cf.Parm = (APTR)2; func_NewActionFunc( &rg, 6, &cf, 1 );
    cf.Parm = (APTR)3; func_NewActionFunc( &rg, 12, &cf, 1 );
    cf.Parm = (APTR)4; func_NewActionFunc( &rg, 12, &cf, 0 );
    cf.Parm = (APTR)5; func_NewActionFunc( &rg, 13, &cf, 1 );
    cf.Parm = (APTR)6; func_NewActionFunc( &rg, 13, &cf, 0 );
    cf.Parm = (APTR)7; func_NewActionFunc( &rg, 14, &cf, 0 );
    cf.Parm = (APTR)8; func_NewActionFunc( &rg, 14, &cf, 0 );

    kprintf( "-----------------------\n" );
    act = BufLength( (UBYTE*)rg.Actions ) / 4;
    for (i=0; i < act; i++)
        {
        kprintf( "Actions=%lx, Actions[%ld]=%lx\n", rg.Actions, i, rg.Actions[i] );
        if (rf = (struct CmdFunc *)rg.Actions[i])
            {
            num = BufLength( (UBYTE*)rf ) / 8;
            for (j=0; j < num; j++)
                kprintf( "  Func=%lx, Parm=%lx\n", rf[j].Func, rf[j].Parm );
            }
        }

    func_DeleteAction( &rg, 10 );
    func_DeleteAction( &rg, 13 );

    kprintf( "-----------------------\n" );
    act = BufLength( (UBYTE*)rg.Actions ) / 4;
    for (i=0; i < act; i++)
        {
        kprintf( "Actions=%lx, Actions[%ld]=%lx\n", rg.Actions, i, rg.Actions[i] );
        if (rf = (struct CmdFunc *)rg.Actions[i])
            {
            num = BufLength( (UBYTE*)rf ) / 8;
            for (j=0; j < num; j++)
                kprintf( "  Func=%lx, Parm=%lx\n", rf[j].Func, rf[j].Parm );
            }
        }

    func_DeleteAllActions( &rg );

  END_ERROR:
    __builtin_putreg( 12, a4 );
    Cleanup();
}
