#ifndef ARG3_PARSE
#define ARG3_PARSE

struct ParseArgs
{
    UBYTE   *CmdLine;
    LONG    CmdLength;
    LONG    CmdCurChr;
    APTR    StoreList;
    UBYTE   *Buffer;        /* Optional string parsing space. */
    LONG    BufSize;        /* Size of RDA_Buffer (0..n) */
    UBYTE   Error;
    UBYTE   SepSpace;
    UWORD   UserData;
    LONG    Flags;          /* Flags for any required control */
    APTR    PoolHeader;
};

//#define PAB_STDIN     0       /* Use "STDIN" rather than "COMMAND LINE" */
//#define PAF_STDIN     1
#define PAB_NOALLOC     1       /* If set, do not allocate extra string space.*/
#define PAF_NOALLOC     2
#define PAB_PROMPT      2       /* Enable reprompting for string input. */
#define PAF_PROMPT      4
#define PAB_WORKBENCH   3       /* if launched by Workbench. */
#define PAF_WORKBENCH   8
#define PAB_POOLED      4
#define PAF_POOLED      (1L<<4)
#define PAB_ACCEPTEOL   5       /* do not quit on $a but length or $0 */
#define PAF_ACCEPTEOL   (1L<<5)

#define MAX_TEMPLATE_ITEMS      100
#define MAX_MULTIARGS           128

#define AITEM_EQUAL     -2              /* "=" Symbol */
#define AITEM_ERROR     -1              /* error */
#define AITEM_NOTHING   0               /* *N, ;, endstreamch */
#define AITEM_UNQUOTED  1               /* unquoted item */
#define AITEM_QUOTED    2               /* quoted item */

#define BIT_A_REQUIRED  0   ; 0x01
#define BIT_K_KEYWORD   1   ; 0x02
#define BIT_S_SWITCH    2   ; 0x04
#define BIT_N_NUMBER    3   ; 0x08
#define BIT_F_RESTOF    4   ; 0x10
#define BIT_T_TOGGLE    5   ; 0x20
#define BIT_M_MULTI     6   ; 0x40
#define BIT_7_DONE      7   ; 0x80

#define PAERR_OK                    0
#define PAERR_LINE_TOO_LONG         1
#define PAERR_BAD_TEMPLATE          2
#define PAERR_REQARG_MISSING        3
#define PAERR_BADARG_OR_TOOLONG     4
#define PAERR_AFTEREQUAL_MISSING    5
#define PAERR_NOTENOUGH_MEMORY      6
#define PAERR_TOOMANY_ARGUMENTS     7
#define PAERR_BAD_NUMBER            8
#define PAERR_NO_CMDLINE            9


#endif
