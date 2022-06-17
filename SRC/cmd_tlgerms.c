/****************************************************************
 *
 *      Project:   AZUR
 *      Function:  définition des objets
 *
 *      Created:   21/04/93 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

//#include "lci:objmenu.h"
#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:cmd_tlflags.h"
#include "lci:cmd_tlfuncs.h"
#include "cmd_tlnames.c"
#include "cmd_tltemplates.c"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/


/****************************************************************
 *
 *      Common
 *
 ****************************************************************/

/*
static void CmdFunc_Ghost( struct CmdObj *co, struct CmdFunc *cf, struct AZMenuEntry *pm )
{
    pm->Flags &= ~ITEMENABLED;
}
*/

/****************************************************************
 *
 *      Objets
 *
 ****************************************************************/

//»»»»»» Cmd_kprintf

static void CmdFunc_kprintf_Do( struct CmdObj *co )
{
    if (co->ArgRes[0]) kprintf( "%ls\n", co->ArgRes[0] );
}

struct TagItem __far CmdTL_kprintf[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_kprintf,
    AZT_Gen_Flags,          FLAGS_kprintf,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_kprintf,
    AZT_ActionDo,           CmdFunc_kprintf_Do,
    TAG_END,
    };

//»»»»»» Cmd_AbortARexx

struct TagItem __far CmdTL_AbortARexx[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_AbortARexx,
    AZT_Gen_Flags,          FLAGS_AbortARexx,
    AZT_ActionDo,           CmdFunc_AbortARexx_Do,
    TAG_END,
    };

//»»»»»» Cmd_Abbreviate

struct TagItem __far CmdTL_Abbreviate[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Abbreviate,
    AZT_Gen_Flags,          FLAGS_Abbreviate,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Abbreviate,
    AZT_ActionDo,           CmdFunc_Abbreviate_Do,
    TAG_END,
    };

//»»»»»» Cmd_ActivateCmdLine

struct TagItem __far CmdTL_ActivateCmdLine[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ActivateCmdLine,
    AZT_Gen_Flags,          FLAGS_ActivateCmdLine,
    AZT_ActionDo,           CmdFunc_ActivateCmdLine_Do,
    TAG_END,
    };

//»»»»»» Cmd_AddParm

static void CmdFunc_AddParm_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_AddParm[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_AddParm,
    AZT_Gen_Flags,          FLAGS_AddParm,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_AddParm,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_AddParm_Do,
    TAG_END,
    };

//»»»»»» Cmd_Beep

struct TagItem __far CmdTL_Beep[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Beep,
    AZT_Gen_Flags,          FLAGS_Beep,
    AZT_ActionDo,           CmdFunc_Beep_Do,
    TAG_END,
    };

//»»»»»» Cmd_Block

struct TagItem __far CmdTL_Block[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Block,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Block,
    AZT_Gen_Flags,          FLAGS_Block,
    AZT_ActionNew,          CmdFunc_Block_New,
    AZT_ActionDo,           CmdFunc_Block_Do,
    TAG_END,
    };

//»»»»»» Cmd_Block2CB

struct TagItem __far CmdTL_Block2CB[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Block2CB,
    AZT_Gen_Flags,          FLAGS_Block2CB,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Block2CB,
    AZT_ActionDo,           CmdFunc_Block2CB_Do,
    TAG_END,
    };

//»»»»»» Cmd_Block2File

struct TagItem __far CmdTL_Block2File[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Block2File,
    AZT_Gen_Flags,          FLAGS_Block2File,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Block2File,
    AZT_ActionDo,           CmdFunc_Block2File_Do,
    TAG_END,
    };

//»»»»»» Cmd_Block2Mac

struct TagItem __far CmdTL_Block2Mac[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Block2Mac,
    AZT_Gen_Flags,          FLAGS_Block2Mac,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Block2Mac,
    AZT_ActionDo,           CmdFunc_Block2Mac_Do,
    TAG_END,
    };

//»»»»»» Cmd_BMAdd

struct TagItem __far CmdTL_BMAdd[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_BMAdd,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_BMAdd,
    AZT_Gen_Flags,          FLAGS_BMAdd,
    AZT_ActionDo,           CmdFunc_BMAdd_Do,
    TAG_END,
    };

//»»»»»» Cmd_BMCursor

struct TagItem __far CmdTL_BMCursor[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_BMCursor,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_BMCursor,
    AZT_Gen_Flags,          FLAGS_BMCursor,
    AZT_ActionDo,           CmdFunc_BMCursor_Do,
    TAG_END,
    };

//»»»»»» Cmd_BMRemove

struct TagItem __far CmdTL_BMRemove[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_BMRemove,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_BMRemove,
    AZT_Gen_Flags,          FLAGS_BMRemove,
    AZT_ActionDo,           CmdFunc_BMRemove_Do,
    TAG_END,
    };

//»»»»»» Cmd_CB2File

struct TagItem __far CmdTL_CB2File[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_CB2File,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_CB2File,
    AZT_Gen_Flags,          FLAGS_CB2File,
    AZT_ActionDo,           CmdFunc_CB2File_Do,
    TAG_END,
    };

//»»»»»» Cmd_CB2Win

struct TagItem __far CmdTL_CB2Win[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_CB2Win,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_CB2Win,
    AZT_Gen_Flags,          FLAGS_CB2Win,
    AZT_ActionDo,           CmdFunc_CB2Win_Do,
    TAG_END,
    };

