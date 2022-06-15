/* Simple ARexx interface by Michael Sinz
 *
 * This is a very "Simple" interface to the world of ARexx...
 * For more complex interfaces into ARexx, it is best that you
 * understand the functions that are provided by ARexx.
 * In many cases they are more powerful than what is presented
 * here.
 *
 * This code is fully re-entrant and self-contained other than
 * the use of SysBase/AbsExecBase and the ARexx RVI support
 * library which is also self-contained...
 */

/* Modified by Jean-Michel Forgeas for
 * - custom messages (input/output redirection)
 * - custom callback mechanism
 * - extended port naming control
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "SimpleRexx.h"

#include "arg3_protos.h"

/* The prototypes for the few ARexx functions we will call...
 */
struct RexxMsg *CreateRexxMsg(struct MsgPort *,char *,char *);
void *CreateArgstring(char *,long);
void DeleteRexxMsg(struct RexxMsg *);
void DeleteArgstring(char *);
BOOL IsRexxMsg(struct Message *);

/* Pragmas for the above functions...  (To make this all self-contained...)
 * If you use RexxGlue.o, this is not needed...
 *
 * These are for Lattice C 5.x  (Note the use of RexxContext->RexxSysBase)
 */
#pragma libcall RexxContext->RexxSysBase CreateRexxMsg 90 09803
#pragma libcall RexxContext->RexxSysBase CreateArgstring 7E 0802
#pragma libcall RexxContext->RexxSysBase DeleteRexxMsg 96 801
#pragma libcall RexxContext->RexxSysBase DeleteArgstring 84 801
#pragma libcall RexxContext->RexxSysBase IsRexxMsg A8 801

/* Prototypes for the RVI ARexx calls...  (link with RexxVars.o)
 */
long __stdargs CheckRexxMsg(struct RexxMsg *);
long __stdargs GetRexxVar(struct RexxMsg *,char *,char **);
long __stdargs SetRexxVar(struct RexxMsg *,char *,char *,long);

/****************************************************************************
 * This function returns the port name of your ARexx port.
 */
char *ARexxName( AREXXCONTEXT RexxContext )
{
    return( RexxContext ? (char*) RexxContext->PortName : 0 );
}

/****************************************************************************
 * This function returns the signal mask that the Rexx port is using.
 */
ULONG ARexxSignal( AREXXCONTEXT RexxContext )
{
  ULONG signal=0;

    if (RexxContext)
        {
        signal = (ULONG) 1L << RexxContext->ARexxPort->mp_SigBit;
        }
    return( signal ); //RexxContext ? (ULONG) 1L << RexxContext->ARexxPort->mp_SigBit : 0 );
}

/****************************************************************************
 * This function returns a RexxMsg that contains the commands sent or replied
 */
struct RexxMsg *GetARexxMsg( AREXXCONTEXT RexxContext, ULONG extobj, void (*func)(), APTR *pptr )
{
  struct RexxMsg *msg=NULL;

    if (RexxContext)
        {
        msg = (struct RexxMsg *) GetMsg( RexxContext->ARexxPort );
        if (pptr) *pptr = msg;
        if (msg)
            {
            if (msg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
                {
                if (extobj && func) (*func)( extobj, msg );

                DeleteArgstring( msg->rm_Args[0] );
                if ((msg->rm_Action & RXFF_RESULT) && !msg->rm_Result1 && msg->rm_Result2)
                    DeleteArgstring( (char*)msg->rm_Result2 );
                DeleteRexxMsg( msg );
                RexxContext->Outstanding -= 1;

                msg = REXX_DO_NOT_REPLY;
                }
            }
        }
    return( msg );
}

/****************************************************************************
 * Use this to return a ARexx message...
 * If you wish to return something, it must be in the ResultString.
 * If you wish to return an Error, it must be in the ResultCode.
 * If there is a ResultCode, the ResultString is ignored.
 */
void ReplyARexxMsg( AREXXCONTEXT RexxContext, struct RexxMsg *rmsg, LONG ResultCode, char *ResultString, LONG ErrorCode, char *ErrorString )
{
    if (RexxContext && rmsg && (rmsg != REXX_DO_NOT_REPLY))
        {
        rmsg->rm_Result1 = ResultCode;
        rmsg->rm_Result2 = 0;
        if ((rmsg->rm_Result1 == RC_OK) && (rmsg->rm_Action & RXFF_RESULT) && ResultString)
            {
            rmsg->rm_Result2 = (LONG) CreateArgstring( ResultString, (LONG)strlen( ResultString ) );
            }
        else if (rmsg->rm_Result1 != RC_OK)
            {
            rmsg->rm_Result2 = ErrorCode;
            }
        SetARexxLastError( RexxContext, rmsg, ErrorCode, ErrorString );
        ReplyMsg( rmsg );
        }
}

/****************************************************************************
 * This function will set an error string for the ARexx
 * application in the variable defined as <appname>.LASTERROR
 */
short SetARexxLastError( AREXXCONTEXT RexxContext, struct RexxMsg *rmsg, LONG ErrorCode, char *ErrorString )
{
    if (RexxContext && rmsg && CheckRexxMsg( rmsg ))
        {
        UBYTE buf[50]; stci_d( buf, ErrorCode );
        SetRexxVar( rmsg, RexxContext->ErrorName, buf, (long)strlen( buf ) );
        if (! ErrorString) ErrorString = "";
        SetRexxVar( rmsg, RexxContext->ErrorTextName, ErrorString, (long)strlen( ErrorString ) );
        }
    return( TRUE );
}

/****************************************************************************
 * This function will send a string to ARexx...
 * The default host port will be that of your task...
 */
static struct RexxMsg *reallysend( AREXXCONTEXT RexxContext, char *CmdString, LONG action, LONG in, LONG out )
{
  register struct MsgPort *RexxPort;
  register struct RexxMsg *rmsg=0;

