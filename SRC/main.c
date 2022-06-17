/****************************************************************
 *
 *      File:      main.c
 *      Project:   AZur
 *
 *      Created:   07-01-93     Jean-Michel Forgeas
 *
 *      Note:      Ce code est linké avec rstartup.o qui est un
 *                 startup fourni par Commodore et permet de
 *                 faire un executable réentrant (pure, bit p)
 *                 que l'on peut rendre résident.
 *                 ATTENTION: ne pas utiliser l'option SMALLDATA
 *                 Comme stdout n'existe pas, il faut utiliser
 *                 fprintf(Output(),"") au lieu de printf("").
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <dos/dosasl.h>
#include <graphics/gfxbase.h>
#include <devices/console.h>
#include <workbench/workbench.h>
#include <clib/wb_protos.h>
#include <pragmas/wb_pragmas.h>

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objscreen.h"
#include "uiki:objwindow.h"
#include "uiki:objproplift.h"
#include "uiki:objwinhelp.h"
#include "uiki:objarexxsim.h"
#include "uiki:objtimer.h"
#include "uiki:objwinfilesel.h"
#include "uiki:objinputick.h"

#include "lci:arg3.h"
#include "lci:arg3_protos.h"
#include "lci:aps.h"
#include "lci:aps_protos.h"

#include "lci:objmenu.h"
#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:objfile.h"
#include "lci:azur_protos.h"

#define UIKLIB_MINVER   1
#define UIKLIB_MINREV   58


/****** Imported ************************************************/

extern void Mast_ReceiveARexxCmd();
extern void Mast_CmdComeBack();
extern void Proc_ReceiveARexxCmd();
extern void Proc_CmdComeBack();
extern void FromInpuTick();
extern void AutosaveFunc();

extern UBYTE * __far LangEnglish[];
extern UBYTE * __far LangOther[];
extern struct UIKObjRegGerm __far ObjFileRegGerm;
extern struct UIKObjRegGerm __far ObjMenuRegGerm;

extern void (* __far AZ_FuncTable[])();

extern UBYTE __far CopyrigthCode[];
extern ULONG __far CopyrigthCodeLen;
extern UBYTE __far CopyrigthCode2[];
extern ULONG __far CopyrigthCode2Len;
extern UBYTE __far VersionCode[];
extern ULONG __far VersionCodeLen;

extern struct UIKBitMap * __far ButBitMapTable[];

extern struct DiskObject __far def_text;

#include "src:datatemplate2.c"


/****** Exported ***********************************************/

void ProcessMain();

struct Glob { ULONG toto; } __near GLOB;  // data relative base address pour le compilo

#ifdef AZUR_DEMO
UBYTE __far VersionTag[] = "\0$VER: AZur_Demo 1.03 (6.9.95)";
#else
UBYTE __far VersionTag[] = "\0$VER: AZur 1.03 (6.9.95)";
#endif
UBYTE * __far VersionText = &VersionTag[7];


/****** Statics ************************************************/

static void CommonMain();

static UBYTE __far ArgsTemplate[] =
    "NAME/M,MPN=MASTPORTNAME/K,PN=PORTNAME/K,"
    "FN=FONTNAME/K,FH=FONTHEIGHT/K/N,VFN=VIEWFONTNAME/K,VFH=VIEWFONTHEIGHT/K/N,"
    "F=FROM/K,CMD=FC=FROMCMD/K,PARM=PARMFILE/K,PREFS=PREFSID/K,PAT=PATTERN/K,TL=TABLEN/K/N,"
    "SN=PUBSCREEN=SCREENNAME/K,SA=SCREENADDR/K/N,SF=SCREENFRONT/S,SZ=SCREENAZUR/S,FS=FULLSCREEN/S,"
    "DB=DELBLOCK/S,BKP=BACKUP/S,LM=LINEMODE/S,"
    "MINITEL/S,REPLACE/S,IT=INDTYPE/N,NW=NOWINDOW/S,RO=READONLY/S,"
    "H=HELP/K/S,PRI=PRIORITY/K/N,RES=RESIDENT/S,"
    "IOB=IOBUFFER/K/N,OR=OPENREQ/S,NOS=NOAUTOSIZE/S,ML=MAXLINES/K/N,NP=NOPROC/S,"
    "Q=QUIET/S,SAVE/S,REQWIN/S,SCD=SCDEPTH/K/N,SCM=SCMODE/K/N,EM=ELEMASK/K/N,"
    "PU=PUDDLE/K/N,DELCLIP=DELUNIT/K/N,TABTAB/S,NPT=NULLPOINTER/S,"
    "DEBUG/S,LEFT/K/N,TOP/K/N,WIDTH/K/N,HEIGHT/K/N,ASYNC/S"
    ;

#define ARG_NAME           0    /* fichiers à ouvrir */
#define ARG_MASTPORTNAME   1    /* nom du port ARexx du master process */
#define ARG_PORTNAME       2    /* nom du port ARexx des process d'édition */
#define ARG_FONTNAME       3    /* fonte de l'interface */
#define ARG_FONTHEIGHT     4    /* hauteur de la fonte */
#define ARG_VFONTNAME      5    /* fonte d'édition */
#define ARG_VFONTHEIGHT    6    /* hauteur de la fonte */
#define ARG_FROM           7    /* script ARexx à lancer sur chaque fichier ouvert */
#define ARG_FROMCMD        8    /* objet de commande à lancer sur chaque fichier ouvert */
#define ARG_PARMFILE       9    /* fichier de paramètres pour remplacer le défaut */
#define ARG_PREFSID       10    /* fichier de préférences pour remplacer le défaut */
#define ARG_PATTERN       11    /* pattern du file selector */
#define ARG_TABLEN        12    /* longueur de tabulation */
#define ARG_SCREENNAME    13    /* fenêtre ouvre sur un écran public nommé  */
#define ARG_SCREENADDR    14    /* fenêtre ouvre sur un écran d'adresse donnée  */
#define ARG_SCREENFRONT   15    /* fenêtre ouvre sur l'écran de devant */
#define ARG_SCREENAZUR    16    /* fenêtre ouvre sur un écran propre à AZur */
#define ARG_FULLSCREEN    17    /* fenêtre s'ouvre en plein écran */
#define ARG_CHARDELBLOCK  18    /* la frappe d'un caractère supprime les blocs sourants */
#define ARG_BACKUP        19    /* backup l'ancien fichier avant de l'écraser */
#define ARG_LINEMODE      20    /* mode bloc par ligne */
#define ARG_MINITEL       21    /* ... indique que le port série est relié à un minitel */
#define ARG_REPLACE       22    /* écriture par remplacement au lieu d'insertion */
#define ARG_INDTYPE       23    /* type d'indentation pour insertion de texte */
#define ARG_NOWINDOW      24    /* pas de fenêtre pour les fichiers ouverts  */
#define ARG_READONLY      25    /* pas de modification ni sauvegarde possibles */
#define ARG_HELP          26    /* fenêtre d'aide */
#define ARG_PRIORITY      27    /* priorité des process d'édition, le master se met la même + 1 */
#define ARG_RESIDENT      28    /* la process master doit rester en mémoire  */
#define ARG_IOBUFFER      29    /* (KO) taille des buffers de lecture/écriture sur disque */
#define ARG_OPENREQ       30    /* fileselector dès l'ouverture d'une fenêtre sans fichier */
#define ARG_NOAUTOSIZE    31    /* empêche l'adaptation des bords de la fenêtre */
#define ARG_MAXLINES      32    /* nombre maximum de lignes dans la fenêtre */
#define ARG_NOPROC        33    /* le master ne doit pas lancer de process d'édition */
#define ARG_QUIET         34    /* empêche l'ouverture des requêtes d'erreur */
#define ARG_SAVE          35    /* sauvegarder le fichier juste avant de quitter */
#define ARG_REQWIN        36    /* Les requesters sont des fenêtres */
#define ARG_DEPTH         37    /* Profondeur pour l'écran ARG_SCREENAZUR */
#define ARG_MODEID        38    /* Mode pour l'écran ARG_SCREENAZUR */
#define ARG_ELEMASK       39    /* Masque des éléments du display */
#define ARG_PUDDLE        40    /* Taille des blocs de mémoire en kilo octets */
#define ARG_DELCLIP       41    /* Clip unit utilisée pour sauvegarder blocs et lignes effacées */
#define ARG_TABTAB        42    /* Laisse les tabs tels quels */
#define ARG_NULLPOINTER   43    /* fait disparaitre le pointeur quand touche appuyée */
#define ARG_DEBUG         44    /* Affiche des infos dans une console */
#define ARG_LEFT          45    /*  */
#define ARG_TOP           46    /*  */
#define ARG_WIDTH         47    /*  */
#define ARG_HEIGHT        48    /*  */
#define ARG_ASYNC         49    /* Juste pour que azurquick lance pas AZur avec le nom de fichier ASYNC */
#define ARG_TOTAL         50

static struct AZurPrefs __far DefaultPrefs = {
    AZP_SAVEICON | AZP_REXXNAMESEP,  //  Flags1
    AZP_LOADICON | AZP_STRIPSAVE | AZP_STRIPLOAD | AZP_STRIPCHANGE | AZP_TAB2SPACE | AZP_BLINKON | AZP_APPICON, // ULONG Flags2
    0,  //  Flags3
    SEF_CASE|SEF_ACCENT|SEF_TOBLOCK|SEF_PATTERN, // SearchFlags
    BUMF_BUTUP|BUMF_UPDOWN1|BUMF_VERTPROP|BUMF_UPDOWN2|BUMF_BUTDOWN |
        BUMF_BUTLEFT|BUMF_LEFTRIGHT1|BUMF_HORIZPROP|BUMF_LEFTRIGHT2|BUMF_BUTRIGHT |
#ifdef AZUR_DEMO
        BUMF_CMDLINE |
#endif
        BUMF_STATUSBAR,     // ULONG EleMask
    "topaz.font", // FontName[32]
    "topaz.font", // ViewFontName[32]
    8,      // FontHeight
    8,      // ViewFontHeight
    2, 1,   // BlinkOnDelay, BlinkOffDelay

    0,0,0,0,// UBYTE TextPen, BackPen, BlockTPen, BlockBPen;
    0,0,0,0,// UBYTE CursTextPen, CursBackPen, CursBlockTPen, CursBlockBPen;

    { // ViewInfo
    { 35,15,500,150 }, // ViewBox
    0,0, // mins
    11,11, // VBorderWidth, HBorderHeight
    12, // VButHeight
    24, // VArrHeight
    14, // HButWidth
    26, // HArrWidth
    },

    "#?", // PatOpen[64]
#ifdef AZUR_DEMO
    "AZur:Parms/AZur.par", // ParmFile[64]
#else
    "Parms/AZur.par", // ParmFile[64]
#endif
    20,     // IOBufLength
    INDENT_STAY,    // IndentType
    4,  // tab length
    5,  // PuddleSize
    1,  // DelClip
    "AZurMast", // MastPortName[16]
    "AZur", // PortName[16]
    "", // ARexxConsoleDesc[96]
    "", // DOSConsoleDesc[96]
    0,0,0,0,
    0,
    2,  // ScreenDepth
    BLKMODE_CHAR,
    2, 1,   // Block XLim et YLim
    20, // HistCmdLineMax
    OSCAN_TEXT,  // ScreenOverscan
    '_', // CharSpace;
    '¶', // CharEOL;
    0x0, // CharEOF;
    '¬', // CharTabS;
    '·', // CharTabE;
    9,   // LeftMargin;
    60,  // RightMargin;
    "AZur:Utilities/AZurQuick", // IconTool[64];
    "T:",        // BackupDirName[64];
    "*.bkp",     // BackupName[16];
    20, // ULONG UndoMaxNum;
    1,  // ULONG UndoMaxSize;
    '.',// UBYTE CharRexxDot;
    0,  // UBYTE Priority;
    5*60,  // UWORD AutsIntSec;
    {0}, {0}, {0}, {0}, {0},  // struct IBox Pos...
    "", // UBYTE AutsDirName[64];
    "*.auts", // UBYTE AutsName[16];
    100, // UWORD AutsIntMod;
    0, // UBYTE AutsRot;
    0, // UBYTE pad;
    { // struct FoldDrawInfo [5]
     { FSF_BOLD, 1, 2, 0 },
    },
    { 0 },
    { 0 },
    "", // AppIconName
    };

