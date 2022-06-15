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

#include <clib/macros.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "uiki:uiklist.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "Azur:DevelKit/Includes/funcs.h"
#include "Azur:DevelKit/Includes/azur.h"
#include "Azur:DevelKit/Includes/objfile.h"
#include "Azur:DevelKit/Includes/obj.h"
#include "Azur:DevelKit/Includes/errors.h"

#include "AZur:/DIC/dicserv.h"
#include "azdic.h"


/****** Imported ************************************************/


/****** Statics ************************************************/

static void AZFunc_Load( struct CmdObj *co );
static void AZFunc_Unload( struct CmdObj *co );
static void AZFunc_Do( struct CmdObj *co );

static UBYTE * __far CmdNames[] = { "AZDicTrans", 0 };

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
static WORD __far left, __far top, __far width, __far height;


/****** Exported ***********************************************/

struct Library * __far DOSBase;
struct Library * __far SysBase;
struct Library * __far UIKBase;

struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,       (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,(ULONG) "DIC=FROMDIC=NAME=DICNAME/A/M,TO=TODIC/K,LINE/S,FL=FROMLINE/S,ALL/S,REP=REPLACE/S",
    AZT_Gen_Flags,      AZAFF_DOMODIFY | AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionLoad,     AZFunc_Load,
    AZT_ActionUnload,   AZFunc_Unload,
    AZT_ActionDo,       AZFunc_Do,
    TAG_END
    };

#define ARG_FROMDIC     co->ArgRes[0]
#define ARG_TODIC       co->ArgRes[1]
#define ARG_LINE        co->ArgRes[2]
#define ARG_FROMLINE    co->ArgRes[3]
#define ARG_ALL         co->ArgRes[4]
#define ARG_REPLACE     co->ArgRes[5]


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

//--------------------------------------------------------------

#define TRANSF_REPLACE      0x0001
#define TRANSF_REFRESH      0x0002
#define TRANSF_MOVECURS     0x0004

static LONG end_of_word( UBYTE *ptr, ULONG len, LONG col )
{
    if (col >= len) return(-1);
    for (; AZ_CharIsWord( ptr[col] ); ) { if (++col >= len) return(-1); }
    if (ptr[col] == '\'') col++;
    return( col );
}

static LONG start_of_word( UBYTE *ptr, ULONG len, LONG col )
{
    if (col >= len)  return(-1);
    if (! AZ_CharIsWord( ptr[col] )) return(-1);
    for (; col > 0; col--) if (! AZ_CharIsWord( ptr[col-1] )) break;
    return( col );
}

static LONG next_word( UBYTE *ptr, ULONG len, LONG col )
{
    if (col >= len) return(-1);
    for (; !AZ_CharIsWord( ptr[col] ); ) { if (++col >= len) return(-1); }
    return( col );
}

static void func_ReqListOK( struct AZurProc *ap, UBYTE *lineptr, struct AZRequest *ar )
{
    BufSetS( lineptr, StrLen(lineptr), &ap->RexxResult );
    AZ_ReqClose( 0, ar );
}
static void func_ReqListCancel( struct AZurProc *ap, struct AZRequest *ar )
{
    AZ_ReqClose( 0, ar );
}

