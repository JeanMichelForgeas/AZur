/****************************************************************
 *
 *      File:      filedef_key.c
 *      Project:   AZur
 *
 *      Created:   16-06-93     Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <devices/inputevent.h>

#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"
#include "lci:_pool_protos.h"

#include "lci:objfile.h"

#define FOUND_SHIFT 1
#define FOUND_ALT   2
#define FOUND_CTRL  4

struct KeyNames
    {
    UBYTE *Name;
    ULONG Code;
    ULONG Qual;
    };

#define SPKEY_BEFORE_KEY    0x80000001
#define SPKEY_AFTER_KEY     0x80000002
#define SPKEY_BEFORE_LINE   0x80000003
#define SPKEY_AFTER_LINE    0x80000004


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/

static ULONG __far demo_count;

/* CodeKeyTab[128][8] contient des codes ASCII
 * normal shift alt ctrl shift-alt shift-ctrl alt-ctrl shift-alt-ctrl
 *   0      1    2   3        4         5        6            7
 * l'index peut être atteind avec FOUND_SHIFT FOUND_ALT FOUND_CTRL
 */

/* ASCIIKeyTab[256][2] contient des codes RAWKEY
 * code qualifier
 *  0      1
 */

static UBYTE __far QualTab[] = {
    0,
    IEQUALIFIER_LSHIFT,
    IEQUALIFIER_LALT,
    IEQUALIFIER_CONTROL,
    IEQUALIFIER_LSHIFT+IEQUALIFIER_LALT,
    IEQUALIFIER_CONTROL+IEQUALIFIER_LALT,
    IEQUALIFIER_CONTROL+IEQUALIFIER_LSHIFT,
    IEQUALIFIER_CONTROL+IEQUALIFIER_LSHIFT+IEQUALIFIER_LALT,
    };

static UBYTE __far TabCodes[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,   -1,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,   -1,   -1,   -1,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,   -1,   -1,   -1,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,   -1,   -1,   -1,   -1,
    0x40, 0x41, 0x42,   -1, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,   -1, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,   -1,   -1,   -1,   -1,   -1, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    };

static struct KeyNames __far TabA[] = {
    { "lt",            -1, IEQUALIFIER_LALT },
    { "miga",          -1, IEQUALIFIER_LCOMMAND },
    { "nyMode",        -1, SQUAL_ANYSPECIAL },
    { "nySpecial",     -1, SQUAL_ANYSPECIAL },
    { "fterKey",      -1, SPKEY_AFTER_KEY },
    { "fterLine",     -1, SPKEY_AFTER_LINE },
    { 0 }, };
static struct KeyNames __far TabB[] = {
    { "ackSpace",      0x41, 0 },
    { "reak",          0x00, 0 },
    { "eforeKey",     -1, SPKEY_BEFORE_KEY },
    { "eforeLine",    -1, SPKEY_BEFORE_LINE },
    { 0 }, };
static struct KeyNames __far TabC[] = {
    { "trl",           -1, IEQUALIFIER_CONTROL },
    { "tl",            -1, IEQUALIFIER_CONTROL },
    { "ontrol",        -1, IEQUALIFIER_CONTROL },
    { "ursUp",         0x4c, 0 },
    { "ursor_Up",      0x4c, 0 },
    { "ursDown",       0x4d, 0 },
    { "ursor_Down",    0x4d, 0 },
    { "ursLeft",       0x4f, 0 },
    { "ursor_Left",    0x4f, 0 },
    { "ursRight",      0x4e, 0 },
    { "ursor_Right",   0x4e, 0 },
    { 0 }, };
static struct KeyNames __far TabD[] = {
    { "el",            0x46, 0 },
    { "elete",         0x46, 0 },
    { "own",           0x4d, 0 },
    { 0 }, };
static struct KeyNames __far TabE[] = {
    { "nter",          0x43, 0 },
    { "sc",            0x45, 0 },
    { "scape",         0x45, 0 },
    { "nd",            0x1d, 0 },
    { 0 }, };
static struct KeyNames __far TabF[] = {
    { "1",             0x50, 0 },
    { "01",            0x50, 0 },
    { "2",             0x51, 0 },
    { "02",            0x51, 0 },
    { "3",             0x52, 0 },
    { "03",            0x52, 0 },
    { "4",             0x53, 0 },
    { "04",            0x53, 0 },
    { "5",             0x54, 0 },
    { "05",            0x54, 0 },
    { "6",             0x55, 0 },
    { "06",            0x55, 0 },
    { "7",             0x56, 0 },
    { "07",            0x56, 0 },
    { "8",             0x57, 0 },
    { "08",            0x57, 0 },
    { "9",             0x58, 0 },
    { "09",            0x58, 0 },
    { "10",            0x59, 0 },
    { "11",            0x00, 0 },
    { "12",            0x00, 0 },
    { 0 }, };
