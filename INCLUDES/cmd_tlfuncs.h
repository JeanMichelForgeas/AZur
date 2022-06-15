void CmdFunc_Qualifier_Do( struct CmdObj *co );
void CmdFunc_Help_Do( struct CmdObj *co );
void CmdFunc_Illegal_Do( struct CmdObj *co );
void CmdFunc_AbortARexx_Do( struct CmdObj *co );
void CmdFunc_Abbreviate_Do( struct CmdObj *co );
void CmdFunc_Lock_Do( struct CmdObj *co );
void CmdFunc_ExeARexxCmd_Do( struct CmdObj *co );
void CmdFunc_ExeARexxScript_Do( struct CmdObj *co );
void CmdFunc_ExeARexx_SetMenu( struct CmdObj *co );
void CmdFunc_ExeDOSCmd_Do( struct CmdObj *co );
void CmdFunc_ExeDOSScript_Do( struct CmdObj *co );
void CmdFunc_ExeLastCmd_Do( struct CmdObj *co );
void CmdFunc_ActivateCmdLine_Do( struct CmdObj *co );
void CmdFunc_InactivateCmdLine_Do( struct CmdObj *co );
void CmdFunc_Debug_Do( struct CmdObj *co );
void CmdFunc_File2Mac_Do( struct CmdObj *co );
void CmdFunc_Mac2File_Do( struct CmdObj *co );
void CmdFunc_Mac2Win_Do( struct CmdObj *co );
void CmdFunc_MacDelete_Do( struct CmdObj *co );
void CmdFunc_MacEnd_Do( struct CmdObj *co );
void CmdFunc_MacPlay_Do( struct CmdObj *co );
void CmdFunc_MacRecord_Do( struct CmdObj *co );

void CmdFunc_WinClose_Do( struct CmdObj *co );
void CmdFunc_File2NewWin_Do( struct CmdObj *co );
void CmdFunc_WinIconify_Do( struct CmdObj *co );
void CmdFunc_ViewUpdate_Do( struct CmdObj *co );
void CmdFunc_Win2Back_Do( struct CmdObj *co );
void CmdFunc_Win2Front_Do( struct CmdObj *co );
void CmdFunc_WinAct_Do( struct CmdObj *co );
void CmdFunc_WinChange_Do( struct CmdObj *co );
void CmdFunc_WinLast_Do( struct CmdObj *co );
void CmdFunc_WinNext_Do( struct CmdObj *co );
void CmdFunc_WinPrev_Do( struct CmdObj *co );
void CmdFunc_WinZoom_Do( struct CmdObj *co );
void CmdFunc_Screen2Back_Do( struct CmdObj *co );
void CmdFunc_Screen2Front_Do( struct CmdObj *co );

void CmdFunc_File2Win_Do( struct CmdObj *co );
void CmdFunc_WinClear_Do( struct CmdObj *co );
void CmdFunc_ReqOpen_Do( struct CmdObj *co );
void CmdFunc_ReqFile_Do( struct CmdObj *co );
void CmdFunc_ReqList_Do( struct CmdObj *co );
void CmdFunc_ReqMulti_Do( struct CmdObj *co );
void CmdFunc_ReqClose_Do( struct CmdObj *co );
//void CmdFunc_Win2File_New( struct CmdObj *co );
void CmdFunc_Win2File_Do( struct CmdObj *co );
void CmdFunc_Prefs2File_Do( struct CmdObj *co );
void CmdFunc_File2Prefs_Do( struct CmdObj *co );
void CmdFunc_File2Parms_Do( struct CmdObj *co );

void CmdFunc_Del_Do( struct CmdObj *co );
//void CmdFunc_LineDel_Do( struct CmdObj *co );
//void CmdFunc_LineInsert_Do( struct CmdObj *co );
void CmdFunc_Text2Win_Do( struct CmdObj *co );
void CmdFunc_Text2CB_Do( struct CmdObj *co );
void CmdFunc_Text2File_Do( struct CmdObj *co );
void CmdFunc_LineClone_Do( struct CmdObj *co );
void CmdFunc_LineJoin_Do( struct CmdObj *co );
void CmdFunc_CharConvert_Do( struct CmdObj *co );
void CmdFunc_Search_Do( struct CmdObj *co );
void CmdFunc_Replace_Do( struct CmdObj *co );
void CmdFunc_Surround_Do( struct CmdObj *co );
void CmdFunc_Sort_Do( struct CmdObj *co );
void CmdFunc_Match_Do( struct CmdObj *co );
void CmdFunc_Indent_Do( struct CmdObj *co );
void CmdFunc_ObtainName_Do( struct CmdObj *co );
void CmdFunc_RexxNameObtain_Do( struct CmdObj *co );
void CmdFunc_RexxNameRelease_Do( struct CmdObj *co );
void CmdFunc_RexxNameClear_Do( struct CmdObj *co );
void CmdFunc_Fold_Do( struct CmdObj *co );
void CmdFunc_TextJustify_Do( struct CmdObj *co );
void CmdFunc_TextFormat_Do( struct CmdObj *co );

