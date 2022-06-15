
    include "dos/dos.i"

    include "uiki:uikbuf.i"
    include "uiki:uiklist.i"
    include "uiki:uikbox.i"

    include "uiki:objmenu.i"


AZUR_PRO    SET 0


    XREF    _intena

 STRUCTURE  AZSema,0
    STRUCT      azse_Semaphore,SS_SIZE
    UWORD       azse_ReadCount
    UBYTE       azse_Flags
    UBYTE       azse_padb
    LABEL   AZSema_SIZE

AZSEMB_WRITE    equ 0


 STRUCTURE  AZDisplayCmd,0
    LONG        adc_DisplayNum
    LONG        adc_DisplayFrom
    LONG        adc_ScrollNum
    LONG        adc_ScrollFrom
    LONG        adc_Line
    LONG        adc_Col
    LABEL   AZDisplayCmd_SIZE


 STRUCTURE  AZKey,0
    UWORD       ak_Code
    ULONG       ak_Qualifier
    LABEL   AZKey_SIZE

 STRUCTURE  AZShortcut,0
    APTR        aks_CO
    UWORD       aks_NumKeys
    STRUCT      aks_Keys,AZKey_SIZE
    LABEL   AZShortcut_SIZE

 STRUCTURE  KeyHandle,0
    STRUCT      akh_KeyList,UIKPList_SIZE
    APTR        akh_NoSpecial

    APTR        akh_CurKeys
    UWORD       akh_IndexCurKey
    UWORD       akh_IndexTabQual

    STRUCT      akh_RecKey,AZKey_SIZE
    ULONG       akh_AZQual

    APTR        akh_KeyFound

    STRUCT      akh_CodeKeyTab,128*8
    STRUCT      akh_ASCIIKeyTab,256*2
    APTR        akh_BeforeKey
    APTR        akh_AfterKey
    APTR        akh_BeforeLine
    APTR        akh_AfterLine

    APTR        akh_Pool
    LABEL   KeyHandle_SIZE

; reçu en paramètre par les fonctions Action_Do des objets
; mis sur BeforeKey et AfterKey
;
 STRUCTURE  KeyCom_Key,0
    UWORD       akck_Code
    ULONG       akck_Qualifier
    ULONG       akck_InputNum
    APTR        akck_InputBuf
    LABEL   KeyCom_Key_SIZE

; reçu en paramètre par les fonctions Action_Do des objets
; mis sur BeforeLine et AfterLine
;
 STRUCTURE  KeyCom_Line,0
    LONG        akcl_NewLine
    LONG        akcl_NewCol
    LONG        akcl_OldLine
    LONG        akcl_OldCol
    LABEL   KeyCom_Line_SIZE

 STRUCTURE  AZEqual,0
    APTR        ae_Equal
    APTR        ae_TrueCmd
    LABEL   AZEqual_SIZE

 STRUCTURE  EqualHandle,0
    APTR        eh_Pool
    STRUCT      eh_EqualList,UIKPList_SIZE
    LABEL   EqualHandle_SIZE

SQUAL_QUAL1     equ $00010000
SQUAL_QUAL2     equ $00020000
SQUAL_QUAL3     equ $00040000
SQUAL_LOCK1     equ $00080000
SQUAL_LOCK2     equ $00100000
SQUAL_LOCK3     equ $00200000
SQUAL_ANYSPECIAL equ $00400000

SQUAL_NOSPECIAL equ $00800000
SQUAL_NOREPEAT  equ $01000000

SQUAL_MASK  equ (IEQUALIFIER_LSHIFT|IEQUALIFIER_LALT|IEQUALIFIER_CONTROL|IEQUALIFIER_LCOMMAND|SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3|SQUAL_LOCK1|SQUAL_LOCK2|SQUAL_LOCK3|SQUAL_ANYSPECIAL)

SQUAL_DBLCLIC   equ $02000000
SQUAL_TPLCLIC   equ $04000000
SQUAL_JOYRIGHT  equ $08000000
SQUAL_JOYLEFT   equ $10000000
SQUAL_JOYDOWN   equ $20000000
SQUAL_JOYUP     equ $40000000
SQUAL_JOYFIRE   equ $80000000