static struct TagItem __far flags_boolmap1[] =  // Flags1
    {
    { AZTAG_LaunchF_FileReq,   AZP_OPENREQ   },
    { AZTAG_LaunchF_NoWindow,  AZP_NOWINDOW  },
    { AZTAG_LaunchF_ReadOnly,  AZP_READONLY  },
    { AZTAG_LaunchF_QuitAfter, AZP_QUITAFTER },
    { AZTAG_LaunchF_CreateFile,AZP_CREATEFILE},
    { TAG_DONE }
    };
static struct TagItem __far flags_boolmap2[] =  // Flags2
    {
    { AZTAG_LaunchF_IgnoreIcon,AZP_IGNOREICON},
    { TAG_DONE }
    };
static struct TagItem __far flags_boolmap3[] =  // Flags3
    {
    { TAG_DONE }
    };


/****************************************************************
 *
 *      Common code
 *
 ****************************************************************/

static void LibAlert( UBYTE *name, ULONG ver, ULONG rev )
{
    eng_ShowRequest( "WANTED: %ls %ld.%ld minimum", 0, name, (UBYTE*)ver, (UBYTE*)rev );
}

static void StartErr( ULONG num )
{
    eng_ShowRequest( "Start err %ld", 0, (UBYTE*)num, 0, 0 );
}

static AZUR *Init( ULONG size )
{
  AZUR *vars = (AZUR *) AllocSpMem( size ); // dans arg3.lib

    __builtin_putreg( 12, vars ); // A4 pointe sur la structure des variables
    if (! vars) goto END_ERROR;

    if (! (IntuitionBase = (APTR) OpenLibrary( "intuition.library", 0 ))) goto END_ERROR; // pas de LibAlert()
    if (! (LayersBase = OpenLibrary( "layers.library", 0 ))) { LibAlert( "layers.library", 0, 0 ); goto END_ERROR; }
    if (! (IconBase = (APTR) OpenLibrary( "icon.library", 0 ))) { LibAlert( "icon.library", 0, 0 ); goto END_ERROR; }
    if (! (DOSBase = OpenLibrary( DOSNAME, 0 ))) { LibAlert( DOSNAME, 0, 0 ); goto END_ERROR; }
    if (! (GfxBase = (APTR) OpenLibrary( "graphics.library", 0 ))) { LibAlert( "graphics.library", 0, 0 ); goto END_ERROR; }
    if (! (UIKBase = (APTR) OpenLibrary( UIKNAME, UIKLIB_MINVER )) || ((UIKBase->Library.lib_Revision < UIKLIB_MINREV) && (UIKBase->Library.lib_Version == UIKLIB_MINVER)))
        { LibAlert( UIKNAME, UIKLIB_MINVER, UIKLIB_MINREV ); goto END_ERROR; }
    ExecBase = (APTR) *((ULONG*)4);
    WorkbenchBase = (APTR) OpenLibrary( "workbench.library", 37 ); // 2.0 only

    if (! (vars->Pool = AsmCreatePool( MEMF_ANY|MEMF_CLEAR, 4096, 4096, *(APTR*)4 ))) goto END_ERROR;

    if (! (vars->RexxResult = BufAllocP( vars->Pool, 0, 100 ))) goto END_ERROR;
    if (! (vars->LastCmd = BufAllocP( vars->Pool, 0, 20 ))) goto END_ERROR;
    if (! (vars->UBufTmp = BufAllocP( vars->Pool, 0, 100 ))) goto END_ERROR;
    if (! (vars->FileSelResult = BufAllocP( vars->Pool, 256, 256 ))) goto END_ERROR;

    return( vars );
  END_ERROR:
    vars->MaxError = 20;
    return(0);
}

/****************************************************************
 *
 *      Azur Master
 *
 ****************************************************************/

void func_StartAppIcon() // appelée aussi par le timer
{
    if ((AGP.AZMast->Prefs->Flags2 & AZP_APPICON) && WorkbenchBase && AGP.AZMast->AppIconPort)
        {
        if (!AGP.AZMast->AppIcon && FindTask( "Workbench" ))
            {
            struct DiskObject defdobj = def_text;
            struct DiskObject *dobj;

            AGP.AZMast->DiskObj = GetDiskObject( "AZur:Icons/def_appicon" );

            if (! (dobj = AGP.AZMast->DiskObj)) dobj = &defdobj;
            dobj->do_Type = 0;
            AGP.AZMast->AppIcon = AddAppIconA( 0, 0, AGP.Prefs->AppIconName, AGP.AZMast->AppIconPort, 0, dobj, NULL );
            }
#ifdef AZUR_DEMO
        /*------ Démo uniquement ------*/
        UIK_Call( AGP.AZMast->Timer, UIKFUNC_Timer_Start, 0, 0 );
#else
        /*------ Normal uniquement ------*/
        // si Workbench n'est pas lancé :
        if (! AGP.AZMast->AppIcon)
            {
            if (AGP.AZMast->DiskObj) { FreeDiskObject( AGP.AZMast->DiskObj ); AGP.AZMast->DiskObj = 0; }
            if (AGP.AZMast->Timer->TotalTime.tv_secs < 30)
                UIK_Call( AGP.AZMast->Timer, UIKFUNC_Timer_Start, 0, 0 );
            else{
                UIK_Call( AGP.AZMast->Timer, UIKFUNC_Timer_Stop, 0, 0 );
                CLR_APPICON;
                }
            }
        else UIK_Call( AGP.AZMast->Timer, UIKFUNC_Timer_Stop, 0, 0 );
#endif
        }

#ifdef AZUR_DEMO
    /*------ Démo uniquement ------*/
    {
    ULONG testval = 10, secs = AGP.AZMast->Timer->TotalTime.tv_secs * 3;
    if (secs / testval > ((AZURDEMO_MAXTIME*3)/10))  /* pour valeur pas reconnaissable */
        Signal( AGP.AZMast->Process, 0x00030000 | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D ); /* pour valeur pas reconnaissable */
    }
#endif
}

void func_StopAppIcon()
{
  APTR amsg;

    if (AGP.AZMast->AppIcon)
        {
        while (amsg = (struct AppMessage *) GetMsg( AGP.AZMast->AppIconPort )) ReplyMsg( amsg );
        RemoveAppIcon( AGP.AZMast->AppIcon );
        AGP.AZMast->AppIcon = 0;
        if (AGP.AZMast->DiskObj) { FreeDiskObject( AGP.AZMast->DiskObj ); AGP.AZMast->DiskObj = 0; }
#ifndef AZUR_DEMO
        UIK_Call( AGP.AZMast->Timer, UIKFUNC_Timer_Stop, 0, 0 );
#endif
        }
}

static void AppIcon_IconReceived(  )
{
  struct AppMessage *amsg;
  struct WBArg *argptr;
  ULONG i;

    while (amsg = (struct AppMessage *) GetMsg( AGM.AppIconPort ))
        {
        if (amsg->am_Type == AMTYPE_APPICON)
            {
            //------ Si plusieurs, iconification dans des nouvelles fenêtres
            argptr = amsg->am_ArgList;

            if (amsg->am_NumArgs == 0)  // double-clic
                {
                if (! func_LaunchProcess( AZTAG_Launch_DirLock,    AGM.CurDirLock,
                                          AZTAG_LaunchF_QuitAfter, TRUE,
                                          TAG_END )) break;
                }
            else{
                for (i=0; i < amsg->am_NumArgs; i++) // icons
                    {
                    CurrentDir( argptr->wa_Lock );
                    if (argptr->wa_Name && argptr->wa_Name[0] && lo_FileExists( argptr->wa_Name ))
                        {
                        if (! func_LaunchProcess( AZTAG_LaunchF_Iconify,   (i==0)?FALSE:TRUE,
                                                  AZTAG_Launch_FileName,   argptr->wa_Name,
                                                  AZTAG_Launch_DirLock,    argptr->wa_Lock,
                                                  AZTAG_LaunchF_QuitAfter, TRUE,
                                                  TAG_END )) break;
                        }
                    argptr++;
                    }
                }
            CurrentDir( AGM.CurDirLock );
            }
        ReplyMsg( amsg );
        }
    AGM.AppIconVect = AppIcon_IconReceived;
}

static ULONG My_IsListEmpty( struct List *list )
{
  ULONG empty;

    Forbid();
    empty = IsListEmpty( list );
    Permit();
    return( empty );
}

static void signal_all( ULONG sigmask )
{
  struct AZurNode *node;

    ObtainSemaphore( &AGM.ProcSem );
    Forbid();
    for (node=(struct AZurNode *)AGM.ProcList.lh_Head; node->ln_Succ; node = (struct AZurNode *)node->ln_Succ)
        Signal( node->ln_Task, sigmask );
    Permit();
    ReleaseSemaphore( &AGM.ProcSem );
    AGM.CtrlDVect = signal_all;
}

static void ReplyQuickMsg( struct AZQuickMsg *amsg )
{
    if (amsg->Flags & AZQMF_ASYNC)
        {
        ARG_FreeCmdLine( amsg->CmdLine );
        UnLock( amsg->CurDirLock );
        DeleteExtIO( amsg );
        }
    else ReplyMsg( amsg );
}

static void Cleanup()
{
  APTR amsg, vars = (APTR) __builtin_getreg( 12 ); // A4 pointe sur la structure des variables

    if (vars)
        {
        if (DOSBase)
            {
            if (GfxBase)
                {
                if (IconBase)
                    {
                    if (LayersBase)
                        {
                        if (IntuitionBase)
                            {
                            if (UIKBase)
                                {
                                if (AGM.UIK)
                                    {
                                    func_StopAppIcon();
                                    if (AGM.AppIconPort) DeletePort( AGM.AppIconPort );

                                    if (! (My_IsListEmpty( ((struct List *)&AGM.RexxNameList) )))
                                        rxn_Cleanup();

                                    if (AGM.LaunchPort.mp_Node.ln_Name)
                                        {
                                        RemPort( &AGM.LaunchPort ); // delink, ne libère pas
                                        while (amsg = GetMsg( &AGM.LaunchPort )) ReplyQuickMsg( amsg );
                                        }

                                    signal_all( SIGBREAKF_CTRL_C );
                                    SetTaskPri( AGM.Process, 0 );
                                    while (! (My_IsListEmpty( ((struct List *)&AGM.ProcList) )))
                                        {
                                        // UIK_ProcessPendingEvents( AGM.UIK, 0 ); // pour commandes IPUIK_
                                        Delay(1);
                                        }
                                    SetTaskPri( AGM.Process, AGM.OldPri );

                                    ParmCleanup();

                                    eng_UnregisterAllObjects();
                                    // ARG_FreeParse( AGM.Parse ); pas besoin car pooled
                                    ARG_FreeCmdLine( AGM.CmdLine );
                                    PoolFree( AGM.ArgRes );
                                    PoolFree( AGM.PArg );

                                    UIK_Remove( AGM.UIK );
                                    func_FreeVarList();

                                    if (AGM.NilFh) Close( AGM.NilFh );
                                    }

                                if (AGM.OKSigBit != -1) FreeSignal( AGM.OKSigBit );
                                if (AGM.ErrorSigBit != -1) FreeSignal( AGM.ErrorSigBit );
                                /* pooled
                                BufFree( AGM.FileSelResult );
                                BufFree( AGM.UBufTmp );
                                BufFree( AGM.LastCmd );
                                BufFree( AGM.RexxResult );
                                */

                                if (AGM.Pool) AsmDeletePool( AGM.Pool, *(APTR*)4 );
                                if (WorkbenchBase) CloseLibrary( WorkbenchBase );

                                CloseLibrary( UIKBase );
                                }
                            CloseLibrary( IntuitionBase );
                            }
                        CloseLibrary( LayersBase );
                        }
                    CloseLibrary( IconBase );
                    }
                CloseLibrary( GfxBase );
                }
            CloseLibrary( DOSBase );
            }
        FreeSpMem( vars );
        }
    exit(0);
}

static ULONG add_filenode( UBYTE *filename )
{
  struct Node *node;
  ULONG len = StrLen( filename );

    if (node = PoolAlloc( sizeof(struct Node) + len + 1 ))
        {
        node->ln_Name = (APTR)((ULONG)node + sizeof(struct Node));
        StrCpy( node->ln_Name, filename );
        AddTail( &AGM.FileList, node );
        ++AGM.FileCount;
        }
    return( (ULONG)node );
}

static UBYTE *char_found( UBYTE *name, UBYTE ch )
{
    for (; *name; name++) if (*name == ch) return( name );
    return(0);
}

static void verif_fontname( UBYTE *name )
{
    if (StrLen( name ) < 31)
        if (! char_found( name, '.' ))
            StrCat( name, ".font" );
}

//struct Library * __far KeymapBase;

