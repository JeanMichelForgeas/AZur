        IFND    AZURDEFS_OBJFILE
AZURDEFS_OBJFILE SET     1
**
**      Copyright (C) 1993 Jean-Michel Forgeas
**                  Tous Droits Réservés
**

    IFND UIKDEFS_OBJ
    INCLUDE "UIKI:uikobj.i"
    ENDC

    IFND UIKDEFS_LIST
    INCLUDE "UIKI:uiklist.i"
    ENDC


UNB_BLKSTART   equ 11
UNB_BLKEND     equ 12
UNB_ENDSPACE   equ 13
UNB_CONTINUE   equ 14
UNB_FOLDED     equ 15

UNF_BLKSTART   equ $0800
UNF_BLKEND     equ $1000
UNF_ENDSPACE   equ $2000
UNF_CONTINUE   equ $4000
UNF_FOLDED     equ $8000


 STRUCTURE  AZObjFile,0
    STRUCT      aof_Obj,UIKOBJ_SIZE

    APTR        aof_OpenFileName
    APTR        aof_SaveFileName
    ULONG       aof_padl3
    ULONG       aof_padl4
    APTR        aof_BkupFileName

    APTR        aof_OpenFS
    APTR        aof_SaveFS
    ULONG       aof_MaxLines

    UBYTE       aof_BlinkCount
    UBYTE       aof_ScrollLen
    UWORD       aof_SysQuals

    STRUCT      aof_Text,UIKPList_SIZE

    ULONG       aof_Flags

    APTR        aof_WO
    STRUCT      aof_ViewList,LH_SIZE

    APTR        aof_CtrlDVect

    APTR        aof_padl5

    LONG        aof_Line
    LONG        aof_Col
    LONG        aof_OldLine
    LONG        aof_OldCol
    APTR        aof_BlockInfo
    LONG        aof_LeftMargin
    LONG        aof_RightMargin
    ULONG       aof_ModNum
    STRUCT      aof_padl,4*4
    APTR        aof_Stat
    LONG        aof_CEdCol

    APTR        aof_SearchString
    APTR        aof_ReplaceString
    APTR        aof_LineString
    APTR        aof_ColString
    APTR        aof_ReqString

    APTR        aof_ActiveView
    STRUCT      aof_VI,AZViewInfo_SIZE

    STRUCT      aof_Ind,24

    LONG        aof_FoundLine
    LONG        aof_FoundCol
    LONG        aof_FoundELine
    LONG        aof_FoundECol
    ULONG       aof_WorkLine
    ULONG       aof_WorkCol
    ULONG       aof_WorkReqLine
    ULONG       aof_WorkReqCol

    WORD        aof_MouseX
    WORD        aof_MouseY
    ULONG       aof_Seconds
    ULONG       aof_Micros

    APTR        aof_HexString

    STRUCT      aof_padl8,37*4

    APTR        aof_Menu
    APTR        aof_CmdLine

    UBYTE       aof_CurBlockMode
    UBYTE       aof_TickCount
    WORD        aof_MemButs
    WORD        aof_CmdLineIndex
    APTR        aof_SeRep;
    LABEL   AZObjFile_SIZE




        ENDC
