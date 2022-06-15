/****************************************************************
 *
 *      Project:    AZCharTable
 *      Function:   Fenêtre table des caractères
 *
 *      Created:    27/11/94    Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <exec/execbase.h>
#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <graphics/rastport.h>

#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/exec_pragmas.h>

/****** UIK *******/
#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikdrawauto.h"
#include "uiki:uikglobal.h"
#include "uiki:uikcolors.h"
#include "uiki:uikmacros.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"
#include "uiki:objstring.h"

/****** Azur ******/
#include "Azur:DevelKit/Includes/funcs.h"
#include "Azur:DevelKit/Includes/azur.h"
#include "Azur:DevelKit/Includes/obj.h"
#include "Azur:DevelKit/Includes/objfile.h"
#include "Azur:DevelKit/Includes/objview.h"
#include "Azur:DevelKit/Includes/objzone.h"

#define INTER_WIDTH  2
#define INTER_HEIGHT 1


/****** Imported ************************************************/


/****** Exported ************************************************/

struct DOSBase  * __far DOSBase;
struct ExecBase * __far SysBase;
struct UIKBase  * __far UIKBase;


/****** Statics *************************************************/

static void AZFunc_Do( struct CmdObj *co );


/****** Germe de l'objet de commande AZur ***********************/

/* attention à bien donner ici le même nom que son nom de fichier  */
static UBYTE * __far CmdNames[] = { "AZFileProt", 0 };

struct TagItem __far GermTagList[] =
    {
    AZT_Gen_Name,           (ULONG) CmdNames,
    AZT_Gen_ArgTemplate,    (ULONG) "TITLE/F",
    AZT_Gen_Flags,          AZAFF_OKINMACRO | AZAFF_OKINAREXX | AZAFF_OKINMENU | AZAFF_OKINKEY | AZAFF_OKINMOUSE | AZAFF_OKINJOY,
    AZT_ActionDo,           (ULONG) AZFunc_Do,
    TAG_END
    };

/****************************************************************
 *
 *      Germe d'objet UIK
 *      Utile pour catalogue de langue et variables locales
 *
 ****************************************************************/

static UBYTE * __far LangEnglish[] = {
    "",
    "_Deletable",
    "_Executable",
    "_Writable",
    "_Readable",
    "_Archived",
    "_Pure",
    "_Script",
    "_Comment",
    "No valid file",
    0
    };

static UBYTE * __far LangOther[] = {
    "",
    "_Effaçable",
    "E_xécutable",
    "_Modifiable",
    "_Lisible",
    "_Archivé",
    "_Pure",
    "_Script",
    "_Commentaire",
    "Pas de fichier valide",
    0
    };

static struct TagItem __far TotoGermTagList[] = {
    UIKTAG_GEN_LangEnglish, (ULONG)LangEnglish,
    UIKTAG_GEN_LangOther,   (ULONG)LangOther,
    UIKTAG_GEN_Catalog,     (ULONG)"AZur:Catalogs/AZFileProt.catalog",
    TAG_DONE
    };

struct ObjToto
    {
    struct UIKObj       Obj;
    struct UIKObj       *Checks[7];
    struct UIKObjStrGad *Comment;
    };

static void Generic_Refresh( struct UIKObjToto *toto )
{
    UIK_DrawBox( toto );
}

static struct UIKObjGerm __far TotoGerm = {
    "",     /* pas de nom */
    NULL,   /* int   (*AddVector)(); */
    NULL,   /* int   (*StartVector)(); */
    NULL,   /* void  (*StopVector)(); */
    NULL,   /* void  (*RemoveVector)(); */
    Generic_Refresh, /* void  (*RefreshVector)(); */
    NULL,   /* void  (*ResizeVector)(); */
    NULL,   /* void  (*ExceptActVector)(); */
    NULL,   /* void  (*ExceptInaVector)(); */
    NULL,   /* void  (*ExceptMovVector)(); */
    NULL,   /* void  (*ExceptKeyVector)(); */
    NULL,   /* int   (*StartAfterVector)(); */
    NULL,   /* int   (*SetGetValues)(); */
    NULL,   /* ULONG *ExtentedVectors; */
    TotoGermTagList, /* struct TagItem *TagList; */
    0,      /* UWORD MinWidth; */
    0,      /* UWORD MinHeight; */
    0,      /* ULONG DefaultWidthHeight; */
    OBJECTID_USER | 0,  /* UWORD ObjId; */
    sizeof(struct ObjToto), /* UWORD ObjSize; */
    BOX_DISPLAYABLE | BOX_TRANSLATELEFTTOP,      /* ULONG DefaultFlags */
    };

