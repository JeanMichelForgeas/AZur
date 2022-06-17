/****************************************************************
 *
 *      File:       funcs_reqs.c    fonctions de requesters
 *      Project:    AZur            editeur de texte
 *
 *      Created:    06/05/93        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uikbase.h"
#include "uiki:uikobj.h"
#include "uiki:uikgfx.h"
#include "uiki:uikdrawauto.h"
#include "uiki:uikmacros.h"
#include "uiki:uikglobal.h"
#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objchecktoggle.h"
#include "uiki:objradioexclude.h"
#include "uiki:objarexxsim.h"
#include "uiki:objstring.h"
#include "uiki:objmemory.h"
#include "uiki:objlist.h"
#include "uiki:objscreen.h"
#include "uiki:objwindow.h"
#include "uiki:objreqstring.h"
#include "uiki:objwinfontsel.h"
#include "uiki:objdisplaymode.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"
#include "lci:azur_protos.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern struct UIKBitMap * __far UIKBitMapTable[];
extern UBYTE __far VersionTag[];
extern UBYTE * __far VersionText;
extern UBYTE * __far LangEnglish[];
extern UBYTE * __far LangOther[];

extern struct TagItem __far IntuitionScreenTags;

extern struct AZLayout __far AZL_Search[];

extern ULONG sync_request( struct UIKObj *rs );


/****** Exported ************************************************/


/****** Statics *************************************************/

static UWORD __far NullAuto[] = {
    END_AUTO
    };

/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static struct Window *verif_window( struct Window *win )
{
  ULONG ilock;
  struct Screen *sc;
  struct Window *w=0;

    if (!win) return(0);
    ilock = LockIBase( 0 );
    sc = IntuitionBase->FirstScreen;
    while (sc)
        {
        w = sc->FirstWindow;
        while (w)
            {
            if (w == win) goto END_ERROR;
            w = w->NextWindow;
            }
        sc = sc->NextScreen;
        }
  END_ERROR:
    UnlockIBase( ilock );
    return( w );
}

/* si w == 0 ou inexistante --> fenêtre principale AZur
 */
static struct Window *activate_window( struct Window *w )
{
  struct Window *act;

    if (w) w = verif_window( w );
    if (!w) w = AGP.WO->Obj.w;

    if (/*!TST_REQWIN &&*/ w && !(w->Flags & WFLG_WINDOWACTIVE))
        {
        act = IntuitionBase->ActiveWindow;
        ActivateWindow( w );
        return( act );
        }
    else return(0);
}

static void setbuffers( struct UIKObjReqString *rs )
{
    if (rs->Str)
        {
        if (rs->Str->Obj.UserULong1)
            {
            ULONG len; UBYTE *ptr;
            ptr = UIK_OSt_Get( rs->Str, &len );
            BufSetS( ptr, len, (UBYTE**)rs->Str->Obj.UserULong1 );
            }
        }
    if (rs->Str2)
        {
        if (rs->Str2->Obj.UserULong1)
            {
            ULONG len; UBYTE *ptr;
            ptr = UIK_OSt_Get( rs->Str2, &len );
            BufSetS( ptr, len, (UBYTE**)rs->Str2->Obj.UserULong1 );
            }
        }
}

struct ASSR {
    struct Window *ActWin;
    void (*func)();
    };

static void cleanup_asyncrs( struct UIKObjReqString *rs )
{
  struct ASSR *as = (APTR)rs->Obj.UserULong1;
  struct AZRequest *ar = (APTR)rs->Obj.UserULong2;

    if (as->func) (*as->func)( ar );

    if ((ar->Button != REQBUT_OK && ar->Button != REQBUT_MIDDLE) || !(ar->Flags & ARF_NOTERM))
        {
        func_ReqClose( 0, ar );
        activate_window( as->ActWin );
        UIK_MemSpFree( as );
        }
}

static void sm_okfunc( int nil, struct UIKObjReqString *rs )
{
  struct AZRequest *ar = (struct AZRequest *)rs->Obj.UserULong2;

    setbuffers( rs );
    ar->Button = REQBUT_OK;
    if (ar->Flags & ARF_ASYNC) cleanup_asyncrs( rs );
    else rs->Obj.UserULong1 = REQBUT_OK;
}

static void sm_middlefunc( int nil, struct UIKObjReqString *rs )
{
  struct AZRequest *ar = (struct AZRequest *)rs->Obj.UserULong2;

    setbuffers( rs );
    ar->Button = REQBUT_MIDDLE;
    if (ar->Flags & ARF_ASYNC) cleanup_asyncrs( rs );
    else rs->Obj.UserULong1 = REQBUT_MIDDLE;
}

static void sm_cancelfunc( int nil, struct UIKObjReqString *rs )
{
  struct AZRequest *ar = (struct AZRequest *)rs->Obj.UserULong2;

    ar->Button = REQBUT_CANCEL;
    if (ar->Flags & ARF_ASYNC) cleanup_asyncrs( rs );
    else rs->Obj.UserULong1 = REQBUT_CANCEL;
}

/** rs->UserULong1 est utilisé pour la synchronisation des requesters
 ** rs->UserULong2 est utilisé par SmartRequest pour stocker la struct AZRequest
 **/

/* Attention : si utilisation de AllocNewStack() / RestoreStack() !!!
 * Codée en assembleur à cause des problèmes d'adressage de stackbuf
 * dans la pile, qui pourrait être adressé relatif à SP au lieu de A5
 * dans d'autres versions du compilateur.
 */

static void okfunc( int nil, struct UIKObjReqString *rs )
{
    setbuffers( rs );
    rs->Obj.UserULong1 = REQBUT_OK;
}

static void req_rawkeys( int nil, struct UIKObjReqString *obj )
{
  struct IntuiMessage *imsg = obj->Obj.UIK->IntuiMsg;

    if (imsg && imsg->Code == 0x45) okfunc( 0, obj );
}

static void req_rawkeys_async( int nil, struct UIKObjReqString *obj )
{
  struct IntuiMessage *imsg = obj->Obj.UIK->IntuiMsg;

    if (imsg && imsg->Code == 0x45) UIK_Remove( obj );
}

ULONG sync_request( struct UIKObj *rs )
{
  ULONG rest;

    rs->UserULong1 = -1;

    if ((rest = StackRest()) < 3500)
        {
        // kprintf( "Attention : stack=%ld\n", rest );
        if (rest < 2000) return( REQBUT_CANCEL );
        }

    while (rs->UserULong1 == -1)
        {
        ULONG events;
        events = UIK_WaitFirstEvents( AGP.UIK, SIGBREAKF_CTRL_C );
        if (events & SIGBREAKF_CTRL_C)
            {
            Signal( FindTask(0), SIGBREAKF_CTRL_C );
            rs->UserULong1 = REQBUT_CANCEL;
            }
        else if (AGP.UIK->Obj.Status != UIKS_STARTED) rs->UserULong1 = REQBUT_CANCEL;
        }

    return( rs->UserULong1 );
}

//---------------------------------------------------------------

ULONG func_AskReqBegin( UBYTE *text, UBYTE *title )
{
    if (AGP.WO)
        {
        if (UIK_UniconifyWindow( AGP.WO ))
            {
            struct UIKObjReqString *obj;
            struct TagItem tl[] = { UIKTAG_OBJ_BoxFl_NotDrawn, TRUE, TAG_END };

            if (obj = (struct UIKObjReqString *) UIK_AddObjectTagsB( UIKBase, "UIKObj_ReqString", AGP.WO,
                UIKTAG_ReqString_TextPtr,       text,
                UIKTAG_ReqString_OKFunc,        okfunc,
                UIKTAG_ReqString_CloseFunc,     okfunc,
                UIKTAG_ReqString_OKTitlePtr,    title,
                UIKTAG_ReqString_TextListTagList, tl,
                UIKTAG_ReqStringFl_Window,      TST_REQWIN,
                TAG_END ))
                {
                struct Window *w = IntuitionBase->ActiveWindow;
                UIK_AddVector( obj->Req, (void*)req_rawkeys, RAWKEY, (ULONG)obj );
                obj->Obj.UserULong1 = -1;
                obj->Obj.UserULong2 = (ULONG) w;
                if (UIK_Start( obj ))
                    {
                    activate_window( obj->Req->w );
                    if (AGP.FObj)
                        {
                        UIK_InactivateKeyTank( AGP.FObj );
                        UIK_InactivateKeyTank( AGP.FObj->CmdLine );
                        }
                    return( (ULONG)obj );
                    }
                UIK_Remove( obj );
                func_RemoveNullPointer();
                activate_window(w);
                }
            }
        }
    return(0);
}

ULONG func_AskReqQuery( ULONG handle )
{
  ULONG rc=-1;
  struct UIKObj *obj = (struct UIKObj *)handle;

    if (obj)
        {
        if (UIK_ProcessPendingEvents( AGP.UIK, SIGBREAKF_CTRL_C ) & SIGBREAKF_CTRL_C)
            {
            Signal( FindTask(0), SIGBREAKF_CTRL_C );
            rc = REQBUT_CANCEL;
            }
        else{
            rc = obj->UserULong1;
            }
        }
    return( rc );
}

