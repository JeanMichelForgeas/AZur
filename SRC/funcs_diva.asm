;    OPT D+,O+,W-
*****************************************************************
*
*       Project:    AZur
*       Function:   Fonctions diverses
*
*       Created:    10/07/93    Jean-Michel Forgeas
*
*****************************************************************


******* Includes ************************************************

        INCLUDE "exec/types.i"
        INCLUDE "exec/ables.i"
        INCLUDE "lib/console_lib.i"
        INCLUDE "lib/exec_lib.i"
        INCLUDE "lib/dos_lib.i"
        INCLUDE "UIKI:uikbase.i"
        INCLUDE "UIKI:uikglobal.i"
        INCLUDE "UIKI:uik_lib.i"
        INCLUDE "UIKI:objscreen.i"
        INCLUDE "LCI:azur.i"
        INCLUDE "LCI:eng_obj.i"
        INCLUDE "LCI:objfile.i"


******* Imported ************************************************

        XREF    _PoolAlloc

        XREF    _eng_NewGerm
        XREF    _eng_DisposeGerm
        XREF    _eng_NewObject
        XREF    _eng_DisposeObject
        XREF    _func_ReqShowText
        XREF    _SET_MODIFIED


******* Exported ************************************************

        XDEF    _AZRawKeyConvert

        XDEF    _intAZ_GetLineFlags
        XDEF    _intAZ_SetLineFlags
        XDEF    _intAZ_ORLineFlags
        XDEF    _intAZ_ANDLineFlags

        XDEF    _intAZ_SemaCreate
        XDEF    _intAZ_SemaWrite
        XDEF    _intAZ_SemaFreeWrite
        XDEF    _intAZ_SemaRead
        XDEF    _intAZ_SemaFreeRead
        XDEF    _func_SemaCreate
        XDEF    _func_SemaWrite
        XDEF    _func_SemaFreeWrite
        XDEF    _func_SemaRead
        XDEF    _func_SemaFreeRead

        XDEF    _intAZ_GetErrorSeverity
        XDEF    _intAZ_SetCmdResult
        XDEF    _func_SetCmdResult

        XDEF    _func_GetAZur
        XDEF    _func_MatchStr
        XDEF    _func_CharIsWord
        XDEF    _func_CharIsFile
        XDEF    _func_SetQualifier

        XDEF    _intAZ_CharCurrent
        XDEF    _intAZ_LineBuf
        XDEF    _intAZ_LineBufLen
        XDEF    _intAZ_LineLen
        XDEF    _intAZ_Buf_Len
        XDEF    _intAZ_Node_Buf_Len

        XDEF    _func_CharCurrent
        XDEF    _func_NumLines
        XDEF    _func_LineBuf
        XDEF    _func_LineLen
        XDEF    _func_LineBufLen
        XDEF    _func_BufAlloc
        XDEF    _func_BufFree
        XDEF    _func_BufPrintf

        XDEF    _func_LoadGerm
        XDEF    _func_UnloadGerm
        XDEF    _func_NewObject
        XDEF    _func_DisposeObject
        XDEF    _func_SetModified

        XDEF    _func_CodeText,_func_DecodeText
        XDEF    _func_IsNCSubStr
        XDEF    _intAZ_IsNCSubStr
        XDEF    _func_IsSubStr
        XDEF    _intAZ_IsSubStr
        XDEF    _intAZ_StrNCEqual

        XDEF    _func_DoColorMask


*****************************************************************


*****************************************************************
*
                    SECTION     text,CODE
*
*****************************************************************



****************************************************************
*   actual = AZRawKeyConvert( APTR *ev, UBYTE *buf, UBYTE *len )
*   d0                              a0         a1          d1
****************************************************************

_AZRawKeyConvert
    movem.l a2/a6,-(sp)
    move.l  az_UIK(a4),a6
    move.l  uike_KeySupp+uks_ConsoleMsg+IO_DEVICE(a6),a6
    sub.l   a2,a2
    jsr     _LVORawKeyConvert(a6)
    movem.l (sp)+,a2/a6
    rts


**********************************************************************
* void func_SemaFreeRead( struct AZSema *as )
*                                        A0
**********************************************************************

_func_SemaFreeRead
        move.l  4(sp),a0

_intAZ_SemaFreeRead
        subq.w  #1,azse_ReadCount(a0)
        rts     ; laisse a0 inchangé

**********************************************************************
* void func_SemaRead( struct AZSema *as )
*                                    A0
**********************************************************************

_func_SemaRead
        move.l  4(sp),a0

_intAZ_SemaRead
        move.l  a1,-(sp)
SemaReadLoop
        DISABLE a1
        btst.b  #AZSEMB_WRITE,azse_Flags(a0)
        bne.s   mustwait
        addq.w  #1,azse_ReadCount(a0)
        ENABLE  a1
        move.l  (sp)+,a1
        rts     ; préserve les registres

mustwait
    ENABLE  a1
    movem.l a0/a6,-(sp)
    move.l  az_ExecBase(a4),a6
    jsr     _LVOObtainSemaphore(a6)     a0 pointe sur semaphore
    move.l  (sp),a0
    jsr     _LVOReleaseSemaphore(a6)
    movem.l (sp)+,a0/a6
    bra.s   SemaReadLoop


**********************************************************************
* void func_SemaFreeWrite( struct AZSema *as )
*                                         A0
**********************************************************************

_func_SemaFreeWrite
    move.l  4(sp),a0

_intAZ_SemaFreeWrite
    move.l  a6,-(sp)
    move.l  az_ExecBase(a4),a6
    jsr     _LVOReleaseSemaphore(a6)     a0 pointe sur semaphore
    bclr.b  #AZSEMB_WRITE,azse_Flags(a0)
    move.l  (sp)+,a6
    rts     ; préserve les registres

**********************************************************************
* void func_SemaWrite( struct AZSema *as )
*                                     A0
**********************************************************************

_func_SemaWrite
    move.l  4(sp),a0

_intAZ_SemaWrite
    movem.l d0/d1/a0/a1/a6,-(sp)
    move.l  az_ExecBase(a4),a6
    FORBID
.loop
    tst.w   azse_ReadCount(a0)
    beq.s   .nolock
        moveq   #5,d1               ; 1/10e sec
        move.l  az_DOSBase(a4),a6
        jsr     _LVODelay(a6)       ; fenêtre : si déjà des Read on passe ici et
        move.l  az_ExecBase(a4),a6  ;   il peut y en avoir d'autres, mais quand
        move.l  8(sp),a0            ;   on reteste le nombre est fixe car Forbid.
        bra.b   .loop
.nolock
    jsr     _LVOObtainSemaphore(a6)     a0 pointe sur semaphore
    move.l  8(sp),a0
    bset.b  #AZSEMB_WRITE,azse_Flags(a0)
    PERMIT
    movem.l (sp)+,d0/d1/a0/a1/a6
    rts     ; préserve les registres


**********************************************************************
* struct AZSema *func_SemaCreate( void )
*
**********************************************************************

_func_SemaCreate
_intAZ_SemaCreate
    movem.l d0/a6,-(sp)
    moveq   #AZSema_SIZE,d0
    jsr     _PoolAlloc
    move.l  d0,(sp)
    beq.b   .end
    move.l  d0,a0
    move.l  4.w,a6
    jsr     _LVOInitSemaphore(a6)
.end
    movem.l (sp)+,d0/a6
    rts


**********************************************************************
* func_GetAZur()
*
**********************************************************************

_func_GetAZur
    move.l  a4,d0
    rts


**********************************************************************
* intAZ_GetErrorSeverity( WORD error )
*                              d1
**********************************************************************

_intAZ_GetErrorSeverity
    lea     _ErrorCodeTable(pc),a1
    moveq   #0,d0
    move.b  0(a1,d1.w),d0           rc (code)
    rts

_intAZ_SetCmdResult
    move.l  a6,-(sp)
    move.l  az_UIKBase(a4),a6

