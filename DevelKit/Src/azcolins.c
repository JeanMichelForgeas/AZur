/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Ins�re du texte sur une colonne � partir
 *                  des position/nombre choisies.
 *                  Montre comment :
 *                  - lire les arguments donn�s � la commande
 *                  - indiquer un code retour avec message
 *                  - ins�rer du texte
 *
 *
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

static UBYTE * __far CmdNames[] = { "AZColIns", 0 };


/****** Exported ***********************************************/

/* Cette TagList est elle-m�me le germe d'objet. Elle est
 * r�f�renc�e dans cmdheader.o qui est link� avec chaque objet.
 */
struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,           (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,    (ULONG) "TEXT/A,SL=STARTLINE/N,SC=STARTCOL/N,EL=ENDLINE/N,OV=OVERLAY/S,NC=NOCURS/S,INC/N",
    AZT_Gen_Flags,          AZAFF_DOMODIFY | AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionDo,           (ULONG) AZFunc_Do,
    TAG_END
    };

/* Exemple d'utilisation pour ins�rer une colonne de tabulations
 * verticalement � partir des positions du curseur : cliquez une fois
 * sur la position de d�but, puis une fois sur la ligne de fin, puis :
 *              AZColIns \t NOCURS
 * Ou bien tracez un bloc rectangulaire et faites :
 *              AZColIns \t
 * Pour d�caler tout un texte :
 *              AZColIns \t 1 1 -1 NOCURS
 * Pour mettre quelques lignes en commentaires C, tracer un bloc puis :
 *              AZColIns "// "
 */

/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static void AZFunc_Do( struct CmdObj *co )
{
  LONG startline, startcol, endline, inc, initcol;
  UBYTE *text; /* pas besoin de d�faut pour text car il est obligatoire : /A dans ArgTemplate */
  LONG i, len, flags;
  struct AZBlock *azb;
  struct AZurProc *ap = AZ_GetAZur();

    /*----------------- Valeurs par d�faut */
    startline = ap->FObj->OldLine;
    startcol = ap->FObj->OldCol;
    endline = ap->FObj->Line;
    inc = 1;
    flags = ARPF_RECT | ARPF_CONVERT | ARPF_NOOVERLAY;

    if (azb = AZ_BlockPtr( AZ_BlockCurrent() ))
        {
        startline = azb->LineStart;
        startcol = azb->ColStart;
        endline = azb->LineEnd;
        }

    /*----------------- Valeurs donn�es en argument */
    /* les arguments ont le m�me format que ceux donn�s par la fonction
     * ReadArgs() de dos.library : ce sont des pointeurs sur des nombres
     * ou sur des chaines.
     * On retire 1 des num�ros de ligne/colone car pour l'utilisateur
     * le texte va de 1 � n, mais pour la liste de lignes de AZur �a va
     * de 0 � n-1.
     */
    text = (UBYTE*)co->ArgRes[0];
    if (co->ArgRes[1]) startline = *(ULONG*)co->ArgRes[1] - 1;
    if (co->ArgRes[2])
        {
        startcol = *(ULONG*)co->ArgRes[2];
        if (startcol != -1) startcol--;
        initcol = startcol;
        }
    if (co->ArgRes[3])
        {
        endline = *(ULONG*)co->ArgRes[3];
        if (endline != -1) endline--;
        }
    if (co->ArgRes[4]) flags    |= ARPF_OVERLAY;
    if (co->ArgRes[6]) inc       = *(ULONG*)co->ArgRes[6];
    len = strlen( text );  /* on a pas besoin du 0 de fin de chaine alors on peut l'�craser */
    text[len] = '\n';      /* par un \n qui va permettre d'�crire en rectangle */
    len++;
    if (endline == -1) endline = AZ_NumLines() - 1; // si ENDLINE=-1

    /*----------------- V�rifications */
    if (startline > endline)
        { AZ_SetCmdResult( co, 20, 1000, "STARTLINE sup�rieure � ENDLINE" ); return; }

    /*----------------- On y va... */
    if (inc == 1)
        {
        AZ_TextPrep( startline, startcol, flags, 0, 0 );  // met � jour ap->FObj->WorkLine
        for (i=startline; i <= endline && ap->FObj->WorkLine <= endline; i++)
            {
            if (initcol == -1) startcol = AZ_LineLen( i );
            AZ_TextPrep( i, startcol, flags, 0, 0 );
            AZ_TextPut( text, len, 0, 1 ); // affiche au fur et � mesure � cause AZ_TextPrep()
            }
        // AZ_TextPut( 0, 0, 0, 1 ); // re-affiche tout � la fin
        }
    else{
        AZ_TextPrep( startline, startcol, flags, 0, 0 );  // met � jour ap->FObj->WorkLine
        for (i=startline; i <= endline && ap->FObj->WorkLine <= endline; i+=inc)
            {
            if (initcol == -1) startcol = AZ_LineLen( i );
            AZ_TextPrep( i, startcol, flags, 0, 0 );
            AZ_TextPut( text, len, 0, 1 );   // affiche au fur et � mesure � cause AZ_TextPrep()
            }
        }

    AZ_DisplayStats(0);

    /* AZ_TextDel() et AZ_TextPut() mettent dans les variables
     * ap->FObj->WorkLine et ap->FObj->WorkCol les ligne et colonne
     * d'arriv�e apr�s modification. On peut s'en servir pour mettre
     * � jour la position du curseur.
     */
    if (! co->ArgRes[5]) // si pas NOCURS
        AZ_CursorPos( ap->FObj->WorkLine, ap->FObj->WorkCol );  /* remet le curseur */
}
