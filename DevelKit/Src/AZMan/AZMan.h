/*********************************************
 *      AZMan.h, Copyright (C) 1994,
 *  Patrice Trognon et Jean-Michel Forgeas
 *          Tous Droits Réservés
 *********************************************/

#define TMPFILE_FMT     "T:AZMan_%ls"

#define SIZE_AMIGA_FILENAME 32

struct Reference
    {
    UBYTE   FnctName[SIZE_AMIGA_FILENAME];
    UBYTE   Owner[SIZE_AMIGA_FILENAME];
    UBYTE   DocFileName[SIZE_AMIGA_FILENAME];
    ULONG   Offset;
    ULONG   Size;
    };
typedef struct Reference REFERENCE;

#define UpperChar(x) (((x>=0x61 && x<=0x7a) || (x>=0xe0 && x<=0xf6) || (x>=0xf8 && x<=0xfe)) ? x &= 0xdf : x)

#ifdef  LATTICE
int CXBRK(void) { return 0; }
int chkabort(void) { return 0; }
#endif
