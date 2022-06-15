
#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef W2L
#define W2L(a,b)        ((ULONG)((((UWORD)(a))<<16)|((UWORD)(b))))
#define B2L(a,b,c,d)    ((ULONG)((((UBYTE)(a))<<24)|(((UBYTE)(b))<<16)|(((UBYTE)(c))<<8)|((UBYTE)(d))))
#define B2W(a,b)        ((UWORD)((((UBYTE)(a))<<8)|((UBYTE)(b))))
#endif

struct AZRequest
    {
    struct UIKObj   *Obj;
    ULONG           Flags;
    ULONG           ErrMsg;
    UBYTE           Button;
    UBYTE           Reservedb;
    UWORD           Reservedw;
    ULONG           Reservedl;
    UWORD           Left, Top, Width, Height;
    struct UIKObjWindow *WO;
    };

struct AZDisplayCmd
    {
    LONG    DisplayNum;
    LONG    DisplayFrom;
    LONG    ScrollNum;
    LONG    ScrollFrom;
    LONG    Line;
    LONG    Col;
    };

/* reçu en paramètre par les fonctions Action_Do des objets
 * mis sur BeforeKey et AfterKey
 */
struct KeyCom_Key
    {
    UWORD   Code;
    ULONG   Qualifier;
    ULONG   InputNum;
    UBYTE   *InputBuf;
    };

/* reçu en paramètre par les fonctions Action_Do des objets
 * mis sur BeforeLine et AfterLine
 */
struct KeyCom_Line
    {
    LONG    NewLine, NewCol;
    LONG    OldLine, OldCol;
    };

#define SQUAL_QUAL1     0x00010000
#define SQUAL_QUAL2     0x00020000
#define SQUAL_QUAL3     0x00040000
#define SQUAL_LOCK1     0x00080000
#define SQUAL_LOCK2     0x00100000
#define SQUAL_LOCK3     0x00200000
#define SQUAL_ANYSPECIAL 0x00400000

#define SQUAL_NOSPECIAL 0x00800000
#define SQUAL_NOREPEAT  0x01000000

#define SQUAL_MASK  (IEQUALIFIER_LSHIFT|IEQUALIFIER_LALT|IEQUALIFIER_CONTROL|IEQUALIFIER_LCOMMAND|SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3|SQUAL_LOCK1|SQUAL_LOCK2|SQUAL_LOCK3|SQUAL_ANYSPECIAL)

#define SQUAL_DBLCLIC   0x02000000
#define SQUAL_TPLCLIC   0x04000000
#define SQUAL_JOYRIGHT  0x08000000
#define SQUAL_JOYLEFT   0x10000000
#define SQUAL_JOYDOWN   0x20000000
#define SQUAL_JOYUP     0x40000000
#define SQUAL_JOYFIRE   0x80000000

#define SCODE_MOUSEMOVE 0xff    /* IECODE_NOBUTTON qui n'est jamais reçu ici */
#define SCODE_JOYRIGHT  0x0100  /* Correspondance avec les évènements UIK !!! */
#define SCODE_JOYLEFT   0x0200
#define SCODE_JOYDOWN   0x0400  /* BACKW */
#define SCODE_JOYUP     0x0800  /* FORW */
#define SCODE_JOYFIRE   0x1000
#define SCODE_MOUSETICK 0x2000
#define SCODE_JOYTICK   0x4000

#define SQUAL_MJOY  (IEQUALIFIER_MIDBUTTON|IEQUALIFIER_RBUTTON|IEQUALIFIER_LEFTBUTTON|SQUAL_DBLCLIC|SQUAL_TPLCLIC|SQUAL_JOYRIGHT|SQUAL_JOYLEFT|SQUAL_JOYDOWN|SQUAL_JOYUP|SQUAL_JOYFIRE)

struct AZMenuEntry
    {
    UBYTE       Depth;          /* 0=Menu, 1=Item, 2=SubItem */
    UBYTE       ComKey;         /* touche d'activation de cet item/SubItem */
    UBYTE       *Title;         /* numéro du titre dans tableaux de chaines */
    ULONG       Reserved;
    struct CmdObj *CmdObj;      /* objet associé à cet item/SubItem */
    ULONG       MutualExclude;  /* voir doc Intuition pour ce champ */
    struct MenuItem *IntuiData; /* structure Intuition associée (si menu en activité) */
    UWORD       Flags;          /* voir "DevelKit/Man/Objets" */
    };

