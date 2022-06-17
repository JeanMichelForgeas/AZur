/****************************************************************
 *
 *      File:       funcs_pointeur.c    fonctions de pointeurs
 *      Project:    AZur                editeur de texte
 *
 *      Created:    06/05/93            Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikobj.h"

#include "uiki:objwindow.h"

#include "lci:azur.h"
#include "lci:objfile.h"
#include "lci:azur_protos.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/


/****** Statics *************************************************/

static UWORD chip RecordPointer[] = {
    0x0000, 0x0000, /* vert. and horiz. start posn. */
    0x0000, 0xffff,
    0x7ffe, 0x8001,
    0x7ffe, 0x8001,
    0x6006, 0x9ff9,
    0x6006, 0x94e9,
    0x6006, 0x9ff9,
    0x7ffe, 0x8001,
    0x0000, 0xffff,
    0x3ffc, 0xc003,
    0x3ffc, 0xc003,
    0x0000, 0x0000, /* reserved, must be NULL */
};

static UWORD chip PlayPointer[] = {
    0x0000, 0x0000, /* vert. and horiz. start posn. */
    0x0000, 0xffff,
    0x7efe, 0x8101,
    0x7e7e, 0x8181,
    0x7e3e, 0x81c1,
    0x7e1e, 0x81e1,
    0x7e3e, 0x81c1,
    0x7e7e, 0x8181,
    0x7efe, 0x8101,
    0x0000, 0xffff,
    0x0000, 0x0000, /* reserved, must be NULL */
};

/* === NullPointer Imagery =============================================== */
static UWORD chip WaitPointer[] = {
    0x0000, 0x0000, /* vert. and horiz. start posn. */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x07C0, 0x1FF8,
    0x1FF0, 0x3FEC,
    0x3FF8, 0x7FDE,
    0x3FF8, 0x7FBE,
    0x7FFC, 0xFF7F,
    0x7EFC, 0xFFFF,
    0x7FFC, 0xFFFF,
    0x3FF8, 0x7FFE,
    0x3FF8, 0x7FFE,
    0x1FF0, 0x3FFC,
    0x07C0, 0x1FF8,
    0x0000, 0x07E0,
    0x0000, 0x0000, /* reserved, must be NULL */
};

/* === NullPointer Imagery =============================================== */
static UWORD chip NullPointer[(1 * 2) + 4] = {
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
};


/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static struct Window *get_window( void )
{
    return( /*wo ? (wo->Obj.w) :*/ (/*AGP.FObj &&*/ AGP.WO ? AGP.WO->Obj.w : 0) );
}

void func_SetRecordPointer( void )
{
  struct Window *w = get_window();

    if (w && AGP.PtrType != PTRT_MACRECORD)
        {
        SetPointer( w, &RecordPointer[0], 10, 16, -1, 0 );
        AGP.PtrType = PTRT_MACRECORD;
        }
}

void func_SetPlayPointer( void )
{
  struct Window *w = get_window();

    if (w && AGP.PtrType != PTRT_MACPLAY)
        {
        SetPointer( w, &PlayPointer[0], 10, 16, -1, 0 );
        AGP.PtrType = PTRT_MACPLAY;
        }
}

static void put_wait_pointer( struct Window *w )
{
    if (((struct Library*)GfxBase)->lib_Version >= 39)
        {
        struct TagItem tl[] = {
            WA_BusyPointer, TRUE,
            WA_PointerDelay, TRUE,
            TAG_END
            };
        SetWindowPointerA( w, tl );
        }
    else{
        SetPointer( w, &WaitPointer[0], 16, 16, -6, 0 );
        }
}

static void set_wait_pointer( struct Window *w )
{
    if (AGP.PtrType != PTRT_WAITING)
        {
        put_wait_pointer( w );
        AGP.PtrType = PTRT_WAITING;
        }
}
void func_SetMainWaitPointer( void )
{
  struct Window *w = get_window();

    AGP.WaitPtrCount++;
    if (w && !(AGP.WorkFlags & WRF_MAC_PTRLOCK))
        {
        if (AGP.WaitPtrCount == 1) set_wait_pointer( w );
        }
}

void func_SetWaitPointer( struct Window *w ) // fonction pour devkit
{
    if (w) put_wait_pointer( w );
    else func_SetMainWaitPointer();
}

static void set_null_pointer( struct Window *w )
{
    if (AGP.PtrType != PTRT_NULL)
        {
        SetPointer( w, &NullPointer[0], 1, 16, 0, 0 );
        AGP.NullPointer = 1;
        AGP.PtrType = PTRT_NULL;
        }
}
void func_SetNullPointer( void )
{
  struct Window *w = get_window();

    if (w && TST_NULLPOINTER && !(AGP.WorkFlags & WRF_MAC_PTRLOCK) && !AGP.NullPointer && !AGP.WaitPtrCount)
        {
        set_null_pointer( w );
        }
}

static void put_system_pointer( struct Window *w )
{
    if (((struct Library*)GfxBase)->lib_Version >= 39)
        {
        struct TagItem tl[] = {
            TAG_END
            };
        SetWindowPointerA( w, tl );
        }
    else{
        ClearPointer( w );
        }
}

static void set_system_pointer( struct Window *w )
{
    if (AGP.PtrType != PTRT_SYSTEM)
        {
        put_system_pointer( w );
        AGP.PtrType = PTRT_SYSTEM;
        }
}

static void choose_pointer( struct Window *w )
{
    if (AGP.WaitPtrCount)
        {
        set_wait_pointer( w );
        }
    else{
        if (AGP.NullPointer)
            {
            set_null_pointer( w );
            }
        else{
            set_system_pointer( w );
            }
        }
}

void func_SetMainSystemPointer( void )
{
  struct Window *w = get_window();

    if (AGP.WaitPtrCount > 0) AGP.WaitPtrCount--;
    if (w && !(AGP.WorkFlags & WRF_MAC_PTRLOCK))
        {
        choose_pointer( w );
        }
}

void func_SetSystemPointer( struct Window *w ) // fonction pour devkit
{
    if (w) put_system_pointer( w );
    else func_SetMainSystemPointer();
}

void func_ResetPointer( void )
{
  struct Window *w = get_window();

    if (w)
        {
        if (AGP.PtrType == PTRT_MACRECORD) SetPointer( w, &RecordPointer[0], 10, 16, -1, 0 );
        else{
            if (AGP.PtrType == PTRT_MACPLAY) SetPointer( w, &PlayPointer[0], 10, 16, -1, 0 );
            else{
                if (AGP.WaitPtrCount) put_wait_pointer( w );
                else{
                    if (AGP.NullPointer) SetPointer( w, &NullPointer[0], 1, 16, 0, 0 );
                    else{
                        put_system_pointer( w );
                        }
                    }
                }
            }
        }
}

void func_SetNormalPointer( void )
{
  struct Window *w = get_window();

    if (w && !(AGP.WorkFlags & WRF_MAC_PTRLOCK))
        {
        choose_pointer( w );
        }
}

void func_RemoveNullPointer( void )
{
  struct Window *w = get_window();

    if (w && !(AGP.WorkFlags & WRF_MAC_PTRLOCK) && AGP.NullPointer)
        {
        AGP.NullPointer = 0;
        func_SetNormalPointer();
        }
}
