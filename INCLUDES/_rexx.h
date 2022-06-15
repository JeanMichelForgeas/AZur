/*
 * Simple ARexx interface by Michael Sinz
 *
 * This is a very "Simple" interface...
 */

#ifndef SIMPLE_REXX_H
#define SIMPLE_REXX_H

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
    char            ErrorName[28];  /* The name of the <base>.LASTERROR... */
    char            Extension[8];   /* Default file name extension... */
};
#define AREXXCONTEXT struct ARexxContext *

/* The value of RexxMsg (from GetARexxMsg) if there was an error returned
 */
#define REXX_RETURN_ERROR ((struct RexxMsg *)-1L)

void FreeARexx( AREXXCONTEXT );
AREXXCONTEXT InitARexx( char *,char * );
char *ARexxName( AREXXCONTEXT );
ULONG ARexxSignal( AREXXCONTEXT );
struct RexxMsg *GetARexxMsg( AREXXCONTEXT, ULONG, void* );
void ReplyARexxMsg( AREXXCONTEXT,struct RexxMsg *,char *,LONG );
struct RexxMsg *SendARexxMsg( AREXXCONTEXT,char *,short );
short SetARexxLastError( AREXXCONTEXT, struct RexxMsg *, char * );

#endif
