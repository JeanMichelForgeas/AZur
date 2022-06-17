
UBYTE * __far ArgsTemplate2 =
    "NAME/M,MPN=MASTPORTNAME/K,PN=PORTNAME/K,F=FROM/K,CMD=FC=FROMCMD/K,"
    "PAT=PATTERN/K,NW=NOWINDOW/S,RO=READONLY/S,PRI=PRIORITY/K/N,OR=OPENREQ/S,"
    "KILL/S,ASYNC/S,LEFT/K/N,TOP/K/N,WIDTH/K/N,HEIGHT/K/N,PREFS=PREFSID/K,"
    "RES=RESIDENT/S"
    ;

#define ARG2_NAME           0    /* fichiers � ouvrir */
#define ARG2_MASTPORTNAME   1    /* nom du port de communication de la t�che master voulue */
#define ARG2_PORTNAME       2    /* nom du port ARexx des process d'�dition */
#define ARG2_FROM           3    /* script ARexx � lancer sur chaque fichier ouvert */
#define ARG2_FROMCMD        4    /* objet de commande � lancer sur chaque fichier ouvert */
#define ARG2_PATTERN        5    /* pattern du file selector */
#define ARG2_NOWINDOW       6    /* pas de fen�tre pour les fichiers ouverts  */
#define ARG2_READONLY       7    /* pas de modification ni sauvegarde possibles */
#define ARG2_PRIORITY       8    /* priorit� des process d'�dition, le master se met la m�me + 1 */
#define ARG2_OPENREQ        9    /* fileselector d�s l'ouverture d'une fen�tre sans fichier */
#define ARG2_KILL          10    /* envoie CTRL+C � la t�che maitresse QUICKNAME */
#define ARG2_ASYNC         11    /* lancement asynchrone */
#define ARG2_LEFT          12    /*  */
#define ARG2_TOP           13    /*  */
#define ARG2_WIDTH         14    /*  */
#define ARG2_HEIGHT        15    /*  */
#define ARG2_PREFS         16    /*  */
#define ARG2_RESIDENT      17    /*  */
#define ARG2_TOTAL         18

struct AZQuickMsg
    {
    struct Message  Msg;
    UBYTE           *CmdLine;
    ULONG           CurDirLock;
    struct Task     *Parent;
    struct Task     *Edit;
    ULONG           Flags;
    };

#define QUICKPREFIX "Quick_"

/*------ Flags */

#define AZQMF_ASYNC     0x0001