void func_AskReqEnd( ULONG handle )
{
    if (handle) activate_window( (struct Window *) ((struct UIKObj *)handle)->UserULong2 );
    UIK_Remove( (struct UIKObj *)handle );
    func_RemoveNullPointer();
}

//---------------------------------------------------------------

void func_ReqShowText( UBYTE *text )
{
  struct UIKObjReqString *obj;
  struct UIKObj *wo=AGP.WO;

    if (wo)
        {
        if (UIK_UniconifyWindow( (struct UIKObjWindow *)wo ))
            {
            if (obj = (struct UIKObjReqString *) UIK_AddObjectTagsB( UIKBase, "UIKObj_ReqString", wo,
                UIKTAG_ReqString_TextPtr, text,
                UIKTAG_ReqStringFl_Window,TST_REQWIN,
                UIKTAG_ReqString_OKFunc,  okfunc,
                UIKTAG_ReqString_CloseFunc, okfunc,
                TAG_END ))
                {
                struct Window *w = IntuitionBase->ActiveWindow;
                UIK_AddVector( obj->Req, (void*)req_rawkeys, RAWKEY, (ULONG)obj );
                func_RemoveNullPointer();
                if (UIK_Start( obj ))
                    {
                    activate_window( obj->Req->w );
                    if (AGP.FObj)
                        {
                        UIK_InactivateKeyTank( AGP.FObj );
                        UIK_InactivateKeyTank( AGP.FObj->CmdLine );
                        }
                    sync_request( obj );
                    }
                UIK_Remove( obj );
                activate_window(w);
                }
            }
        }
}

static ULONG bouchon() { return(1); }

static struct TagItem __far smartreq_tags1[] =
    {
    { UIKTAG_OBJ_Title },
    { UIKTAG_OBJ_TitleFl_Addr, TRUE },
    { UIKTAG_OBJ_TitleUnderscore, '_' },
    //{ UIKTAG_OBJ_FontName, 0 },
    //{ UIKTAG_OBJ_FontHeight, 0 },
    { TAG_END }
    };
static struct TagItem __far smartreq_tags2[] =
    {
    { UIKTAG_OBJ_Title },
    { UIKTAG_OBJ_TitleFl_Addr, TRUE },
    { UIKTAG_OBJ_TitleUnderscore, '_' },
    //{ UIKTAG_OBJ_FontName, 0 },
    //{ UIKTAG_OBJ_FontHeight, 0 },
    { TAG_END }
    };
static struct TagItem __far smartreq_tags[] = {
    { UIKTAG_ReqString_TextPtr,     },
    { UIKTAG_ReqString_OKFunc,      sm_okfunc },
    { UIKTAG_ReqString_CancelFunc,  sm_cancelfunc },
    { UIKTAG_ReqString_MiddleFunc,  sm_middlefunc },
    { UIKTAG_ReqString_OKTitlePtr,     0 },
    { UIKTAG_ReqString_CancelTitlePtr, 0 },
    { UIKTAG_ReqString_MiddleTitlePtr, 0 },
    { UIKTAG_ReqString_StringPtr,   },
    { UIKTAG_ReqString_StringPtr2,  },
    { UIKTAG_ReqString_StringTagList,  0 },
    { UIKTAG_ReqString_StringTagList2, 0 },
    { UIKTAG_OBJ_Title },
    { UIKTAG_OBJ_TitleFl_Addr,        FALSE },
    { UIKTAG_ReqStringFl_Window },
    { UIKTAG_ReqStringFl_Activate,    FALSE },
    { UIKTAG_ReqStringFl_StringQuiet, TRUE },
    { TAG_IGNORE, 0 },
    { UIKTAG_OBJ_BoxFl_TranslateLeftTop, FALSE },
    { TAG_END } };

static ULONG smart_request( struct UIKObj *parent, ULONG titleindex, UBYTE *titleptr, ULONG textindex, UBYTE *textptr, UBYTE *title1, UBYTE **buf1, UBYTE *title2, UBYTE **buf2, UBYTE *ok, UBYTE *mid, UBYTE *can, ULONG (*AddFunc)(), ULONG rsflags, void (*afterfunc)(), struct AZRequest *ar )
{
  ULONG started=0, code = REQBUT_OK;
  struct UIKObj *obj;

    if (! AddFunc) AddFunc = bouchon;

    if (!parent) parent = AGP.WO;

    if (parent && (!ar || !ar->Obj))
        {
        struct UIKObjReqString *rs;
        struct TagItem *tls1 = (APTR) UIK_MemSpAlloc( sizeof(smartreq_tags1)+sizeof(smartreq_tags2)+sizeof(smartreq_tags), MEMF_ANY );
        struct TagItem *tls2 = (APTR) ((ULONG)tls1 + sizeof(smartreq_tags1));
        struct TagItem *tl = (APTR) ((ULONG)tls2 + sizeof(smartreq_tags2));

        if (tls1 && tls2 && tl)
            {
            CopyMem( smartreq_tags, tl, sizeof(smartreq_tags) );
            CopyMem( smartreq_tags1, tls1, sizeof(smartreq_tags1) );
            CopyMem( smartreq_tags2, tls2, sizeof(smartreq_tags2) );
            //tls1[3].ti_Data = tls2[3].ti_Data = (ULONG)AGP.Prefs->ViewFontName;
            //tls1[4].ti_Data = tls2[4].ti_Data = AGP.Prefs->ViewFontHeight;

            if (UIK_UniconifyWindow( (APTR)parent ))
                {
                tls1[0].ti_Data = (ULONG)title1; tls2[0].ti_Data = (ULONG)title2;
                tl[9].ti_Data = (ULONG)tls1; tl[10].ti_Data = (ULONG)tls2;

                if (textindex) tl[0].ti_Data = (ULONG)UIK_LangString( AGP.UIK, textindex );
                else tl[0].ti_Data = (ULONG)textptr;

                if (ok == (UBYTE*)-1 || ok == 0) tl[4].ti_Tag = TAG_IGNORE;
                else tl[4].ti_Data = (ULONG)ok;

                if (can == (UBYTE*)-1) tl[5].ti_Tag = TAG_IGNORE;
                else if (can == 0) tl[2].ti_Tag = TAG_IGNORE;
                else tl[5].ti_Data = (ULONG)can;

                if (mid == (UBYTE*)-1) tl[6].ti_Tag = TAG_IGNORE;
                else if (mid == 0) tl[3].ti_Tag = TAG_IGNORE;
                else tl[6].ti_Data = (ULONG)mid;

                if (buf1 == 0) tl[7].ti_Tag = TAG_IGNORE; else tl[7].ti_Data = (ULONG)*buf1;
                if (buf2 == 0) tl[8].ti_Tag = TAG_IGNORE; else tl[8].ti_Data = (ULONG)*buf2;

                if (! (tl[11].ti_Data = titleindex))
                    {
                    if (tl[11].ti_Data = (ULONG)titleptr) tl[12].ti_Data = TRUE;
                    }

                tl[13].ti_Data = (rsflags & ARF_WINDOW) || TST_REQWIN ? TRUE : FALSE;
                tl[14].ti_Data = rsflags & ARF_ACTIVATE ? TRUE : FALSE;
                tl[15].ti_Data = rsflags & ARF_RETQUIET ? TRUE : FALSE;

                if (ar)
                    {
                    if (*((ULONG*)&ar->Width) && *((ULONG*)&ar->Left) != -1)
                        {
                        tl[16].ti_Tag = UIKTAG_OBJ_LeftTop;
                        tl[16].ti_Data = *((ULONG*)&ar->Left);
                        }
                    }

                if (rs = (struct UIKObjReqString *) UIK_AddObject( "UIKObj_ReqString", parent, tl ))
                    {
                    struct Window *w = IntuitionBase->ActiveWindow;
                    if (! ar)
                        {
                        if (obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_Memory", rs,
                                UIKTAG_OBJMem_MemSize,  sizeof(struct AZRequest),
                                UIKTAG_OBJMem_MemType,  MEMF_CLEAR,
                                TAG_END ))
                            { ar = (struct AZRequest *) UIK_OMe_MemPtr( obj ); }
                        }
                    if (ar)
                        {
                        ar->Obj = rs;
                        ar->WO = (APTR)rs->Req;
                        ar->Flags |= rsflags;
                        rs->Obj.UserULong2 = (ULONG)ar;

                        if (AddFunc( ar ))
                            {
                            func_RemoveNullPointer();
                            if (UIK_Start( rs ))
                                {
                                activate_window( rs->Req->w );
                                if (AGP.FObj)
                                    {
                                    UIK_InactivateKeyTank( AGP.FObj );
                                    UIK_InactivateKeyTank( AGP.FObj->CmdLine );
                                    }
                                if (rs->Str) rs->Str->Obj.UserULong1 = (ULONG)buf1;
                                if (rs->Str2) rs->Str2->Obj.UserULong1 = (ULONG)buf2;

                                if (ar->Flags & ARF_ASYNC)
                                    {
                                    struct ASSR *as;
                                    if (as = (APTR) UIK_MemSpAlloc( sizeof(struct ASSR), MEMF_ANY ))
                                        {
                                        as->ActWin = w;
                                        as->func = afterfunc;
                                        rs->Obj.UserULong1 = (ULONG) as;
                                        started = 1;
                                        }
                                    }
                                else{
                                    code = sync_request( rs );
                                    ar->Button = code;
                                    if (code == REQBUT_CANCEL) ar->ErrMsg = TEXT_CMDCANCELLED;
                                    }
                                }
                            else { code = ar->Button = REQBUT_CANCEL; ar->ErrMsg = TEXT_CMDCANCELLED; }
                            }
                        else { code = ar->Button = REQBUT_CANCEL; ar->ErrMsg = TEXT_CMDCANCELLED; }
                        }
                    if (! started)
                        {
                        if (ar) func_ReqClose( 0, ar ); else UIK_Remove( rs );
                        activate_window( w );
                        }
                    }
                }
            }
        UIK_MemSpFree( tls1 );
        }
    return( code );
}

