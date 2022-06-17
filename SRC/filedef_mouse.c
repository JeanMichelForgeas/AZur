/****************************************************************
 *
 *      File:      filedef_mouse.c
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

#include "uiki:objinputick.h"
#include "uiki:objwindow.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"
#include "lci:_pool_protos.h"

#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"


struct KeyNames
    {
    UBYTE *Name;
    ULONG Code;
    ULONG Qual;
    };


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ***********************************************/


/****** Statics ************************************************/

static struct KeyNames __far TabA[] = {
    { "lt",            -1, IEQUALIFIER_LALT },
    { "miga",          -1, IEQUALIFIER_LCOMMAND },
    { "nyMode",        -1, SQUAL_ANYSPECIAL },
    { "nySpecial",     -1, SQUAL_ANYSPECIAL },
    { 0 }, };
static struct KeyNames __far TabB[] = {
    { "utLeft",     IECODE_LBUTTON, 0 },
    { "utMiddle",   IECODE_MBUTTON, 0 },
    { "utRight",    IECODE_RBUTTON, 0 },
    { 0 }, };
static struct KeyNames __far TabC[] = {
    { "trl",           -1, IEQUALIFIER_CONTROL },
    { "ontrol",        -1, IEQUALIFIER_CONTROL },
    { 0 }, };
static struct KeyNames __far TabD[] = {
    { "ouble",         -1, SQUAL_DBLCLIC },
    { 0 }, };
static struct KeyNames __far TabJ[] = {
    { "oyDown",     SCODE_JOYDOWN, 0 },
    { "oyFire",     SCODE_JOYFIRE, 0 },
    { "oyLeft",     SCODE_JOYLEFT, 0 },
    { "oyRight",    SCODE_JOYRIGHT, 0 },
    { "oyTick",     SCODE_JOYTICK, 0 },
    { "oyUp",       SCODE_JOYUP, 0 },
    { 0 }, };
static struct KeyNames __far TabL[] = {
    { "ock1",          -1, SQUAL_LOCK1 },
    { "ock2",          -1, SQUAL_LOCK2 },
    { "ock3",          -1, SQUAL_LOCK3 },
    { "eft_Amiga",     -1, IEQUALIFIER_LCOMMAND },
    { "eftAmiga",      -1, IEQUALIFIER_LCOMMAND },
    { 0 }, };
static struct KeyNames __far TabM[] = {
    { "eta",           -1, SQUAL_QUAL1 },
    { "eta2",          -1, SQUAL_QUAL2 },
    { "ode",           -1, SQUAL_LOCK1 },
    { "ode2",          -1, SQUAL_LOCK2 },
    { "ouseMove",   SCODE_MOUSEMOVE, 0 },
    { 0 }, };
static struct KeyNames __far TabN[] = {
    { "oSpecial",      -1, SQUAL_NOSPECIAL },
    { "oRepeat",       -1, SQUAL_NOREPEAT },
    { 0 }, };
static struct KeyNames __far TabO[] = {
    { "therwise",      -1, SQUAL_NOSPECIAL },
    { 0 }, };
static struct KeyNames __far TabQ[] = {
    { "ualButLeft",    -1, IEQUALIFIER_LEFTBUTTON },
    { "ualButMiddle",  -1, IEQUALIFIER_MIDBUTTON },
    { "ualButRight",   -1, IEQUALIFIER_RBUTTON },
    { "ualJoyDown",    -1, SQUAL_JOYDOWN },
    { "ualJoyFire",    -1, SQUAL_JOYFIRE },
    { "ualJoyLeft",    -1, SQUAL_JOYLEFT },
    { "ualJoyRight",   -1, SQUAL_JOYRIGHT },
    { "ualJoyUp",      -1, SQUAL_JOYUP },
    { "ual1",          -1, SQUAL_QUAL1 },
    { "ual2",          -1, SQUAL_QUAL2 },
    { "ual3",          -1, SQUAL_QUAL3 },
    { 0 }, };
static struct KeyNames __far TabS[] = {
    { "hift",          -1, IEQUALIFIER_LSHIFT },
    { 0 }, };
static struct KeyNames __far TabT[] = {
    { "riple",         -1, SQUAL_TPLCLIC },
    { 0 }, };
static struct KeyNames __far TabW[] = {
    { "inTick",     SCODE_MOUSETICK, 0 },
    { 0 }, };

static struct KeyNames __far TabNull[] = {
    { 0 }, };