retry_internal
    move.l  az_UIK(a4),a0
    move.l  12(sp),d0               error (msg_index)
    jsr     _LVOUIK_LangString(a6)
    move.l  d0,a0                   msg
    move.l  12(sp),d1               error (msg_index)
    lea     _ErrorCodeTable(pc),a1
    move.b  0(a1,d1.w),d0           rc (code)
    move.l  8(sp),a1                co
    bra.s   _func_SetCmdResult_com


*****************************************************************************
* func_SetCmdResult( struct CmdObj *co, ULONG rc, WORD error, UBYTE *msg )
*
*****************************************************************************

_func_SetCmdResult
    move.l  a6,-(sp)
    move.l  az_UIKBase(a4),a6
    move.l  20(sp),a0               msg
    move.l  16(sp),d1               error (msg_index)
    move.l  12(sp),d0               rc (code)
    move.l  8(sp),a1                co

    ; si msg == 0 on peut utiliser les messages internes de AZur
    cmp.l   #0,a0
    bne.s   _func_SetCmdResult_com
    move.l  d0,-(sp)
    move.l  #EndErrorCodeTable,d0
    sub.l   #_ErrorCodeTable,d0     --> code error max
    cmp.w   d0,d1
    move.l  (sp)+,d0
    bhi.s   _func_SetCmdResult_com

    move.l  d1,12(sp)
    bra.s   retry_internal



_func_SetCmdResult_com
    cmp.b   az_MaxError(a4),d0
    bcs.s   .5
    move.b  d0,az_MaxError(a4)
.5
    cmp.l   #0,a1                   co == 0 ?
    beq.s   .10
    move.b  d0,aco_ResCode(a1)
.10
    cmp.w   #-1,d1
    beq.s   .nocode
    move.w  d1,az_RexxError(a4)
    move.w  d1,az_LastError(a4)
.nocode

    move.l  a0,d0
    beq.s   .end                    pas de texte ?

    movem.l d1/a0-a1,-(sp)
    move.l  a0,a1                   a0 = msg
.loop   tst.b   (a1)+
        bne.s   .loop
    suba.l a0,a1
    subq.w #1,a1
    move.l a1,d0                    d0 = strlen(msg)

    lea     az_RexxResult(a4),a1    a1 = &uikbuffer
    jsr     _LVOBufSetS(a6)         BufSetS
    movem.l (sp)+,d1/a0-a1

    ;------ Depuis un objet de commande ?
    move.l  a1,d0                   co est valide ?
    beq.s   .end

    ;------ Contexte OK ?
    move.b  aco_ExeFrom(a1),d0
    ;btst    #0,d0   ; FROMMACRO
    ;bne.s   .end
    btst    #1,d0   ; FROMAREXX
    bne.s   .end
    btst    #6,d0   ; FROMMASTER
    bne.s   .end
    btst    #7,d0   ; FROMHOTKEY
    bne.s   .end

    ;------ Texte doit être affiché ?
    move.w  d1,d0
    lsr.w   #3,d0                   divise par huit -> numéro de l'octet de la table
    cmp.w   #EndShowTextTable-ShowTextTable,d0
    bhi.s   .noreq
    and.l   #7,d1                   numéro du bit
    btst    d1,ShowTextTable(pc,d0.w)
    beq.s   .noreq

    ;------ Affiche requester
    move.l  a0,-(sp)
    jsr     _func_ReqShowText
    addq.w  #4,sp
.noreq
.end
    move.l  (sp)+,a6
    rts