static void launch_quick( void **var )
{
  APTR ArgRes[ARG2_TOTAL];
  struct ParseArgs PArg;
  APTR *Parse=0;
  struct Node *node;
  ULONG kill=0, proc, num, filecount=0;
  struct AZQuickMsg *amsg;
  struct Task *task=0;

  ULONG FileReq, NoWindow, ReadOnly, Priority;
  WORD left, top, width, height;
  UBYTE *FromCmd, *FromScript, *PortName, *Pattern, *PrefsName;

    while (amsg = (APTR) GetMsg( &AGM.LaunchPort ))
        {
        FileReq  = TST_OPENREQ;
        NoWindow = TST_NOWINDOW;
        ReadOnly = TST_READONLY;
        Priority = AGM.Process->pr_Task.tc_Node.ln_Pri - 1;

        FromCmd    = AGM.FromCmdName;
        FromScript = AGM.FromName;
        PortName   = AGM.Prefs->PortName;
        Pattern    = AGM.Prefs->PatOpen;
        PrefsName  = 0;

        left   = -1;
        top    = -1;
        width  = -1;
        height = -1;

        if (amsg->CmdLine)
            {
            __builtin_memset( &ArgRes[0], 0, sizeof(ArgRes) );
            __builtin_memset( &PArg, 0, sizeof(struct ParseArgs) );

            PArg.CmdLine = amsg->CmdLine;
            PArg.CmdLength = StrLen( amsg->CmdLine );
            PArg.SepSpace = 0x9;
            PArg.PoolHeader = AGM.Pool;
            PArg.Flags = PAF_PROMPT | PAF_POOLED;
            if (Parse = ARG_Parse( ArgsTemplate2, ArgRes, &PArg, 0, (ULONG)DOSBase ))
                {
                APTR a, *tab;
                ULONG *n;

                if (ArgRes[ARG2_OPENREQ])        FileReq  = TRUE;
                if (ArgRes[ARG2_NOWINDOW])       NoWindow = TRUE;
                if (ArgRes[ARG2_READONLY])       ReadOnly = TRUE;
                if (n = ArgRes[ARG2_PRIORITY])   Priority = *n;

                if (a = ArgRes[ARG2_PORTNAME])   PortName   = a;
                if (a = ArgRes[ARG2_FROM])       FromScript = a;
                if (a = ArgRes[ARG2_FROMCMD])    FromCmd    = a;
                if (a = ArgRes[ARG2_PATTERN])    Pattern    = a;
                if (a = ArgRes[ARG2_PREFS])      PrefsName  = a;

                if (n = ArgRes[ARG2_LEFT  ])     left   = (WORD)*n;
                if (n = ArgRes[ARG2_TOP   ])     top    = (WORD)*n;
                if (n = ArgRes[ARG2_WIDTH ])     width  = (WORD)*n;
                if (n = ArgRes[ARG2_HEIGHT])     height = (WORD)*n;

                if (ArgRes[ARG2_RESIDENT])       SET_RESIDENT;

                if (ArgRes[ARG2_KILL])           kill = 1;

                if ((tab = ArgRes[ARG2_NAME]) && kill == 0)
                    {
                    ULONG i;
                    struct AnchorPath *ma;

                    filecount = AGM.FileCount;
                    if (ma = PoolAlloc( sizeof(struct AnchorPath) + 256 ))
                        {
                        ma->ap_BreakBits = SIGBREAKF_CTRL_C;
                        ma->ap_Strlen = 256;

                        for (i=0; tab[i]; i++)
                            {
                            if (UIK_IsSystemVersion2())
                                {
                                if (MatchFirst( tab[i], ma ) == 0)
                                    {
                                    do add_filenode( ma->ap_Buf ); while (MatchNext( ma ) == 0);
                                    MatchEnd( ma );
                                    if (ma->ap_FoundBreak & SIGBREAKF_CTRL_C) break;
                                    }
                                else add_filenode( tab[i] );
                                }
                            else add_filenode( tab[i] );
                            }
                        PoolFree( ma );
                        }
                    filecount = AGM.FileCount - filecount;
                    }
                }
            }

        //--- Files
        if (kill)
            {
            Signal( AGM.Process, SIGBREAKF_CTRL_C );
            }
        else{
            if (IsListEmpty( &AGM.FileList ))
                {
                proc = func_LaunchProcess(
                                    AZTAG_Launch_FromScript,    FromScript,
                                    AZTAG_Launch_FromCmd,       FromCmd,
                                    AZTAG_Launch_PortName,      PortName,
                                    AZTAG_Launch_Pattern,       Pattern,
                                    AZTAG_LaunchF_FileReq,      FileReq,
                                    AZTAG_LaunchF_NoWindow,     NoWindow,
                                    AZTAG_LaunchF_ReadOnly,     ReadOnly,
                                    AZTAG_Launch_Priority,      Priority,
                                    AZTAG_Launch_QuickMsg,      amsg,
                                    AZTAG_Launch_DirLock,       amsg->CurDirLock,
                                    AZTAG_LaunchF_QuitAfter,    TRUE,
                                    AZTAG_Launch_LeftTop,       W2L(left,top),
                                    AZTAG_Launch_WidthHeight,   W2L(width,height),
                                    AZTAG_Launch_PrefsFileName, PrefsName,
                                    TAG_END );
                if (proc) task = (struct Task *) (proc - sizeof(struct Task));
                }
            else{
                for (num=1,node=AGM.FileList.lh_Head; node->ln_Succ; node = node->ln_Succ, num++)
                    {
                    proc = func_LaunchProcess(
                                        AZTAG_LaunchF_Iconify,      (num == filecount) ? FALSE : TRUE,
                                        AZTAG_Launch_FromScript,    FromScript,
                                        AZTAG_Launch_FromCmd,       FromCmd,
                                        AZTAG_Launch_PortName,      PortName,
                                        AZTAG_Launch_Pattern,       Pattern,
                                        AZTAG_LaunchF_FileReq,      FileReq,
                                        AZTAG_LaunchF_NoWindow,     NoWindow,
                                        AZTAG_LaunchF_ReadOnly,     ReadOnly,
                                        AZTAG_Launch_Priority,      Priority,
                                        AZTAG_Launch_QuickMsg,      (num == 1) ? amsg : 0, // la première répondra
                                        AZTAG_Launch_FileName,      node->ln_Name,
                                        AZTAG_Launch_DirLock,       amsg->CurDirLock,
                                        AZTAG_LaunchF_QuitAfter,    TRUE,
                                        AZTAG_Launch_LeftTop,       W2L(left,top),
                                        AZTAG_Launch_WidthHeight,   W2L(width,height),
                                        AZTAG_Launch_PrefsFileName, PrefsName,
                                        TAG_END );
                    if (proc) { if (num == 1) task = (struct Task *) (proc - sizeof(struct Task)); }
                    else break;
                    if (SIGBREAKF_CTRL_C & SetSignal( 0, 0 )) break; // pour arrêt de lancement en chaine
                    }
                while (node = RemHead( &AGM.FileList )) PoolFree( node );
                }
            }

        if (task) amsg->Edit = task;
        else ReplyQuickMsg( amsg );

        ARG_FreeParse( Parse );     // libère les arguments
        }
    *var = launch_quick;        // réarme launch_func
}

static ULONG add_main_interface()
{
  struct AZurPrefs *pr = AGP.AZMast->Prefs;
  struct Screen *sc;
  ULONG i;

    AGM.IntuiScreenTags[0].ti_Tag  = SA_PubName;
    AGM.IntuiScreenTags[0].ti_Data = (ULONG)AGM.PubScreenName;
    AGM.IntuiScreenTags[1].ti_Tag  = SA_Interleaved;
    AGM.IntuiScreenTags[1].ti_Data = TRUE;
    AGM.IntuiScreenTags[2].ti_Tag  = TAG_END;
    StrCpy( AGM.PubScreenName, "AZurScreen" );
    if (UIK_IsSystemVersion2())
        {
        for (i=1; i < 100; i++)
            {
            if ((sc = LockPubScreen( AGM.PubScreenName )) == NULL) break;
            UnlockPubScreen( NULL, sc );
            sprintf( AGM.PubScreenName, "%ls%ld", "AZurScreen", i );
            }
        }

    AGM.WinObj = 0; AGM.MenuObj=0;

 // pour combler le manque d'éditeur de préférences pour ça
 CopyMem( &UIKBase->UIKPrefs->Colors, &AGP.Prefs->ColRegs, sizeof(struct UIKColors) );

     if (AGM.ScreenObj = (APTR) UIK_AddObjectTagsB( UIKBase, "UIKObj_Screen", AGM.UIK,
        UIKTAG_OBJ_LeftTop,         *((ULONG*)&pr->ScreenLeft),
        UIKTAG_OBJ_WidthHeight,     *((ULONG*)&pr->ScreenWidth),
        UIKTAG_OBJ_FontName,        pr->FontName,
        UIKTAG_OBJ_FontHeight,      pr->FontHeight,      /* pour option SZ sous 1.3 */
        UIKTAG_OBJ_Title,           UIK_IsSystemVersion2() ? TEXT_AZurScreenTitle : "AZur Screen",
        UIKTAG_OBJ_TitleFl_Addr,    UIK_IsSystemVersion2() ? FALSE : TRUE,
        UIKTAG_ObjScreen_Depth,     pr->ScreenDepth,
        UIKTAG_ObjScreen_ViewModes, pr->ScreenModeId,
        UIKTAG_ObjScreen_ColorTable,pr->ColorTable,
        UIKTAG_OBJScreen_RedirectColors,&pr->ColRegs,
        UIKTAG_OBJScreen_IntuiTags, AGM.IntuiScreenTags,
        UIKTAG_OBJ_MinVerRev,       W2L(1,8), /* Corriger aussi LibAlert() ci-dessous */
        // UIKTAG_OBJ_BoxFl_StartSpecify, TRUE,
        TAG_END ))
        {
        if (AGP.AZMast->MainMenuHandle && AGP.AZMast->MainMenuHandle->MenuEntry && intAZ_LineBufLen( (UBYTE*)AGP.AZMast->MainMenuHandle->MenuEntry ) > sizeof(struct AZMenuEntry))
            {
            if (AGM.WO = AGM.WinObj = (APTR) UIK_AddObjectTagsB( UIKBase, "UIKObj_Window", AGM.ScreenObj,
                UIKTAG_OBJ_AltTitle,        UIK_IsSystemVersion2() ? TEXT_AZurScreenTitle : "AZur Screen",
                UIKTAG_OBJ_TitleFl_Addr,    UIK_IsSystemVersion2() ? FALSE : TRUE,
                UIKTAG_OBJWindowFl_NearFullScreen,  TRUE,
                UIKTAG_OBJWindowFl_With_Depth,      FALSE,
                UIKTAG_OBJWindowFl_With_Size,       FALSE,
                UIKTAG_OBJWindowFl_With_Close,      FALSE,
                UIKTAG_OBJWindowFl_With_Zoom,       FALSE,
                UIKTAG_OBJWindowFl_With_Icon,       FALSE,
                UIKTAG_OBJWindowFl_With_Drag,       FALSE,
                UIKTAG_OBJWindowFl_NoBlackPerimeter,TRUE,
                UIKTAG_OBJWindowFl_Backdrop,        TRUE,
                UIKTAG_OBJWindowFl_NoFill,          TRUE,
                UIKTAG_OBJWindowFl_SimpleRefresh,   TRUE,
                TAG_END ))
                {
                AGM.MenuObj = (APTR) UIK_AddPrivateObjectTagsB( UIKBase, &ObjMenuRegGerm, AGM.WinObj,
                    UIKTAG_Menu_Entry,              AGP.AZMast->MainMenuHandle->MenuEntry,
                    TAG_END );
                }
            }
        return(1);
        }
    else LibAlert( "UIKObj_Screen", 1, 8 ); /* UIKTAG_OBJ_MinVerRev,       W2L(1,8) */

    return(0);
}

static void activate_window( void **var )
{
  struct Window *w;

    if (AGP.WO)
        {
        if (AGP.WO->Obj.Status == UIKS_ICONIFIED) UIK_UniconifyWindow( AGP.WO );
        if (w = AGP.WO->Obj.w)
            {
            if (! (w->Flags & WFLG_BACKDROP)) WindowToFront( w );
            ActivateWindow( w );
            ScreenToFront( w->WScreen );
            }
        }
    *var = activate_window;  // ctrlf_f_func
}

