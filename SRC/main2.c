#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:arg3.h"
#include "lci:arg3_protos.h"

#include "src:datatemplate2.c"

#define UIKLIB_MINVER   1
#define UIKLIB_MINREV   51

static UBYTE __far VersionTag[] = "\0$VER: AZurQuick 0.85 (30.12.94)";

extern struct Library * __far DOSBase;

struct Library * __far ExecBase;
struct Library * __far UIKBase;
struct Library * __far IntuitionBase;

#define TEXT_PARSE_ERROR        1
#define TEXT_PARSE_ERRORBASE    2
#define TEXT_NOMEMORY           8
#define TEXT_PARSE_COMMENT      12
#define TEXT_ERR_NoAZur         13

static UBYTE * __far LangEnglish[] =
    {
    "",
    "Parsing error: ",
    "parsing ok",
    "Line too long",
    "Bad template",
    "Required argument missing",
    "Argument line invalid or too long",
    "Argument after '=' or keyword missing",
    "Not enough memory",
    "Too many arguments",
    "Bad number",
    "No command line",
    "Type 'AZurQuick ?'",
    "You have to run one AZur for MastPortName='%ls'",
    0
    };

static UBYTE * __far LangOther[] =
    {
    "",
    "Erreur : ",
    "OK",
    "Ligne d'arguments trop longue",
    "Template non valide",
    "Argument obligatoire manquant",
    "Ligne d'arguments incorrecte ou trop longue",
    "Argument de mot-clé manquant",
    "Mémoire insuffisante",
    "Nombre d'arguments incorrect",
    "Chiffre non valide",
    "Pas de ligne d'arguments",
    "Tapez 'AZurQuick ?'",
    "Il faut lancer un AZur pour MastPortName='%ls'",
    0
    };


static void ShowRequest( UBYTE *fmt, UBYTE *p1, UBYTE *p2, UBYTE *p3 )
{
  UBYTE buf[200];

    sprintf( buf, fmt, p1, p2, p3 );
    if (IntuitionBase)
        {
        struct IntuiText msg = { -1, -1, JAM1, 10, 10, NULL, NULL, NULL },
                         ok  = { -1, -1, JAM1, 6, 3, NULL, "OK", NULL };
        msg.IText = buf;
        AutoRequest( NULL, &msg, 0, &ok, 0, 0, (__builtin_strlen( buf ) + 13) * 8, 50 );
        }
}

static void no_memory( struct UIKGlobal *uik )
{
    ShowRequest( "%ls", UIK_LangString( uik, TEXT_NOMEMORY ), 0, 0 );
}

static void LibAlert( UBYTE *name, ULONG ver, ULONG rev )
{
    ShowRequest( "WANTED: %ls %ld.%ld minimum", name, (UBYTE*)ver, (UBYTE*)rev );
}