ShowTextTable
    dc.b    %00100000
            ; 0
            ; TEXT_COPYRIGHT      1
            ; TEXT_WINHELP_TITLE  2
            ; TEXT_MSG_HELP       3
            ; TEXT_UnTitled       4
            ; TEXT_ERR_NotFound   5
            ; TEXT_REQ_OpenFile       6
            ; TEXT_REQ_SaveFile       7

    dc.b    %11100000
            ; TEXT_REQ_BKupFile       8
            ; TEXT_REQ_Text2File      9
            ; TEXT_REQ_Block2File     10
            ; TEXT_PARSE_ERROR        11
            ; TEXT_PARSE_ERRORBASE    12
            ;
            ;
            ;

    dc.b    %10111111
            ;
            ;
            ; TEXT_NOMEMORY           18
            ;
            ;
            ;
            ; TEXT_PARSE_COMMENT      22
            ; TEXT_OBJ_MEMORYOBJ      23

    dc.b    %11000001
            ; TEXT_OBJ_NOTFOUND       24
            ; TEXT_MODIFIED           25
            ; TEXT_QUITVERIFY         26
            ; TEXT_OPENVERIFY         27
            ; TEXT_CLEARVERIFY        28
            ; TEXT_CMDCANCELLED       29
            ; TEXT_ERR_OPENFILE       30
            ; TEXT_ERR_READFILE       31

    dc.b    %00000000
            ; TEXT_YES                32
            ; TEXT_NO                 33
            ; TEXT_CANCEL             34
            ; TEXT_OPEN               35
            ; TEXT_NEW                36
            ; TEXT_GADREMPLACER       37
            ; TEXT_GADSAUTER          38
            ; TEXT_CHANGER            39

    dc.b    %00101000
            ; TEXT_PAR                40
            ; TEXT_ASQREQ_LoadFile    41
            ; TEXT_ASQREQ_LoadTitle   42
            ; TEXT_CannotExeCmd       43
            ; TEXT_ResultOf           44
            ; TEXT_IllegalFunc        45
            ; TEXT_Char_Sensitive     46
            ; TEXT_Char_Insensitive   47

    dc.b    %00000000
            ; TEXT_Char_Add           48
            ; TEXT_Char_Overlay       49
            ; TEXT_Char_Modified      50
            ; TEXT_Char_Unmodified    51
            ; TEXT_Absolute           52
            ; TEXT_Relative           53
            ; TEXT_Line               54
            ; TEXT_Column             55

    dc.b    %11111110
            ; TEXT_MsgLineCol         56
            ; TEXT_ExeObj_Macro       57
            ; TEXT_ExeObj_ARexx       58
            ; TEXT_ExeObj_Menu        59
            ; TEXT_ExeObj_Key         60
            ; TEXT_ExeObj_Master      61
            ; TEXT_ExeObj_HotKey      62
            ; TEXT_ExeObj_Modify      63

    dc.b    %11110111
            ; TEXT_ERR_OPENWRITEFILE  64
            ; TEXT_ERR_WRITEFILE      65
            ; TEXT_CmdNotRegistered   66
            ; TEXT_BadPosition        67
            ; TEXT_ERR_OpenConsole    68
            ; TEXT_ERR_ConsoleInUse   69
            ; TEXT_GermNotFound       70
            ; TEXT_GermInUse          71

    dc.b    %01101111
            ; TEXT_MaxLinesReached    72
            ; TEXT_BadVersion         73
            ; TEXT_ERR_BackupFile     74
            ; TEXT_ERR_BadValue       75
            ; TEXT_Title_LoadParms    76
            ; TEXT_ERR_NoRecMacro     77
            ; TEXT_ERR_ParmLine       78
            ; TEXT_REMPLACER          79

    dc.b    %00000000
            ; TEXT_CHERCHER           80
            ; TEXT_GADCHERCHER        81
            ; TEXT_INFORMATION        82
            ; TEXT_VERSION            83
            ; TEXT_AZURNAME           84
            ; TEXT_SELECTFONT         85
            ; TEXT_PREFSPRINT         86
            ; TEXT_NUMBER             87

    dc.b    %10000001
            ; TEXT_NUMBERINFO         88
            ; TEXT_GADOK              89
            ; TEXT_PREFSDISPLAY       90
            ; TEXT_PREFSEDIT          91
            ; TEXT_PREFSFILE          92
            ; TEXT_HEXSTRING          93
            ; TEXT_HEXINFO            94
            ; TEXT_FONTTOOHIGH        95

    dc.b    %11110111
            ; TEXT_ExeObj_Mouse       96
            ; TEXT_ExeObj_Joystick    97
            ; TEXT_PROPFONT           98
            ; TEXT_AZurScreenTitle    99
            ; TEXT_ERROR_CreateTask  100
            ; TEXT_NoBlockDefined    101
            ; TEXT_ERR_OpenClip      102
            ; TEXT_ERR_WriteClip     103

    dc.b    %11000011
            ; TEXT_ERR_ReadClip      104
            ; TEXT_ERR_BadClipUnit   105
            ; TEXT_REQ_File2CB       106
            ; TEXT_REQ_CB2File       107
            ; TEXT_REQ_File2FileSrc  108
            ; TEXT_REQ_File2FileDst  109
            ; TEXT_ERR_Nothing2Search   110
            ; TEXT_ERR_CantSendARexx    111

    dc.b    %10000001
            ; TEXT_ERR_CantSendDOS      112
            ; TEXT_Macro2Load           113
            ; TEXT_Macro2Save           114
            ; TEXT_
            ; TEXT_
            ; TEXT_
            ; TEXT_
            ; TEXT_ERR_NoConversion     119

    dc.b    %00001110
            ; TEXT_Search_FoundTimes    120
            ; TEXT_Search_NotFound      121
            ; TEXT_ERR_FirstCharNotWord 122
            ; TEXT_ERR_LastCharNotWord  123
            ; TEXT_Search_ReplaceTimes  124
            ; TEXT_CTLD_Save            125
            ; TEXT_CTLD_SaveQuit        126
            ; TEXT_CTLD_Cancel          127

    dc.b    %00000010
            ; TEXT_OffsetNumber         128
            ; TEXT_ERR_BadHexDigit      129
            ; TEXT_TitleSearch_Aire     130
            ; TEXT_TitleSearch_Document 131
            ; TEXT_TitleSearch_Bloc     132
            ; TEXT_TitleSearch_Curseur  133
            ; TEXT_TitleSearch_Arriere  134
            ; TEXT_TitleSearch_Exten    135

    dc.b    %00000000
            ; TEXT_TitleSearch_Pattern   136
            ; TEXT_TitleSearch_Convert   137
            ; TEXT_TitleSearch_Occur     138
            ; TEXT_TitleSearch_Toutes    139
            ; TEXT_TitleSearch_Prochaine 140
            ; TEXT_TitleSearch_Montrer   141
            ; TEXT_TitleSearch_Sensi     142
            ; TEXT_TitleSearch_MajMin    143

    dc.b    %11000000
            ; TEXT_TitleSearch_Accents   144
            ; TEXT_TitleSearch_Restri    145
            ; TEXT_TitleSearch_DebLig    146
            ; TEXT_TitleSearch_FinLig    147
            ; TEXT_TitleSearch_DebMot    148
            ; TEXT_TitleSearch_FinMot    149
            ; TEXT_ERR_ARexxLocked       150
            ; TEXT_ERR_MatchPairs        151

    dc.b    %11000001
            ; TEXT_ERR_TooManyBlocks     152
            ; TEXT_ARexxScript2Exe       153
            ; TEXT_ExeARexx_Parms        154
            ; TEXT_Title_DisplayMode     155
            ; TEXT_Title_SavePrefs       156
            ; TEXT_Title_LoadPrefs       157
            ; TEXT_ERR_DOSOpenConsole    158
            ; TEXT_ERR_DOSConsoleInUse   159

    dc.b    %00000000                   ;160
    dc.b    %00000000                   ;168
    dc.b    %00000000                   ;176
    dc.b    %00000000                   ;184
    dc.b    %00000000                   ;192
    dc.b    %00000000                   ;200
    dc.b    %00000000                   ;208
    dc.b    %00000000                   ;216
    dc.b    %00000000                   ;224
    dc.b    %00000000                   ;232
    dc.b    %00000000                   ;240
    dc.b    %00000000                   ;248
    dc.b    %00100010                   ;256
            ; TEXT_Env2_ConDOS       256
            ; TEXT_NoBMDefined       257
            ; TEXT_Title_BMName      258
            ; TEXT_Title_Bookmarks   259
            ; TEXT_ERR_MatchNotFound 260
            ; TEXT_ERR_RequestInUse  261
            ;
            ;

    dc.b    %01000000                   ;264
            ;
            ;
            ;
            ;
            ;
            ;
            ; TEXT_ERR_MenuKeyTwice  270
            ; TEXT_ERR_SameBkpName   271     utilisé avec ReqShowText()

    dc.b    %00001010                   ;272
            ; TEXT_ERR_SameAutsName  272     utilisé avec ReqShowText()
            ; TEXT_ERR_InvalidDir    273
            ; TEXT_Title_SelectDir   274
            ; TEXT_ERR_InvalidPri    275
            ; TEXT_Title_Priority    276
            ; TEXT_PriorityText      277
            ; TEXT_Edit2_Plis        278
            ; TEXT_Edit2_PliTexte    279

    dc.b    %00000000                   ;280
            ; TEXT_Edit2_PliFond     280
            ; TEXT_Edit2_PliSouligne 281
            ; TEXT_Edit2_PliGras     282
            ; TEXT_Edit2_PliItalique 283
            ; TEXT_InfoRequest       284
            ; TEXT_InfoTitle         285
            ;                        286
            ;                        287

    dc.b    %00000000                   ;288
    dc.b    %00000000                   ;296
    dc.b    %10000000                   ;304
            ; TEXT_Win1__Vertical     304
            ; TEXT_Win1_TitleButton   305
            ; TEXT_Win1__Palette      306
            ; TEXT_Win1_Pal2          307
            ; TEXT_Win1_Pal3          308
            ; TEXT_Win1_Pal4          309
            ; TEXT_Win1_Pal5          310
            ; TEXT_ERR_MatchQuotes    311

    dc.b    %00000000
            ; TEXT_OldIcon          312
            ; TEXT_DeleteIcon       313
            ; TEXT_OverDefaultTool  314
            ; TEXT_AutoSaveRequest  315
            ; TEXT_Char_AutoFormat  316
            ; TEXT_Char_NoFormat    317
            ; TEXT_Char_DebugSet    318
            ; TEXT_Char_NoDebug     319

    dc.b    %00001000
            ; TEXT_PowerPacker_Psw      320
            ; TEXT_Decompress_ReadErr   321
            ; TEXT_Compress_WriteErr    322
            ; TEXT_ERR_AppendCompress   323
            ; TEXT_ConfirmSave          324
            ; TEXT_ConfirmCreateIcon    325
            ;
            ;

    dc.b    %00000000
EndShowTextTable

    CNOP    0,4

