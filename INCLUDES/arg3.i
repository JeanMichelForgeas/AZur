        IFND    ARG3_PARSE
ARG3_PARSE  SET 1


    STRUCTURE   ParseArgs,0
        APTR        arg_CmdLine
        LONG        arg_CmdLength
        LONG        arg_CmdCurChr
        APTR        arg_StoreList
        APTR        arg_Buffer          ; Optional string parsing space.
        LONG        arg_BufSize         ; Size of RDA_Buffer (0..n)
        UBYTE       arg_Error           ; Error code
        UBYTE       arg_SepSpace
        UWORD       arg_UserData
        LONG        arg_Flags           ; Flags for any required control
        APTR        arg_PoolHeader
        LABEL   ParseArgs_SIZEOF


    ;BITDEF  PA,STDIN,0     ; Use "STDIN" rather than "COMMAND LINE"
    BITDEF  PA,NOALLOC,1    ; If set, do not allocate extra string space.
    BITDEF  PA,PROMPT,2     ; Enable reprompting for string input.
    BITDEF  PA,WORKBENCH,3  ; If launched from Workbench
    BITDEF  PA,POOLED,4
    BITDEF  PA,ACCEPTEOL,5


MAX_TEMPLATE_ITEMS  equ     100
MAX_MULTIARGS       equ     128


AITEM_EQUAL     EQU     -2              ; "=" Symbol
AITEM_ERROR     EQU     -1              ; error
AITEM_NOTHING   EQU     0               ; *N, ;, endstreamch
AITEM_UNQUOTED  EQU     1               ; unquoted item
AITEM_QUOTED    EQU     2               ; quoted item

BIT_A_REQUIRED  equ 0   ; $01
BIT_K_KEYWORD   equ 1   ; $02
BIT_S_SWITCH    equ 2   ; $04
BIT_N_NUMBER    equ 3   ; $08
BIT_F_RESTOF    equ 4   ; $10
BIT_T_TOGGLE    equ 5   ; $20
BIT_M_MULTI     equ 6   ; $40
BIT_7_DONE      equ 7   ; $80

PAERR_OK                    equ 0
PAERR_LINE_TOO_LONG         equ 1
PAERR_BAD_TEMPLATE          equ 2
PAERR_REQARG_MISSING        equ 3
PAERR_BADARG_OR_TOOLONG     equ 4
PAERR_AFTEREQUAL_MISSING    equ 5
PAERR_NOTENOUGH_MEMORY      equ 6
PAERR_TOOMANY_ARGUMENTS     equ 7
PAERR_BAD_NUMBER            equ 8
PAERR_NO_CMDLINE            equ 9

        ENDC
