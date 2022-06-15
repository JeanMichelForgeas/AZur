#ifndef AZURDEFS_OBJFILE

#define AZURDEFS_OBJFILE
/*
 *      Copyright (C) 1992 Jean-Michel Forgeas
 *              Tous Droits Réservés
 */


struct AZBlock
    {
    LONG    LineStart, LineEnd, ColStart, ColEnd;
    UBYTE   Mode;
    UBYTE   ReservedB;
    UWORD   ReservedW;
    ULONG   ReservedL[3];
    };

/*--- Flags pour TextPrep() */
#define ARPF_CONVERT    0x00000001
#define ARPF_NOCONVERT  0x00000002
#define ARPF_RECT       0x00000004
#define ARPF_NORECT     0x00000008
#define ARPF_OVERLAY    0x00000010
#define ARPF_NOOVERLAY  0x00000020
#define ARPF_CONTINUE   0x80000000

struct AZObjFile
    {
    UBYTE               Obj[228];

    UBYTE               *OpenFileName;
    UBYTE               *SaveFileName;
    ULONG               Reserved0;
    ULONG               Reserved1;
    UBYTE               *BkupFileName;

    struct UIKObjWinFileSel *OpenFS;
    struct UIKObjWinFileSel *SaveFS;
    ULONG               MaxLines;

    UBYTE               BlinkCount;
    UBYTE               ScrollLen;
    UWORD               SysQuals;

    UBYTE               Text[24];       /* struct UIKPList */

    ULONG               Flags;

    struct UIKObjWindow *WO;
    struct List         ViewList;

    ULONG               Reserved2;
    ULONG               Reserved3;

    LONG                Line, Col, OldLine, OldCol;
    LONG                Reserved4[1];
    LONG                LeftMargin, RightMargin;
    ULONG               ModNum;
    LONG                Reserved5[6];

    UBYTE               *SearchString;
    UBYTE               *ReplaceString;
    UBYTE               *LineString;
    UBYTE               *ColString;
    UBYTE               *ReqString;

    struct AZObjView    *ActiveView;
    struct AZViewInfo   VI;         /* copie des prefs */

    UBYTE               Reserved6[32];

    LONG                FoundLine, FoundCol;
    LONG                WorkLine, WorkCol;
    LONG                WorkReqLine, WorkReqCol;  /* func_ReqLineColumn() */

    WORD                MouseX, MouseY, OldMouseX, OldMouseY;
    ULONG               Seconds, Micros;

    UBYTE               *HexString;
    };

#define LINEFLAGS(b)    (*((UWORD*)(((ULONG)(b))-6)))
#define SETBLOCK(a,b)   ( *((UWORD*)(((ULONG)(a))-6)) &= ~0x7000; *((UWORD*)(((ULONG)(a))-6)) |= b; )

#define SETCURSOR_VISIBLE(o)    {o->Flags |= AZFILEF_CURSVISIBLE; o->BlinkCount = AGP.Prefs->BlinkOnDelay;}
#define SETCURSOR_INVISIBLE(o)  {o->Flags &= ~AZFILEF_CURSVISIBLE; o->BlinkCount = AGP.Prefs->BlinkOffDelay;}
#define PROPCHANGED(o)      (o->Flags |= AZFILEF_DRAWPROPS)
#define STATCHANGED(o)      (o->Flags |= AZFILEF_DRAWSTATS)
#define STATCHANGEDALL(o)   (o->Flags |= AZFILEF_DRAWSTATSALL)
#define UPDATE_STAT_PROP(o) (o->Flags |= (AZFILEF_DRAWSTATS|AZFILEF_DRAWPROPS))
#define UPDATE_STATALL_PROP(o) (o->Flags |= (AZFILEF_DRAWSTATSALL|AZFILEF_DRAWPROPS))
#define TST_MODIFIED(o)     (o->->Flags & AZFILEF_MODIFIED)


/*---- File Flags ----*/

#define AZFILEF_ICONIFY         0x0001
#define AZFILEF_MODIFIED        0x0002
#define AZFILEF_STARTSET        0x0004
#define AZFILEF_CURSVISIBLE     0x0008
#define AZFILEF_CURSDISABLED    0x0010
#define AZFILEF_DRAWSTATS       0x0020
#define AZFILEF_reserved        0x0040
#define AZFILEF_DRAWPROPS       0x0080
#define AZFILEF_DRAWSTATSALL    0x0100


#endif