_ErrorCodeTable
    dc.b  0 ; 0
    dc.b  0 ; TEXT_COPYRIGHT      1
    dc.b  0 ; TEXT_WINHELP_TITLE  2
    dc.b  0 ; TEXT_MSG_HELP       3
    dc.b  0 ; TEXT_UnTitled       4

    dc.b  5 ; TEXT_ERR_NotFound   5

    dc.b  0 ; TEXT_REQ_OpenFile       6
    dc.b  0 ; TEXT_REQ_SaveFile       7
    dc.b  0 ; TEXT_REQ_BKupFile       8
    dc.b  0 ; TEXT_REQ_Text2File      9
    dc.b  0 ; TEXT_REQ_Block2File     10

    dc.b 20 ; TEXT_PARSE_ERROR        11
    dc.b  0 ; TEXT_PARSE_ERRORBASE    12
    dc.b 20,20,20,20,20
    dc.b 20 ; TEXT_NOMEMORY           18
    dc.b 20,20,20
    dc.b  0 ; TEXT_PARSE_COMMENT      22
    dc.b 20 ; TEXT_OBJ_MEMORYOBJ      23
    dc.b 20 ; TEXT_OBJ_NOTFOUND       24

    dc.b  0 ; TEXT_MODIFIED           25
    dc.b  0 ; TEXT_QUITVERIFY         26
    dc.b  0 ; TEXT_OPENVERIFY         27
    dc.b  0 ; TEXT_CLEARVERIFY        28
    dc.b  5 ; TEXT_CMDCANCELLED       29

    dc.b 10 ; TEXT_ERR_OPENFILE       30
    dc.b 20 ; TEXT_ERR_READFILE       31

    dc.b  0 ; TEXT_YES                32
    dc.b  0 ; TEXT_NO                 33
    dc.b  0 ; TEXT_CANCEL             34
    dc.b  0 ; TEXT_OPEN               35
    dc.b  0 ; TEXT_NEW                36
    dc.b  0 ; TEXT_GADREMPLACER       37
    dc.b  0 ; TEXT_GADSAUTER          38
    dc.b  0 ; TEXT_CHANGER            39
    dc.b  0 ; TEXT_PAR                40

    dc.b  0 ; TEXT_ASQREQ_LoadFile    41
    dc.b  0 ; TEXT_ASQREQ_LoadTitle   42

    dc.b 10 ; TEXT_CannotExeCmd       43
    dc.b  0 ; TEXT_ResultOf           44
    dc.b 20 ; TEXT_IllegalFunc        45

    dc.b  0 ; TEXT_Char_Sensitive     46
    dc.b  0 ; TEXT_Char_Insensitive   47
    dc.b  0 ; TEXT_Char_Add           48
    dc.b  0 ; TEXT_Char_Overlay       49
    dc.b  0 ; TEXT_Char_Modified      50
    dc.b  0 ; TEXT_Char_Unmodified    51

    dc.b  0 ; TEXT_Absolute           52
    dc.b  0 ; TEXT_Relative           53
    dc.b  0 ; TEXT_Line               54
    dc.b  0 ; TEXT_Column             55
    dc.b  0 ; TEXT_MsgLineCol         56

    dc.b 10 ; TEXT_ExeObj_Macro       57
    dc.b 10 ; TEXT_ExeObj_ARexx       58
    dc.b 10 ; TEXT_ExeObj_Menu        59
    dc.b 10 ; TEXT_ExeObj_Key         60
    dc.b 10 ; TEXT_ExeObj_Master      61
    dc.b 10 ; TEXT_ExeObj_HotKey      62
    dc.b  5 ; TEXT_ExeObj_Modify      63

    dc.b 10 ; TEXT_ERR_OPENWRITEFILE  64
    dc.b 20 ; TEXT_ERR_WRITEFILE      65

    dc.b  5 ; TEXT_CmdNotRegistered   66
    dc.b  5 ; TEXT_BadPosition        67
    dc.b 10 ; TEXT_ERR_OpenConsole    68
    dc.b 10 ; TEXT_ERR_ConsoleInUse   69

    dc.b  5 ; TEXT_GermNotFound       70
    dc.b 10 ; TEXT_GermInUse          71

    dc.b  5 ; TEXT_MaxLinesReached    72
    dc.b 20 ; TEXT_BadVersion         73

    dc.b 20 ; TEXT_ERR_BackupFile     74
    dc.b 10 ; TEXT_ERR_BadValue       75
    dc.b  0 ; TEXT_Title_LoadParms    76
    dc.b 10 ; TEXT_ERR_NoRecMacro     77
    dc.b  5 ; TEXT_ERR_ParmLine       78

    dc.b  0 ; TEXT_REMPLACER          79
    dc.b  0 ; TEXT_CHERCHER           80
    dc.b  0 ; TEXT_GADCHERCHER        81

    dc.b  0 ; TEXT_INFORMATION        82
    dc.b  0 ; TEXT_VERSION            83
    dc.b  0 ; TEXT_AZURNAME           84

    dc.b  0 ; TEXT_SELECTFONT         85
    dc.b  0 ; TEXT_PREFSPRINT         86
    dc.b  0 ; TEXT_NUMBER             87
    dc.b  0 ; TEXT_NUMBERINFO         88
    dc.b  0 ; TEXT_GADOK              89
    dc.b  0 ; TEXT_PREFSDISPLAY       90
    dc.b  0 ; TEXT_PREFSEDIT          91
    dc.b  0 ; TEXT_PREFSFILE          92
    dc.b  0 ; TEXT_HEXSTRING          93
    dc.b  0 ; TEXT_HEXINFO            94

    dc.b  5 ; TEXT_FONTTOOHIGH        95

    dc.b 10 ; TEXT_ExeObj_Mouse       96
    dc.b 10 ; TEXT_ExeObj_Joystick    97

    dc.b  5 ; TEXT_PROPFONT           98
    dc.b  0 ; TEXT_AZurScreenTitle    99
    dc.b 20 ; TEXT_ERROR_CreateTask  100
    dc.b  5 ; TEXT_NoBlockDefined    101
    dc.b 10 ; TEXT_ERR_OpenClip      102
    dc.b 20 ; TEXT_ERR_WriteClip     103
    dc.b 20 ; TEXT_ERR_ReadClip      104
    dc.b 10 ; TEXT_ERR_BadClipUnit   105

    dc.b  0 ; TEXT_REQ_File2CB       106
    dc.b  0 ; TEXT_REQ_CB2File       107
    dc.b  0 ; TEXT_REQ_File2FileSrc  108
    dc.b  0 ; TEXT_REQ_File2FileDst  109

    dc.b  5 ; TEXT_ERR_Nothing2Search   110
    dc.b 10 ; TEXT_ERR_CantSendARexx    111
    dc.b 10 ; TEXT_ERR_CantSendDOS      112
    dc.b  0 ; TEXT_Macro2Load           113
    dc.b  0 ; TEXT_Macro2Save           114
    dc.b  0 ; TEXT_notused10 115
    dc.b  0 ; TEXT_notused11 116
    dc.b  0 ; TEXT_notused12 117
    dc.b  0 ; TEXT_notused13 118

    dc.b  5 ; TEXT_ERR_NoConversion     119

    dc.b  0 ; TEXT_Search_FoundTimes    120
    dc.b  5 ; TEXT_Search_NotFound      121
    dc.b 10 ; TEXT_ERR_FirstCharNotWord 122
    dc.b 10 ; TEXT_ERR_LastCharNotWord  123
    dc.b  0 ; TEXT_Search_ReplaceTimes  124

    dc.b  0 ; TEXT_CTLD_Save            125
    dc.b  0 ; TEXT_CTLD_SaveQuit        126
    dc.b  0 ; TEXT_CTLD_Cancel          127
    dc.b  0 ; TEXT_OffsetNumber         128

    dc.b 10 ; TEXT_ERR_BadHexDigit      129

    dc.b  0 ; TEXT_TitleSearch_Aire     130
    dc.b  0 ; TEXT_TitleSearch_Document 131
    dc.b  0 ; TEXT_TitleSearch_Bloc     132
    dc.b  0 ; TEXT_TitleSearch_Curseur  133
    dc.b  0 ; TEXT_TitleSearch_Arriere  134
    dc.b  0 ; TEXT_TitleSearch_Exten    135
    dc.b  0 ; TEXT_TitleSearch_Pattern  136
    dc.b  0 ; TEXT_TitleSearch_Convert  137
    dc.b  0 ; TEXT_TitleSearch_Occur    138
    dc.b  0 ; TEXT_TitleSearch_Toutes   139
    dc.b  0 ; TEXT_TitleSearch_Prochaine 140
    dc.b  0 ; TEXT_TitleSearch_Montrer  141
    dc.b  0 ; TEXT_TitleSearch_Sensi    142
    dc.b  0 ; TEXT_TitleSearch_MajMin   143
    dc.b  0 ; TEXT_TitleSearch_Accents  144
    dc.b  0 ; TEXT_TitleSearch_Restri   145
    dc.b  0 ; TEXT_TitleSearch_DebLig   146
    dc.b  0 ; TEXT_TitleSearch_FinLig   147
    dc.b  0 ; TEXT_TitleSearch_DebMot   148
    dc.b  0 ; TEXT_TitleSearch_FinMot   149

    dc.b 20 ; TEXT_ERR_ARexxLocked      150
    dc.b 10 ; TEXT_ERR_MatchPairs       151
    dc.b 10 ; TEXT_ERR_TooManyBlocks    152
    dc.b  0 ; TEXT_ARexxScript2Exe      153
    dc.b  0 ; TEXT_ExeARexx_Parms       154
    dc.b  0 ; TEXT_Title_DisplayMode    155
    dc.b  0 ; TEXT_Title_SavePrefs      156
    dc.b  0 ; TEXT_Title_LoadPrefs      157
    dc.b 10 ; TEXT_ERR_DOSOpenConsole   158
    dc.b 10 ; TEXT_ERR_DOSConsoleInUse  159
    dcb.b           256-159,0
    dc.b  5 ; TEXT_NoBMDefined       257
    dc.b  0 ; TEXT_Title_BMName      258
    dc.b  0 ; TEXT_Title_Bookmarks   259
    dc.b  5 ; TEXT_ERR_MatchNotFound 260
    dc.b 10 ; TEXT_ERR_RequestInUse  261
    dcb.b           269-261,0
    dc.b  5 ; TEXT_ERR_MenuKeyTwice  270
    dc.b  5 ; TEXT_ERR_SameBkpName   271
    dc.b  5 ; TEXT_ERR_SameAutsName  272
    dc.b 10 ; TEXT_ERR_InvalidDir    273
    dc.b  0 ; TEXT_Title_SelectDir   274
    dc.b 10 ; TEXT_ERR_InvalidPri    275
    dc.b  0 ; TEXT_Title_Priority    276
    dc.b  0 ; TEXT_PriorityText      277
    dc.b  0 ; TEXT_Edit2_Plis        278
    dc.b  0 ; TEXT_Edit2_PliTexte    279
    dc.b  0 ; TEXT_Edit2_PliFond     280
    dc.b  0 ; TEXT_Edit2_PliSouligne 281
    dc.b  0 ; TEXT_Edit2_PliGras     282
    dc.b  0 ; TEXT_Edit2_PliItalique 283
    dc.b  0 ; TEXT_InfoRequest       284
    dc.b  0 ; TEXT_InfoTitle         285
    dcb.b           310-285,0
    dc.b  5 ; TEXT_ERR_MatchQuotes   311
    dc.b  0 ; TEXT_OldIcon           312
    dc.b  0 ; TEXT_DeleteIcon        313
    dc.b  0 ; TEXT_OverDefaultTool   314
    dc.b  0 ; TEXT_AutoSaveRequest   315
    dc.b  0 ; TEXT_Char_AutoFormat   316
    dc.b  0 ; TEXT_Char_NoFormat     317
    dc.b  0 ; TEXT_Char_DebugSet     318
    dc.b  0 ; TEXT_Char_NoDebug      319
    dc.b  0 ; TEXT_PowerPacker_Psw      320
    dc.b  0 ; TEXT_Decompress_ReadErr   321
    dc.b  0 ; TEXT_Compress_WriteErr    322
    dc.b 10 ; TEXT_ERR_AppendCompress   323
    dc.b  0 ; TEXT_ConfirmSave          324
    dc.b  0 ; TEXT_ConfirmCreateIcon    325

