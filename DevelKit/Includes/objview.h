#ifndef AZURDEFS_OBJVIEW

#define AZURDEFS_OBJVIEW
/*
 *      Copyright (C) 1992 Jean-Michel Forgeas
 *              Tous Droits Réservés
 */


#ifndef UIKDEFS_OBJ
#include "uiki:uikobj.h"
#endif


struct AZObjView
    {
    struct UIKObj   Obj;

    //struct UIKObj   *Stat;
    ULONG           padl;
    struct AZObjZone *Zone;

    struct UIKObj   *VUpDown1;
    struct UIKObj   *VBut1;
    struct UIKObj   *VUpDown2;
    struct UIKObj   *VBut2;
    struct UIKObj   *VProp;

    struct UIKObj   *Split;

    struct UIKObj   *HLeftRight1;
    struct UIKObj   *HBut1;
    struct UIKObj   *HLeftRight2;
    struct UIKObj   *HBut2;
    struct UIKObj   *HProp;

    UBYTE           ArrowTimer;
    UBYTE           padb;
    UWORD           padw;

    struct AZObjView *PrevView;
    struct AZObjView *NextView;

    ULONG OldVPos, OldHPos;
    };

/*-------Fonctions internes publiques------*/

#define AZFUNC_VIEW_            0

#define AZFUNC_VIEW_LastCmd     0


/*-----------------------------------------*/

/*----View Flags----*/

/*---------Obj View Tags---------- */

/*---------Obj View Flags---------- */

/*-- */

#endif
