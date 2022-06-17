/****************************************************************
 *
 *      Project:   AZUR
 *      Function:  objets
 *
 *      Created:   17/04/93 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"
#include "uiki:objwindow.h"
#include "uiki:objwinfilesel.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:objfile.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

struct cmdreq_functab
    {
    UBYTE   *Name;
    ULONG   (*OpenFunc)();
    ULONG   Offset;
    ULONG   Flags;
    };


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern void ofi_FS_OKFunc();
extern void ofi_FS_CancelFunc();

extern ULONG func_ReqOpenDisplayPrefs();
extern ULONG func_ReqOpenEditPrefs();
extern ULONG func_ReqOpenEnvPrefs();
extern ULONG func_ReqOpenSearchPrefs();
extern ULONG func_ReqOpenPrintPrefs();
extern ULONG func_ReqOpenFontPrefs();
extern ULONG func_ReqOpenWindowPrefs();
extern ULONG func_ReqOpenHex();
extern ULONG func_ReqOpenSearch();
extern ULONG func_ReqOpenSearchReplace();
extern ULONG func_ReqOpenInfo();
extern ULONG func_ReqOpenAbout();
extern ULONG func_ReqOpenLineColumn();
extern ULONG func_ReqOpenOffset();
extern ULONG func_ReqOpenBookmark();


/****** Exported ***********************************************/


/****** Statics ************************************************/

#define REQOFFS(a) (ULONG)&(((struct AZur *)0L)->a)

static struct cmdreq_functab __far Tab[] =
    {
    { "DisplayPrefs",  func_ReqOpenDisplayPrefs , REQOFFS(AZReqDisplayPrefs ), 0 },
    { "EditPrefs",     func_ReqOpenEditPrefs    , REQOFFS(AZReqEditPrefs    ), 0 },
    { "EnvPrefs",      func_ReqOpenEnvPrefs     , REQOFFS(AZReqEnvPrefs     ), 0 },
    { "FilePrefs",     func_ReqOpenEnvPrefs     , REQOFFS(AZReqEnvPrefs     ), 0 },
    { "SearchPrefs",   func_ReqOpenSearchPrefs  , REQOFFS(AZReqSearchPrefs  ), 0 },
    //{ "PrintPrefs",    func_ReqOpenPrintPrefs   , REQOFFS(AZReqPrintPrefs   ), 0 },
    //{ "FontPrefs",     func_ReqOpenFontPrefs    , REQOFFS(AZReqFontPrefs    ), 0 },
    { "FontPrefs",     func_ReqOpenWindowPrefs  , REQOFFS(AZReqWindowPrefs  ), 0 },
    { "WindowPrefs",   func_ReqOpenWindowPrefs  , REQOFFS(AZReqWindowPrefs  ), 0 },

    { "Hex",           func_ReqOpenHex          , REQOFFS(AZReqHex          ), 0 },
    { "Search",        func_ReqOpenSearch       , REQOFFS(AZReqSearch       ), 0 },
    { "Find",          func_ReqOpenSearch       , REQOFFS(AZReqSearch       ), 0 },
    { "Replace",       func_ReqOpenSearchReplace, REQOFFS(AZReqReplace      ), 0 },
    { "FindChange",    func_ReqOpenSearchReplace, REQOFFS(AZReqReplace      ), 0 },
    { "Info",          func_ReqOpenInfo         , REQOFFS(AZReqInfo         ), 0 },
    { "About",         func_ReqOpenAbout        , REQOFFS(AZReqAbout        ), 0 },
    { "LineCol",       func_ReqOpenLineColumn   , REQOFFS(AZReqLine         ), 0 },
    { "Offset",        func_ReqOpenOffset       , REQOFFS(AZReqOffset       ), 0 },
    { "Bookmarks",     func_ReqOpenBookmark     , REQOFFS(AZReqBookmark     ), 0 },

    { 0, 0 },
    };


/****************************************************************
 *
 *      Objets
 *
 ****************************************************************/