EndErrorCodeTable

    CNOP    0,4


****************************************************************
* void func_SetQualifier( ULONG qual, ULONG on )
*
****************************************************************

_func_SetQualifier
    movem.l 4(sp),d0-d1
    move.l  az_AZMast(a4),a0
    move.l  az_KeyHandle(a0),a0
    tst.l   d1
    beq.s   .off
.on
    or.l    d0,akh_AZQual(a0)
    rts
.off
    not.l   d0
    and.l   d0,akh_AZQual(a0)
    rts

****************************************************************
* UBYTE func_CharCurrent( LONG col, ULONG len, UBYTE *ptr )
* {                            d0         d1          a0
*     if (col < len) return( ptr[col] );
*     else if (col == len) return( '\n' );
*     else return( ' ' );
* }*
****************************************************************

_func_CharCurrent
    movem.l 4(sp),d0-d1/a0

_intAZ_CharCurrent ; (d0,d1,a0)
    cmp.l   d1,d0       ; len,col
    beq.s   .equ
    bhi.s   .sup
.inf
    move.b  0(a0,d0.l),d0
    rts
.sup
    moveq   #$20,d0     ; espace
    rts
.equ ; doit tester la dernière ligne
    move.l  az_FObj(a4),a1
    move.l  aof_Text+uikpl_NodeNum(a1),d0
    subq.l  #1,d0
    asl.l   #3,d0
    move.l  aof_Text+uikpl_UNode(a1),a1
    move.l  uikn_Buf(a1,d0.l),d0
    cmp.l   a0,d0
    beq.s   .sup
.10
    moveq   #$0A,d0     ; fin ligne
    rts


*************************************************************
*   func_MatchStr( UBYTE *sub, UBYTE *str, UBYTE *start )
*
*************************************************************

_func_MatchStr
    move.l  4(sp),a0
    moveq   #0,d0
    move.b  (a0),d0
    move.b  WordCharDef(pc,d0.w),d1
    beq.s   .start          si pas un mot -> commence

    ;------ début de mot
    movem.l 8(sp),a0/a1
    cmp.l   a0,a1           str == start ?
    beq.s   .start
    move.b  -(a0),d0        si non teste le caractère précédent
    move.b  WordCharDef(pc,d0.w),d1
    bne.s   .error          si un mot -> erreur

.start
    movem.l 4(sp),a0/a1
.loop
    move.b  (a0)+,d1
    beq.s   .ok
    move.b  (a1)+,d0
    beq.s   .error
    bclr    #5,d0
    bclr    #5,d1
    cmp.b   d0,d1
    beq.s   .loop

.error
    moveq   #0,d0
    rts

.ok
    move.b  -2(a0),d0        caractère de fin
    move.b  WordCharDef(pc,d0.w),d1
    beq.s   .end            si pas un mot -> fin ok

    ;------ fin de mot
    move.b  (a1),d0         si non teste le caractère après la fin
    move.b  WordCharDef(pc,d0.w),d1
    bne.s   .error          si un mot -> erreur

.end
    moveq   #1,d0
    rts


****************************************************************
* func_CharIsWord( UWORD ch )
*
****************************************************************

_func_CharIsWord
    moveq   #0,d1
    move.b  7(sp),d1
    moveq   #0,d0
    move.b  WordCharDef(pc,d1.w),d0
    rts

WordCharDef
;          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;          |0 1 2 3 4 5 6 7 8 9 a b c d e f|
;+---------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;| 0000 00 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 0001 10 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 0010 20 |  ! " # $ % & ' ( ) * + , - . /|
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 0011 30 |0 1 2 3 4 5 6 7 8 9 : ; < = > ?|
    dc.b    1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
;| 0100 40 |@ A B C D E F G H I J K L M N O|
    dc.b    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 0101 50 |P Q R S T U V W X Y Z [ \ ] ^ _|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1
