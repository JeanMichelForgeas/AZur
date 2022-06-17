/****************************************************************
 *
 *      Project:   AZUR
 *      Function:  objets
 *
 *      Created:   08/11/94 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"
#include "uiki:objarexxsim.h"
#include "uiki:objwinfilesel.h"

#include "lci:objmenu.h"
#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:objfile.h"
#include "lci:objzone.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include <rexx/storage.h>
#include <rexx/errors.h>

#include <stddef.h>
#define OF(n) (offsetof(struct AZurPrefs,n))
#define OFO(n) (offsetof(struct AZurPrefs,FDI)+offsetof(struct FoldDrawInfo,n))

struct defprefs {
    UBYTE   *Name;
    ULONG   (*SetFunc)();
    ULONG   (*GetFunc)();
    ULONG   Offs;
    ULONG   Len;
    LONG    Min, Max;
    };


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/

static ULONG set_num( UBYTE *ptr, struct defprefs *dp );
static ULONG set_string( STRPTR ptr, struct defprefs *dp );
static ULONG set_flag( UBYTE *ptr, struct defprefs *dp );
static ULONG set_left( STRPTR ptr, struct defprefs *dp );
static ULONG set_right( STRPTR ptr, struct defprefs *dp );

static ULONG get_num( struct defprefs *dp );
static ULONG get_string( struct defprefs *dp );
static ULONG get_flag( struct defprefs *dp );

static struct defprefs __far _defprefs[] = {

    //--------- Values

    { "FN=FontName",            set_string,    get_string,    OF(FontName),           32, 0, 0 },
    { "VFN=ViewFontName",       set_string,    get_string,    OF(ViewFontName),       32, 0, 0 },
    { "Pat=PatOpen",            set_string,    get_string,    OF(PatOpen),            64, 0, 0 },
    { "Parm=ParmFile",          set_string,    get_string,    OF(ParmFile),           64, 0, 0 },
    { "MPN=MastPortName",       set_string,    get_string,    OF(MastPortName),       16, 0, 0 },
    { "PN=PortName",            set_string,    get_string,    OF(PortName),           16, 0, 0 },
    { "RCon=ARexxConsoleDesc",  set_string,    get_string,    OF(ARexxConsoleDesc),   96, 0, 0 },
    { "DCon=DOSConsoleDesc",    set_string,    get_string,    OF(DOSConsoleDesc),     96, 0, 0 },
    { "IT=IconTool",            set_string,    get_string,    OF(IconTool),           64, 0, 0 },
    { "BkpDir=BackupDirName",   set_string,    get_string,    OF(BackupDirName),      64, 0, 0 },
    { "BkpName=BackupName",     set_string,    get_string,    OF(BackupName),         16, 0, 0 },
    { "AutDir=AutsDirName",     set_string,    get_string,    OF(AutsDirName),        64, 0, 0 },
    { "AutName=AutsName",       set_string,    get_string,    OF(AutsName),           16, 0, 0 },
    { "AIN=AppIconName",        set_string,    get_string,    OF(AppIconName),        32, 0, 0 },
    { "FH=FontHeight",          set_num,       get_num,       OF(FontHeight),          1, 1, 255 },
    { "VFH=ViewFontHeight",     set_num,       get_num,       OF(ViewFontHeight),      1, 1, 255 },
    { "BND=BlinkOnDelay",       set_num,       get_num,       OF(BlinkOnDelay),        1, 0, 255 },
    { "BFD=BlinkOffDelay",      set_num,       get_num,       OF(BlinkOffDelay),       1, 0, 255 },
    { "TPen=TextPen",           set_num,       get_num,       OF(TextPen),             1, 0, 255 },
    { "BPen=BackPen",           set_num,       get_num,       OF(BackPen),             1, 0, 255 },
    { "BTPen=BlockTPen",        set_num,       get_num,       OF(BlockTPen),           1, 0, 255 },
    { "BBPen=BlockBPen",        set_num,       get_num,       OF(BlockBPen),           1, 0, 255 },
    { "CTPen=CursTextPen",      set_num,       get_num,       OF(CursTextPen),         1, 0, 255 },
    { "CBPen=CursBackPen",      set_num,       get_num,       OF(CursBackPen),         1, 0, 255 },
    { "CBTPen=CursBlockTPen",   set_num,       get_num,       OF(CursBlockTPen),       1, 0, 255 },
    { "CBBPen=CursBlockBPen",   set_num,       get_num,       OF(CursBlockBPen),       1, 0, 255 },
    { "IO=IOBufLength",         set_num,       get_num,       OF(IOBufLength),         4, 0, MAXSIGNED },
    { "Ind=IndentType",         set_num,       get_num,       OF(IndentType),          1, 0, 255 },
    { "Tab=TabLen",             set_num,       get_num,       OF(TabLen),              1, 0, 255 },
    { "Pud=PuddleSize",         set_num,       get_num,       OF(PuddleSize),          1, 0, 255 },
    { "Del=DelClip",            set_num,       get_num,       OF(DelClip),             1, 0, 255 },
    { "SCL=ScreenLeft",         set_num,       get_num,       OF(ScreenLeft),          2, 0x8000, 0x7fff },
    { "SCT=ScreenTop",          set_num,       get_num,       OF(ScreenTop),           2, 0x8000, 0x7fff },
    { "SCW=ScreenWidth",        set_num,       get_num,       OF(ScreenWidth),         2, 1, 0xffff },
    { "SCH=ScreenHeight",       set_num,       get_num,       OF(ScreenHeight),        2, 1, 0xffff },
    { "SCM=ScreenModeId",       set_num,       get_num,       OF(ScreenModeId),        4, 0, MAXSIGNED },
    { "SCD=ScreenDepth",        set_num,       get_num,       OF(ScreenDepth),         1, 1, 24 },
    { "SCO=ScreenOverscan",     set_num,       get_num,       OF(ScreenOverscan),      1, 0, 4 },
    { "BLM=BlockMode",          set_num,       get_num,       OF(BlockMode),           1, 0, 5 },
    { "BLX=BlockXLim",          set_num,       get_num,       OF(BlockXLim),           1, 0, 255 },
    { "BLY=BlockYLim",          set_num,       get_num,       OF(BlockYLim),           1, 0, 255 },
    { "Hist=HistCmdLineMax",    set_num,       get_num,       OF(HistCmdLineMax),      2, 1, 0xffff },
    { "CSp=CharSpace",          set_num,       get_num,       OF(CharSpace),           1, 0, 255 },
    { "CEOL=CharEOL",           set_num,       get_num,       OF(CharEOL),             1, 0, 255 },
    { "CEOF=CharEOF",           set_num,       get_num,       OF(CharEOF),             1, 0, 255 },
    { "CTabS=CharTabS",         set_num,       get_num,       OF(CharTabS),            1, 0, 255 },
    { "CTabE=CharTabE",         set_num,       get_num,       OF(CharTabE),            1, 0, 255 },
    { "LM=LeftMargin",          set_left,      get_num,       OF(LeftMargin),          4, 0, 0 },
    { "RM=RightMargin",         set_right,     get_num,       OF(RightMargin),         4, 0, 0 },
    { "UN=UndoMaxNum",          set_num,       get_num,       OF(UndoMaxNum),          4, 0, MAXSIGNED },
    { "US=UndoMaxSize",         set_num,       get_num,       OF(UndoMaxSize),         4, 0, MAXSIGNED },
    { "CRD=CharRexxDot",        set_num,       get_num,       OF(CharRexxDot),         1, 0, 255 },
    { "Pri=Priority",           set_num,       get_num,       OF(Priority),            1, -128, 127 },
    { "AutSec=AutsIntSec",      set_num,       get_num,       OF(AutsIntSec),          2, 0, 0xffff },
    { "AutMod=AutsIntMod",      set_num,       get_num,       OF(AutsIntMod),          2, 0, 0xffff },
    { "BkpRot=BackupRot",       set_num,       get_num,       OF(BackupRot),           1, 0, 255 },
    { "FAPen=FoldOnAPen",       set_num,       get_num,       OFO(OnAPen),             1, 0, 255 },
    { "FBPen=FoldOnBPen",       set_num,       get_num,       OFO(OnBPen),             1, 0, 255 },
    { "FFAPen=FoldOffAPen",     set_num,       get_num,       OFO(OffAPen),            1, 0, 255 },
    { "FFBPen=FoldOffBPen",     set_num,       get_num,       OFO(OffBPen),            1, 0, 255 },

    //-------- Flags

    { "ABD=AutoBlockDel",   set_flag, get_flag, OF(Flags1), 4, AZP_CHARDELBLOCK, 0 },
    { "UBN=UseBkpName",     set_flag, get_flag, OF(Flags1), 4, AZP_BKPFILENAME , 0 },
    { "EQ=ErrorQuiet",      set_flag, get_flag, OF(Flags1), 4, AZP_ERRORQUIET , 0 },
    { "SI=SaveIcon",        set_flag, get_flag, OF(Flags1), 4, AZP_SAVEICON , 0 },
    { "Bkp=DoBackup",       set_flag, get_flag, OF(Flags1), 4, AZP_KEEPBKP , 0 },
    { "TR=TextReplace",     set_flag, get_flag, OF(Flags1), 4, AZP_OVERLAY , 0 },
    { "BLM=BlockLineMode",  set_flag, get_flag, OF(Flags1), 4, AZP_DEFAULTBLM , 0 },
    { "AS=DoAutoSave",      set_flag, get_flag, OF(Flags1), 4, AZP_AUTOSAVE , 0 },
    { "RO=ReadOnly",        set_flag, get_flag, OF(Flags1), 4, AZP_READONLY , 0 },
    { "Minitel",            set_flag, get_flag, OF(Flags1), 4, AZP_MINITEL , 0 },
    { "RW=ReqWin",          set_flag, get_flag, OF(Flags1), 4, AZP_REQWIN , 0 },
    { "FS=FullScreen",      set_flag, get_flag, OF(Flags1), 4, AZP_FULLSCREEN , 0 },
    { "NP=NullPointer",     set_flag, get_flag, OF(Flags1), 4, AZP_NULLPOINTER , 0 },
    { "RSep=RexxNameSep",   set_flag, get_flag, OF(Flags1), 4, AZP_REXXNAMESEP , 0 },
    { "RNum=RexxNameNum",   set_flag, get_flag, OF(Flags1), 4, AZP_REXXNAMENUM , 0 },

    { "T2S=Tab2Space",          set_flag, get_flag, OF(Flags2), 4, AZP_TAB2SPACE , 0 },
    { "S2T=Space2Tab",          set_flag, get_flag, OF(Flags2), 4, AZP_SPACE2TAB , 0 },
    { "Blink",                  set_flag, get_flag, OF(Flags2), 4, AZP_BLINKON , 0 },
    { "EOL=ShowEOL",            set_flag, get_flag, OF(Flags2), 4, AZP_SHOWEOL , 0 },
    { "EOF=ShowEOF",            set_flag, get_flag, OF(Flags2), 4, AZP_SHOWEOF , 0 },
    { "SC=StripChange",         set_flag, get_flag, OF(Flags2), 4, AZP_STRIPCHANGE , 0 },
    { "SS=StripSave",           set_flag, get_flag, OF(Flags2), 4, AZP_STRIPSAVE , 0 },
    { "SL=StripLoad",           set_flag, get_flag, OF(Flags2), 4, AZP_STRIPLOAD , 0 },
    { "LCR=LoadCR2LF",          set_flag, get_flag, OF(Flags2), 4, AZP_LOADCR2LF , 0 },
    { "LCRLF=LoadCRLF2LF",      set_flag, get_flag, OF(Flags2), 4, AZP_LOADCRLF2LF , 0 },
    { "SCR=SaveLF2CR",          set_flag, get_flag, OF(Flags2), 4, AZP_SAVELF2CR , 0 },
    { "SCRLF=SaveLF2CRLF",      set_flag, get_flag, OF(Flags2), 4, AZP_SAVELF2CRLF , 0 },
    { "AF=AutoFormat",          set_flag, get_flag, OF(Flags2), 4, AZP_AUTOFORMAT , 0 },
    { "UM=UseMargins=UseMargin",set_flag, get_flag, OF(Flags2), 4, AZP_USEMARGIN   , 0 },
    { "LI=LoadIcon",            set_flag, get_flag, OF(Flags2), 4, AZP_LOADICON    , 0 },
    { "LRC=LeaveRexxCase",      set_flag, get_flag, OF(Flags2), 4, AZP_LEAVEREXCASE, 0 },
    { "RAS=ReqAutoSave",        set_flag, get_flag, OF(Flags2), 4, AZP_REQAUTOSAVE, 0 },
    { "AI=AppIcon",             set_flag, get_flag, OF(Flags2), 4, AZP_APPICON, 0 },
    /*
    { "",         set_flag, get_flag, OF(Flags2), 4, AZP_REALTAB     , 0 },
    { "",         set_flag, get_flag, OF(Flags2), 4, AZP_TABDISPLAYS , 0 },
    { "",         set_flag, get_flag, OF(Flags2), 4, AZP_TABDISPLAYE , 0 },
    { "",         set_flag, get_flag, OF(Flags2), 4, AZP_SPACEDISPLAY, 0 },
    { "",         set_flag, get_flag, OF(Flags2), 4, AZP_CORRECTCASE , 0 },
    { "",         set_flag, get_flag, OF(Flags2), 4, AZP_NUMBERLINES , 0 },
    */

    { "SeCase",                             set_flag, get_flag, OF(SearchFlags), 4, SEF_CASE , 0 },
    { "SeAcc=SeAccent",                     set_flag, get_flag, OF(SearchFlags), 4, SEF_ACCENT , 0 },
    { "SeAll",                              set_flag, get_flag, OF(SearchFlags), 4, SEF_ALL , 0 },
    { "SeBack=SeBackward=SePrev=SePrevious",set_flag, get_flag, OF(SearchFlags), 4, SEF_BACKWARD   , 0 },
    { "SeIB=SeIntoBlock",                   set_flag, get_flag, OF(SearchFlags), 4, SEF_BLOCK , 0 },
    { "SeTB=SeToBlocks",                    set_flag, get_flag, OF(SearchFlags), 4, SEF_TOBLOCK , 0 },
    { "SeConv=SeConvert",                   set_flag, get_flag, OF(SearchFlags), 4, SEF_CONVERT , 0 },
    { "SeFull",                             set_flag, get_flag, OF(SearchFlags), 4, SEF_FULL , 0 },
    { "SeSOL",                              set_flag, get_flag, OF(SearchFlags), 4, SEF_STARTOFLINE, 0 },
    { "SeEOL",                              set_flag, get_flag, OF(SearchFlags), 4, SEF_ENDOFLINE , 0 },
    { "SeNV=SeNoverify",                    set_flag, get_flag, OF(SearchFlags), 4, SEF_NOVERIFY , 0 },
    { "SeSOW",                              set_flag, get_flag, OF(SearchFlags), 4, SEF_STARTOFWORD, 0 },
    { "SeEOW",                              set_flag, get_flag, OF(SearchFlags), 4, SEF_ENDOFWORD , 0 },

    { "FI=FoldOnItalic",        set_flag, get_flag, OFO(OnFlags),  1, FSF_ITALIC, 0 },
    { "FU=FoldOnUnderline",     set_flag, get_flag, OFO(OnFlags),  1, FSF_UNDERLINED, 0 },
    { "FB=FoldOnBold",          set_flag, get_flag, OFO(OnFlags),  1, FSF_BOLD, 0 },
    { "FFI=FoldOffItalic",      set_flag, get_flag, OFO(OffFlags), 1, FSF_ITALIC, 0 },
    { "FFU=FoldOffUnderline",   set_flag, get_flag, OFO(OffFlags), 1, FSF_UNDERLINED, 0 },
    { "FFB=FoldOffBold",        set_flag, get_flag, OFO(OffFlags), 1, FSF_BOLD, 0 },

    { 0, 0 }
};

