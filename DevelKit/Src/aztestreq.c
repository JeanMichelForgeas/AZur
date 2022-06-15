/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Teste la requ�te multi-usages de AZur
 *
 *      Created:    24/09/93 Jean-Michel Forgeas
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
 *      � A4, et de d�clarer toutes variables (donn�es) globales
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


/****** Imported ************************************************/

ULONG __far SysBase;
ULONG __far DOSBase;


/****** Statics ************************************************/

static void AZFunc_Do( struct CmdObj *co );

static UBYTE * __far CmdNames[] = { "AZTestReq", 0 };


/****** Exported ***********************************************/

/* Cette TagList est elle-m�me le germe d'objet AZTestReq. Elle est
 * r�f�renc�e dans cmdheader.o qui est link� avec chaque objet.
 */
struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,   (ULONG) CmdNames,
    AZT_Gen_Flags,  AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionDo,   AZFunc_Do,
    TAG_END
    };


/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static void AZFunc_Do( struct CmdObj *co )
{
  UBYTE *buf1=0, *buf2=0, *res=0;
  ULONG result;
  struct AZurProc *ap = AZ_GetAZur();

    if (!(buf1 = AZ_BufAlloc( 20 )) || !(buf2 = AZ_BufAlloc( 0 )))
        { AZ_SetCmdResult( co, 20, 1000, "Manque de m�moire" ); goto END_ERROR; }

    strcpy( buf1, "Texte initial" );

    /*------ La requ�te avec toutes les options */

    result = AZ_SmartRequest(
            /*------ Le titre */
                "Salut, je m'appelle Max",
            /*------ Les lignes explicatives d�limit�es par des\n dans une seule cha�ne */
                "Cette requ�te comporte 2 zones de saisie\n" /* <-- pas de virgule donc concat�n�e */
                "et 3 boutons.\n"
                "Le r�sultat sera affich� dans la requ�te suivante",
            /*------ Titre et r�ceptacle pour la 1�re zone se saisie */
                "Zone _1", &buf1,
            /*------ Titre et r�ceptacle pour la 2�me zone se saisie */
                "Zone _2", &buf2,
            /*------ Titres pour les 3 boutons */
                "Choix _A",
                "Choix _B",
                "A_nnule",
            /*------ Fonction appel�e avant que le requester s'affiche */
                NULL, /* pas utilis� dans cet exemple */
            /*------ Flags divers */
                ARF_RETQUIET | (TST_REQWIN ? ARF_WINDOW : 0)  /* voir azur.h */
            );
    if (result == REQBUT_CTRL_C || result == REQBUT_CANCEL) goto END_ERROR;

    /*------ La requ�te d'affichage du r�sultat */

    /* on pr�pare le texte dans un UIKBuffer */
    AZ_BufPrintf( &res, 0,
            "R�sultats affich�s par AZ_SmartRequest() :\n"
            "Texte Zone 1 : %ls\n"   /* <-- pas de virgule donc concat�n�e */
            "Texte Zone 2 : %ls\n"
            "Bouton       : %lc\n"
            "On aurait pu utiliser plus simplement la\nfonction AZ_ReqShowText()...",
            buf1, buf2, (result == REQBUT_OK ? 'A' : 'B') );

    /* puis on l'affiche dans une requ�te */
    AZ_SmartRequest( "R�sultat de Max", res, 0,0,0,0, 0,0,0, 0, (TST_REQWIN ? ARF_WINDOW : 0) );

  END_ERROR:
    if (buf1) AZ_BufFree( buf1 );
    if (buf2) AZ_BufFree( buf2 );
    if (res)  AZ_BufFree( res );
}
