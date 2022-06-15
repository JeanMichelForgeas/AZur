#ifndef AZURDEFS_OBJ

#define AZURDEFS_OBJ
/*
 *      Copyright (C) 1993 Jean-Michel Forgeas
 *              Tous Droits Réservés
 */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define WL(a,b)        ((ULONG)((((UWORD)a)<<16)|((UWORD)b)))
#define BL(a,b,c,d)    ((ULONG)((((UBYTE)a)<<24)|(((UBYTE)b)<<16)|(((UBYTE)c)<<8)|((UBYTE)d)))
#define BW(a,b)        ((UWORD)((((UBYTE)a)<<8)|((UBYTE)b)))

/*------------------------------------------------*/

struct CmdFunc
    {
    void    (*Func)();
    APTR    Parm;
    };
typedef struct CmdFunc CF;

/*------------------------------------------------*/

struct CmdGerm
    {
    struct CmdGerm  *RegGerm;

    /*----- execution env ------*/
    UWORD           Reserved;
    UBYTE           ExeFrom;
    UBYTE           ResCode;
    ULONG           Flags;

    /*--------------------------*/
    struct CmdFunc  **Actions;
    ULONG           UsageCount;
    ULONG           Segment;
    UBYTE           **Names;
    UWORD           ObjSize;
    UWORD           Id;
    UWORD           Version;
    UWORD           Revision;
    APTR            CmdTemplate;
    ULONG           UserData;
    UBYTE           Items;
    };
typedef struct CmdGerm CG;

/*------ Flags */

#define AZAFF_DOMODIFY    0x0001  /* Y modifie le texte (interdit si readonly) */
#define AZAFF_OKINMACRO   0x0002  /* O est autorisée depuis une macro */
#define AZAFF_OKINAREXX   0x0004  /* A est autorisée depuis une commande ARexx */
#define AZAFF_OKINMENU    0x0008  /* M est autorisée depuis un menu */
#define AZAFF_OKINKEY     0x0010  /* K est autorisée depuis une touche */
#define AZAFF_OKINMOUSE   0x0020  /* S est autorisée depuis la souris */
#define AZAFF_OKINJOY     0x0040  /* J est autorisée depuis le soystick */
#define AZAFF_OKINMASTER  0x0080  /* T est autorisée depuis le Master Process */
#define AZAFF_OKINHOTKEY  0x0100  /* I est autorisée depuis l'input.device */
#define AZAFM_OKMASK      0x01ff  /* masque des bit précédents */

#define AZAFF_INTERNAL    0x40000000
#define AZAFF_EXECUTING   0x80000000 /* cette commande est en cours d'execution */

#define AZAFM_UserFlags   0x00ff0000 /* 8 bits sont laissés à l'usage du programmeur */

#define AZAFB_DOMODIFY    0
#define AZAFB_OKINMACRO   1
#define AZAFB_OKINAREXX   2
#define AZAFB_OKINMENU    3
#define AZAFB_OKINKEY     4
#define AZAFB_OKINMOUSE   5
#define AZAFB_OKINJOY     6
#define AZAFB_OKINMASTER  7
#define AZAFB_OKINHOTKEY  8

#define AZAFB_INTERNAL    30
#define AZAFB_EXECUTING   31


/*------------------------------------------------*/

struct CmdObj
    {
    struct CmdGerm      *RegGerm;

    /*----- execution env ------*/
    UWORD               Reserved;
    UBYTE               ExeFrom;
    UBYTE               ResCode;
    ULONG               Flags;

    /*--------------------------*/
    ULONG               UserData;
    APTR                *ArgRes;
    };
typedef struct CmdObj CO;

/*------ Flags */

#define AZCOM_OKMASK        0x000001ff /* ActionNew recopie AZAFM_OKMASK des flags du germe */

#define AZCOF_EXECUTING     0x80000000 /* cette commande est en cours d'execution */

#define AZCOB_EXECUTING     31

#define AZCOM_UserFlags     0x00ff0000 /* 8 bits sont laissés à l'usage du programmeur */

/*----------- ExeFrom -------------- */

#define FROMPGM     0x00  /* vient du programme (autre) */
#define FROMMACRO   0x01  /* vient d'une macro */
#define FROMAREXX   0x02  /* vient de ARexx */
#define FROMMENU    0x04  /* vient d'un menu */
#define FROMKEY     0x08  /* vient d'une touche */
#define FROMMOUSE   0x10  /* vient de la souris */
#define FROMJOY     0x20  /* vient du joystick */
#define FROMMASTER  0x40  /* vient du master process */
#define FROMHOTKEY  0x80  /* vient de l'input.device */


/********************************************************
 *
 *      Actions
 *
 ********************************************************/

/*----- Action types */

#define AZA_LOAD        0
#define AZA_UNLOAD      1
#define AZA_NEW         2
#define AZA_DISPOSE     3
#define AZA_INITMENU    4
#define AZA_SETMENU     5
#define AZA_DO          6
#define AZA_UNDO        7

#define AZA_NUMACTIONS  8


/********************************************************
 *
 *      Tags
 *
 ********************************************************/

/*----- Les Tags avec AZT_USER ne sont pas utilisés par AZur */

#define AZT_USER    (TAG_USER|(1L<<30))


/*----- Tags utilisés par AZur */

#define AZT_ACTION  (TAG_USER|(1L<<29))
#define AZT_GEN     (TAG_USER|(1L<<28))
#define AZT_OBJ     (TAG_USER|(1L<<27))


/*=================== Tags génériques (pour germe) =================*/

#define AZT_ActionLoad          (AZT_ACTION|0x0000)
#define AZT_ActionUnload        (AZT_ACTION|0x0001)
#define AZT_ActionNew           (AZT_ACTION|0x0002)
#define AZT_ActionDispose       (AZT_ACTION|0x0003)
#define AZT_ActionInitMenu      (AZT_ACTION|0x0004)
#define AZT_ActionSetMenu       (AZT_ACTION|0x0005)
#define AZT_ActionDo            (AZT_ACTION|0x0006)
#define AZT_ActionUndo          (AZT_ACTION|0x0007)

#define AZT_Gen_Name            (AZT_GEN|0x0000)
#define AZT_Gen_Size            (AZT_GEN|0x0001)
#define AZT_Gen_Id              (AZT_GEN|0x0002)
#define AZT_Gen_VersionRevision (AZT_GEN|0x0003)
#define AZT_Gen_UserData        (AZT_GEN|0x0004)
#define AZT_Gen_ArgTemplate     (AZT_GEN|0x0005)
#define AZT_Gen_Flags           (AZT_GEN|0x0006)


/*=================== Tags pour création d'objet ===================*/

/* les flags commencent par AZCO, voir ci-dessus */
/* utilisez seulement les flags utilisateur */
#define AZT_Flags               (AZT_OBJ|0x0000)
#define AZT_UserData            (AZT_OBJ|0x0001)
#define AZT_CmdLine             (AZT_OBJ|0x0002)
#define AZT_MinVerRev           (AZT_OBJ|0x0005)


#endif