;| 0110 60 |` a b c d e f g h i j k l m n o|
    dc.b    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 0111 70 |p q r s t u v w x y z { } ~  |
    dc.b    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0
;| 1000 80 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1001 90 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1010 a0 |  ¡ ¢ £ ¤ ¥ ¦ § ¨ © ª « ¬ ­ ® ¯|
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1011 b0 |° ± ² ³ ´ µ ¶ · ¸ ¹ º » ¼ ½ ¾ ¿|
    dc.b    0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0
;| 1100 c0 |À Á Â Ã Ä Å Æ Ç È É Ê Ë Ì Í Î Ï|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 1101 d0 |Ð Ñ Ò Ó Ô Õ Ö × Ø Ù Ú Û Ü Ý Þ ß|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 1110 e0 |à á â ã ä å æ ç è é ê ë ì í î ï|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 1111 f0 |ð ñ ò ó ô õ ö ÷ ø ù ú û ü ý þ ÿ|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;+---------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


****************************************************************
* func_CharIsFile( UWORD ch )
*
****************************************************************

_func_CharIsFile
    moveq   #0,d1
    move.b  7(sp),d1
    moveq   #0,d0
    move.b  FileCharDef(pc,d1.w),d0
    rts

FileCharDef
;          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;          |0 1 2 3 4 5 6 7 8 9 a b c d e f|
;+---------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;| 0000 00 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 0001 10 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 0010 20 |  ! " # $ % & ' ( ) * + , - . /|
    dc.b    0,1,0,1,1,1,1,0,0,0,0,1,0,1,1,1
;| 0011 30 |0 1 2 3 4 5 6 7 8 9 : ; < = > ?|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0
;| 0100 40 |@ A B C D E F G H I J K L M N O|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 0101 50 |P Q R S T U V W X Y Z [ \ ] ^ _|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1
;| 0110 60 |` a b c d e f g h i j k l m n o|
    dc.b    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 0111 70 |p q r s t u v w x y z { } ~  |
    dc.b    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0
;| 1000 80 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1001 90 |               |
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1010 a0 |  ¡ ¢ £ ¤ ¥ ¦ § ¨ © ª « ¬ ­ ® ¯|
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1011 b0 |° ± ² ³ ´ µ ¶ · ¸ ¹ º » ¼ ½ ¾ ¿|
    dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
;| 1100 c0 |À Á Â Ã Ä Å Æ Ç È É Ê Ë Ì Í Î Ï|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 1101 d0 |Ð Ñ Ò Ó Ô Õ Ö × Ø Ù Ú Û Ü Ý Þ ß|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 1110 e0 |à á â ã ä å æ ç è é ê ë ì í î ï|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;| 1111 f0 |ð ñ ò ó ô õ ö ÷ ø ù ú û ü ý þ ÿ|
    dc.b    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
;+---------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


****************************************************************
* UBYTE func_NumLines( void )
****************************************************************

_func_NumLines
    move.l  az_FObj(a4),a1
    move.l  aof_Text+uikpl_NodeNum(a1),d0
    rts


****************************************************************
* UBYTE func_LineBuf( LONG line )
****************************************************************

_func_LineBuf
    move.l  4(sp),d0
_intAZ_LineBuf
    move.l  az_FObj(a4),a0
    move.l  aof_Text+uikpl_NodeNum(a0),d1
    cmp.l   #-1,d0
    beq.s   .fin
    cmp.l   d1,d0
    bcc.s   .error
    bra.s   .ok
.fin
    move.l  d1,d0
    subq.l  #1,d0
.ok
    asl.l   #3,d0
    move.l  aof_Text+uikpl_UNode(a0),a0
    move.l  uikn_Buf(a0,d0.l),d0
    rts
.error
    moveq   #0,d0
    rts


****************************************************************
* UBYTE func_LineLen( LONG line )
****************************************************************

_func_LineLen
    move.l  4(sp),d0
_intAZ_LineLen
    move.l  az_FObj(a4),a0
    move.l  aof_Text+uikpl_NodeNum(a0),d1
    cmp.l   #-1,d0
    beq.s   .fin
    cmp.l   d1,d0
    bcc.s   .error
    bra.s   .ok
.fin
    move.l  d1,d0
    subq.l  #1,d0
.ok
    asl.l   #3,d0
    move.l  aof_Text+uikpl_UNode(a0),a0
    move.l  uikn_Buf(a0,d0.l),a0
    move.l  -nubu_CurSize(a0),d0
    rts
.error
    moveq   #0,d0
    rts


****************************************************************
* UBYTE func_Buf_Len( LONG line, UBYTE **pptr )
*                          D0            A0
****************************************************************

_func_Buf_Len
    move.l  4(sp),d0
_intAZ_Buf_Len
    move.l  az_FObj(a4),a1
    move.l  aof_Text+uikpl_UNode(a1),a1
    asl.l   #3,d0
    move.l  uikn_Buf(a1,d0.l),a1
    move.l  -nubu_CurSize(a1),d0
    move.l  a1,(a0)
    rts

_intAZ_Node_Buf_Len ; ( line d0, node A0, **ptr a1 )
    asl.l   #3,d0
    move.l  uikn_Buf(a0,d0.l),a0
    move.l  -nubu_CurSize(a0),d0
    move.l  a0,(a1)
    rts


****************************************************************
* UBYTE func_LineBufLen( UBYTE *buf )
*
****************************************************************

_func_LineBufLen
    move.l  4(sp),d0
    beq.s   end
    move.l  d0,a0
_intAZ_LineBufLen
    move.l  -nubu_CurSize(a0),d0
end rts


*********************************************
*   get_node( line )
*
*********************************************

;get_node
;    move.l  az_FObj(a4),a0
;    move.l  aof_Text+uikpl_UNode(a0),a0
;    asl.l   #3,d0
;    add.l   d0,a0
;    rts


****************************************************************
* LineFlags ( line, flags )
*             D0    D1
****************************************************************

_intAZ_GetLineFlags
    move.l  az_FObj(a4),a0
    move.l  aof_Text+uikpl_UNode(a0),a0
    asl.l   #3,d0
    add.l   d0,a0
    moveq   #0,d0
    move.w  uikn_Flags(a0),d0
    rts

_intAZ_SetLineFlags
    move.l  az_FObj(a4),a0
    move.l  aof_Text+uikpl_UNode(a0),a0
    asl.l   #3,d0
    move.w  d1,uikn_Flags(a0,d0.l)
    rts

_intAZ_ORLineFlags
    move.l  az_FObj(a4),a0
    move.l  aof_Text+uikpl_UNode(a0),a0
    asl.l   #3,d0
    or.w    d1,uikn_Flags(a0,d0.l)
    rts

_intAZ_ANDLineFlags
    move.l  az_FObj(a4),a0
    move.l  aof_Text+uikpl_UNode(a0),a0
    asl.l   #3,d0
    and.w   d1,uikn_Flags(a0,d0.l)
    rts


*****************************************************************************
* func_BufAlloc( ULONG size )
*
*****************************************************************************

_func_BufAlloc
    move.l  a6,-(sp)
    move.l  az_UIKBase(a4),a6
    sub.l   a0,a0
    move.l  8(sp),d0
    jsr     _LVOBufResize(a6)
    move.l  (sp)+,a6
    rts


*****************************************************************************
* func_BufFree( UBYTE *buf )
*
*****************************************************************************

_func_BufFree
    move.l  a6,-(sp)
    move.l  az_UIKBase(a4),a6
    move.l  8(sp),a1
    jsr     _LVOBufFree(a6)
    move.l  (sp)+,a6
    rts


*******************************************************************************
*   UIKBUF *BufPrintf( UIKBUF **ubuf, ULONG start, UYTE *fmt, ... )
*
*******************************************************************************

_func_BufPrintf
    move.l  4(sp),a0
    move.l  8(sp),d0
    move.l  12(sp),d1
    lea     16(sp),a1
    exg     d1,a1
    move.l  a6,-(sp)
    move.l  az_UIKBase(a4),a6
    jsr     _LVOVBufPrintf(a6)
    move.l  (sp)+,a6
    rts


*****************************************************************************
* ULONG func_LoadGerm( UBYTE *name )
*
*****************************************************************************

_func_LoadGerm
    move.l  4(sp),a0
    jmp     _eng_NewGerm


*****************************************************************************
* ULONG func_UnloadGerm( UBYTE *name )
*
*****************************************************************************

_func_UnloadGerm
    move.l  4(sp),a0
    jmp     _eng_DisposeGerm


*****************************************************************************
* struct CmdObj *func_NewObject( UBYTE *name, struct TagItem *objtaglist )
*
*****************************************************************************

_func_NewObject
    movem.l 4(sp),a0-a1
    jmp     _eng_NewObject


*****************************************************************************
* void func_DisposeObject( struct CmdObj *co )
*
*****************************************************************************

_func_DisposeObject
    move.l  4(sp),a1
    jmp     _eng_DisposeObject


