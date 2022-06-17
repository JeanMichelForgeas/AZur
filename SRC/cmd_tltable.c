/****************************************************************
 *
 *      Project:   AZUR
 *      Function:  définitions des taglists d'objets
 *
 *      Created:   17/04/93 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "lci:azur.h"
#include "lci:eng_obj.h"


/****** Imported ************************************************/

extern struct TagItem __far CmdTL_kprintf[];
extern struct TagItem __far CmdTL_AbortARexx[];
extern struct TagItem __far CmdTL_Abbreviate[];
extern struct TagItem __far CmdTL_ActivateCmdLine[];
extern struct TagItem __far CmdTL_AddParm[];
extern struct TagItem __far CmdTL_Beep[];
extern struct TagItem __far CmdTL_Block2CB[];
extern struct TagItem __far CmdTL_Block2File[];
extern struct TagItem __far CmdTL_Block2Mac[];
extern struct TagItem __far CmdTL_Block[];
extern struct TagItem __far CmdTL_BMAdd[];
extern struct TagItem __far CmdTL_BMCursor[];
extern struct TagItem __far CmdTL_BMRemove[];
extern struct TagItem __far CmdTL_CB2File[];
extern struct TagItem __far CmdTL_CB2Win[];
extern struct TagItem __far CmdTL_CharConvert[];
extern struct TagItem __far CmdTL_CorrectWord[];
extern struct TagItem __far CmdTL_CorrectWordCase[];
extern struct TagItem __far CmdTL_CursLastChange[];
extern struct TagItem __far CmdTL_Cursor[];
extern struct TagItem __far CmdTL_CursorOld[];
extern struct TagItem __far CmdTL_Debug[];
extern struct TagItem __far CmdTL_Del[];
extern struct TagItem __far CmdTL_ExeARexxCmd[];
extern struct TagItem __far CmdTL_ExeARexxScript[];
extern struct TagItem __far CmdTL_ExeDOSCmd[];
extern struct TagItem __far CmdTL_ExeDOSScript[];
extern struct TagItem __far CmdTL_ExeLastCmd[];
extern struct TagItem __far CmdTL_File2CB[];
extern struct TagItem __far CmdTL_File2File[];
extern struct TagItem __far CmdTL_File2Mac[];
extern struct TagItem __far CmdTL_File2NewWin[];
extern struct TagItem __far CmdTL_File2Parms[];
extern struct TagItem __far CmdTL_File2Prefs[];
extern struct TagItem __far CmdTL_File2Win[];
extern struct TagItem __far CmdTL_Fold[];
extern struct TagItem __far CmdTL_GetBlock[];
extern struct TagItem __far CmdTL_GetBlockInfo[];
extern struct TagItem __far CmdTL_GetBMInfo[];
extern struct TagItem __far CmdTL_GetBMList[];
extern struct TagItem __far CmdTL_GetChar[];
extern struct TagItem __far CmdTL_GetCurrentDir[];
extern struct TagItem __far CmdTL_GetError[];
extern struct TagItem __far CmdTL_GetFileInfo[];
extern struct TagItem __far CmdTL_GetFilePath[];
extern struct TagItem __far CmdTL_GetFold[];
extern struct TagItem __far CmdTL_GetFoldInfo[];
extern struct TagItem __far CmdTL_GetFontInfo[];
extern struct TagItem __far CmdTL_GetLine[];
extern struct TagItem __far CmdTL_GetLineInfo[];
extern struct TagItem __far CmdTL_GetLockInfo[];
extern struct TagItem __far CmdTL_GetMasterPort[];
extern struct TagItem __far CmdTL_GetModified[];
extern struct TagItem __far CmdTL_GetNumBlocks[];
extern struct TagItem __far CmdTL_GetNumBM[];
extern struct TagItem __far CmdTL_GetNumFolds[];
extern struct TagItem __far CmdTL_GetNumLines[];
extern struct TagItem __far CmdTL_GetPort[];
extern struct TagItem __far CmdTL_GetPortList[];
extern struct TagItem __far CmdTL_GetPos[];
extern struct TagItem __far CmdTL_GetPrefs[];
extern struct TagItem __far CmdTL_GetPri[];
extern struct TagItem __far CmdTL_GetPubScreen[];
extern struct TagItem __far CmdTL_GetReadonly[];
extern struct TagItem __far CmdTL_GetResident[];
extern struct TagItem __far CmdTL_GetScreenList[];
extern struct TagItem __far CmdTL_GetScreenInfo[];
extern struct TagItem __far CmdTL_GetTaskInfo[];
extern struct TagItem __far CmdTL_GetText[];
extern struct TagItem __far CmdTL_GetVar[];
extern struct TagItem __far CmdTL_GetVarList[];
extern struct TagItem __far CmdTL_GetViewInfo[];
extern struct TagItem __far CmdTL_GetWinBorders[];
extern struct TagItem __far CmdTL_GetWinInfo[];
extern struct TagItem __far CmdTL_GetWinList[];
extern struct TagItem __far CmdTL_GetWord[];
extern struct TagItem __far CmdTL_Help[];
extern struct TagItem __far CmdTL_Illegal[];
extern struct TagItem __far CmdTL_InactivateCmdLine[];
extern struct TagItem __far CmdTL_Indent[];
extern struct TagItem __far CmdTL_LineClone[];
extern struct TagItem __far CmdTL_LineJoin[];
extern struct TagItem __far CmdTL_Load[];
extern struct TagItem __far CmdTL_Lock[];
extern struct TagItem __far CmdTL_Mac2File[];
extern struct TagItem __far CmdTL_Mac2Win[];
extern struct TagItem __far CmdTL_MacDelete[];
extern struct TagItem __far CmdTL_MacEnd[];
extern struct TagItem __far CmdTL_MacPlay[];
extern struct TagItem __far CmdTL_MacRecord[];
extern struct TagItem __far CmdTL_Match[];
extern struct TagItem __far CmdTL_NOp[];
extern struct TagItem __far CmdTL_Prefs2File[];
extern struct TagItem __far CmdTL_Qualifier[];
extern struct TagItem __far CmdTL_ReDo[];
extern struct TagItem __far CmdTL_RemakeScreen[];
extern struct TagItem __far CmdTL_Replace[];
extern struct TagItem __far CmdTL_ReqClose[];
extern struct TagItem __far CmdTL_ReqFile[];
extern struct TagItem __far CmdTL_ReqList[];
extern struct TagItem __far CmdTL_ReqMulti[];
extern struct TagItem __far CmdTL_ReqOpen[];
extern struct TagItem __far CmdTL_RexxNameObtain[];
extern struct TagItem __far CmdTL_RexxNameRelease[];
extern struct TagItem __far CmdTL_RexxNameClear[];
extern struct TagItem __far CmdTL_Screen2Back[];
extern struct TagItem __far CmdTL_Screen2Front[];
extern struct TagItem __far CmdTL_Search[];
extern struct TagItem __far CmdTL_SetModified[];
extern struct TagItem __far CmdTL_SetCompress[];
extern struct TagItem __far CmdTL_SetCurrentDir[];
extern struct TagItem __far CmdTL_SetResident[];
extern struct TagItem __far CmdTL_SetDebug[];
extern struct TagItem __far CmdTL_SetFilePath[];
extern struct TagItem __far CmdTL_SetPrefs[];
extern struct TagItem __far CmdTL_SetPri[];
extern struct TagItem __far CmdTL_SetQuoteMode[];
extern struct TagItem __far CmdTL_SetStatusBar[];
extern struct TagItem __far CmdTL_SetVar[];
extern struct TagItem __far CmdTL_SetWriteReplace[];
extern struct TagItem __far CmdTL_Space2Tab[];
extern struct TagItem __far CmdTL_Sort[];
extern struct TagItem __far CmdTL_Surround[];
extern struct TagItem __far CmdTL_Tab2Space[];
extern struct TagItem __far CmdTL_Template[];
extern struct TagItem __far CmdTL_Text2CB[];
extern struct TagItem __far CmdTL_Text2File[];
extern struct TagItem __far CmdTL_Text2Win[];
extern struct TagItem __far CmdTL_TextFormat[];
extern struct TagItem __far CmdTL_TextJustify[];
extern struct TagItem __far CmdTL_Undo[];
extern struct TagItem __far CmdTL_Unload[];
extern struct TagItem __far CmdTL_Version[];
extern struct TagItem __far CmdTL_ViewActivate[];
extern struct TagItem __far CmdTL_ViewCenter[];
extern struct TagItem __far CmdTL_ViewDown[];
extern struct TagItem __far CmdTL_ViewLeft[];
extern struct TagItem __far CmdTL_ViewRight[];
extern struct TagItem __far CmdTL_ViewScroll[];
extern struct TagItem __far CmdTL_ViewSize[];
extern struct TagItem __far CmdTL_ViewSplit[];
extern struct TagItem __far CmdTL_ViewSwitch[];
extern struct TagItem __far CmdTL_ViewUp[];
extern struct TagItem __far CmdTL_ViewUpdate[];
extern struct TagItem __far CmdTL_Win2Back[];
extern struct TagItem __far CmdTL_Win2CB[];
extern struct TagItem __far CmdTL_Win2File[];
extern struct TagItem __far CmdTL_Win2Front[];
extern struct TagItem __far CmdTL_WinChange[];
extern struct TagItem __far CmdTL_WinClear[];
extern struct TagItem __far CmdTL_WinClose[];
extern struct TagItem __far CmdTL_WinAct[];
extern struct TagItem __far CmdTL_WinIconify[];
extern struct TagItem __far CmdTL_WinLast[];
extern struct TagItem __far CmdTL_WinNext[];
extern struct TagItem __far CmdTL_WinPrev[];
extern struct TagItem __far CmdTL_WinZoom[];