struct AZViewInfo
    {
    WORD        Left;
    WORD        Top;
    WORD        Width;
    WORD        Height;
    UWORD       MinWidth;
    UWORD       MinHeight;
    UWORD       VBorderWidth;
    UWORD       HBorderHeight;
    UWORD       VButHeight;
    UWORD       VArrHeight;
    UWORD       HButWidth;
    UWORD       HArrWidth;
    };

#define BUMF_BUTUP      0x0001
#define BUMF_UPDOWN1    0x0002
#define BUMF_VERTPROP   0x0004
#define BUMF_UPDOWN2    0x0008
#define BUMF_BUTDOWN    0x0010
#define BUMF_BUTLEFT    0x0020
#define BUMF_LEFTRIGHT1 0x0040
#define BUMF_HORIZPROP  0x0080
#define BUMF_LEFTRIGHT2 0x0100
#define BUMF_BUTRIGHT   0x0200
#define BUMF_STATUSBAR  0x0400
#define BUMF_CMDLINE    0x0800

#define BUMF_RIGHTBORDER    (BUMF_BUTUP|BUMF_UPDOWN1|BUMF_VERTPROP|BUMF_UPDOWN2|BUMF_BUTDOWN)
#define BUMF_BOTTOMBORDER   (BUMF_BUTLEFT|BUMF_LEFTRIGHT1|BUMF_HORIZPROP|BUMF_LEFTRIGHT2|BUMF_BUTRIGHT)

#ifndef INTUITION_INTUITION_H
struct IBox
{
    WORD Left;
    WORD Top;
    WORD Width;
    WORD Height;
};
#endif

struct AZurPrefs
    {
    ULONG       Flags1;
    ULONG       Flags2;
    ULONG       Flags3;
    ULONG       SearchFlags;
    ULONG       EleMask;
    UBYTE       FontName[32];
    UBYTE       ViewFontName[32];
    UBYTE       FontHeight;
    UBYTE       ViewFontHeight;
    UBYTE       BlinkOnDelay;
    UBYTE       BlinkOffDelay;

    UBYTE       TextPen, BackPen, BlockTPen, BlockBPen;
    UBYTE       CursTextPen, CursBackPen, CursBlockTPen, CursBlockBPen;

    struct AZViewInfo VI;

    UBYTE       PatOpen[64];
    UBYTE       ParmFile[64];

    ULONG       IOBufLength;
    UBYTE       IndentType;
    UBYTE       TabLen;
    UBYTE       PuddleSize;
    UBYTE       DelClip;

    UBYTE       MastPortName[16];
    UBYTE       PortName[16];
    UBYTE       ARexxConsoleDesc[96];
    UBYTE       DOSConsoleDesc[96];

    UWORD       ScreenLeft, ScreenTop, ScreenWidth, ScreenHeight;
    ULONG       ScreenModeId;
    UBYTE       ScreenDepth;

    UBYTE       BlockMode;
    UBYTE       BlockXLim;
    UBYTE       BlockYLim;

    UWORD       HistCmdLineMax;
    UBYTE       ScreenOverscan;
    UBYTE       CharSpace;
    UBYTE       CharEOL;
    UBYTE       CharEOF;
    UBYTE       CharTabS;
    UBYTE       CharTabE;

    LONG        LeftMargin;
    LONG        RightMargin;
    UBYTE       IconTool[64];
    UBYTE       BackupDirName[64];
    UBYTE       BackupName[16];
    ULONG       UndoMaxNum;
    ULONG       UndoMaxSize;

    UBYTE       CharRexxDot;
    BYTE        Priority;
    UWORD       AutsIntSec;

    struct IBox PosBookmarks;
    struct IBox PosLineCol;
    struct IBox PosOffset;
    struct IBox PosSearch;
    struct IBox PosReplace;

    UBYTE       AutsDirName[64];
    UBYTE       AutsName[16];
    UWORD       AutsIntMod;
    UBYTE       BackupRot;
    UBYTE       padb;
    };

#define MAXSIGNED   0x7fffffff
#define PLUSLINES   10
#define PLUSCOLS    3