SCODE_MOUSEMOVE equ $ff    ; IECODE_NOBUTTON qui n'est jamais reçu ici
SCODE_JOYRIGHT  equ $0100  ; Correspondance avec les évènements UIK !!!
SCODE_JOYLEFT   equ $0200
SCODE_JOYDOWN   equ $0400  ; BACKW
SCODE_JOYUP     equ $0800  ; FORW
SCODE_JOYFIRE   equ $1000
SCODE_MOUSETICK equ $2000
SCODE_JOYTICK   equ $4000

SQUAL_MJOY  equ (IEQUALIFIER_MIDBUTTON|IEQUALIFIER_RBUTTON|IEQUALIFIER_LEFTBUTTON|SQUAL_DBLCLIC|SQUAL_TPLCLIC|SQUAL_JOYRIGHT|SQUAL_JOYLEFT|SQUAL_JOYDOWN|SQUAL_JOYUP|SQUAL_JOYFIRE)

 STRUCTURE  AZMenuEntry,0
    UBYTE       ame_Depth
    UBYTE       ame_ComKey
    APTR        ame_Title
    ULONG       ame_Reserved
    APTR        ame_CmdObj
    ULONG       ame_MutualExclude
    APTR        ame_IntuiData
    UWORD       ame_Flags
    LABEL   AZMenuEntry_SIZE

 STRUCTURE  AZViewInfo,0
    STRUCT      azvi_ViewBox,UIKBox_SIZE
    UWORD       azvi_MinWidth
    UWORD       azvi_MinHeight
    UWORD       azvi_ZoneWidth
    UWORD       azvi_ZoneHeight
    UWORD       azvi_VButHeight
    UWORD       azvi_VArrHeight
    UWORD       azvi_HButWidth
    UWORD       azvi_HArrWidth
    LABEL   AZViewInfo_SIZE

BUMF_BUTUP      equ $0001
BUMF_UPDOWN1    equ $0002
BUMF_VERTPROP   equ $0004
BUMF_UPDOWN2    equ $0008
BUMF_BUTDOWN    equ $0010
BUMF_BUTLEFT    equ $0020
BUMF_LEFTRIGHT1 equ $0040
BUMF_HORIZPROP  equ $0080
BUMF_LEFTRIGHT2 equ $0100
BUMF_BUTRIGHT   equ $0200
BUMF_STATUSBAR  equ $0400
BUMF_CMDLINE    equ $0800

BUMF_RIGHTBORDER    equ (BUMF_BUTUP|BUMF_UPDOWN1|BUMF_VERTPROP|BUMF_UPDOWN2|BUMF_BUTDOWN)
BUMF_BOTTOMBORDER   equ (BUMF_BUTLEFT|BUMF_LEFTRIGHT1|BUMF_HORIZPROP|BUMF_LEFTRIGHT2|BUMF_BUTRIGHT)

 STRUCTURE  AZurPrefs,0
    ULONG       azp_Flags1
    ULONG       azp_Flags2
    ULONG       azp_Flags3
    ULONG       azp_SearchFlags
    ULONG       azp_EleMask
    STRUCT      azp_FontName,32
    STRUCT      azp_ViewFontName,32
    UBYTE       azp_FontHeight
    UBYTE       azp_ViewFontHeight
    UBYTE       azp_BlinkOnDelay
    UBYTE       azp_BlinkOffDelay

    UBYTE       azp_TextPen
    UBYTE       azp_BackPen
    UBYTE       azp_BlockTPen
    UBYTE       azp_BlockBPen
    UBYTE       azp_CursTextPen
    UBYTE       azp_CursBackPen
    UBYTE       azp_CursBlockTPen
    UBYTE       azp_CursBlockBPen

    STRUCT      azp_VI,AZViewInfo_SIZE

    STRUCT      azp_PatOpen,64
    STRUCT      azp_ParmFile,64

    ULONG       azp_IOBufLength
    UBYTE       azp_IndentType
    UBYTE       azp_TabLen
    UBYTE       azp_PuddleSize
    UBYTE       azp_DelClip

    STRUCT      azp_MastPortName,16
    STRUCT      azp_PortName,16
    STRUCT      azp_ARexxConsoleDesc,96
    STRUCT      azp_DOSConsoleDesc,96

    UWORD       azp_ScreenLeft
    UWORD       azp_ScreenTop
    UWORD       azp_ScreenWidth
    UWORD       azp_ScreenHeight
    ULONG       azp_ScreenModeId
    UBYTE       azp_ScreenDepth

    UBYTE       azp_BlockMode
    UBYTE       azp_BlockXLim
    UBYTE       azp_BlockYLim

    UWORD       azp_HistCmdLineMax
    UBYTE       azp_ScreenOverscan
    UBYTE       azp_CharSpace
    UBYTE       azp_CharEOL
    UBYTE       azp_CharEOF
    UBYTE       azp_CharTabS
    UBYTE       azp_CharTabE

    LONG        azp_LeftMargin
    LONG        azp_RightMargin
    STRUCT      azp_IconTool,64
    STRUCT      azp_BackupDirName,64
    STRUCT      azp_BackupName,16
    ULONG       azp_UndoMaxNum
    ULONG       azp_UndoMaxSize

    UBYTE       azp_CharRexxDot
    UBYTE       azp_Priority
    UWORD       azp_AutsIntSec

    STRUCT      azp_PosBookmarks,ibox_SIZEOF
    STRUCT      azp_PosLineCol,ibox_SIZEOF
    STRUCT      azp_PosOffset,ibox_SIZEOF
    STRUCT      azp_PosSearch,ibox_SIZEOF
    STRUCT      azp_PosReplace,ibox_SIZEOF

    STRUCT      azp_AutsDirName,64
    STRUCT      azp_AutsName,16
    UWORD       azp_AutsIntMod
    UBYTE       azp_BackupRot
    UBYTE       azp_padb
    LABEL   AZurPrefs_SIZE