static struct KeyNames * __far TabNamesPtr[] = {
    TabA,
    TabB,
    TabC,
    TabD, TabNull, TabNull, TabNull, TabNull, TabNull,
    TabJ, TabNull,
    TabL,
    TabM,
    TabN,
    TabO, TabNull,
    TabQ, TabNull,
    TabS,
    TabT, TabNull, TabNull,
    TabW, TabNull, TabNull, TabNull,
    };

static UBYTE __far defaults[] =
"ButLeft Block POINTER MOVECURS BCHAR START CLEAR\0"
"ButLeft+CTRL Block STARTDO POINTER MOVECURS BCHAR\0"
"MouseMove+QUALBUTLEFT Block POINTER MOVECURS\0"
"MouseMove+QUALBUTLEFT+CTRL Block POINTER MOVECURS\0"
"#\0";

/****************************************************************
 *
 *      Keys handling
 *
 ****************************************************************/

ULONG Parm_InitMouse()
{
    return(1);
}

//-----------------------------------------

ULONG Parm_CleanupMouse( ULONG quit )
{
  struct MouseHandle *mh = AGP.AZMast->MouseHandle;
  APTR pool=0;

    if (mh)
        {
        eng_DisposeObject( mh->NoSpecial );
        AsmDeletePool( mh->Pool, ExecBase );
        AGP.AZMast->MouseHandle = 0;
        }
    if (! quit)
        {
        if (! (pool = AsmCreatePool( MEMF_ANY|MEMF_CLEAR, 500, 500, ExecBase ))) goto END_ERROR;
        if (! (mh = (struct MouseHandle *) BufAllocP( pool, sizeof(struct MouseHandle), sizeof(struct MouseHandle) ))) goto END_ERROR;
        if (! UIK_nl_AllocListP( pool, &mh->KeyList, 10, 0, 0, 0 )) goto END_ERROR;
        if (! (mh->CurKeys = (struct AZKey *) BufAllocP( pool, 0, 1*sizeof(struct AZKey) ))) goto END_ERROR;
        mh->Pool = pool;
        AGP.AZMast->MouseHandle = mh;
        }
    return(1);

  END_ERROR:
    if (pool) AsmDeletePool( pool, ExecBase );
    return(0);
}

//-----------------------------------------

void Mouse_Joy( struct MouseHandle *kh, ULONG code, ULONG qual, ULONG from )
{
  struct KeyHandle *keyh = (struct KeyHandle *) AGP.AZMast->KeyHandle;
  UBYTE any=0;

    kh->RecKey.Code = code;

    //------------- kh->RecKey.Qualifier
    kh->RecKey.Qualifier = (qual & (SQUAL_MASK|SQUAL_MJOY));
    kh->RecKey.Qualifier |= keyh->AZQual;

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
                        {
                        break;
                        }
                    }
                if (i > kh->IndexCurKey)
                    {
                    kh->KeyFound = sc;
                    break;
                    }
                }
            }
        }

    if (kh->KeyFound)
        {
        kh->IndexCurKey++;
        if (sc->NumKeys == kh->IndexCurKey)
            {
            struct CmdObj *co = kh->KeyFound->CO;
            ULONG dispose=0;

            kh->IndexCurKey = 0;
            BufTruncate( (APTR)kh->CurKeys, 0 );
            keyh->AZQual &= ~(SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3); // avant car l'action peut les mettre ON

            if (! co)
                {
                if (co = eng_NewObjectTags( kh->KeyFound->CmdName, AZT_CmdLine, kh->KeyFound->CmdLine, AZT_ExeFrom, from, TAG_END ))
                    dispose = 1;
                }
            if (co) func_ExecuteCmdObj( co, 0 );
            if (dispose) eng_DisposeObject( co );
            //func_ExecuteCmdObj( kh->KeyFound, 0 );
            }
        }
    else{
        kh->IndexCurKey = 0;
        if ((any == 0) && (kh->RecKey.Qualifier & (SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3|SQUAL_LOCK1|SQUAL_LOCK2|SQUAL_LOCK3)))
            {
            any = 1;
            kh->RecKey.Qualifier &= ~(SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3|SQUAL_LOCK1|SQUAL_LOCK2|SQUAL_LOCK3);
            kh->RecKey.Qualifier |= SQUAL_ANYSPECIAL;
            kh->CurKeys[kh->IndexCurKey].Qualifier = kh->RecKey.Qualifier;
            goto RETRY_ANYSPECIAL;
            }
        else if (any)
            {
            if (kh->NoSpecial) func_ExecuteCmdObj( kh->NoSpecial, 0 );
            else if (kh->RecKey.Qualifier & SQUAL_ANYSPECIAL)
                {
                kh->RecKey.Qualifier &= ~SQUAL_ANYSPECIAL; /* 3ème passage : touche normale */
                kh->CurKeys[kh->IndexCurKey].Qualifier = kh->RecKey.Qualifier;
                any = 0;
                goto RETRY_ANYSPECIAL;
                }
            }
        BufTruncate( (APTR)kh->CurKeys, 0 );
        keyh->AZQual &= ~(IEQUALIFIER_LSHIFT|IEQUALIFIER_LALT|IEQUALIFIER_CONTROL|IEQUALIFIER_LCOMMAND|SQUAL_QUAL1|SQUAL_QUAL2|SQUAL_QUAL3);
        }
    }
}

