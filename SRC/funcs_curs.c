/****************************************************************
 *
 *      File:       funcs_curs.c    manipule le curseur
 *      Project:    AZur            editeur de texte
 *
 *      Created:    25/06/93        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"
#include "lci:objview.h"
#include "lci:objzone.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/


/****** Statics *************************************************/


/****************************************************************
 *
 *      Utils
 *
 ****************************************************************/

// LINE/N,COL=COLUMN/N,PAGE/S,REL=RELATIVE/S,WORD/N,TAB/N,OFFSET/S,UP/S,LEFT/S,W=WRAP/S,ENDWORD/S

#define ARGLINE     arg[ 0]
#define ARGCOLUMN   arg[ 1]
#define ARGPAGE     arg[ 2]
#define ARGRELATIVE arg[ 3]
#define ARGWORD     arg[ 4]
#define ARGTAB      arg[ 5]
#define ARGOFFSET   arg[ 6]
#define ARGUP       arg[ 7]
#define ARGLEFT     arg[ 8]
#define ARGWRAP     arg[ 9]
#define ARGENDWORD  arg[10]

static ULONG add_offset( LONG *pline, LONG *pcol, ULONG *plen, LONG offs )
{
  UBYTE *ptr, ch;
  APTR (*func)();

    ptr = intAZ_LineBuf( *pline );

    if (offs > 0) { func = (APTR)func_CharNext; }
    else { func = (APTR)func_CharPrev; offs = -offs; }

    while (offs)
        {
        if ((*func)( ptr, plen, pline, pcol, &ch ) == (APTR)-1) return(0);
        offs--;
        }
    return(1);
}

struct getposinfo
    {
    UBYTE *lineptr;
    ULONG offset;
    LONG searchline;

    LONG startindex;
    LONG startreal;
    LONG line;
    LONG col;
    };

//---------------------------
static LONG fold_getrealptr( struct getposinfo *gpi, LONG realline, struct UIKPList *ul, LONG ind, LONG startindex, LONG startreal )
{
    if (gpi->lineptr == ul->UNode[ind].Buf)
        {
        gpi->startindex = startindex;
        gpi->startreal  = startreal ;
        gpi->line = realline;
        return(1);
        }
    return(0);
}
static LONG find_real_ptr( struct getposinfo *gpi, LONG line )
{
    gpi->lineptr = AGP.FObj->Text.UNode[line].Buf;
    func_FoldApplyLines( (void*)fold_getrealptr, gpi, 0 );
    return( gpi->line );
}

//---------------------------
static LONG fold_getrealindex( struct getposinfo *gpi, LONG realline, struct UIKPList *ul, LONG ind, LONG startindex, LONG startreal )
{
    gpi->startindex = startindex;   // Si searchline est supérieure au nombre de lignes
    gpi->startreal  = startreal ;   // on doit retourner quand même le résultat. Donc
    gpi->line = realline;           // résultat en dehors du test de return.
    if (gpi->searchline == realline) return(1);
    return(0);
}
static LONG find_real_index( struct getposinfo *gpi, LONG realline )
{
    gpi->searchline = realline;
    func_FoldApplyLines( (void*)fold_getrealindex, gpi, 0 );
    return( gpi->line );
}

//---------------------------
static LONG fold_getoffset( struct getposinfo *gpi, LONG realline, struct UIKPList *ul, LONG ind, LONG startindex, LONG startreal )
{
  ULONG len;

    len = BufLength( ul->UNode[ind].Buf );
    gpi->startindex = startindex;   // Si offset est supérieur au nombre de lignes
    gpi->startreal  = startreal ;   // on doit retourner quand même le résultat. Donc
    gpi->line = realline;           // résultat en dehors du test de return.
    gpi->col = (gpi->offset & 0x7fffffff);
    if (len >= gpi->offset) return(1);
    gpi->offset -= (len+1); // +1 à cause du \n de fin de ligne
    return(0);
}

//---------------------------
static ULONG unfold( LONG sline, LONG eline )
{
  struct AZDisplayCmd DI;
  LONG line, col, change;
  ULONG add=0, num_act=0;

    func_CursorErase();

    line = eline + 1;
    while (func_FoldGetPrev( &line, &col ) && (line >= sline))
        {
        change = func_FoldOffSub( line, &DI );
        num_act++;
        add += change;
        }
    if (num_act > 1) func_ReDisplay();
    else if (num_act == 1) func_DisplayCmd( &DI );

    func_CursorDraw();
    return( add );
}

