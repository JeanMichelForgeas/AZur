/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Utilisation de dictionnaire
 *
 *      Created:    20/12/94 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <exec/types.h>
#include <exec/io.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "Azur:DevelKit/Includes/funcs.h"
#include "Azur:DevelKit/Includes/azur.h"
#include "Azur:DevelKit/Includes/obj.h"
#include "Azur:DevelKit/Includes/errors.h"

#include "AZur:/DIC/dicserv.h"
#include "azdic.h"


/****** Imported ************************************************/


/****** Statics ************************************************/

static void AZFunc_Load( struct CmdObj *co );
static void AZFunc_Unload( struct CmdObj *co );
static void AZFunc_Do( struct CmdObj *co );

static UBYTE * __far CmdNames[] = { "AZDic", 0 };

static ULONG __far ErrTable[] = {
    0,
    TEXT_NOMEMORY,              // ERR_NOMEMORY
    TEXT_ERR_NotFound,          // ERR_NODICSERV
    TEXT_ERR_OPENFILE,          // ERR_OPENFILE
    TEXT_ERR_READFILE,          // ERR_READFILE
    TEXT_CMDCANCELLED,          // ERR_CANCELLED
    TEXT_ERR_OPENWRITEFILE,     // ERR_OPENWRITEFILE
    TEXT_ERR_WRITEFILE,         // ERR_WRITEFILE
    TEXT_Search_NotFound,       // ERR_SEARCHNOTFOUND
    TEXT_ERR_OPENFILE,          // ERR_NOTFOUND  (pour le chargement d'un dict)
    0,
    0,
    };

static APTR __far DicHandle;
static ULONG __far launched;


/****** Exported ***********************************************/

struct Library * __far DOSBase;
struct Library * __far SysBase;
struct Library * __far UIKBase;

struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,       (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,(ULONG) "DIC=NAME=DICNAME/M,CREATE/S,LOAD/S,UNLOAD/S,UNLOADALL/S,SAVE/S,LIST/K,STR/K,STR2/K,ADD/S,DEL=DELETE/S,SE=SEARCH/S,TR=TRANSLATE/S,CASE/S,ACCENT/S",
    AZT_Gen_Flags,      AZAFF_DOMODIFY | AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY | AZAFF_OKINMASTER,
    AZT_ActionLoad,     AZFunc_Load,
    AZT_ActionUnload,   AZFunc_Unload,
    AZT_ActionDo,       AZFunc_Do,
    TAG_END
    };

#define ARG_NAME        co->ArgRes[ 0]
#define ARG_CREATE      co->ArgRes[ 1]
#define ARG_LOAD        co->ArgRes[ 2]
#define ARG_UNLOAD      co->ArgRes[ 3]
#define ARG_UNLOADALL   co->ArgRes[ 4]
#define ARG_SAVE        co->ArgRes[ 5]
#define ARG_LIST        co->ArgRes[ 6]
#define ARG_STRING      co->ArgRes[ 7]
#define ARG_STRING2     co->ArgRes[ 8]
#define ARG_ADD         co->ArgRes[ 9]
#define ARG_DELETE      co->ArgRes[10]
#define ARG_SEARCH      co->ArgRes[11]
#define ARG_TRANSLATE   co->ArgRes[12]
#define ARG_CASE        co->ArgRes[13]
#define ARG_ACCENT      co->ArgRes[14]


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

static ULONG start_dicserv( ULONG secs )
{
  ULONG i, found=0;

    if (! (DicHandle = FindPort( PORTNAME )))
        {
        AZ_ExecuteAZurCmd( "ExeDOSCmd ASYNC CMD AZur:Utilities/DicServ", 0 );
        for (i=0; i < (secs*10); i++)
            {
            Delay(5); // 1/10 sec
            if (FindPort( PORTNAME )) { launched = 1; break; }
            }
        }

    Forbid();
    if (DicHandle = FindPort( PORTNAME ))
        {
        DicServ_Lock( DicHandle );
        found = 1;
        }
    Permit();

    return( found );
}

static void AZFunc_Load( struct CmdObj *co )
{
  struct AZurProc *ap = AZ_GetAZur();

    UIKBase = ap->UIKBase;
    SysBase = ap->ExecBase;
    DOSBase = ap->DOSBase;

    start_dicserv(5);
}