static ULONG TransWord( UBYTE **from, UBYTE **totab, LONG line, LONG scol, ULONG flags )
{
  struct AZurProc *ap = AZ_GetAZur();
  struct AZBlock *azb;
  LONG ecol, len;
  ULONG errmsg=0, errdic=0;
  UBYTE *ptr, *str2, name[43], ch;

    name[0] = 0;
    ecol = scol;

    if (azb = AZ_BlockPtr( AZ_BlockCurrent() ))
        {
        line = azb->LineStart; scol = ecol = azb->ColStart;
        if (line == azb->LineEnd)
            {
            ptr = AZ_LineBuf( line );
            ecol = azb->ColEnd;
            if ((len = ecol - scol + 1) > 0)
                {
                if (++len > 40) len = 40;
                StcCpy( name, &ptr[scol], MIN(40,len) );
                AZ_BlockClearAll();
                }
            }
        }

    if (name[0] == 0)
        {
        RETESTE:
        ptr = AZ_LineBuf( line );
        len = AZ_LineBufLen( ptr );
        if (scol >= len) { goto END_ERROR; }

        if (AZ_CharIsWord( ptr[scol] ))
            {
            ecol = scol;
            for (; scol > 0; scol--) if (! AZ_CharIsWord( ptr[scol-1] )) break;
            for (; ecol < len; ecol++) if (! AZ_CharIsWord( ptr[ecol+1] )) break;
            if (ptr[ecol+1] == '\'') ecol++;
            if ((len = ecol - scol + 2) > 40) len = 40;
            StcCpy( name, &ptr[scol], MIN(40,len) );
            }
        }

    if (name[0])
        {
        ULONG numfound, button, namelen;

        namelen = StrLen( name );
        BufSetS( name, namelen, &ap->UBufTmp ); // pour les titres et AddEntry
        BufTruncate( ap->RexxResult, 0 );

        if (! (numfound = DicServ_SearchEntry( DicHandle, from, name, FUF_TRANSLATE, &ap->RexxResult )))
            {
            if (namelen > 1)
                {
                StrCat( name, "*" ); // pour toutes les solutions
                numfound = DicServ_SearchEntry( DicHandle, from, name, FUF_TRANSLATE, &ap->RexxResult );
                }
            }

        ASK_ADDENTRY:
        if (! numfound) // si pas trouvé dans ceux en entrée, on demande une requête
            {
            if (*totab)
                {
                AZ_CursorPos( line, scol );  // -1 pour enlever le '*' de fin
                button = AZ_SmartRequest( CmdNames[0], "Définition d'une nouvelle entrée pour :", "Le dictionnaire",&ap->UBufTmp, "La traduction",&ap->RexxResult, "_Enrichir", "_Sauter", (APTR)-1, 0, ARF_WINDOW|ARF_ACTIVATE );
                if (button == REQBUT_CTRL_C || button == REQBUT_CANCEL) { errmsg = TEXT_CMDCANCELLED; goto END_ERROR; }
                if (button == REQBUT_OK)    // si bouton "Enrichir" on écrit le nouveau mot puis on reteste
                    {
                    str2 = ap->RexxResult;
                    while (*str2)
                        {
                        for (ptr=str2; *ptr && *ptr != '/'; ptr++) ;
                        if (*ptr == '/') { *ptr++ = 0; }
                        if (errdic = DicServ_AddEntry( DicHandle, totab, ap->UBufTmp, str2, 0 )) goto END_ERROR;
                        str2 = ptr;
                        }
                    goto RETESTE;
                    }
                else // "Sauter"
                    { numfound = 0; }
                }
            else { errmsg = TEXT_Search_NotFound; goto END_ERROR; } // on ne peut pas enrichir
            }

        if (numfound > 1 && TRANSF_REPLACE)
            {
            struct AZRequest AR;

            __builtin_memset( &AR, 0, sizeof(struct AZRequest) );
            AR.Left = left; AR.Top = top; AR.Width = width; AR.Height = height;

            AZ_CursorPos( line, scol );
            if (errmsg = AZ_ReqList( &AR, ART_ReqTitle,   ap->UBufTmp,
                                          ART_ReqFlags,   ARF_MEMPOS,
                                          ART_ListInit,   ap->RexxResult,
                                          ART_OKFunc,     func_ReqListOK,
                                          ART_CancelFunc, func_ReqListCancel,
                                          ART_UserData,   ap,
                                          TAG_END ))
                {
                if (errmsg != TEXT_CMDCANCELLED) goto END_ERROR;
                else{   // si répond CANCEL à la liste, on peut vouloir créer une autre entrée...
                    numfound = 0;
                    BufTruncate( ap->RexxResult, 0 );
                    goto ASK_ADDENTRY;
                    }
                }
            numfound = 1;
            left = AR.Left; top = AR.Top; width = AR.Width; height = AR.Height;
            }

        if (numfound == 1)
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

            if (flags & TRANSF_REPLACE)
                {
                AZ_TextDel( line, ecol+1, scol-ecol-1, 0, 0 );
                if (len = BufLength( ap->RexxResult ))
                    {
                    AZ_TextPrep( line, scol, ARPF_CONVERT|ARPF_NOOVERLAY, 0, 0 );
                    AZ_TextPut( ap->RexxResult, len, 0, 0 );
                    }
                AZ_SetModified( line, scol );
                if (flags & TRANSF_REFRESH) AZ_Display( line, 1, 0, -1 );
                ecol = scol + len - 1;
                }
            }
        }

    if (flags & TRANSF_MOVECURS) // on avance sur le prochain mot même si on n'était pas sur un mot
        {
        ptr = AZ_LineBuf( line );
        len = AZ_LineBufLen( ptr );
        // si le mot suivant touche, ne pas sauter le mot. Cas du remplacement du "l'"
        // while ((ptr = AZ_CharNext( ptr, &len, &line, &ecol, &ch )) != (APTR)-1) if (! AZ_CharIsWord( ch )) break;
        while ((ptr = AZ_CharNext( ptr, &len, &line, &ecol, &ch )) != (APTR)-1) if (AZ_CharIsWord( ch )) break;
        if (ptr != (APTR)-1) AZ_CursorPos( line, ecol );
        }

  END_ERROR:
    if (errdic) errmsg = ErrTable[errdic];
    return( errmsg );
}