ULONG func_SmartRequest( UBYTE *titleptr, UBYTE *textptr, UBYTE *title1, UBYTE **buf1, UBYTE *title2, UBYTE **buf2, UBYTE *ok, UBYTE *mid, UBYTE *can, ULONG (*AddFunc)(), ULONG rsflags )
{
    return( smart_request( 0, 0, titleptr, 0, textptr, title1, buf1, title2, buf2, ok, mid, can, AddFunc, rsflags, 0, 0 ) );
}

ULONG func_IntSmartRequest( struct UIKObj *parent, ULONG titleindex, UBYTE *titleptr, ULONG textindex, UBYTE *textptr, ULONG title1, UBYTE **buf1, ULONG title2, UBYTE **buf2, ULONG ok, ULONG mid, ULONG can, ULONG (*AddFunc)(), ULONG rsflags, void (*afterfunc)(), struct AZRequest *ar )
{
    return( smart_request( parent, titleindex, titleptr, textindex, textptr,
        UIK_LangString( AGP.UIK, title1 ), buf1,
        UIK_LangString( AGP.UIK, title2 ), buf2,
        (ok  == -1) ? -1 : UIK_LangString( AGP.UIK, ok ),
        (mid == -1) ? -1 : UIK_LangString( AGP.UIK, mid ),
        (can == -1) ? -1 : UIK_LangString( AGP.UIK, can ),
        AddFunc, rsflags, afterfunc, ar ) );
}

//=====================================================================================

UBYTE *make_listptr( struct TextFont *font, WORD totwidth, WORD maxheight, WORD *plh )
{
  WORD num, len, i, cw;
  UBYTE *mem;

    num = (maxheight + (font->tf_YSize-1)) / font->tf_YSize;
    num--; // bug dans le contage du ReqString
    cw = UIK_CharWidth( font, ' ' );
    len = num + 1 + totwidth / cw;
    if (! (mem = UIK_MemSpAlloc( len, MEMF_ANY ))) return(0);
    for (i=0; i < num; i++) mem[i] = '\n';
    for (i=num; i < len; i++) mem[i] = ' ';
    mem[len-1] = 0;
    *plh = (num+1) * font->tf_YSize; // num + 1 à cause du bug dans le contage du ReqString
    return( mem );
}

void __asm getgad_check( register __a0 struct AZLayout *al, register __d0 UBYTE num, register __a1 ULONG *pflags, register __d1 ULONG flag )
{
    if (al[num].Obj->Flags & BOX_OVER) *pflags &= ~flag; else *pflags |= flag;
}

ULONG __asm getgad_radio( register __a0 struct AZLayout *al, register __d0 UBYTE num )
{
    return( (ULONG) ((al[num].Obj->Flags & BOX_OVER) ? 0 : 1) );
}

ULONG __asm getgad_strnum( register __a0 struct AZLayout *al, register __d0 UBYTE num )
{
    return( (ULONG) Str2Num( UIK_OSt_Get( al[num].Obj, 0 ) ) );
}

void __asm setgad_active( register __a0 struct AZLayout *al, register __d0 UBYTE num, register __d1 ULONG bool )
{
    if (bool) al[num].Flags |= AZLF_BOX_ACTIVE; else al[num].Flags &= ~AZLF_BOX_ACTIVE;
}

void __asm setgad_disable( register __a0 struct AZLayout *al, register __d0 UBYTE num, register __d1 ULONG bool )
{
    if (bool) al[num].Flags |= AZLF_BOX_DISABLE; else al[num].Flags &= ~AZLF_BOX_DISABLE;
}

void __asm setgad_strnum( register __a0 struct AZLayout *al, register __d0 UBYTE num, register __d1 ULONG val )
{
  UBYTE buf[32];
    sprintf( buf, "%ld", val ); UIK_OSt_Set( al[num].Obj, buf, StrLen(buf) );
}

//---------------------------------------------------------------

static UBYTE *prep_rsr( struct AZRequest *ar, struct TextFont *font, APTR userdata )
{
  struct AZLayout **pal, *al, *al2;
  WORD totwidth;
  WORD /*top,*/ height;
  WORD listheight;
  UBYTE *mem;

    //------ Placement des frames
    pal = ar->Info;
    if (! (al = *pal))
        {
        if (! (*pal = (struct AZLayout *) LayoutPrepare( font, AZL_Search, userdata ))) return(0);

        al = &(*pal)[0];  // Aire
        al2 = &(*pal)[5]; // Extensions
        al->TotW = al2->TotW = al->Width = al2->Width = MAX(al->Width,al2->Width);

        al = &(*pal)[8];   // Occurences
        al2 = &(*pal)[12]; // Sensibilité
        al->TotW = al2->TotW = al->Width = al2->Width = MAX(al->Width,al2->Width);

        //al = &(*pal)[15]; // Restrictions

        LayoutWidth( font, (*pal) );
        al = (*pal);
        }

    al[0].Top = al[8].Top = al[15].Top = font->tf_YSize / 2;
    al[5].Top = al[12].Top = al[0].Top + al[0].Height + font->tf_YSize;

    //------ Création de place dans le ReqString
    totwidth = al[0].Width + INTERFRAME_WIDTH + al[8].Width + INTERFRAME_WIDTH + al[15].Width;
    height = al[0].TotH + al[5].TotH + font->tf_YSize / 2; // espace inter box

    if (! (mem = make_listptr( font, totwidth, height, &listheight ))) return(0);

    //------ Ajustement du Top des frames
    if ((height = (listheight - height) / 2) > 0)
        { al[0].Top += height; al[5].Top += height; al[8].Top += height; al[12].Top += height; al[15].Top += height; }

    return( mem );
}

void set_backw_radio( struct UIKObj *obj, struct AZLayout *al )
{
  struct AZRequest *ar = (struct AZRequest *)obj->UserULong2;
  struct AZLayout *albase = *((struct AZLayout **)ar->Info); // sauf ObjString

    if (ar == &AGP.AZReqReplace)
        UIK_Call( albase[4].Obj, UIKFUNC_Check_Unselect, 0, 0 );
}

void set_search_radio( struct UIKObj *obj, struct AZLayout *al )
{
  struct AZLayout *albase = *((struct AZLayout **)((struct AZRequest *)obj->UserULong2)->Info); // sauf ObjString

    UIK_Call( albase[9].Obj, UIKFUNC_Radio_Select, 0, 0 );
    set_backw_radio( obj, al );
}

static ULONG AddReqSearchReplace( struct AZRequest *ar ) // appelée par func_IntSmartRequest(), WO existe
{
  struct UIKObjReqString *rs = (struct UIKObjReqString *)ar->Obj;
  struct AZLayout *al = *((struct AZLayout **)ar->Info);
  ULONG *pf, flags;
  LONG diff;

    //------ Calcul des abscisses
    diff = (rs->Text->Obj.Box.Width - (al[0].TotW + al[8].TotW + al[15].TotW)) / 2;
    //al[0].Left = al[5].Left = 0;
    al[8].Left = al[12].Left = al[0].TotW + diff;
    al[15].Left = al[8].Left + al[8].TotW + diff;

    //------ Mise des flags des gadgets (si pas async, ne peut pas être la tâche mère)
    pf = (ar->Flags & ARF_TOPREFS) ? &AGP.Prefs->SearchFlags : &AGP.FObj->SeRep->SearchFlags;

    if ((func_BlockExists()) == 0)
        { setgad_disable( al, 2, 1 ); *pf &= ~SEF_BLOCK; }
    else{
        setgad_disable( al, 2, 0 );
        if ((*pf & (SEF_FULL|SEF_BLOCK)) == (SEF_FULL|SEF_BLOCK)) *pf &= ~SEF_BLOCK;
        if (*pf & SEF_BLOCK) *pf |= SEF_ALL;
        }
    if ((ar == &AGP.AZReqReplace) && (*pf & SEF_ALL))
        {
        *pf &= ~SEF_BACKWARD;
        }
    flags = *pf; // flags vérifiés

    setgad_active( al, 1, flags & SEF_FULL );
    setgad_active( al, 2, flags & SEF_BLOCK );
    setgad_active( al, 3, !(flags & (SEF_FULL|SEF_BLOCK)) );
    setgad_active( al, 4, flags & SEF_BACKWARD );

    setgad_active( al, 6, flags & SEF_PATTERN );
    setgad_active( al, 7, flags & SEF_CONVERT );

    setgad_active( al, 9, flags & SEF_ALL );
    setgad_active( al, 10, !(flags & SEF_ALL) );
    setgad_active( al, 11, flags & SEF_TOBLOCK );

    setgad_active( al, 13, flags & SEF_CASE );
    setgad_active( al, 14, flags & SEF_ACCENT );

    setgad_active( al, 16, flags & SEF_STARTOFLINE );
    setgad_active( al, 17, flags & SEF_ENDOFLINE );
    setgad_active( al, 18, flags & SEF_STARTOFWORD );
    setgad_active( al, 19, flags & SEF_ENDOFWORD );

    //------ Ajout des objets ------
    if (! LayoutAddObjects( rs, *((struct AZLayout **)ar->Info), &AGP, ar )) goto END_ERROR;

    return(1);

  END_ERROR:
    return(0);
}