static void AZFunc_Unload( struct CmdObj *co )
{
    if (launched) // arrête ce qu'on a lancé
        AZ_ExecuteAZurCmd( "ExeDOSCmd CMD AZur:Utilities/DicServ", 0 );
    if (DicHandle) DicServ_Unlock( DicHandle );
}

//------------------------------------------------------------------------------

static void AZFunc_Do( struct CmdObj *co )
{
  struct AZurProc *ap = AZ_GetAZur();
  ULONG errmsg=0, errdic=0;
  UBYTE *ptr, *TabName[2];

    if (! DicHandle) start_dicserv(1);

    if (! DicHandle)
        {
        UBYTE buf[200];
        sprintf( buf, UIK_LangString( ap->UIK, TEXT_ERR_NotFound ), "AZur:Utilities/DicServ", ":" );
        AZ_ReqShowText( buf );
        }
    else{
        if (! ARG_NAME)
            {
            TabName[1] = 0;
            TabName[0] = AZ_ReqFileNameO( "AZur:Dics", "#?", "Choisir un dictionnaire", 0 );
            if (TabName[0]) ARG_NAME = &TabName[0];
            }

        if (ARG_NAME)
            {
            if (ARG_CREATE)
                {
                errdic = DicServ_Create( DicHandle, ARG_NAME );
                }
            else if (ARG_LOAD)
                {
                errdic = DicServ_Load( DicHandle, ARG_NAME );
                }
            else if (ARG_UNLOAD)
                {
                errdic = DicServ_Unload( DicHandle, ARG_NAME );
                }
            else if (ARG_UNLOADALL)
                {
                errdic = DicServ_UnloadAll( DicHandle, 0 );
                }
            else if (ARG_SAVE)
                {
                errdic = DicServ_Save( DicHandle, ARG_NAME );
                }
            else if (ARG_LIST)
                {
                errdic = DicServ_List( DicHandle, ARG_NAME, ARG_LIST );
                }
            else if (ARG_ADD)
                {
                ULONG flags=0;

                if (ARG_CASE)   flags |= FUF_CASE;
                if (ARG_ACCENT) flags |= FUF_ACCENT;

                if (ARG_STRING == 0) errmsg = TEXT_PARSE_REQARGMISSING;
                else errdic = DicServ_AddEntry( DicHandle, ARG_NAME, ARG_STRING, ARG_STRING2, flags );
                }
            else if (ARG_DELETE)
                {
                if (ARG_STRING == 0) errmsg = TEXT_PARSE_REQARGMISSING;
                else errdic = DicServ_DeleteEntry( DicHandle, ARG_NAME, ARG_STRING );
                }
            else if (ARG_SEARCH || ARG_TRANSLATE)
                {
                ULONG flags=0;

                if (ARG_TRANSLATE) flags |= FUF_TRANSLATE;
                if (ARG_CASE)      flags |= FUF_CASE;
                if (ARG_ACCENT)    flags |= FUF_ACCENT;

                if (ARG_STRING == 0) errmsg = TEXT_PARSE_REQARGMISSING;
                else{ ULONG numfound;
                    if (! (numfound = DicServ_SearchEntry( DicHandle, ARG_NAME, ARG_STRING, flags, &ap->RexxResult )))
                        errmsg = TEXT_Search_NotFound;
                    else if (numfound == 1 && (flags & FUF_TRANSLATE))
                        {
                        for (ptr=ap->RexxResult; *ptr; ptr++)
                            {
                            while (*ptr && *ptr != ' ') ptr++;
                            if (*(ptr+1) == ':' && *(ptr+2) == ' ')
                                {
                                BufCutS( &ap->RexxResult, 0, ptr+3 - ap->RexxResult );
                                break;
                                }
                            }
                        }
                    }
                }
            else errmsg = TEXT_PARSE_REQARGMISSING;
            }
        else errmsg = TEXT_PARSE_REQARGMISSING;
        }

    if (errdic) errmsg = ErrTable[errdic];
    if (errmsg) AZ_SetCmdResult( co, 0, errmsg, 0 );
}