//»»»»»» Cmd_CharConvert

struct TagItem __far CmdTL_CharConvert[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_CharConvert,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_CharConvert,
    AZT_Gen_Flags,          FLAGS_CharConvert,
    AZT_ActionDo,           CmdFunc_CharConvert_Do,
    TAG_END,
    };

//»»»»»» Cmd_CorrectWord

static void CmdFunc_CorrectWord_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_CorrectWord[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_CorrectWord,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_CorrectWord,
    AZT_Gen_Flags,          FLAGS_CorrectWord,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_CorrectWord_Do,
    TAG_END,
    };

//»»»»»» Cmd_CorrectWordCase

static void CmdFunc_CorrectWordCase_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_CorrectWordCase[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_CorrectWordCase,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_CorrectWordCase,
    AZT_Gen_Flags,          FLAGS_CorrectWordCase,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_CorrectWordCase_Do,
    TAG_END,
    };

//»»»»»» Cmd_CursorOld

struct TagItem __far CmdTL_CursorOld[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_CursorOld,
    AZT_Gen_Flags,          FLAGS_CursorOld,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_CursorOld_Do,
    TAG_END,
    };

//»»»»»» Cmd_CursLastChange

struct TagItem __far CmdTL_CursLastChange[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_CursLastChange,
    AZT_Gen_Flags,          FLAGS_CursLastChange,
    AZT_ActionDo,           CmdFunc_CursLastChange_Do,
    TAG_END,
    };

//»»»»»» Cmd_Cursor

struct TagItem __far CmdTL_Cursor[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Cursor,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Cursor,
    AZT_Gen_Flags,          FLAGS_Cursor,
    AZT_ActionDo,           CmdFunc_Cursor_Do,
    TAG_END,
    };

//»»»»»» Cmd_Debug

struct TagItem __far CmdTL_Debug[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Debug,
    AZT_Gen_Flags,          FLAGS_Debug,
    AZT_ActionDo,           CmdFunc_Debug_Do,
    TAG_END,
    };

//»»»»»» Cmd_Del

struct TagItem __far CmdTL_Del[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Del,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Del,
    AZT_Gen_Flags,          FLAGS_Del,
    AZT_ActionDo,           CmdFunc_Del_Do,
    TAG_END,
    };

//»»»»»» Cmd_ExeARexxCmd

struct TagItem __far CmdTL_ExeARexxCmd[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ExeARexxCmd,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ExeARexxCmd,
    AZT_Gen_Flags,          FLAGS_ExeARexxCmd,
    AZT_ActionDo,           CmdFunc_ExeARexxCmd_Do,
    AZT_ActionSetMenu,      CmdFunc_ExeARexx_SetMenu,
    TAG_END,
    };

//»»»»»» Cmd_ExeARexxScript

struct TagItem __far CmdTL_ExeARexxScript[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ExeARexxScript,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ExeARexxScript,
    AZT_Gen_Flags,          FLAGS_ExeARexxScript,
    AZT_ActionDo,           CmdFunc_ExeARexxScript_Do,
    AZT_ActionSetMenu,      CmdFunc_ExeARexx_SetMenu,
    TAG_END,
    };

//»»»»»» Cmd_ExeDOSCmd

struct TagItem __far CmdTL_ExeDOSCmd[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ExeDOSCmd,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ExeDOSCmd,
    AZT_Gen_Flags,          FLAGS_ExeDOSCmd,
    AZT_ActionDo,           CmdFunc_ExeDOSCmd_Do,
    TAG_END,
    };

//»»»»»» Cmd_ExeDOSScript

struct TagItem __far CmdTL_ExeDOSScript[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ExeDOSScript,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ExeDOSScript,
    AZT_Gen_Flags,          FLAGS_ExeDOSScript,
    AZT_ActionDo,           CmdFunc_ExeDOSScript_Do,
    TAG_END,
    };

//»»»»»» Cmd_ExeLastCmd

struct TagItem __far CmdTL_ExeLastCmd[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ExeLastCmd,
    AZT_Gen_Flags,          FLAGS_ExeLastCmd,
    AZT_ActionDo,           CmdFunc_ExeLastCmd_Do,
    TAG_END,
    };

//»»»»»» Cmd_File2CB

struct TagItem __far CmdTL_File2CB[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_File2CB,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_File2CB,
    AZT_Gen_Flags,          FLAGS_File2CB,
    AZT_ActionDo,           CmdFunc_File2CB_Do,
    TAG_END,
    };

//»»»»»» Cmd_File2File

struct TagItem __far CmdTL_File2File[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_File2File,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_File2File,
    AZT_Gen_Flags,          FLAGS_File2File,
    AZT_ActionDo,           CmdFunc_File2File_Do,
    TAG_END,
    };

//»»»»»» Cmd_File2Mac

struct TagItem __far CmdTL_File2Mac[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_File2Mac,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_File2Mac,
    AZT_Gen_Flags,          FLAGS_File2Mac,
    AZT_ActionDo,           CmdFunc_File2Mac_Do,
    TAG_END,
    };

//»»»»»» Cmd_File2NewWin

struct TagItem __far CmdTL_File2NewWin[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_File2NewWin,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_File2NewWin,
    AZT_Gen_Flags,          FLAGS_File2NewWin,
    AZT_ActionDo,           CmdFunc_File2NewWin_Do,
    TAG_END,
    };

//»»»»»» Cmd_File2Parms

