/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Rappel :
 *                  Un objet de commande AZur doit �tre "Pure", c'est
 *                  � dire r�entrant et re-ex�cutable.
 *                  Donc son code et ses donn�es peuvent �tre partag�s
 *                  par toutes les t�ches (fen�tres) AZur.
 *
 *                  Un objet ou un ensemble d'objets peuvent avoir
 *                  besoin de partager des variables globales, qui
 *                  soient partag�es par tous les objets sur toutes
 *                  les t�ches, ou bien diff�rentes pour chaque t�che
 *                  AZur.
 *                  Cet exemple montre donc comment allouer, utiliser
 *                  puis lib�rer ces variables.
 *
 *                  Se reporter � la documentation sur les variables
 *                  partag�es.
 *
 *      Created:    05/08/93 Jean-Michel Forgeas
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
#include "/includes/objfile.h"

struct MyVars
    {
    ULONG Counter;
    };

/****** Imported ************************************************/


/****** Statics ************************************************/

static void AZFunc_Load( struct CmdObj *co );
static void AZFunc_Unload( struct CmdObj *co );
static void AZFunc_Do( struct CmdObj *co );

static UBYTE * __far CmdNames[] = { "AZTestVar", "Banane", 0 };


/****** Exported ***********************************************/

/* Cette TagList est elle-m�me le germe d'objet. Elle est
 * r�f�renc�e dans cmdheader.o qui est link� avec chaque objet.
 */
struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,   (ULONG) CmdNames,
    AZT_Gen_Flags,  AZAFF_DOMODIFY | AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionLoad,     AZFunc_Load,
    AZT_ActionUnload,   AZFunc_Unload,
    AZT_ActionDo,       AZFunc_Do,
    TAG_END
    };


/****************************************************************
 *
 *  Fonctions appel�es seulement une fois dans la vie du germe
 *
 ****************************************************************/

static void AZFunc_Load( struct CmdObj *co )
{
  struct MyVars *v;

    if ((v = AZ_VarNew( "Counter", sizeof(struct MyVars) )) == 0)
        {
        AZ_SetCmdResult( co, 20, 1000, "manque de m�moire" );
        return;
        }

    if (v != (APTR)-1)  /* si la variable 'Counter' n'existe pas d�j� */
        {               /* alors on l'initialise */
        v->Counter = 10;
        AZ_VarUnlock( "Counter" );
        }
}

static void AZFunc_Unload( struct CmdObj *co )
{
    AZ_VarDispose( "Counter" );
}

/****************************************************************
 *
 *  Fonction appel�e � chaque fois qu'un objet cr�� par le germe
 *  AZTestVar est ex�cut�
 *
 ****************************************************************/

static void AZFunc_Do( struct CmdObj *co )
{
  struct MyVars *v;
  UBYTE buf[100];

    if (! (v = AZ_VarLock( "Counter" )))
        { AZ_SetCmdResult( co, 20, 1001, "Quoi? On s'est lav� les pieds dans ma soupe?" ); return; }
    v->Counter++;
    sprintf( buf, "    Count=%ld       ", v->Counter );
    AZ_VarUnlock( "Counter" );

    AZ_TextPrep( 0,0, ARPF_OVERLAY, 0, 0 );
    AZ_TextPut( buf, strlen(buf), 0, 1 );
}
