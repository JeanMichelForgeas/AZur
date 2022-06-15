#ifndef AZURDEFS_OBJFILE

#define AZURDEFS_OBJFILE
/*
 *      Copyright (C) 1992 Jean-Michel Forgeas
 *              Tous Droits Réservés
 */

#ifndef UIKDEFS_OBJ
#include "uiki:uikobj.h"
#endif

#include "lci:objview.h"


#define AZBM_MAXNAMELEN  55
struct AZBM
    {
    LONG    Line, Col;
    UBYTE   Name[AZBM_MAXNAMELEN];
    UBYTE   Flags;
    };
#define AZBMF_DELPROOF  0x01

struct AZBlock  /* taille == puissance de 2 pour éviter multiplication */
    {
    LONG    LineStart, LineEnd, ColStart, ColEnd;
    UBYTE   Mode;
    UBYTE   Flags;
    UWORD   padw2;
    ULONG   padl[3];
    };
#define AZBF_EXTCURS    0x01

struct OpenWindowStuff
    {
    struct UIKObjWindow *newwo;
    ULONG   tag_lefttop;
    ULONG   tag_widthheight;
    ULONG   tag_textwidthheight;
    };

struct AZTextRP
    {
    ULONG   flags;
    ULONG   totlines;
    ULONG   maxlines;
    LONG    line;
    LONG    col;
    LONG    storecol;
    LONG    indcol;
    BYTE    tablen;
    BYTE    tab2sp;
    BYTE    cr_replaced;
    ULONG   padl[3];
    struct AZDisplayCmd DC;
    };

struct SearchReplace
    {
    /*----- Mémorisation continuelle */
    UIKBUF  *SearchStr;
    UIKBUF  *ReplaceStr;
    UIKBUF  *ConvSearchStr;
    UIKBUF  *ConvReplaceStr;
    ULONG   SearchFlags;
    ULONG   padl; // UndoSearchFlags;

    /*----- Paramètres de travail */
    ULONG   Flags;
    UBYTE   *SStr;
    ULONG   SLen;
    UBYTE   *RStr;
    ULONG   RLen;
    LONG    StartLine, StartCol;
    LONG    EndLine, EndCol;
    LONG    Line, Col, ELine, ECol;
    ULONG   SPatNum, RPatNum;
    };

struct SearchRepQuery
    {
    /*------ arguments : initialiser tous ces champs */
    LONG    StartLine;
    LONG    StartCol;
    LONG    EndLine;
    LONG    EndCol;
    UBYTE   *SStr;
    ULONG   SLen;
    UBYTE   *RStr;
    ULONG   RLen;
    ULONG   SearchFlags;
    UWORD   Reserved;
    UBYTE   DoDisplay;
    UBYTE   DoCursor;

    /*------ results : pas besoin d'inititialiser */
    ULONG   FoundCount;
    struct AZDisplayCmd DC;
    };

#define ARPF_CONVERT    0x00000001
#define ARPF_NOCONVERT  0x00000002
#define ARPF_RECT       0x00000004
#define ARPF_NORECT     0x00000008
#define ARPF_OVERLAY    0x00000010
#define ARPF_NOOVERLAY  0x00000020
#define ARPF_STRIPEOL   0x00000040
#define ARPF_NOSTRIPEOL 0x00000080
#define ARPF_NOCURS     0x00000100

#define ARPF_CONTINUE   0x80000000