/*---- BlockMode : Block types ----*/
#define BLKMODE_NONE    0
#define BLKMODE_CHAR    1
#define BLKMODE_WORD    2
#define BLKMODE_LINE    3
#define BLKMODE_RECT    4
#define BLKMODE_FULL    5

/*---- IndentType : Indent types----*/
#define NOINDENT        0
#define INDENT_LEFT     1
#define INDENT_STAY     2
#define INDENT_RIGHT    3
#define INDENT_TTX      4

/*----------------------------------*/
struct AZurProc
    {
    /*--- commun au master et aux process */
    struct UIKBase      *UIKBase;
    struct DOSBase      *DOSBase;
    struct GfxBase      *GfxBase;
    struct ExecBase     *ExecBase;
    struct LayersBase   *LayersBase;
    struct IntuitionBase *IntuitionBase;
    struct UIKGlobal    *UIK;
    struct AZur         *AZMast;

    struct UIKObj       *WinHelp;
    struct UIKObj       *ARexx;
    struct UIKObjTimer  *Timer;

    struct AZurPrefs    *Prefs;
    struct AZurPrefs    *TmpPrefs;
    UBYTE               *DefPrefsName;
    struct UIKObjWindow *WO;
    struct WorkbenchBase *WorkbenchBase;

    struct _jump        *JumpOffsets;
    struct FileInfoBlock FIB;
    struct Process      *Process;
    ULONG               CurDirLock;
    UBYTE               Reserved2;
    UBYTE               Reserved3;
    WORD                RexxError;
    UBYTE               *RexxResult;
    APTR                Pool;
    UBYTE               ExeFrom;
    UBYTE               Reserved4;
    UWORD               Reserved5;
    UBYTE               *LastCmd;

    /* les champs ci-dessous sont à 0 pour la tâche mère.
     * voir l'autodoc de AZ_GetAZur() dans azur.doc.
     */
    struct AZObjFile    *FObj;
    ULONG               UserData;
    ULONG               Reserved[9+11+19*8];
    UBYTE               *UBufTmp;
    };
typedef struct AZurProc AZURPROC;


/*---- Evènements résultant des requesters synchrones : AZ_SmartRequest(), etc... */
#define REQBUT_NOEVENT  -1
#define REQBUT_CANCEL   0
#define REQBUT_OK       1
#define REQBUT_MIDDLE   2
#define REQBUT_CTRL_C   3

/*---- AZ_SmartRequest() Flags */
#define ARF_WINDOW     0x0001   /* ouvre une fenêtre au lieu d'un requester */
#define ARF_ACTIVATE   0x0002   /* active la première zone de saisie */
#define ARF_RETQUIET   0x0004   /* ne termine pas si RETURN dans zone de saisie */
#define ARF_STRGAD     0x0008
#define ARF_ASYNC      0x0010
#define ARF_MEMPOS     0x0080
#define ARF_NOSORT     0x0100

/*---- AZRequest Tags */
#define ART_ReqTitle        (TAG_USER|0x0020)
#define ART_ReqFlags        (TAG_USER|0x0021)
#define ART_OKTitle         (TAG_USER|0x0022)
#define ART_OKFunc          (TAG_USER|0x0023)
#define ART_CancelTitle     (TAG_USER|0x0024)
#define ART_CancelFunc      (TAG_USER|0x0025)
#define ART_MiddleTitle     (TAG_USER|0x0026)
#define ART_MiddleFunc      (TAG_USER|0x0027)
#define ART_Str1Title       (TAG_USER|0x0028)
#define ART_Str1Func        (TAG_USER|0x0029)
#define ART_Str1Max         (TAG_USER|0x002a)
#define ART_Str1Init        (TAG_USER|0x002b)
#define ART_Str2Title       (TAG_USER|0x002c)
#define ART_Str2Func        (TAG_USER|0x002d)
#define ART_Str2Max         (TAG_USER|0x002e)
#define ART_Str2Init        (TAG_USER|0x002f)
#define ART_Comment         (TAG_USER|0x002f)
#define ART_ListTitle       (TAG_USER|0x0030)
#define ART_ListFunc        (TAG_USER|0x0031)
#define ART_ListInit        (TAG_USER|0x0032)
#define ART_ListSelect      (TAG_USER|0x0033)

#define ART_List2StrFunc    (TAG_USER|0x0040)
#define ART_Str2ListFunc    (TAG_USER|0x0041)