//----------------------------------

static void ReqSearchPrefsEnd( struct AZRequest *ar )
{
  ULONG *pf = (ar->Flags & ARF_TOPREFS) ? &AGP.Prefs->SearchFlags : &AGP.FObj->SeRep->SearchFlags;

    if (ar->Button == REQBUT_OK || ar->Button == REQBUT_MIDDLE)
        {
        struct AZLayout *al = *((struct AZLayout **)ar->Info);

        getgad_check( al, 1, pf, SEF_FULL );
        getgad_check( al, 2, pf, SEF_BLOCK ); // pas le 3 car c'est une absence de 1 et 2
        getgad_check( al, 4, pf, SEF_BACKWARD );

        getgad_check( al, 6, pf, SEF_PATTERN );
        getgad_check( al, 7, pf, SEF_CONVERT );

        getgad_check( al, 9, pf, SEF_ALL ); // pas le 10 car c'est une absence de 9
        getgad_check( al, 11, pf, SEF_TOBLOCK );

        getgad_check( al, 13, pf, SEF_CASE );
        getgad_check( al, 14, pf, SEF_ACCENT );

        getgad_check( al, 16, pf, SEF_STARTOFLINE );
        getgad_check( al, 17, pf, SEF_ENDOFLINE );
        getgad_check( al, 18, pf, SEF_STARTOFWORD );
        getgad_check( al, 19, pf, SEF_ENDOFWORD );
        }
    if ((ar == &AGP.AZReqReplace) && (*pf & SEF_ALL))
        {
        *pf &= ~SEF_BACKWARD;
        }
    if (AGP.FObj) UPDATE_STAT(AGP.FObj);
}

extern ULONG DoRealSearch( APTR co, ULONG repl, ULONG convert1, ULONG convert2, ULONG clear0, ULONG docursor );

static void ReqSearchAsyncPrefsEnd( struct AZRequest *ar ) // appelée en cas de async
{
    ReqSearchPrefsEnd( ar );
}

static void ReqSearchAsyncNormEnd( struct AZRequest *ar ) // appelée en cas de async
{
  ULONG repl = (ar == &AGP.AZReqReplace) ? 1 : 0;

    ReqSearchPrefsEnd( ar );
    if (ar->Button == REQBUT_OK)
        {
        DoRealSearch( 0, repl, 1, 1, 1, 1 );
        }
    else if (ar->Button == REQBUT_MIDDLE)
        {
        DoRealSearch( 0, 0, 1, 1, 1, 1 );
        }
}

#define SEREP_PREFS   0
#define SEREP_SEARCH  1
#define SEREP_REPLACE 2

static ULONG do_reqsearch( struct AZRequest *ar, ULONG moreflags, ULONG type ) // num : 0=prefs 1=search 2=replace
{
  struct AZObjFile *fo = AGP.FObj;
  struct UIKObj *wo;
  struct TextFont *font;
  ULONG res=0, mast = (AGP.AZMast == (APTR)&AGP) ? 1 : 0;
  UBYTE *mem;

    if (! (wo = AGP.WO)) goto END_ERROR;
    font = wo->ObjExt->Font;
    if (! (mem = prep_rsr( ar, font, 0 ))) goto END_ERROR;

    switch (type)
        {
        case SEREP_PREFS:
            res = func_IntSmartRequest( 0, TEXT_PREFSSEARCH,0, 0,mem, 0,0, 0,0, -1,0,-1, AddReqSearchReplace, ARF_WINDOW|ARF_ACTIVATE|ARF_TOPREFS|moreflags /*|ARF_RETQUIET*/, (void*)ReqSearchAsyncPrefsEnd, ar );
            break;

        case SEREP_SEARCH:
            if (! mast)
                res = func_IntSmartRequest( 0, TEXT_CHERCHER,0, 0,mem, 0,&fo->SeRep->SearchStr, 0,0, TEXT_GADCHERCHER,0,TEXT_CANCEL, AddReqSearchReplace, ARF_WINDOW|ARF_ACTIVATE|moreflags /*|ARF_RETQUIET*/, ReqSearchAsyncNormEnd, ar );
            break;

        case SEREP_REPLACE:
            if (! mast)
                res = func_IntSmartRequest( 0, TEXT_REMPLACER,0, 0,mem, TEXT_CHANGER,&fo->SeRep->SearchStr, TEXT_PAR,&fo->SeRep->ReplaceStr, TEXT_GADREMPLACER,TEXT_GADSAUTER,TEXT_CANCEL, AddReqSearchReplace, ARF_WINDOW|ARF_ACTIVATE|moreflags /*|ARF_RETQUIET*/, ReqSearchAsyncNormEnd, ar );
            break;
        }

    UIK_MemSpFree( mem );

    if (!(moreflags & ARF_ASYNC)) ReqSearchPrefsEnd( ar );
    return( res );

  END_ERROR:
    ar->Button = REQBUT_CANCEL;
    ar->ErrMsg = TEXT_CMDCANCELLED;
    return( res );
}

static void req_search_err( struct AZRequest *ar )
{
    if (ar->WO && ar->WO->Obj.w)
        {
        WindowToFront( ar->WO->Obj.w );
        ActivateWindow( ar->WO->Obj.w );
        }
    else func_ReqShowText( UIK_LangString( AGP.UIK, TEXT_ERR_RequestInUse ) );
}

ULONG func_ReqSearch()
{
    if (AGP.AZReqSearch.Obj)
        {
        req_search_err( &AGP.AZReqSearch );
        return( REQBUT_CANCEL );
        }
    AGP.AZReqSearch.Info = &AGP.SearchLayout;
    AGP.AZReqSearch.Flags = 0;
    return( do_reqsearch( &AGP.AZReqSearch, 0, SEREP_SEARCH ) );
}

ULONG func_ReqSearchReplace()
{
    if (AGP.AZReqReplace.Obj)
        {
        req_search_err( &AGP.AZReqReplace );
        return( REQBUT_CANCEL );
        }
    AGP.AZReqReplace.Info = &AGP.ReplaceLayout;
    AGP.AZReqReplace.Flags = 0;
    return( do_reqsearch( &AGP.AZReqReplace, 0, SEREP_REPLACE ) );
}

//---------------------------------------------------------------

ULONG func_ReqNumber( ULONG *presult )
{
  ULONG code;

    BufTruncate( AGP.UBufTmp, 0 );
    code = func_IntSmartRequest( 0, TEXT_NUMBER,0, TEXT_NUMBERINFO,0, 0,&AGP.UBufTmp, 0,0, TEXT_GADOK,0,TEXT_CANCEL, 0, ARF_ACTIVATE/*|ARF_RETQUIET*/, 0, 0 );
    if (presult)
        {
        if (code == REQBUT_OK) ARG_StrToLong( AGP.UBufTmp, presult );
        else *presult = 0;
        }
    return( code );
}

//---------------------------------------------------------------

ULONG func_ReqString( UBYTE *title, UBYTE *text, UBYTE **presult )
{
  ULONG code;

    BufTruncate( AGP.UBufTmp, 0 );
    code = func_SmartRequest( title, text, 0,&AGP.UBufTmp, 0,0, (APTR)-1,0,(APTR)-1, 0, ARF_ACTIVATE/*|ARF_RETQUIET*/ );
    if (presult)
        {
        *presult = (code == REQBUT_OK) ? AGP.UBufTmp : 0;
        }
    return( code );
}

//---------------------------------------------------------------

static void calc_line_col( ULONG relative )
{
  struct AZObjFile *fo = AGP.FObj;
  LONG i, arg[15]; // LINE COL PAGE REL WORD TAB OFFSET UP LEFT

    for (i=0; i < 15; i++) arg[i] = 0;
    arg[0] = (ARG_StrToLong( AGP.FObj->LineString, &fo->WorkReqLine ) <= 0) ? 0 : (LONG)&fo->WorkReqLine;
    //arg[1] = (ARG_StrToLong( AGP.FObj->ColString, &fo->WorkReqCol ) <= 0) ? 0 : (LONG)&fo->WorkReqCol;
    arg[3] = relative;
    func_ComputeLineColFlags( &arg[0], &fo->WorkReqLine, &fo->WorkReqCol, CLCF_FOLDED );
}

