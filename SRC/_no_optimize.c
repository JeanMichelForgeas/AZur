/****************************************************************
 *
 *      File:       funcs_text.c    manipule le texte
 *      Project:    AZur            editeur de texte
 *
 *      Created:    17/06/93        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"

/* this macro lets us long-align structures on the stack */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((LONG)(a_##name+3) & ~3);

#define DBUG(a)


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/


/****** Statics *************************************************/


/****************************************************************
 *
 *      Utils
 *
 ****************************************************************/

UBYTE *func_BufPadS( UBYTE ch, ULONG numch, UBYTE **pub, ULONG start )
{
  UBYTE *ptr = *pub;
  ULONG len = BufLength( ptr );

    if (numch)
        {
        if (start > len) numch += (start - len);
        if (ptr = BufResizeS( pub, len + numch ))
            {
            UBYTE *copy = ptr + start, *max = copy + numch;
            if (start < len) UIK_MemCopy( copy, max, len-start );
            for (; copy < max; ) *copy++ = ch;
            }
        }
    return( ptr );
}