MAXSIGNED   equ $7fffffff
PLUSLINES   equ 10
PLUSCOLS    equ 3

;---- BlockMode : Block types
BLKMODE_NONE    equ 0
BLKMODE_CHAR    equ 1
BLKMODE_WORD    equ 2
BLKMODE_LINE    equ 3
BLKMODE_RECT    equ 4
BLKMODE_FULL    equ 5

;---- IndentType : Indent types
NOINDENT        equ 0
INDENT_LEFT     equ 1
INDENT_STAY     equ 2
INDENT_RIGHT    equ 3
INDENT_TTX      equ 4

;----------------------------------
AZRequest_SIZEOF equ 4*8

 STRUCTURE  AZurProc,0
    ;--- commun au master et aux process
    APTR        az_UIKBase
    APTR        az_DOSBase
    APTR        az_GfxBase
    APTR        az_ExecBase
    APTR        az_LayersBase
    APTR        az_IntuitionBase
    APTR        az_UIK
    APTR        az_AZMast

    APTR        az_WinHelp
    APTR        az_ARexx
    APTR        az_Timer

    APTR        az_Prefs
    APTR        az_TmpPrefs
    APTR        az_DefPrefsName
    APTR        az_WO
    ULONG       az_padl9

    APTR        az_JumpOffsets
    STRUCT      az_FIB,fib_SIZEOF
    APTR        az_Process
    ULONG       az_CurDirLock
    UBYTE       az_OKSigBit
    UBYTE       az_ErrorSigBit
    UWORD       az_RexxError
    APTR        az_RexxResult
    APTR        az_Pool
    UBYTE       az_ExeFrom
    UBYTE       az_NullPointer
    UWORD       az_WorkFlags
    APTR        az_LastCmd

    ;--- seulement pour process
    APTR        az_FObj
    ULONG       az_UserData
    ULONG       az_RexxConsole
    APTR        az_RexxConsoleMsg
    ULONG       az_DOSConsole
    ULONG       az_NilFh
    ULONG       az_Tmp
    APTR        az_ARexxFileName
    APTR        az_DOSFileName
    APTR        az_QuickMsg
    UBYTE       az_MaxError
    UBYTE       az_padb
    UWORD       az_padw

    ;--- pour AZur et process
    APTR        az_SearchLayout
    APTR        az_Edit1Layout
    APTR        az_Edit2Layout
    APTR        az_Env1Layout
    APTR        az_Env2Layout
    APTR        az_WindowLayout
    APTR        az_ReplaceLayout
    APTR        az_SearchPrefsLayout
    STRUCT      az_LayoutReserved,4*3

    STRUCT      az_AZReqDisplayPrefs,AZRequest_SIZEOF
    STRUCT      az_AZReqEditPrefs,AZRequest_SIZEOF
    STRUCT      az_AZReqEnvPrefs,AZRequest_SIZEOF
    STRUCT      az_AZReqSearchPrefs,AZRequest_SIZEOF
    STRUCT      az_AZReqPrintPrefs,AZRequest_SIZEOF
    STRUCT      az_AZReqFontPrefs,AZRequest_SIZEOF
    STRUCT      az_AZReqWindowPrefs,AZRequest_SIZEOF
    STRUCT      az_AZReqSearch,AZRequest_SIZEOF
    STRUCT      az_AZReqReplace,AZRequest_SIZEOF
    STRUCT      az_AZReqInfo,AZRequest_SIZEOF
    STRUCT      az_AZReqAbout,AZRequest_SIZEOF
    STRUCT      az_AZReqLine,AZRequest_SIZEOF
    STRUCT      az_AZReqOffset,AZRequest_SIZEOF
    STRUCT      az_AZReqBookmark,AZRequest_SIZEOF
    STRUCT      az_AZReqHex,AZRequest_SIZEOF
    STRUCT      az_AZReqReserved,4*AZRequest_SIZEOF

    APTR        az_UBufTmp

    APTR        az_MainMenuHandle
    APTR        az_MenuHandle
    APTR        az_KeyHandle
    APTR        az_HotKeyHandle
    APTR        az_MouseHandle
    APTR        az_DictHandle
    APTR        az_TmplHandle
    APTR        az_EqualHandle

    STRUCT      az_padl2,4*5
    UWORD       az_padw2
    UWORD       az_LastError
    STRUCT      az_padl3,4*2
    LABEL   AZurProc_SIZE

 STRUCTURE  AZur,AZurProc_SIZE
    ;--- seulement pour master
    STRUCT      az_FileList,LH_SIZE
    ULONG       az_FileCount

    APTR        az_Parms

    STRUCT      az_ProcList,LH_SIZE
    STRUCT      az_ProcSem,SS_SIZE
    STRUCT      az_RGSem,SS_SIZE
    STRUCT      az_oldRGList,UIKPList_SIZE

    APTR        az_LastActNode
    APTR        az_EnvFileName
    APTR        az_ArcFileName

    APTR        az_CmdLine
    APTR        az_PArg
    APTR        az_ArgRes
    APTR        az_Parse

    APTR        az_FromName
    APTR        az_FromCmdName
    APTR        az_RGHash
    APTR        az_PatSel
    APTR        az_PubScreen
    APTR        az_ScreenAdd
    APTR        az_ScreenObj

    ULONG       az_MaxLines
    UBYTE       az_OldPri
    UBYTE       az_padb2
    UWORD       az_ProcNum
    APTR        az_CtrlDVect

    STRUCT      az_VarList,UIKList_SIZE
    STRUCT      az_LaunchPort,MP_SIZE
    APTR        az_ObjSema
    APTR        az_VarSema
    APTR        az_WinObj
    APTR        az_MenuObj
    LABEL   AZur_SIZE