static struct KeyNames __far TabH[] = {
    { "elp",           0x5f, 0 },
    { "ome",           0x3d, 0 },
    { 0 }, };
static struct KeyNames __far TabI[] = {
    { "nsert",         0x0f, 0 },
    { 0 }, };
static struct KeyNames __far TabL[] = {
    { "ock1",          -1, SQUAL_LOCK1 },
    { "ock2",          -1, SQUAL_LOCK2 },
    { "ock3",          -1, SQUAL_LOCK3 },
    { "eft",           0x4f, 0 },
    { "eft_Amiga",     -1, IEQUALIFIER_LCOMMAND },
    { "eftAmiga",      -1, IEQUALIFIER_LCOMMAND },
    { 0 }, };
static struct KeyNames __far TabM[] = {
    { "eta",           -1, SQUAL_QUAL1 },
    { "eta2",          -1, SQUAL_QUAL2 },
    { "ode",           -1, SQUAL_LOCK1 },
    { "ode2",          -1, SQUAL_LOCK2 },
    { 0 }, };
static struct KeyNames __far TabN[] = {
    { "oSpecial",      -1, SQUAL_NOSPECIAL },
    { "oRepeat",       -1, SQUAL_NOREPEAT },
    { "um_Pad",        -1, IEQUALIFIER_NUMERICPAD },
    { "umPad",         -1, IEQUALIFIER_NUMERICPAD },
    { "P_NumL",        0x5a, 0 },
    { "P_ScrL",        0x5b, 0 },
    { "P_PrtSc",       0x5d, 0 },
    { "P_Home",        0x3d, 0 },
    { "P_Up",          0x3e, 0 },
    { "P_PgUp",        0x3f, 0 },
    { "P_Left",        0x2d, 0 },
    { "P_Right",       0x2f, 0 },
    { "P_End",         0x1d, 0 },
    { "P_Down",        0x1e, 0 },
    { "P_PgDn",        0x1f, 0 },
    { "P_Ins",         0x0f, 0 },
    { "P_Del",         0x3c, 0 },
    { "P_7",           0x3d, 0 },
    { "P_8",           0x3e, 0 },
    { "P_9",           0x3f, 0 },
    { "P_4",           0x2d, 0 },
    { "P_5",           0x2e, 0 },
    { "P_6",           0x2f, 0 },
    { "P_1",           0x1d, 0 },
    { "P_2",           0x1e, 0 },
    { "P_3",           0x1f, 0 },
    { "P_0",           0x0f, 0 },
    { "P_Point",       0x3c, 0 },
    { "P_/",           0x5c, 0 },
    { "P_*",           0x5d, 0 },
    { "P_-",           0x4a, 0 },
    { "P_+",           0x5e, 0 },
    { "P7",            0x3d, 0 },
    { "P8",            0x3e, 0 },
    { "P9",            0x3f, 0 },
    { "P4",            0x2d, 0 },
    { "P5",            0x2e, 0 },
    { "P6",            0x2f, 0 },
    { "P1",            0x1d, 0 },
    { "P2",            0x1e, 0 },
    { "P3",            0x1f, 0 },
    { "P0",            0x0f, 0 },
    { "P/",            0x5c, 0 },
    { "P*",            0x5d, 0 },
    { "P-",            0x4a, 0 },
    { "P+",            0x5e, 0 },
    { "um7",           0x3d, 0 },
    { "um8",           0x3e, 0 },
    { "um9",           0x3f, 0 },
    { "um4",           0x2d, 0 },
    { "um5",           0x2e, 0 },
    { "um6",           0x2f, 0 },
    { "um1",           0x1d, 0 },
    { "um2",           0x1e, 0 },
    { "um3",           0x1f, 0 },
    { "um0",           0x0f, 0 },
    { "um/",           0x5c, 0 },
    { "um*",           0x5d, 0 },
    { "um-",           0x4a, 0 },
    { "um+",           0x5e, 0 },
    { 0 }, };
static struct KeyNames __far TabO[] = {
    { "therwise",      -1, SQUAL_NOSPECIAL },
    { 0 }, };