/****************************************************************
 *
 *      Objets
 *
 ****************************************************************/

//»»»»»» Cmd_GetPrefs       ATTR/A/K/F

static LONG get_sized_val( struct defprefs *dp )
{
  LONG val;

    switch (dp->Len)
        {
        case 1: val = (ULONG) *((UBYTE*)((ULONG)AGP.Prefs + dp->Offs)); break;
        case 2: val = (ULONG) *((UWORD*)((ULONG)AGP.Prefs + dp->Offs)); break;
        case 4: val = (ULONG) *((ULONG*)((ULONG)AGP.Prefs + dp->Offs)); break;
        default: val = 0; break;
        }
    return( val );
}

static ULONG get_num( struct defprefs *dp )
{
  LONG val = get_sized_val( dp );

    if (func_BufPrintf( &AGP.RexxResult, BufLength(AGP.RexxResult), "%ld ", val )) return(0);
    else return( TEXT_NOMEMORY );
}

static ULONG get_string( struct defprefs *dp )
{
    if (func_BufPrintf( &AGP.RexxResult, BufLength(AGP.RexxResult), "\"%ls\" ", ((ULONG)AGP.Prefs + dp->Offs) )) return(0);
    else return( TEXT_NOMEMORY );
}

static ULONG get_flag( struct defprefs *dp )
{
  LONG flags = get_sized_val( dp );

    if (func_BufPrintf( &AGP.RexxResult, BufLength(AGP.RexxResult), "%ls ", (flags & dp->Min) ? "ON" : "OFF" )) return(0);
    else return( TEXT_NOMEMORY );
}

