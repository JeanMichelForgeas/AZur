/****************************************************************
 *
 *      File:      deffile.c    lecture du fichier de définitions
 *      Project:   AZUR         editeur de texte
 *
 *      Created:   30-11-92     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"
#include "uiki:objtimer.h"

#include "lci:arg3_protos.h"

#include "lci:objmenu.h"
#include "lci:azur.h"
#include "lci:eng_obj.h"
#include "lci:objfile.h"
#include "lci:azur_protos.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/


/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static void add_file( struct read_file *rf, UBYTE **start, UBYTE **end )
{
  ULONG file, len, readlen, oldsize, offs1, offs2;
  UBYTE *name;

    offs1 = *start - rf->mem;
    offs2 = *end - rf->mem;
    name = *start + skip_char( *start, ' ', '\t' );

    if (file = Open( name, MODE_OLDFILE ))
        {
        if (len = lo_FileSize( name ))
            {
            oldsize = intAZ_LineBufLen( rf->mem );
            if (BufResizeS( &rf->mem, oldsize + len + 1 ))
                {
                readlen = Read( file, rf->mem + oldsize, len );
                if (readlen == -1) readlen = 0;
                if (readlen != len) BufResizeS( &rf->mem, oldsize + readlen +1 );
                rf->mem[oldsize + readlen] = '\n'; // pour éviter les # collés aux entêtes
                *start = rf->mem + offs1;
                *end = rf->mem + offs2;
                rf->max = rf->mem + intAZ_LineBufLen( rf->mem );
                }
            }
        Close( file );
        }
    else eng_ShowRequest( UIK_LangString( AGP.UIK, TEXT_ERR_NotFound ), TEXT_ERR_NotFound, "Include" , name, 0 );
}

static ULONG link( struct read_file *rf, ULONG offs )
{
  ULONG len, cutoffs=-1, cutlen=0;
  UBYTE *start, *end;

    rf->ptr = rf->mem + offs;

    while (rf->ptr < rf->max)
        {
        if (len = StrLen( rf->ptr ))
            {
            if (ARG_MatchArg( rf->ptr, "INCLUDES:" ) == 0 || ARG_MatchArg( rf->ptr, "LINKS:" ) == 0)
                {
                cutoffs = rf->ptr - rf->mem;
                start = end = rf->ptr + len + 1;
                while (end < rf->max && *end != '#') end++;
                cutlen = end + StrLen(end) - rf->ptr;
                while (start < end)
                    {
                    if (len = StrLen( start )) add_file( rf, &start, &end );
                    start += len + 1;
                    }
                break;
                }
            }
        rf->ptr += len + 1;
        }
    if (cutlen)
        {
        BufCutS( &rf->mem, cutoffs, cutlen );
        rf->max = rf->mem + intAZ_LineBufLen( rf->mem );
        }
    return( cutoffs );
}

//------------------------------------------------------------

static UBYTE *cutbuf( struct read_file *rf, UBYTE *p, UBYTE *start )
{
  ULONG offs = start - rf->mem, len = p - start;
  ULONG offs2 = rf->linestart - rf->mem;

    BufCutS( &rf->mem, offs, len );
    rf->max = rf->mem + intAZ_LineBufLen( rf->mem );
    rf->linestart = rf->mem + offs2;
    return( rf->mem + offs );
}

void strip_comments( struct read_file *rf, ULONG offs, ULONG verify )
{
  UBYTE *p, *start, quoting=0, *prev;
  ULONG comments=0;

    p = start = rf->mem + offs;
    rf->linestart = p + skip_char( p, ' ', '\t' );

    while (p < rf->max)
        {
        if (comments == 0 && *p == '\n')
            {
            *p++ = 0; prev = p - 2;
            if (p >= rf->max) break;

            if (quoting && verify)
                {
                rf->linestart += skip_char( rf->linestart, ' ', '\t' );
                eng_ShowRequest( UIK_LangString( AGP.UIK, TEXT_ERR_MatchQuotes ), TEXT_ERR_MatchQuotes, rf->linestart, 0, 0 );
                quoting = 0;
                }

            if (prev >= rf->mem && *prev == '\\')
                {
                ULONG len = skip_char( p, ' ', '\t' );
                if (len > 1) p += (len - 1);
                p = cutbuf( rf, p, prev );
                }
            rf->linestart = p;
            }
        else if (quoting == 0 && *p == '/' && *(p+1) == '*')
            {
            if (comments == 0) start = p;
            p+=2;
            comments++;
            }
        else if (comments && *p == '*' && *(p+1) == '/')
            {
            p+=2;
            comments--;
            if (comments == 0) p = cutbuf( rf, p, start );
            }
        else if ((*p == 0x22 || *p == 0x27) && comments == 0)
            {
            if (quoting == 0) quoting = *p;
            else if (quoting == *p) quoting = 0;
            p++;
            }
        else{
            p++;
            }
        }

    if (comments)
        {
        BufCutS( &rf->mem, start - rf->mem, -1 );
        rf->max = rf->mem + intAZ_LineBufLen( rf->mem );
        }
}

//-------------------------------------------------------------------------

static ULONG interpret_vide( struct read_file *rf, ULONG append )
{
  ULONG len;

    while (!rf->diese && rf->ptr < rf->max)
        {
        rf->ptr += skip_char( rf->ptr, ' ', '\t' );
        if (len = StrLen( rf->ptr ))
            {
            if (*rf->ptr == '#') rf->diese = 1;
            }
        rf->ptr += len + 1;
        }
    return(1);
}

static ULONG interpret( UBYTE *filename, struct read_file *rf )
{
  ULONG len, append;
  UBYTE *name;

    rf->ptr = rf->mem;

    while (rf->ptr < rf->max)
        {
        rf->diese = 1;
        name = filename;
        len = StrLen( rf->ptr ) + 1;
        append = (ARG_MatchArg( rf->ptr, "ADD" ) == 1) || (ARG_MatchArg( rf->ptr, "APPEND" ) == 1) ? 1 : 0;

        if (len == 1)
            {
            rf->ptr++;
            }
        else if (ARG_MatchArg( rf->ptr, name="CMDEQUALS:" ) == 0 || ARG_MatchArg( rf->ptr, name="ALIAS:" ) == 0)
            {
            rf->ptr += len; rf->diese = 0;
            if (! interpret_equals( rf, append )) goto END_ERROR;
            }
        else if (ARG_MatchArg( rf->ptr, name="MAIN_MENUS:" ) == 0)
            {
            rf->ptr += len; rf->diese = 0;
            if (! interpret_menus( rf, append, 1 )) goto END_ERROR;
            }
        else if (ARG_MatchArg( rf->ptr, name="MENUS:" ) == 0)
            {
            rf->ptr += len; rf->diese = 0;
            if (! interpret_menus( rf, append, 0 )) goto END_ERROR;
            }
        else if (ARG_MatchArg( rf->ptr, name="KEYS:" ) == 0 || ARG_MatchArg( rf->ptr, name="KEYBOARD:" ) == 0)
            {
            rf->ptr += len; rf->diese = 0;
            if (! interpret_keys( rf, append )) goto END_ERROR;
            }
        else if (ARG_MatchArg( rf->ptr, name="HOTKEYS:" ) == 0 || ARG_MatchArg( rf->ptr, name="HOT_KEYS:" ) == 0)
            {
            rf->ptr += len; rf->diese = 0;
            if (! interpret_vide( rf, append )) goto END_ERROR;
            }
        else if (ARG_MatchArg( rf->ptr, name="MOUSE_JOY:" ) == 0 || ARG_MatchArg( rf->ptr, name="MOUSE:" ) == 0)
            {
            rf->ptr += len; rf->diese = 0;
            if (! interpret_mouse( rf, append )) goto END_ERROR;
            }
        else if (ARG_MatchArg( rf->ptr, name="TIMER:" ) == 0)
            {
            rf->ptr += len; rf->diese = 0;
            if (! interpret_vide( rf, append )) goto END_ERROR;
            }
        else if (ARG_MatchArg( rf->ptr, name="DICTIONARY:" ) == 0)
            {
            rf->ptr += len; rf->diese = 0;
            if (! interpret_vide( rf, append )) goto END_ERROR;
            }
        else if (ARG_MatchArg( rf->ptr, name="TEMPLATES:" ) == 0)
            {
            rf->ptr += len; rf->diese = 0;
            if (! interpret_vide( rf, append )) goto END_ERROR;
            }
        else{
            rf->ptr += len;
            }
        if (!rf->diese) eng_ShowRequest( UIK_LangString( AGP.UIK, TEXT_ERR_NotFound ), TEXT_ERR_NotFound, name, "#", 0 );
        }
    return(1);

  END_ERROR:
    eng_ShowRequest( UIK_LangString( AGP.UIK, TEXT_ERR_ParmLine ), TEXT_ERR_ParmLine, name, rf->ptr, 0 );
    return(0);
}

static ULONG ParmRead( UBYTE *filename )
{
  struct read_file RF;
  ULONG file, size, errmsg=0, offs=0;

    if (! (file = Open( filename, MODE_OLDFILE )))
        {
        ULONG save = UIK_DisableDOSReq();
        filename = "AZur:Parms/AZur.par";
        if (! (file = Open( filename, MODE_OLDFILE ))) errmsg = TEXT_ERR_OPENFILE;
        UIK_EnableDOSReq( 0, save );
        }
    if (errmsg) goto END_ERROR;

    if (size = lo_FileSize( filename ))
        {
        if (RF.mem = BufAllocP( AGP.Pool, size, 500 ))
            {
            if (Read( file, RF.mem, size ) == size)
                {
                RF.max = RF.mem + size;
                RF.menus = 0;
                RF.keys = 0;
                while (TRUE)
                    {
                    strip_comments( &RF, offs, 1 );
                    if ((offs = link( &RF, offs )) == -1) break;
                    }
/*{
ULONG out = Open( "ram:x", MODE_NEWFILE );
if (out)
    {
    ULONG i, len = BufLength( RF.mem );

    for (i=0; i < len; i++) if (RF.mem[i] == 0) RF.mem[i] = '\n';
    Write( out, RF.mem, len );
    for (i=0; i < len; i++) if (RF.mem[i] == '\n') RF.mem[i] = 0;
    Close( out );
    }
}*/
                interpret( filename, &RF );
                }
            else errmsg = TEXT_ERR_READFILE;
            BufFree( RF.mem );
            }
        else errmsg = TEXT_NOMEMORY;
        }

    Close( file );

  END_ERROR:
    return( errmsg );
}