static void makeassigns()
{
#ifdef AZUR_DEMO
  ULONG lock, save;

    save = UIK_DisableDOSReq();
    if (! (lock = Lock( "UIK:", ACCESS_READ ))) Execute( "Assign UIK: AZur_Demo:UIK", 0, AGM.NilFh );
    else UnLock( lock );
    if (! (lock = Lock( "AZur:", ACCESS_READ ))) Execute( "Assign AZur: AZur_Demo:", 0, AGM.NilFh );
    else UnLock( lock );
    UIK_EnableDOSReq( 0, save );
#endif
}

void main2( void )
{
  UBYTE *fname, *QuickPortName;
  struct Node *node;
  ULONG i, ok=1, breaksig, num, help=0, mem=0, old, noproc=0, errmsg;
  struct Rectangle rect;
  struct UIKPrefs *up;
  void *launch_func;
  void *ctrlf_f_func;

    if (! (AGM.NilFh = Open( "NIL:", MODE_NEWFILE ))) goto END_ERROR;
    makeassigns();

//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "\n1 mem=%ld (%ld)\n", mem, old-mem );
    AGM.Process = (struct Process *) FindTask(0);
    AGM.JumpOffsets = (void*) AZ_FuncTable;
    AGM.OldPri = AGM.Process->pr_Task.tc_Node.ln_Pri;
    AGM.OKSigBit = AGM.ErrorSigBit = -1;
    AGM.MaxLines = 0x7fffffff; // nombre de lignes illimité (sans signe)
    up = UIKBase->UIKPrefs;

    if ((AGM.OKSigBit = AllocSignal(-1)) == -1 || (AGM.ErrorSigBit = AllocSignal(-1)) == -1)
        { StartErr(1); goto END_ERROR; }

    //--- Inits
    InitSemaphore( &AGM.ProcSem );
    InitSemaphore( &AGM.RGSem );
    InitSemaphore( &AGM.RexxNameSem );
    NewList( &AGM.ProcList );
    NewList( &AGM.FileList );
    NewList( &AGM.RexxNameList );
    if (! (AGM.ObjSema = intAZ_SemaCreate())) goto END_ERROR;  // alloue dans pool
    if (! (AGM.VarSema = intAZ_SemaCreate())) goto END_ERROR;  // alloue dans pool
    if (! (AGM.ARexxFileName = BufAllocP( AGM.Pool, 0, 10 ))) goto END_ERROR;
    if (! (AGM.DOSFileName = BufAllocP( AGM.Pool, 0, 10 ))) goto END_ERROR;
    if (! (AGM.Parms = PoolAlloc( 32 + sizeof(struct AZProcParms) )))
        { StartErr(2); goto END_ERROR; }
    QuickPortName = (UBYTE*)((ULONG)AGM.Parms + sizeof(struct AZProcParms));
    StrCpy( QuickPortName, QUICKPREFIX );

    SetSignal( 0, SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F );

    //--- UIK
    if (! (AGM.UIK = (struct UIKGlobal *) UIK_AddObjectTagsB( UIKBase, "UIK", 0,
        UIKTAG_GEN_LangEnglish,     LangEnglish,
        UIKTAG_GEN_LangOther,       LangOther,
        UIKTAG_GEN_LangDuplicate,   FALSE,
        UIKTAG_GEN_Catalog,         "AZurEditor.catalog",
        TAG_END ))) { StartErr(2); goto END_ERROR; }

    DefaultPrefs.TextPen = UIK_GetColor( AGP.UIK, UIKCOL_BLACK );
    DefaultPrefs.BackPen = UIK_GetColor( AGP.UIK, UIKCOL_WHITE );
    DefaultPrefs.BlockTPen = UIK_GetColor( AGP.UIK, UIKCOL_WHITE );
    DefaultPrefs.BlockBPen = UIK_GetColor( AGP.UIK, UIKCOL_GREY );
    DefaultPrefs.CursTextPen = UIK_GetColor( AGP.UIK, UIKCOL_WHITE );
    DefaultPrefs.CursBackPen = UIK_GetColor( AGP.UIK, UIKCOL_BLACK );
    DefaultPrefs.CursBlockTPen = UIK_GetColor( AGP.UIK, UIKCOL_WHITE );
    DefaultPrefs.CursBlockBPen = UIK_GetColor( AGP.UIK, UIKCOL_LIGHT );
    for (i=0; i < NUMFOLDINFOS; i++)
        {
        DefaultPrefs.FDI[i].OnFlags = FSF_BOLD;
        DefaultPrefs.FDI[i].OnAPen  = UIK_GetColor( AGP.UIK, UIKCOL_BLACK );
        DefaultPrefs.FDI[i].OnBPen  = UIK_GetColor( AGP.UIK, UIKCOL_GREY );
        }

    UIK_InvertBlackWhite( AGM.UIK, ButBitMapTable, 1 );

    if (AGM.AppIconPort = (struct MsgPort *) CreatePort( 0, 0 ))
        {
        AGM.AppIconVect = AppIcon_IconReceived;
        UIK_AddVector( AGM.UIK, (void*)&AGM.AppIconVect, 1<<AGM.AppIconPort->mp_SigBit, 0 );
        }

    //--- Timer
    if (! (AGM.Timer = (struct UIKObjTimer *) UIK_AddObjectTagsB( UIKBase, "UIKObj_Timer", AGM.UIK,
            //UIKTAG_Timer_Unit,  UNIT_MICROHZ,
            UIKTAG_Timer_Unit,      UNIT_VBLANK,
            UIKTAG_Timer_TimeFunc,  func_StartAppIcon,
            UIKTAG_Timer_Seconds,   1,
            TAG_END ))) { StartErr(3); goto END_ERROR; }

    /*------------Premier Parse------------*/
    if (! (AGM.DefPrefsName = PoolAlloc( 32 ))) goto END_ERROR;
    StrCpy( AGM.DefPrefsName, "AZur.prefs" );

    if (!(AGM.PArg = PoolAlloc( sizeof(struct ParseArgs) )) || !(AGM.ArgRes = PoolAlloc( 4*ARG_TOTAL )))
        { StartErr(5); goto END_ERROR; }
    AGM.PArg->Error = PAERR_NOTENOUGH_MEMORY;
    if (AGM.CmdLine = ARG_BuildCmdLine( AGM.argc, AGM.argv, &AGM.CurDirLock, ArgsTemplate ))
        {
        AGM.PArg->CmdLine = AGM.CmdLine;
        AGM.PArg->CmdLength = StrLen( AGM.CmdLine );
        AGM.PArg->SepSpace = 0x9;
        AGM.PArg->PoolHeader = AGM.Pool;
        AGM.PArg->Flags = PAF_PROMPT | PAF_POOLED;
        if (AGM.Parse = ARG_Parse( ArgsTemplate, AGM.ArgRes, AGM.PArg, AGM.argc, (ULONG)DOSBase ))
            {
            APTR p, apsh, a, *tab;
            ULONG *n;

            if (a = AGM.ArgRes[ARG_PREFSID]) StcCpy( AGM.DefPrefsName, UIK_BaseName(a), 32 );

            /*------------Preferences------------*/
            if (! (AGM.EnvFileName = PoolAlloc( 64 ))) goto END_ERROR;
            if (! (AGM.ArcFileName = PoolAlloc( 64 ))) goto END_ERROR;
            if (! (AGM.Prefs = PoolAlloc( 2 * sizeof(struct AZurPrefs) ))) goto END_ERROR;
            AGM.TmpPrefs = (struct AZurPrefs *)((ULONG)AGM.Prefs + sizeof(struct AZurPrefs));

            StcCpy( DefaultPrefs.ARexxConsoleDesc, (((struct Library *)(*(ULONG*)4))->lib_Version >= 36) ?
                    "CON:50/1000/500/90/Console AZur - ARexx/CLOSE/AUTO" :
                    "NEWCON:70/90/500/90/Console AZur - ARexx", 96 );
            StcCpy( DefaultPrefs.DOSConsoleDesc, (((struct Library *)(*(ULONG*)4))->lib_Version >= 36) ?
                    "CON:50/1000/500/90/Console AZur - Output/CLOSE/AUTO" :
                    "NEWCON:70/90/500/90/Console AZur - Output", 96 );

            fname = UIKBase->DefaultFontAttr.tta_Name;
            StcCpy( DefaultPrefs.FontName, fname, 32 );
            DefaultPrefs.FontHeight = UIKBase->DefaultFontAttr.tta_YSize;
            StcCpy( DefaultPrefs.ViewFontName, GfxBase->DefaultFont->tf_Message.mn_Node.ln_Name, 32 );
            DefaultPrefs.ViewFontHeight = GfxBase->DefaultFont->tf_YSize;

            CopyMem( &up->Colors, &AGP.Prefs->ColRegs, sizeof(struct UIKColors) );
            CopyMem( up->ColorTable, AGP.Prefs->ColorTable, MAX_UIKCOLORS*2 );
            DefaultPrefs.ScreenModeId = up->DefScreen_ModeId;
            DefaultPrefs.ScreenWidth = up->DefScreen_Width;
            DefaultPrefs.ScreenHeight = up->DefScreen_Height;
            if (UIK_IsSystemVersion2())
                {
                if (QueryOverscan( DefaultPrefs.ScreenModeId, &rect, OSCAN_TEXT ))
                    {
                    DefaultPrefs.ScreenWidth = rect.MaxX - rect.MinX + 1;
                    DefaultPrefs.ScreenHeight = rect.MaxY - rect.MinY + 1;
                    }
                }
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "2 mem=%ld (%ld)\n", mem, old-mem );

            if (apsh = APS_RegisterAppl( "AZurEditor", NULL, NULL, NULL, NULL, NULL, NULL ))
                {
                //------ si ENV: et ENVARC: existent on les utilise, sinon S:
                ULONG tmp = UIK_DisableDOSReq();
                if (lo_DirCreate( "ENV:AZurEditor" ) && lo_DirCreate( "ENVARC:AZurEditor" ))
                    {
                    sprintf( AGM.EnvFileName, "ENV:AZurEditor/%ls",    AGM.DefPrefsName );
                    sprintf( AGM.ArcFileName, "ENVARC:AZurEditor/%ls", AGM.DefPrefsName );
                    }
                else{
                    sprintf( AGM.EnvFileName, "S:AZurEditor/%ls", AGM.DefPrefsName );
                    sprintf( AGM.ArcFileName, "S:AZurEditor/%ls", AGM.DefPrefsName );
                    }
                UIK_EnableDOSReq( 0, tmp );

                //------ relis une preference
                if (! (p = APS_OpenPrefs( apsh, AGM.EnvFileName, AGM.DefPrefsName, sizeof(struct AZurPrefs), 0, &DefaultPrefs, sizeof(struct AZurPrefs) ))) goto END_ERROR;
                CopyMem( p, AGM.Prefs, sizeof(struct AZurPrefs) );
                APS_ClosePrefs( apsh, AGM.DefPrefsName );
                APS_UnregisterAppl( apsh );
                }
            else { StartErr(4); goto END_ERROR; }

            if (AGP.Prefs->ColRegs.Grey==0 && AGP.Prefs->ColRegs.Black==0 && AGP.Prefs->ColRegs.White==0)
                {
                CopyMem( &UIKBase->UIKPrefs->Colors, &AGP.Prefs->ColRegs, sizeof(struct UIKColors) );
                CopyMem( UIKBase->UIKPrefs->ColorTable, AGP.Prefs->ColorTable, MAX_UIKCOLORS*2 );
                }
 if (lo_FileSize( "ENV:AzurEditor/AZur.prefs" ) != 960) SET_APPICON;

            AGM.Prefs->Flags1 &= ~AZPM_NOTSAVED1;
            AGM.Prefs->Flags2 &= ~AZPM_NOTSAVED2;
            AGM.Prefs->Flags3 &= ~AZPM_NOTSAVED3;
            AGM.Prefs->SearchFlags &= ~AZPM_NOTSAVEDSEARCH;
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "3 mem=%ld (%ld)\n", mem, old-mem );

            /*------------arguments------------*/

            if (a = AGM.ArgRes[ARG_MASTPORTNAME]) StcCpy( AGM.Prefs->MastPortName, a, 16 );
            if (a = AGM.ArgRes[ARG_PORTNAME])   StcCpy( AGM.Prefs->PortName, a, 16 );
            if (a = AGM.ArgRes[ARG_FONTNAME])   StcCpy( AGM.Prefs->FontName, a, 32 );
            if (n = AGM.ArgRes[ARG_FONTHEIGHT]) AGM.Prefs->FontHeight = (UBYTE)(*n);
            if (a = AGM.ArgRes[ARG_FROM])       AGM.FromName = a;
            if (a = AGM.ArgRes[ARG_FROMCMD])    AGM.FromCmdName = a;
            if (a = AGM.ArgRes[ARG_PARMFILE])   StcCpy( AGM.Prefs->ParmFile, a, 64 );
            if (a = AGM.ArgRes[ARG_PATTERN])    StcCpy( AGM.Prefs->PatOpen, a, 64 );
            if (n = AGM.ArgRes[ARG_TABLEN])     AGM.Prefs->TabLen = (UWORD) *n;
            if (a = AGM.ArgRes[ARG_SCREENNAME]) AGM.PubScreen = a;
            if (n = AGM.ArgRes[ARG_SCREENADDR]) AGM.ScreenAdd = (struct Screen *) *n;
            if (AGM.ArgRes[ARG_SCREENFRONT])    SET_FRONTSCREEN;
            if (AGM.ArgRes[ARG_SCREENAZUR])     SET_AZURSCREEN;
            if (AGM.ArgRes[ARG_FULLSCREEN])     SET_FULLSCREEN;
            if (AGM.ArgRes[ARG_CHARDELBLOCK])   SET_CHARDELBLOCK;
            if (AGM.ArgRes[ARG_BACKUP])         SET_KEEPBKP;
            if (AGM.ArgRes[ARG_LINEMODE])       SET_DEFAULTBLM;
            if (AGM.ArgRes[ARG_MINITEL])        SET_MINITEL;
            if (AGM.ArgRes[ARG_REPLACE])        SET_OVERLAY;
            if (n = AGM.ArgRes[ARG_INDTYPE])    AGM.Prefs->IndentType = *n;
            if (AGM.ArgRes[ARG_NOWINDOW])       SET_NOWINDOW;
            if (AGM.ArgRes[ARG_READONLY])       SET_READONLY;
            if (AGM.ArgRes[ARG_HELP])           help = 1;
            if (n = AGM.ArgRes[ARG_PRIORITY])   AGM.Prefs->Priority = *n;
            if (AGM.ArgRes[ARG_RESIDENT])       SET_RESIDENT;
            if (a = AGM.ArgRes[ARG_VFONTNAME])  StcCpy( AGM.Prefs->ViewFontName, a, 32 );
            if (n = AGM.ArgRes[ARG_VFONTHEIGHT]) AGM.Prefs->ViewFontHeight = (UBYTE)(*n);
            if (n = AGM.ArgRes[ARG_IOBUFFER])   AGM.Prefs->IOBufLength = *n;
            if (AGM.ArgRes[ARG_OPENREQ])        SET_OPENREQ;
            if (AGM.ArgRes[ARG_NOAUTOSIZE])     SET_NOAUTOSIZE;
            if (n = AGM.ArgRes[ARG_MAXLINES])   AGM.MaxLines = *n;
            if (AGM.ArgRes[ARG_NOPROC])         { SET_RESIDENT; noproc = 1; }
            if (AGM.ArgRes[ARG_QUIET])          SET_ERRORQUIET;
            if (AGM.ArgRes[ARG_SAVE])           SET_SAVE;
            if (AGM.ArgRes[ARG_REQWIN])         SET_REQWIN;
            if (n = AGM.ArgRes[ARG_DEPTH])      AGM.Prefs->ScreenDepth = (UBYTE)*n;
            if (n = AGM.ArgRes[ARG_MODEID])
                {
                AGM.Prefs->ScreenModeId = *n;
                if (UIK_IsSystemVersion2())
                    {
                    if (QueryOverscan( AGM.Prefs->ScreenModeId, &rect, AGM.Prefs->ScreenOverscan ))
                        {
                        DefaultPrefs.ScreenWidth = rect.MaxX - rect.MinX + 1;
                        DefaultPrefs.ScreenHeight = rect.MaxY - rect.MinY + 1;
                        }
                    }
                }
            if (n = AGM.ArgRes[ARG_ELEMASK])    AGM.Prefs->EleMask = *n;
            if (n = AGM.ArgRes[ARG_PUDDLE])     if (! (AGM.Prefs->PuddleSize = (UBYTE)*n)) AGM.Prefs->PuddleSize = 1;
            if (n = AGM.ArgRes[ARG_DELCLIP])    AGM.Prefs->DelClip = (UBYTE)*n;
            if (AGM.ArgRes[ARG_TABTAB])         AGM.Prefs->Flags2 &= ~(AZP_TAB2SPACE|AZP_SPACE2TAB|AZP_TABDISPLAYS|AZP_TABDISPLAYE|AZP_REALTAB);
            if (AGM.ArgRes[ARG_NULLPOINTER])    SET_NULLPOINTER;
            if (AGM.ArgRes[ARG_DEBUG])          SET_DEBUG;
            if (n = AGM.ArgRes[ARG_LEFT  ])     AGM.Prefs->VI.ViewBox.Left   = (WORD)*n;
            if (n = AGM.ArgRes[ARG_TOP   ])     AGM.Prefs->VI.ViewBox.Top    = (WORD)*n;
            if (n = AGM.ArgRes[ARG_WIDTH ])     AGM.Prefs->VI.ViewBox.Width  = (WORD)*n;
            if (n = AGM.ArgRes[ARG_HEIGHT])     AGM.Prefs->VI.ViewBox.Height = (WORD)*n;

            if (tab = AGM.ArgRes[ARG_NAME])
                {
                ULONG i;
                struct AnchorPath *ma;

                if (! (ma = PoolAlloc( sizeof(struct AnchorPath) + 256 ))) { StartErr(6); goto END_ERROR; }
                ma->ap_BreakBits = SIGBREAKF_CTRL_C;
                ma->ap_Strlen = 256;

                for (i=0; tab[i]; i++)
                    {
                    if (UIK_IsSystemVersion2())
                        {
                        if (MatchFirst( tab[i], ma ) == 0)
                            {
                            do add_filenode( ma->ap_Buf ); while (MatchNext( ma ) == 0);
                            MatchEnd( ma );
                            if (ma->ap_FoundBreak & SIGBREAKF_CTRL_C) goto END_ERROR;
                            }
                        else add_filenode( tab[i] );
                        }
                    else add_filenode( tab[i] );
                    }
                PoolFree( ma );
                }
            }
        }
    if (AGM.PArg->Error != PAERR_OK)
        {
        eng_ShowRequest( "%ls, %ls", 0,
            UIK_LangString( AGM.UIK, TEXT_PARSE_ERRORBASE + AGM.PArg->Error ),
            UIK_LangString( AGM.UIK, TEXT_PARSE_COMMENT ), 0 );
        goto END_ERROR;
        }
    if (AGM.argc == 2 && AGM.argv[1][0] == '?')
        {
        goto END_ERROR;
        }
    if (help)
        {
        if (AGM.WinHelp = (struct UIKObjWinHelp *) UIK_AddObjectTagsB( UIKBase, "UIKObj_WinHelp", AGM.UIK,
            UIKTAG_OBJ_LeftTop,             SETL(10,20),
            UIKTAG_OBJ_WidthHeight,         SETL(500,180),
            UIKTAG_WinHelp_WindowTitle,     (ULONG) UIK_LangString( AGM.UIK, TEXT_WINHELP_TITLE ),
            UIKTAG_WinHelp_TextPtr,         (ULONG) UIK_LangString( AGM.UIK, TEXT_MSG_HELP ),
            TAG_END ))
            {
            UIK_Start( AGM.UIK );
            UIK_Do( AGM.UIK, SIGBREAKF_CTRL_C );
            }
        goto END_ERROR;
        }
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "4 mem=%ld (%ld)\n", mem, old-mem );

    //--- vecteur d'arrêt sur Ctrl+D
    AGM.CtrlDVect = signal_all;
    if (! UIK_AddVector( AGM.UIK, (void*)&AGM.CtrlDVect, SIGBREAKF_CTRL_D, SIGBREAKF_CTRL_D ))
        { StartErr(10); goto END_ERROR; }

    //--- liste des variables
    if (! UIK_nl_AllocList( &AGM.VarList, 10 )) { StartErr(13); goto END_ERROR; }

    //--- germes d'objets
    if (! eng_RegisterInternalObjects())
        { StartErr(11); goto END_ERROR; }
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "5 mem=%ld (%ld)\n", mem, old-mem );

    //--- fichier de paramètres
    if (! ParmInit()) { StartErr(12); goto END_ERROR; }
    if (errmsg = func_ReadParms( AGM.Prefs->ParmFile ))
        {
        eng_ShowRequest( "%ls '%ls'", errmsg, UIK_LangString( AGM.UIK, errmsg ), AGM.Prefs->ParmFile, 0 );
        goto END_ERROR;
        }
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "6 mem=%ld (%ld)\n", mem, old-mem );

    //--- font
    verif_fontname( AGM.Prefs->FontName );
    verif_fontname( AGM.Prefs->ViewFontName );
    { struct TextFont *font; ULONG ok=0;
    if (font = UIK_OpenFont( AGM.Prefs->ViewFontName, AGM.Prefs->ViewFontHeight ))
        {
        if (! (font->tf_Flags & FPF_PROPORTIONAL)) ok = 1;
        CloseFont( font );
        }
    if (! ok)
        {
        eng_ShowRequest( "%ls", 0, (UBYTE*)UIK_LangString( AGM.UIK, TEXT_PROPFONT ), 0, 0 );
        goto END_ERROR;
        /*
        StcCpy( AGM.Prefs->ViewFontName, GfxBase->DefaultFont->tf_Message.mn_Node.ln_Name, 32 );
        AGM.Prefs->ViewFontHeight = GfxBase->DefaultFont->tf_YSize;
        */
        }
    }

    //--- ARexx
    AGM.ARexx = (APTR) UIK_AddObjectTagsB( UIKBase, "UIKObj_ARexxSim", AGM.UIK,
            UIKTAG_ARexxSim_AppName,        AGM.Prefs->MastPortName,
            UIKTAG_ARexxSim_VarName,        "AZURMAST",
            UIKTAG_ARexxSim_Extension,      "raz",
            (TST_REXXNAMESEP ? UIKTAG_ARexxSim_PortNameDot : TAG_IGNORE), AGM.Prefs->CharRexxDot,
            UIKTAG_ARexxSim_PortNameNoNum,  (TST_REXXNAMENUM ? FALSE : TRUE),
            UIKTAG_ARexxSim_CmdReceiveFunc, Mast_ReceiveARexxCmd,
            UIKTAG_ARexxSim_CmdBackFunc,    Mast_CmdComeBack,
            UIKTAG_ARexxSimFl_EnableKeep,   TRUE,
            UIKTAG_ARexxSimFl_LeaveCase,    TST_LEAVEREXCASE,
            TAG_END ); // pas de test d'erreur car possible pas ARexx installé

    //--- Screen
    if (TST_AZURSCREEN) add_main_interface();

    if (AGP.WO)
        {
        ctrlf_f_func = activate_window;
        UIK_AddVector( AGP.UIK, (void*)&ctrlf_f_func, SIGBREAKF_CTRL_F, (ULONG)&ctrlf_f_func );
        }
    if (AGM.ScreenObj) AGM.PubScreen = AGM.PubScreenName;

    if (! UIK_Start( AGM.UIK )) goto END_ERROR;

