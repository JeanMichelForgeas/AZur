/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Ajoute des types de conversion supplémentaires
 *
 *
 *
 *      Created:    30/05/95 Jean-Michel Forgeas
 *
 *      NOTA BENE:
 *      Les fonctions de votre objet de commande sont appelées
 *      avec l'adresse de la structure de données générales
 *      (struct AZurProc) dans le registre A4 (on peut aussi
 *      l'obtenir avec la fonction AZ_GetAZur()).
 *
 *      Les fonctions de AZur attendent aussi dans A4 l'adresse
 *      de cette structure.
 *      Il est dond nécessaire de compiler en adressage relatif
 *      à A4, et de déclarer toutes vos variables (données) globales
 *      en "far" afin de ne pas modifier ce registre, si toutefois
 *      vous désirez appeler des fonctions AZur depuis votre
 *      objet de commande.
 *      Attention que les constantes non déclarées ne soient pas
 *      non plus adressées par A4.
 *
 *      Pour utiliser des fonctions de dos.library comme printf()
 *      il faut ouvrir vous-même cette librairie, car comme on ne
 *      link pas avec c.o ou autre startup normal, elle n'est pas
 *      ouverte automatiquement. Il n'y a pas de "stdout" défini
 *      non plus.
 *      On ne peut pas utiliser non plus fprintf( Output(), ...)
 *      car AZur a pu être lancé du Workbench.
 *      Vous pouvez par exemple ouvrir la dos.library puis ouvrir
 *      "CON:" pour écrire dedans.
 *      Mais il est beaucoup plus simple d'utiliser kprintf() en
 *      conjonction avec Sushi (Software Toolkit de Commodore).
 *
 *      SAS C 5.x
 *      ---------
 *      - Toutes variables globales et statiques en __far
 *      - Options de compilation : -b1
 *      - Ne jamais utiliser geta4
 *      - ne jamais déclarer des fonctions avec __saveds
 *
 *      DICE C
 *      ------
 *      - Toutes variables globales et statiques en __far
 *      - Options de compilation : -md -ms
 *      - Ne jamais utiliser geta4
 *      - ne jamais déclarer des fonctions avec __geta4
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

/* Cet exemple utilise les fonctions de manipulations de buffer
 * dynamiques de UIK et nécessite de posséder UIK pour pouvoir
 * compiler.
 */
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "/includes/funcs.h"
#include "/includes/azur.h"
#include "/includes/obj.h"
#include "/includes/objfile.h"
#include "/includes/errors.h"

#define ARG_CHAR       0
#define ARG_WORD       1
#define ARG_LINE       2
#define ARG_ALL        3
#define ARG_NOCURS     4
#define ARG_ISO2SUF    5
#define ARG_SUF2ISO    6


/****** Imported ************************************************/


/****** Statics ************************************************/

static void AZFunc_Do( struct CmdObj *co );

static UBYTE * __far CmdNames[] = { "AZAccent","AZA", 0 };

/*               normal chars:       ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ */
static UBYTE __far iso2suf_tab1[] = "AAAAAA°CEEEEIIIIDNOOOOO×OUUUUYTSaaaaaa°ceeeeiiiidnooooo÷ouuuuyty";
static UBYTE __far iso2suf_tab2[] = "`'^~\"/°,`'^\"`'^\"_~`'^~\"°/`'^\"'__`'^~\"/°,`'^\"`'^\"_~`'^~\"°/`'^\"'_\"";


/****** Exported ***********************************************/

struct UIKBase  * __far UIKBase;

/* Cette TagList est elle-même le germe d'objet. Elle est
 * référencée dans cmdheader.o qui est linké avec chaque objet.
 */
struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,           (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,    (ULONG) "CHAR/S,WORD/S,LINE/S,ALL/S,NC=NOCURS/S,IS=ISO2SUF/S,SI=SUF2ISO/S",
    AZT_Gen_Flags,          AZAFF_DOMODIFY | AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionDo,           (ULONG) AZFunc_Do,
    TAG_END
    };

/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static UBYTE func_Iso2Suf( UBYTE *ptr, UBYTE *result )
{
  UBYTE ch, ch1, ch2, index, sup=0;

    ch = *ptr;
    *result = 0;
    if (ch >= 0xC0)
        {
        index = ch - 0xC0;
        ch1 = iso2suf_tab1[index];
        if (ch1 != '°')
            {
            sup = 1;
            ch2 = iso2suf_tab2[index];
            result[0] = (ch2 & 0x80) ? 1 : 2; result[1] = ch1; result[2] = ch2;
            }
        }
    if (*result == 0)
        {
        UBYTE *str = 0;
        /*if (ch == 'o')      { if (*(ptr+1) == 'e') { sup = 2; str = "o_e"; } }
        else if (ch == 'O') { if (*(ptr+1) == 'E') { sup = 2; str = "O_E"; } }
        else*/ if (ch == 'Æ') { sup = 1; str = "A_E"; }
        else if (ch == 'æ') { sup = 1; str = "a_e"; }
        if (str) { result[0] = 3; StrCpy( &result[1], str ); }
        }
//if (sup) kprintf( "sup=%ld, add=%ld\n", sup, *result );
    return( sup );
}

static UBYTE search_tab( UBYTE ch, UBYTE *tab )
{
    for (; *tab; tab+=2) if (*tab == ch) return( tab[1] );
    return(0);
}

static UBYTE func_Suf2Iso( UBYTE *ptr, UBYTE *result )
{
  UBYTE ch, ch1, res=0, sup=0;

    ch = *ptr++; ch1 = *ptr++;
    *result = 0;
    if (ch1 == '_')
        {
        switch (ch)
            {
            case 'A': if (*ptr == 'E' || *ptr == 'e') { sup = 3; result[0] = 1; result[1] = 'Æ'; } break;
            case 'a': if (*ptr == 'e')                { sup = 3; result[0] = 1; result[1] = 'æ'; } break;
            case 'O': if (*ptr == 'E' || *ptr == 'e') { sup = 3; result[0] = 2; result[1] = 'O'; result[2] = 'E'; } break;
            case 'o': if (*ptr == 'e')                { sup = 3; result[0] = 2; result[1] = 'o'; result[2] = 'e'; } break;
            }
        }
    if (sup == 0)
        {
        switch (ch)
            {
            case 'A': res = search_tab( ch1, "\"Ä'Á/Å^Â`À~Ã" ); break;
            case 'a': res = search_tab( ch1, "\"ä'á/å^â`à~ã" ); break;
            case 'C': res = search_tab( ch1, ",Ç" ); break;
            case 'c': res = search_tab( ch1, ",ç" ); break;
            case 'D': res = search_tab( ch1, "_Ð" ); break;
            case 'd': res = search_tab( ch1, "_ð" ); break;
            case 'E': res = search_tab( ch1, "\"Ë'É^Ê`È" ); break;
            case 'e': res = search_tab( ch1, "\"ë'é^ê`è" ); break;
            case 'I': res = search_tab( ch1, "\"Ï'Í^Î`Ì" ); break;
            case 'i': res = search_tab( ch1, "\"ï'í^î`ì" ); break;
            case 'N': res = search_tab( ch1, "~Ñ" ); break;
            case 'n': res = search_tab( ch1, "~ñ" ); break;
            case 'O': res = search_tab( ch1, "\"Ö'Ó/Ø^Ô`Ò~Õ" ); break;
            case 'o': res = search_tab( ch1, "\"ö'ó/Ø^ô`ò~õ" ); break;
            case 'T': res = search_tab( ch1, "_Þ" ); break;
            case 't': res = search_tab( ch1, "_þ" ); break;
            case 'U': res = search_tab( ch1, "\"Ü'Ú^Û`Ù" ); break;
            case 'u': res = search_tab( ch1, "\"ü'ú^û`ù" ); break;
            case 'Y': res = search_tab( ch1, "'Ý" ); break;
            case 'y': res = search_tab( ch1, "'ý\"ÿ" ); break;
            case 'S': res = search_tab( ch1, "_ß" ); break;
            }
        }
    if (res) { sup = 2; result[0] = 1; result[1] = res; }
    return( sup );
}

static ULONG convert_chars( ULONG type, UBYTE **pptr, ULONG start, ULONG len, LONG line )
{
  struct AZurProc *ap = AZ_GetAZur();
  struct AZObjFile *fo = ap->FObj;
  LONG i;
  UBYTE buf[20], sup, add;

    /* Cet exemple de modification en direct dans le texte en mémoire
     * ne réactualise pas la position des marques (bookmarks).
     * Cette conversion est très rapide, mais pour supprimer ou insérer
     * du texte il sera donc préférable d'utiliser les fonctions du
     * kit de développement comme AZ_TextDel() ou AZ_TextPut().
     */
    fo->WorkLine = line;
    fo->WorkCol = start + len;
    switch (type)
        {
        case ARG_ISO2SUF:
            for (i=len-1; i >= 0; i--)
                {
                if (sup = func_Iso2Suf( (*pptr)+start+i, &buf[0] ))
                    {
                    (*pptr)[start+i] = buf[1];
                    if (sup > 1)
                        {
                        sup--;
                        BufCutS( pptr, start+i+1, sup );
                        fo->WorkCol -= sup;
                        }
                    if (buf[0] > 1)
                        {
                        add = buf[0] - 1;
                        if (! BufPasteS( &buf[2], add, pptr, start+i+1 )) break;
                        fo->WorkCol += add;
                        }
                    }
                }
            break;

        case ARG_SUF2ISO:
            for (i=len-1; i >= 0; i--)
                {
                if (sup = func_Suf2Iso( (*pptr)+start+i, &buf[0] ))
                    {
                    (*pptr)[start+i] = buf[1];
                    if (sup > 1)
                        {
                        BufCutS( pptr, start+i+1, sup-1 );
                        fo->WorkCol -= (sup-1);
                        }
                    if (buf[0] > 1)
                        {
                        add = buf[0] - 1;
                        if (! BufPasteS( &buf[2], add, pptr, start+i+1 )) break;
                        fo->WorkCol += add;
                        }
                    }
                }
            break;
        }
    return(0);
}

static void AZFunc_Do( struct CmdObj *co )
{
  struct AZurProc *ap = AZ_GetAZur();
  struct AZObjFile *fo = ap->FObj;
  LONG sline, scol, eline, ecol, rect;
  ULONG type, len;
  UBYTE *ptr;
  ULONG block, numblocks;
  struct AZBlock *azb;

    UIKBase = ap->UIKBase;

         if (co->ArgRes[ARG_ISO2SUF]) type = ARG_ISO2SUF;
    else if (co->ArgRes[ARG_SUF2ISO]) type = ARG_SUF2ISO;
    else { AZ_SetCmdResult( co, 0, TEXT_ERR_NoConversion, 0 ); return; }

    fo->WorkCol = -1;
    if (numblocks = AZ_BlockExists())
        {
        for (block=numblocks; block > 0; block--) // on part du haut
            {
            azb = AZ_BlockPtr( block );
            sline = azb->LineStart;
            scol  = azb->ColStart;
            eline = azb->LineEnd;
            ecol  = azb->ColEnd;
            rect  = (azb->Mode == BLKMODE_RECT || azb->Mode == BLKMODE_FULL) ? azb->Mode : 0;

            AZ_BlockRemove( block, 0, 0 ); // avant toute modif
            AZ_TextApplyReal( sline, scol, eline, ecol, rect, 0, convert_chars, (APTR)type );
            if (sline != eline) scol = 0;
            AZ_Display( sline, eline-sline+1, scol, -1 );
            if (! co->ArgRes[ARG_NOCURS]) AZ_CursorPos( eline, fo->WorkCol );
            }
        }
    else{
        ptr = AZ_LineBuf( fo->Line );
        len = AZ_LineBufLen( ptr );

        if (co->ArgRes[ARG_WORD])
            {
            sline = eline = fo->Line; scol = ecol = fo->Col;
            if (! AZ_CharIsWord( ptr[scol] )) return;
            for (; scol>0; scol--) if (! AZ_CharIsWord( ptr[scol-1] )) break;
            for (; ecol<len; ecol++) if (! AZ_CharIsWord( ptr[ecol+1] )) break;
            }
        else if (co->ArgRes[ARG_LINE])
            {
            sline = fo->Line; scol = 0; eline = fo->Line; ecol = len-1;
            }
        else if (co->ArgRes[ARG_ALL])
            {
            sline = 0; scol = 0; eline = AZ_NumLines() - 1;
            ecol = AZ_LineLen( eline ) - 1;
            }
        else // if (co->ArgRes[ARG_CHAR]) c'est le défaut
            {
            sline = eline = fo->Line; scol = ecol = fo->Col;
            }
        AZ_TextApplyReal( sline, scol, eline, ecol, 0, 0, convert_chars, (APTR)type );
        if (sline != eline) scol = 0;
        AZ_Display( sline, eline-sline+1, scol, -1 );
        if (fo->WorkCol != -1)
            {
            ULONG len = AZ_LineLen( fo->WorkLine );
            if (fo->WorkCol > len) fo->WorkCol = len;
            if (! co->ArgRes[ARG_NOCURS]) AZ_CursorPos( eline, fo->WorkCol );
            }
        }
    AZ_DisplayStats(0);
}
