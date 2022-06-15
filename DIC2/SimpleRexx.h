/* Simple ARexx interface by Michael Sinz
 * This is a very "Simple" interface...
 */

#ifndef SIMPLE_REXX_H
#define SIMPLE_REXX_H

#include <rexx/errors.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>

/* A structure for the ARexx handler context
 * This is *VERY* *PRIVATE* and should not be touched...
 */
struct ARexxContext
{
    struct MsgPort  *ARexxPort;     /* The port messages come in at... */
    struct Library  *RexxSysBase;   /* We will hide the library pointer here... */
    long            Outstanding;    /* The count of outstanding ARexx messages... */
    char            PortName[24];   /* The port name goes here... */
    char            ErrorName[34];  /* The name of the <base>.LASTERROR... */
    char            ErrorTextName[38]; /* The name of the <base>.LASTERRORTEXT... */
    char            Extension[8];   /* Default file name extension... */
};
#define AREXXCONTEXT struct ARexxContext *

/* The value of RexxMsg (from GetARexxMsg) if it was our own message
 */
#define REXX_DO_NOT_REPLY ((struct RexxMsg *)-1L)

void FreeARexx( AREXXCONTEXT );
AREXXCONTEXT InitARexx( char *, char *, char, ULONG );
char *ARexxName( AREXXCONTEXT );
ULONG ARexxSignal( AREXXCONTEXT );
struct RexxMsg *GetARexxMsg( AREXXCONTEXT, ULONG, void *, APTR * );
void ReplyARexxMsg(  AREXXCONTEXT, struct RexxMsg *, LONG, char *, LONG, char * );
struct RexxMsg *SendARexxMsg( AREXXCONTEXT, char *, short );
struct RexxMsg *SendARexxMsgCustom( AREXXCONTEXT, char *, long, long, long );
short SetARexxLastError( AREXXCONTEXT, struct RexxMsg *, long, char * );

#endif