#define ART_UserData        (TAG_USER|0x0050)

/*---- AZ_LaunchProc() : Flags                     Défaut : */
#define AZTAG_LaunchF_Iconify    (TAG_USER|0x0001)  /* FALSE */
#define AZTAG_LaunchF_FileReq    (TAG_USER|0x0002)  /* FALSE */
#define AZTAG_LaunchF_NoWindow   (TAG_USER|0x0003)  /* FALSE */
#define AZTAG_LaunchF_ReadOnly   (TAG_USER|0x0004)  /* FALSE */
#define AZTAG_LaunchF_QuitAfter  (TAG_USER|0x0005)  /* FALSE */
#define AZTAG_LaunchF_CreateFile (TAG_USER|0x0006)  /* FALSE */

/*---- AZ_LaunchProc() : Paramètres */
#define AZTAG_Launch_FromCmd    (TAG_USER|0x0101)  /* commande ARexx en ligne */
#define AZTAG_Launch_FromScript (TAG_USER|0x0102)  /* script ARexx */
#define AZTAG_Launch_PortName   (TAG_USER|0x0103)  /* nom pour le port ARexx */
#define AZTAG_Launch_Priority   (TAG_USER|0x0104)  /* priorité pour le process */
#define AZTAG_Launch_Pattern    (TAG_USER|0x0105)  /* pattern pour le sélecteur de fichiers */
#define AZTAG_Launch_FileName   (TAG_USER|0x0106)  /* nom du fichier à charger */
#define AZTAG_Launch_ResultPort (TAG_USER|0x0107)  /* adresse où ranger le nom du port (UBYTE**) */
#define AZTAG_Launch_CmdObj     (TAG_USER|0x0108)  /* objet de commande créant la tâche */
#define AZTAG_Launch_QuickMsg   (TAG_USER|0x0109)  /* doit répondre ce message quand terminée */
#define AZTAG_Launch_DirLock    (TAG_USER|0x010a)  /* lock sur répertoire sur lequel le process va faire DupLock() puis CurrentDir() */
#define AZTAG_Launch_LeftTop    (TAG_USER|0x010b)  /* Position d'ouverture de la fenêtre */
#define AZTAG_Launch_WidthHeight (TAG_USER|0x010c) /* Taille d'ouverture de la fenêtre */
#define AZTAG_Launch_TextWidthHeight (TAG_USER|0x010d)  /* taille en caractères */

/*---------------------------------- */

/*---- Preferences flags : Flags1 ----*/
#define AZP_CHARDELBLOCK    0x00000001  /* La frappe d'un caractère supprime les blocs en cours */
#define AZP_DEBUG           0x00000010  /* Affiche des infos dans une console */
#define AZP_BKPFILENAME     0x00000004  /* Utilise le nom du fichier comme nom de backup */
#define AZP_USEASL          0x00000008  /* Utilise Le FileSelectore de l'asl.library */
#define AZP_ERRORQUIET      0x00000010  /* Pas de requêtes d'affichage des erreurs */
#define AZP_SAVEICON        0x00000020  /* Crée une icône après avoir sauvegardé */
#define AZP_KEEPBKP         0x00000040  /* Recopie le fichier dans T: avant de sauvegarder */
#define AZP_OVERLAY         0x00000080  /* Ecrit par-dessus au lieu d'insérer */
#define AZP_STATUSLOCK      0x00000100  /* Bloque le rafraichissement de la barre de status */
#define AZP_ABORTAREXX      0x00000200  /* Retourne 20 pour la prochaine commande ARexx */
#define AZP_DEFAULTBLM      0x00000400  /* Blocks par lignes entières */
#define AZP_AUTOBACKUP      0x00000800  /* Sauvegarde automatique */
#define AZP_AUTOSAVE        AZP_AUTOBACKUP
#define AZP_MINITEL         0x00001000  /* Indique que le port série est relié à un minitel */
#define AZP_AREXXLOCK       0x00002000  /* Bloque l'exécution des commandes venant de ARexx */
#define AZP_FRONTSCREEN     0x00004000  /* L'ouverture à été demandée sur l'écran de devant */
#define AZP_AZURSCREEN      0x00008000  /* L'ouverture à été demandée sur un écran AZur */
#define AZP_READONLY        0x00010000  /* Pas de modification possible */
#define AZP_NOWINDOW        0x00020000  /* Pas de fenêtre ouverte (tâche de fond) */
#define AZP_DISPLOCK        0x00040000  /* Affichage verrouillé */
#define AZP_RESIDENT        0x00080000  /* La tâche maîtresse doit rester en mémoire */
#define AZP_OPENREQ         0x00100000  /* Fileselector dès l'ouverture d'une fenêtre sans fichier */
#define AZP_NOAUTOSIZE      0x00200000  /* Pas d'adaptation des bords de la fenêtre */
#define AZP_INPUTLOCK       0x00400000  /* Pas d'entrée de la part de l'utilisateur */
#define AZP_SAVE            0x00800000  /* Sauvegarder le fichier juste avant de quitter */
#define AZP_REQWIN          0x01000000  /* Les requêtes sont dans des fenêtres au lieu de requesters */
#define AZP_FULLSCREEN      0x02000000  /* La fenêtre doit s'ouvrir en plein écran */
#define AZP_SAVELOCK        0x04000000  /* Bloque la commande de sauvegarde */
#define AZP_NULLPOINTER     0x08000000  /* Supprime le pointeur après utilisation du clavier */
#define AZP_QUITAFTER       0x10000000  /* Se termine après la fermeture de la fenêtre */
#define AZP_REXXNAMESEP     0x20000000  /* Met un séparateur entre le nom du port et son numéro */
#define AZP_REXXNAMENUM     0x40000000  /* Numérote dès le premier nom de port ARexx */
#define AZP_CREATEFILE      0x80000000  /* File2NewWin : mode création pour le file requester */