static struct KeyNames __far TabP[] = {
    { "ageUp",         0x3f, 0 },
    { "age_Up",        0x3f, 0 },
    { "ageDown",       0x1f, 0 },
    { "age_Down",      0x1f, 0 },
    { "ause",          0x00, 0 },
    { 0 }, };
static struct KeyNames __far TabQ[] = {
    { "ual1",          -1, SQUAL_QUAL1 },
    { "ual2",          -1, SQUAL_QUAL2 },
    { "ual3",          -1, SQUAL_QUAL3 },
    { "ualButLeft",    -1, IEQUALIFIER_LEFTBUTTON },
    { "ualButMiddle",  -1, IEQUALIFIER_MIDBUTTON },
    { "ualButRight",   -1, IEQUALIFIER_RBUTTON },
    { "ualJoyDown",    -1, SQUAL_JOYDOWN },
    { "ualJoyFire",    -1, SQUAL_JOYFIRE },
    { "ualJoyLeft",    -1, SQUAL_JOYLEFT },
    { "ualJoyRight",   -1, SQUAL_JOYRIGHT },
    { "ualJoyUp",      -1, SQUAL_JOYUP },
    { 0 }, };
static struct KeyNames __far TabR[] = {
    { "eturn",         0x44, 0 },
    { "ight",          0x4e, 0 },
    { 0 }, };
static struct KeyNames __far TabS[] = {
    { "hift",          -1, IEQUALIFIER_LSHIFT },
    { "ft",            -1, IEQUALIFIER_LSHIFT },
    { "pace",          0x40, 0 },
    { "paceBar",       0x40, 0 },
    { 0 }, };
static struct KeyNames __far TabT[] = {
    { "ab",            0x42, 0 },
    { 0 }, };
static struct KeyNames __far TabU[] = {
    { "p",             0x4c, 0 },
    { 0 }, };

static struct KeyNames __far TabNull[] = {
    { 0 }, };

static struct KeyNames * __far TabNamesPtr[] = {
    TabA,
    TabB,
    TabC,
    TabD,
    TabE,
    TabF,
    TabNull,
    TabH,
    TabI,
    TabNull,
    TabNull,
    TabL,
    TabM,
    TabN,
    TabO,
    TabP,
    TabQ,
    TabR,
    TabS,
    TabT,
    TabU,
    TabNull,
    TabNull,
    TabNull,
    TabNull,
    TabNull,
    };

static UBYTE __far defaults[] =
"RETURN Text2Win CONVERT COL=-1 PREFS \\n\0"
"ENTER Text2Win CONVERT COL=-1 INDENTPREV \\n\0"
"Del+SHIFT Del LINES\0"
"Del Del\0"
"BackSpace Del PREV\0"
"CursUp Cursor REL -1\0"
"CursDown Cursor REL +1\0"
"CursLeft Cursor REL COL -1\0"
"CursRight Cursor REL COL +1\0"
"SHIFT+CursUp Cursor LINE -1 PAGE RELATIVE\0"
"SHIFT+CursDown Cursor LINE +1 PAGE RELATIVE\0"
"SHIFT+CursLeft Cursor COL 1\0"
"SHIFT+CursRight Cursor COL -1\0"
"ALT+CursUp Cursor 1 1\0"
"ALT+CursDown Cursor -1 -1\0"
"#\0";

//static UBYTE __far CodeKeyTab[128][8];     /* table contenant des codes ASCII */
static UBYTE __far ASCIIKeyTab[256][2];    /* table contenant des codes/qualifiers, seulement shift alt ctrl */


/****************************************************************
 *
 *      Keys handling
 *
 ****************************************************************/

static void InitKeyTabs()
{
  struct InputEvent ev;
  register ULONG ind, code, qual, xcode, ascii;
  UBYTE buffer[30];

    ev.ie_NextEvent = NULL;
    ev.ie_Class = IECLASS_RAWKEY;
    ev.ie_SubClass = 0;

    for (ascii=0; ascii < 256; ascii++)
        { ASCIIKeyTab[ascii][0] = -1; /*ASCIIKeyTab[ascii][1] = 0;*/ }

    for (qual=0; qual < 7; qual++)
        {
        ev.ie_Qualifier = QualTab[qual];
        for (code=0; code < 0x7F; code++)
            {
            if ((xcode = TabCodes[code]) != 0xff)     // évite le pavé numérique
                {
                ev.ie_Code = xcode;
                if (AZRawKeyConvert( &ev, buffer, 30 ) == 1)
                    {
                    ascii = buffer[0];
                    ind = qual;
                    //CodeKeyTab[xcode][ind] = ascii;
                    if (ASCIIKeyTab[ascii][0] == -1)
                        {
                        ASCIIKeyTab[ascii][0] = ev.ie_Code;
                        ASCIIKeyTab[ascii][1] = ev.ie_Qualifier;
                        }
                    }
                }
            }
        }
}

