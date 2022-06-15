        IFND    AZURDEFS_OBJ

AZURDEFS_OBJ SET     1
**
**      Copyright (C) 1993 Jean-Michel Forgeas
**              Tous Droits Réservés
**

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND UTILITY_TAGITEM_I
    INCLUDE "utility/tagitem.i"
    ENDC

    IFND ARG3_PARSE
    INCLUDE "lci:arg3.i"
    ENDC

;------------------------------------------------

 STRUCTURE  CmdFunc,0
    FPTR        acf_Func
    APTR        acf_Parm
    LABEL   CmdFunc_SIZE

;------------------------------------------------

 STRUCTURE  CmdGerm,0

    ;------------------------- Shared with CmdObj
    APTR        acg_RegGerm

    ;----- execution env ------
    UWORD       acg_padw
    UBYTE       acg_ExeFrom
    UBYTE       acg_ResCode
    ULONG       acg_Flags

    ;------------------------- Germ spécific
    APTR        acg_Actions
    ULONG       acg_UsageCount
    ULONG       acg_Segment
    APTR        acg_Names
    UWORD       acg_ObjSize
    UWORD       acg_Id
    UWORD       acg_Version
    UWORD       acg_Revision
    APTR        acg_CmdTemplate
    ULONG       acg_UserData
    UBYTE       acg_Items
    UBYTE       acg_padb
    ;ULONG       acg_SemaCount
    LABEL   CmdGerm_SIZE

;------ Flags

AZAFF_DOMODIFY    equ $0001  ; Y modifie le texte (interdit si readonly)
AZAFF_OKINMACRO   equ $0002  ; O est autorisée depuis une macro
AZAFF_OKINAREXX   equ $0004  ; A est autorisée depuis une commande ARexx
AZAFF_OKINMENU    equ $0008  ; M est autorisée depuis un menu
AZAFF_OKINKEY     equ $0010  ; K est autorisée depuis une touche
AZAFF_OKINMOUSE   equ $0020  ; S est autorisée depuis la souris
AZAFF_OKINJOY     equ $0040  ; J est autorisée depuis le soystick
AZAFF_OKINMASTER  equ $0080  ; T est autorisée depuis le Master Process
AZAFF_OKINHOTKEY  equ $0100  ; I est autorisée depuis l'input.device
AZAFM_OKMASK      equ $01ff  ; masque des bit précédents

AZAFF_INTERNAL    equ $40000000
AZAFF_EXECUTING   equ $80000000  ; cette commande est en cours d'execution

AZAFM_UserFlags   equ $00ff0000 ; 8 bits sont laissés à l'usage du programmeur

AZAFB_DOMODIFY    equ 0
AZAFB_OKINMACRO   equ 1
AZAFB_OKINAREXX   equ 2
AZAFB_OKINMENU    equ 3
AZAFB_OKINKEY     equ 4
AZAFB_OKINMOUSE   equ 5
AZAFB_OKINJOY     equ 6
AZAFB_OKINMASTER  equ 7
AZAFB_OKINHOTKEY  equ 8

AZAFB_INTERNAL    equ 30
AZAFB_EXECUTING   equ 31


;------------------------------------------------

 STRUCTURE  CmdObj,0
    ;------------------------- Shared with CmdGerm
    APTR        aco_RegGerm

    ;----- execution env ------
    UWORD       aco_padw
    UBYTE       aco_ExeFrom
    UBYTE       aco_ResCode
    ULONG       aco_Flags

    ;------------------------- Object spécific
    ULONG       aco_UserData
    APTR        aco_ArgRes
    APTR        aco_ExtraBuf
    STRUCT      aco_PArg,ParseArgs_SIZEOF
    LABEL   CmdObj_SIZE

;------ Flags

AZCOM_OKMASK    equ $000001ff   ; ActionNew recopie AZAFM_OKMASK des flags du germe

AZCOF_EXECUTING equ $80000000

AZCOB_EXECUTING equ 31

AZCOM_UserFlags equ $00ff0000   ; 8 bits sont laissés à l'usage du programmeur

;----------- ExeFrom --------------

FROMPGM     equ $00  ; vient du programme (autre)
FROMMACRO   equ $01  ; vient d'une macro
FROMAREXX   equ $02  ; vient de ARexx
FROMMENU    equ $04  ; vient d'un menu
FROMKEY     equ $08  ; vient d'une touche
FROMMOUSE   equ $10  ; vient de la souris
FROMJOY     equ $20  ; vient du joystick
FROMMASTER  equ $40  ; vient du master process
FROMHOTKEY  equ $80  ; vient de l'input.device


;*******************************************************
 *
 *      Actions
 *
 *******************************************************

;----- Action types

AZA_LOAD        equ 0
AZA_UNLOAD      equ 1
AZA_NEW         equ 2
AZA_DISPOSE     equ 3
AZA_INITMENU    equ 4
AZA_SETMENU     equ 5
AZA_DO          equ 6
AZA_UNDO        equ 7

AZA_NUMACTIONS  equ 8


;*******************************************************
 *
 *      Tags
 *
 *******************************************************

;----- Les Tags avec AZT_USER ne sont pas utilisés par AZur

AZT_USER    equ (TAG_USER|(1<<30))


;----- Tags utilisés par AZur

AZT_ACTION  equ (TAG_USER|(1<<29))
AZT_GEN     equ (TAG_USER|(1<<28))
AZT_OBJ     equ (TAG_USER|(1<<27))


;=================== Tags génériques (pour germe) =================

AZT_ActionLoad          equ (AZT_ACTION|$0000)
AZT_ActionUnload        equ (AZT_ACTION|$0001)
AZT_ActionNew           equ (AZT_ACTION|$0002)
AZT_ActionDispose       equ (AZT_ACTION|$0003)
AZT_ActionInitMenu      equ (AZT_ACTION|$0004)
AZT_ActionSetMenu       equ (AZT_ACTION|$0005)
AZT_ActionDo            equ (AZT_ACTION|$0006)
AZT_ActionUndo          equ (AZT_ACTION|$0007)

AZT_Gen_Name            equ (AZT_GEN|$0000)
AZT_Gen_Size            equ (AZT_GEN|$0001)
AZT_Gen_Id              equ (AZT_GEN|$0002)
AZT_Gen_VersionRevision equ (AZT_GEN|$0003)
AZT_Gen_UserData        equ (AZT_GEN|$0004)
AZT_Gen_ArgTemplate     equ (AZT_GEN|$0005)
AZT_Gen_Flags           equ (AZT_GEN|$0006)


;=================== Tags pour création d'objet ===================

AZT_Flags               equ (AZT_OBJ|$0000)
AZT_UserData            equ (AZT_OBJ|$0001)
AZT_CmdLine             equ (AZT_OBJ|$0002)
AZT_ExtraBuf            equ (AZT_OBJ|$0003)
AZT_ExeFrom             equ (AZT_OBJ|$0004)
AZT_MinVerRev           equ (AZT_OBJ|$0005)


    ENDC