void FromInpuTick( struct UIKObjInputick *ipt, UBYTE buttons, UBYTE changes )
{
  struct MouseHandle *kh = (struct MouseHandle *) AGP.AZMast->MouseHandle;
  ULONG sysquals, quals, codes;

    if (TST_INPUTLOCK) return;

    quals = ((ULONG)(buttons & (buttons ^ changes))) << 27; // grâce à la correspondance avec les évènements UIK
    codes = ((ULONG)(changes & buttons)) << 8;
    if (codes == 0) // ne parcours pas la liste si pas de SCODE_JOYTICK
        {
        if (kh->TickFlags & SCODE_JOYTICK) codes = SCODE_JOYTICK;
        else return;
        }

    sysquals = AGP.FObj->SysQuals & (IEQUALIFIER_LSHIFT|IEQUALIFIER_LALT|IEQUALIFIER_CONTROL|IEQUALIFIER_LCOMMAND|IEQUALIFIER_MIDBUTTON|IEQUALIFIER_RBUTTON|IEQUALIFIER_LEFTBUTTON);
    if (AGP.FObj->SysQuals & (IEQUALIFIER_RSHIFT|IEQUALIFIER_RALT|IEQUALIFIER_RCOMMAND))
        {
        if (AGP.FObj->SysQuals & IEQUALIFIER_RSHIFT)   sysquals |= IEQUALIFIER_LSHIFT;
        if (AGP.FObj->SysQuals & IEQUALIFIER_RALT)     sysquals |= IEQUALIFIER_LALT;
        if (AGP.FObj->SysQuals & IEQUALIFIER_RCOMMAND) sysquals |= IEQUALIFIER_LCOMMAND;
        }
    quals |= sysquals;

    kh->JoyQuals = quals & (SQUAL_JOYRIGHT|SQUAL_JOYLEFT|SQUAL_JOYDOWN|SQUAL_JOYUP|SQUAL_JOYFIRE);
    Mouse_Joy( kh, codes, quals, FROMJOY );
}

//---------------------------------------------------------

static LONG trouve_ecart()
{
  struct AZObjFile *fo = AGP.FObj;
  LONG ecart;
  UBYTE len = fo->ScrollLen;

    ecart = ABS(fo->MouseX - fo->OldMouseX) + ABS(fo->MouseY - fo->OldMouseY);
    /*
    fo->ScrollLen = (ecart /= 7) > 5 ? 5 : ecart;
    if (! fo->ScrollLen) fo->ScrollLen = 1;
    */
    if (ecart < 5)       { if (len < 1) len++; else if (len >  1) len--; }
    else if (ecart < 10) { if (len < 2) len++; else if (len >  2) len--; }
    else if (ecart < 15) { if (len < 3) len++; else if (len >  3) len--; }
    else                 { if (len < 4) len++; else if (len > 15) len--; }
    return( (LONG)(fo->ScrollLen = len) );
}

static void ScrollMouse()
{
  struct AZObjFile *fo = AGP.FObj;

    if (fo->WO && fo->WO->Obj.Status == UIKS_STARTED && !TST_DISPLOCK)
        {
        struct AZObjView *vo = fo->ActiveView;
        struct AZObjZone *zo = vo->Zone;
        struct Rectangle *rect = &zo->Obj.ObjExt->InnerRect;
        LONG newtopline = zo->TopLine, newtopcol = zo->TopCol;

        if (fo->MemButs & (IEQUALIFIER_LEFTBUTTON|IEQUALIFIER_MIDBUTTON|IEQUALIFIER_RBUTTON))
            {
            if (fo->MouseY < rect->MinY)      newtopline -= trouve_ecart(); // haut
            else if (fo->MouseY > rect->MaxY) newtopline += trouve_ecart(); // bas

            if (fo->MouseX < rect->MinX)      newtopcol -= trouve_ecart();  // gauche
            else if (fo->MouseX > rect->MaxX) newtopcol += trouve_ecart();  // droite

            if (newtopline == zo->TopLine && newtopcol == zo->TopCol) fo->ScrollLen = 0;
            else if (oview_ChangeViewPosition( vo, newtopline, newtopcol )) UPDATE_PROP(fo); //UPDATE_STAT_PROP(fo);
            }
        else fo->ScrollLen = 0;
        }
}