;---- Evènements résultant des requesters synchrones : AZ_SmartRequest(), etc...
REQBUT_NOEVENT  equ -1
REQBUT_CANCEL   equ 0
REQBUT_OK       equ 1
REQBUT_MIDDLE   equ 2
REQBUT_CTRL_C   equ 3

;---- AZ_LaunchProc() : Flags
AZTAG_LaunchF_Iconify   equ (TAG_USER|$0001)  ; FALSE
AZTAG_LaunchF_FileReq   equ (TAG_USER|$0002)  ; FALSE
AZTAG_LaunchF_NoWindow  equ (TAG_USER|$0003)  ; FALSE
AZTAG_LaunchF_ReadOnly  equ (TAG_USER|$0004)  ; FALSE
AZTAG_LaunchF_QuitAfter equ (TAG_USER|$0005)  ; FALSE

;---- AZ_LaunchProc() : Paramètres
AZTAG_Launch_FromCmd    equ (TAG_USER|$0101)  ; commande ARexx en ligne
AZTAG_Launch_FromScript equ (TAG_USER|$0102)  ; script ARexx
AZTAG_Launch_PortName   equ (TAG_USER|$0103)  ; nom pour le port ARexx
AZTAG_Launch_Priority   equ (TAG_USER|$0104)  ; priorité pour le process
AZTAG_Launch_Pattern    equ (TAG_USER|$0105)  ; pattern pour le sélecteur de fichiers
AZTAG_Launch_FileName   equ (TAG_USER|$0106)  ; nom du fichier à charger
AZTAG_Launch_ResultPort equ (TAG_USER|$0107)  ; adresse où ranger le nom du port (UBYTE**)
AZTAG_Launch_CmdObj     equ (TAG_USER|$0108)  ; objet de commande créant la tâche
AZTAG_Launch_QuickMsg   equ (TAG_USER|$0109)  ; doit répondre ce message quand terminée
AZTAG_Launch_DirLock    equ (TAG_USER|$010a)  ; lock sur répertoire sur lequel le process va faire DupLock() puis CurrentDir()
AZTAG_Launch_LeftTop     equ (TAG_USER|$010b) ; position en pixels
AZTAG_Launch_WidthHeight equ (TAG_USER|$010c) ; taille en pixels
AZTAG_Launch_TextWidthHeight equ (TAG_USER|$010d) ; taille en caractères */

