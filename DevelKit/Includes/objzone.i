        IFND    AZURDEFS_OBJZONE
AZURDEFS_OBJZONE SET     1
**
**      Copyright (C) 1993 Jean-Michel Forgeas
**                  Tous Droits Réservés
**


 STRUCTURE  AZObjZone,0
    STRUCT      aoz_Obj,228
    STRUCT      aoz_Gad,44

    ULONG       aoz_TopLine
    ULONG       aoz_Rows
    ULONG       aoz_TopCol
    ULONG       aoz_Cols
    ULONG       aoz_MaxLineLen
    ULONG       aoz_HPropLineLen
    ULONG       aoz_NewHPropLineLen

    UBYTE       aoz_TextPen
    UBYTE       aoz_BackPen
    UBYTE       aoz_BlockTPen
    UBYTE       aoz_BlockBPen
    UBYTE       aoz_CursTextPen
    UBYTE       aoz_CursBackPen
    UBYTE       aoz_CursBlockTPen
    UBYTE       aoz_CursBlockBPen

    UBYTE       aoz_Reserved;

    LABEL   AZObjZone_SIZE


        ENDC