ULONG Parm_InitKey()
{
    InitKeyTabs();
    return(1);
}

//-----------------------------------------

ULONG Parm_CleanupKey( ULONG quit )
{
  struct KeyHandle *kh = (struct KeyHandle *) AGP.AZMast->KeyHandle;
  APTR pool=0;

    if (kh)
        {
        eng_DisposeObject( kh->NoSpecial );
        eng_DisposeObject( kh->BeforeKey );
        eng_DisposeObject( kh->AfterKey );
        eng_DisposeObject( kh->BeforeLine );
        eng_DisposeObject( kh->AfterLine );
        AsmDeletePool( kh->Pool, ExecBase );
        AGP.AZMast->KeyHandle = 0;
        }
    if (! quit)
        {
        if (! (pool = AsmCreatePool( MEMF_ANY|MEMF_CLEAR, 2000, 2000, ExecBase ))) goto END_ERROR;
        if (! (kh = (struct KeyHandle *) BufAllocP( pool, sizeof(struct KeyHandle), sizeof(struct KeyHandle) ))) goto END_ERROR;
        if (! UIK_nl_AllocListP( pool, &kh->KeyList, 20, 0, 0, 0 )) goto END_ERROR;
        if (! (kh->CurKeys = (struct AZKey *) BufAllocP( pool, 0, 5*sizeof(struct AZKey) ))) goto END_ERROR;
        kh->Pool = pool;
        AGP.AZMast->KeyHandle = kh;
        }
    return(1);

  END_ERROR:
    if (pool) AsmDeletePool( pool, ExecBase );
    return(0);
}

//-----------------------------------------

static ULONG demo_fold_callback( ULONG *ptotlen, LONG realline, struct UIKPList *ul, LONG ind )
{
    *ptotlen += (1 + BufLength( ul->UNode[ind].Buf ));
    return(0);
}