static struct UIKObjRegGerm __far TotoRegGerm = { &TotoGerm, 0, 0, 0 } ;


/****************************************************************
 *
 *      Code
 *
 ****************************************************************/

static ULONG get_protection( ULONG *pp )
{
  struct AZurProc *ap = AZ_GetAZur();
  ULONG lock, rc=0;

    if (lock = Lock( ap->FObj->OpenFileName, ACCESS_READ ))
        {
        if (Examine( lock, &ap->FIB ))
            {
            if (ap->FIB.fib_DirEntryType < 0)
                {
                *pp = ap->FIB.fib_Protection;
                rc = 1;
                }
            }
        UnLock( lock );
        }
    return( rc );
}

static void do_ok( struct UIKObj *ok, struct UIKObjWindow *wo )
{
  struct AZurProc *ap = AZ_GetAZur();
  struct ObjToto *toto = (struct ObjToto *)ok->UserULong1;
  UBYTE *ptr;
  ULONG i, set, protect, mask;

    if (get_protection( &protect ))
        {
        for (i=0; i < 7; i++)
            {
            mask = 1 << i;
            set = toto->Checks[i]->Flags & BOX_OVER;
            if (i < 4) { if (set) protect |= mask; else protect &= ~mask; }
            else { if (set) protect &= ~mask; else protect |= mask; }
            }
        SetProtection( ap->FObj->OpenFileName, protect );
        }

    if (ptr = UIK_OSt_Get( toto->Comment, 0 )) SetComment( ap->FObj->OpenFileName, ptr );

    UIK_Remove( wo );
}

static void do_cancel( struct UIKObj *cancel, struct UIKObjWindow *wo )
{
    UIK_Remove( wo );
}