#define AZM_NODISPLAY (AZP_NOWINDOW|AZP_DISPLOCK)

/*---- Preferences flags : Flags2 ----*/
#define AZP_TAB2SPACE       0x00000001  /* Toute tabulation est transformée (load + edit) */
#define AZP_SPACE2TAB       0x00000002  /* Tous espaces sont transformés en tabulation */
#define AZP_REALTAB         0x00000004  /* Tabulations réelles */
#define AZP_TABDISPLAYS     0x00000008  /* On affiche un symbole spécial pour la tabulation */
#define AZP_TABDISPLAYE     0x00000010  /* On affiche le vide de tabulation */
#define AZP_SPACEDISPLAY    0x00000020  /* Affichage des espaces en pointillés */
#define AZP_BLINKON         0x00000040  /* Clignotement du curseur */
#define AZP_SHOWEOL         0x00000080  /* Montre fins de lignes */
#define AZP_SHOWEOF         0x00000100  /* Montre fin de fichier */
#define AZP_STRIPCHANGE     0x00000200  /* Enlève les espaces de fin de ligne à chaque changement de ligne */
#define AZP_STRIPSAVE       0x00000400  /* Enlève les espaces de fin de ligne lors de la sauvegarde */
#define AZP_STRIPLOAD       0x00000800  /* Enlève les espaces de fin de ligne lors du chargement */
#define AZP_LOADCR2LF       0x00001000  /* Transforme CR sans LF en LF lors du chargement */
#define AZP_LOADCRLF2LF     0x00002000  /* Transforme CR avec LF en LF lors du chargement */
#define AZP_SAVELF2CR       0x00004000  /* Transforme LF en CR seul lors de la sauvegarde */
#define AZP_SAVELF2CRLF     0x00008000  /* Transforme LF en CR + LF lors de la sauvegarde */
#define AZP_WORDWRAP        0x00010000  /* Passe à la ligne par mots entiers */
#define AZP_CORRECTCASE     0x00020000  /* Corrige les majuscules/minuscules */
#define AZP_USEMARGIN       0x00040000  /* Utilise les marges */
#define AZP_NUMBERLINES     0x00080000  /* Numérote les lignes */