;----------------------------------

;---- process flags : Flags1 ----
AZP_CHARDELBLOCK    equ $00000001  ; La frappe d'un caractère supprime les blocs en cours
AZP_DEBUG           equ $00000002  ; Affiche des infos dans une console
AZP_BKPFILENAME     equ $00000004  ; Utilise le nom du fichier comme nom de backup
AZP_USEASL          equ $00000008  ; Utilise Le FileSelectore de l'asl.library
AZP_ERRORQUIET      equ $00000010  ; Pas de requêtes d'affichage des erreurs
AZP_SAVEICON        equ $00000020  ; Crée une icône après avoir sauvegardé
AZP_KEEPBKP         equ $00000040  ; Recopie le fichier dans T: avant de sauvegarder
AZP_OVERLAY         equ $00000080  ; Ecrit par-dessus au lieu d'insérer
AZP_STATUSLOCK      equ $00000100  ; Bloque le rafraichissement de la barre de status
AZP_notused4        equ $00000200  ;
AZP_DEFAULTBLM      equ $00000400  ; Blocks par lignes entières
AZP_MINITEL         equ $00001000  ; Indique que le port série est relié à un minitel
AZP_AREXXLOCK       equ $00002000  ; Bloque l'exécution des commandes venant de ARexx
AZP_FRONTSCREEN     equ $00004000  ; L'ouverture à été demandée sur l'écran de devant
AZP_AZURSCREEN      equ $00008000  ; L'ouverture à été demandée sur un écran AZur
AZP_READONLY        equ $00010000  ; Pas de modification possible
AZP_NOWINDOW        equ $00020000  ; Pas de fenêtre ouverte (tâche de fond)
AZP_DISPLOCK        equ $00040000  ; Affichage verrouillé
AZP_RESIDENT        equ $00080000  ; La tâche maîtresse doit rester en mémoire
AZP_OPENREQ         equ $00100000  ; Fileselector dès l'ouverture d'une fenêtre sans fichier
AZP_NOAUTOSIZE      equ $00200000  ; Pas d'adaptation des bords de la fenêtre
AZP_INPUTLOCK       equ $00400000  ; Pas d'entrée de la part de l'utilisateur
AZP_SAVE            equ $00800000  ; Sauvegarder le fichier juste avant de quitter
AZP_REQWIN          equ $01000000  ; Les requêtes sont dans des fenêtres au lieu de requesters
AZP_FULLSCREEN      equ $02000000  ; La fenêtre doit s'ouvrir en plein écran
AZP_SAVELOCK        equ $04000000  ; Bloque la commande de sauvegarde
AZP_NULLPOINTER     equ $08000000  ; Supprime le pointeur après utilisation du clavier
AZP_QUITAFTER       equ $10000000  ; Pas de UIK_Do2 après avoir exécuté les scripts ARexx en argument
AZP_REXXNAMESEP     equ $20000000  ; Met un séparateur entre le nom du port et son numéro
AZP_REXXNAMENUM     equ $40000000  ; Numérote dès le premier nom de port ARexx

AZM_NODISPLAY       equ (AZP_NOWINDOW|AZP_DISPLOCK)