ULONG func_ReqLineColumn()
{
  ULONG code=0;

    BufTruncate( AGP.FObj->LineString, 0 );
    switch (code = func_IntSmartRequest( 0, TEXT_MsgLineCol,0, 0,0, TEXT_Line,&AGP.FObj->LineString, 0,0/*TEXT_Column,&AGP.FObj->ColString*/, TEXT_Absolute,TEXT_Relative,TEXT_CANCEL, 0, ARF_ACTIVATE /*|ARF_RETQUIET*/, 0, 0 ))
        {
        case REQBUT_OK:
            calc_line_col(0);
            break;

        case REQBUT_MIDDLE:
            calc_line_col(1);
            break;

        case REQBUT_CANCEL:
        default:
            break;
        }

  END_ERROR:
    return( code );
}

//---------------------------------------------------------------

ULONG func_ReqOffset()
{
  struct AZObjFile *fo = AGP.FObj;
  ULONG code=0, result;
  LONG i, arg[15];

    switch (code = func_IntSmartRequest( 0, TEXT_OffsetNumber,0, 0,0, 0,&AGP.FObj->ReqString, 0,0, TEXT_GADOK,0,TEXT_CANCEL, 0, ARF_ACTIVATE/*|ARF_RETQUIET*/, 0, 0 ))
        {
        case REQBUT_OK:
            //if (presult) ARG_StrToLong( AGP.FObj->ReqString, presult );
            for (i=0; i < 15; i++) arg[i] = 0;
            arg[6] = (ARG_StrToLong( AGP.FObj->ReqString, &result ) <= 0) ? 0 : (LONG)&result;
            func_ComputeLineColFlags( &arg[0], &fo->WorkReqLine, &fo->WorkReqCol, CLCF_FOLDED );
            break;

        case REQBUT_CANCEL:
        default:
            break;
        }

  END_ERROR:
    return( code );
}

//=================================================================

static void remove_async_rs( int nil, struct UIKObjReqString *rs )
{
    func_ReqClose( 0, (APTR)rs->Obj.UserULong2 );
}

//-----------------------------------------

ULONG func_ReqOpenSearch( struct AZRequest *ar )
{
    if (AGP.AZMast == (APTR)&AGP) { ar->Button = REQBUT_CANCEL; return( TEXT_CMDCANCELLED ); }
    ar->Info = &AGP.SearchLayout;
    do_reqsearch( ar, ARF_ASYNC|ARF_NOTERM|ARF_MEMPOS, SEREP_SEARCH );
    return( ar->ErrMsg );
}

//-----------------------------------------

ULONG func_ReqOpenSearchReplace( struct AZRequest *ar )
{
    if (AGP.AZMast == (APTR)&AGP) { ar->Button = REQBUT_CANCEL; return( TEXT_CMDCANCELLED ); }
    ar->Info = &AGP.ReplaceLayout;
    do_reqsearch( ar, ARF_ASYNC|ARF_NOTERM|ARF_MEMPOS, SEREP_REPLACE );
    return( ar->ErrMsg );
}

//-----------------------------------------

ULONG func_ReqOpenSearchPrefs( struct AZRequest *ar )
{
    ar->Info = &AGP.SearchPrefsLayout;
    do_reqsearch( ar, ARF_ASYNC, SEREP_PREFS );
    return( ar->ErrMsg );
}

//-----------------------------------------

static void display_okfunc( struct UIKObjDisplayMode *dm, struct ModeInfo *mi, ULONG UserData)
{
  struct AZurPrefs *pr = AGP.Prefs;

    pr->ScreenDepth  = mi->Depth;
    pr->ScreenModeId = mi->vpm.ModeId;
    pr->ScreenHeight = mi->Height;
    pr->ScreenWidth  = mi->Width;
    pr->ScreenOverscan  = mi->OScanType;
    func_ReqClose( 0, (APTR)dm->Obj.UserULong2 );
    ActualizePrefs(1);
}

static void display_cancelfunc( struct UIKObjDisplayMode *dm )
{
    func_ReqClose( 0, (APTR)dm->Obj.UserULong2 );
}

static void display_windowclosed( struct UIKObj *wo )
{
    func_ReqClose( 0, (APTR)wo->UserULong2 );
}

ULONG func_ReqOpenDisplayPrefs( struct AZRequest *ar )
{
  struct AZurPrefs *pr = AGP.Prefs;
  struct UIKObj *wo=0, *dm=0;
  struct ModeInfo mi;

    if (! AGP.WO) goto END_ERROR;

    mi.Depth         = pr->ScreenDepth;
    mi.vpm.ModeId    = pr->ScreenModeId;
    mi.Width         = pr->ScreenWidth;
    mi.Height        = pr->ScreenHeight;
    mi.OScanType     = pr->ScreenOverscan;
    mi.MI_ColorShades = 0;

    if (! (wo = (struct UIKObjWindow *) UIK_AddObjectTagsB( UIKBase, "UIKObj_Window", AGP.WO,
            UIKTAG_OBJ_LeftTop,       SETL(10,30),
            UIKTAG_OBJ_WidthHeight,   SETL(574,165),
            UIKTAG_OBJ_Title,         TEXT_Title_DisplayMode,
            UIKTAG_OBJ_AltTitle,      TEXT_AZURNAME,
            UIKTAG_OBJWindow_MinimumWidthHeight, SETL(537,165),
            UIKTAG_OBJWindowFl_AutoShrink, TRUE,
            UIKTAG_OBJ_UserValue2,    ar,
            TAG_END ))) goto END_ERROR;
    if (! UIK_AddVector( wo, (void *) display_windowclosed, CLOSEWINDOW, 0 )) goto END_ERROR;

    if (! (dm = UIK_AddObjectTagsB( UIKBase, "UIKObj_DisplayMode", wo,
            UIKTAG_OBJ_LeftTop,           SETL(10,5),
            UIKTAG_OBJ_BoxFl_TranslateLeftTop, TRUE,
            UIKTAG_OBJ_UserValue1,      wo,
            UIKTAG_OBJ_UserValue2,      ar,
            UIKTAG_ObjDM_OKFunc,        display_okfunc,
            UIKTAG_ObjDM_CancelFunc,    display_cancelfunc,
            UIKTAG_ObjDM_ModeInfo,      &mi,
            TAG_END ))) goto END_ERROR;

    if (! UIK_Start( wo )) goto END_ERROR;
    ar->Obj = wo;
    return(0);

  END_ERROR:
    UIK_Remove( wo );
    ar->Button = REQBUT_CANCEL;
    return( TEXT_CMDCANCELLED );
}

//-----------------------------------------

/* ULONG func_ReqOpenPrintPrefs( struct AZRequest *ar )
{
  struct UIKObjReqString *rs=0;
  struct UIKObj *wo=AGP.WO;

    if (! wo) goto END_ERROR;
    if (! UIK_UniconifyWindow( (struct UIKObjWindow *)wo )) goto END_ERROR;

    if (! (rs = (struct UIKObjReqString *)UIK_AddObjectTagsB( UIKBase, "UIKObj_ReqString", wo,
                UIKTAG_OBJ_Title,           TEXT_PREFSPRINT,
                UIKTAG_ReqString_TextPtr,   "\n    Prévu pour AZur_Beta6    \n\n",
                UIKTAG_ReqStringFl_Window,  TST_REQWIN,
                UIKTAG_ReqString_OKFunc,    remove_async_rs,
                UIKTAG_ReqString_CancelFunc,remove_async_rs,
                UIKTAG_OBJ_UserValue2,      ar,
                TAG_END ))) goto END_ERROR;
    func_RemoveNullPointer();
    if (! UIK_Start( rs )) goto END_ERROR;
    if (AGP.FObj)
        {
        UIK_InactivateKeyTank( AGP.FObj );
        UIK_InactivateKeyTank( AGP.FObj->CmdLine );
        }
    ar->Obj = rs;
    return(0);

  END_ERROR:
    UIK_Remove( rs );
    ar->Button = REQBUT_CANCEL;
    return( TEXT_CMDCANCELLED );
}
*/

//-----------------------------------------