ULONG GetSet_OnePref( STRPTR attr, ULONG val, ULONG doset ) // func_fileio
{
  struct defprefs *tab;
  ULONG errmsg=0;

    for (tab=_defprefs; tab->Name; tab++) { if (ARG_MatchArg( tab->Name, attr ) != -1) break; }
    if (tab->Name == 0) { errmsg = 15; goto END_ERROR; }

    if (doset) { if (tab->SetFunc) { if (errmsg = tab->SetFunc( val, tab )) goto END_ERROR; } }
    else       { if (tab->GetFunc) { if (errmsg = tab->GetFunc( tab )) goto END_ERROR; } }

  END_ERROR:
    return( errmsg );
}

ULONG GetSetPrefs( STRPTR attrs, ULONG doset, ULONG display ) // func_fileio
{
  struct ParseArgs PArg;
  APTR *Parse=0;
  UBYTE buf[200], *mem=0, *res[3], *template;
  ULONG len, errmsg=0, dosbase=(ULONG)DOSBase;

    if (doset) template = "TAG/A,DATA/A,ATTR/K/F"; else template = "TAG/A,ATTR/K/F";

    if (! (mem = BufAllocP( AGP.Pool, StrLen(attrs), 10 ))) { errmsg = TEXT_NOMEMORY; goto END_ERROR; }
    StrCpy( mem, attrs );
    if (doset) CopyMem( AGP.Prefs, AGP.TmpPrefs, sizeof(struct AZurPrefs) );

    while (*mem)
        {
        __builtin_memset( &PArg, 0, sizeof(struct ParseArgs) );
        res[0] = res[1] = res[2] = 0;

        PArg.CmdLine = mem;
        PArg.CmdLength = StrLen( mem );
        PArg.Buffer = buf;
        PArg.BufSize = 200;
        PArg.PoolHeader = AGP.Pool;
        PArg.Flags = PAF_POOLED|PAF_ACCEPTEOL;
        if (Parse = ARG_Parse( template, res, &PArg, 0, dosbase ))
            {
            if (errmsg = GetSet_OnePref( (STRPTR)res[0], (ULONG)res[1], doset )) goto END_ERROR;

            if (doset) { if (res[2]) StrCpy( mem, res[2] ); else *mem = 0; }
            else       { if (res[1]) StrCpy( mem, res[1] ); else *mem = 0; }

            ARG_FreeParse( Parse );
            }
        if (PArg.Error) { errmsg = TEXT_PARSE_ERRORBASE + PArg.Error; goto END_ERROR; }
        }

  END_ERROR:
    BufFree( mem );
    if (errmsg)
        {
        if (doset) CopyMem( AGP.TmpPrefs, AGP.Prefs, sizeof(struct AZurPrefs) );
        }
    else{
        if (doset) ActualizePrefs( display );
        else{
            len = BufLength( AGP.RexxResult );
            if (len && (AGP.RexxResult[len-1] == ' ')) BufTruncate( AGP.RexxResult, len - 1 );
            }
        }
    return( errmsg );
}