//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "7 mem=%ld (%ld)\n", mem, old-mem );
    //--- Files
    SetTaskPri( AGM.Process, AGM.Prefs->Priority );
    if (! noproc)
        {
        if (IsListEmpty( &AGM.FileList ))
            {
            ok = func_LaunchProcess( AZTAG_Launch_FromScript, AGM.FromName,
                                     AZTAG_Launch_FromCmd,    AGM.FromCmdName,
                                     AZTAG_LaunchF_QuitAfter, TRUE,
                                     TAG_END );
            }
        else{
            for (num=1,node=AGM.FileList.lh_Head; node->ln_Succ; node = node->ln_Succ, num++)
                {
                ok = func_LaunchProcess( AZTAG_LaunchF_Iconify,   (num == AGM.FileCount) ? FALSE : TRUE,
                                         AZTAG_Launch_FromScript, AGM.FromName,
                                         AZTAG_Launch_FromCmd,    AGM.FromCmdName,
                                         AZTAG_Launch_FileName,   node->ln_Name,
                                         AZTAG_LaunchF_QuitAfter, TRUE,
                                         TAG_END );
                if (!ok) break;
                if (SIGBREAKF_CTRL_C & SetSignal( 0, 0 )) goto END_ERROR; // pour arrêt de lancement en chaine
                }
            while (node = RemHead( &AGM.FileList )) PoolFree( node );
            }
        if (!ok && My_IsListEmpty( &AGM.ProcList )) { StartErr(16); goto END_ERROR; }
        }
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "8 mem=%ld (%ld)\n", mem, old-mem );

    //--- vecteur de lancement d'une autre tâche
    launch_func = launch_quick;
    if (UIK_AddVector( AGM.UIK, (void*)&launch_func, SIGBREAKF_CTRL_E, (ULONG)&launch_func ))
        {
        UBYTE *ptr = (UBYTE*) UIK_Call( AGM.ARexx, UIKFUNC_ARexxSim_ARexxName, 0, 0 );
        ULONG len = StrLen(QuickPortName);
        if (! ptr) ptr = AGM.Prefs->MastPortName;
        StcCpy( QuickPortName+len, ptr, 32-len );
        UIK_StrToUpper( QuickPortName );

        //--- Ajout du port de lancement
        AGM.LaunchPort.mp_Node.ln_Type = NT_MSGPORT;
        AGM.LaunchPort.mp_Node.ln_Pri = -10;
        AGM.LaunchPort.mp_Node.ln_Name = QuickPortName;
        AGM.LaunchPort.mp_Flags = PA_SIGNAL;
        AGM.LaunchPort.mp_SigBit = SIGBREAKB_CTRL_E;
        AGM.LaunchPort.mp_SigTask = AGM.Process;
        AddPort( &AGM.LaunchPort );
        }

    //--- AppIcon
    if (AGM.Prefs->AppIconName[0] == 0) StcCpy( AGM.Prefs->AppIconName, AGM.Prefs->MastPortName, 32 );
    if (TST_APPICON) func_StartAppIcon();

    //--- Attente
    SetTaskPri( AGM.Process, AGM.Prefs->Priority + 1 );
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "9 mem=%ld (%ld)\n", mem, old-mem );
    breaksig = UIK_Do2( AGM.UIK, SIGBREAKF_CTRL_C, 1 );

  END_ERROR:
}

