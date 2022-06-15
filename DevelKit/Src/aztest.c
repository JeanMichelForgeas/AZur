/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Teste les diff�rentes actions d'un objet de
 *                  commande en �crivant des messages avec la
 *                  fonction kprintf (linker avec debug.lib du
 *                  Software ToolKit Commodore).
 *                  Plusieurs fonctions peuvent �tre branch�es
 *                  sur n'importe quelle Action.
 *                  Ces messages peuvent �tre re�us soit :
 *                  - sur un terminal branch� sur le port s�rie,
 *                  - dans un �mulateur avec un modem en boucle,
 *                  - dans une console Sushi (ToolKit Commodore).
 *
 *      Created:    12/07/93 Jean-Michel Forgeas
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


/****** Statics ************************************************/

static void AZFunc_Load( struct CmdObj *co );
static void AZFunc_Unload( struct CmdObj *co );
static void AZFunc_New( struct CmdObj *co );
static void AZFunc_Dispose( struct CmdObj *co );
static void AZFunc_InitMenu( struct CmdObj *co );
static void AZFunc_SetMenu( struct CmdObj *co );
static void AZFunc_Do1( struct CmdObj *co );
static void AZFunc_Do2( struct CmdObj *co );
static void AZFunc_Undo( struct CmdObj *co );

static UBYTE * __far CmdNames[] = { "AZTest", "AZTestSynonyme", "Ouiiieeuuu", 0 };


/****** Exported ***********************************************/

/* Cette TagList est elle-m�me le germe d'objet AZTest. Elle est
 * r�f�renc�e dans cmdheader.o qui est link� avec chaque objet.
 */
struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,   (ULONG) CmdNames,
    AZT_Gen_Flags,  AZAFF_DOMODIFY | AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,

    AZT_ActionLoad,     AZFunc_Load,
    AZT_ActionUnload,   AZFunc_Unload,
    AZT_ActionNew,      AZFunc_New,
    AZT_ActionDispose,  AZFunc_Dispose,
    AZT_ActionInitMenu, AZFunc_InitMenu,
    AZT_ActionSetMenu,  AZFunc_SetMenu,
    AZT_ActionDo,       AZFunc_Do1,
    AZT_ActionDo,       AZFunc_Do2,  /* une 2�me fonction sur l'action 'ActionDo' */
    AZT_ActionUndo,     AZFunc_Undo,

    TAG_END
    };

ULONG __far SysBase;
ULONG __far DOSBase;


/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static void AZFunc_Load( struct CmdObj *co )
{
    /* exemple d'ouverture de librairies */
    SysBase = *((ULONG*)4);
    if (! (DOSBase = OpenLibrary( "dos.library",  0 )))
        { AZ_SetCmdResult( co, 20, 1000, "Ouverture dos.library impossible" ); return; }

    kprintf( "Commande AZTest charg�e\n" );
}

static void AZFunc_Unload( struct CmdObj *co )
{
    if (DOSBase) CloseLibrary( DOSBase );

    kprintf( "Commande AZTest d�charg�e\n" );
}

static void AZFunc_New( struct CmdObj *co )
{ kprintf( "Nouvel objet AZTest cr��\n" ); }

static void AZFunc_Dispose( struct CmdObj *co )
{ kprintf( "Objet AZTest supprim�\n" ); }

static void AZFunc_InitMenu( struct CmdObj *co )
{ kprintf( "Init du menu auquel cet objet AZTest est attach�\n" ); }

static void AZFunc_SetMenu( struct CmdObj *co )
{ kprintf( "Action sur le menu auquel cet objet AZTest est attach�\n" ); }

static void AZFunc_Do1( struct CmdObj *co )
{ kprintf( "Premi�re fonction d'ex�cution de AZTest\n" ); }

static void AZFunc_Do2( struct CmdObj *co )
{ kprintf( "Deuxi�me fonction d'ex�cution de AZTest\n" ); }

static void AZFunc_Undo( struct CmdObj *co )
{ kprintf( "M�morisation pour un �ventuel futur Undo de AZTest\n" ); }