void RawKeys( struct AZObjFile *fo, struct KeyHandle *kh )
{
  register struct IntuiMessage *imsg = AGP.UIK->IntuiMsg;
  struct MouseHandle *mh = (struct MouseHandle *) AGP.AZMast->MouseHandle;
  UBYTE any=0, rescode;

    set_intui_vars();
    if (imsg->Code & 0x80) return;
    if (imsg->Code >= 0x60 && imsg->Code <= 0x67) return;

    kh->RecKey.Code = imsg->Code;

    //------------- kh->RecKey.Qualifier
    kh->RecKey.Qualifier = kh->IndexTabQual = 0;
    if (imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT))
        {
        kh->RecKey.Qualifier |= IEQUALIFIER_LSHIFT;
        kh->IndexTabQual |= FOUND_SHIFT;
        }
    if (imsg->Qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT))
        {
        kh->RecKey.Qualifier |= IEQUALIFIER_LALT;
        kh->IndexTabQual |= FOUND_ALT;
        }
    if (imsg->Qualifier & IEQUALIFIER_CONTROL)
        {
        kh->RecKey.Qualifier |= IEQUALIFIER_CONTROL;
        kh->IndexTabQual |= FOUND_CTRL;
        }

    if (imsg->Qualifier & (IEQUALIFIER_LCOMMAND|IEQUALIFIER_RCOMMAND))
        {
        kh->RecKey.Qualifier |= IEQUALIFIER_LCOMMAND;
        }
    kh->RecKey.Qualifier |= (imsg->Qualifier & (IEQUALIFIER_REPEAT | SQUAL_MJOY));
    kh->RecKey.Qualifier |= kh->AZQual;
    kh->RecKey.Qualifier |= mh->JoyQuals;

    if (kh->RecKey.Qualifier & IEQUALIFIER_REPEAT)
        {
        register struct Window *w = imsg->IDCMPWindow;
        register struct IntuiMessage *succ, *msg;

        Forbid();
        msg = (struct IntuiMessage *)imsg->ExecMessage.mn_Node.ln_Succ;
        while (succ = (struct IntuiMessage *)msg->ExecMessage.mn_Node.ln_Succ)
            {
            if (msg->IDCMPWindow == w)
                {
                if (msg->Code == imsg->Code && msg->Class == imsg->Class && (msg->Qualifier & IEQUALIFIER_REPEAT))
                    { Remove( msg ); ReplyMsg( msg ); }
                //else break;
                }
            msg = succ;
            }
        Permit();
        }
    /*
    else{ // dans tous les cas
        func_SetNullPointer();
        }
    */

    //------------ mémorisation
    if (! BufPasteS( (APTR)&kh->RecKey, sizeof(struct AZKey), (APTR)&kh->CurKeys, intAZ_LineBufLen( (APTR)kh->CurKeys ) )) return;

    //------------ recherche
    {   register struct UIKPList *list;
        register struct UIKNode *node;
        register struct AZShortcut *sc;
        register struct AZKey *akl, *akc;
        register LONG ind, i;

    list = &kh->KeyList;
    RETRY_ANYSPECIAL:
    node = list->UNode;
    kh->KeyFound = 0;
    for (ind=0; ind < list->NodeNum; ind++, node++)
        {
        sc = (struct AZShortcut *) node->Buf;  // ligne contenant la [suite de] Shortcut

        if (kh->IndexCurKey < sc->NumKeys)
            {
            if (sc->Keys[kh->IndexCurKey].Code == kh->RecKey.Code)  // accélère : évite de rechercher pour toutes les touches
                {
                for (i=0; i <= kh->IndexCurKey; i++)
                    {
                    akl = &sc->Keys[i];      // paire correspondant à l'ordre dans la suite
                    akc = &kh->CurKeys[i];   // paire correspondant à l'ordre dans la mémorisation
                    if ((akl->Code != akc->Code) || ((akl->Qualifier & (SQUAL_MASK|SQUAL_MJOY)) != (akc->Qualifier & (SQUAL_MASK|SQUAL_MJOY))))
                        break;
                    if ((akc->Qualifier & IEQUALIFIER_REPEAT) && (akl->Qualifier & SQUAL_NOREPEAT))
                        break;
                    }
                if (i > kh->IndexCurKey)
                    {
                    kh->KeyFound = sc;
                    break;
                    }
                }
            }
        }

    if (kh->KeyFound) //------ Commande
        {
        kh->IndexCurKey++;
        if (sc->NumKeys == kh->IndexCurKey)
            {
            kh->IndexCurKey = 0;
            BufTruncate( (APTR)kh->CurKeys, 0 );
            kh->AZQual &= ~(SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3); // avant car l'action peut les mettre ON

            if (AGP.WO && AGP.WO->Obj.Status == UIKS_ICONIFIED)
                {
                if (StrNCCmp( kh->KeyFound->CmdName, "File2NewWin" ) != 0)
                    {
                    UIK_UniconifyWindow( AGP.WO );
                    kh->KeyFound = 0;
                    }
                }

            if (kh->KeyFound && TST_INPUTLOCK)
                {
                if (! kh->KeyFound->CmdName)
                    kh->KeyFound = 0;
                else if (StrNCCmp( kh->KeyFound->CmdName, "Lock" ) != 0) // (( == 0 && kh->KeyFound->ArgRes[1] && (kh->KeyFound->ArgRes[7] || kh->KeyFound->ArgRes[8])))
                    kh->KeyFound = 0;
                }

            if (kh->KeyFound)
                {
                struct CmdObj *co = kh->KeyFound->CO;
                ULONG dispose=0;

                /*
                struct KeyCom_Key kc;
                kc.Code = kh->RecKey.Code;
                kc.Qualifier = kh->RecKey.Qualifier;
                kc.InputNum = AGP.UIK->KeySupp.Length;
                kc.InputBuf = AGP.UIK->KeySupp.KeyBuffer;
                */

                rescode = 0;
                //if (kh->BeforeKey) { func_ExecuteCmdObj( kh->BeforeKey, (ULONG)&kc ); rescode = kh->BeforeKey->ResCode; }
                if (rescode == 0)
                    {
                    if (! co)
                        {
                        if (co = eng_NewObjectTags( kh->KeyFound->CmdName, AZT_CmdLine, kh->KeyFound->CmdLine, AZT_ExeFrom, FROMKEY, TAG_END ))
                            dispose = 1;
                        }
                    if (co)
                        {
                        func_ExecuteCmdObj( co, 0 );
                        rescode = co->ResCode;
                        StoreLastCmd( co );
                        }
                    if (dispose) eng_DisposeObject( co );
                    }
                //if (rescode == 0 && kh->AfterKey) func_ExecuteCmdObj( kh->AfterKey, (ULONG)&kc );
                }
            }
        }
    //------ Touche simple
    else if (AGP.WO && AGP.WO->Obj.Status == UIKS_ICONIFIED)
        {
        UIK_UniconifyWindow( AGP.WO );
        }
    else{
        if ((any == 0) && (kh->RecKey.Qualifier & (SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3|SQUAL_LOCK1|SQUAL_LOCK2|SQUAL_LOCK3)))
            {
            any = 1;
            kh->RecKey.Qualifier &= ~(SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3|SQUAL_LOCK1|SQUAL_LOCK2|SQUAL_LOCK3);
            kh->RecKey.Qualifier |= SQUAL_ANYSPECIAL;
            for (i=0; i <= kh->IndexCurKey; i++)
                {
                kh->CurKeys[i].Qualifier &= ~(SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3|SQUAL_LOCK1|SQUAL_LOCK2|SQUAL_LOCK3);
                kh->CurKeys[i].Qualifier |= SQUAL_ANYSPECIAL;
                }
            goto RETRY_ANYSPECIAL;
            }
        else if (any)
            {
            if (kh->NoSpecial)
                {
                if (!TST_INPUTLOCK) func_ExecuteCmdObj( kh->NoSpecial, 0 );
                }
            else if (kh->RecKey.Qualifier & SQUAL_ANYSPECIAL)
                {
                kh->RecKey.Qualifier &= ~SQUAL_ANYSPECIAL; /* 3ème passage : touche normale */
                for (i=0; i <= kh->IndexCurKey; i++)
                    kh->CurKeys[i].Qualifier &= ~SQUAL_ANYSPECIAL;
                any = 0;
                goto RETRY_ANYSPECIAL;
                }
            }
        else if (AGP.UIK->KeySupp.Length)
            {
            if (!TST_READONLY && !TST_INPUTLOCK)
                {
                if (AGP.UIK->KeySupp.KeyBuffer[0] != 0x9b)
                    {
                    struct KeyCom_Key kc;
                    kc.Code = kh->RecKey.Code;
                    kc.Qualifier = kh->RecKey.Qualifier;
                    kc.InputNum = AGP.UIK->KeySupp.Length;
                    kc.InputBuf = AGP.UIK->KeySupp.KeyBuffer;

                    if (TST_CHARDELBLOCK)
                        {
                        func_Block2CB( AGP.Prefs->DelClip );
                        func_BlockEraseAll();
                        }
                    else{
                        func_BlockClearAll();
                        }

                    if (! (AGP.WorkFlags & WRF_MAC_RECORDING))
                        func_SetNullPointer();

                    rescode = 0;
                    if (kh->BeforeKey && !(AGP.WorkFlags & WRF_MAC_QUIET))
                        { func_ExecuteCmdObj( kh->BeforeKey, (ULONG)&kc ); rescode = kh->BeforeKey->ResCode; }
                    if (rescode == 0)
                        {
                        ULONG i, len = AGP.UIK->KeySupp.Length, flags = ARPF_NOCONVERT | ARPF_RECT;
                        UBYTE *ptr = AGP.UIK->KeySupp.KeyBuffer;

                        for (i=0; i < len; i++) if (ptr[i] == '\n') { flags &= ~ARPF_RECT; break; }
                        if (TST_OVERLAY) flags |= ARPF_OVERLAY;

                        if (AGP.WorkFlags & WRF_MAC_RECORDING)
                            {
                            ULONG len = StrLen(ptr);
                            UBYTE *p, *mem = BufAllocP( AGP.Pool, len+2, 4 );
                            p = ptr;
                            if (mem)
                                {
                                /* *mem = '"'; StrCpy( mem+1, ptr ); *(mem+len+1) = '"'; */
                                StrCpy( mem, ptr );
                                BufPasteS( "TEXT ", 5, &mem, 0 );
                                if (flags & ARPF_OVERLAY) BufPasteS( "OVERLAY ", 8, &mem, 0 );
                                if (flags & ARPF_RECT)    BufPasteS( "RECT ", 5, &mem, 0 );
                                p = mem;
                                }
                            func_MacroAdd( "Text2Win", p );
                            if (mem) BufFree( mem );
                            }

                        if (!(AGP.WorkFlags & WRF_MAC_QUIET))
                            {
                            rescode = commun_text_2( 0, ptr, len, 0, flags, 0 );
#ifdef AZUR_DEMO
    if (++demo_count > 50)
        {
        ULONG totlen=0;
        BufTruncate( AGP.UBufTmp, 0 );
        func_FoldApplyLines( (void*)demo_fold_callback, &totlen, &AGP.UBufTmp );
        if (totlen > (AZURDEMO_MAXSIZE+15)) SET_READONLY;
        demo_count = 0;
        }
#endif
                            }
                        }
                    if (rescode == 0 && kh->AfterKey && !(AGP.WorkFlags & WRF_MAC_QUIET))
                        func_ExecuteCmdObj( kh->AfterKey, (ULONG)&kc );
                    }
                }
            }
        kh->IndexCurKey = 0;
        BufTruncate( (APTR)kh->CurKeys, 0 );
        kh->AZQual &= ~(IEQUALIFIER_LSHIFT|IEQUALIFIER_LALT|IEQUALIFIER_CONTROL|IEQUALIFIER_LCOMMAND|SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3);
        }
    }
}