void main( int argc, char **argv )
{
  AZUR *az;

    LangEnglish[TEXT_VERSION] = LangOther[TEXT_VERSION] = VersionText;

//mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST );
    if (! (az = Init( sizeof(AZUR) ))) goto END_ERROR;  //--- init A4, libs
    AGM.AZMast = az;
    AGM.argc = argc;
    AGM.argv = argv;
    AllocNewStack( AGM.NewStack, 10000 );
    //------------------------------------------------------------------------------------------
    // pas d'utilisation de variable en pile à la fois avant et après à cause de AllocNewStack()
    //------------------------------------------------------------------------------------------
    main2();
    RestoreStack( AGM.NewStack );
  END_ERROR:
    Cleanup();
}

//-------- appelable depuis un process

ULONG func_LaunchProcess( ULONG tag, ... )
{
  struct Process *me;
  struct MsgPort *procport=0;
  struct CommandLineInterface *cli;
  ULONG stacksize;
  UBYTE name[20], *patreq=0, **resultport;
  struct AZurNode *node;
  struct AZur *azur = AGP.AZMast;
  ULONG OKMask = 1L<<AGP.OKSigBit;
  ULONG ErrorMask = 1L<<AGP.ErrorSigBit;
  struct TagItem *taglist = (struct TagItem *) &tag;
  ULONG priority;
  struct AZProcParms *parms;

    me = (struct Process *) FindTask(0);
    cli = (struct CommandLineInterface *) (me->pr_TaskNum ? BADDR(me->pr_CLI) : 0);
    stacksize = cli ? 4 * cli->cli_DefaultStack : me->pr_StackSize;
    sprintf( name, "AZUR$%08.8lx", azur );

    ObtainSemaphore( &azur->ProcSem );
    if (node = (APTR) AllocMem( sizeof(struct AZurNode), MEMF_ANY | MEMF_CLEAR ))
        {
        SetSignal( 0, OKMask | ErrorMask );

        priority = GetTagData( AZTAG_Launch_Priority, (ULONG)AGP.Prefs->Priority, taglist );
        resultport = (UBYTE**) GetTagData( AZTAG_Launch_ResultPort, 0, taglist );

        if ((&AGP != (struct AZurProc *)azur) && AGP.FObj->OpenFS && AGP.FObj->OpenFS->FS->StrPatGad)
            {
            patreq = UIK_OSt_Get( AGP.FObj->OpenFS->FS->StrPatGad, 0 );
            }
        if (patreq == 0) patreq = azur->Prefs->PatOpen;

        parms = azur->Parms;
        parms->ParmParentTask = me;
        parms->ParmFromCmd    = (UBYTE*) GetTagData( AZTAG_Launch_FromCmd,    (ULONG)azur->FromCmdName, taglist );
        parms->ParmFromScript = (UBYTE*) GetTagData( AZTAG_Launch_FromScript, (ULONG)azur->FromName, taglist );
        parms->ParmPortName   = (UBYTE*) GetTagData( AZTAG_Launch_PortName,   (ULONG)azur->Prefs->PortName, taglist );
        parms->ParmPattern    = (UBYTE*) GetTagData( AZTAG_Launch_Pattern,    0, taglist );
        if (parms->ParmPattern == 0 || parms->ParmPattern[0] == 0) parms->ParmPattern = patreq;
        parms->ParmCmdObj     = (struct CmdObj *) GetTagData( AZTAG_Launch_CmdObj, 0, taglist );
        parms->ParmFileName   = (UBYTE*) GetTagData( AZTAG_Launch_FileName,   0, taglist );
        parms->ParmIconify    = GetTagData( AZTAG_LaunchF_Iconify,  0, taglist );
        parms->ParmQuickMsg   = GetTagData( AZTAG_Launch_QuickMsg,  0, taglist );
        parms->ParmDirLock    = GetTagData( AZTAG_Launch_DirLock,   azur->CurDirLock, taglist );
        parms->ParmLeftTop    = GetTagData( AZTAG_Launch_LeftTop,    -1, taglist );
        parms->ParmWidthHeight= GetTagData( AZTAG_Launch_WidthHeight,-1, taglist );
        parms->ParmTextWH     = GetTagData( AZTAG_Launch_TextWidthHeight, -1, taglist );
        parms->ParmFlags1     = PackBoolTags( azur->Prefs->Flags1, taglist, flags_boolmap1 );
        parms->ParmFlags2     = PackBoolTags( azur->Prefs->Flags2, taglist, flags_boolmap2 );
        parms->ParmFlags3     = PackBoolTags( azur->Prefs->Flags3, taglist, flags_boolmap3 );
        parms->ParmSearchFlags = azur->Prefs->SearchFlags;
        parms->ParmPrefs      = AGP.Prefs;
        parms->ParmPrefsName  = AGP.DefPrefsName;
        parms->ParmPrefsFileName = (UBYTE*) GetTagData( AZTAG_Launch_PrefsFileName, 0, taglist );

        Forbid();
        if (procport = CreateProc( name, priority, GetProcSegment(), MAX(stacksize, 10000) ))
            {
            ULONG events;
            struct Task *child = (struct Task *) (((UBYTE *) procport) - sizeof(struct Task));

            if (azur == (APTR)&AGP)
                //------------------- Part de la tâche maîtresse
                {
                while (TRUE)
                    {
                    // uniquement si utilise UIK_WaitFirstEvents() pour le IPUIK_Start() du screen
                    // events = UIK_WaitFirstEvents( AGM.UIK, ErrorMask | OKMask | SIGBREAKF_CTRL_C );
                    events = Wait( ErrorMask | OKMask | SIGBREAKF_CTRL_C );

                    if (events & ErrorMask)
                        {
                        procport = 0;
                        break; // si while()
                        }
                    else if (events & SIGBREAKF_CTRL_C)
                        {
                        Signal( child, SIGBREAKF_CTRL_C );
                        Signal( me, SIGBREAKF_CTRL_C );
                        events = Wait( ErrorMask | OKMask );
                        break; // si while()
                        }
                    else if (events & OKMask)
                        {
                        if (resultport)  // parms->ParmResultPort == nom du port ARexx créé
                            BufSetS( parms->ParmResultPort, StrLen( parms->ParmResultPort ), resultport );
                        break; // si while()
                        }
                    }
                }
            else//------------------- Part d'une tâche fille
                {
                events = Wait( ErrorMask | OKMask | SIGBREAKF_CTRL_C );

                if (events & ErrorMask)
                    {
                    procport = 0;
                    }
                else if (events & SIGBREAKF_CTRL_C)
                    {
                    Signal( child, SIGBREAKF_CTRL_C );
                    Signal( me, SIGBREAKF_CTRL_C );
                    events = Wait( ErrorMask | OKMask );
                    }
                else if (events & OKMask)
                    {
                    if (resultport)  // parms->ParmResultPort == nom du port ARexx créé
                        BufSetS( parms->ParmResultPort, StrLen( parms->ParmResultPort ), resultport );
                    }
                }

            if (events & ErrorMask) procport = 0;
            else if (events & OKMask)
                {
                node->ln_Task = child;
                node->ln_PortName = parms->ParmResultPort;
                AddTail( &azur->ProcList, (struct Node *)node );    // doit être sous Forbid()
                azur->ProcNum++;
                }
            }
        Permit();
        if (! procport) FreeMem( node, sizeof(struct AZurNode) );
        }
  END:
    ReleaseSemaphore( &azur->ProcSem );
    return( (ULONG) procport );
}

/****************************************************************
 *
 *      Azur Process
 *
 ****************************************************************/

/*
static ULONG count_procs()
{
  ULONG num;
  struct AZurNode *node;

    ObtainSemaphore( &AGP.AZMast->ProcSem );
    for (num=0, node=(struct AZurNode *)AGP.AZMast->ProcList.lh_Head; node->ln_Succ; node = (struct AZurNode *)node->ln_Succ) num++;
    ReleaseSemaphore( &AGP.AZMast->ProcSem );
    return( num );
}
*/

/*
/** Appelée par ActualizePrefs()
 **/
void ChangeMainScreen()
{
  struct AZurPrefs *pr = AGP.AZMast->Prefs;
  struct UIKObjScreen *os;

 // pour combler le manque d'éditeur de préférences pour ça
 CopyMem( &UIKBase->UIKPrefs->Colors, &AGP.Prefs->ColRegs, sizeof(struct UIKColors) );

    if (os = AGP.AZMast->ScreenObj)
        {
        if (pr->ScreenWidth != os->Obj.Box.Width || pr->ScreenHeight != os->Obj.Box.Height
            || pr->ScreenDepth != os->NS.Depth || pr->ScreenModeId != os->ModeId)
            {
            if (TST_AZURSCREEN)
                {
                IPUIK_Stop( os );
                if (os = (APTR) IPUIK_AddObjectTags( AGP.AZMast->Process, "UIKObj_Screen", AGP.AZMast->UIK,
                    UIKTAG_OBJ_LeftTop,         *((ULONG*)&pr->ScreenLeft),
                    UIKTAG_OBJ_WidthHeight,     *((ULONG*)&pr->ScreenWidth),
                    UIKTAG_OBJ_FontName,        pr->FontName,
                    UIKTAG_OBJ_FontHeight,      pr->FontHeight,      /* pour option SZ sous 1.3 */
                    UIKTAG_OBJ_Title,           UIK_IsSystemVersion2() ? TEXT_AZurScreenTitle : "AZur Screen",
                    UIKTAG_OBJ_TitleFl_Addr,    UIK_IsSystemVersion2() ? FALSE : TRUE,
                    UIKTAG_ObjScreen_Depth,     pr->ScreenDepth,
                    UIKTAG_ObjScreen_ViewModes, pr->ScreenModeId,
                    UIKTAG_ObjScreen_ColorTable,pr->ColorTable,
                    UIKTAG_OBJScreen_RedirectColors,&pr->ColRegs,
                    UIKTAG_OBJScreen_IntuiTags, IntuitionScreenTags,
                    // UIKTAG_OBJ_BoxFl_StartSpecify, TRUE,
                    TAG_END ))
                    {
                    IPUIK_Remove( AGP.AZMast->Process, AGP.AZMast->ScreenObj );
                    AGP.AZMast->ScreenObj = os;
                    }
                else UIK_Start( os );
                }
            }
        }
}
*/

