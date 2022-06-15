/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Enl�ve une colonne de caract�res � partir
 *                  des position/valeur/nombre choisies.
 *                  Montre comment :
 *                  - lire les arguments donn�s � la commande
 *                  - indiquer un code retour avec message
 *                  - supprimer du texte
 *
 *      Created:    13/08/93 Jean-Michel Forgeas
 *
 *      NOTA BENE:
 *      Les fonctions de votre objet de commande sont appel�es
 *      avec l'adresse de la structure de donn�es g�n�rales
 *      (struct AZurProc) dans le registre A4 (on peut aussi
 *      l'obtenir avec la fonction AZ_GetAZur()).
 *
 *      Les fonctions de AZur attendent aussi dans A4 l'adresse
 *      de cette structure.
 *      Il est dond n�cessaire de compiler en adressage relatif
 *      � A4, et de d�clarer toutes vos variables (donn�es) globales
 *      en "far" afin de ne pas modifier ce registre, si toutefois
 *      vous d�sirez appeler des fonctions AZur depuis votre
 *      objet de commande.
 *      Attention que les constantes non d�clar�es ne soient pas
 *      non plus adress�es par A4.
 *
 *      Pour utiliser des fonctions de dos.library comme printf()
 *      il faut ouvrir vous-m�me cette librairie, car comme on ne
 *      link pas avec c.o ou autre startup normal, elle n'est pas
 *      ouverte automatiquement. Il n'y a pas de "stdout" d�fini
 *      non plus.
 *      On ne peut pas utiliser non plus fprintf( Output(), ...)
 *      car AZur a pu �tre lanc� du Workbench.
 *      Vous pouvez par exemple ouvrir la dos.library puis ouvrir
 *      "CON:" pour �crire dedans.
 *      Mais il est beaucoup plus simple d'utiliser kprintf() en
 *      conjonction avec Sushi (Software Toolkit de Commodore).
 *
 *      SAS C 5.x
 *      ---------
 *      - Toutes variables globales et statiques en __far
 *      - Options de compilation : -b1
 *      - Ne jamais utiliser geta4
 *      - ne jamais d�clarer des fonctions avec __saveds
 *
 *      DICE C
 *      ------
 *      - Toutes variables globales et statiques en __far
 *      - Options de compilation : -md -ms
 *      - Ne jamais utiliser geta4
 *      - ne jamais d�clarer des fonctions avec __geta4
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include "/includes/funcs.h"
#include "/includes/azur.h"
#include "/includes/obj.h"
#include "/includes/objfile.h"


/****** Imported ************************************************/


/****** Statics ************************************************/

static void AZFunc_Do( struct CmdObj *co );

static UBYTE * __far CmdNames[] = { "AZColDel", 0 };


/****** Exported ***********************************************/

/* Cette TagList est elle-m�me le germe d'objet. Elle est
 * r�f�renc�e dans cmdheader.o qui est link� avec chaque objet.
 */
struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,           (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,    (ULONG) "SL=STARTLINE/N,SC=STARTCOL/N,EL=ENDLINE/N,EC=ENDCOL/N,R=RECT/S,NC=NOCURS/S",
    AZT_Gen_Flags,          AZAFF_DOMODIFY | AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionDo,           (ULONG) AZFunc_Do,
    TAG_END
    };


/* Exemple d'utilisation pour effacer une colonne sur tout le
 * texte :
 *              AZColDel 1 1 -1 4 NOCURS
 */

/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static void AZFunc_Do( struct CmdObj *co )
{
   struct AZurProc *ap = AZ_GetAZur();
   LONG startline, startcol, endline, endcol, rect;
   struct AZDisplayCmd DC;

    /*----------------- Valeurs par d�faut */
    startline = ap->FObj->OldLine;
    startcol = ap->FObj->OldCol;
    endline = ap->FObj->Line;
    endcol = ap->FObj->Col;
    rect = 1;

    /*----------------- Valeurs donn�es en argument */
    /* les arguments ont le m�me format que ceux donn�s par la fonction
     * ReadArgs() de dos.library : ce sont des pointeurs sur des nombres
     * ou sur des chaines.
     * On retire 1 des num�ros de ligne/colone car pour l'utilisateur
     * le texte va de 1 � n, mais pour la liste de lignes de AZur �a va
     * de 0 � n-1.
     */
    if (co->ArgRes[0]) startline = *(ULONG*)co->ArgRes[0] - 1;
    if (co->ArgRes[1]) startcol  = *(ULONG*)co->ArgRes[1] - 1;
    if (co->ArgRes[2]) endline   = *(ULONG*)co->ArgRes[2] - 1;
    if (co->ArgRes[3]) endcol    = *(ULONG*)co->ArgRes[3] - 1;
    if (co->ArgRes[4]) rect      = 0;

    if (endline == -2) endline = AZ_NumLines() - 1; // si ENDLINE=-1

    /*----------------- On y va... */
    AZ_TextDelRect( startline, startcol, endline, endcol, rect, &DC );

    /*----------------- Affichage des modifications */
    AZ_DisplayCmd( &DC );
    AZ_DisplayStats(0);

    /* AZ_TextDel() et AZ_TextPut() mettent dans les variables
     * ap->FObj->WorkLine et ap->FObj->WorkCol les ligne et colonne
     * d'arriv�e apr�s modification. On peut s'en servir pour mettre
     * � jour la position du curseur.
     */
    if (! co->ArgRes[5]) // si pas NOCURS
        AZ_CursorPos( ap->FObj->WorkLine, ap->FObj->WorkCol );  /* remet le curseur */
}