struct TagItem __far CmdTL_File2Parms[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_File2Parms,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_File2Parms,
    AZT_Gen_Flags,          FLAGS_File2Parms,
    AZT_ActionDo,           CmdFunc_File2Parms_Do,
    TAG_END,
    };

//»»»»»» Cmd_File2Win

struct TagItem __far CmdTL_File2Win[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_File2Win,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_File2Win,
    AZT_Gen_Flags,          FLAGS_File2Win,
    AZT_ActionDo,           CmdFunc_File2Win_Do,
    TAG_END,
    };

//»»»»»» Cmd_Fold

struct TagItem __far CmdTL_Fold[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Fold,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Fold,
    AZT_Gen_Flags,          FLAGS_Fold,
    AZT_ActionDo,           CmdFunc_Fold_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetBlock

struct TagItem __far CmdTL_GetBlock[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetBlock,
    AZT_Gen_Flags,          FLAGS_GetBlock,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetBlock,
    AZT_ActionDo,           CmdFunc_GetBlock_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetBlockInfo

struct TagItem __far CmdTL_GetBlockInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetBlockInfo,
    AZT_Gen_Flags,          FLAGS_GetBlockInfo,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetBlockInfo,
    AZT_ActionDo,           CmdFunc_GetBlockInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetBMInfo

struct TagItem __far CmdTL_GetBMInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetBMInfo,
    AZT_Gen_Flags,          FLAGS_GetBMInfo,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetBMInfo,
    AZT_ActionDo,           CmdFunc_GetBMInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetBMList

struct TagItem __far CmdTL_GetBMList[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetBMList,
    AZT_Gen_Flags,          FLAGS_GetBMList,
    AZT_ActionDo,           CmdFunc_GetBMList_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetChar

struct TagItem __far CmdTL_GetChar[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetChar,
    AZT_Gen_Flags,          FLAGS_GetChar,
    AZT_ActionDo,           CmdFunc_GetChar_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetCurrentDir

struct TagItem __far CmdTL_GetCurrentDir[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetCurrentDir,
    AZT_Gen_Flags,          FLAGS_GetCurrentDir,
    AZT_ActionDo,           CmdFunc_GetCurrentDir_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetError

struct TagItem __far CmdTL_GetError[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetError,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetError,
    AZT_Gen_Flags,          FLAGS_GetError,
    AZT_ActionDo,           CmdFunc_GetError_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetFileInfo

struct TagItem __far CmdTL_GetFileInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetFileInfo,
    AZT_Gen_Flags,          FLAGS_GetFileInfo,
    AZT_ActionDo,           CmdFunc_GetFileInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetFilePath

struct TagItem __far CmdTL_GetFilePath[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetFilePath,
    AZT_Gen_Flags,          FLAGS_GetFilePath,
    AZT_ActionDo,           CmdFunc_GetFilePath_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetFold

struct TagItem __far CmdTL_GetFold[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetFold,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetFold,
    AZT_Gen_Flags,          FLAGS_GetFold,
    AZT_ActionDo,           CmdFunc_GetFold_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetFoldInfo

struct TagItem __far CmdTL_GetFoldInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetFoldInfo,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetFoldInfo,
    AZT_Gen_Flags,          FLAGS_GetFoldInfo,
    AZT_ActionDo,           CmdFunc_GetFoldInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetFontInfo

struct TagItem __far CmdTL_GetFontInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetFontInfo,
    AZT_Gen_Flags,          FLAGS_GetFontInfo,
    AZT_ActionDo,           CmdFunc_GetFontInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetLine

struct TagItem __far CmdTL_GetLine[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetLine,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetLine,
    AZT_Gen_Flags,          FLAGS_GetLine,
    AZT_ActionDo,           CmdFunc_GetLine_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetLineInfo

struct TagItem __far CmdTL_GetLineInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetLineInfo,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetLineInfo,
    AZT_Gen_Flags,          FLAGS_GetLineInfo,
    AZT_ActionDo,           CmdFunc_GetLineInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetLockInfo

struct TagItem __far CmdTL_GetLockInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetLockInfo,
    AZT_Gen_Flags,          FLAGS_GetLockInfo,
    AZT_ActionDo,           CmdFunc_GetLockInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetMasterPort

struct TagItem __far CmdTL_GetMasterPort[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetMasterPort,
    AZT_Gen_Flags,          FLAGS_GetMasterPort,
    AZT_ActionDo,           CmdFunc_GetMasterPort_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetModified

struct TagItem __far CmdTL_GetModified[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetModified,
    AZT_Gen_Flags,          FLAGS_GetModified,
    AZT_ActionDo,           CmdFunc_GetModified_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetNumBlocks

struct TagItem __far CmdTL_GetNumBlocks[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetNumBlocks,
    AZT_Gen_Flags,          FLAGS_GetNumBlocks,
    AZT_ActionDo,           CmdFunc_GetNumBlocks_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetNumBM

struct TagItem __far CmdTL_GetNumBM[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetNumBM,
    AZT_Gen_Flags,          FLAGS_GetNumBM,
    AZT_ActionDo,           CmdFunc_GetNumBM_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetNumFolds

struct TagItem __far CmdTL_GetNumFolds[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetNumFolds,
    AZT_Gen_Flags,          FLAGS_GetNumFolds,
    AZT_ActionDo,           CmdFunc_GetNumFolds_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetNumLines

struct TagItem __far CmdTL_GetNumLines[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetNumLines,
    AZT_Gen_Flags,          FLAGS_GetNumLines,
    AZT_ActionDo,           CmdFunc_GetNumLines_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetPort

struct TagItem __far CmdTL_GetPort[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetPort,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetPort,
    AZT_Gen_Flags,          FLAGS_GetPort,
    AZT_ActionDo,           CmdFunc_GetPort_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetPortList

struct TagItem __far CmdTL_GetPortList[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetPortList,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetPortList,
    AZT_Gen_Flags,          FLAGS_GetPortList,
    AZT_ActionDo,           CmdFunc_GetPortList_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetPos

struct TagItem __far CmdTL_GetPos[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetPos,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetPos,
    AZT_Gen_Flags,          FLAGS_GetPos,
    AZT_ActionDo,           CmdFunc_GetPos_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetPrefs

struct TagItem __far CmdTL_GetPrefs[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetPrefs,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetPrefs,
    AZT_Gen_Flags,          FLAGS_GetPrefs,
    AZT_ActionDo,           CmdFunc_GetPrefs_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetPri

struct TagItem __far CmdTL_GetPri[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetPri,
    AZT_Gen_Flags,          FLAGS_GetPri,
    AZT_ActionDo,           CmdFunc_GetPri_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetPubScreen

struct TagItem __far CmdTL_GetPubScreen[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetPubScreen,
    AZT_Gen_Flags,          FLAGS_GetPubScreen,
    AZT_ActionDo,           CmdFunc_GetPubScreen_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetReadonly

struct TagItem __far CmdTL_GetReadonly[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetReadonly,
    AZT_Gen_Flags,          FLAGS_GetReadonly,
    AZT_ActionDo,           CmdFunc_GetReadonly_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetResident

struct TagItem __far CmdTL_GetResident[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetResident,
    AZT_Gen_Flags,          FLAGS_GetResident,
    AZT_ActionDo,           CmdFunc_GetResident_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetScreenInfo

struct TagItem __far CmdTL_GetScreenInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetScreenInfo,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetScreenInfo,
    AZT_Gen_Flags,          FLAGS_GetScreenInfo,
    AZT_ActionDo,           CmdFunc_GetScreenInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetScreenList

struct TagItem __far CmdTL_GetScreenList[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetScreenList,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetScreenList,
    AZT_Gen_Flags,          FLAGS_GetScreenList,
    AZT_ActionDo,           CmdFunc_GetScreenList_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetTaskInfo

struct TagItem __far CmdTL_GetTaskInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetTaskInfo,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetTaskInfo,
    AZT_Gen_Flags,          FLAGS_GetTaskInfo,
    AZT_ActionDo,           CmdFunc_GetTaskInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetText

struct TagItem __far CmdTL_GetText[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetText,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetText,
    AZT_Gen_Flags,          FLAGS_GetText,
    AZT_ActionDo,           CmdFunc_GetText_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetVar

struct TagItem __far CmdTL_GetVar[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetVar,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetVar,
    AZT_Gen_Flags,          FLAGS_GetVar,
    AZT_ActionDo,           CmdFunc_GetVar_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetVarList

struct TagItem __far CmdTL_GetVarList[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetVarList,
    AZT_Gen_Flags,          FLAGS_GetVarList,
    AZT_ActionDo,           CmdFunc_GetVarList_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetViewInfo

struct TagItem __far CmdTL_GetViewInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetViewInfo,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetViewInfo,
    AZT_Gen_Flags,          FLAGS_GetViewInfo,
    AZT_ActionDo,           CmdFunc_GetViewInfo_Do,
    TAG_END,
    };


//»»»»»» Cmd_GetWinList

struct TagItem __far CmdTL_GetWinList[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetWinList,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetWinList,
    AZT_Gen_Flags,          FLAGS_GetWinList,
    AZT_ActionDo,           CmdFunc_GetWinList_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetWinBorders

struct TagItem __far CmdTL_GetWinBorders[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetWinBorders,
    AZT_Gen_Flags,          FLAGS_GetWinBorders,
    AZT_ActionDo,           CmdFunc_GetWinBorders_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetWinInfo

struct TagItem __far CmdTL_GetWinInfo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetWinInfo,
    AZT_Gen_Flags,          FLAGS_GetWinInfo,
    AZT_ActionDo,           CmdFunc_GetWinInfo_Do,
    TAG_END,
    };

//»»»»»» Cmd_GetWord

struct TagItem __far CmdTL_GetWord[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_GetWord,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_GetWord,
    AZT_Gen_Flags,          FLAGS_GetWord,
    AZT_ActionDo,           CmdFunc_GetWord_Do,
    TAG_END,
    };

//»»»»»» Cmd_Help

struct TagItem __far CmdTL_Help[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Help,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Help,
    AZT_Gen_Flags,          FLAGS_Help,
    AZT_ActionDo,           CmdFunc_Help_Do,
    TAG_END,
    };

//»»»»»» Cmd_Illegal

struct TagItem __far CmdTL_Illegal[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Illegal,
    AZT_Gen_Flags,          FLAGS_Illegal,
    AZT_ActionDo,           CmdFunc_Illegal_Do,
    TAG_END,
    };

//»»»»»» Cmd_InactivateCmdLine

struct TagItem __far CmdTL_InactivateCmdLine[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_InactivateCmdLine,
    AZT_Gen_Flags,          FLAGS_InactivateCmdLine,
    AZT_ActionDo,           CmdFunc_InactivateCmdLine_Do,
    TAG_END,
    };

//»»»»»» Cmd_Indent

struct TagItem __far CmdTL_Indent[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Indent,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Indent,
    AZT_Gen_Flags,          FLAGS_Indent,
    AZT_ActionDo,           CmdFunc_Indent_Do,
    TAG_END,
    };

//»»»»»» Cmd_LineClone

struct TagItem __far CmdTL_LineClone[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_LineClone,
    AZT_Gen_Flags,          FLAGS_LineClone,
    AZT_ActionDo,           CmdFunc_LineClone_Do,
    TAG_END,
    };

//»»»»»» Cmd_LineJoin

struct TagItem __far CmdTL_LineJoin[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_LineJoin,
    AZT_Gen_Flags,          FLAGS_LineJoin,
    AZT_ActionDo,           CmdFunc_LineJoin_Do,
    TAG_END,
    };

//»»»»»» Cmd_Load

struct TagItem __far CmdTL_Load[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Load,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Load,
    AZT_Gen_Flags,          FLAGS_Load,
    AZT_ActionDo,           CmdFunc_Load_Do,
    TAG_END,
    };

//»»»»»» Cmd_Lock

struct TagItem __far CmdTL_Lock[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Lock,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Lock,
    AZT_Gen_Flags,          FLAGS_Lock,
    AZT_ActionDo,           CmdFunc_Lock_Do,
    TAG_END,
    };

//»»»»»» Cmd_Mac2File

struct TagItem __far CmdTL_Mac2File[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Mac2File,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Mac2File,
    AZT_Gen_Flags,          FLAGS_Mac2File,
    AZT_ActionDo,           CmdFunc_Mac2File_Do,
    TAG_END,
    };

//»»»»»» Cmd_Mac2Win

struct TagItem __far CmdTL_Mac2Win[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Mac2Win,
    AZT_Gen_Flags,          FLAGS_Mac2Win,
    AZT_ActionDo,           CmdFunc_Mac2Win_Do,
    TAG_END,
    };

//»»»»»» Cmd_MacDelete

struct TagItem __far CmdTL_MacDelete[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_MacDelete,
    AZT_Gen_Flags,          FLAGS_MacDelete,
    AZT_ActionDo,           CmdFunc_MacDelete_Do,
    TAG_END,
    };

//»»»»»» Cmd_MacEnd

struct TagItem __far CmdTL_MacEnd[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_MacEnd,
    AZT_Gen_Flags,          FLAGS_MacEnd,
    AZT_ActionDo,           CmdFunc_MacEnd_Do,
    TAG_END,
    };

//»»»»»» Cmd_MacPlay

struct TagItem __far CmdTL_MacPlay[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_MacPlay,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_MacPlay,
    AZT_Gen_Flags,          FLAGS_MacPlay,
    AZT_ActionDo,           CmdFunc_MacPlay_Do,
    TAG_END,
    };

//»»»»»» Cmd_MacRecord

struct TagItem __far CmdTL_MacRecord[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_MacRecord,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_MacRecord,
    AZT_Gen_Flags,          FLAGS_MacRecord,
    AZT_ActionDo,           CmdFunc_MacRecord_Do,
    TAG_END,
    };

//»»»»»» Cmd_Match

struct TagItem __far CmdTL_Match[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Match,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Match,
    AZT_Gen_Flags,          FLAGS_Match,
    AZT_ActionDo,           CmdFunc_Match_Do,
    TAG_END,
    };

//»»»»»» Cmd_NOp

static void CmdFunc_NOp_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_NOp[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_NOp,
    AZT_Gen_Flags,          FLAGS_NOp,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_NOp_Do,
    TAG_END,
    };

//»»»»»» Cmd_Prefs2File

struct TagItem __far CmdTL_Prefs2File[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Prefs2File,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Prefs2File,
    AZT_Gen_Flags,          FLAGS_Prefs2File,
    AZT_ActionDo,           CmdFunc_Prefs2File_Do,
    TAG_END,
    };

//»»»»»» Cmd_File2Prefs

struct TagItem __far CmdTL_File2Prefs[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_File2Prefs,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_File2Prefs,
    AZT_Gen_Flags,          FLAGS_File2Prefs,
    AZT_ActionDo,           CmdFunc_File2Prefs_Do,
    TAG_END,
    };

//»»»»»» Cmd_ReDo

static void CmdFunc_ReDo_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ReDo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ReDo,
    AZT_Gen_Flags,          FLAGS_ReDo,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ReDo_Do,
    TAG_END,
    };

//»»»»»» Cmd_RemakeScreen

static void CmdFunc_RemakeScreen_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_RemakeScreen[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_RemakeScreen,
    AZT_Gen_Flags,          FLAGS_RemakeScreen,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_RemakeScreen_Do,
    TAG_END,
    };

//»»»»»» Cmd_Replace

struct TagItem __far CmdTL_Replace[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Replace,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Replace,
    AZT_Gen_Flags,          FLAGS_Replace,
    AZT_ActionDo,           CmdFunc_Replace_Do,
    TAG_END,
    };

//»»»»»» Cmd_ReqClose

struct TagItem __far CmdTL_ReqClose[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ReqClose,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ReqClose,
    AZT_Gen_Flags,          FLAGS_ReqClose,
    AZT_ActionDo,           CmdFunc_ReqClose_Do,
    TAG_END,
    };

//»»»»»» Cmd_ReqFile

struct TagItem __far CmdTL_ReqFile[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ReqFile,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ReqFile,
    AZT_Gen_Flags,          FLAGS_ReqFile,
    AZT_ActionDo,           CmdFunc_ReqFile_Do,
    TAG_END,
    };

//»»»»»» Cmd_ReqList

struct TagItem __far CmdTL_ReqList[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ReqList,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ReqList,
    AZT_Gen_Flags,          FLAGS_ReqList,
    AZT_ActionDo,           CmdFunc_ReqList_Do,
    TAG_END,
    };

//»»»»»» Cmd_ReqMulti

struct TagItem __far CmdTL_ReqMulti[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ReqMulti,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ReqMulti,
    AZT_Gen_Flags,          FLAGS_ReqMulti,
    AZT_ActionDo,           CmdFunc_ReqMulti_Do,
    TAG_END,
    };

//»»»»»» Cmd_ReqOpen

struct TagItem __far CmdTL_ReqOpen[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ReqOpen,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ReqOpen,
    AZT_Gen_Flags,          FLAGS_ReqOpen,
    AZT_ActionDo,           CmdFunc_ReqOpen_Do,
    TAG_END,
    };

//»»»»»» Cmd_RexxNameClear

struct TagItem __far CmdTL_RexxNameClear[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_RexxNameClear,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_RexxNameClear,
    AZT_Gen_Flags,          FLAGS_RexxNameClear,
    AZT_ActionDo,           CmdFunc_RexxNameClear_Do,
    TAG_END,
    };

//»»»»»» Cmd_RexxNameObtain

struct TagItem __far CmdTL_RexxNameObtain[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_RexxNameObtain,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_RexxNameObtain,
    AZT_Gen_Flags,          FLAGS_RexxNameObtain,
    AZT_ActionDo,           CmdFunc_RexxNameObtain_Do,
    TAG_END,
    };

//»»»»»» Cmd_RexxNameRelease

struct TagItem __far CmdTL_RexxNameRelease[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_RexxNameRelease,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_RexxNameRelease,
    AZT_Gen_Flags,          FLAGS_RexxNameRelease,
    AZT_ActionDo,           CmdFunc_RexxNameRelease_Do,
    TAG_END,
    };

//»»»»»» Cmd_Screen2Back

struct TagItem __far CmdTL_Screen2Back[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Screen2Back,
    AZT_Gen_Flags,          FLAGS_Screen2Back,
    AZT_ActionDo,           CmdFunc_Screen2Back_Do,
    TAG_END,
    };

//»»»»»» Cmd_Screen2Front

struct TagItem __far CmdTL_Screen2Front[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Screen2Front,
    AZT_Gen_Flags,          FLAGS_Screen2Front,
    AZT_ActionDo,           CmdFunc_Screen2Front_Do,
    TAG_END,
    };

//»»»»»» Cmd_Search

struct TagItem __far CmdTL_Search[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Search,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Search,
    AZT_Gen_Flags,          FLAGS_Search,
    AZT_ActionDo,           CmdFunc_Search_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetModified

struct TagItem __far CmdTL_SetModified[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetModified,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetModified,
    AZT_Gen_Flags,          FLAGS_SetModified,
    AZT_ActionDo,           CmdFunc_SetModified_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetResident

struct TagItem __far CmdTL_SetResident[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetResident,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetResident,
    AZT_Gen_Flags,          FLAGS_SetResident,
    AZT_ActionDo,           CmdFunc_SetResident_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetCompress

struct TagItem __far CmdTL_SetCompress[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetCompress,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetCompress,
    AZT_Gen_Flags,          FLAGS_SetCompress,
    AZT_ActionDo,           CmdFunc_SetCompress_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetCurrentDir

struct TagItem __far CmdTL_SetCurrentDir[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetCurrentDir,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetCurrentDir,
    AZT_Gen_Flags,          FLAGS_SetCurrentDir,
    AZT_ActionDo,           CmdFunc_SetCurrentDir_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetDebug

struct TagItem __far CmdTL_SetDebug[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetDebug,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetDebug,
    AZT_Gen_Flags,          FLAGS_SetDebug,
    AZT_ActionDo,           CmdFunc_SetDebug_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetFilePath

struct TagItem __far CmdTL_SetFilePath[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetFilePath,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetFilePath,
    AZT_Gen_Flags,          FLAGS_SetFilePath,
    AZT_ActionDo,           CmdFunc_SetFilePath_Do,
    TAG_END,
    };

//»»»»»» Cmd_Qualifier

struct TagItem __far CmdTL_Qualifier[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Qualifier,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Qualifier,
    AZT_Gen_Flags,          FLAGS_Qualifier,
    AZT_ActionDo,           CmdFunc_Qualifier_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetPrefs

struct TagItem __far CmdTL_SetPrefs[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetPrefs,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetPrefs,
    AZT_Gen_Flags,          FLAGS_SetPrefs,
    AZT_ActionDo,           CmdFunc_SetPrefs_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetPri

struct TagItem __far CmdTL_SetPri[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetPri,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetPri,
    AZT_Gen_Flags,          FLAGS_SetPri,
    AZT_ActionDo,           CmdFunc_SetPri_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetQuoteMode

static void CmdFunc_SetQuoteMode_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_SetQuoteMode[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetQuoteMode,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetQuoteMode,
    AZT_Gen_Flags,          FLAGS_SetQuoteMode,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_SetQuoteMode_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetStatusBar

static void CmdFunc_SetStatusBar_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_SetStatusBar[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetStatusBar,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetStatusBar,
    AZT_Gen_Flags,          FLAGS_SetStatusBar,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_SetStatusBar_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetVar

struct TagItem __far CmdTL_SetVar[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetVar,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetVar,
    AZT_Gen_Flags,          FLAGS_SetVar,
    AZT_ActionDo,           CmdFunc_SetVar_Do,
    TAG_END,
    };

//»»»»»» Cmd_SetWriteReplace

struct TagItem __far CmdTL_SetWriteReplace[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_SetWriteReplace,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_SetWriteReplace,
    AZT_Gen_Flags,          FLAGS_SetWriteReplace,
    AZT_ActionDo,           CmdFunc_SetWriteReplace_Do,
    TAG_END,
    };

//»»»»»» Cmd_Space2Tab

static void CmdFunc_Space2Tab_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_Space2Tab[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Space2Tab,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Space2Tab,
    AZT_Gen_Flags,          FLAGS_Space2Tab,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_Space2Tab_Do,
    TAG_END,
    };

//»»»»»» Cmd_Sort

struct TagItem __far CmdTL_Sort[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Sort,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Sort,
    AZT_Gen_Flags,          FLAGS_Sort,
    AZT_ActionDo,           CmdFunc_Sort_Do,
    TAG_END,
    };

//»»»»»» Cmd_Surround

struct TagItem __far CmdTL_Surround[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Surround,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Surround,
    AZT_Gen_Flags,          FLAGS_Surround,
    AZT_ActionDo,           CmdFunc_Surround_Do,
    TAG_END,
    };

//»»»»»» Cmd_Tab2Space

static void CmdFunc_Tab2Space_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_Tab2Space[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Tab2Space,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Tab2Space,
    AZT_Gen_Flags,          FLAGS_Tab2Space,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_Tab2Space_Do,
    TAG_END,
    };

//»»»»»» Cmd_Template

static void CmdFunc_Template_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_Template[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Template,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Template,
    AZT_Gen_Flags,          FLAGS_Template,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_Template_Do,
    TAG_END,
    };

//»»»»»» Cmd_Text2CB

struct TagItem __far CmdTL_Text2CB[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Text2CB,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Text2CB,
    AZT_Gen_Flags,          FLAGS_Text2CB,
    AZT_ActionDo,           CmdFunc_Text2CB_Do,
    TAG_END,
    };

//»»»»»» Cmd_Text2File

struct TagItem __far CmdTL_Text2File[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Text2File,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Text2File,
    AZT_Gen_Flags,          FLAGS_Text2File,
    AZT_ActionDo,           CmdFunc_Text2File_Do,
    TAG_END,
    };

//»»»»»» Cmd_Text2Win

struct TagItem __far CmdTL_Text2Win[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Text2Win,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Text2Win,
    AZT_Gen_Flags,          FLAGS_Text2Win,
    AZT_ActionDo,           CmdFunc_Text2Win_Do,
    TAG_END,
    };

//»»»»»» Cmd_TextFormat

struct TagItem __far CmdTL_TextFormat[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_TextFormat,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_TextFormat,
    AZT_Gen_Flags,          FLAGS_TextFormat,
    AZT_ActionDo,           CmdFunc_TextFormat_Do,
    TAG_END,
    };

//»»»»»» Cmd_TextJustify

struct TagItem __far CmdTL_TextJustify[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_TextJustify,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_TextJustify,
    AZT_Gen_Flags,          FLAGS_TextJustify,
    AZT_ActionDo,           CmdFunc_TextJustify_Do,
    TAG_END,
    };

//»»»»»» Cmd_Undo

static void CmdFunc_Undo_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_Undo[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Undo,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Undo,
    AZT_Gen_Flags,          FLAGS_Undo,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_Undo_Do,
    TAG_END,
    };

//»»»»»» Cmd_Unload

struct TagItem __far CmdTL_Unload[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Unload,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Unload,
    AZT_Gen_Flags,          FLAGS_Unload,
    AZT_ActionDo,           CmdFunc_Unload_Do,
    TAG_END,
    };

//»»»»»» Cmd_Version

struct TagItem __far CmdTL_Version[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Version,
    AZT_Gen_Flags,          FLAGS_Version,
    AZT_ActionDo,           CmdFunc_Version_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewActivate

static void CmdFunc_ViewActivate_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewActivate[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewActivate,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ViewActivate,
    AZT_Gen_Flags,          FLAGS_ViewActivate,
    AZT_ActionDo,           CmdFunc_ViewActivate_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewCenter

static void CmdFunc_ViewCenter_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewCenter[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewCenter,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ViewCenter,
    AZT_Gen_Flags,          FLAGS_ViewCenter,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ViewCenter_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewDown

static void CmdFunc_ViewDown_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewDown[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewDown,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ViewDown,
    AZT_Gen_Flags,          FLAGS_ViewDown,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ViewDown_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewLeft

static void CmdFunc_ViewLeft_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewLeft[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewLeft,
    AZT_Gen_Flags,          FLAGS_ViewLeft,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ViewLeft_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewRight

static void CmdFunc_ViewRight_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewRight[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewRight,
    AZT_Gen_Flags,          FLAGS_ViewRight,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ViewRight_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewScroll

static void CmdFunc_ViewScroll_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewScroll[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewScroll,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ViewScroll,
    AZT_Gen_Flags,          FLAGS_ViewScroll,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ViewScroll_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewSize

static void CmdFunc_ViewSize_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewSize[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewSize,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ViewSize,
    AZT_Gen_Flags,          FLAGS_ViewSize,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ViewSize_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewSplit

static void CmdFunc_ViewSplit_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewSplit[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewSplit,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ViewSplit,
    AZT_Gen_Flags,          FLAGS_ViewSplit,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ViewSplit_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewSwitch

static void CmdFunc_ViewSwitch_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewSwitch[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewSwitch,
    AZT_Gen_Flags,          FLAGS_ViewSwitch,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ViewSwitch_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewUp

static void CmdFunc_ViewUp_Do( struct CmdObj *co )
{
}

struct TagItem __far CmdTL_ViewUp[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewUp,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ViewUp,
    AZT_Gen_Flags,          FLAGS_ViewUp,
    //AZT_ActionInitMenu,     CmdFunc_Ghost,
    AZT_ActionDo,           CmdFunc_ViewUp_Do,
    TAG_END,
    };

//»»»»»» Cmd_ViewUpdate

struct TagItem __far CmdTL_ViewUpdate[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_ViewUpdate,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_ViewUpdate,
    AZT_Gen_Flags,          FLAGS_ViewUpdate,
    AZT_ActionDo,           CmdFunc_ViewUpdate_Do,
    TAG_END,
    };

//»»»»»» Cmd_Win2Back

struct TagItem __far CmdTL_Win2Back[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Win2Back,
    AZT_Gen_Flags,          FLAGS_Win2Back,
    AZT_ActionDo,           CmdFunc_Win2Back_Do,
    TAG_END,
    };

//»»»»»» Cmd_Win2CB

struct TagItem __far CmdTL_Win2CB[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Win2CB,
    AZT_Gen_Flags,          FLAGS_Win2CB,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Win2CB,
    AZT_ActionDo,           CmdFunc_Win2CB_Do,
    TAG_END,
    };

//»»»»»» Cmd_Win2File

struct TagItem __far CmdTL_Win2File[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Win2File,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_Win2File,
    AZT_Gen_Flags,          FLAGS_Win2File,
    //AZT_ActionNew,          CmdFunc_Win2File_New,
    AZT_ActionDo,           CmdFunc_Win2File_Do,
    TAG_END,
    };

//»»»»»» Cmd_Win2Front

struct TagItem __far CmdTL_Win2Front[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_Win2Front,
    AZT_Gen_Flags,          FLAGS_Win2Front,
    AZT_ActionDo,           CmdFunc_Win2Front_Do,
    TAG_END,
    };

//»»»»»» Cmd_WinChange

struct TagItem __far CmdTL_WinChange[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_WinChange,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_WinChange,
    AZT_Gen_Flags,          FLAGS_WinChange,
    AZT_ActionDo,           CmdFunc_WinChange_Do,
    TAG_END,
    };

//»»»»»» Cmd_WinClear

struct TagItem __far CmdTL_WinClear[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_WinClear,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_WinClear,
    AZT_Gen_Flags,          FLAGS_WinClear,
    AZT_ActionDo,           CmdFunc_WinClear_Do,
    TAG_END,
    };

//»»»»»» Cmd_WinClose

struct TagItem __far CmdTL_WinClose[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_WinClose,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_WinClose,
    AZT_Gen_Flags,          FLAGS_WinClose,
    AZT_ActionDo,           CmdFunc_WinClose_Do,
    TAG_END,
    };

//»»»»»» Cmd_WinAct

struct TagItem __far CmdTL_WinAct[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_WinAct,
    AZT_Gen_Flags,          FLAGS_WinAct,
    AZT_ActionDo,           CmdFunc_WinAct_Do,
    TAG_END,
    };

//»»»»»» Cmd_WinIconify

struct TagItem __far CmdTL_WinIconify[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_WinIconify,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_WinIconify,
    AZT_Gen_Flags,          FLAGS_WinIconify,
    AZT_ActionDo,           CmdFunc_WinIconify_Do,
    TAG_END,
    };

//»»»»»» Cmd_WinLast

struct TagItem __far CmdTL_WinLast[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_WinLast,
    AZT_Gen_Flags,          FLAGS_WinLast,
    AZT_ActionDo,           CmdFunc_WinLast_Do,
    TAG_END,
    };

//»»»»»» Cmd_WinNext

struct TagItem __far CmdTL_WinNext[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_WinNext,
    AZT_Gen_Flags,          FLAGS_WinNext,
    AZT_ActionDo,           CmdFunc_WinNext_Do,
    TAG_END,
    };

//»»»»»» Cmd_WinPrev

struct TagItem __far CmdTL_WinPrev[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_WinPrev,
    AZT_Gen_Flags,          FLAGS_WinPrev,
    AZT_ActionDo,           CmdFunc_WinPrev_Do,
    TAG_END,
    };

//»»»»»» Cmd_WinZoom

struct TagItem __far CmdTL_WinZoom[] = {
    AZT_Gen_Name,           (ULONG) CmdNames_WinZoom,
    AZT_Gen_ArgTemplate,    (ULONG) CmdTmpl_WinZoom,
    AZT_Gen_Flags,          FLAGS_WinZoom,
    AZT_ActionDo,           CmdFunc_WinZoom_Do,
    TAG_END,
    };