static void ProcCleanup( AZUR *mast, int error )
{
  APTR vars = (APTR) __builtin_getreg( 12 ); // A4 pointe sur la structure des variables
  struct Task *me = FindTask(0);
  struct AZurNode *node;

    if (vars)
        {
        if (DOSBase)
            {
            if (GfxBase)
                {
                if (IconBase)
                    {
                    if (LayersBase)
                        {
                        if (IntuitionBase)
                            {
                            if (UIKBase)
                                {
                                if (AGP.CurDirLock)
                                    {
                                    if (AGP.UIK)
                                        {
                                        if (AGP.QuickMsg) ReplyQuickMsg( AGP.QuickMsg );

                                        UIK_Remove( AGP.FObj ); // séparé à cause console ARexx
                                        Forbid();
                                        if (mast->ProcNum == 1) // dernier ?
                                            {
                                            //IPUIK_Stop( mast->Process, mast->ScreenObj );
                                            //ChangeMainScreen();
                                            }
                                        Permit();
                                        UIK_Remove( AGP.UIK );

                                        BufFree( AGP.LastCmd );
                                        BufFree( AGP.RexxResult );
                                        if (AGP.RexxConsole) Close( AGP.RexxConsole );
                                        if (AGP.DOSConsole)
                                            {
                                            if (TST_DEBUG) Delay(200);
                                            Close( AGP.DOSConsole );
                                            }
                                        if (AGP.NilFh) Close( AGP.NilFh );

                                        if (AGP.Pool) AsmDeletePool( AGP.Pool, *(APTR*)4 );
                                        }
                                    UnLock( AGP.CurDirLock );
                                    }
                                if (WorkbenchBase) CloseLibrary( WorkbenchBase );
                                CloseLibrary( UIKBase );
                                }
                            CloseLibrary( IntuitionBase );
                            }
                        CloseLibrary( LayersBase );
                        }
                    CloseLibrary( IconBase );
                    }
                CloseLibrary( GfxBase );
                }
            CloseLibrary( DOSBase );
            }
        FreeSpMem( vars ); // dans arg3.lib
        }
    //------ AGP n'est plus valide

    Forbid();
    if (error)
        {
        Signal( mast->Parms->ParmParentTask, 1L<<mast->ErrorSigBit );
        }
    else{
        ObtainSemaphore( &mast->ProcSem );
        for (node=(struct AZurNode *)mast->ProcList.lh_Head; node->ln_Succ; node = (struct AZurNode *)node->ln_Succ)
            {
            if (me == node->ln_Task)
                {
                Remove( (struct Node *)node );     // doit être sous Forbid()
                mast->ProcNum--;
                FreeMem( node, sizeof(struct AZurNode) );
                break;
                }
            }
        if (My_IsListEmpty( &mast->ProcList ))
            {
            if (! (mast->Prefs->Flags1 & AZP_RESIDENT))
                Signal( mast->Process, SIGBREAKF_CTRL_C );
            }
        ReleaseSemaphore( &mast->ProcSem );
        }
}


/*
static ULONG DoRexx( ULONG msg )
{
    if (msg && TST_NOWINDOW)
        {
        AGP.TmpRexxMsg = msg;
        while (AGP.TmpRexxMsg)
            {
            if (UIK_WaitFirstEvents( AGP.UIK, SIGBREAKF_CTRL_C ) & SIGBREAKF_CTRL_C) return(0);
            }
        }
    return(1);
}
*/

/* voir aussi dans func_reqs.c */

#ifdef AZUR_DEMO
//------------- Demo
UBYTE __far SaveCopyrigthCode[] = "\x38\x3B\x01\x31\x37\x2B\x13\x14\x23\x61\x45\x3A\x0E\x04\x3A\x1A\x06\x11\x11\x38\x6D\xCC\x59\x50\x4D\x7A\x5C\x4F\x3C\x1C\x2D\x23\x48\x34\x08\x17\x2B\x0D\x03\x56\x3F\x23\x3F\x02\x1C\x00\x07\x49\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x2E\x1A\x15\x6C\x1F\x0C\x1E\x09\x00\x30\x48\x3D\x13\x0A\x29\x3F\x13\x1C\x05\x7E\x49\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x49\x2C\x0A\x1B\x16\x22\x3E\x11\x0B\x00\x00\x2A\x07\x01\x56\x09\x3E\x22\x02\x0B\x00\x19\x6F\x48\x1C\x17\x15\x29\x6D\x03\x16\x13\x16\x2A\x0C\x0B\x13\x17\x4C";
UBYTE __far SaveCopyrigthCode2[] = "\x38\x3B\x01\x31\x37\x2B\x13\x14\x23\x61\x45\x3A\x0E\x04\x3A\x1A\x06\x11\x11\x38\x6D\xCC\x59\x50\x4D\x7A\x5C\x4F\x3C\x1C\x2D\x23\x48\x34\x08\x17\x2B\x0D\x03\x56\x3F\x23\x3F\x02\x1C\x00\x07\x49\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x3C\x00\x03\x0A\x6C\x09\x17\x16\x08\x00\x30\x48\x3D\x9F\x0A\x29\x3F\x13\x90\x12\x7E\x49\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x49\x38\x1D\x19\x1E\x3E\x2C\x08\x14\x04\x54\x27\x0D\x4F\x12\x90\x21\x22\x0B\x0A\x15\x06\x22\x1C\x06\x19\x17\x60\x6D\x13\x1C\x0F\x00\x26\x48\x06\x18\x0D\x29\x3F\x01\x10\x15\x11\x43";
UBYTE __far SaveVersionCode[] = "\x5D\x37\x31\x11\x52\x4F\x37\x23\x39\x3F\x3A\x3D\x04\x19\x2C\x48\x5E\x58\x49\x7F\x6D\x4D\x4F\x4F\x4D\x6D\x51\x5A\x5F\x79";
#else
//------------- Commercial
UBYTE __far SaveCopyrigthCode[] = "\x38\x3B\x01\x31\x44\x4F\x35\x16\x3C\x34\x17\x10\x06\x1C\x37\x48\xC6\x56\x48\x75\x74\x51\x59\x2B\x11\x22\x06\x42\x3B\x10\x2F\x25\x00\x15\x41\x32\x2C\x1A\x08\x13\x18\x3F\x47\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x38\x0D\x18\x63\x3A\x06\x11\x11\x38\x3E\x45\x2B\x04\x07\x26\x1A\x19\x13\x1D\x46\x47\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x47\x26\x16\x0C\x19\x26\x1A\x0C\x1F\x18\x20\x6D\x15\x0B\x0E\x13\x31\x09\x02\x5A\x59\x28\x24\x16\x0D\x13\x1D\x21\x1D\x1B\x1F\x16\x22\x6D\x03\x16\x13\x16\x2A\x0C\x0B\x13\x17\x4C";
UBYTE __far SaveCopyrigthCode2[] = "\x38\x3B\x01\x31\x44\x4F\x35\x16\x3C\x34\x17\x10\x06\x1C\x37\x48\xC6\x56\x48\x75\x74\x51\x59\x2B\x11\x22\x06\x42\x3B\x10\x2F\x25\x00\x15\x41\x32\x2C\x1A\x08\x13\x18\x3F\x6D\x45\x73\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x35\x1B\x36\x1B\x4F\x32\x0B\x23\x24\x11\x0A\x41\x26\xAA\x1B\x0A\x04\x0F\xA5\x3E\x6F\x73\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x73\x31\x06\x2C\x0F\x1D\x17\x14\x21\x28\x45\x1A\x0E\x19\x2E\x0D\x1D\x15\x10\x2D\x21\x49\x59\x05\x1D\x30\x1C\x1D\x1F\x1B\x39\x39\x0C\x16\x0F\x54\x2A\x06\x1B\x13\x0B\x28\x24\x11\x1C\x61";
UBYTE __far SaveVersionCode[] = "\x5D\x37\x31\x11\x52\x4F\x37\x23\x39\x3F\x45\x48\x4F\x44\x70\x48\x47\x40\x57\x75\x63\x5C\x4C\x48\x74";
#endif

void CopyProt()
{
    CopyMem( SaveCopyrigthCode, CopyrigthCode, CopyrigthCodeLen );
    CopyMem( SaveCopyrigthCode2, CopyrigthCode2, CopyrigthCode2Len );
    CopyMem( SaveVersionCode, VersionCode, VersionCodeLen );
}

/*
static void ctrl_c_func_do( void **var )
{
    *var = ctrl_c_func_do;  // ctrlf_c_func
}
*/

void ProcessMain()
{
  struct Process *me = (struct Process *) FindTask(0);
  AZUR *mast;
  UBYTE *portname, *ptr;
  ULONG events, loadfile, fromarexx, async, len, iconify, error=1, startrc, lefttop, widthheight, textwh;
  UIKBUF *ub_cmd=0, *ub_script=0, *ub_pat=0, *ub_file=0;
  void *ctrlf_f_func;
  struct AZProcParms *parms;
//ULONG mem, old;

    if (ARG_StrToLong( &me->pr_Task.tc_Node.ln_Name[4], (ULONG*)&mast ) == -1) goto END_ERROR; // arg3.lib

    if (! Init( sizeof(AZURPROC) )) goto END_ERROR;  //--- init A4, libs

    AGP.AZMast = mast;
    //me->pr_Task.tc_Node.ln_Name[4] = 0;
    ptr = me->pr_Task.tc_Node.ln_Name;
    ptr[4] = 'E'; ptr[5] = 'D'; ptr[6] = 0;
    *((ULONG*)&ptr[8]) = (ULONG) &AGP;

    if (! (AGP.ARexxFileName = BufAllocP( AGP.Pool, 0, 10 ))) goto END_ERROR;
    if (! (AGP.DOSFileName = BufAllocP( AGP.Pool, 0, 10 ))) goto END_ERROR;
    if (! (AGP.Prefs = PoolAlloc( 32 + 2 * sizeof(struct AZurPrefs) ))) goto END_ERROR;
    AGP.TmpPrefs = (struct AZurPrefs *)((ULONG)AGP.Prefs + sizeof(struct AZurPrefs));
    AGP.DefPrefsName = (UBYTE *)((ULONG)AGP.Prefs + 2 * sizeof(struct AZurPrefs));
    /*
    Forbid();
    CopyMem( mast->Prefs, AGP.Prefs, sizeof(struct AZurPrefs) );
    StrCpy( AGP.DefPrefsName, mast->DefPrefsName );
    *((struct IBox *) &AGP.AZReqBookmark.Left) = AGP.Prefs->PosBookmarks;
    *((struct IBox *) &AGP.AZReqLine.Left)     = AGP.Prefs->PosLineCol  ;
    *((struct IBox *) &AGP.AZReqOffset.Left)   = AGP.Prefs->PosOffset   ;
    *((struct IBox *) &AGP.AZReqSearch.Left)   = AGP.Prefs->PosSearch   ;
    *((struct IBox *) &AGP.AZReqReplace.Left)  = AGP.Prefs->PosReplace  ;
    Permit();
    */

//mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST );
    //--- recueille les paramètres
    parms = mast->Parms;

    Forbid();
    CopyMem( parms->ParmPrefs, AGP.Prefs, sizeof(struct AZurPrefs) );
    StrCpy( AGP.DefPrefsName, parms->ParmPrefsName );
    Permit();
    *((struct IBox *) &AGP.AZReqBookmark.Left) = AGP.Prefs->PosBookmarks;
    *((struct IBox *) &AGP.AZReqLine.Left)     = AGP.Prefs->PosLineCol  ;
    *((struct IBox *) &AGP.AZReqOffset.Left)   = AGP.Prefs->PosOffset   ;
    *((struct IBox *) &AGP.AZReqSearch.Left)   = AGP.Prefs->PosSearch   ;
    *((struct IBox *) &AGP.AZReqReplace.Left)  = AGP.Prefs->PosReplace  ;

    if (parms->ParmFromCmd)    { if (len = StrLen( parms->ParmFromCmd )) { if (ub_cmd = BufAllocP( AGP.Pool, len, 4 )) StrCpy( ub_cmd, parms->ParmFromCmd ); } }
    if (parms->ParmFromScript) { if (len = StrLen( parms->ParmFromScript )) { if (ub_script = BufAllocP( AGP.Pool, len, 4 )) StrCpy( ub_script, parms->ParmFromScript ); } }
    if (parms->ParmPattern)    { if (len = StrLen( parms->ParmPattern )) { if (ub_pat = BufAllocP( AGP.Pool, len, 4 )) StrCpy( ub_pat, parms->ParmPattern ); } }
    if (parms->ParmFileName)   { if (len = StrLen( parms->ParmFileName )) { if (ub_file = BufAllocP( AGP.Pool, len, 4 )) StrCpy( ub_file, parms->ParmFileName ); } }
    portname  = parms->ParmPortName;
    iconify   = parms->ParmIconify;
    AGP.QuickMsg = (APTR) parms->ParmQuickMsg;
    AGP.Prefs->Flags1 = parms->ParmFlags1; // peuvent être modifiés par les arguments de LaunchProcess
    AGP.Prefs->Flags2 = parms->ParmFlags2;
    AGP.Prefs->Flags3 = parms->ParmFlags3;
    AGP.Prefs->SearchFlags = parms->ParmSearchFlags;
    lefttop     = parms->ParmLeftTop;
    widthheight = parms->ParmWidthHeight;
    textwh      = parms->ParmTextWH;
    fromarexx = (parms->ParmCmdObj && (parms->ParmCmdObj->ExeFrom & FROMAREXX)) ? 1 : 0;

    if (parms->ParmPrefsFileName) do_openprefs( 0, parms->ParmPrefsFileName );

    //--- inits
    AGP.Process = me;
    AGP.JumpOffsets = (void*) AZ_FuncTable;

    if ((AGP.OKSigBit = AllocSignal( mast->OKSigBit )) == -1 || (AGP.ErrorSigBit = AllocSignal( mast->ErrorSigBit )) == -1)
        { StartErr(55); goto END_ERROR; }

    if (AGP.CurDirLock = DupLock( parms->ParmDirLock ))
        {
        CurrentDir( AGP.CurDirLock );
        }
    loadfile = NAME_IS_NULL;
    if (ub_file && ub_file[0])
        {
        ULONG lock;

        loadfile = NAME_IS_VOID; // au moins, un nom est passé en argument --> voir après si lancer en synchrone

        if (lock = Lock( ub_file, ACCESS_READ ))
            {
            if (Examine( lock, &AGP.FIB ))
                {
                if (AGP.FIB.fib_DirEntryType < 0) // si fichier
                    {
                    ULONG filelock = lock;
                    lock = ParentDir( filelock );
                    UnLock( filelock );

                    loadfile = NAME_IS_FILE; // fichier : donc si ARexx lancer en synchrone
                    lo_GetPath( parms->ParmFileName, &ub_file );
                    BufTruncate( ub_file, BufLength( ub_file ) - 1 );
                    }
                else{
                    loadfile = NAME_IS_DIR; // directory : donc même si ARexx lancer en asynchrone
                    BufFreeS( &ub_file );
                    }

                CurrentDir( lock );
                if (AGP.CurDirLock) UnLock( AGP.CurDirLock );
                AGP.CurDirLock = lock; lock = 0;
                }
            if (lock) UnLock( lock );
            }
        }
    if (! AGP.CurDirLock)
        {
        if (! (AGP.CurDirLock = DupLock( mast->CurDirLock ))) { StartErr(56); goto END_ERROR; }
        CurrentDir( AGP.CurDirLock );
        }
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "\n1 mem=%ld (%ld)\n", mem, old-mem );

    /*------------ajout UIK------------*/
    if (! (AGP.UIK = (struct UIKGlobal *) UIK_AddObjectTagsB( UIKBase, "UIK", 0,
            UIKTAG_GEN_LangEnglish,     LangEnglish,
            UIKTAG_GEN_LangOther,       LangOther,
            UIKTAG_GEN_LangDuplicate,   FALSE,
            UIKTAG_GEN_Catalog,         "AZurEditor.catalog",
            TAG_END ))) { StartErr(57); goto END_ERROR; }
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "2 mem=%ld (%ld)\n", mem, old-mem );

    /*------------ajouts avant start------------*/
    if (! (AGP.NilFh = Open( "NIL:", MODE_NEWFILE ))) goto END_ERROR;

    if (! (AGP.Timer = (struct UIKObjTimer *) UIK_AddObjectTagsB( UIKBase, "UIKObj_Timer", AGP.UIK,
            UIKTAG_Timer_Unit,      UNIT_VBLANK,
            UIKTAG_Timer_TimeFunc,  AutosaveFunc,
            UIKTAG_Timer_Seconds,   AGP.Prefs->AutsIntSec,
            TAG_END ))) { StartErr(58); goto END_ERROR; }
    if (AGP.Prefs->AutsIntSec && TST_AUTOSAVE) UIK_Call( AGP.Timer, UIKFUNC_Timer_Start, 0, 0 );