/*
static void Font_Cancel( struct UIKObjWinFontSel *wfos )
{
    func_ReqClose( 0, (APTR)wfos->Obj.UserULong2 );
}

static void Font_OK( struct UIKObjWinFontSel *wfos, struct TextFont *font, UWORD Style )
{
  struct Node *node;
  struct AZObjView *view, *act=0, *corr=0;
  struct AZObjFile *fo=AGP.FObj;
  WORD left, top, width, height, fontdiff_status=0, fontdiff_cmdl=0, fontdiff;
  struct TagItem tl[] = { { UIKTAG_OBJ_LeftTop,     },
                          { UIKTAG_OBJ_WidthHeight, },
                          { TAG_END } };

    if (fo->Stat) fontdiff_status = font->tf_YSize - AGP.Prefs->ViewFontHeight;
    if (fo->CmdLine) fontdiff_cmdl = font->tf_YSize - AGP.Prefs->ViewFontHeight;
    fontdiff = fontdiff_status + fontdiff_cmdl;

    if (fo)
        {
        ofi_ComputeViewBox( &fo->VI, fo->WO, font->tf_YSize );
        ofi_VerifViewSize( &fo->VI, font->tf_YSize, (fo->WO->LeftBorderWidth + fo->WO->RightBorderWidth) );
        for (node=fo->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
            {
            view = (struct AZObjView *)node->ln_Name;
            if (view->Obj.Box.Height < fo->VI.MinHeight)
                {
                Font_Cancel( wfos );
                func_ReqShowText( UIK_LangString( AGP.UIK, TEXT_FONTTOOHIGH ) );
                return;
                }
            if (view->Obj.Box.Height - fontdiff >= fo->VI.MinHeight) corr = view;
            }
        if (! corr)
            {
            Font_Cancel( wfos );
            func_ReqShowText( UIK_LangString( AGP.UIK, TEXT_FONTTOOHIGH ) );
            return;
            }
        }

    StcCpy( AGP.Prefs->ViewFontName, font->tf_Message.mn_Node.ln_Name, 32 );
    AGP.Prefs->ViewFontHeight = font->tf_YSize;
    func_ReqClose( 0, (APTR)wfos->Obj.UserULong2 );

    if (fo)
        {
        func_CursorErase();

        if (fo->Stat /*&& fontdiff_status*/)
            {
            struct UIKObj *stat;
            left = fo->Stat->Box.Left - fo->WO->LeftBorderWidth;
            top = fo->Stat->Box.Top - fo->WO->TopBorderHeight;
            if (stat = (struct UIKObj *) ofi_AddStat( W2L(left,top),
                                                      W2L(fo->Stat->Box.Width,fo->Stat->Box.Height+fontdiff_status),
                                                      font->tf_Message.mn_Node.ln_Name,
                                                      font->tf_YSize ))
                {
                UIK_Remove( fo->Stat );
                fo->Stat = stat;
                UIK_Start( fo->Stat );
                }
            }

        for (node=fo->ViewList.lh_Head; node->ln_Succ; node = node->ln_Succ)
            {
            view = (struct AZObjView *)node->ln_Name;
            left = view->Obj.Box.Left - fo->WO->LeftBorderWidth;
            top = view->Obj.Box.Top - fo->WO->TopBorderHeight + fontdiff_status;
            width = view->Obj.Box.Width;
            height = view->Obj.Box.Height;
            if (view == corr) height -= fontdiff;
            tl[0].ti_Data = W2L(left,top);
            tl[1].ti_Data = W2L(width,height);

            if (! (view = (struct AZObjView *) ofi_AddViewObj( fo, tl ))) break;
            view->Zone->TopLine = ((struct AZObjView *)node->ln_Name)->Zone->TopLine;
            if (! UIK_Start( view )) { UIK_Remove( view ); break; }

            if ((APTR)fo->ActiveView == (APTR)node->ln_Name) act = view;
            UIK_Remove( (struct UIKObj *)node->ln_Name );
            node->ln_Name = (APTR) view;
            }
        ofi_LinkViews( fo );
        ofi_SetActiveView( fo, act ? act : (APTR) fo->ViewList.lh_Head->ln_Name );

        if (fo->CmdLine /*&& fontdiff_cmdl*/)
            {
            struct UIKObj *cmdl;
            left = fo->CmdLine->Box.Left - fo->WO->LeftBorderWidth;
            top = fo->CmdLine->Box.Top - fo->WO->TopBorderHeight - fontdiff_cmdl;
            if (cmdl = (struct UIKObj *) ofi_AddCmdLine( W2L(left,top),
                                                         W2L(fo->CmdLine->Box.Width,fo->CmdLine->Box.Height+fontdiff_cmdl),
                                                         font->tf_Message.mn_Node.ln_Name,
                                                         font->tf_YSize ))
                {
                UIK_Remove( fo->CmdLine );
                fo->CmdLine = cmdl;
                UIK_Start( fo->CmdLine );
                }
            }

        ofi_ActualizeViewLimits( fo );
        func_CursorDraw();
        }
}

ULONG func_ReqOpenFontPrefs( struct AZRequest *ar )
{
  struct TTextAttr tta;
  struct UIKObj *fo;

    tta.tta_Name  = AGP.Prefs->ViewFontName;
    tta.tta_YSize = AGP.Prefs->ViewFontHeight;
    tta.tta_Style = 0;
    tta.tta_Flags = 0;
    tta.tta_Tags  = 0;

    if (! (fo = UIK_AddObjectTagsB( UIKBase, "UIKObj_WinFontSel", AGP.WO,
            UIKTAG_OBJ_Title,           TEXT_SELECTFONT,
            UIKTAG_WFOS_OKFUNC,         Font_OK,
            UIKTAG_WFOS_CANCELFUNC,     Font_Cancel,
            UIKTAG_WFOSFl_FixedWidth,   TRUE,
            UIKTAG_WFOSFl_PropWidth,    FALSE,
            UIKTAG_WFOS_HighlightFont,  &tta,
            UIKTAG_OBJ_UserValue2,      ar,
            TAG_END ))) goto END_ERROR;
    func_RemoveNullPointer();
    if (! UIK_Start( fo )) goto END_ERROR;
    ar->Obj = fo;
    return(0);

  END_ERROR:
    UIK_Remove( fo );
    ar->Button = REQBUT_CANCEL;
    return( TEXT_CMDCANCELLED );
}
*/

//-----------------------------------------

struct text_info {
    ULONG bytes;
    ULONG bytesinwords;
    ULONG words;
    ULONG totallines;
    ULONG maxlinewidth;
    ULONG blanklines;
    ULONG numlines_done;
    ULONG curline;
    };

static ULONG count_line_words( struct text_info *ti, UBYTE *ptr, ULONG len )
{
  register ULONG status_word, numwords;

    for (status_word=0, numwords=0; len; len--, ptr++)
        {
        if (func_CharIsWord( *ptr ))
            {
            if (status_word == 0) { numwords++; status_word = 1; }
            ti->bytesinwords++;
            }
        else{ if (status_word == 1) { status_word = 0; } }
        }
    return( numwords );
}

static ULONG fold_callback( struct text_info *ti, LONG realline, struct UIKPList *ul, LONG ind )
{
  register ULONG len;
  register UBYTE *ptr;

    if (ti->numlines_done) ti->bytes++;     // \n de fin de ligne

    ptr = ul->UNode[ind].Buf;
    len = intAZ_LineBufLen( ptr );
    ti->totallines++;
    if (len)
        {
        ti->bytes += len;
        ti->words += count_line_words( ti, ptr, len );
        if (len > ti->maxlinewidth) ti->maxlinewidth = len;
        }
    else ti->blanklines++;

    if (ul == &AGP.FObj->Text && ind == AGP.FObj->Line) ti->curline = realline;

    ti->numlines_done++;
    return(0);
}

static void verif_maxsize( struct AZRequest *ar )
{
#ifdef AZUR_DEMO
    if (*((ULONG*)&ar->Width) > AZURDEMO_MAXSIZE+2) SET_READONLY;
#endif
}

ULONG func_ReqOpenInfo( struct AZRequest *ar )
{
  struct AZObjFile *fo = AGP.FObj;
  ULONG averlinewidth=0, nbaverlinewidth=0, averchword=0, numnl=0;
  struct text_info TI;
  UBYTE *mastername;

    if (!AGP.WO || !AGP.WO->Obj.w || (AGP.AZMast == (APTR)&AGP)) { ar->Button = REQBUT_CANCEL; return( TEXT_CMDCANCELLED ); }

    __builtin_memset( &TI, 0, sizeof(struct text_info) );
    BufTruncate( AGP.UBufTmp, 0 );
    func_FoldApplyLines( (void*)fold_callback, &TI, &AGP.UBufTmp );

    if (TI.totallines) numnl = TI.totallines - 1;
    averlinewidth = (TI.bytes - numnl) / TI.totallines;
    if (TI.totallines - TI.blanklines) nbaverlinewidth = (TI.bytes - numnl) / (TI.totallines - TI.blanklines);
    if (TI.words) averchword = TI.bytesinwords / TI.words;

    mastername =
        (UBYTE*) IPUIK_CallObjectFunc( AGP.AZMast->Process, AGP.AZMast->ARexx, UIKFUNC_ARexxSim_ARexxName, 0, 0 );
    if (! mastername) mastername = "";

    /* Caractères : %ld,   Mots : %ld,
     * Caractères dans les mots : %ld,   par mot : %ld
     * Total lignes : %ld,   Lignes non vides : %ld
     * Ligne courante réelle : %ld
     * Lignes visibles : %ld,   Lignes repliées : %ld
     * Plis : %ld,   Blocs : %ld,    Marques : %ld
     * Largeur max : %ld,   Largeur moyenne : %ld,
     * Largeur moyenne des lignes non vides : %ld
     * Nom de l'écran public : %ls
     * Nom du port ARexx : %ls
     * Nom du port ARexx maître : %ls
     */
    func_BufPrintf( &AGP.UBufTmp, 0, UIK_LangString( AGP.UIK, TEXT_InfoRequest ),
        TI.bytes, TI.words,
        TI.bytesinwords, averchword,
        TI.curline+1,
        TI.totallines, TI.totallines - TI.blanklines,
        fo->Text.NodeNum, TI.totallines - fo->Text.NodeNum,
        BufLength(AGP.UBufTmp)/sizeof(struct memfold), func_BlockExists(), func_BMExists(),
        TI.maxlinewidth, averlinewidth, nbaverlinewidth,
        func_GetPubScreenName(),
        UIK_Call( AGP.ARexx, UIKFUNC_ARexxSim_ARexxName, 0, 0 ),
        mastername
        );

#ifdef AZUR_DEMO
    *((ULONG*)&ar->Left) = -1;
    *((ULONG*)&ar->Width) = TI.bytes; // pour verif_maxsize()
#endif
    smart_request( 0, TEXT_InfoTitle, 0, 0, AGP.UBufTmp, 0,0, 0,0, (APTR)-1,0,0, 0, ARF_WINDOW|ARF_ASYNC, verif_maxsize, ar );
}