static struct cmdreq_functab *get_tab_entry( UBYTE *name )
{
  struct cmdreq_functab *tab;

    for (tab=Tab; tab->Name; tab++)
        {
        if (ARG_MatchArg( name, tab->Name ) != -1)
            return( tab );
        }
    return(0);
}

ULONG func_ReqOpen( UBYTE *reqname, ULONG *button )
{
  struct cmdreq_functab *tab;
  struct AZRequest *ar;

    if (! (tab = get_tab_entry( reqname ))) goto END_ERROR;
    ar = (struct AZRequest *) (((ULONG)&AGP)+tab->Offset);

    if (ar->Obj)
        {
        if (ar->WO && ar->WO->Obj.w)
            {
            WindowToFront( ar->WO->Obj.w );
            ActivateWindow( ar->WO->Obj.w );
            goto END_ERROR;
            }
        else goto END_INUSE;
        }

    ar->Flags = tab->Flags;
    ar->Button = REQBUT_NOEVENT;
    ar->padb = 0;
    ar->padw = 0;
    ar->ErrMsg = (*tab->OpenFunc)( ar );

    if (button) *button = (ULONG)ar->Button;
    return( (ULONG)ar->ErrMsg );

  END_ERROR:
    return( TEXT_CMDCANCELLED );

  END_INUSE:
    return( TEXT_ERR_RequestInUse );
}

void func_ReqClose( UBYTE *reqname, struct AZRequest *ar ) // soit un nom, soit une structure
{
  struct cmdreq_functab *tab;
  struct UIKObj *obj;

    if (reqname)
        {
        if (! (tab = get_tab_entry(reqname))) return;
        ar = (struct AZRequest *) (((ULONG)&AGP)+tab->Offset);
        }
    else{
        if (! ar) return;
        }

    if (ar->WO)
        {
        struct Window *w;
        if ((w = ar->WO->Obj.w) && (ar->Flags & ARF_MEMPOS))
            {
            ar->Left   = w->LeftEdge;
            ar->Top    = w->TopEdge;
            ar->Width  = w->Width;
            ar->Height = w->Height;
            if (ar->WO->WinFlags & UIKW_AUTOADJUSTSIZE)
                {
                ar->Width  -= (ar->WO->LeftBorderWidth + ar->WO->RightBorderWidth);
                ar->Height -= (ar->WO->TopBorderHeight + ar->WO->BottomBorderHeight);
                }
            }
        ar->WO = 0;
        }

    obj = ar->Obj; // ar peut être un objet mémoire ajouté au ReqString par smart_request()
    ar->Obj = 0;   // donc il faut mettre ar->Obj à 0 avant le UIK_Remove, sinon écriture
    UIK_Remove( obj ); // la mémoire libre... Merci FVMem alias The Memkeeper.
}

//»»»»»» Cmd_func_ReqOpen   "REQUESTER/A"

