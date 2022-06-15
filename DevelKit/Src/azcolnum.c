/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Construit une colonne de nombres � partir
 *                  des position/valeur/nombre choisies.
 *                  Montre comment :
 *                  - lire les arguments donn�s � la commande
 *                  - indiquer un code retour avec message
 *                  - supprimer et ajouter du texte
 *
 *
 *
 *      Created:    02/08/93 Jean-Michel Forgeas
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

static UBYTE * __far CmdNames[] = { "AZColNum", 0 };


/****** Exported ***********************************************/

/* Cette TagList est elle-m�me le germe d'objet. Elle est
 * r�f�renc�e dans cmdheader.o qui est link� avec chaque objet.
 */
struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,           (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,    (ULONG) "VALUE/N,SL=STARTLINE/N,SC=STARTCOL/N,EL=ENDLINE/N,OV=OVERLAY/S,NC=NOCURS/S,BL=BLOCK/S",
    AZT_Gen_Flags,          AZAFF_DOMODIFY | AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionDo,           (ULONG) AZFunc_Do,
    TAG_END
    };

/* Exemple d'utilisation pour num�roter un texte � partir des
 * positions du curseur, cliquez une premi�re fois sur la position
 * de d�part, puis une seconde sur la ligne de fin :
 *              AZColNum 0 NOCURS
 * Puis pour renum�roter � partir de 10, sans re-cliquer :
 *              AZColNum 10 NOCURS OVERLAY
 * Puis pour renum�roter tout le texte :
 *              AZColNum 1 1 1 -1 NOCURS
 */

/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static void AZFunc_Do( struct CmdObj *co )
{
  LONG i, len, flags, startline, startcol, endline, value;
  UBYTE fmt[50], buf[100];
  struct AZBlock *azb;
  struct AZurProc *ap = AZ_GetAZur();

    /*----------------- Valeurs par d�faut */
    startline = ap->FObj->OldLine;
    startcol = ap->FObj->OldCol;
    endline = ap->FObj->Line;
    value = startline + 1;
    flags = ARPF_RECT | ARPF_NOCONVERT | ARPF_NOOVERLAY;

    if (azb = AZ_BlockPtr( AZ_BlockCurrent() ))
        {
        startline = azb->LineStart;
        startcol = azb->ColStart;
        endline = azb->LineEnd;
        value = 0;
        AZ_BlockEraseAll();
        }

    /*----------------- Valeurs donn�es en argument */
    /* les arguments ont le m�me format que ceux donn�s par la fonction
     * ReadArgs() de dos.library : ce sont des pointeurs sur des nombres
     * ou sur des chaines.
     * On retire 1 des num�ros de ligne/colone car pour l'utilisateur
     * le texte va de 1 � n, mais pour la liste de lignes de AZur �a va
     * de 0 � n-1.
     */
    if (co->ArgRes[0]) value     = *(ULONG*)co->ArgRes[0];
    if (co->ArgRes[1]) startline = *(ULONG*)co->ArgRes[1] - 1;
    if (co->ArgRes[2]) startcol  = *(ULONG*)co->ArgRes[2] - 1;
    if (co->ArgRes[3]) endline   = *(ULONG*)co->ArgRes[3] - 1;
    if (co->ArgRes[4]) flags    |= ARPF_OVERLAY;

    if (endline == -2) endline = AZ_NumLines() - 1; // si ENDLINE=-1

    /*----------------- V�rifications */
    if (startline > endline)
        { AZ_SetCmdResult( co, 20, 1000, "STARTLINE sup�rieure � ENDLINE" ); return; }

    /*----------------- On y va... */
    sprintf( buf, "%ld", value + endline - startline );
    len = strlen( buf );  // largeur du plus grand nombre
    sprintf( fmt, "%%%ld.%ldld%%lc", len, len );

    AZ_TextPrep( startline, startcol, flags, 0, 0 );
    for (i=startline; i <= endline; i++)
        {
        sprintf( buf, fmt, value, (i == endline ? 0: '\n') );
        AZ_TextPut( buf, strlen( buf ), 0, 0 );
        value++;
        }

    /*----------------- On met un bloc par-dessus ? */
    if (co->ArgRes[6]) // si BLOCK
        if (AZ_BlockStart( startline, startcol, 0 ))
            AZ_BlockDefine( endline, startcol+len+1, BLKMODE_RECT, 0 );

    /*----------------- Affichage des modifications */
    AZ_TextPut( 0, 0, 0, 1 ); // re-affiche tout � la fin
    AZ_DisplayStats(0);

    /* AZ_TextDel() et AZ_TextPut() mettent dans les variables
     * ap->FObj->WorkLine et ap->FObj->WorkCol les ligne et colonne
     * d'arriv�e apr�s modification. On peut s'en servir pour mettre
     * � jour la position du curseur.
     */
    if (! co->ArgRes[5]) // si pas NOCURS
        AZ_CursorPos( ap->FObj->WorkLine, ap->FObj->WorkCol );  /* remet le curseur */
}