void CmdFunc_GetPrefs_Do( struct CmdObj *co )
{
  ULONG errmsg;

    if (errmsg = GetSetPrefs( co->ArgRes[0], 0, 1 ))
        intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_SetPrefs       ATTR/A/K/F

static ULONG set_num( UBYTE *ptr, struct defprefs *dp )
{
  LONG val;

    if (ARG_StrToLong( ptr, &val ) < 0) goto END_ERROR;
    if (val < dp->Min || val > dp->Max) goto END_ERROR;
    switch (dp->Len)
        {
        case 1: *((UBYTE*)((ULONG)AGP.Prefs + dp->Offs)) = (UBYTE)val; break;
        case 2: *((UWORD*)((ULONG)AGP.Prefs + dp->Offs)) = (UWORD)val; break;
        case 4: *((ULONG*)((ULONG)AGP.Prefs + dp->Offs)) = (ULONG)val; break;
        default: break;
        }
    return(0);
  END_ERROR:
    return( TEXT_BADNUMBER );
}

static ULONG set_string( STRPTR ptr, struct defprefs *dp )
{
    StcCpy( (STRPTR)((ULONG)AGP.Prefs + dp->Offs), ptr, dp->Len );
    return(0);
}

static ULONG set_flag( UBYTE *ptr, struct defprefs *dp )
{
  ULONG flags = (ULONG) get_sized_val( dp );

    if (StrNCCmp( ptr, "ON" ) == 0) flags |= dp->Min;
    else if (StrNCCmp( ptr, "OFF" ) == 0) flags &= ~dp->Min;
    else if (StrNCCmp( ptr, "TOGGLE" ) == 0) { if (flags & dp->Min) flags &= ~dp->Min; else flags |= dp->Min; }
    else return( TEXT_BADARG );

    switch (dp->Len)
        {
        case 1: *((UBYTE*)((ULONG)AGP.Prefs + dp->Offs)) = (UBYTE)flags; break;
        case 2: *((UWORD*)((ULONG)AGP.Prefs + dp->Offs)) = (UWORD)flags; break;
        case 4: *((ULONG*)((ULONG)AGP.Prefs + dp->Offs)) = (ULONG)flags; break;
        default: break;
        }
    return(0);
}

static ULONG set_left( STRPTR ptr, struct defprefs *dp )
{
  struct defprefs DP = *dp;

    DP.Min = 0;
    DP.Max = AGP.Prefs->RightMargin - 1;
    return( set_num( ptr, &DP ) );
}

static ULONG set_right( STRPTR ptr, struct defprefs *dp )
{
  struct defprefs DP = *dp;

    DP.Min = AGP.Prefs->LeftMargin + 1;
    DP.Max = MAXSIGNED;
    return( set_num( ptr, &DP ) );
}

/* TurboText names
    "AC=AREXXCONSOLENAME",
    "DC=DOSCONSOLENAME",
    "AUTOINDENT",
    "AUTOSAVE",
    "AUTOSAVECONFIRM",
    "AUTOSAVEDELAY",
    "BACKUPTEMPLATE",
    "CHANGESTRING",
    "CORRECTCASE",
    "FILEPATTERN",
    "FINDBACKWARD",
    "FINDHEX",
    "FINDIGNOREACCENTS",
    "FINDIGNORECASE",
    "FINDINFOLDS",
    "FINDSTRING",
    "FINDWHOLEWORDS",
    "FINDWILDCARD",
    "FOLDINDENT",
    "FREEFORM",
    "ICONTOOL",
    "MAKEBACKUPS",
    "MAXBACKUPVERSION",
    "OVERSTRIKE",
    "PAGELENGTH",
    "PRINTDEVICE",
    "SAVEICONS",
    "SCREENSIZE",
    "SCREENTYPE",
    "SHOWEOF",
    "SHOWEOLS",
    "SHOWPAGES",
    "SHOWSPACES",
    "SHOWTABS",
    "STRIPEOLBLANKS",
    "TABSGIVESPACES",
    "TABWIDTH",
    "TRANSLATECRS",
    "WINDOWPOS",
    "WINDOWSIZE",
    "WORDWRAP",
*/

void CmdFunc_SetPrefs_Do( struct CmdObj *co )
{
  ULONG errmsg;

    if (errmsg = GetSetPrefs( co->ArgRes[0], 1, 1 ))
        intAZ_SetCmdResult( co, errmsg );
}
