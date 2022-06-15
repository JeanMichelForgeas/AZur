
#include <devices/keymap.h>
#include <intuition/intuitionbase.h>

#include "uiki:uikbuf.h"
#include "uiki:uiklist.h"
#include "uiki:uikbox.h"
#include "uiki:uikcolors.h"

#include "uiki:objmenu.h"

#include "lci:text.h"
#include "lci:_pool_protos.h"
#include "lci:arg3_protos.h"

/*#define ZONE_IS_GAD  /* touch objfile.c objview.c objzone.c */ */
//#define AZUR_PRO    1

/*
touch arexx.c datatext.c filedef_key.c funcs_fileio.c funcs_prefs.c funcs_reqs.c funcs_text.c main.c objfile.c
touch arexx.c.info datatext.c.info filedef_key.c.info funcs_fileio.c.info funcs_prefs.c.info funcs_reqs.c.info funcs_text.c.info main.c.info objfile.c.info
*/
//#define AZUR_DEMO               1
#define AZURDEMO_MAXSIZE    10000   /* touch arexx.c filedef_key.c funcs_reqs.c funcs_text.c */
#define AZURDEMO_MAXLINES     400   /* touch objfile.c */
#define AZURDEMO_MAXTIME     3600   /* touch funcs_fileio.c main.c objfile.c */

#define PTRT_NULL      0
#define PTRT_WAITING   1
#define PTRT_SYSTEM    2
#define PTRT_MACRECORD 3
#define PTRT_MACPLAY   4

#define NAME_IS_NULL 0
#define NAME_IS_VOID 1
#define NAME_IS_FILE 2
#define NAME_IS_DIR  3

/*----------------- LoadFile() */
#define LOAF_DECOMPRESS     0x0001
#define LOAF_RES_FILENOICON 0x0002

/*----------------- SaveFile() */
#define SAVF_PARAGRAPH      0x0001
#define SAVF_SPACE2TAB      0x0002
#define SAVF_AUTSFILE       0x0004
#define SAVF_COMPRESS       0x0008

/*----------------- TextFormat() */
#define FMTF_JOINPARA   0x1000
#define FMTF_ONLYTWO    0x2000
#define FMTF_KEEPALINEA 0x4000

/*----------------- TextJustify() */
#define JUS_CENTER  1
#define JUS_FLUSH   2
#define JUS_LEFT    3
#define JUS_RIGHT   4
#define JUS_MASK    0x07

/*----------------- ComputeLineColFlags() */
#define CLCF_FOLDED     0x0001

/*----------------- */

struct memfold {
    LONG sline;
    LONG eline;
    LONG folded;
    LONG id;
    };

struct mempar {
    LONG sline;
    LONG eline;
    };

/*----------------- */

struct WaitAppl
    {
    struct Node Node;
    APTR    RMsg;
    UBYTE   Name[1];
    };

/*----------------- */
#define INTERFRAME_WIDTH    8
#define INTERGAD_WIDTH      8

struct AZLayout
    {
    UBYTE   Depth;
    UBYTE   Id;
    UWORD   Flags;
    ULONG   Title;
    WORD    Left, Top;
    WORD    Width, Height;
    WORD    TotW, TotH;
    void    (*Func)();
    ULONG   UserData;
    ULONG   UserParm;
    struct UIKObj *Obj;
    };

#define AZLID_GADSPACE  1
#define AZLID_FRAME     2
#define AZLID_INVISIBLE 3
#define AZLID_GADRADIO  4
#define AZLID_GADCHECK  5
#define AZLID_GADSTRNG  6
#define AZLID_GADSTNUM  7
#define AZLID_GADSTWFI  8
#define AZLID_GADSTFO1  9
#define AZLID_GADSTFO2 10

#define AZLF_TITLE_LEFT     0x0001
#define AZLF_TITLE_RIGHT    0x0002
#define AZLF_BOX_ACTIVE     0x0008
#define AZLF_BOX_DISABLE    0x0010
#define AZLF_SAMELINE       0x0020
#define AZLF_SUB_RIGHT      0x0040
#define AZLF_SUB_EQUAL      0x0080
#define AZLF_ATT_LEFT       0x1000
#define AZLF_ATT_TOP        0x2000
#define AZLF_ATT_RIGHT      0x4000
#define AZLF_ATT_BOT        0x8000

/*------------------- */

struct AZRequest
    {
    struct UIKObj   *Obj;
    ULONG           Flags;
    ULONG           ErrMsg;
    UBYTE           Button;
    UBYTE           padb;
    UWORD           padw;
    APTR            Info;
    UWORD           Left, Top, Width, Height;
    struct UIKObjWindow *WO;
    };

/*---- AZRequest Flags, aussi pour AZ_SmartRequest() */
#define ARF_WINDOW      0x0001  /* ouvre une fenêtre au lieu d'un requester */
#define ARF_ACTIVATE    0x0002  /* active la première zone de saisie */
#define ARF_RETQUIET    0x0004  /* ne termine pas si RETURN dans zone de saisie */
#define ARF_STRGAD      0x0008
#define ARF_ASYNC       0x0010
#define ARF_NOTERM      0x0020  /* si async : ne termine pas sur OK */
#define ARF_TOPREFS     0x0040
#define ARF_MEMPOS      0x0080
#define ARF_NOSORT      0x0100

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