static ULONG func_StripChangeEOL( struct AZObjFile *fo, LONG line )
{
  ULONG changed=0;

    if (TST_STRIPCHANGE)
        {
        UBYTE *ptr;
        ULONG linelen, lineend;

        linelen = intAZ_Buf_Len( line, &ptr );
        lineend = skip_endchar( ptr, ' ', '\t', linelen );
        if (lineend < linelen)
            {
            changed = 1;
            BufTruncate( ptr, lineend );
            if (TST_SHOWEOL || TST_SHOWEOF) // sinon pas la peine on ne voit rien
                func_Display( line, 1, lineend, linelen+1 );
            }
        }
    return( changed );
}

//---------------------------
LONG func_ComputeLineColFlags( LONG *arg, LONG *pline, LONG *pcol, ULONG flags )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZObjView *vo = fo->ActiveView;
  LONG rc=1, line, col, linemul, colmul, words, tabs, depl;
  ULONG /*offset,*/ relative, len;
  struct getposinfo GPI;

    line = fo->Line; col = fo->Col;
    // pas ici car pas la peine : la ligne au-dessus n'est là que par sécurité // if (flags & CLCF_FOLDED) line = find_real_line();

    if (ARGOFFSET)
        {
        GPI.offset = *(ULONG*)ARGOFFSET;
        func_FoldApplyLines( (void*)fold_getoffset, &GPI, 0 );
        col = GPI.col;
        line = GPI.startindex + (GPI.line - GPI.startreal);
        unfold( GPI.startindex, line );
        }
    else{
        if (ARGPAGE)
            {
            if (vo)
                {
                linemul = vo->Zone->Rows - 1;
                colmul = vo->Zone->Cols - 1;
                }
            else{
                linemul = 72;
                colmul = 80;
                }
            }

        //------ ARGUP se conduit comme si il avait ARGRELATIVE seulement pour LINE
        line = fo->Line;
        if (ARGRELATIVE || ARGUP)
            {
            if (flags & CLCF_FOLDED)
                line = find_real_ptr( &GPI, line );
            if (ARGLINE) depl = (*(LONG*)ARGLINE); else depl = 0;
            if (ARGUP && depl>0) depl = -depl;
            if (ARGPAGE) depl = depl * linemul;
            line += depl;
            if (flags & CLCF_FOLDED)
                {
                find_real_index( &GPI, line );
                line = GPI.startindex + (GPI.line - GPI.startreal);
                unfold( GPI.startindex, line );
                }
            }
        else{
            if (ARGLINE)
                {
                line = (*(LONG*)ARGLINE);
                if (ARGPAGE)
                    line = (line - 1) * linemul;
                else{
                    if (flags & CLCF_FOLDED)
                        {
                        if (line != -1) line--;  // Coordonnée origine 0
                        find_real_index( &GPI, line );
                        line = GPI.startindex + (GPI.line - GPI.startreal);
                        unfold( GPI.startindex, line );
                        }
                    else{
                        if (line == -1) line = fo->Text.NodeNum - 1;
                        else line--;      // Coordonnée origine 0
                        }
                    }
                }
            // else : pas besoin de regarder les folds car le curseur ne change pas de ligne
            }
        /*
        relative = (ARGRELATIVE || ARGUP);

        if (ARGLINE) line = (*(LONG*)ARGLINE);
        else { if (relative) line = 0; else line = fo->Line; }

        if (relative)
            {
            if (ARGUP && line>0) line = -line;
            if (ARGPAGE) line = line * linemul;
            line += fo->Line;
            }
        else if (line == -1) line = fo->Text.NodeNum - 1;
        else if (ARGLINE)
            {
            line--; // Coordonnée origine 0
            if (ARGPAGE) line = line * linemul;
            }
        */

        if (line < 0 || line >= fo->Text.NodeNum) goto END_ERROR;
        len = intAZ_LineLen( line );

        //------ ARGLEFT se conduit comme si il avait ARGRELATIVE seulement pour COL
        relative = (ARGRELATIVE || ARGLEFT);

        if (ARGCOLUMN) col = (*(LONG*)ARGCOLUMN);
        else { if (relative) col = 0; else col = fo->Col; }

        if (relative)
            {
            if (ARGLEFT && col>0) col = -col;
            if (ARGPAGE)
                {
                col = col * colmul;
                col += fo->Col;
                }
            else{
                if (col < 0 || ARGWRAP)
                    {
                    LONG tmpcol = fo->Col;
                    rc = add_offset( &line, &tmpcol, &len, col );
                    col = tmpcol;
                    if (! rc) goto END_ERROR;
                    }
                else col += fo->Col;
                }
            }
        else if (col == -1)
            {
            func_StripChangeEOL( fo, line );
            col = len = intAZ_LineLen( line );
            }
        else if (ARGCOLUMN)
            {
            col--; // Coordonnée origine 0
            if (col < 0) goto END_ERROR;
            if (ARGPAGE) col = col * colmul;
            if (((ULONG)col) > MAXSIGNED) col = MAXSIGNED;
            }

        //------ ARGWORD
        if (ARGWORD)
            {
            words = *(LONG*)ARGWORD;
            if (ARGLEFT && words > 0) words = -words;
            rc = func_CountWords( line, col, words, &line, &col, 1 );
            if (words == 0) rc = 1;
            }

        //------ ARGTAB
        if (ARGTAB)
            {
            tabs = *(LONG*)ARGTAB;
            if (ARGLEFT && tabs > 0) tabs = -tabs;
            col = func_CountTabs( col, tabs );
            }
        }

    //----------------------------------------------------
  END_ERROR:
    if (col < 0) { col = 0; rc = 0; }
    if (line < 0) { line = 0; rc = 0; }
    else if (line >= fo->Text.NodeNum)
        {
        line = fo->Text.NodeNum - 1; rc = 0;
        len = intAZ_LineLen( line );
        }

    if (ARGWRAP)
        {
        if (col <= len)
            {
            if (! ARGCOLUMN) col = fo->CEdCol;
            else fo->CEdCol = col;
            }
        if (col > len)
            {
            col = len;
            }
        }

    if (ARGENDWORD)
        {
        UBYTE *ptr, ch;
        len = intAZ_Buf_Len( line, &ptr );
        ch = intAZ_CharCurrent( col, len, ptr );
        if (func_CharIsWord( ch ))
            {
            while ((ptr = func_CharNext( ptr, &len, &line, &col, &ch )) != (APTR)-1)
                { if (! func_CharIsWord( ch )) break; }
            }
        }

    *pline = line; *pcol = col;
    if (rc==0 && ARGPAGE) rc = 1;
    return( rc );
}
//kprintf( "line=%ld, startindex=%ld, GPI.line=%ld, GPI.startreal=%ld\n",line, GPI.startindex, GPI.line, GPI.startreal );