*****************************************************************************
* void func_SetModified( LONG line, LONG col )
*
*****************************************************************************

_func_SetModified
    movem.l 4(sp),d0-d1
    jmp     _SET_MODIFIED


**********************************************
*   length = func_CodeText( key, string )
*     d0                    a0     a1
***********************************************

_func_CodeText:
        movem.l 4(sp),a0-a1             a0=key, a1=string
        moveq   #0,d0

.20     tst.b   (a1)
        beq.s   .90                     fin de chaine
        move.b  (a0)+,d1
        bne.s   .30                     fin de clef
        movea.l 4(sp),a0
        move.b  (a0)+,d1

.30     eor.b   d1,(a1)+
        addq.l  #1,d0
        bra.s   .20

.90     rts

***********************************************
*   func_DecodeText( key, string, length )
*                    a0     a1      d0
***********************************************

_func_DecodeText:
        movem.l 4(sp),a0-a1             a0=key, a1=string
        move.l  12(sp),d0               d0=length
        move.l  a0,d1
        bra.s   .40

.20     move.b  (a0)+,d1
        bne.s   .30                     fin de clef
        movea.l 4(sp),a0
        move.b  (a0)+,d1

.30     eor.b   d1,(a1)+
.40     dbra    d0,.20

.90     rts


***************************************************
*   func_IsNCSubStr( UBYTE *sub, UBYTE *str )
*                           a0        a1
***************************************************

_func_IsNCSubStr
    movem.l 4(sp),a0-a1
_intAZ_IsNCSubStr
    move.b  (a0)+,d1
    beq.b   .ok
    move.b  (a1)+,d0
    beq.b   .error
    bclr    #5,d0
    bclr    #5,d1
    cmp.b   d0,d1
    beq.b   _intAZ_IsNCSubStr
.error
    moveq   #0,d0
    rts
.ok moveq   #1,d0
    rts


***************************************************
*   func_IsSubStr( UBYTE *sub, UBYTE *str )
*                     a0        a1
***************************************************

_func_IsSubStr
    movem.l 4(sp),a0-a1
_intAZ_IsSubStr
    move.b  (a0)+,d1
    beq.b   .ok
    move.b  (a1)+,d0
    beq.b   .error
    cmp.b   d0,d1
    bne.b   .error
    bra.b   _intAZ_IsSubStr
.error
    moveq   #0,d0
    rts
.ok moveq   #1,d0
    rts


***************************************************
*   func_StrNCEqual( UBYTE *sub, UBYTE *str )
*                           a0          a1
***************************************************

_intAZ_StrNCEqual
        move.b  (a1)+,d0
        beq.s   .end
        move.b  (a0)+,d1
        bclr    #5,d0
        bclr    #5,d1
        cmp.b   d1,d0
        beq.s   _intAZ_StrNCEqual
.error
        moveq   #0,d0
        rts
.end
        tst.b   (a0)
        bne.s   .error
        moveq   #1,d0
        rts


******************************************************************
*   d0 = func_DoColorMask( struct UIKObj *obj, UBYTE othercolors )
*
******************************************************************

_func_DoColorMask
    move.l  4(sp),a0
    move.l  8(sp),d0
.loop   cmp.l   #0,a0
        beq.b   .uik
        cmp.w   #UIKID_SCREEN,uo_Id(a0)
        beq.b   .screen
        move.l  uo_Parent(a0),a0
        bra.b   .loop
.screen
    lea     uosc_Colors(a0),a0
    bra.s   .common
.uik
    move.l  az_UIKBase(a4),a0
    move.l  lb_UIKPrefs(a0),a0
    lea     uikp_Colors(a0),a0

.common
    or.b    (a0)+,d0    UIKCOL_GREY
    or.b    (a0)+,d0    UIKCOL_BLACK
    or.b    (a0)+,d0    UIKCOL_WHITE
    or.b    (a0)+,d0    UIKCOL_LIGHT
    addq.w  #1,a0       pas UIKCOL_ZONE
    or.b    (a0)+,d0    UIKCOL_LOOK
    or.b    (a0)+,d0    UIKCOL_IMP

    cmp.b   #$01,d0
    bhi.b   .03
    moveq   #$01,d0     set mask
    rts
.03 cmp.b   #$03,d0
    bhi.b   .07
    moveq   #$03,d0     set mask
    rts
.07 cmp.b   #$07,d0
    bhi.b   .0f
    moveq   #$07,d0     set mask
    rts
.0f cmp.b   #$0f,d0
    bhi.b   .1f
    moveq   #$0f,d0     set mask
    rts
.1f cmp.b   #$1f,d0
    bhi.b   .3f
    moveq   #$1f,d0     set mask
    rts
.3f cmp.b   #$3f,d0
    bhi.b   .7f
    moveq   #$3f,d0     set mask
    rts
.7f cmp.b   #$7f,d0
    bhi.b   .ff
    moveq   #$7f,d0     set mask
    rts
.ff moveq   #$ff,d0     set mask
    rts


*****************************************************************


;                OBJFILE "AllocNewStack.o"

;****** AllocNewStack.asm/AllocNewStack *************************************
;
;
;    NAME
;       AllocNewStack -- allocates a new stack
;
;    SYNOPSIS
;       success = AllocNewStack( StackDataBuffer, Stacksize)
;       D0                       A0               D0
;
;       LONG    success;
;       ULONG   StackDataBuffer[3];
;       ULONG   Stacksize;
;
;    FUNCTION
;       Allocates a block of memory and initializes it as new systemstack
;       for the process. If OS 2.04 is available, exec's StackSwap() is
;       used.
;
;       The datas of the old stack are saved in the StackDataBuffer, for
;       later restoration.
;
;    INPUTS
;       NewStackBuffer = 3-longword-buffer to hold the data of the old
;               stack.
;
;       stacksize = size of the new stack in byte
;
;    RESULT
;       success (-1) or failure (0)
;
;    ATTENTION
;       You have to restore the original system stack before terminating
;       the process. Use RestoreStack().
;
;       If you're calling this routine from C language, make shure that
;       the StackDataBuffer is not allocated on the stack itself, or
;       your program will crash. You can avoid this by declaring the
;       buffer as static or gobal.
;
;
;    EXAMPLE
;       void main(void) {
;               static ULONG NewStack[3];
;
;               AllocNewStack(NewStack, 10240L);
;               .
;               .       /* do some stuff */
;               .
;               RestoreStack(NewStack);
;       }
;
;
;    SEE ALSO
;       RestoreStack()
;
;****************************************************************************

                XDEF    _AllocNewStack

_AllocNewStack:
                movem.l a2/a6,-(SP)

                move.l  a0,a2
                move.l  d0,1*4(a2)

                move.l  #$00010001,d1           ; MEMF_PUBLIC|MEMF_CLEAR
                move.l  ($4).w,a6
                jsr     -$C6(a6)               ; AllocMem()
                move.l  d0,0*4(a2)             ; new_SPLower
                beq.b   .ErrorExit
                add.l   d0,1*4(a2)             ; new_SPUpper

                move.l  1*4(a2),a1
                movem.l (SP)+,d0-d1/a0
                movem.l d0-d1/a0,-(a1)
                move.l  a1,2*4(a2)             ; new_SPReg

                cmp.w   #37,$14(a6)            ; LIB_VERSION
                bpl.b   .UseOS

                jsr     -$78(a6)               ; Multitasking verbieten,
                sub.l   a1,a1                   ; damit kein Taskwechsel er-
                jsr     -$126(a6)              ; folgt bevor wir fertig
                move.l  d0,a1                   ; sind

                move.l  $3A(a1),d0             ; SPLower mit Eintrag im
                move.l  0*4(a2),$3A(a1)       ; StackDataBuffer tauschen
                move.l  d0,0*4(a2)

                move.l  $3E(a1),d0             ; SPUpper tauschen
                move.l  1*4(a2),$3E(a1)       ;
                move.l  d0,1*4(a2)

                move.l  2*4(a2),d0             ; SPReg ist in der Task-
                move.l  d0,$36(a1)             ; struktur nicht gültig,
                move.l  SP,2*4(a2)             ; deshalb den SP in den
                                                ; Puffer eintragen

                move.l  d0,SP                   ; Stack tauschen!
                jsr     -$7E(a6)
                bra.b   .Exit