//-----------------------------------------

ULONG ParmInit()        //------ étape initiale
{
    if (! Parm_InitEqual()) goto END_ERROR;
    if (! Parm_InitMainMenu()) goto END_ERROR;
    if (! Parm_InitMenu()) goto END_ERROR;
    if (! Parm_InitKey()) goto END_ERROR;
    if (! Parm_InitMouse()) goto END_ERROR;
    return(1);
  END_ERROR:
    return(0);
}

void ParmCleanup()      //------ étape terminale
{
    Parm_CleanupMouse(1);
    Parm_CleanupKey(1);
    Parm_CleanupMenu(1);
    Parm_CleanupMainMenu(1);
    Parm_CleanupEqual(1);
}

ULONG func_ReadParms( UBYTE *filename ) //------ lit le fichier et installe les défauts
{
  ULONG errmsg=0, def=1;

    if (StrNCCmp( filename, "NO" ) == 0) def = 0;
    else{
        if (StrNCCmp( filename, "DEFAULT" ) == 0) filename = "AZur:Parms/AZur.par";
        if (errmsg = ParmRead( filename )) goto END_ERROR;
        }
    if (! Parm_VerifEqual( def )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
    if (! Parm_VerifMainMenu( def )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
    if (! Parm_VerifMenu( def )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
    if (! Parm_VerifKey( def )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
    if (! Parm_VerifMouse( def )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }

  END_ERROR:
    return( errmsg );
}