LONG func_ComputeLineCol( LONG *arg, LONG *pline, LONG *pcol )
{
    return( func_ComputeLineColFlags( arg, pline, pcol, 0 ) );
}

/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

static void set_all_hprops( struct AZObjView *vo )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZObjZone *zo = vo->Zone;

    if (fo->Line >= zo->TopLine && fo->Line <= zo->TopLine + zo->Rows) // ligne comprise dans view ?
        {
        /*if (vo != fo->ActiveView)*/ HGetMaxLen( zo, zo->TopLine ); // car sinon déjà fait dans hv_scroll()
        UpdateHProp( vo, 0 );
        }
}

static ULONG hv_scroll( upd )
{
  struct AZObjFile *fo = AGP.FObj;
  struct AZObjView *vo;
  struct AZObjZone *zo;
  LONG numdep=0, all=0, newtopline, newtopcol, bas, droite;
  ULONG updatestats = 0;

    if (! (fo && fo->WO && fo->WO->Obj.Status == UIKS_STARTED && !TST_DISPLOCK)) return(0);

    vo = fo->ActiveView;
    zo = vo->Zone;
    bas = zo->TopLine + zo->Rows - 1;
    droite = zo->TopCol + zo->Cols - 1;

    //------------------------------------- hauteur
    //------ si au-dessus
    if (fo->Line < zo->TopLine)
        {
        if (fo->OldLine < zo->TopLine || fo->OldLine >= bas) { newtopline = fo->Line - zo->Rows/2; all = 1; }
        else { newtopline = zo->TopLine - (fo->OldLine - fo->Line); }
        numdep++;
        }
    //------ si au-dessous
    else if (fo->Line >= bas)
        {
        if (fo->OldLine < zo->TopLine || fo->OldLine >= bas) { newtopline = fo->Line - zo->Rows/2; all = 1; }
        else { newtopline = zo->TopLine + (fo->Line - fo->OldLine); }
        numdep++;
        }
    else newtopline = zo->TopLine;
    {
    LONG numlines = fo->Text.NodeNum;
    LONG lastline = numlines + PLUSLINES;
    if (newtopline < 0) newtopline = 0;
    if (newtopline + zo->Rows > lastline) newtopline = lastline - zo->Rows;
    if (newtopline < 0) newtopline = 0;
    if (newtopline >= numlines) newtopline = numlines - 1;
    }
    //------------------------------------- exploration pour prop du bas
    if (upd || numdep) HGetMaxLen( zo, newtopline ); // si numdep, HGetMaxLen() sera fait par oview_ChangeViewPosition()

    //------------------------------------- largeur
    //------ si à gauche
    if (fo->Col < zo->TopCol)
        {
        if (fo->OldCol < zo->TopCol || fo->OldCol >= droite) { newtopcol = fo->Col - zo->Cols/2; all = 1; }
        else { newtopcol = zo->TopCol - (fo->OldCol - fo->Col); }
        numdep++;
        upd = 1; // ici car il faut rafraichir aussi les autres views
        }
    //------ si à droite
    else if (fo->Col >= droite)  // si on agrandi en horizontal
        {
        if (fo->OldCol < zo->TopCol || fo->OldCol >= droite) { newtopcol = fo->Col - zo->Cols/2; all = 1; }
        else { newtopcol = zo->TopCol + (fo->Col - fo->OldCol); }
        numdep++;
        upd = 1; // ici car il faut rafraichir aussi les autres views
        }
    else{
        newtopcol = zo->TopCol;
        }
    if (newtopcol < 0) newtopcol = 0;
    if (newtopcol + zo->Cols > zo->NewHPropLineLen) newtopcol = zo->NewHPropLineLen - zo->Cols;
    if (newtopcol < 0) newtopcol = 0;

    if (numdep)
        {
        if (numdep > 1 || all)
            {
            zo->TopLine = newtopline;
            zo->TopCol = newtopcol;
            oview_DisplayView( vo );
            UPDATE_PROP(fo); //UPDATE_STAT_PROP(fo);
            updatestats = 1;
            }
        else{
            if (oview_ChangeViewPosition( vo, newtopline, newtopcol ))
                {
                UPDATE_PROP(fo); //UPDATE_STAT_PROP(fo);
                updatestats = 1;
                }
            }
        }

    if (upd) map_view_nodes1_nolock( (void*)set_all_hprops, 0 );  // toutes les views ?

    return( updatestats );
}