static void AZFunc_Do( struct CmdObj *co )
{
  struct AZurProc *ap = AZ_GetAZur();
  struct UIKObjWindow *wo=0;
  struct ObjToto *toto;
  struct UIKObj *ok, *cancel;
  struct TextFont *font;
  WORD width, height, i, gadtop, intergad;
  ULONG protect;
  UBYTE *ptr;

    UIKBase = ap->UIKBase;
    SysBase = ap->ExecBase;
    DOSBase = ap->DOSBase;

    if (!ap->WO) goto END_ERROR;
    if (!ap->WO->Obj.w) goto END_ERROR;

    font = ap->WO->Obj.ObjExt->Font;

    gadtop = 2 + font->tf_YSize / 2; intergad = 12 + 1;
    if (font->tf_YSize > 12)
        { gadtop += ((font->tf_YSize - 12) / 2); intergad = font->tf_YSize + 1; }

    width = 400;
    height = 11 * intergad;

    if (! (wo = (struct UIKObjWindow *) UIK_AddObjectTags( "UIKObj_Window", ap->WO,
            UIKTAG_OBJ_WidthHeight,             W2L(width,height),
            UIKTAG_OBJ_Title,                   co->ArgRes[0] ? co->ArgRes[0] : "Attrs",
            UIKTAG_OBJ_TitleFl_Addr,            TRUE,
            UIKTAG_OBJWindowFl_CloseStop,       FALSE,
            UIKTAG_OBJWindowFl_CloseRemove,     TRUE,
            UIKTAG_OBJWindowFl_AutoCenter,      TRUE,
            UIKTAG_OBJWindowFl_With_Size,       FALSE,
            UIKTAG_OBJWindowFl_With_Icon,       FALSE,
            TAG_END ))) goto END_ERROR;
    if (! (toto = (APTR) UIK_AddPrivateObjectTags( &TotoRegGerm, wo,
            UIKTAG_OBJ_WidthHeight,             W2L(width,height),
            UIKTAG_OBJ_BoxFl_Over,              TRUE,
            UIKTAG_OBJ_UserValue1,              ap,
            TAG_END ))) goto END_ERROR;

    if (! get_protection( &protect ))
        {
        ptr = UIK_LangString( toto, 9 );
        AZ_SmartRequest( 0, ptr, 0,0,0,0, 0,0,0, 0, (TST_REQWIN ? ARF_WINDOW : 0) );
        AZ_SetCmdResult( co, 20, 1000, ptr );
        goto END_ERROR;
        }

    for (i=0; i < 7; i++, gadtop += intergad)
        {
        if (! (toto->Checks[i] = (APTR) UIK_AddObjectTags( "UIKObj_CheckToggle", toto,
                UIKTAG_OBJ_LeftTop,             W2L(20,gadtop),
                UIKTAG_OBJ_WidthHeight,         W2L(26,12),
                UIKTAG_OBJ_ActInactPens,        UIKCOLS(UIKCOL_WHITE,UIKCOL_GREY,UIKCOL_BLACK,UIKCOL_BLACK),
                UIKTAG_OBJ_LangMaster,          toto,
                UIKTAG_OBJ_Title,               i+1,
                UIKTAG_OBJ_TitleFl_Right,       TRUE,
                UIKTAG_OBJ_TitleUnderscore,     '_',
                UIKTAG_OBJ_BoxFl_Over,          (i < 4) ? (protect & (1<<i) ? TRUE : FALSE) : (protect & (1<<i) ? FALSE : TRUE),
                UIKTAG_OBJ_BoxFl_Filled,        TRUE,
                UIKTAG_OBJ_BoxFl_NoPush,        TRUE,
                TAG_END ))) goto END_ERROR;
        }

    gadtop += intergad - 3;
    if (! (toto->Comment = (APTR) UIK_AddObjectTags( "UIKObj_String", toto,
            UIKTAG_OBJ_LeftTop,             W2L(10,gadtop),
            UIKTAG_OBJ_WidthHeight,         W2L(width-20,0),
            UIKTAG_OBJ_LangMaster,          toto,
            UIKTAG_OBJ_Title,               8,
            UIKTAG_OBJ_TitleFl_Above,       TRUE,
            UIKTAG_OBJ_TitleUnderscore,     '_',
            UIKTAG_OBJStr_TextPointer,      ap->FIB.fib_Comment,
            UIKTAG_OBJStr_MaxChars,         79,
            TAG_END ))) goto END_ERROR;

    gadtop += intergad + 5;
    if (! (ok = (APTR) UIK_AddObjectTags( "UIKObj_OK", toto,
            UIKTAG_OBJ_LeftTop,             W2L(10,gadtop),
            UIKTAG_OBJ_WidthHeight,         W2L(45,intergad),
            UIKTAG_OBJ_UserValue1,          toto,
            TAG_END ))) goto END_ERROR;
    if (! (cancel = (APTR) UIK_AddObjectTags( "UIKObj_Cancel", toto,
            UIKTAG_OBJ_LeftTop,             W2L(width-10-45,gadtop),
            UIKTAG_OBJ_WidthHeight,         W2L(45,intergad),
            UIKTAG_OBJ_UserValue1,          toto,
            TAG_END ))) goto END_ERROR;

    if (! UIK_AddVector( ok, do_ok, IDCMP_GADGETUP, (ULONG)wo )) goto END_ERROR;
    if (! UIK_AddVector( cancel, do_cancel, IDCMP_GADGETUP, (ULONG)wo )) goto END_ERROR;

    if (! UIK_Start( wo )) goto END_ERROR;

    return;

  END_ERROR:
    UIK_Remove( wo );
}
