/****************************************************************
 *
 *      Project:    DicServ
 *
 *      Created:    21/12/94    Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/*
#include "arg3.h"
#include "arg3_protos.h"
*/

#include "dicserv.h"
#include "simplerexx.h"
#include "_pool_protos.h"

#include "Projects:UIK/INCLUDES/uik_protosint.h"


/****** Imported ************************************************/

extern ULONG DOSBase;
extern ULONG SysBase;


/****** Exported ***********************************************/

void main( ULONG argc, char **argv );

APTR IntuitionBase;

struct SignalSemaphore DicSem;
struct List DicList;
APTR Pool;
APTR ARexx;
ULONG UseCount, StopWait;
struct Task *Process;


/****** Statics ************************************************/

static void warn( UBYTE *fmt, ULONG p1, ULONG p2 );

/*
static UBYTE Template[] = "SCRIPT/A,SYNC/S,U=UNIT/N,S=SPEED/N,W=WIRES/N,PO=PARITYON/S,DB=DATABITS/N,SB=STOPBITS/N,BL=BUFLEN/N,DONOTWARN/S,PN=PORTNAME/K,SC=SCRIPTDIR/K,REPORT/K";
static struct ParseArgs PArg;
*/
static ULONG FromShell;
static struct DicPort Port;

static UBYTE versiontag[] = "\0$VER: DicServ 0.1 (21.12.94)";

/*
static UBYTE *TextMsg[] =
    {
    "parsing ok",
    "*** Line too long",
    "*** Bad template",
    "*** Required argument missing",
    "*** Argument line invalid or too long",
    "*** Argument after '=' or keyword missing",
    "*** Not enough memory",
    "*** Too many arguments",
    "*** Bad number",
    "*** No command line",
    0
    };
*/


/****************************************************************
 *
 *      Master Code
 *
 ****************************************************************/

void main( ULONG argc, char **argv )
{
  /*
  UBYTE *cmdline;
  APTR parse;
  ULONG rexx_event;
  */
  struct MsgPort *otherport;
  ULONG rc, err=0;

    Forbid();
    if (otherport = FindPort( PORTNAME ))
        {
        err = 1;
        Signal( otherport->mp_SigTask, SIGBREAKF_CTRL_C );
        }
    else{
        Port.Port.mp_Node.ln_Type = NT_MSGPORT;
        Port.Port.mp_Node.ln_Pri = -10;
        Port.Port.mp_Node.ln_Name = PORTNAME;
        Port.Port.mp_Flags = PA_SIGNAL;
        Port.Port.mp_SigBit = SIGBREAKB_CTRL_E;
        Port.Port.mp_SigTask = FindTask(0);
        AddPort( &Port );
        }
    Permit();
    if (err) exit(20);

    rc = 20;
    FromShell = argc;

    InitSemaphore( &DicSem );
    NewList( &DicList );
    Process = FindTask(0);

    if (IntuitionBase = OpenLibrary( "intuition.library", 0 ))
        {
        if (Pool = AsmCreatePool( MEMF_CLEAR, 4096, 4096, *((APTR*)4) ))
            {
            /*
            if (cmdline = ARG_BuildCmdLine( argc, argv, 0, Template ))
                {
                PArg.CmdLine = cmdline;
                PArg.CmdLength = strlen( cmdline );
                PArg.Flags = PAF_PROMPT;
                parse = ARG_Parse( Template, &Res, &PArg, argc, DOSBase );

                if (PArg.Error == PAERR_OK)
                    {
                    if (! (argc == 2 && argv[1][0] == '?'))
                        {
                        if (Res.PortName  == 0) Res.PortName  = REXXNAME;
                        if (Res.ScriptDir == 0) Res.ScriptDir = "RAM:";
                        if (ARexx = InitARexx( Res.PortName, 0, 0, 1 ))
                            {
                            rc = 0;
                            RexxInit();
                            rexx_event = ARexxSignal( ARexx );
                            */
                            //warn( "DicServ: running", 0, 0 );

                            Port.Lock        = direct_Lock       ;
                            Port.Unlock      = direct_Unlock     ;
                            Port.Find        = direct_Find       ;
                            Port.Create      = direct_Create     ;
                            Port.Load        = direct_Load       ;
                            Port.Unload      = direct_Unload     ;
                            Port.UnloadAll   = direct_UnloadAll  ;
                            Port.Save        = direct_Save       ;
                            Port.AddEntry    = direct_AddEntry   ;
                            Port.DeleteEntry = direct_DeleteEntry;
                            Port.SearchEntry = direct_SearchEntry;
                            Port.List        = direct_List       ;

                            while (TRUE)
                                {
                                ULONG events = Wait( /*rexx_event |*/ SIGBREAKF_CTRL_C );
                                /*
                                if (events & rexx_event)
                                    {
                                    RexxASyncReceived(0);
                                    }
                                */
                                if (events & SIGBREAKF_CTRL_C)
                                    {
                                    Forbid();
                                    ObtainSemaphore( &DicSem );     // attend que dernière fonction terminée
                                    ReleaseSemaphore( &DicSem );
                                    if (UseCount == 0)
                                        {
                                        RemPort( &Port );   // plus de port de rendez-vous
                                        Permit();
                                        break;
                                        }
                                    else{
                                        StopWait = 1;
                                        Permit();
                                        }
                                    }
                                }
                            direct_UnloadAll(1);            // ferme tous les dictionnaires
                            /*
                            RexxCleanup();
                            FreeARexx( ARexx );
                            }
                        else warn( "DicServ: cannot open ARexx", 0, 0 );
                        }
                    }
                else warn( "DicServ: %ls", (ULONG)TextMsg[PArg.Error], 0 );

                if (parse) ARG_FreeParse( parse );
                ARG_FreeCmdLine( cmdline );
                }
            else warn( "DicServ: seems there is not enough memory", 0, 0 );
            */
            AsmDeletePool( Pool, *((APTR*)4) );
            }
        else warn( "DicServ: seems there is not enough memory", 0, 0 );
        CloseLibrary( IntuitionBase );
        }
    exit( rc );
}

static void warn( UBYTE *fmt, ULONG p1, ULONG p2 )
{
  UBYTE buf[200];
  ULONG len;
  struct IntuiText msg = { -1, -1, JAM1, 10, 10, NULL, NULL, NULL },
                   ok  = { -1, -1, JAM1, 6, 3, NULL, "OK", NULL };

    sprintf( buf, fmt, p1, p2 );
    len = strlen( buf );
    /*if (FromShell)
        {
        Write( Output(), buf, len );
        Write( Output(), "\n", 1 );
        }
    else*/{
        msg.IText = buf;
        AutoRequest( NULL, &msg, 0, &ok, 0, 0, (len+13) * 8, 50 );
        }
}