#ifdef AZUR_DEMO
    UIK_Call( AGP.Timer, UIKFUNC_Timer_Start, 0, 0 );
#endif

    UIK_AddObjectTagsB( UIKBase, "UIKObj_Inputick", AGP.UIK,
                UIKTAG_Inputick_EventFunc,  FromInpuTick,
                TAG_END );
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "3 mem=%ld (%ld)\n", mem, old-mem );

    AGP.ARexx = (APTR) UIK_AddObjectTagsB( UIKBase, "UIKObj_ARexxSim", AGP.UIK,
            UIKTAG_ARexxSim_AppName,        (portname ? portname : AGP.Prefs->PortName),
            UIKTAG_ARexxSim_VarName,        "AZUR",
            UIKTAG_ARexxSim_Extension,      "raz",
            (TST_REXXNAMESEP ? UIKTAG_ARexxSim_PortNameDot : TAG_IGNORE), AGP.Prefs->CharRexxDot,
            UIKTAG_ARexxSim_PortNameNoNum,  (TST_REXXNAMENUM ? FALSE : TRUE),
            UIKTAG_ARexxSim_CmdReceiveFunc, Proc_ReceiveARexxCmd,
            UIKTAG_ARexxSim_CmdBackFunc,    Proc_CmdComeBack,
            UIKTAG_ARexxSimFl_EnableKeep,   TRUE,
            UIKTAG_ARexxSimFl_LeaveCase,    TST_LEAVEREXCASE,
            TAG_END );
    parms->ParmResultPort = (UBYTE *) UIK_Call( AGP.ARexx, UIKFUNC_ARexxSim_ARexxName, 0, 0 );
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "4 mem=%ld (%ld)\n", mem, old-mem );

    /*
    if (TST_AZURSCREEN && !(TST_NOWINDOW) && AGP.AZMast->ScreenObj)
        {
        Forbid();
        if (AGP.AZMast->ScreenObj->Obj.Status == UIKS_STOPPED)
            IPUIK_Start( AGP.AZMast->Process, AGP.AZMast->ScreenObj );
        Permit();
        }
    */
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "5 mem=%ld (%ld)\n", mem, old-mem );

    if (! (AGP.FObj = (struct AZObjFile *) UIK_AddPrivateObjectTagsB( UIKBase, &ObjFileRegGerm, AGP.UIK,
            UIKTAG_OBJ_LeftTop,     lefttop,
            UIKTAG_OBJ_WidthHeight, widthheight,
            UIKTAG_OBJ_FontName,    AGP.Prefs->FontName,
            UIKTAG_OBJ_FontHeight,  AGP.Prefs->FontHeight,
            UIKTAG_OBJ_UserValue1,  ub_pat,
            AZTAG_File_FileName,    ub_file,
            AZTAG_File_TextWidthHeight, textwh,
            AZTAG_FileFl_Iconify,   iconify,
            TAG_END ))) { StartErr(60); goto END_ERROR; }
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "6 mem=%ld (%ld)\n", mem, old-mem );

    if (AGP.WO)
        {
        ctrlf_f_func = activate_window;
        if (! UIK_AddVector( AGP.UIK, (void*)&ctrlf_f_func, SIGBREAKF_CTRL_F, (ULONG)&ctrlf_f_func ))
            { StartErr(61); goto END_ERROR; }
        }

    if (loadfile == NAME_IS_NULL) async = 1;
    else{
        if (loadfile == NAME_IS_DIR) async = 1;
        else{
            if (loadfile == NAME_IS_FILE) async = 0;
            else // (loadfile == NAME_IS_VOID)
                {
                if (TST_CREATEFILE) async = 1;
                else{
                    if (fromarexx == 0) async = 1;
                    else{
                        AGP.MaxError = 20;
                        goto END_ERROR; // signale l'erreur à la fin du cleanup
                        }
                    }
                }
            }
        }

    if (async) // redonne la main au mast avant le start
        {
        AGP.MaxError = 20;
        Signal( parms->ParmParentTask, 1L<<mast->OKSigBit ); // car la demande de fichier du start peut bloquer
        ObtainSemaphore( &mast->ProcSem );  // on attend que le père ait fini avec moi-même
        ReleaseSemaphore( &mast->ProcSem );
        }
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "7 mem=%ld (%ld)\n", mem, old-mem );

    /*------------start------------*/
    startrc = UIK_Start( AGP.UIK );
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "8 mem=%ld (%ld)\n", mem, old-mem );
    if (startrc)
        {
        if (async == 0) // parent pas encore signalé
            {
            Signal( parms->ParmParentTask, 1L<<mast->OKSigBit );
            ObtainSemaphore( &mast->ProcSem );  // on attend que le père ait fini avec moi-même
            ReleaseSemaphore( &mast->ProcSem );
            }
        }
    else{
        StartErr(62);
        if (async) goto END_ERROR_NOCODE; // car parent déjà signalé ci-dessus
        else goto END_ERROR; // signale l'erreur à la fin du cleanup
        }

    CopyProt();
    if (TST_DEBUG)
        AGP.DOSConsole = Open( AGP.Prefs->DOSConsoleDesc, MODE_NEWFILE );

    /*------------aprés start------------*/
    BufFree( ub_file );
    BufFree( ub_pat );
    if (ub_cmd)
        {
        startrc = UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_SendSyncCmd, (ULONG)ub_cmd, 1 );
        if (!startrc || (startrc & (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D))) { StartErr(63); goto END_ERROR_NOCODE; }
        }
    if (ub_script)
        {
        startrc = UIK_CallObjectFunc( AGP.ARexx, UIKFUNC_ARexxSim_SendSyncCmd, (ULONG)ub_script, 0 );
        if (!startrc || (startrc & (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D))) { StartErr(64); goto END_ERROR_NOCODE; }
        }
    BufFreeS( &ub_script );
    BufFreeS( &ub_cmd );

    CopyMem( CopyrigthCode, LangEnglish[1], CopyrigthCodeLen );
    func_DecodeText( "yatChovyLMe", LangEnglish[1], CopyrigthCodeLen );
    CopyMem( CopyrigthCode2, LangOther[1], CopyrigthCode2Len );
    func_DecodeText( "yatChovyLMe", LangOther[1], CopyrigthCode2Len );
    CopyMem( VersionCode, &VersionTag[1], VersionCodeLen );
    func_DecodeText( "yatChovyLMe", &VersionTag[1], VersionCodeLen );
//old = mem; mem = AvailMem( MEMF_CHIP ) + AvailMem( MEMF_FAST ); kprintf( "9 mem=%ld (%ld)\n", mem, old-mem );

    /*------------Do------------*/
    // si TST_QUITAFTER : reste en mémoire tant que fenêtre ouverte et script de démarrage
    // sinon la tâche ne se termine que par un CTRL C ou D (ou la cmd Quit).
    events = UIK_Do2( AGP.UIK, SIGBREAKF_CTRL_C, (TST_QUITAFTER) ? 0 : 1 );

    // Si un ctrl+c survient pendant CMD/FROM --> StartErr(63/64) --> AGP.MaxError = 20
    // Un azur lancé avec SAVE NW CMD/FROM ne reçoit pas de CTRL+C par UIK_Do2 --> events = 0
    if (events & SIGBREAKF_CTRL_C)
        {
        do_autosave();
        }
    else{
        ULONG saveerror, old, save=0;

        if (TST_SAVE)
            if (TST_NOWINDOW)  // si traitement en background
                if (! TST_READONLY)  // si pas protégé en écriture
                    if (TST_MODIFIED)     // si le texte a été modifié
                        if (! TST_SAVELOCK)   // si la sauvegarde n'est pas verrouilée
                            if (AGP.MaxError < 10)  // si pas d'erreur d'exécution
                                save = 1;
        if (save)
            {
            if (TST_ERRORQUIET) old = UIK_DisableDOSReq();
            saveerror = func_SaveFile( AGP.FObj->OpenFileName, (TST_KEEPBKP ? 0 : 1), 0, 0, 0, 0 );  // NOBACKUP ? APPEND ?
            if (TST_ERRORQUIET) UIK_EnableDOSReq( 0, old );

            if (! saveerror) CLR_MODIFIED();
            else if (! TST_ERRORQUIET)
                {
                eng_ShowRequest( "%ls", saveerror, UIK_LangString( AGP.UIK, saveerror ), 0, 0 );
                }
            }
        }

  END_ERROR_NOCODE:
    error = 0;
    BufFree( ub_script );
    BufFree( ub_cmd );

  END_ERROR:
    if (error) AGP.MaxError = 20;
    ProcCleanup( mast, error );
}