void FromMouse( UWORD rcode, ULONG from )
{
  struct MouseHandle *kh = (struct MouseHandle *) AGP.AZMast->MouseHandle;
  struct AZObjFile *fo = AGP.FObj;
  ULONG sysquals, qual, code=rcode;
  struct TimeLimit *tl=0;

    if (TST_INPUTLOCK) return;

    if (code == SCODE_MOUSETICK) // ne parcours pas la liste si pas de SCODE_MOUSETICK
        {
        ScrollMouse();
        if (! (kh->TickFlags & SCODE_MOUSETICK)) return;
        }

    if (code != SCODE_MOUSETICK) set_intui_vars(); // déjà fait par WindowTick() dans objfile.c
                                                   // sinon gêne la suite de WindowTick()
    sysquals = fo->SysQuals;
    qual = sysquals & (IEQUALIFIER_LSHIFT|IEQUALIFIER_LALT|IEQUALIFIER_CONTROL|IEQUALIFIER_LCOMMAND|IEQUALIFIER_MIDBUTTON|IEQUALIFIER_RBUTTON|IEQUALIFIER_LEFTBUTTON);
    if (sysquals & (IEQUALIFIER_RSHIFT|IEQUALIFIER_RALT|IEQUALIFIER_RCOMMAND))
        {
        if (sysquals & IEQUALIFIER_RSHIFT) qual |= IEQUALIFIER_LSHIFT;
        if (sysquals & IEQUALIFIER_RALT) qual |= IEQUALIFIER_LALT;
        if (sysquals & IEQUALIFIER_RCOMMAND) qual |= IEQUALIFIER_LCOMMAND;
        }
    switch (code)
        {
        case IECODE_LBUTTON: tl = &kh->TLL; qual &= ~IEQUALIFIER_LEFTBUTTON; func_RemoveNullPointer(); break;
        case IECODE_MBUTTON: tl = &kh->TLM; qual &= ~IEQUALIFIER_MIDBUTTON; func_RemoveNullPointer(); break;
        case IECODE_RBUTTON: if (kh->TickFlags & SCODE_TESTRBUT) { tl = &kh->TLR; qual &= ~IEQUALIFIER_RBUTTON; } func_RemoveNullPointer(); break;
        case SCODE_MOUSEMOVE: func_RemoveNullPointer(); ScrollMouse();
        break;
        }
    if (tl)
        {
        tl->Seconds = fo->Seconds; tl->Micros = fo->Micros;
        if ((ABS(fo->MouseX - tl->OldMouseX) <= 2 && ABS(fo->MouseY - tl->OldMouseY) <= 1)
            && DoubleClick( tl->StartSeconds, tl->StartMicros, tl->Seconds, tl->Micros ))
            {
            switch (tl->ClicStatus)
                {
                case 0: tl->ClicStatus = 2; qual |= SQUAL_DBLCLIC; break;
                case 1: tl->ClicStatus = 2; qual |= SQUAL_DBLCLIC; break;
                case 2: tl->ClicStatus = 3; qual |= SQUAL_TPLCLIC; break;
                case 3: tl->ClicStatus = 0; break;
                }
            }
        else{
            tl->ClicStatus = 0;
            }
        tl->StartSeconds = tl->Seconds; tl->StartMicros = tl->Micros;
        tl->StartLine = fo->Line; tl->StartCol = fo->Col;
        tl->OldMouseX = fo->MouseX; tl->OldMouseY = fo->MouseY;
        }

//kprintf( "code=%lx, ext=%lx, qual=%lx\n", code, AGP.FObj->SysQuals, qual & (SQUAL_MASK|SQUAL_MJOY) );
    Mouse_Joy( kh, code, qual, FROMMOUSE );
}

//-------------------------------------------------------------------------