struct AZObjFile
    {
    struct UIKObj       Obj;

    UIKBUF              *OpenFileName;
    UIKBUF              *SaveFileName;
    ULONG               padl3;
    ULONG               padl4;
    UIKBUF              *BkupFileName;

    struct UIKObjWinFileSel *OpenFS;
    struct UIKObjWinFileSel *SaveFS;
    ULONG               MaxLines;

    UBYTE               BlinkCount;
    BYTE                ScrollLen;
    UWORD               SysQuals;

    struct UIKPList     Text;

    ULONG               Flags;

    struct UIKObjWindow *WO;
    struct List         ViewList;

    void                (*CtrlDVect)();

    UBYTE               Mask;
    UBYTE               padb1;
    UWORD               padw1;

    LONG                Line, Col, OldLine, OldCol;
    struct AZBlockInfo  *BlockInfo;
    LONG                LeftMargin, RightMargin;
    ULONG               ModNum;
    ULONG               padl[4];
    struct UIKObj       *Stat;
    LONG                CEdCol;

    UIKBUF              *SearchString;
    UIKBUF              *ReplaceString;
    UIKBUF              *LineString;
    UIKBUF              *ColString;
    UIKBUF              *ReqString;

    struct AZObjView    *ActiveView;
    struct AZViewInfo   VI;         /* copie des prefs */

    UBYTE               Ind[24];    /* S I A O M U */

    LONG                FoundLine, FoundCol, FoundELine, FoundECol;
    LONG                WorkLine, WorkCol;
    LONG                WorkReqLine, WorkReqCol;  /* func_ReqLineColumn() */

    WORD                MouseX, MouseY, OldMouseX, OldMouseY;
    ULONG               Seconds, Micros;

    UIKBUF              *HexString;

    ULONG               padl8[37];

    struct UIKObjMenu   *Menu;
    struct UIKObj       *CmdLine;

    UBYTE               CurBlockMode;
    UBYTE               TickCount;
    WORD                MemButs;
    UWORD               CmdLineIndex;
    struct SearchReplace *SeRep;

    struct AZTextRP     TRP;

    struct UIKPList     CmdLineList;
    ULONG               padl9;
    LONG                LCLine;
    LONG                LCCol;
    struct AZMacroInfo  *MacInfo;
    struct AZBMInfo     *BMInfo;
    UIKBUF              *AutsFileName;

    struct MsgPort      *AppWinPort;
    struct AppWindow    *AppWin;
    APTR                AppWinVect;
    int                 (*OldWOStartAfter)();
    void                (*OldWOStop)();
    };

/* Flag pour les UIKNode->Flags */
#define UNF_BLKSTART    0x0800
#define UNF_BLKEND      0x1000
#define UNF_ENDSPACE    0x2000
#define UNF_CONTINUE    0x4000
#define UNF_FOLDED      0x8000

/*
#define LINEFLAGS(b)    (*((UWORD*)(((ULONG)(b))-6)))
#define SETBLOCK(a,b)   ( *((UWORD*)(((ULONG)(a))-6)) &= ~0x7000; *((UWORD*)(((ULONG)(a))-6)) |= b; )
*/

#define SETCURSOR_VISIBLE(o)    {o->Flags |= AZFILEF_CURSVISIBLE; o->BlinkCount = AGP.Prefs->BlinkOnDelay;}
#define SETCURSOR_INVISIBLE(o)  {o->Flags &= ~AZFILEF_CURSVISIBLE; o->BlinkCount = AGP.Prefs->BlinkOffDelay;}
#define UPDATE_REQBM(o)     (o->Flags |= AZFILEF_DRAWREQBM)
#define UPDATE_PROP(o)      (o->Flags |= AZFILEF_DRAWPROPS)
#define UPDATE_STAT(o)      (o->Flags |= AZFILEF_DRAWSTATS)
#define UPDATE_STATALL(o)   (o->Flags |= AZFILEF_DRAWSTATSALL)
#define UPDATE_STAT_PROP(o) (o->Flags |= (AZFILEF_DRAWSTATS|AZFILEF_DRAWPROPS))
#define UPDATE_STATALL_PROP(o) (o->Flags |= (AZFILEF_DRAWSTATSALL|AZFILEF_DRAWPROPS))
#define TST_MODIFIED        (AGP.FObj->Flags & AZFILEF_MODIFIED)

/*-------Fonctions internes publiques------*/

#define AZFUNC_FILE_            0

#define AZFUNC_FILE_LastCmd     0


/*-----------------------------------------*/

/*----File Flags----*/

#define AZFILEF_ICONIFY         0x0001
#define AZFILEF_MODIFIED        0x0002
#define AZFILEF_STARTSET        0x0004
#define AZFILEF_CURSVISIBLE     0x0008
#define AZFILEF_CURSDISABLED    0x0010
#define AZFILEF_DRAWSTATS       0x0020
#define AZFILEF_DRAWREQBM       0x0040
#define AZFILEF_DRAWPROPS       0x0080
#define AZFILEF_DRAWSTATSALL    0x0100
#define AZFILEF_MODAUTS         0x0200
#define AZFILEF_DOAUTOSAVE      0x0400


/*---------Obj File Tags---------- */

#define AZTAG_File_FileName         (UIKTAG_USER|0x0001)
#define AZTAG_File_TextWidthHeight  (UIKTAG_USER|0x0002)


/*---------Obj File Flags---------- */

#define AZTAG_FileFl_Iconify    (UIKTAG_USER|0x0041)


/*-- */

#endif