    if (RexxContext && CmdString && action)
        {
        if (rmsg = CreateRexxMsg( RexxContext->ARexxPort, RexxContext->Extension, RexxContext->PortName ))
            {
            rmsg->rm_Action = action;
            if (in) rmsg->rm_Stdin = in;
            if (out) rmsg->rm_Stdout = out;
            if (rmsg->rm_Args[0] = CreateArgstring( CmdString, (LONG)strlen( CmdString ) ))
                {
                Forbid();
                if (RexxPort = FindPort( RXSDIR ))
                    {
                    PutMsg( RexxPort, rmsg );
                    RexxContext->Outstanding += 1;
                    }
                else{
                    DeleteArgstring( rmsg->rm_Args[0] );
                    DeleteRexxMsg( rmsg );
                    rmsg = NULL;
                   }
                Permit();
                }
            else{
                DeleteRexxMsg( rmsg );
                rmsg = NULL;
                }
            }
        }
    return( rmsg );
}

struct RexxMsg *SendARexxMsgCustom( AREXXCONTEXT RexxContext, char *CmdString, LONG action, LONG in, LONG out )
{
    return( reallysend( RexxContext, CmdString, action, in, out ) );
}
struct RexxMsg *SendARexxMsg( AREXXCONTEXT RexxContext, char *CmdString, short IsString )
{
    return( reallysend( RexxContext, CmdString, RXCOMM | (1L << RXFB_RESULT) | (IsString ? (1L << RXFB_STRING):0), 0, 0 ) );
}

/****************************************************************************
 * This function closes down the ARexx context that was opened
 * with InitARexx...
 */
void FreeARexx( AREXXCONTEXT RexxContext )
{
  register struct RexxMsg *rmsg;

    if (RexxContext)
        {
        RexxContext->PortName[0] = '\0';  // the port cannot be found

        while (RexxContext->Outstanding)
            {
            WaitPort( RexxContext->ARexxPort );
            while (rmsg = GetARexxMsg( RexxContext, 0, NULL, NULL ))
                {
                if (rmsg != REXX_DO_NOT_REPLY)
                    ReplyARexxMsg( RexxContext, rmsg, 100, NULL, 0, NULL );
                }
            }

        if (RexxContext->ARexxPort)
            {
            while (rmsg = GetARexxMsg( RexxContext, 0, NULL, NULL ))
                {
                ReplyARexxMsg( RexxContext, rmsg, 100, NULL, 0, NULL );
                }
            DeletePort( RexxContext->ARexxPort );
            }

        if (RexxContext->RexxSysBase) CloseLibrary( RexxContext->RexxSysBase );
        FreeMem( RexxContext, sizeof(struct ARexxContext) );
        }
}

/****************************************************************************
 * This routine initializes an ARexx port for your process
 * This should only be done once per process.  You must call it
 * with a valid application name and you must use the handle it
 * returns in all other calls...
 *
 * NOTE:  The AppName should not have spaces in it...
 *        Example AppNames:  "MyWord" or "FastCalc" etc...
 *        The name *MUST* be less that 16 characters...
 *        If it is not, it will be trimmed...
 *        The name will also be UPPER-CASED...
 *
 * NOTE:  The Default file name extension, if NULL will be
 *        "rexx"  (the "." is automatic)
 */

AREXXCONTEXT InitARexx( char *AppName, char *Extension, char dot, ULONG nonum )
{
  register AREXXCONTEXT RexxContext=NULL;
  register short count;
  register char *ptr;

    if (RexxContext = AllocMem( sizeof(struct ARexxContext), MEMF_PUBLIC|MEMF_CLEAR ))
        {
        if (RexxContext->RexxSysBase = OpenLibrary( "rexxsyslib.library", NULL ))
            {
            /* Set up the extension... */
            if (Extension == 0) Extension = "rexx";
            stccpy( RexxContext->Extension, Extension, 8 );
            ARG_StrToUpper( RexxContext->Extension );

            /* Set up a port name... */
            stccpy( RexxContext->PortName, AppName, 17 );
            ptr = ARG_StrToUpper( RexxContext->PortName ) - 1;

            /* Set up the last error RVI name... This is <appname>.LASTERROR */
            strcpy( RexxContext->ErrorName, RexxContext->PortName );
            strcat( RexxContext->ErrorName, ".LASTERROR" );

            /* Set up the last error RVI name... This is <appname>.LASTERRORTEXT */
            strcpy( RexxContext->ErrorTextName, RexxContext->PortName );
            strcat( RexxContext->ErrorTextName, ".LASTERRORTEXT" );

            /* We need to make a unique port name... */
            Forbid();
            if (nonum==0 || FindPort( RexxContext->PortName ))
                {
                if (dot) *ptr++ = dot;
                for (count=1, RexxContext->ARexxPort=(void*)1; RexxContext->ARexxPort; count++)
                    {
                    stci_d( ptr, count );
                    RexxContext->ARexxPort = FindPort( RexxContext->PortName );
                    }
                }
            RexxContext->ARexxPort = (struct MsgPort *)CreatePort( RexxContext->PortName, NULL );
            Permit();
            }
        if (!RexxContext->RexxSysBase || !RexxContext->ARexxPort)
            {
            FreeARexx( RexxContext );
            RexxContext = NULL;
            }
        }
    return( RexxContext );
}