ULONG interpret_mouse( struct read_file *rf, ULONG append )
{
  ULONG len;
  struct MouseHandle *kh = AGP.AZMast->MouseHandle;
  struct AZShortcut *sc=0;
  struct CmdObj **pco;
  UBYTE *cmdname, *parm, *mem=0;
  LONG lline=-1;

    if (append == 0 || kh == 0)
        {
        if (! Parm_CleanupMouse(0)) return(0);
        kh = AGP.AZMast->MouseHandle;
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
                UBYTE *p = rf->ptr, *end;
                ULONG data, code, qual;
                struct AZKey azk;
                struct KeyNames *kn;

                lline = -1;
                if (! (sc = (struct AZShortcut *) UIK_nl_AllocNode( &kh->KeyList, -1, sizeof(struct AZShortcut)-sizeof(struct AZKey), 0 ))) goto END_ERROR;
                lline = kh->KeyList.NodeNum - 1;

                code = -1; qual = 0;
                while (TRUE)
                    {
                    //-------------------- cherche code+qualifier
                    if (*p == 0) goto END_ERROR;
                    end = FindTabName( TabNamesPtr, p, sizeof(struct KeyNames), (APTR)&kn );

                    if (end == 0) goto END_ERROR;
                    else{
                        data = kn->Code;
                        if (data != -1) code = data;
                        else qual |= kn->Qual;
                        p = end;
                        }

                    //-------------------- fin de séquence?
                    if (qual == SQUAL_NOSPECIAL) break;

                    if (*p == ' ' || *p == '\t') // stockage de la touche
                        {
                        if (code == -1) goto END_ERROR;
                        if (code & SCODE_MOUSETICK) kh->TickFlags |= SCODE_MOUSETICK;
                        if (code & SCODE_JOYTICK) kh->TickFlags |= SCODE_JOYTICK;
                        if (code == IECODE_RBUTTON) kh->TickFlags |= SCODE_TESTRBUT;
                        azk.Code = code; azk.Qualifier = qual;
                        if (! (sc = (struct AZShortcut *) UIK_nl_PasteToNode( &kh->KeyList, lline, UIK_nl_NodeBufLen( &kh->KeyList, lline ), (APTR)&azk, sizeof(struct AZKey) ))) goto END_ERROR;
                        sc->NumKeys++;
                        break; // sortie pour analyse de la commande
                        }
                    else if (*p == ',') // stockage de la touche
                        {
                        if (code == -1) goto END_ERROR;
                        if (code & SCODE_MOUSETICK) kh->TickFlags |= SCODE_MOUSETICK;
                        if (code & SCODE_JOYTICK) kh->TickFlags |= SCODE_JOYTICK;
                        if (code == IECODE_RBUTTON) kh->TickFlags |= SCODE_TESTRBUT;
                        azk.Code = code; azk.Qualifier = qual;
                        if (! (sc = (struct AZShortcut *) UIK_nl_PasteToNode( &kh->KeyList, lline, UIK_nl_NodeBufLen( &kh->KeyList, lline ), (APTR)&azk, sizeof(struct AZKey) ))) goto END_ERROR;
                        sc->NumKeys++;
                        code = -1; qual = 0; // on continue pour un tour
                        }

                    p++; // saute +-,
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

                if (qual == SQUAL_NOSPECIAL)
                    {
                    pco = &kh->NoSpecial;
                    UIK_nl_FreeNode( &kh->KeyList, lline );

                    *pco = eng_NewObjectTags( cmdname, AZT_CmdLine, parm, AZT_ExeFrom, FROMMOUSE, TAG_END );
                    }
                else{
                    sc->CO = 0;
                    sc->CmdName = cmdname;
                    sc->CmdLine = parm;
                    }
                }
            }
        rf->ptr += len + 1;
        }
/*
{
ULONG ind, i;
struct UIKNode *node = kh->KeyList.UNode;

    for (ind=0; ind < kh->KeyList.NodeNum; ind++, node++)
        {
        sc = (struct AZShortcut *) node->Buf;
        kprintf( "%ld: ", sc->NumKeys );

        for (i=0; i < sc->NumKeys; i++)
            {
            if (i) kprintf( "," );
            kprintf( "%lx+%lx", sc->Keys[i].Qualifier, sc->Keys[i].Code );
            }
        kprintf( " %ls\n", sc->CO->RegGerm->Names[0] );
        }
}
*/
    return(1);

  END_ERROR:
    if (lline != -1) UIK_nl_FreeNode( &kh->KeyList, lline );
    return(0);
}

//-----------------------------------------

ULONG Parm_VerifMouse( ULONG defaut )
{
  struct MouseHandle *kh = (struct MouseHandle *) AGP.AZMast->MouseHandle;

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
                rc = interpret_mouse( &rf, 1 );
                UIK_MemSpFree( mem );
                }
            return( rc );
            }
        else return( Parm_CleanupMouse(0) );
        }
    return(1);
}