;---- Preferences flags : Flags2 ----
AZP_TAB2SPACE       equ $00000001  ; Toute tabulation est transformée (load + edit) */
AZP_SPACE2TAB       equ $00000002  ; Tous espaces sont transformés en tabulation
AZP_REALTAB         equ $00000004  ; Tabulations réelles
AZP_TABDISPLAYS     equ $00000008  ; On affiche un symbole spécial pour la tabulation
AZP_TABDISPLAYE     equ $00000010  ; On affiche le vide de tabulation
AZP_SPACEDISPLAY    equ $00000020  ; Affichage des espaces en pointillés
AZP_BLINKON         equ $00000040  ; Clignotement du curseur
AZP_SHOWEOL         equ $00000080  ; Montre fins de lignes
AZP_SHOWEOF         equ $00000100  ; Montre fin de fichier
AZP_STRIPCHANGE     equ $00000200  ; Enlève les espaces de fin de ligne à chaque changement de ligne
AZP_STRIPSAVE       equ $00000400  ; Enlève les espaces de fin de ligne lors de la sauvegarde
AZP_STRIPLOAD       equ $00000800  ; Enlève les espaces de fin de ligne lors du chargement
AZP_LOADCR2LF       equ $00001000  ; Transforme CR sans LF en LF lors du chargement
AZP_LOADCRLF2LF     equ $00002000  ; Transforme CR avec LF en LF lors du chargement
AZP_SAVELF2CR       equ $00004000  ; Transforme LF en CR seul lors de la sauvegarde
AZP_SAVELF2CRLF     equ $00008000  ; Transforme LF en CR + LF lors de la sauvegarde
AZP_WORDWRAP        equ $00010000  ; Passe à la ligne par mots entiers
AZP_CORRECTCASE     equ $00020000  ; Corrige les majuscules/minuscules
AZP_USEMARGIN       equ $00040000  ; Utilise les marges
AZP_NUMBERLINES     equ $00080000  ; Numérote les lignes

;---- Preferences flags : SearchFlags ----
SEF_CASE        equ $00000001    ; Tiens compte des majuscules
SEF_not_used    equ $00000002    ;
SEF_ACCENT      equ $00000004    ; Tiens compte des accents
SEF_ALL         equ $00000008    ; Chercher tout : puis requester pour donner le total
SEF_BACKWARD    equ $00000010    ; En arrière
SEF_BLOCK       equ $00000020    ; Recherche seulement dans les blocks
SEF_TOBLOCK     equ $00000040    ; Trouvées remplacées mis en blocks
SEF_CONVERT     equ $00000080    ; Conversion type C
SEF_PATTERN     equ $00000100    ; ? = n'importe quel caractère
SEF_FULL        equ $00000200    ; Tout le texte
SEF_STARTOFLINE equ $00000400    ; Restriction au début de ligne
SEF_ENDOFLINE   equ $00000800    ; Restriction au début de ligne
SEF_NOVERIFY    equ $00001000    ; Pas de vérification avant remplacement
SEF_STARTOFWORD equ $00002000    ; Restriction au début de mot
SEF_ENDOFWORD   equ $00004000    ; Restriction au début de mot

SEF_WORD        equ (SEF_STARTOFWORD|SEF_ENDOFWORD)

SEF_VERIFYOK    equ $20000000    ; tmp : pour la verif en cours de remplacement
SEF_DOLF        equ $40000000    ; tmp : mis selon le contenu de la chaine à chercher
SEF_RECT        equ $80000000    ; tmp : mis selon le bloc

;------------------equ ------

* Commodore DBUG macro (needs debug.lib)
* Usage: pass name of format string,value,value
*        values may be register name, variablename(PC), or #value
* ex: DBUG fmtRtnD0,#testname,d0  ; format string, addr of rtn name, d0
DBUG    MACRO   * passed name of format string, with args on stack
        movem.l d0-d7/a0-a6,-(sp)
        move.l  \3,-(sp)
        move.l  \2,-(sp)
        lea     .fmt\@(pc),a0
        lea     (sp),a1
        XREF    KPrintF
        jsr     KPrintF
        addq.w  #8,sp
        movem.l (sp)+,d0-d7/a0-a6
        bra.b   .\@
.fmt\@  dc.b    \1,0
        cnop    0,4
.\@
        ENDM