//-------------------------------------------------------------------------

struct AZShortcut *Keys_AddShortCut( UBYTE *p, ULONG menu, ULONG qual )
{
  struct KeyHandle *kh = AGP.AZMast->KeyHandle;
  struct AZShortcut *sc=0;
  struct CmdObj **pco;
  UBYTE *cmdname, *parm, *mem=0;
  LONG lline=-1;

  UBYTE *end;
  ULONG index, code;
  struct AZKey azk;
  struct KeyNames *kn;

    if (kh == 0)
        {
        if (! Parm_CleanupKey(0)) return(0);
        kh = AGP.AZMast->KeyHandle;
        }

    lline = -1;
    if (! (sc = (struct AZShortcut *) UIK_nl_AllocNode( &kh->KeyList, -1, sizeof(struct AZShortcut)-sizeof(struct AZKey), 0 ))) goto END_ERROR;
    lline = kh->KeyList.NodeNum - 1;

    code = -1;
    while (TRUE)
        {
        //-------------------- cherche code+qualifier
        if (*p == 0) goto END_ERROR;
        end = FindTabName( TabNamesPtr, p, sizeof(struct KeyNames), (APTR)&kn );

        //-------------------- cherche touche seule
        if (end == 0)
            {
            UBYTE ch = UIK_ToLower( *p );

            if (ch == '@')
                {
                if ((index = ARG_StrToLong( ++p, &code )) == 0) goto END_ERROR;
                p += index;
                }
            else{
                if (qual & IEQUALIFIER_NUMERICPAD)
                    {
                    switch (ch)
                        {
                        case '0': code = 0x0f; break;
                        case '1': code = 0x1d; break;
                        case '2': code = 0x1e; break;
                        case '3': code = 0x1f; break;
                        case '4': code = 0x2d; break;
                        case '5': code = 0x2e; break;
                        case '6': code = 0x2f; break;
                        case '7': code = 0x3d; break;
                        case '8': case '#': code = 0x3e; break;
                        case '9': case '^': code = 0x3f; break;
                        case '.': code = 0x3c; break;
                        case '[': case '{': case '(': code = 0x5a; break;
                        case ')': case '}': case ']': code = 0x5b; break;
                        case '/': code = 0x5c; break;
                        case '*': code = 0x5d; break;
                        case '-': code = 0x4a; break;
                        case '+': code = 0x5e; break;
                        default: code = ASCIIKeyTab[ch][0]; qual |= ASCIIKeyTab[ch][1]; break;
                        }
                    }
                else { code = ASCIIKeyTab[ch][0]; qual |= ASCIIKeyTab[ch][1]; }
                p++;
                }
            }
        //-------------------- prend touche tableau
        else{
            if (kn->Code != -1) code = kn->Code;
            else qual |= kn->Qual;
            p = end;
            }

        //-------------------- fin de séquence?
        if (qual == SQUAL_NOSPECIAL || qual > 0x80000000) break;

        if (*p == ' ' || *p == '\t' || *p == 0x22 || *p == 0x27) // stockage de la touche
            {
            p++;
            if (code == -1) goto END_ERROR;
            azk.Code = code; azk.Qualifier = qual;
            if (! (sc = (struct AZShortcut *) UIK_nl_PasteToNode( &kh->KeyList, lline, UIK_nl_NodeBufLen( &kh->KeyList, lline ), (APTR)&azk, sizeof(struct AZKey) ))) goto END_ERROR;
            sc->NumKeys++;
            break; // sortie pour analyse de la commande
            }
        else if (*p == ',') // stockage de la touche
            {
            p++;
            if (code == -1) goto END_ERROR;
            azk.Code = code; azk.Qualifier = qual;
            if (! (sc = (struct AZShortcut *) UIK_nl_PasteToNode( &kh->KeyList, lline, UIK_nl_NodeBufLen( &kh->KeyList, lline ), (APTR)&azk, sizeof(struct AZKey) ))) goto END_ERROR;
            sc->NumKeys++;
            code = -1; qual = 0; // on continue pour un tour
            }
        else p++; // saute +-
        }

