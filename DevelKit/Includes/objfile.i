        IFND    AZURDEFS_OBJFILE
AZURDEFS_OBJFILE SET     1
**
**      Copyright (C) 1993 Jean-Michel Forgeas
**                  Tous Droits Réservés
**


;--- Flags pour TextPrep()
ARPF_CONVERT    equ $00000001
ARPF_NOCONVERT  equ $00000002
ARPF_RECT       equ $00000004
ARPF_NORECT     equ $00000008
ARPF_OVERLAY    equ $00000010
ARPF_NOOVERLAY  equ $00000020
ARPF_CONTINUE   equ $80000000


 STRUCTURE  AZObjFile,0
    STRUCT      aof_Obj,228

    APTR        aof_OpenFileName
    APTR        aof_SaveFileName
    ULONG       aof_Reserved0
    ULONG       aof_Reserved1
    APTR        aof_BkupFileName

    APTR        aof_OpenFS
    APTR        aof_SaveFS
    ULONG       aof_MaxLines

    UBYTE       aof_BlinkCount
    UBYTE       aof_padb
    UWORD       aof_SysQuals

    STRUCT      aof_Text,24     UIKPList_SIZE

    ULONG       aof_Flags

    APTR        aof_WO
    STRUCT      aof_ViewList,LH_SIZE

    ULONG       aof_Reserved2
    ULONG       aof_Reserved3

    LONG        aof_Line
    LONG        aof_Col
    LONG        aof_OldLine
    LONG        aof_OldCol
    STRUCT      aof_Reserved4,4*1
    LONG        aof_LeftMargin
    LONG        aof_RightMargin
    ULONG       aof_ModNum
    STRUCT      aof_Reserved5,4*6

    APTR        aof_SearchString
    APTR        aof_ReplaceString
    APTR        aof_LineString
    APTR        aof_ColString
    APTR        aof_ReqString

    APTR        aof_ActiveView
    STRUCT      aof_VI,AZViewInfo_SIZE

    STRUCT      aof_Reserved6,32

    LONG        aof_FoundLine
    LONG        aof_FoundCol
    LONG        aof_WorkLine
    LONG        aof_WorkCol
    LONG        aof_WorkReqLine
    LONG        aof_WorkReqCol

    WORD        aof_MouseX
    WORD        aof_MouseY
    WORD        aof_OldMouseX
    WORD        aof_OldMouseY
    ULONG       aof_Seconds
    ULONG       aof_Micros

    APTR        aof_HexString

    LABEL   AZObjFile_SIZE




        ENDC
