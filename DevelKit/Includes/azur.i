    IFND    AZURDEFS_AZUR_I
AZURDEFS_AZUR_I SET   1

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND UTILITY_TAGITEM_I
    INCLUDE "utility/tagitem.i"
    ENDC

    IFND DOS_DOS_I
    INCLUDE "dos/dos.i"
    ENDC

    IFND DEVICES_INPUTEVENT_I
    INCLUDE "devices/inputevent.i"
    ENDC

 STRUCTURE  AZDisplayCmd,0
    LONG        adc_DisplayNum
    LONG        adc_DisplayFrom
    LONG        adc_ScrollNum
    LONG        adc_ScrollFrom
    LONG        adc_Line
    LONG        adc_Col
    LABEL   AZDisplayCmd_SIZE

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
    WORD        azvi_Left
    WORD        azvi_Top
    WORD        azvi_Width
    WORD        azvi_Height
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
    ULONG       az_WO
    ULONG       az_Reserved1

    APTR        az_JumpOffsets
    STRUCT      az_FIB,fib_SIZEOF
    APTR        az_Process
    ULONG       az_CurDirLock
    UBYTE       az_Reserved2
    UBYTE       az_Reserved3
    UWORD       az_RexxError
    APTR        az_RexxResult
    APTR        az_Pool
    UBYTE       az_ExeFrom
    UBYTE       az_Reserved4
    UWORD       az_Reserved5
    APTR        az_LastCmd

    ;--- seulement pour process
    APTR        az_FObj
    ULONG       az_UserData
    LABEL   AZurProc_SIZE


;---- Evènements résultant des requesters synchrones : AZ_SmartRequest(), etc...
REQBUT_NOEVENT  equ -1
REQBUT_CANCEL   equ 0
REQBUT_OK       equ 1
REQBUT_MIDDLE   equ 2
REQBUT_CTRL_C   equ 3

;---- AZ_LaunchProc() : Flags                Défaut :
AZTAG_LaunchF_Iconify   equ (TAG_USER|$0001)  ; FALSE
AZTAG_LaunchF_FileReq   equ (TAG_USER|$0002)  ; FALSE
AZTAG_LaunchF_NoWindow  equ (TAG_USER|$0003)  ; FALSE
AZTAG_LaunchF_ReadOnly  equ (TAG_USER|$0004)  ; FALSE
AZTAG_LaunchF_QuitAfter equ (TAG_USER|$0005)  ; FALSE

;---- AZ_LaunchProc() : Paramètres
AZTAG_Launch_FromCmd    equ (TAG_USER|$0101)  ; Commande ARexx en ligne
AZTAG_Launch_FromScript equ (TAG_USER|$0102)  ; Script ARexx
AZTAG_Launch_PortName   equ (TAG_USER|$0103)  ; Nom pour le port ARexx
AZTAG_Launch_Priority   equ (TAG_USER|$0104)  ; Priorité pour le process
AZTAG_Launch_Pattern    equ (TAG_USER|$0105)  ; Pattern pour le sélecteur de fichiers
AZTAG_Launch_FileName   equ (TAG_USER|$0106)  ; Nom du fichier à charger
AZTAG_Launch_ResultPort equ (TAG_USER|$0107)  ; Adresse où ranger le nom du port (UBYTE**)
AZTAG_Launch_CmdObj     equ (TAG_USER|$0108)  ; Objet de commande créant la tâche
AZTAG_Launch_QuickMsg   equ (TAG_USER|$0109)  ; Doit répondre ce message quand terminée
AZTAG_Launch_DirLock    equ (TAG_USER|$010a)  ; Lock sur répertoire sur lequel le process va faire DupLock() puis CurrentDir()
AZTAG_Launch_LeftTop     equ (TAG_USER|$010b) ; Position d'ouverture de la fenêtre
AZTAG_Launch_WidthHeight equ (TAG_USER|$010c) ; Taille d'ouverture de la fenêtre
AZTAG_Launch_TextWidthHeight equ (TAG_USER|$010d) ; taille en caractères

;---- Preferences flags : Flags1 ----
AZP_CHARDELBLOCK    equ $00000001  ; La frappe d'un caractère supprime les blocs en cours
AZP_DEBUG           equ $00000002  ; Affiche des infos dans une console
AZP_BKPFILENAME     equ $00000004  ; Utilise le nom du fichier comme nom de backup
AZP_USEASL          equ $00000008  ; Utilise Le FileSelector de l'asl.library
AZP_ERRORQUIET      equ $00000010  ; Pas de requêtes d'affichage des erreurs
AZP_SAVEICON        equ $00000020  ; Crée une icône après avoir sauvegardé
AZP_KEEPBKP         equ $00000040  ; Recopie le fichier dans T: avant de sauvegarder
AZP_OVERLAY         equ $00000080  ; Ecrit par-dessus au lieu d'insérer
AZP_STATUSLOCK      equ $00000100  ; Bloque le rafraichissement de la barre de status
AZP_ABORTAREXX      equ $00000200  ; Retourne 20 pour la prochaine commande ARexx
AZP_DEFAULTBLM      equ $00000400  ; Blocks par lignes entières
AZP_AUTOBACKUP      equ $00000800  ; Sauvegarde automatique
AZP_AUTOSAVE        equ AZP_AUTOBACKUP
AZP_MINITEL         equ $00001000  ; Indique que le port série est relié à un minitel
AZP_AREXXLOCK       equ $00002000  ; Bloque les commandes venant de ARexx
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
AZP_QUITAFTER       equ $10000000  ; Se termine aussitôt après la fermeture de la fenêtre
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

******************************************************
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


    ENDC    ; AZURDEFS_AZUR_I