//-----------------------------------------

/* voir aussi dans main.c */

#ifdef AZUR_DEMO
//------------- Demo
UBYTE __far CopyrigthCode[] = "\x38\x3B\x01\x31\x37\x2B\x13\x14\x23\x61\x45\x3A\x0E\x04\x3A\x1A\x06\x11\x11\x38\x6D\xCC\x59\x50\x4D\x7A\x5C\x4F\x3C\x1C\x2D\x23\x48\x34\x08\x17\x2B\x0D\x03\x56\x3F\x23\x3F\x02\x1C\x00\x07\x49\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x2E\x1A\x15\x6C\x1F\x0C\x1E\x09\x00\x30\x48\x3D\x13\x0A\x29\x3F\x13\x1C\x05\x7E\x49\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x49\x2C\x0A\x1B\x16\x22\x3E\x11\x0B\x00\x00\x2A\x07\x01\x56\x09\x3E\x22\x02\x0B\x00\x19\x6F\x48\x1C\x17\x15\x29\x6D\x03\x16\x13\x16\x2A\x0C\x0B\x13\x17\x4C";
ULONG __far CopyrigthCodeLen = 152;
UBYTE __far CopyrigthCode2[] = "\x38\x3B\x01\x31\x37\x2B\x13\x14\x23\x61\x45\x3A\x0E\x04\x3A\x1A\x06\x11\x11\x38\x6D\xCC\x59\x50\x4D\x7A\x5C\x4F\x3C\x1C\x2D\x23\x48\x34\x08\x17\x2B\x0D\x03\x56\x3F\x23\x3F\x02\x1C\x00\x07\x49\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x3C\x00\x03\x0A\x6C\x09\x17\x16\x08\x00\x30\x48\x3D\x9F\x0A\x29\x3F\x13\x90\x12\x7E\x49\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x49\x38\x1D\x19\x1E\x3E\x2C\x08\x14\x04\x54\x27\x0D\x4F\x12\x90\x21\x22\x0B\x0A\x15\x06\x22\x1C\x06\x19\x17\x60\x6D\x13\x1C\x0F\x00\x26\x48\x06\x18\x0D\x29\x3F\x01\x10\x15\x11\x43";
ULONG __far CopyrigthCode2Len = 158;
UBYTE __far VersionCode[] = "\x5D\x37\x31\x11\x52\x4F\x37\x23\x39\x3F\x3A\x3D\x04\x19\x2C\x48\x5E\x58\x49\x7F\x6D\x4D\x4F\x4F\x4D\x6D\x51\x5A\x5F\x79";
ULONG __far VersionCodeLen = 30;
#else
//------------- Commercial
UBYTE __far CopyrigthCode[] = "\x38\x3B\x01\x31\x44\x4F\x35\x16\x3C\x34\x17\x10\x06\x1C\x37\x48\xC6\x56\x48\x75\x74\x51\x59\x2B\x11\x22\x06\x42\x3B\x10\x2F\x25\x00\x15\x41\x32\x2C\x1A\x08\x13\x18\x3F\x47\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x38\x0D\x18\x63\x3A\x06\x11\x11\x38\x3E\x45\x2B\x04\x07\x26\x1A\x19\x13\x1D\x46\x47\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x47\x26\x16\x0C\x19\x26\x1A\x0C\x1F\x18\x20\x6D\x15\x0B\x0E\x13\x31\x09\x02\x5A\x59\x28\x24\x16\x0D\x13\x1D\x21\x1D\x1B\x1F\x16\x22\x6D\x03\x16\x13\x16\x2A\x0C\x0B\x13\x17\x4C";
ULONG __far CopyrigthCodeLen = 152;
UBYTE __far CopyrigthCode2[] = "\x38\x3B\x01\x31\x44\x4F\x35\x16\x3C\x34\x17\x10\x06\x1C\x37\x48\xC6\x56\x48\x75\x74\x51\x59\x2B\x11\x22\x06\x42\x3B\x10\x2F\x25\x00\x15\x41\x32\x2C\x1A\x08\x13\x18\x3F\x6D\x45\x73\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x35\x1B\x36\x1B\x4F\x32\x0B\x23\x24\x11\x0A\x41\x26\xAA\x1B\x0A\x04\x0F\xA5\x3E\x6F\x73\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x59\x41\x54\x63\x48\x4F\x56\x59\x6C\x6D\x45\x73\x31\x06\x2C\x0F\x1D\x17\x14\x21\x28\x45\x1A\x0E\x19\x2E\x0D\x1D\x15\x10\x2D\x21\x49\x59\x05\x1D\x30\x1C\x1D\x1F\x1B\x39\x39\x0C\x16\x0F\x54\x2A\x06\x1B\x13\x0B\x28\x24\x11\x1C\x61";
ULONG __far CopyrigthCode2Len = 156;
UBYTE __far VersionCode[] = "\x5D\x37\x31\x11\x52\x4F\x37\x23\x39\x3F\x45\x48\x4F\x44\x70\x48\x47\x40\x57\x75\x63\x5C\x4C\x48\x74";
ULONG __far VersionCodeLen = 25;
#endif

ULONG func_ReqOpenAbout( struct AZRequest *ar )
{
  struct UIKObjReqString *rs=0;
  struct UIKObj *list, *obj, *wo=AGP.WO;
  UBYTE *cpr, *mem=0;
  ULONG len, lines;
  UWORD i, o, top, left, width, fh, ew, cprw, l2left, l2top;
  struct UIKBitMap *ubm;
  struct TextFont *font;

    if (! wo) goto END_ERROR;
    if (! UIK_UniconifyWindow( (struct UIKObjWindow *)wo )) goto END_ERROR;

    UIK_InvertBlackWhite( AGP.UIK, UIKBitMapTable, 1 );
    ubm = UIKBitMapTable[wo->AutoRatio - 1];
    font = wo->ObjExt->Font;
    fh = font->tf_YSize;
    ew = UIK_CharWidth( font, ' ' );

    //----------- prep
    CopyMem( CopyrigthCode, LangEnglish[1], CopyrigthCodeLen );
    func_DecodeText( "yatChovyLMe", LangEnglish[1], CopyrigthCodeLen );
    CopyMem( CopyrigthCode2, LangOther[1], CopyrigthCode2Len );
    func_DecodeText( "yatChovyLMe", LangOther[1], CopyrigthCode2Len );
    CopyMem( VersionCode, &VersionTag[1], VersionCodeLen );
    func_DecodeText( "yatChovyLMe", &VersionTag[1], VersionCodeLen );

    lines = (ubm->Rows + fh - 1) / fh;
    l2top = 0;
    if (lines < 5) lines = 5; // pour la deuxième liste
    else l2top = (ubm->Rows - 5*fh) / 2;
    l2left = ubm->Width + UIK_CharWidth( font, 'M' );

    cpr = UIK_LangString( AGP.UIK, TEXT_COPYRIGHT );
    for (i=o=cprw=0; cpr[i]; i++)
        {
        if (cpr[i] == '\n')
            {
            if (len = i - o) { cprw = MAX(cprw,UIK_StrWidth( font, &cpr[o], len )); }
            o = i + 1;
            }
        }

    for (i=len=0; len<3; i++) if (cpr[i] == '\n') len++;
    CopyMem( VersionText, &cpr[i], StrLen(VersionText) );

    //----------- string
    width = l2left + cprw;
    len = lines - 1 + width / ew - 1;
    if (! (mem = UIK_MemSpAlloc( len+1, MEMF_ANY ))) goto END_ERROR;
    for (i=0; i<lines-1; i++) mem[i] = '\n';
    for (; i<len; i++) mem[i] = ' ';
    mem[len] = 0;

    //----------- objets
    if (! (rs = (struct UIKObjReqString *)UIK_AddObjectTagsB( UIKBase, "UIKObj_ReqString", wo,
                UIKTAG_OBJ_Title,           TEXT_INFORMATION,
                UIKTAG_ReqString_TextPtr,   mem,
                UIKTAG_ReqStringFl_Window,  TRUE,
                UIKTAG_ReqString_OKFunc,    remove_async_rs,
                //UIKTAG_ReqString_CancelFunc, remove_async_rs,
                UIKTAG_OBJ_UserValue2,      ar,
                TAG_END ))) goto END_ERROR;

    if (! (list = UIK_GetObject( rs->Req, NULL, UIKID_LIST ))) goto END_ERROR;
    left = list->Box.Left;
    top = list->Box.Top;
    if (rs->Req->Id == UIKID_WINDOW) top -= ((struct UIKObjWindow *)rs->Req)->TopBorderHeight;

    if (! (obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_Box", rs->Req,
                UIKTAG_OBJ_LeftTop,         W2L(left,top),
                UIKTAG_OBJ_WidthHeight,     W2L(ubm->Width,ubm->Rows),
                UIKTAG_OBJ_BoxAutomate,     NullAuto,
                UIKTAG_OBJ_Title,           ubm,
                UIKTAG_OBJ_TitleFl_Image,   TRUE,
                TAG_END ))) goto END_ERROR;

    if (! (obj = UIK_AddObjectTagsB( UIKBase, "UIKObj_List", rs->Req,
                UIKTAG_OBJ_LeftTop,         W2L(left+l2left,top+l2top),
                UIKTAG_OBJ_WidthHeight,     W2L(cprw,5*fh),
                UIKTAG_OBJ_BoxAltAutomate,  NullAuto,
                UIKTAG_OBJList_TextPtr,     cpr,
                TAG_END ))) goto END_ERROR;

    func_RemoveNullPointer();
    if (! UIK_Start( rs )) goto END_ERROR;
    if (AGP.FObj)
        {
        UIK_InactivateKeyTank( AGP.FObj );
        UIK_InactivateKeyTank( AGP.FObj->CmdLine );
        }

    UIK_MemSpFree( mem );
    ar->Obj = rs;
    return(0);

  END_ERROR:
    UIK_Remove( rs );
    UIK_MemSpFree( mem );
    ar->Button = REQBUT_CANCEL;
    return( TEXT_CMDCANCELLED );
}