.UseOS          move.l  a2,a0
                jsr     -$2DC(a6)              ; StackSwap()

.Exit           moveq   #-1,d0
.ErrorExit      movem.l (SP)+,a2/a6
                rts




;****** AllocNewStack.asm/RestoreStack **************************************
;
;
;    NAME
;       RestoreStack -- restores a systemstack exchanged with
;               AllocNewStack()
;
;    SYNOPSIS
;       RestoreStack( StackDataBuffer)
;                     A0
;
;       ULONG   StackDataBuffer[3];
;
;    FUNCTION
;       The actual stack is exchanged with the one described by the
;       StackDataBuffer and the memoryblock is freed.
;
;    INPUTS
;       StackDataBuffer - pointer to the buffer initialized by
;               AllocNewStack()
;
;    RESULT
;       none
;
;    SEE ALSO
;       AllocNewStack()
;
;****************************************************************************


                XDEF    _RestoreStack

_RestoreStack:
                move.l  2*4(a0),a1
                move.l  (SP)+,-(a1)             ; Register und RTS-Adresse
                movem.l a2/a6,-(a1)             ; auf dem Systemstack (!!)
                move.l  a1,2*4(a0)             ; retten

                move.l  a0,a2
                move.l  ($4).w,a6

                cmp.w   #37,$14(a6)            ; LIB_VERSION
                bpl.b   .UseOS

                jsr     -$78(a6)               ; Disable()
                sub.l   a1,a1
                jsr     -$126(a6)              ; FindTask()
                move.l  d0,a0

                move.l  $3A(a0),a1             ; SPLower
                move.l  0*4(a2),$3A(a0)

                move.l  $3E(a0),d0             ; SPUpper
                move.l  1*4(a2),$3E(a0)

                move.l  2*4(a2),$36(a0)       ; SPReg
                move.l  2*4(a2),SP

                jsr     -$7E(a6)               ; Enable()
                bra.b   .GoOn

.UseOS          jsr     -$2DC(a6)              ; StackSwap()

                move.l  0*4(a2),a1             ; Werte sind nur gültig,
                move.l  1*4(a2),d0             ; wenn die OS-Routinen zum
.GoOn           sub.l   a1,d0                   ; Tauschen benutzt wurden!
                jsr     -$D2(a6)               ; FreeMem()

                movem.l (SP)+,a2/a6
                rts



*****************************************************************
        END




*******************************
* func_
*
*******************************

_

    rts



_func_Char2Upper:
        move.l  4(sp),d0
        btst    #5,d0
        beq.s   .end            ; bit 5 already clear
        move.b  d0,d1
        and.w   #7,d1
        lsr.w   #3,d0
        btst    d1,caseList(pc,d0.w)
        beq.s   .end
        move.l  4(sp),d0
        andi.b  #%11011111,d0   ; convert to upper case
.end    rts

caseList:
        dc.b    0,0                     ; Colonne 0
        dc.b    0,0                     ; Colonne 1
        dc.b    0,0                     ; Colonne 2
        dc.b    0,0                     ; Colonne 3
        dc.b    %11111110,%11111111     ; Colonne 4 (bit 5=0)
        dc.b    %11111111,%00000111     ; Colonne 5 (bit 5=0)
        dc.b    %11111110,%11111111     ; Colonne 6
        dc.b    %11111111,%00000111     ; Colonne 7
        dc.b    0,0                     ; Colonne 8
        dc.b    0,0                     ; Colonne 9
        dc.b    0,0                     ; Colonne A
        dc.b    0,0                     ; Colonne B
        dc.b    %11111111,%11111111     ; Colonne C (bit 5=0)
        dc.b    %11111110,%11111110     ; Colonne D (bit 5=0)
        dc.b    %11111111,%11111111     ; Colonne E
        dc.b    %01111110,%01111111     ; Colonne F



****************************************************************************************
*   UBYTE *func_CharNext( UBYTE *ptr, LONG *plen, LONG *pline, LONG *pcol, UBYTE *pch )
*   {
*     LONG line=*pline, col=*pcol;
*     struct UIKPList *pl = &AGP.FObj->Text;
*
*       if (line == pl->NodeNum - 1) // cas spécial car la dernière ligne n'a pas de \n
*           {
*           if (++col >= *plen) { col = *plen; ptr = (APTR)-1; }
*           else *pch = intAZ_CharCurrent( col, *plen, ptr );
*           }
*       else if (col >= *plen)
*           {
*           ptr = pl->UNode[++line].Buf; col = 0; *plen = BUFLEN(ptr);
*           *pch = intAZ_CharCurrent( col, *plen, ptr );
*           }
*       else *pch = intAZ_CharCurrent( ++col, *plen, ptr );
*
*       *pline = line; *pcol = col;
*       return( ptr );
*   }
****************************************************************************************

_func_CharNext
    movem.l d2-d3/a2-a3/a5-a6,-(sp)
    move.l  4+6*4(sp),a1
    movem.l 4+7*4(sp),a2-a3/a5-a6
    exg     a1,a5               a1=plen, a2=pline, a3=pcol, a5=ptr, a6=pch

    move.l  (a2),d2             line = *pline;
    move.l  (a3),d3             col = *pcol;
    move.l  az_FObj(a4),a0      a0 = AGP.FObj

    move.l  aof_Text+uikpl_NodeNum(a1),d0
    subq.l  #1,d0
    cmp.l   d0,d2               if (line == pl->NodeNum - 1)
    bne.s   .avant_fin              {
        addq.l  #1,d3
        cmp.l   (a1),d3             if (++col >= *plen)
        bcs.s   .optimise               {
        move.l  (a1),d3                 col = *plen;
        moveq   #-1,d0                  ptr = (APTR)-1;
        bra.s   .enderror               }
.avant_fin
    cmp.l   (a1),d3             else if (col >= *plen)
    bcs.s   .else                   {
        addq.l  #1,d2                   ++line;
        move.l  d2,d0
        asl.l   #3,d0
        move.l  aof_Text+uikpl_UNode(a0),a0
        move.l  uikn_Buf(a0,d0.l),a5    ptr = pl->UNode[++line].Buf;
        moveq   #0,d3                   col = 0;
        move.l  -nubu_CurSize(a5),(a1)  *plen = BUFLEN(ptr);
        bra.s   .charcurrent        }
.else
    addq.l  #1,d3               ++col

.charcurrent
    cmp.l   (a1),d3
    bcs.s   .optimise
.noopt
    move.l  d3,d0
    move.l  (a1),d1
    move.l  a5,a0
    bsr.s   _intAZ_CharCurrent  *pch = intAZ_CharCurrent( col, *plen, ptr );
    move.b  d0,(a6)
    bra.s   .end
.optimise
    move.b  0(a5,d3.l),(a6)

.end
    move.l  a5,d0               return( ptr );
.enderror
    move.l  d2,(a2)             *pline = line;
    move.l  d3,(a3)             *pcol = col;
    movem.l (sp)+,d2-d3/a2-a3/a5-a6
    rts


****************************************************************
* BufFlags ( buffer, flags )
*            A0      D1
****************************************************************

_intAZ_GetBufFlags
    moveq   #0,d0
    move.w  -nubu_UserUWord(a0),d0
    rts

_intAZ_SetBufFlags
    move.w  d1,-nubu_UserUWord(a0)
    rts

_intAZ_ORBufFlags
    or.w    d1,-nubu_UserUWord(a0)
    rts

_intAZ_ANDBufFlags
    and.w   d1,-nubu_UserUWord(a0)
    rts



****************************************************************
*
        SECTION data,DATA
*
****************************************************************/

