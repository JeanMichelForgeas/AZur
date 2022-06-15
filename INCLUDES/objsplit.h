#ifndef AZDEFS_OBJSPLIT

#define AZDEFS_OBJSPLIT
/*
 *      Copyright (C) 1993 Jean-Michel Forgeas
 *                  Tous Droits Réservés
 */

#ifndef UIKDEFS_OBJ
#include "uiki:uikobj.h"
#endif

#ifndef UIKDEFS_TAGITEM_H
#include "uiki:uiktag.h"
#endif


struct AZObjSplit
    {
    struct UIKObj           Obj;
    struct Gadget           Gad;

    ULONG                   Flags;
    ULONG                   AZurProc;

    struct UIKHook          *MBHook;
    struct RastPort         *BarRPort;
    struct RastPort         *SaveRPort;
    struct RastPort         *WorkRPort;
    WORD                    YOffset;
    WORD                    OldY;

    UBYTE                   Mask;
    };

/*-------Fonctions internes publiques------*/


/*-----------------------------------------*/

/*----SplitFlags----*/
#define SPLF_MOVESTARTED        0x0001

/*---------Obj Split Tags---------- */
#define AZT_Split_          (UIKTAG_USER|0x0001)

/*---------Obj Split Flags---------- */
#define AZTF_SplitFl_       (UIKTAG_USER|0x0020)

/*-- */

#endif