/*---- Evènements résultant des requesters synchrones : AZ_SmartRequest(), etc... */
#define REQBUT_NOEVENT  -1
#define REQBUT_CANCEL   0
#define REQBUT_OK       1
#define REQBUT_MIDDLE   2
#define REQBUT_CTRL_C   3

/*------------------- */

struct AZSema
    {
    struct SignalSemaphore Semaphore;
    UWORD   ReadCount;
    UBYTE   Flags;
    UBYTE   padb;
    };
#define AZSEMF_WRITE 0x01


struct AZDisplayCmd
    {
    LONG    DisplayNum;
    LONG    DisplayFrom;
    LONG    ScrollNum;
    LONG    ScrollFrom;
    LONG    Line;
    LONG    Col;
    };

struct read_file
    {
    void (*readfn)();
    UBYTE *mem, *max, *ptr;
    UBYTE menus, keys;
    UBYTE ok;
    UBYTE diese;
    UBYTE padb;
    ULONG offs;
    UBYTE *linestart;
    };

struct AZKey
    {
    UWORD Code;
    ULONG Qualifier;
    };
struct AZShortcut
    {
    struct CmdObj   *CO;
    UBYTE           *CmdName, *CmdLine;
    UWORD           NumKeys;
    struct AZKey    Keys[1];
    };

