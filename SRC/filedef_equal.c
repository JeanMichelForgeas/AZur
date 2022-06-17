/****************************************************************
 *
 *      File:      filedef_equal.c
 *      Project:   AZur
 *
 *      Created:   31-07-93     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:hash.h"
#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/


/****************************************************************
 *
 *      CmdEquals handling
 *
 ****************************************************************/

ULONG Parm_InitEqual()
{
    return(1);
}

ULONG Parm_CleanupEqual( ULONG quit )
{
  struct EqualHandle *eh = (struct EqualHandle *) AGP.AZMast->EqualHandle;
  APTR pool=0;

    if (eh)
        {
        AsmDeletePool( eh->Pool, ExecBase );
        AGP.AZMast->EqualHandle = 0;
        }
    if (! quit)
        {
        if (! (pool = AsmCreatePool( MEMF_ANY|MEMF_CLEAR, 1024, 1024, ExecBase ))) goto END_ERROR;
        if (! (eh = AsmAllocPooled( pool, sizeof(struct EqualHandle), ExecBase ))) goto END_ERROR;
        if (! UIK_nl_AllocListP( pool, &eh->EqualList, 20, 0, 0, 0 )) goto END_ERROR;
        eh->Pool = pool;
        AGP.AZMast->EqualHandle = eh;
        }
    return(1);

  END_ERROR:
    if (pool) AsmDeletePool( pool, ExecBase );
    return(0);
}

//-----------------------------------------

ULONG Parm_VerifEqual( ULONG defaut )
{
    return(1);
}

//-----------------------------------------

ULONG interpret_equals( struct read_file *rf, ULONG append )
{
  ULONG len;
  UBYTE *p, *equal, *truecmd;
  struct AZEqual *aq;
  struct EqualHandle *eh = AGP.AZMast->EqualHandle;

    if (append == 0 || eh == 0)
        {
        if (! Parm_CleanupEqual(0)) return(0);
        eh = AGP.AZMast->EqualHandle;
        }

    while (!rf->diese && rf->ptr < rf->max)
        {
        rf->ptr += skip_char( rf->ptr, ' ', '\t' );

        if (len = StrLen( rf->ptr ))
            {
            if (*rf->ptr == '#')
                {
                rf->diese = 1;
                }
            else{
                p = rf->ptr;
                //p += skip_char( p, ' ', '\t' );  // saute espaces entre début et equal
                if (*p)
                    {
                    if (*p != '"') goto END_ERROR;
                    equal = ++p;    // skip 1er "
                    p += find_char( p, '"', '"' );
                    if (*p)         // 2eme " ok
                        {
                        *p++ = 0;   // 2eme " = 0 -> equal OK

                        p += skip_char( p, ' ', '\t' );  // saute espaces entre equal et truecmd
                        if (*p)
                            {
                            UBYTE *buf;
                            ULONG len1, len2;

                            if (*p == '"')
                                {
                                UBYTE *s = ++p;
                                while (*s) s++;
                                if (*(s-1) == '"') *(s-1) = 0;
                                }
                            truecmd = p;

                            len1 = StrLen( equal );
                            len2 = StrLen( truecmd );
                            if (! (aq = (struct AZEqual *) UIK_nl_AllocNode( &eh->EqualList, -1, sizeof(struct AZEqual)+len1+len2+2, 0 ))) goto END_ERROR;
                            buf = ((UBYTE*)aq) + sizeof(struct AZEqual);
                            StrCpy( buf, equal );
                            StrCpy( buf+len1+1, truecmd );
                            aq->Equal = buf;
                            aq->TrueCmd = buf+len1+1;
                            }
                        }
                    }
                }
            }
        rf->ptr += len + 1;
        }

    return(1);

  END_ERROR:
    return(0);
}