//---------------------------------------------------------------

static void InsertHexString( struct AZRequest *ar )
{
  UBYTE buf, hex=0, rot, *p = AGP.FObj->HexString;
  ULONG errmsg=0;

    if (!TST_READONLY && (ar->Button != REQBUT_CANCEL))
        {
        rot = (StrLen(p) & 1 ? 0 : 4);  /* si impaire digit a gauche */
        while (*p)
            {
            if (*p >= 'a' && *p <= 'f') *p -= 0x20;
            if ((*p < '0' || *p > '9') && (*p < 'A' || *p > 'F'))
                { errmsg = TEXT_ERR_BadHexDigit; goto END_ERROR; }
            ++p;
            }
        p = AGP.FObj->HexString;
        while (*p)
            {
            buf = (*p - ((*p >= 'A' && *p <= 'F') ? 0x40-9 : 0x30)) << rot;
            if (rot)     /* digit a gauche */
                {
                hex = buf;
                rot = 0;
                }
            else{       /* digit a droite */
                hex |= buf;
                func_TextPut( &hex, 1, 0, 0 );
                rot = 4;
                }
            ++p;
            }
        if (AGP.FObj->HexString[0])
            {
            func_TextPut( 0, 0, 0, 1 );
            UPDATE_STAT_PROP(AGP.FObj);
            func_CursorPos( AGP.FObj->WorkLine, AGP.FObj->WorkCol );
            }
        }

  END_ERROR:
    if (errmsg) func_ReqShowText( UIK_LangString( AGP.UIK, errmsg ) );
}

ULONG func_ReqOpenHex( struct AZRequest *ar )
{
    if (AGP.AZMast == (APTR)&AGP) { ar->Button = REQBUT_CANCEL; return( TEXT_CMDCANCELLED ); }
    //BufTruncate( AGP.FObj->HexString, 0 );
    func_IntSmartRequest( 0, TEXT_HEXSTRING,0, TEXT_HEXINFO,0, 0,&AGP.FObj->HexString, 0,0, TEXT_GADOK,0,TEXT_CANCEL, 0, ARF_WINDOW|ARF_ACTIVATE|ARF_ASYNC /*|ARF_NOTERM|ARF_RETQUIET*/, InsertHexString, ar );
    return( ar->ErrMsg );
}

//-----------------------------------------

static void do_line_col( struct AZRequest *ar )
{
  LONG i, arg[15], line, col; // LINE COL PAGE REL WORD TAB OFFSET UP LEFT

    if (ar->Button != REQBUT_CANCEL)
        {
        for (i=0; i < 15; i++) arg[i] = 0;
        arg[0] = (ARG_StrToLong( AGP.FObj->LineString, &line ) <= 0) ? 0 : (LONG)&line;
        arg[1] = (ARG_StrToLong( AGP.FObj->ColString, &col ) <= 0) ? 0 : (LONG)&col;
        arg[3] = (ar->Button == REQBUT_MIDDLE ? 1 : 0);
        func_ComputeLineColFlags( &arg[0], &line, &col, CLCF_FOLDED );
        func_CursorPos( line, col );
        ActivateWindow( AGP.WO->Obj.w );
        }
}

ULONG func_ReqOpenLineColumn( struct AZRequest *ar )
{
    if (AGP.AZMast == (APTR)&AGP) { ar->Button = REQBUT_CANCEL; return( TEXT_CMDCANCELLED ); }
    BufTruncate( AGP.FObj->LineString, 0 );
    BufTruncate( AGP.FObj->ColString, 0 );
    func_IntSmartRequest( 0, TEXT_MsgLineCol,0, 0,0, TEXT_Line,&AGP.FObj->LineString, TEXT_Column,&AGP.FObj->ColString, TEXT_Absolute,TEXT_Relative,TEXT_CANCEL, 0, ARF_WINDOW|ARF_ACTIVATE|ARF_ASYNC|ARF_NOTERM|ARF_MEMPOS /*|ARF_RETQUIET*/, do_line_col, ar );
    return( ar->ErrMsg );
}

//-----------------------------------------

static void do_req_offset( struct AZRequest *ar ) // appelée en cas de async
{
  LONG i, arg[15], line, col;
  ULONG result;

   if (ar->Button == REQBUT_OK)
        {
        for (i=0; i < 15; i++) arg[i] = 0;
        arg[6] = (ARG_StrToLong( AGP.FObj->ReqString, &result ) <= 0) ? 0 : (LONG)&result;
        func_ComputeLineColFlags( &arg[0], &line, &col, CLCF_FOLDED );
        func_CursorPos( line, col );
        ActivateWindow( AGP.WO->Obj.w );
        }
}

ULONG func_ReqOpenOffset( struct AZRequest *ar )
{
    if (AGP.AZMast == (APTR)&AGP) { ar->Button = REQBUT_CANCEL; return( TEXT_CMDCANCELLED ); }
    func_IntSmartRequest( 0, TEXT_OffsetNumber,0, 0,0, 0,&AGP.FObj->ReqString, 0,0, TEXT_GADOK,0,TEXT_CANCEL, 0, ARF_WINDOW|ARF_ACTIVATE|ARF_ASYNC|ARF_NOTERM|ARF_MEMPOS /*|ARF_RETQUIET*/, do_req_offset, ar );
    return( ar->ErrMsg );
}

//-----------------------------------------

extern ULONG BM_cancel();
extern ULONG BM_apply_2_buftmp();
extern ULONG BM_sel_2_buftmp();

static void BM_ok( int nil, UBYTE *name, struct AZRequest *ar )
{
    if (BM_sel_2_buftmp( ar, name ))
        {
        ULONG id = func_BMGetId( AGP.UBufTmp, 0, 0 );
        struct AZBM *azb = (struct AZBM *) func_BMGetPtr( id );
        if (azb)
            {
            func_CursorPos( azb->Line, azb->Col );
            if (AGP.WO->Obj.w) ActivateWindow( AGP.WO->Obj.w );
            }
        }
}

ULONG func_ReqOpenBookmark( struct AZRequest *ar )
{
  ULONG errmsg=0;

    if (AGP.AZMast == (APTR)&AGP) { ar->Button = REQBUT_CANCEL; return( TEXT_CMDCANCELLED ); }
    BufTruncate( AGP.UBufTmp, 0 );
    errmsg = func_BMApply( BM_apply_2_buftmp, 1, -1, 0, &AGP.UBufTmp );
    if (!errmsg || errmsg == TEXT_NoBMDefined)
        {
        errmsg = func_ReqList( ar,
            ART_ReqTitle,   UIK_LangString( AGP.UIK, TEXT_Title_Bookmarks ),
            ART_ReqFlags,   ARF_WINDOW|ARF_ASYNC|ARF_MEMPOS,
            ART_OKFunc,     BM_ok,
            ART_CancelFunc, BM_cancel,
            ART_ListInit,   AGP.UBufTmp,
            ART_UserData,   ar,
            TAG_END );
        }
    return( errmsg );
}