void CmdFunc_ReqOpen_Do( struct CmdObj *co )
{
  ULONG errmsg;

    if (errmsg = func_ReqOpen( co->ArgRes[0], 0 ))
        intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_ReqClose   "REQUESTER/A"

void CmdFunc_ReqClose_Do( struct CmdObj *co )
{
    func_ReqClose( co->ArgRes[0], 0 );
}

//»»»»»» Cmd_ReqFile    TITLE=PROMPT,PATH,PAT=PATTERN,DIRMODE/S,AS=ASSIGN/S,EXIST/S,SI=SHOWICONS/S

#define RFA_TITLE     co->ArgRes[0]
#define RFA_PATH      co->ArgRes[1]
#define RFA_PATTERN   co->ArgRes[2]
#define RFA_DIRMODE   co->ArgRes[3]
#define RFA_ASSIGN    co->ArgRes[4]
#define RFA_EXIST     co->ArgRes[5]
#define RFA_SHOWICONS co->ArgRes[6]

void CmdFunc_ReqFile_Do( struct CmdObj *co )
{
  struct UIKObj *wfs, *parent;
  ULONG errmsg=0;

    if (! (parent = AGP.WO)) parent = AGP.UIK;
    if (wfs = UIK_AddObjectTagsB( UIKBase, "UIKObj_WinFileSel", parent,
                UIKTAG_OBJ_Title,               RFA_TITLE,
                UIKTAG_OBJ_TitleFl_Addr,        TRUE,
                UIKTAG_OBJ_FontName,            AGP.Prefs->ViewFontName,
                UIKTAG_OBJ_FontHeight,          AGP.Prefs->ViewFontHeight,
                UIKTAG_WFS_OKFunc,              ofi_FS_OKFunc,
                UIKTAG_WFS_CancelFunc,          ofi_FS_CancelFunc,
                UIKTAG_WFS_InitPath,            RFA_PATH,
                UIKTAG_WFS_AssignList,          RFA_ASSIGN,
                UIKTAG_WFS_Pattern,             RFA_PATTERN,
                UIKTAG_WFSFl_ReturnReturn,      TRUE,
                UIKTAG_WFSFl_OK_IfNotExists,    RFA_EXIST ? 0 : 1,
                UIKTAG_WFSFl_DirsOnly,          RFA_DIRMODE,
                UIKTAG_WFSFl_ShowIcons,         RFA_SHOWICONS,
                UIKTAG_WFSFl_LeavePath,         FALSE,
                TAG_END ))
        {
        if (UIK_Start( wfs ))
            {
            if (sync_request( wfs ) != REQBUT_OK) errmsg = TEXT_CMDCANCELLED;
            else{
                BufSetS( AGP.FileSelResult, BufLength(AGP.FileSelResult), &AGP.RexxResult );
                }
            }
        else errmsg = TEXT_NOMEMORY;

        UIK_Remove( wfs );
        }
    else errmsg = TEXT_NOMEMORY;

    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}

//»»»»»» Cmd_ReqMulti   TITLE,CONV=CONVERT/S,REQWIN/S,ACT=STRACTIVATE/S
//                      STR1/S,T1=TITLESTR1,D1=DEFSTR1=DEFAULT,
//                      STR2/S,T2=TITLESTR2,D2=DEFSTR2=DEFAULT2,
//                      B1=BUTOK/K,B2=BUTMID/K,B3=BUTCANCEL/K,STDOK/S,STDCANCEL/S,TEXT=PROMPT/F

#define RFM_TITLE       co->ArgRes[ 0]
#define RFM_CONVERT     co->ArgRes[ 1]
#define RFM_REQWIN      co->ArgRes[ 2]
#define RFM_ACTIVATE    co->ArgRes[ 3]
#define RFM_STR1        co->ArgRes[ 4]
#define RFM_TITLESTR1   co->ArgRes[ 5]
#define RFM_DEFSTR1     co->ArgRes[ 6]
#define RFM_STR2        co->ArgRes[ 7]
#define RFM_TITLESTR2   co->ArgRes[ 8]
#define RFM_DEFSTR2     co->ArgRes[ 9]
#define RFM_BUTOK       co->ArgRes[10]
#define RFM_BUTMID      co->ArgRes[11]
#define RFM_BUTCANCEL   co->ArgRes[12]
#define RFM_STDOK       co->ArgRes[13]
#define RFM_STDCANCEL   co->ArgRes[14]
#define RFM_TEXT        co->ArgRes[15]

void CmdFunc_ReqMulti_Do( struct CmdObj *co )
{
  UBYTE *text=0, *ub=0, **pub1=0, **pub2=0, *ok=0, *mid=0, *can=0, *ptr, *ubuf=0;
  ULONG code;

    if (ubuf = BufAllocP( AGP.Pool, 0, 200 ))
        {
        if (text = RFM_TEXT)
            {
            if (RFM_CONVERT) ub = func_C2Bin( text, StrLen( text ) );
            if (ub) text = ub;
            }
        if (RFM_STR1 || RFM_DEFSTR1)
            {
            if (RFM_DEFSTR1) BufSetS( RFM_DEFSTR1, StrLen(RFM_DEFSTR1), &AGP.UBufTmp );
            else BufTruncate( AGP.UBufTmp, 0 );
            pub1 = &AGP.UBufTmp;
            }
        if (RFM_STR2 || RFM_DEFSTR2)
            {
            if (RFM_DEFSTR2) BufSetS( RFM_DEFSTR2, StrLen(RFM_DEFSTR2), &ubuf );
            else BufTruncate( ubuf, 0 );
            pub2 = &ubuf;
            }
        if (RFM_STDOK) ok = (APTR)-1; else ok = RFM_BUTOK;
        if (RFM_STDCANCEL) can = (APTR)-1; else can = RFM_BUTCANCEL;
        mid = RFM_BUTMID;

        code = func_SmartRequest( RFM_TITLE, text, RFM_TITLESTR1, pub1, RFM_TITLESTR2, pub2, ok, mid, can, 0, (RFM_ACTIVATE?ARF_ACTIVATE:0) | ((TST_REQWIN || RFM_REQWIN) ? ARF_WINDOW:0) );

        switch (code)
            {
            case REQBUT_OK:     ptr = "OK";     break;
            case REQBUT_MIDDLE: ptr = "MIDDLE"; break;
            case REQBUT_CANCEL:
            default:            ptr = "CANCEL"; break;
            }
        func_BufPrintf( &AGP.RexxResult, 0, "%ls,\"%ls\",\"%ls\"", ptr, pub1?*pub1:"", pub2?*pub2:"" );

        /* pas de code erreur, afin de pouvoir utiliser les strings avec les 3 boutons */

        BufFree( ub );
        BufFree( ubuf );
        }
}

//»»»»»» Cmd_ReqList    TITLE,CONV=CONVERT/S,SORT/S,STR/S,DS=DEFSTR=DEFAULT/K,
//                      SEL=SELECT/K,LIST=TEXT=PROMPT/A/F

#define RFL_TITLE       co->ArgRes[0]
#define RFL_CONVERT     co->ArgRes[1]
#define RFL_SORT        co->ArgRes[2]
#define RFL_STR         co->ArgRes[3]
#define RFL_DEFSTR      co->ArgRes[4]
#define RFL_SELECT      co->ArgRes[5]
#define RFL_TEXT        co->ArgRes[6]

static void list_ok( int nil, UBYTE *name, struct AZRequest *ar )
{
    if (name) BufSetS( name, StrLen(name), &AGP.RexxResult );
    func_ReqClose( 0, ar );
}

void list_cancel( int nil, struct AZRequest *ar )
{
    func_ReqClose( 0, ar );
}

void CmdFunc_ReqList_Do( struct CmdObj *co )
{
  UBYTE *text=0, *ub=0, **pub=0, *ptr;
  ULONG errmsg=0, code;
  struct AZRequest AR;

    text = RFL_TEXT;

    if (RFL_CONVERT)
        { if (ub = func_C2Bin( text, StrLen( text ) )) text = ub; }

    if (RFL_STR || RFL_DEFSTR)
        {
        if (RFL_DEFSTR) BufSetS( RFL_DEFSTR, StrLen(RFL_DEFSTR), &AGP.UBufTmp );
        else BufTruncate( AGP.UBufTmp, 0 );
        pub = &AGP.UBufTmp;
        }

    __builtin_memset( &AR, 0, sizeof(struct AZRequest) );
    errmsg = func_ReqList( &AR,                         // synchrone
        ART_ReqTitle,       RFL_TITLE ? RFL_TITLE : "",
        ART_ReqFlags,       (RFL_SORT ? 0 : ARF_NOSORT) | (pub ? ARF_STRGAD : 0), // synchrone
        ART_OKFunc,         list_ok,
        ART_CancelFunc,     list_cancel,
        ART_ListInit,       text,
        ART_ListSelect,     RFL_SELECT,
        TAG_END );

    BufFree( ub );
    if (errmsg) intAZ_SetCmdResult( co, errmsg );
}