    parm = "";
    p += skip_char( p, ' ', '\t' );  // saute espaces entre key et cmdname
    if (*p)
        {
        if (! (mem = (UBYTE*)EqualReplace( kh->Pool, p )))
            {
            //if (! (mem = BufAllocP( kh->Pool, StrLen(p), 4 ))) goto END_ERROR;
            if (! (mem = AsmAllocPooled( kh->Pool, StrLen(p)+1, ExecBase ))) goto END_ERROR;
            StrCpy( mem, p );
            }
        cmdname = p = mem;
        p += find_char( p, ' ', '\t' ); // cherche espace de fin cmdname
        if (*p)
            {
            *p++ = 0;                  // cmdname OK
            parm = p + skip_char( p, ' ', '\t' ); // saute espaces entre cmdname et parms
            }
        }
    else goto END_ERROR;

    if (qual == SQUAL_NOSPECIAL || qual > 0x80000000)
        {
        if (menu) goto END_ERROR;

        pco = 0;
        switch (qual)
            {
            case SQUAL_NOSPECIAL:  pco = &kh->NoSpecial; break;
            case SPKEY_BEFORE_KEY: pco = &kh->BeforeKey; break;
            case SPKEY_AFTER_KEY:  pco = &kh->AfterKey;  break;
            case SPKEY_BEFORE_LINE:pco = &kh->BeforeLine;break;
            case SPKEY_AFTER_LINE: pco = &kh->AfterLine; break;
            default: goto END_ERROR; break;
            }
        UIK_nl_FreeNode( &kh->KeyList, lline );

        *pco = eng_NewObjectTags( cmdname, AZT_CmdLine, parm, AZT_ExeFrom, FROMKEY, TAG_END );
        }
    else{
        sc->CO = 0;
        sc->CmdName = cmdname;
        sc->CmdLine = parm;
        }