/****** Exported ***********************************************/

struct TagItem * __far GermTable[] =
    {
    CmdTL_kprintf,
    CmdTL_AbortARexx,
    CmdTL_Abbreviate,
    CmdTL_ActivateCmdLine,
    CmdTL_AddParm,
    CmdTL_Beep,
    CmdTL_Block2CB,
    CmdTL_Block2File,
    CmdTL_Block2Mac,
    CmdTL_Block,
    CmdTL_BMAdd,
    CmdTL_BMCursor,
    CmdTL_BMRemove,
    CmdTL_CB2File,
    CmdTL_CB2Win,
    CmdTL_CharConvert,
    CmdTL_CorrectWord,
    CmdTL_CorrectWordCase,
    CmdTL_CursLastChange,
    CmdTL_Cursor,
    CmdTL_CursorOld,
    CmdTL_Debug,
    CmdTL_Del,
    CmdTL_ExeARexxCmd,
    CmdTL_ExeARexxScript,
    CmdTL_ExeDOSCmd,
    CmdTL_ExeDOSScript,
    CmdTL_ExeLastCmd,
    CmdTL_File2CB,
    CmdTL_File2File,
    CmdTL_File2Mac,
    CmdTL_File2NewWin,
    CmdTL_File2Parms,
    CmdTL_File2Prefs,
    CmdTL_File2Win,
    CmdTL_Fold,
    CmdTL_GetBlock,
    CmdTL_GetBlockInfo,
    CmdTL_GetBMInfo,
    CmdTL_GetBMList,
    CmdTL_GetChar,
    CmdTL_GetCurrentDir,
    CmdTL_GetError,
    CmdTL_GetFileInfo,
    CmdTL_GetFilePath,
    CmdTL_GetFold,
    CmdTL_GetFoldInfo,
    CmdTL_GetFontInfo,
    CmdTL_GetLine,
    CmdTL_GetLineInfo,
    CmdTL_GetLockInfo,
    CmdTL_GetMasterPort,
    CmdTL_GetModified,
    CmdTL_GetNumBlocks,
    CmdTL_GetNumBM,
    CmdTL_GetNumFolds,
    CmdTL_GetNumLines,
    CmdTL_GetPort,
    CmdTL_GetPortList,
    CmdTL_GetPos,
    CmdTL_GetPrefs,
    CmdTL_GetPri,
    CmdTL_GetPubScreen,
    CmdTL_GetReadonly,
    CmdTL_GetResident,
    CmdTL_GetScreenList,
    CmdTL_GetScreenInfo,
    CmdTL_GetTaskInfo,
    CmdTL_GetText,
    CmdTL_GetVar,
    CmdTL_GetVarList,
    CmdTL_GetViewInfo,
    CmdTL_GetWinBorders,
    CmdTL_GetWinInfo,
    CmdTL_GetWinList,
    CmdTL_GetWord,
    CmdTL_Help,
    CmdTL_Illegal,
    CmdTL_InactivateCmdLine,
    CmdTL_Indent,
    CmdTL_LineClone,
    CmdTL_LineJoin,
    CmdTL_Load,
    CmdTL_Lock,
    CmdTL_Mac2File,
    CmdTL_Mac2Win,
    CmdTL_MacDelete,
    CmdTL_MacEnd,
    CmdTL_MacPlay,
    CmdTL_MacRecord,
    CmdTL_Match,
    CmdTL_NOp,
    CmdTL_Prefs2File,
    CmdTL_Qualifier,
    CmdTL_ReDo,
    CmdTL_RemakeScreen,
    CmdTL_Replace,
    CmdTL_ReqClose,
    CmdTL_ReqFile,
    CmdTL_ReqList,
    CmdTL_ReqMulti,
    CmdTL_ReqOpen,
    CmdTL_RexxNameObtain,
    CmdTL_RexxNameRelease,
    CmdTL_RexxNameClear,
    CmdTL_Screen2Back,
    CmdTL_Screen2Front,
    CmdTL_Search,
    CmdTL_SetModified,
    CmdTL_SetCompress,
    CmdTL_SetCurrentDir,
    CmdTL_SetResident,
    CmdTL_SetDebug,
    CmdTL_SetFilePath,
    CmdTL_SetPrefs,
    CmdTL_SetPri,
    CmdTL_SetQuoteMode,
    CmdTL_SetStatusBar,
    CmdTL_SetVar,
    CmdTL_SetWriteReplace,
    CmdTL_Space2Tab,
    CmdTL_Sort,
    CmdTL_Surround,
    CmdTL_Tab2Space,
    CmdTL_Template,
    CmdTL_Text2CB,
    CmdTL_Text2File,
    CmdTL_Text2Win,
    CmdTL_TextFormat,
    CmdTL_TextJustify,
    CmdTL_Undo,
    CmdTL_Unload,
    CmdTL_Version,
    CmdTL_ViewActivate,
    CmdTL_ViewCenter,
    CmdTL_ViewDown,
    CmdTL_ViewLeft,
    CmdTL_ViewRight,
    CmdTL_ViewScroll,
    CmdTL_ViewSize,
    CmdTL_ViewSplit,
    CmdTL_ViewSwitch,
    CmdTL_ViewUp,
    CmdTL_ViewUpdate,
    CmdTL_Win2Back,
    CmdTL_Win2CB,
    CmdTL_Win2File,
    CmdTL_Win2Front,
    CmdTL_WinChange,
    CmdTL_WinClear,
    CmdTL_WinClose,
    CmdTL_WinAct,
    CmdTL_WinIconify,
    CmdTL_WinLast,
    CmdTL_WinNext,
    CmdTL_WinPrev,
    CmdTL_WinZoom,
    0
    };