void func_CursorPosReal( LONG line, LONG col, ULONG scroll, ULONG endblock )
{
  struct AZObjFile *fo = AGP.FObj;
  struct KeyCom_Line kc;
  ULONG upd = 0, line_changed = 0, updatestats = 0;
  struct KeyHandle *kh = (struct KeyHandle *) AGP.AZMast->KeyHandle;

    if (! fo) return;
    if (col < 0) col = 0;
    if (((ULONG)col) > MAXSIGNED) col = MAXSIGNED;
    /*
    if (TST_USEMARGIN)
        {
        if (col < fo->LeftMargin) col = fo->LeftMargin;
        if (col > fo->RightMargin) col = fo->RightMargin;
        }
    */
    if (line < 0) line = 0;
    if (line >= fo->Text.NodeNum) line = fo->Text.NodeNum - 1;
    /*
    if (TST_REALTAB)
        {
        col = pos_tab( col );
        }
    */

    fo->FoundLine = 0; fo->FoundCol = -1;

    if (line != fo->Line || col != fo->Col)
        {
        if (line != fo->Line && fo->Line < fo->Text.NodeNum)
            {
            //------ Tronquer la ligne
            if (func_StripChangeEOL( fo, fo->Line )) upd = 1;

            //------ Appeler la commande de changement de ligne
            kc.OldLine = fo->Line; kc.OldCol = fo->Col;
            kc.NewLine = line; kc.NewCol = col;
            line_changed = 1;
            if (kh->BeforeLine)
                {
                func_ExecuteCmdObj( kh->BeforeLine, (ULONG)&kc );
                if (kh->BeforeLine->ResCode) return;
                }
            }
        //------ Indiquer le changement
        fo->OldCol = fo->Col; fo->OldLine = fo->Line;
        fo->Line = fo->WorkLine = line; fo->Col = fo->WorkCol = col;

        UPDATE_STAT(fo);
        updatestats = 1;
        func_DisplayCurs( fo->OldLine, fo->OldCol );

        //------ Scroller le display
        }
    if (scroll) updatestats |= hv_scroll( upd );

    if (updatestats && AGP.WO && AGP.WO->Obj.w != IntuitionBase->ActiveWindow)
        ofi_TickUpdateDisplay( 0, fo );

    SETCURSOR_VISIBLE(fo);
    func_DisplayCurs( fo->Line, fo->Col );
    func_TextPrep( fo->Line, fo->Col, 0, 0, 0 );
    if (endblock)
        func_BlockSetFutur( fo->Line, fo->Col );
    if (line_changed)
        if (kh->AfterLine) func_ExecuteCmdObj( kh->AfterLine, (ULONG)&kc );
}

void func_CursorPos( LONG line, LONG col )
{
    func_CursorPosReal( line, col, 1, 1 );
}

void func_CursorErase()
{
  struct AZObjFile *fo = AGP.FObj;

    SETCURSOR_INVISIBLE(AGP.FObj);
    func_DisplayCurs( fo->Line, fo->Col );
}

void func_CursorDraw()
{
  struct AZObjFile *fo = AGP.FObj;

    //if (! (fo->Flags & AZFILEF_CURSVISIBLE))
        {
        SETCURSOR_VISIBLE(fo);
        func_DisplayCurs( fo->Line, fo->Col );
        }
}