/*---- Preferences flags : SearchFlags ----*/
#define SEF_CASE        0x00000001    /* Tiens compte des majuscules */
#define SEF_ACCENT      0x00000004    /* Tiens compte des accents */
#define SEF_ALL         0x00000008    /* Chercher tout : puis requester pour donner le total    */
#define SEF_BACKWARD    0x00000010    /* En arrière */
#define SEF_BLOCK       0x00000020    /* Recherche seulement dans les blocks                    */
#define SEF_TOBLOCK     0x00000040    /* Trouvées remplacées mis en blocks */
#define SEF_CONVERT     0x00000080    /* Conversion type C                                      */
#define SEF_PATTERN     0x00000100    /* ? = n'importe quel caractère */
#define SEF_FULL        0x00000200    /* Tout le texte */
#define SEF_STARTOFLINE 0x00000400    /* Restriction au début de ligne */
#define SEF_ENDOFLINE   0x00000800    /* Restriction au début de ligne */
#define SEF_NOVERIFY    0x00001000    /* Pas de vérification avant remplacement */
#define SEF_STARTOFWORD 0x00002000    /* Restriction au début de mot */
#define SEF_ENDOFWORD   0x00004000    /* Restriction au début de mot */

#define SEF_WORD        (SEF_STARTOFWORD|SEF_ENDOFWORD)

/*---- Macros de test des Flags ----*/
#define TST_CHARDELBLOCK    (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_CHARDELBLOCK)
#define TST_DEBUG           (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_DEBUG)
#define TST_BKPFILENAME     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_BKPFILENAME)
#define TST_USEASL          (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_USEASL)
#define TST_ERRORQUIET      (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_ERRORQUIET)
#define TST_SAVEICON        (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_SAVEICON)
#define TST_KEEPBKP         (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_KEEPBKP)
#define TST_OVERLAY         (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_OVERLAY)
#define TST_STATUSLOCK      (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_STATUSLOCK)
#define TST_ABORTAREXX      (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_ABORTAREXX)
#define TST_DEFAULTBLM      (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_DEFAULTBLM)
#define TST_AUTOBACKUP      (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_AUTOBACKUP)
#define TST_AUTOSAVE        TST_AUTOBACKUP
#define TST_MINITEL         (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_MINITEL)
#define TST_AREXXLOCK       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_AREXXLOCK)
#define TST_FRONTSCREEN     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_FRONTSCREEN)
#define TST_AZURSCREEN      (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_AZURSCREEN)
#define TST_READONLY        (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_READONLY)
#define TST_NOWINDOW        (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_NOWINDOW)
#define TST_DISPLOCK        (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_DISPLOCK)
#define TST_RESIDENT        (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_RESIDENT)
#define TST_OPENREQ         (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_OPENREQ)
#define TST_NOAUTOSIZE      (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_NOAUTOSIZE)
#define TST_INPUTLOCK       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_INPUTLOCK)
#define TST_SAVE            (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_SAVE)
#define TST_REQWIN          (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_REQWIN)
#define TST_FULLSCREEN      (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_FULLSCREEN)
#define TST_SAVELOCK        (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_SAVELOCK)
#define TST_NULLPOINTER     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_NULLPOINTER)
#define TST_QUITAFTER       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_QUITAFTER  )
#define TST_REXXNAMESEP     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_REXXNAMESEP)
#define TST_REXXNAMENUM     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZP_REXXNAMENUM)

#define TST_NODISPLAY       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags1 & AZM_NODISPLAY)

#define TST_TAB2SPACE       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_TAB2SPACE)
#define TST_SPACE2TAB       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_SPACE2TAB)
#define TST_REALTAB         (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_REALTAB)
#define TST_TABDISPLAYS     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_TABDISPLAYS)
#define TST_TABDISPLAYE     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_TABDISPLAYE)
#define TST_SPACEDISPLAY    (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_SPACEDISPLAY)
#define TST_BLINKON         (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_BLINKON)
#define TST_SHOWEOL         (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_SHOWEOL)
#define TST_SHOWEOF         (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_SHOWEOF)
#define TST_STRIPCHANGE     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_STRIPCHANGE)
#define TST_STRIPSAVE       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_STRIPSAVE  )
#define TST_STRIPLOAD       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_STRIPLOAD  )
#define TST_LOADCR2LF       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_LOADCR2LF  )
#define TST_LOADCRLF2LF     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_LOADCRLF2LF)
#define TST_SAVELF2CR       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_SAVELF2CR  )
#define TST_SAVELF2CRLF     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_SAVELF2CRLF)
#define TST_WORDWRAP        (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_WORDWRAP   )
#define TST_CORRECTCASE     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_CORRECTCASE)
#define TST_USEMARGIN       (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_USEMARGIN  )
#define TST_NUMBERLINES     (((AZURPROC*)AZ_GetAZur())->Prefs->Flags2 & AZP_NUMBERLINES)