void main( int argc, char **argv )
{
  ULONG len=0, curdir=0, nil, started;
  APTR ArgRes[ARG2_TOTAL];
  struct ParseArgs PArg;
  struct MsgPort *myport=0, *mastport=0;
  APTR *Parse=0;
  UBYTE *CmdLine=0, *ptr, *mpn, quickname[32];
  struct UIKGlobal *UIK;
  struct AZQuickMsg *amsg=0;

    ExecBase = (APTR) *((ULONG*)4);
    strcpy( quickname, QUICKPREFIX );
    mpn = "AZurMast";

    __builtin_memset( &ArgRes[0], 0, sizeof(ArgRes) );
    __builtin_memset( &PArg, 0, sizeof(struct ParseArgs) );

    //------ Libs
    if (! (IntuitionBase = OpenLibrary( "intuition.library", 0 ))) goto END_ERROR;
    if (! (UIKBase = (APTR) OpenLibrary( UIKNAME, UIKLIB_MINVER )) || ((UIKBase->lib_Revision < UIKLIB_MINREV) && (UIKBase->lib_Version == UIKLIB_MINVER)))
        { LibAlert( UIKNAME, UIKLIB_MINVER, UIKLIB_MINREV ); goto END_ERROR; }

    //------ UIK
    if (! (UIK = (struct UIKGlobal *) UIK_AddObjectTags( "UIK", 0,
        UIKTAG_GEN_LangEnglish,     LangEnglish,
        UIKTAG_GEN_LangOther,       LangOther,
        UIKTAG_GEN_LangDuplicate,   FALSE,
        UIKTAG_GEN_Catalog,         "AzurQuick.catalog",
        TAG_END ))) { ShowRequest( "UIK Error...", 0,0,0 ); goto END_ERROR; }

    //------ Parsing
    PArg.Error = PAERR_NOTENOUGH_MEMORY;

    if (CmdLine = ARG_BuildCmdLine( argc, argv, &curdir, ArgsTemplate2 ))
        {
        len = StrLen( CmdLine );
        PArg.CmdLine = CmdLine;
        PArg.CmdLength = len;
        PArg.SepSpace = 0x9;
        PArg.Flags = PAF_PROMPT;
        if (Parse = ARG_Parse( ArgsTemplate2, &ArgRes[0], &PArg, argc, (ULONG)DOSBase ))
            {
            APTR a;

            if (a = ArgRes[ARG2_MASTPORTNAME]) mpn = a;
            }
        }
    if (PArg.Error != PAERR_OK)
        {
        ShowRequest( "%ls. %ls",
            UIK_LangString( UIK, TEXT_PARSE_ERRORBASE + PArg.Error ),
            UIK_LangString( UIK, TEXT_PARSE_COMMENT ), 0 );
        goto END_ERROR;
        }
    if (argc == 2 && argv[1][0] == '?') goto END_ERROR;

    //------ Comm
    if (! (myport = (struct MsgPort *) CreatePort( 0, 0 ))) { no_memory( UIK ); goto END_ERROR; }
    if (! (amsg = (struct AZQuickMsg *) CreateExtIO( myport, sizeof(struct AZQuickMsg) ))) { no_memory( UIK ); goto END_ERROR; }
    if (! (amsg->CurDirLock = DupLock( curdir ))) goto END_ERROR;

    //------------- Nom du port public
    StcCpy( quickname+StrLen(quickname), mpn, 32 );
    UIK_StrToUpper( quickname );

    //------------- Lancement d'une tâche maîtresse si pas de port
    started = 0;
    if (! FindPort( quickname ))
        {
        if (nil = Open( "NIL:", MODE_NEWFILE ))
            {
            if (ptr = UIK_MemSpAlloc( StrLen(CmdLine) + 100, MEMF_PUBLIC ))
                {
                ULONG rc;
                sprintf( ptr, "AZur:AZur %ls", CmdLine );
                Execute( ptr, 0, nil );
                started = 1;
                UIK_MemSpFree( ptr );
                }
            Close( nil );
            }
        }
    if (started) goto END_ERROR;

    //------------- Lancement d'une tâche fille
    Forbid();
    if (mastport = FindPort( quickname ))
        {
        if (len) amsg->CmdLine = CmdLine;
        amsg->Parent = FindTask(0);
        if (ArgRes[ARG2_ASYNC]) amsg->Flags |= AZQMF_ASYNC;
        PutMsg( mastport, amsg );
        }
    Permit();

    if (! mastport)
        {
        if (! ArgRes[ARG2_KILL]) ShowRequest( UIK_LangString( UIK, TEXT_ERR_NoAZur ), quickname+StrLen(QUICKPREFIX), 0,0 );
        }
    else{
        if (ArgRes[ARG2_ASYNC])
            {
            CmdLine = 0; // seront libérés par la tâche mère
            amsg = 0;
            }
        else while (TRUE)
            {
            ULONG events = Wait( SIGBREAKF_CTRL_C | 1L << myport->mp_SigBit );
            if (events & 1L << myport->mp_SigBit)
                {
                if (GetMsg( myport )) break;
                }
            if (events & SIGBREAKF_CTRL_C)
                {
                if (amsg->Edit) Signal( amsg->Edit, SIGBREAKF_CTRL_C );
                }
            }
        }

  END_ERROR:
    if (UIKBase)
        {
        if (myport)
            {
            if (amsg)
                {
                if (amsg->CurDirLock) UnLock( amsg->CurDirLock );
                DeleteExtIO( amsg );
                }
            DeletePort( myport );
            }
        ARG_FreeParse( Parse );
        ARG_FreeCmdLine( CmdLine );
        UIK_Remove( UIK );
        CloseLibrary( UIKBase );
        }
    if (IntuitionBase) CloseLibrary( IntuitionBase );
}