struct KeyHandle
    {
    struct UIKPList KeyList;
    struct CmdObj   *NoSpecial;

    struct AZKey    *CurKeys;       /* rangement de la séquence entrée */
    UWORD           IndexCurKey;    /* index dans la combinaison de touches */
    UWORD           IndexTabQual;   /* index dans tableau CodeKeyTab[128][8] */

    struct AZKey    RecKey;         /* touche reçue */
    ULONG           AZQual;         /* quals supplémentaire */

    struct AZShortcut *KeyFound;    /* après recherche : 0 ou ShortCut */

    struct CmdObj   *BeforeKey;
    struct CmdObj   *AfterKey;
    struct CmdObj   *BeforeLine;
    struct CmdObj   *AfterLine;

    APTR            Pool;
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

struct TimeLimit
    {
    ULONG   StartSeconds, StartMicros;
    ULONG   Seconds, Micros;
    ULONG   StartLine, StartCol;
    WORD    OldMouseX, OldMouseY;
    UBYTE   ClicStatus;
    UBYTE   padb;
    UWORD   padw;
    };

struct MouseHandle
    {
    struct UIKPList KeyList;
    struct CmdObj   *NoSpecial;

    struct AZKey    *CurKeys;       /* rangement de la séquence entrée */
    UWORD           IndexCurKey;    /* index dans la combinaison de touches */
    UWORD           TickFlags;

    struct AZKey    RecKey;         /* touche reçue */
    struct AZShortcut *KeyFound;    /* après recherche : 0 ou ShortCut */

    struct TimeLimit TLL;           /* pour clics Left button */
    struct TimeLimit TLM;           /* pour clics Middle button */
    struct TimeLimit TLR;           /* pour clics Right button */

    ULONG           JoyQuals;
    APTR            Pool;
    };

struct AZEqual
    {
    UBYTE *Equal;
    UBYTE *TrueCmd;
    };
struct EqualHandle
    {
    APTR            Pool;
    struct UIKPList EqualList;
    };

struct MenuHandle
    {
    APTR                Pool;
    struct UIKMenuEntry *MenuEntry;
    UBYTE               ComKeys[256];
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
#define SCODE_TESTRBUT  0x8000

#define SQUAL_MJOY  (IEQUALIFIER_MIDBUTTON|IEQUALIFIER_RBUTTON|IEQUALIFIER_LEFTBUTTON|SQUAL_DBLCLIC|SQUAL_TPLCLIC|SQUAL_JOYRIGHT|SQUAL_JOYLEFT|SQUAL_JOYDOWN|SQUAL_JOYUP|SQUAL_JOYFIRE)

struct AZViewInfo
    {
    struct IBox ViewBox;
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

#define NUMFOLDINFOS 5

struct FoldDrawInfo
    {
    UBYTE OnFlags;
    UBYTE OnAPen;
    UBYTE OnBPen;
    UBYTE pad1;
    UBYTE OffFlags;
    UBYTE OffAPen;
    UBYTE OffBPen;
    UBYTE pad2;
    };

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
    struct FoldDrawInfo FDI[NUMFOLDINFOS];
    struct UIKColors ColRegs;
    UWORD       ColorTable[MAX_UIKCOLORS];

    UBYTE       AppIconName[32];
    };
/* faire "touch main.c cmd_file.c cmd_div.c funcs_prefs.c" après ajout */

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
    struct IntuitionBase *IntuiBase;
    struct UIKGlobal    *UIK;
    struct AZur         *AZMast;

    struct UIKObj       *WinHelp;
    struct UIKObjARexxSim *ARexx;
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
    UBYTE               OKSigBit;
    UBYTE               ErrorSigBit;
    WORD                RexxError;
    UIKBUF              *RexxResult;
    APTR                Pool;
    UBYTE               ExeFrom;
    UBYTE               NullPointer;
    UWORD               WorkFlags;
    UIKBUF              *LastCmd;

    /*--- seulement pour process */
    struct AZObjFile    *FObj;
    ULONG               UserData;
    ULONG               RexxConsole;
    APTR                RexxConsoleMsg;
    ULONG               DOSConsole;
    ULONG               NilFh;
    ULONG               Tmp;
    UIKBUF              *ARexxFileName;
    UIKBUF              *DOSFileName;
    APTR                QuickMsg;
    UBYTE               MaxError;
    BYTE                WaitPtrCount;
    UBYTE               PtrType;
    UBYTE               padb;

    /*--- pour AZur et process */
    struct AZLayout     *SearchLayout;
    struct AZLayout     *Edit1Layout;
    struct AZLayout     *Edit2Layout;
    struct AZLayout     *Env1Layout;
    struct AZLayout     *Env2Layout;
    struct AZLayout     *WindowLayout;
    struct AZLayout     *ReplaceLayout;
    struct AZLayout     *SearchPrefsLayout;
    struct AZLayout     *PaletteLayout;
    struct AZLayout     *LayoutReserved[2]; /* faire PoolFree dans funcs_prefs.c */

    struct AZRequest    AZReqDisplayPrefs;
    struct AZRequest    AZReqEditPrefs;
    struct AZRequest    AZReqEnvPrefs;
    struct AZRequest    AZReqSearchPrefs;
    struct AZRequest    AZReqPrintPrefs;
    struct AZRequest    AZReqFontPrefs;
    struct AZRequest    AZReqWindowPrefs;
    struct AZRequest    AZReqSearch;
    struct AZRequest    AZReqReplace;
    struct AZRequest    AZReqInfo;
    struct AZRequest    AZReqAbout;
    struct AZRequest    AZReqLine;
    struct AZRequest    AZReqOffset;
    struct AZRequest    AZReqBookmark;
    struct AZRequest    AZReqHex;
    struct AZRequest    AZReqReserved[4];

    UIKBUF              *UBufTmp;

    struct MenuHandle   *MainMenuHandle;
    struct MenuHandle   *MenuHandle;
    struct KeyHandle    *KeyHandle;
    UIKBUF              *HotKeyHandle;
    struct MouseHandle  *MouseHandle;
    UIKBUF              *DictHandle;
    UIKBUF              *TmplHandle;
    struct EqualHandle  *EqualHandle;

    ULONG               padl[5];
    UWORD               padw;
    UWORD               LastError;
    UIKBUF              *FileSelResult;
    struct IconBase     *IconBase;
    };
typedef struct AZurProc AZURPROC;

struct AZProcParms
    {
    struct Process      *ParmParentTask;
    UBYTE               *ParmFromCmd;
    UBYTE               *ParmFromScript;
    UBYTE               *ParmPortName;
    UBYTE               *ParmPattern;
    UBYTE               *ParmFileName;
    ULONG               ParmIconify;
    ULONG               ParmFlags1;
    ULONG               ParmFlags2;
    ULONG               ParmFlags3;
    ULONG               ParmSearchFlags;
    UBYTE               *ParmResultPort;    /* argument en retour */
    struct CmdObj       *ParmCmdObj;
    ULONG               ParmQuickMsg;
    ULONG               ParmDirLock;
    ULONG               ParmLeftTop;
    ULONG               ParmWidthHeight;
    ULONG               ParmTextWH;
    struct AZurPrefs    *ParmPrefs;
    UBYTE               *ParmPrefsName;
    UBYTE               *ParmPrefsFileName;
    };

struct AZurNode
    {
    struct  Node *ln_Succ;
    struct  Node *ln_Pred;
    UBYTE   ln_Type;
    BYTE    ln_Pri;
    struct Task *ln_Task;
    UBYTE   *ln_PortName;
    };

struct AZur
    {
    /*--- commun au master et aux process */
    struct UIKBase      *UIKBase;
    struct DOSBase      *DOSBase;
    struct GfxBase      *GfxBase;
    struct ExecBase     *ExecBase;
    struct LayersBase   *LayersBase;
    struct IntuitionBase *IntuiBase;
    struct UIKGlobal    *UIK;
    struct AZur         *AZMast;

    struct UIKObj       *WinHelp;
    struct UIKObjARexxSim *ARexx;
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
    UBYTE               OKSigBit;
    UBYTE               ErrorSigBit;
    WORD                RexxError;
    UIKBUF              *RexxResult;
    APTR                Pool;
    UBYTE               ExeFrom;
    UBYTE               NullPointer;
    UWORD               WorkFlags;
    UIKBUF              *LastCmd;

    /*--- seulement pour process */
    struct AZObjFile    *FObj;
    ULONG               UserData;
    ULONG               RexxConsole;
    APTR                RexxConsoleMsg;
    ULONG               DOSConsole;
    ULONG               NilFh;
    ULONG               Tmp;
    UIKBUF              *ARexxFileName;
    UIKBUF              *DOSFileName;
    APTR                QuickMsg;
    UBYTE               MaxError;
    BYTE                WaitPtrCount;
    UBYTE               PtrType;
    UBYTE               padb;

    /*--- pour AZur et process */
    struct AZLayout     *SearchLayout;
    struct AZLayout     *Edit1Layout;
    struct AZLayout     *Edit2Layout;
    struct AZLayout     *Env1Layout;
    struct AZLayout     *Env2Layout;
    struct AZLayout     *WindowLayout;
    struct AZLayout     *ReplaceLayout;
    struct AZLayout     *SearchPrefsLayout;
    struct AZLayout     *PaletteLayout;
    struct AZLayout     *LayoutReserved[2];

    struct AZRequest    AZReqDisplayPrefs;
    struct AZRequest    AZReqEditPrefs;
    struct AZRequest    AZReqEnvPrefs;
    struct AZRequest    AZReqSearchPrefs;
    struct AZRequest    AZReqPrintPrefs;
    struct AZRequest    AZReqFontPrefs;
    struct AZRequest    AZReqWindowPrefs;
    struct AZRequest    AZReqSearch;
    struct AZRequest    AZReqReplace;
    struct AZRequest    AZReqInfo;
    struct AZRequest    AZReqAbout;
    struct AZRequest    AZReqLine;
    struct AZRequest    AZReqOffset;
    struct AZRequest    AZReqBookmark;
    struct AZRequest    AZReqHex;
    struct AZRequest    AZReqReserved[4];

    UIKBUF              *UBufTmp;

    struct MenuHandle   *MainMenuHandle;
    struct MenuHandle   *MenuHandle;
    struct KeyHandle    *KeyHandle;
    UIKBUF              *HotKeyHandle;
    struct MouseHandle  *MouseHandle;
    UIKBUF              *DictHandle;
    UIKBUF              *TmplHandle;
    struct EqualHandle  *EqualHandle;

    ULONG               padl[5];
    UWORD               padw;
    UWORD               LastError;
    UIKBUF              *FileSelResult;
    struct IconBase     *IconBase;

    /*--- seulement pour master */
    struct List         FileList;
    ULONG               FileCount;

    struct AZProcParms  *Parms;         /* arguments */

    struct List         ProcList;
    struct SignalSemaphore ProcSem;     /* Semaphore pour lancement de process */
    struct SignalSemaphore RGSem;       /* Semaphore d'accés aux CmdGerms */
    struct UIKPList     oldRGList;      /* liste des CmdGerms */

    struct AZurNode     *LastActNode;
    UBYTE               *EnvFileName;
    UBYTE               *ArcFileName;

    UBYTE               *CmdLine;
    struct ParseArgs    *PArg;
    APTR                *ArgRes;
    APTR                *Parse;

    UBYTE               *FromName;
    UBYTE               *FromCmdName;
    APTR                RGHash;
    UBYTE               *PatSel;
    UBYTE               *PubScreen;
    struct Screen       *ScreenAdd;
    struct UIKObjScreen *ScreenObj;

    ULONG               MaxLines;
    UBYTE               OldPri;
    UBYTE               padb2;
    UWORD               ProcNum;
    void                (*CtrlDVect)();

    struct UIKList      VarList;
    struct MsgPort      LaunchPort;
    struct AZSema       *ObjSema;
    struct AZSema       *VarSema;
    struct UIKObjWindow *WinObj;
    struct UIKObjMenu   *MenuObj;
    struct List         RexxNameList;
    struct SignalSemaphore RexxNameSem;

    struct MsgPort      *AppIconPort;
    struct AppIcon      *AppIcon;
    APTR                AppIconVect;
    ULONG               NewStack[3];
    int                 argc;
    UBYTE               **argv;
    UBYTE               PubScreenName[16];
    struct TagItem      IntuiScreenTags[3];
    struct DiskObject   *DiskObj;
    };
typedef struct AZur AZUR;

/*---- WorkFlags */
#define WRF_MAC_RECORDING   0x0001
#define WRF_MAC_QUIET       0x0002
#define WRF_MAC_PTRLOCK     0x0004
#define WRF_MAC_PLAYING     0x0008

/*---- AZ_LaunchProc() : Flags */
#define AZTAG_LaunchF_Iconify    (TAG_USER|0x0001) /* FALSE */
#define AZTAG_LaunchF_FileReq    (TAG_USER|0x0002) /* FALSE */
#define AZTAG_LaunchF_NoWindow   (TAG_USER|0x0003) /* FALSE */
#define AZTAG_LaunchF_ReadOnly   (TAG_USER|0x0004) /* FALSE */
#define AZTAG_LaunchF_QuitAfter  (TAG_USER|0x0005) /* FALSE */
#define AZTAG_LaunchF_CreateFile (TAG_USER|0x0006) /* FALSE */
#define AZTAG_LaunchF_IgnoreIcon (TAG_USER|0x0007) /* FALSE */
/* (penser à flags_boolmap[] dans main.c) */

/*---- AZ_LaunchProc() : Paramètres */
#define AZTAG_Launch_FromCmd     (TAG_USER|0x0101) /* commande ARexx en ligne */
#define AZTAG_Launch_FromScript  (TAG_USER|0x0102) /* script ARexx */
#define AZTAG_Launch_PortName    (TAG_USER|0x0103) /* nom pour le port ARexx */
#define AZTAG_Launch_Priority    (TAG_USER|0x0104) /* priorité pour le process */
#define AZTAG_Launch_Pattern     (TAG_USER|0x0105) /* pattern pour le sélecteur de fichiers */
#define AZTAG_Launch_FileName    (TAG_USER|0x0106) /* nom du fichier à charger */
#define AZTAG_Launch_ResultPort  (TAG_USER|0x0107) /* adresse où ranger le nom du port (UBYTE**) */
#define AZTAG_Launch_CmdObj      (TAG_USER|0x0108) /* objet de commande créant la tâche */
#define AZTAG_Launch_QuickMsg    (TAG_USER|0x0109) /* doit répondre ce message quand terminée */
#define AZTAG_Launch_DirLock     (TAG_USER|0x010a) /* lock sur répertoire sur lequel le process va faire DupLock() puis CurrentDir() */
#define AZTAG_Launch_LeftTop     (TAG_USER|0x010b) /* position en pixels */
#define AZTAG_Launch_WidthHeight (TAG_USER|0x010c) /* taille en pixels */
#define AZTAG_Launch_TextWidthHeight (TAG_USER|0x010d) /* taille en caractères */
#define AZTAG_Launch_PrefsFileName   (TAG_USER|0x010e) /* nom de fichier de préférences */

/*---------------------------------- */
#define AGM  (*((AZUR*)&GLOB))
#define AGP  (*((AZURPROC*)&GLOB))

#define UIKBase         (*((AZUR*)&GLOB)).UIKBase
#define DOSBase         (*((AZUR*)&GLOB)).DOSBase
#define GfxBase         (*((AZUR*)&GLOB)).GfxBase
#define SysBase         (*((AZUR*)&GLOB)).ExecBase
#define ExecBase        (*((AZUR*)&GLOB)).ExecBase
#define IconBase        (*((AZUR*)&GLOB)).IconBase
#define LayersBase      (*((AZUR*)&GLOB)).LayersBase
#define IntuitionBase   (*((AZUR*)&GLOB)).IntuiBase
#define WorkbenchBase   (*((AZUR*)&GLOB)).WorkbenchBase

/*---- Préférences flags : Flags1 ----*/
#define AZPM_NOTSAVED1  (AZP_DEBUG|AZP_ERRORQUIET|AZP_STATUSLOCK|AZP_ABORTAREXX|AZP_AREXXLOCK|AZP_DISPLOCK|AZP_RESIDENT|AZP_OPENREQ|AZP_INPUTLOCK|AZP_SAVE|AZP_SAVELOCK|AZP_QUITAFTER|AZP_CREATEFILE)

#define AZP_CHARDELBLOCK    0x00000001  /* La frappe d'un caractère supprime les blocs en cours */
#define AZP_DEBUG           0x00000002  /* Envoie des informations de débuggage dans une console */
#define AZP_BKPFILENAME     0x00000004  /* Utilise le nom du fichier comme nom de backup */
#define AZP_USEASL          0x00000008  /* Utilise l'asl.library pour le FileSelector */
#define AZP_ERRORQUIET      0x00000010  /* Pas de requêtes d'affichage des erreurs */
#define AZP_SAVEICON        0x00000020  /* Crée une icône après avoir sauvegardé */
#define AZP_KEEPBKP         0x00000040  /* Recopie le fichier dans T: avant de sauvegarder */
#define AZP_OVERLAY         0x00000080  /* Ecrit par-dessus au lieu d'insérer */
#define AZP_STATUSLOCK      0x00000100  /* Bloque le rafraichissement de la barre de status */
#define AZP_ABORTAREXX      0x00000200  /* Retourne 20 pour la prochaine commande ARexx */
#define AZP_DEFAULTBLM      0x00000400  /* Blocks par lignes entières */
#define AZP_AUTOSAVE        0x00000800  /* Backup toutes les prefs->BackupInt secondes */
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
#define AZP_QUITAFTER       0x10000000  /* Pas de UIK_Do2 après avoir exécuté les scripts ARexx en argument */
#define AZP_REXXNAMESEP     0x20000000  /* Met un séparateur entre le nom du port et son numéro */
#define AZP_REXXNAMENUM     0x40000000  /* Numérote dès le premier nom de port ARexx */
#define AZP_CREATEFILE      0x80000000  /* File2NewWin : mode création pour le file requester */

#define AZM_NODISPLAY (AZP_NOWINDOW|AZP_DISPLOCK)

/*---- Préférences flags : Flags2 ----*/
#define AZPM_NOTSAVED2  (AZP_LOADINGFILE|AZP_IGNOREICON|AZP_TOOLASKED|AZP_COMPRESSED )

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
#define AZP_AUTOFORMAT      0x00010000  /*  */
#define AZP_CORRECTCASE     0x00020000  /* Corrige les majuscules/minuscules */
#define AZP_USEMARGIN       0x00040000  /* Utilise les marges */
#define AZP_NUMBERLINES     0x00080000  /* Numérote les lignes */
#define AZP_IGNOREICON      0x00100000  /* File2NewWin : mode chargement sans lire l'icône */
#define AZP_LOADICON        0x00200000  /* Utilise les renseignements de l'icône après le chargement */
#define AZP_LOADINGFILE     0x00400000  /* Utilise les renseignements de l'icône après le chargement */
#define AZP_LEAVEREXCASE    0x00800000  /* ne met pas le nom du port en majuscules */
#define AZP_REQAUTOSAVE     0x01000000  /* Requête de confirmation pour l'autosauve */
#define AZP_TOOLASKED       0x02000000  /*  */
#define AZP_COMPRESSED      0x04000000  /*  */
#define AZP_APPICON         0x08000000  /* Une AppIcon pour la tâche mère */
#define AZP_NEWICONASKED    0x10000000  /* Une décision a été prise pour ne pas créer d'icône */

/*---- Préférences flags : Flags3 ----*/
#define AZPM_NOTSAVED3  (0)

/*---- Préférences flags : SearchFlags ----*/
#define AZPM_NOTSAVEDSEARCH  (SEF_DOPATTERN|SEF_VERIFYOK|SEF_DOLF|SEF_RDOLF|SEF_RECT)

#define SEF_CASE        0x00000001    /* Tiens compte des majuscules */
#define SEF_not_used    0x00000002    /*  */
#define SEF_ACCENT      0x00000004    /* Tiens compte des accents */
#define SEF_ALL         0x00000008    /* Chercher tout : puis requester pour donner le total */
#define SEF_BACKWARD    0x00000010    /* En arrière */
#define SEF_BLOCK       0x00000020    /* Recherche seulement dans les blocks */
#define SEF_TOBLOCK     0x00000040    /* Trouvées remplacées mis en blocks */
#define SEF_CONVERT     0x00000080    /* Conversion type C */
#define SEF_PATTERN     0x00000100    /* ? = n'importe quel caractère */
#define SEF_FULL        0x00000200    /* Tout le texte */
#define SEF_STARTOFLINE 0x00000400    /* Restriction au début de ligne */
#define SEF_ENDOFLINE   0x00000800    /* Restriction au début de ligne */
#define SEF_NOVERIFY    0x00001000    /* Pas de vérification avant remplacement */
#define SEF_STARTOFWORD 0x00002000    /* Restriction au début de mot */
#define SEF_ENDOFWORD   0x00004000    /* Restriction au début de mot */

#define SEF_WORD        (SEF_STARTOFWORD|SEF_ENDOFWORD)

/*!!! Mettre les suivants à 0 dans main.c après avoir lu les préférences */
#define SEF_RDOLF       0x04000000    /* tmp : mis selon le contenu de la chaine à remplacer */
#define SEF_DOPATTERN   0x08000000    /* tmp : mis selon le contenu de la chaine à remplacer */
#define SEF_VERIFYOK    0x20000000    /* tmp : pour la verif en cours de remplacement */
#define SEF_DOLF        0x40000000    /* tmp : mis selon le contenu de la chaine à chercher */
#define SEF_RECT        0x80000000    /* tmp : mis selon le bloc */


/*---- macros ----*/
#define TST_CHARDELBLOCK    ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_CHARDELBLOCK)
#define TST_DEBUG           ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_DEBUG)
#define TST_BKPFILENAME     ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_BKPFILENAME)
#define TST_USEASL          ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_USEASL)
#define TST_ERRORQUIET      ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_ERRORQUIET)
#define TST_SAVEICON        ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_SAVEICON)
#define TST_KEEPBKP         ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_KEEPBKP)
#define TST_OVERLAY         ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_OVERLAY)
#define TST_STATUSLOCK      ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_STATUSLOCK)
#define TST_ABORTAREXX      ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_ABORTAREXX)
#define TST_DEFAULTBLM      ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_DEFAULTBLM)
#define TST_AUTOSAVE        ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_AUTOSAVE)
#define TST_MINITEL         ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_MINITEL)
#define TST_AREXXLOCK       ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_AREXXLOCK)
#define TST_FRONTSCREEN     ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_FRONTSCREEN)
#define TST_AZURSCREEN      ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_AZURSCREEN)
#define TST_READONLY        ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_READONLY)
#define TST_NOWINDOW        ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_NOWINDOW)
#define TST_DISPLOCK        ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_DISPLOCK)
#define TST_RESIDENT        ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_RESIDENT)
#define TST_OPENREQ         ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_OPENREQ)
#define TST_NOAUTOSIZE      ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_NOAUTOSIZE)
#define TST_INPUTLOCK       ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_INPUTLOCK)
#define TST_SAVE            ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_SAVE)
#define TST_REQWIN          ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_REQWIN)
#define TST_FULLSCREEN      ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_FULLSCREEN)
#define TST_SAVELOCK        ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_SAVELOCK)
#define TST_NULLPOINTER     ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_NULLPOINTER)
#define TST_QUITAFTER       ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_QUITAFTER)
#define TST_REXXNAMESEP     ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_REXXNAMESEP)
#define TST_REXXNAMENUM     ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_REXXNAMENUM)
#define TST_CREATEFILE      ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZP_CREATEFILE )

#define TST_NODISPLAY       ((*((AZUR*)&GLOB)).Prefs->Flags1 & AZM_NODISPLAY)

#define TST_TAB2SPACE       ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_TAB2SPACE   )
#define TST_SPACE2TAB       ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_SPACE2TAB   )
#define TST_REALTAB         ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_REALTAB     )
#define TST_TABDISPLAYS     ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_TABDISPLAYS )
#define TST_TABDISPLAYE     ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_TABDISPLAYE )
#define TST_SPACEDISPLAY    ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_SPACEDISPLAY)
#define TST_BLINKON         ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_BLINKON     )
#define TST_SHOWEOL         ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_SHOWEOL     )
#define TST_SHOWEOF         ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_SHOWEOF     )
#define TST_STRIPCHANGE     ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_STRIPCHANGE )
#define TST_STRIPSAVE       ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_STRIPSAVE   )
#define TST_STRIPLOAD       ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_STRIPLOAD   )
#define TST_LOADCR2LF       ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_LOADCR2LF   )
#define TST_LOADCRLF2LF     ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_LOADCRLF2LF )
#define TST_SAVELF2CR       ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_SAVELF2CR   )
#define TST_SAVELF2CRLF     ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_SAVELF2CRLF )
#define TST_AUTOFORMAT      ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_AUTOFORMAT  )
#define TST_CORRECTCASE     ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_CORRECTCASE )
#define TST_USEMARGIN       ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_USEMARGIN   )
#define TST_NUMBERLINES     ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_NUMBERLINES )
#define TST_IGNOREICON      ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_IGNOREICON  )
#define TST_LOADICON        ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_LOADICON    )
#define TST_LOADINGFILE     ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_LOADINGFILE )
#define TST_LEAVEREXCASE    ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_LEAVEREXCASE)
#define TST_REQAUTOSAVE     ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_REQAUTOSAVE )
#define TST_TOOLASKED       ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_TOOLASKED   )
#define TST_COMPRESSED      ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_COMPRESSED  )
#define TST_APPICON         ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_APPICON     )
#define TST_NEWICONASKED    ((*((AZUR*)&GLOB)).Prefs->Flags2 & AZP_NEWICONASKED)

/*--- */

#define SET_CHARDELBLOCK    ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_CHARDELBLOCK)
#define SET_DEBUG           ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_DEBUG)
#define SET_BKPFILENAME     ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_BKPFILENAME)
#define SET_USEASL          ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_USEASL)
#define SET_ERRORQUIET      ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_ERRORQUIET)
#define SET_SAVEICON        ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_SAVEICON)
#define SET_KEEPBKP         ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_KEEPBKP)
#define SET_OVERLAY         ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_OVERLAY)
#define SET_STATUSLOCK      ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_STATUSLOCK)
#define SET_ABORTAREXX      ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_ABORTAREXX)
#define SET_DEFAULTBLM      ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_DEFAULTBLM)
#define SET_AUTOSAVE        ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_AUTOSAVE)
#define SET_MINITEL         ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_MINITEL)
#define SET_AREXXLOCK       ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_AREXXLOCK)
#define SET_FRONTSCREEN     ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_FRONTSCREEN)
#define SET_AZURSCREEN      ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_AZURSCREEN)
#define SET_READONLY        ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_READONLY)
#define SET_NOWINDOW        ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_NOWINDOW)
#define SET_DISPLOCK        ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_DISPLOCK)
#define SET_RESIDENT        ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_RESIDENT)
#define SET_OPENREQ         ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_OPENREQ)
#define SET_NOAUTOSIZE      ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_NOAUTOSIZE)
#define SET_INPUTLOCK       ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_INPUTLOCK)
#define SET_SAVE            ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_SAVE)
#define SET_REQWIN          ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_REQWIN)
#define SET_FULLSCREEN      ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_FULLSCREEN)
#define SET_SAVELOCK        ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_SAVELOCK)
#define SET_NULLPOINTER     ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_NULLPOINTER)
#define SET_QUITAFTER       ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_QUITAFTER)
#define SET_REXXNAMESEP     ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_REXXNAMESEP)
#define SET_REXXNAMENUM     ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_REXXNAMENUM)
#define SET_CREATEFILE      ((*((AZUR*)&GLOB)).Prefs->Flags1 |= AZP_CREATEFILE )

#define SET_TAB2SPACE       ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_TAB2SPACE   )
#define SET_SPACE2TAB       ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_SPACE2TAB   )
#define SET_REALTAB         ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_REALTAB     )
#define SET_TABDISPLAYS     ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_TABDISPLAYS )
#define SET_TABDISPLAYE     ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_TABDISPLAYE )
#define SET_SPACEDISPLAY    ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_SPACEDISPLAY)
#define SET_BLINKON         ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_BLINKON     )
#define SET_SHOWEOL         ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_SHOWEOL     )
#define SET_SHOWEOF         ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_SHOWEOF     )
#define SET_STRIPCHANGE     ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_STRIPCHANGE )
#define SET_STRIPSAVE       ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_STRIPSAVE   )
#define SET_STRIPLOAD       ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_STRIPLOAD   )
#define SET_LOADCR2LF       ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_LOADCR2LF   )
#define SET_LOADCRLF2LF     ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_LOADCRLF2LF )
#define SET_SAVELF2CR       ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_SAVELF2CR   )
#define SET_SAVELF2CRLF     ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_SAVELF2CRLF )
#define SET_AUTOFORMAT      ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_AUTOFORMAT  )
#define SET_CORRECTCASE     ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_CORRECTCASE )
#define SET_USEMARGIN       ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_USEMARGIN   )
#define SET_NUMBERLINES     ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_NUMBERLINES )
#define SET_IGNOREICON      ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_IGNOREICON  )
#define SET_LOADICON        ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_LOADICON    )
#define SET_LOADINGFILE     ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_LOADINGFILE )
#define SET_LEAVEREXCASE    ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_LEAVEREXCASE)
#define SET_REQAUTOSAVE     ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_REQAUTOSAVE )
#define SET_TOOLASKED       ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_TOOLASKED   )
#define SET_COMPRESSED      ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_COMPRESSED  )
#define SET_APPICON         ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_APPICON     )
#define SET_NEWICONASKED    ((*((AZUR*)&GLOB)).Prefs->Flags2 |= AZP_NEWICONASKED)

/*--- */

#define CLR_CHARDELBLOCK    ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_CHARDELBLOCK)
#define CLR_DEBUG           ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_DEBUG)
#define CLR_BKPFILENAME     ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_BKPFILENAME)
#define CLR_USEASL          ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_USEASL)
#define CLR_ERRORQUIET      ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_ERRORQUIET)
#define CLR_SAVEICON        ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_SAVEICON)
#define CLR_KEEPBKP         ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_KEEPBKP)
#define CLR_OVERLAY         ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_OVERLAY)
#define CLR_STATUSLOCK      ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_STATUSLOCK)
#define CLR_ABORTAREXX      ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_ABORTAREXX)
#define CLR_DEFAULTBLM      ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_DEFAULTBLM)
#define CLR_AUTOSAVE        ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_AUTOSAVE)
#define CLR_MINITEL         ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_MINITEL)
#define CLR_AREXXLOCK       ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_AREXXLOCK)
#define CLR_GETSCREEN       ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_GETSCREEN)
#define CLR_FRONTSCREEN     ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_FRONTSCREEN)
#define CLR_AZURSCREEN      ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_AZURSCREEN)
#define CLR_READONLY        ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_READONLY)
#define CLR_NOWINDOW        ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_NOWINDOW)
#define CLR_DISPLOCK        ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_DISPLOCK)
#define CLR_RESIDENT        ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_RESIDENT)
#define CLR_OPENREQ         ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_OPENREQ)
#define CLR_NOAUTOSIZE      ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_NOAUTOSIZE)
#define CLR_INPUTLOCK       ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_INPUTLOCK)
#define CLR_SAVE            ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_SAVE)
#define CLR_REQWIN          ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_REQWIN)
#define CLR_FULLSCREEN      ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_FULLSCREEN)
#define CLR_SAVELOCK        ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_SAVELOCK)
#define CLR_NULLPOINTER     ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_NULLPOINTER)
#define CLR_QUITAFTER       ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_QUITAFTER)
#define CLR_REXXNAMESEP     ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_REXXNAMESEP)
#define CLR_REXXNAMENUM     ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_REXXNAMENUM)
#define CLR_CREATEFILE      ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZP_CREATEFILE )

#define CLRM_NODISPLAY      ((*((AZUR*)&GLOB)).Prefs->Flags1 &= ~AZM_NODISPLAY)

#define CLR_TAB2SPACE       ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_TAB2SPACE   )
#define CLR_SPACE2TAB       ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_SPACE2TAB   )
#define CLR_REALTAB         ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_REALTAB     )
#define CLR_TABDISPLAYS     ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_TABDISPLAYS )
#define CLR_TABDISPLAYE     ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_TABDISPLAYE )
#define CLR_SPACEDISPLAY    ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_SPACEDISPLAY)
#define CLR_BLINKON         ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_BLINKON     )
#define CLR_SHOWEOL         ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_SHOWEOL     )
#define CLR_SHOWEOF         ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_SHOWEOF     )
#define CLR_STRIPCHANGE     ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_STRIPCHANGE )
#define CLR_STRIPSAVE       ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_STRIPSAVE   )
#define CLR_STRIPLOAD       ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_STRIPLOAD   )
#define CLR_LOADCR2LF       ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_LOADCR2LF   )
#define CLR_LOADCRLF2LF     ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_LOADCRLF2LF )
#define CLR_SAVELF2CR       ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_SAVELF2CR   )
#define CLR_SAVELF2CRLF     ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_SAVELF2CRLF )
#define CLR_AUTOFORMAT      ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_AUTOFORMAT  )
#define CLR_CORRECTCASE     ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_CORRECTCASE )
#define CLR_USEMARGIN       ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_USEMARGIN   )
#define CLR_NUMBERLINES     ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_NUMBERLINES )
#define CLR_IGNOREICON      ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_IGNOREICON  )
#define CLR_LOADICON        ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_LOADICON    )
#define CLR_LOADINGFILE     ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_LOADINGFILE )
#define CLR_LEAVEREXCASE    ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_LEAVEREXCASE)
#define CLR_REQAUTOSAVE     ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_REQAUTOSAVE )
#define CLR_TOOLASKED       ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_TOOLASKED   )
#define CLR_COMPRESSED      ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_COMPRESSED  )
#define CLR_APPICON         ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_APPICON     )
#define CLR_NEWICONASKED    ((*((AZUR*)&GLOB)).Prefs->Flags2 &= ~AZP_NEWICONASKED)