static ULONG TransLine( UBYTE **from, UBYTE **to, LONG startline, LONG endline, LONG startcol, ULONG flags )
{
  struct AZurProc *ap = AZ_GetAZur();
  LONG line, col, ecol;
  ULONG errmsg, len, copylen, modifs=0;
  UBYTE *ptr, *ubuf=0, **plinebuf, ch;

    ptr = AZ_LineBuf( startline );
    len = AZ_LineBufLen( ptr );
    if ((col = start_of_word( ptr, len, startcol )) == -1)
        {
        while ((ptr = AZ_CharNext( ptr, &len, &startline, &startcol, &ch )) != (APTR)-1) if (AZ_CharIsWord( ch )) break;
        if (ptr == (APTR)-1) goto END_ERROR; // fin du texte
        }
    else startcol = col;

    if (ubuf = BufAllocP( ap->Pool, 0, 200 ))
        {
        for (line=startline; line <= endline; line++)
            {
            col = startcol;
            BufTruncate( ubuf, 0 );
            ptr = AZ_LineBuf( line );
            len = AZ_LineBufLen( ptr );
            while (col != -1)
                {
                //------ Ajout de la traduction dans le buffer
                BufTruncate( ap->RexxResult, 0 );
                if (errmsg = TransWord( from, to, line, col, 0 )) goto END_ERROR;
                if (! BufPasteS( ap->RexxResult, BufLength(ap->RexxResult), &ubuf, BufLength(ubuf) )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }

                //------ Ajout de l'intervalle dans le buffer
                if ((ecol = end_of_word( ptr, len, col )) == -1) break;
                if ((col = next_word( ptr, len, ecol )) == -1) copylen = len - ecol; else copylen = col - ecol;
                if (! BufPasteS( ptr+ecol, copylen, &ubuf, BufLength(ubuf) )) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
                }
            //------ Remplacement de la ligne
            if (flags & TRANSF_REPLACE)
                {
                plinebuf = &((struct UIKPList *)&ap->FObj->Text)->UNode[line].Buf;
                BufTruncate( *plinebuf, startcol );
                BufPasteS( ubuf, BufLength(ubuf), plinebuf, startcol );
                modifs++;
                if (flags & TRANSF_REFRESH) AZ_Display( line, 1, 0, -1 );
                }
            startcol = 0;
            if (line % 10 == 0) STATCHANGED(ap->FObj);
            }
        if (flags & TRANSF_MOVECURS) AZ_CursorPos( line-1, AZ_LineLen( line-1 ) );
        }
  END_ERROR:
    BufFree( ubuf );
    if (modifs) AZ_SetModified( endline, 0 );
    return( errmsg );
}
// azdictrans t:dic azur:dics/fr-en to azur:dics/fr-en rep

static void AZFunc_Do( struct CmdObj *co )
{
  struct AZurProc *ap = AZ_GetAZur();
  ULONG flags, lastline, errmsg=0, errdic=0, i;
  UBYTE *totab[2];

    if (! DicHandle)
        {
        if (! start_dicserv(1))
            {
            UBYTE buf[200];
            sprintf( buf, UIK_LangString( ap->UIK, TEXT_ERR_NotFound ), "AZur:Utilities/DicServ", ":" );
            AZ_ReqShowText( buf );
            goto END_ERROR;
            }
        }

    flags = TRANSF_REFRESH;
    if (ARG_REPLACE)   flags |= TRANSF_REPLACE;

    totab[0] = ARG_TODIC; totab[1] = 0;
    if (ARG_TODIC)
        {
        totab[0] = ARG_TODIC;
        if (! DicServ_Find( DicHandle, ARG_TODIC ))
            {
            if (errdic = DicServ_Load( DicHandle, totab ))
                {
                if (errdic == ERR_NOTFOUND) errdic = DicServ_Create( DicHandle, totab );
                }
            }
        if (errdic) goto END_ERROR;
        }
    if (ARG_FROMDIC)
        {
        for (i=0; ((UBYTE**)ARG_FROMDIC)[i]; i++) // si manque 1 on essaye de charger le tout
            {
            if (! DicServ_Find( DicHandle, ((UBYTE**)ARG_FROMDIC)[i] ))
                {
                DicServ_Load( DicHandle, ARG_FROMDIC );
                break;
                }
            }
        }

    lastline = AZ_NumLines() - 1;
    if (ARG_ALL)
        {
        AZ_BlockClearAll();
        errmsg = TransLine( ARG_FROMDIC, totab, 0, lastline, 0, flags|TRANSF_MOVECURS );
        }
    else if (ARG_FROMLINE)
        {
        AZ_BlockClearAll();
        errmsg = TransLine( ARG_FROMDIC, totab, ap->FObj->Line, lastline, ap->FObj->Col, flags|TRANSF_MOVECURS );
        }
    else if (ARG_LINE)
        {
        AZ_BlockClearAll();
        errmsg = TransLine( ARG_FROMDIC, totab, ap->FObj->Line, ap->FObj->Line, ap->FObj->Col, flags|TRANSF_MOVECURS );
        }
    else{
        errmsg = TransWord( ARG_FROMDIC, totab, ap->FObj->Line, ap->FObj->Col, flags|TRANSF_MOVECURS );
        }

  END_ERROR:
    UPDATE_STAT_PROP(ap->FObj);
    if (errdic) errmsg = ErrTable[errdic];
    if (errmsg) AZ_SetCmdResult( co, 0, errmsg, 0 );
    else BufTruncate( ap->RexxResult, 0 );
}
