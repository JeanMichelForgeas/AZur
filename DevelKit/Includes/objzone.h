#ifndef AZURDEFS_OBJZONE

#define AZURDEFS_OBJZONE
/*
 *      Copyright (C) 1992 Jean-Michel Forgeas
 *              Tous Droits Réservés
 */


#ifndef UIKDEFS_OBJ
#include "uiki:uikobj.h"
#endif

struct AZObjZone
    {
    struct UIKObj   Obj;
    struct Gadget   Gad;

    ULONG           TopLine, Rows;
    ULONG           TopCol, Cols, MaxLineLen, HPropLineLen, NewHPropLineLen;

    UBYTE           TextPen, BackPen, BlockTPen, BlockBPen;
    UBYTE           CursTextPen, CursBackPen, CursBlockTPen, CursBlockBPen;

    UBYTE           Reserved;
    };

/*-------Fonctions internes publiques------*/

#define AZFUNC_ZONE_            0

#define AZFUNC_ZONE_LastCmd     0


/*-----------------------------------------*/

/*----Zone Flags----*/

/*---------Obj Zone Tags---------- */

#define AZTAG_Zone_     (UIKTAG_USER|0x0001)


/*---------Obj Zone Flags---------- */

/*-- */

#endif
