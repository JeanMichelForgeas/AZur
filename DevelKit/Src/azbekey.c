/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Enlève une colonne de caractères à partir
 *                  des position/valeur/nombre choisies.
 *                  Montre comment :
 *                  - lire les arguments donnés à la commande
 *                  - indiquer un code retour avec message
 *                  - supprimer du texte
 *
 *      Created:    13/08/93 Jean-Michel Forgeas
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

//////////////////////////////////////////////////////////////////
//                                                              //
//          A T T E N T I O N    B I D O U I L L E ! ! !        //
//                                                              //
//        Cette commande ne sert à rien car elle est plus       //
//    avantageusement remplacée par le formatage automatique    //
//      du texte, réglable dans les préférences d'édition.      //
//                                                              //
//////////////////////////////////////////////////////////////////

/****** Includes ************************************************/

#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "/includes/funcs.h"
#include "/includes/azur.h"
#include "/includes/obj.h"
#include "/includes/objfile.h"


/****** Imported ************************************************/


/****** Statics ************************************************/

static void AZFunc_New( struct CmdObj *co );
static void AZFunc_Do( struct CmdObj *co, struct CmdFunc *cf, struct KeyCom_Key *kck );

static UBYTE * __far CmdNames[] = { "AZBeKey", 0 };


/****** Exported ***********************************************/

/* Cette TagList est elle-même le germe d'objet. Elle est
 * référencée dans cmdheader.o qui est linké avec chaque objet.
 */
struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,           (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,    (ULONG) "LM=LEFTMARGIN/N,RM=RIGHTMARGIN/N",
    AZT_Gen_Flags,          AZAFF_DOMODIFY | AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionNew,          (ULONG) AZFunc_New,
    AZT_ActionDo,           (ULONG) AZFunc_Do,
    TAG_END
    };

APTR __far UIKBase;


/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static ULONG truncate_line( UBYTE *ptr, ULONG len )
{
  ULONG i;

    for (i=len; i && ptr[i-1] == ' '; i--);
    if (i < len) BufTruncate( ptr, i ); // fonction UIK
    return( i );
}

static ULONG first_char( UBYTE *ptr, ULONG len )
{
  ULONG i = 0;

    len = truncate_line( ptr, len );
    for (i=0; i < len && ptr[i] == ' '; i++);
    return( i );
}

static void AZFunc_New( struct CmdObj *co )
{
  struct AZurProc *ap = AZ_GetAZur();

    UIKBase = ap->UIKBase;
}

static void AZFunc_Do( struct CmdObj *co, struct CmdFunc *cf, struct KeyCom_Key *kck )
{
  LONG line, col, left, right, diff;
  UBYTE ch, *ptr;
  LONG splitcol, len;
  struct AZurProc *ap = AZ_GetAZur();

    left = ap->FObj->LeftMargin;
    right = ap->FObj->RightMargin;

    /*----------------- Y a-t-il des arguments? */
    if (co->ArgRes[0] || co->ArgRes[1])
        {
        /*----------------- Valeurs données en argument */
        if (co->ArgRes[0]) left  = *(ULONG*)co->ArgRes[0] - 1;
        if (co->ArgRes[1]) right = *(ULONG*)co->ArgRes[1] - 1;

        /*----------------- Vérifications */
        if (left > right)
            { AZ_SetCmdResult( co, 20, 1000, "LEFT supérieur à RIGHT" ); return; }

        /*----------------- mise à jour des vraies valeurs */
        ap->FObj->LeftMargin = left;
        ap->FObj->RightMargin = right;
        }

    /*----------------- Y a-t-il une touche? si oui on y va... */
    if (kck) // si une touche
        {
        line = ap->FObj->Line;
        col = splitcol = ap->FObj->Col;
        ptr = AZ_LineBuf( line );
        len = AZ_LineBufLen( ptr );

        if (col < left && *kck->InputBuf != '\n')
            {
            if (*kck->InputBuf == ' ') // ignore les espaces
                { AZ_SetCmdResult( co, 10, 1001, 0 ); return; }

            //--- trouve premier caractère de la ligne
            col = first_char( ptr, len );

            //--- marge gauche + redessine le resultat + curseur
            for (; col < left; col++) AZ_TextPut( " ", 1, 0, 0 ); // insère espaces sans dessiner
            AZ_TextPut( 0, 0, 0, 1 ); // redessine ce qui a besoin
            AZ_CursorPos( ap->FObj->Line, col );  // remet le curseur
            }
        else if (len + kck->InputNum - 1 > right && *kck->InputBuf != '\n')
            {
            //--- endroit où couper
            if (col >= len)
                {
                ch = *kck->InputBuf;
                col = len;
                }
            else{
                len = truncate_line( ptr, len );
                col = len-1;
                ch = AZ_CharCurrent( col, len, ptr );
                }

            while (line == ap->FObj->Line && ch != ' ')
                {
                splitcol = col;
                if ((ptr = AZ_CharPrev( ptr, &len, &line, &col, &ch )) == (APTR)-1) break;
                }
            diff = ap->FObj->Col - splitcol;

            //--- coupe en deux
            AZ_TextPrep( ap->FObj->Line, splitcol, ARPF_NOCONVERT|ARPF_NORECT|ARPF_NOOVERLAY, 0, INDENT_STAY );
            AZ_TextPut( "\n", 1, 0, 0 );
            line = ap->FObj->WorkLine;
            col = ap->FObj->WorkCol;

            //--- marge gauche + redessine le resultat + curseur
            for (; col < left; col++) AZ_TextPut( " ", 1, 0, 0 ); // insère espaces sans dessiner
            AZ_TextPut( 0, 0, 0, 1 ); // redessine ce qui a besoin

            if (diff >= 0) { line = ap->FObj->WorkLine; col = col + diff; }
            else { line = ap->FObj->Line; col = ap->FObj->Col; }
            AZ_CursorPos( line, col );  // remet le curseur

            //--- ignore l'espace qui vient
            if (col == left && *kck->InputBuf == ' ') // ignore les espaces
                { AZ_SetCmdResult( co, 10, 1002, 0 ); return; }
            }
        }
}