void CmdFunc_BMAdd_Do( struct CmdObj *co );
void CmdFunc_BMCursor_Do( struct CmdObj *co );
void CmdFunc_BMRemove_Do( struct CmdObj *co );
void CmdFunc_CursLastChange_Do( struct CmdObj *co );
void CmdFunc_Cursor_Do( struct CmdObj *co );
void CmdFunc_CursorOld_Do( struct CmdObj *co );
void CmdFunc_Block_New( struct CmdObj *co );
void CmdFunc_Block_Do( struct CmdObj *co );
void CmdFunc_Block2CB_Do( struct CmdObj *co );
void CmdFunc_Block2File_Do( struct CmdObj *co );
void CmdFunc_Block2Mac_Do( struct CmdObj *co );
void CmdFunc_Unload_Do( struct CmdObj *co );
void CmdFunc_Load_Do( struct CmdObj *co );
void CmdFunc_Version_Do( struct CmdObj *co );
void CmdFunc_Beep_Do( struct CmdObj *co );
void CmdFunc_CB2File_Do( struct CmdObj *co );
void CmdFunc_CB2Win_Do( struct CmdObj *co );
void CmdFunc_Win2CB_Do( struct CmdObj *co );
void CmdFunc_File2CB_Do( struct CmdObj *co );
void CmdFunc_File2File_Do( struct CmdObj *co );

void CmdFunc_GetResident_Do( struct CmdObj *co );
void CmdFunc_GetBlock_Do( struct CmdObj *co );
void CmdFunc_GetBlockInfo_Do( struct CmdObj *co );
void CmdFunc_GetBMInfo_Do( struct CmdObj *co );
void CmdFunc_GetBMList_Do( struct CmdObj *co );
void CmdFunc_GetChar_Do( struct CmdObj *co );
void CmdFunc_GetCurrentDir_Do( struct CmdObj *co );
void CmdFunc_GetFileInfo_Do( struct CmdObj *co );
void CmdFunc_GetFilePath_Do( struct CmdObj *co );
void CmdFunc_GetFold_Do( struct CmdObj *co );
void CmdFunc_GetFoldInfo_Do( struct CmdObj *co );
void CmdFunc_GetFontInfo_Do( struct CmdObj *co );
void CmdFunc_GetLockInfo_Do( struct CmdObj *co );
void CmdFunc_GetMasterPort_Do( struct CmdObj *co );
void CmdFunc_GetPort_Do( struct CmdObj *co );
void CmdFunc_GetPortList_Do( struct CmdObj *co );
void CmdFunc_GetPos_Do( struct CmdObj *co );
void CmdFunc_GetPrefs_Do( struct CmdObj *co );
void CmdFunc_GetPri_Do( struct CmdObj *co );
void CmdFunc_GetPubScreen_Do( struct CmdObj *co );
void CmdFunc_GetReadonly_Do( struct CmdObj *co );
void CmdFunc_GetError_Do( struct CmdObj *co );
void CmdFunc_GetLine_Do( struct CmdObj *co );
void CmdFunc_GetLineInfo_Do( struct CmdObj *co );
void CmdFunc_GetWord_Do( struct CmdObj *co );
void CmdFunc_GetModified_Do( struct CmdObj *co );
void CmdFunc_GetNumBlocks_Do( struct CmdObj *co );
void CmdFunc_GetNumBM_Do( struct CmdObj *co );
void CmdFunc_GetNumFolds_Do( struct CmdObj *co );
void CmdFunc_GetNumLines_Do( struct CmdObj *co );
void CmdFunc_GetTaskInfo_Do( struct CmdObj *co );
void CmdFunc_GetText_Do( struct CmdObj *co );
void CmdFunc_GetVarList_Do( struct CmdObj *co );
void CmdFunc_GetVar_Do( struct CmdObj *co );
void CmdFunc_GetScreenInfo_Do( struct CmdObj *co );
void CmdFunc_GetScreenList_Do( struct CmdObj *co );
void CmdFunc_GetViewInfo_Do( struct CmdObj *co );
void CmdFunc_GetWinBorders_Do( struct CmdObj *co );
void CmdFunc_GetWinInfo_Do( struct CmdObj *co );
void CmdFunc_GetWinList_Do( struct CmdObj *co );

void CmdFunc_SetCurrentDir_Do( struct CmdObj *co );
void CmdFunc_SetDebug_Do( struct CmdObj *co );
void CmdFunc_SetFilePath_Do( struct CmdObj *co );
void CmdFunc_SetCompress_Do( struct CmdObj *co );
void CmdFunc_SetResident_Do( struct CmdObj *co );
void CmdFunc_SetPrefs_Do( struct CmdObj *co );
void CmdFunc_SetPri_Do( struct CmdObj *co );
void CmdFunc_SetModified_Do( struct CmdObj *co );
void CmdFunc_SetVar_Do( struct CmdObj *co );
void CmdFunc_SetWriteReplace_Do( struct CmdObj *co );