    return( sc );

  END_ERROR:
    if (lline != -1) UIK_nl_FreeNode( &kh->KeyList, lline );
    return(0);
}

ULONG interpret_keys( struct read_file *rf, ULONG append )
{
  ULONG len, rc=1;
  struct KeyHandle *kh = AGP.AZMast->KeyHandle;

    if (append == 0 || kh == 0)
        {
        if (! Parm_CleanupKey(0)) return(0);
        kh = AGP.AZMast->KeyHandle;
        }

    while (!rf->diese && rf->ptr < rf->max)
        {
        rf->ptr += skip_char( rf->ptr, ' ', '\t' );

        if (len = StrLen( rf->ptr ))
            {
            if (*rf->ptr == '#')
                {
                rf->diese = 1;
                }
            else{
                if ((rc = (ULONG) Keys_AddShortCut( rf->ptr, 0, 0 )) == 0) break;
                }
            }
        rf->ptr += len + 1;
        }
/*
{
ULONG ind, i;
struct UIKNode *node = kh->KeyList.UNode;
struct AZShortcut *sc=0;

    for (ind=0; ind < kh->KeyList.NodeNum; ind++, node++)
        {
        sc = (struct AZShortcut *) node->Buf;
        kprintf( "%ld", sc->NumKeys );

        for (i=0; i < sc->NumKeys; i++)
            {
            if (i) kprintf( "," );
            kprintf( "%lx+%lx", sc->Keys[i].Qualifier, sc->Keys[i].Code );
            }
        kprintf( " %ls\n", sc->CmdName );
        }
}
*/
    return( rc );
}

//-----------------------------------------

ULONG Parm_VerifKey( ULONG defaut )
{
  struct KeyHandle *kh = (struct KeyHandle *) AGP.AZMast->KeyHandle;

    if (kh == 0 || kh->KeyList.NodeNum == 0)
        {
        if (defaut)
            {
            struct read_file rf = {0,0,0,0,0,0,0,0,0,0};
            UBYTE *mem;
            ULONG rc=0;

            if (mem = UIK_MemSpAlloc( sizeof(defaults), MEMF_PUBLIC ))
                {
                CopyMem( defaults, mem, sizeof(defaults) );
                rf.ptr = mem;
                rf.max = mem + sizeof(defaults);
                rc = interpret_keys( &rf, 0 );
                UIK_MemSpFree( mem );
                }
            return( rc );
            }
        else return( Parm_CleanupKey(0) );
        }
    return(1);
}
