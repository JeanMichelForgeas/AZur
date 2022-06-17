/****************************************************************
 *
 *      Project:   AZUR
 *      Function:  Gestion primaire des objets de comande
 *
 *      Created:   12/04/93 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "uiki:objwindow.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:hash.h"

#include "lci:objfile.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;

extern struct TagItem * __far GermTable[];


/****** Exported ***********************************************/


/****** Statics ************************************************/

static void show_error( ULONG code, UWORD msg, UBYTE *cmd );


/****************************************************************
 *
 *      Routines
 *
 ****************************************************************/

struct CmdGerm *func_FindRegGerm( char *name )
{
  HashRoot *root = AGP.AZMast->RGHash;
  HashNode *current;
  UWORD i, j;
  UBYTE *ptr, **names;

    //------ Fast search
    current = root->Table[HPJW(name)];
    while (current)
        {
        if (intAZ_StrNCEqual( current->Symbol, name )) return( (struct CmdGerm *)(((ULONG)current)+sizeof(HashNode)) );
        current = current->Next;
        }
    //------ Deep search
    for (i=0; i < PRIME; i++)
        {
        current = root->Table[i];
        while (current)
            {
            names = ((struct CmdGerm *)(((ULONG)current)+sizeof(HashNode)))->Names;
            for (j=0; ptr = names[j]; j++)
                {
                if (intAZ_StrNCEqual( ptr, name ))
                    {
                    return( (struct CmdGerm *)(((ULONG)current)+sizeof(HashNode)) );
                    }
                }
            current = current->Next;
            }
        }
    return(0);
}

//--------------------------------------------------------------------------

void eng_ShowRequest( UBYTE *fmt, WORD error, UBYTE *p1, UBYTE *p2, UBYTE *p3 )
{
  UBYTE buf[200];

    sprintf( buf, fmt, p1, p2, p3 );

    AGP.RexxError = error;                              // simule intAZ_SetCmdResult
    AGP.MaxError = 20;                                  // pour option SAVE
    if (UIKBase)
        BufSetS( buf, StrLen( buf ), &AGP.RexxResult ); // simule intAZ_SetCmdResult

    if (IntuitionBase && (!AGP.Prefs || !TST_ERRORQUIET))
        {
        struct Window *w=0;
        struct IntuiText msg = { -1, -1, JAM1, 10, 10, NULL, NULL, NULL },
                         ok  = { -1, -1, JAM1, 6, 3, NULL, "OK", NULL };

        msg.IText = buf;
        if (AGP.WO) w = AGP.WO->Obj.w;
        AutoRequest( w, &msg, 0, &ok, 0, 0, (__builtin_strlen( buf ) + 13) * 8, 55 );
        }
}

static void no_memory()
{
    eng_ShowRequest( "%ls", TEXT_PARSE_ERRORBASE + PAERR_NOTENOUGH_MEMORY, UIK_LangString( AGM.UIK, TEXT_PARSE_ERRORBASE + PAERR_NOTENOUGH_MEMORY ), 0, 0 );
}

static ULONG __asm PrepareNewGerm( register __a0 struct tagitem *GermTagList, register __a1 struct CmdGerm *rg )
{
    if (! eng_RGNewAllActions( rg )) goto END_ERROR;
    rg->RegGerm = rg; // pour que DoAction() s'y retrouve
    rg->ObjSize = sizeof(struct CmdObj);
    if (! eng_Tags2Struct( GermTagList, rg )) goto END_ERROR;
    return(1);

  END_ERROR:
    return(0);
}

static void unload_germ( struct CmdGerm *rg )
{
    eng_DoAction( (APTR)rg, AZA_UNLOAD, 0 );
    eng_RGDeleteAllActions( rg );
    if (rg->Segment) UnLoadSeg( rg->Segment );
}

ULONG eng_RegisterInternalObjects( void )
{
  struct TagItem *g, **p = GermTable;
  struct CmdGerm rg, *cg;
  HashRoot *root = AGP.AZMast->RGHash;

    if (! (root = AGP.AZMast->RGHash = Hash_Init( unload_germ ))) goto END_ERROR;

    while (g = *p++)
        {
        __builtin_memset( &rg, 0, sizeof(struct CmdGerm) );
        rg.Flags = AZAFF_INTERNAL;
        if (! PrepareNewGerm( g, &rg )) goto END_ERROR;
        if (! (cg = Hash_Add( root, rg.Names[0], sizeof(struct CmdGerm) ))) goto END_ERROR;
        *cg = rg; cg->RegGerm = cg;
        eng_DoAction( (APTR)cg, AZA_LOAD, 0 );
        }
//PrintHash( AGP.AZMast->RGHash );
    return(1);

  END_ERROR:
    no_memory();
    return(0);
}

void eng_UnregisterAllObjects()
{
    Hash_Free( AGP.AZMast->RGHash );
    AGP.AZMast->RGHash = 0;
}

ULONG func_UnloadGermForce( ULONG force, UBYTE *name )
{
  struct CmdGerm *rg;
  ULONG rc=-1;

    ObtainSemaphore( &AGP.AZMast->RGSem );
    if (rg = func_FindRegGerm( name ))
        {
        if ((rc = rg->UsageCount) == 0 || force)  // sinon retourne UsageCount
            {
            Hash_Remove( AGP.AZMast->RGHash, rg );
            rc = 0;
            }
        }
    ReleaseSemaphore( &AGP.AZMast->RGSem );
    return( rc );
}

ULONG __asm eng_DisposeGerm( register __a0 UBYTE *name )
{
    return( func_UnloadGermForce( 0, name ) );
}

void __asm eng_DisposeObject( register __a1 struct CmdObj *co )
{
    if (co)
        {
        if (co->Flags & AZCOF_NEWOK)
            {
            eng_DoAction( co, AZA_DISPOSE, 0 );
            BufFree( co->ExtraBuf );
            }
        co->RegGerm->UsageCount--;
        if (co->PArg.CmdLine) ARG_FreeParse( &co->PArg );
        PoolFree( co );
        }
}

struct CmdObj * __asm eng_NewPrivateObject( register __a0 struct CmdGerm *rg, register __a1 struct TagItem *objtaglist )
{
  struct CmdObj *co;
  ULONG ver, size = rg->ObjSize + (4 * rg->Items);

    rg->UsageCount++;

    if (! (co = PoolAlloc( size ))) goto END_ERROR;
    co->RegGerm = rg;
    co->ArgRes = (APTR*) (((ULONG)co) + rg->ObjSize);
    co->ExeFrom = AGP.ExeFrom; // défaut : pour si pas de Tag (donc utilisateur)

    co->PArg.Flags = PAF_ACCEPTEOL; // pour le calcul de la longueur sans $0A
    if (! (ver = eng_Tags2Struct( objtaglist, co ))) goto END_ERROR;
    if ((ver != -1) && (ver > (*((ULONG*)&rg->Version))))
        {
        eng_ShowRequest( UIK_LangString( AGP.UIK, TEXT_BadVersion ), TEXT_BadVersion, rg->Names[0], (UBYTE*)(ver>>16), (UBYTE*)(ver&0xffff) );
        goto END_ERROR2;
        }
    if (rg->CmdTemplate && co->PArg.CmdLine) // même si rg->CmdTemplate on ne parse pas si pas de CmdLine
        {
        co->PArg.Buffer = (UBYTE*)&co->PArg.UserData;
        co->PArg.BufSize = 4;
        co->PArg.PoolHeader = AGP.Pool;
        co->PArg.Flags = PAF_POOLED|PAF_ACCEPTEOL;
        if (! ARG_Parse( rg->CmdTemplate, co->ArgRes, &co->PArg, 0, (ULONG)DOSBase ))
            {
            eng_ShowRequest( "%ls: %ls", TEXT_PARSE_ERRORBASE + co->PArg.Error, rg->Names[0],
                UIK_LangString( AGP.UIK, TEXT_PARSE_ERRORBASE + co->PArg.Error ), 0 );
            goto END_ERROR2;
            }
        }
    co->Flags |= (rg->Flags & AZAFM_OKMASK);
    co->Flags |= AZCOF_NEWOK;
    eng_DoAction( co, AZA_NEW, 0 );
    return( co );

  END_ERROR:
    no_memory();
  END_ERROR2:
    eng_DisposeObject( co );
    return(0);
}

static struct TagItem *FindHunkGerm( ULONG segment )
{
 ULONG germ, *segaddr = (ULONG *) (segment << 2);

    if (TypeOfMem( segaddr ) == 0 || (ULONG)segaddr < 1000) return(0);
    germ = *((ULONG *)(segaddr + 2)); // 4 + moveq #20,d0 + rts
    while (segaddr)
        {
        ULONG memsize = *(segaddr - 1) - 8;
        ULONG memstart = (ULONG) (segaddr + 1);
        if (germ >= memstart && germ < memstart + memsize) return( (struct TagItem *) germ );
        segaddr = (ULONG *) ((*segaddr) << 2);
        }
    return(0);
}

struct CmdGerm * __asm eng_NewGerm( register __a0 UBYTE *name )
{
  struct CmdGerm germ, *rg;
  UBYTE filename[150], loaded=0;
  ULONG lock=0, save;
  struct TagItem *GermTagList;
  ULONG found, segment=0;

    ObtainSemaphore( &AGP.AZMast->RGSem );
    if (! (rg = func_FindRegGerm( name )))
        {
        save = UIK_DisableDOSReq();

        sprintf( filename, "AZUR:Objects/%-.90ls", name );  //--- le fichier existe?
        if (lock = Lock( filename, SHARED_LOCK )) name = filename;
        else{
            sprintf( filename, "SYS:AZur/Objects/%-.90ls", name );
            if (lock = Lock( filename, SHARED_LOCK )) name = filename;
            else{
                sprintf( filename, "AZur/Objects/%-.90ls", name );
                if (lock = Lock( filename, SHARED_LOCK )) name = filename;
                else{
                    sprintf( filename, "Objects/%-.90ls", name );
                    if (lock = Lock( filename, SHARED_LOCK )) name = filename;
                    else lock = Lock( name, SHARED_LOCK );
                    }
                }
            }

        UIK_EnableDOSReq( 0, save );

        found = 0;
        if (lock)
            {
            ULONG file, buf, exe=0;

            UnLock( lock );

            //--- vérifier que c'est un fichier exécutable
            if (file = Open( name, MODE_OLDFILE ))
                {
                if ((Read( file, &buf, 4 ) == 4) && (buf == 0x000003f3)) exe = 1;
                Close( file );
                }
            if (exe)
                {
                //--- la mémoire peut se libérer, donc essayer 2 fois LoadSeg()
                if (! (segment = LoadSeg( name )))
                    if (! (segment = LoadSeg( name )))
                        {
                        eng_ShowRequest( "%ls: %ls", TEXT_OBJ_MEMORYOBJ, name, UIK_LangString( AGM.UIK, TEXT_OBJ_MEMORYOBJ ), 0 );
                        goto END_ERROR;
                        }

                //--- vérifie si c'est un germe objet
                if (GermTagList = FindHunkGerm( segment ))
                    {
                    found = 1;
                    __builtin_memset( &germ, 0, sizeof(struct CmdGerm) );
                    germ.Segment = segment;
                    if (! PrepareNewGerm( GermTagList, &germ )) goto END_ERROR;
                    if (! (rg = Hash_Add( AGP.AZMast->RGHash, germ.Names[0], sizeof(struct CmdGerm) ))) goto END_ERROR;
                    *rg = germ; rg->RegGerm = rg;
                    loaded = 1;
                    }
                }
            }
        if (found == 0)
            {
            eng_ShowRequest( "%ls: %ls", TEXT_OBJ_NOTFOUND, name, UIK_LangString( AGM.UIK, TEXT_OBJ_NOTFOUND ), 0 );
            goto END_ERROR;
            }
        }
    ReleaseSemaphore( &AGP.AZMast->RGSem );
    if (loaded) eng_DoAction( (APTR)rg, AZA_LOAD, 0 );

    return( rg );

  END_ERROR:
    if (segment) UnLoadSeg( segment );
    ReleaseSemaphore( &AGP.AZMast->RGSem );
    return(0);
}

struct CmdObj * __asm eng_NewObject( register __a0 UBYTE *name, register __a1 struct TagItem *objtaglist )
{
  struct CmdGerm *rg;

    if (rg = eng_NewGerm( name )) return( eng_NewPrivateObject( rg, objtaglist ) );
    return(0);
}

//------------------------------------------------------------------------------------

UIKBUF *EqualReplace( APTR pool, UBYTE *cmdstring )
{
  struct AZEqual *ae;
  UIKBUF *buf=0;

    if (ae = FindEqual( cmdstring ))
        {
        ULONG len = StrLen( cmdstring );
        if (buf = BufAllocP( pool, len, 4 ))
            {
            CopyMem( cmdstring, buf, len );
            BufCutS( &buf, 0, StrLen(ae->Equal) );
            if (BufPasteS( ae->TrueCmd, StrLen(ae->TrueCmd), &buf, 0 ))
                {
                return( buf );
                }
            BufFree( buf );
            }
        }
    return(0);
}

ULONG eng_ExecuteAZurCmd( UBYTE *cmd, ULONG from, ULONG actionparm )
{
  UBYTE tmp[200], *mem=0, *p=cmd, *cmdname="", *parm="";
  struct CmdObj *co;
  ULONG len, code;

    if (! (mem = p = EqualReplace( AGP.Pool, cmd )))
        {
        len = StrLen( cmd );
        if (len < 200) { CopyMem( cmd, tmp, len+1 ); p = tmp; }
        else{
            if (! (mem = BufAllocP( AGP.Pool, len, len ))) return( 20 );
            CopyMem( cmd, mem, len ); p = mem;
            }
        }

    p += skip_char( p, ' ', '\t' );  // saute espaces entre début et cmdname
    if (*p)
        {
        cmdname = p;
        p += find_char( p, ' ', '\t' ); // cherche espace de fin cmdname
        if (*p)
            {
            *p++ = 0;                  // cmdname OK
            parm = p + skip_char( p, ' ', '\t' ); // saute espaces entre cmdname et parms
            }
        }

    if (*cmdname)
        {
        if (from == -1) from = AGP.ExeFrom;
        if (co = eng_NewObjectTags( cmdname, AZT_CmdLine, parm, AZT_ExtraBuf, mem, AZT_ExeFrom, from, TAG_END ))
            {
            func_ExecuteCmdObj( co, actionparm ); // met intAZ_SetCmdResult
            code = co->ResCode;
            if (from == FROMMENU) StoreLastCmd( co );
            eng_DisposeObject( co );
            return( code );
            }
        else code = 20;
        }
    else code = 0;

    BufFree( mem );
    return( code );
}

ULONG func_ExecuteAZurCmd( UBYTE *cmd, ULONG actionparm )
{
    return( eng_ExecuteAZurCmd( cmd, FROMPGM, actionparm ) );
}

//------------------------------------------------------------------------------------

ULONG func_ExecuteCmdObj( struct CmdObj *co, ULONG actionparm )
{
  ULONG err=0, flags=co->Flags;

    if (TST_READONLY) { if (flags & AZAFF_DOMODIFY) { err = TEXT_ExeObj_Modify; goto END_ERROR; } }

         if (co->ExeFrom & FROMMACRO)  { if (! (flags & AZAFF_OKINMACRO))  { err = TEXT_ExeObj_Macro;    goto END_ERROR; } }
    else if (co->ExeFrom & FROMMENU)   { if (! (flags & AZAFF_OKINMENU))   { err = TEXT_ExeObj_Menu;     goto END_ERROR; } }
    else if (co->ExeFrom & FROMKEY)    { if (! (flags & AZAFF_OKINKEY))    { err = TEXT_ExeObj_Key;      goto END_ERROR; } }
    else if (co->ExeFrom & FROMMOUSE)  { if (! (flags & AZAFF_OKINMOUSE))  { err = TEXT_ExeObj_Mouse;    goto END_ERROR; } }
    else if (co->ExeFrom & FROMJOY)    { if (! (flags & AZAFF_OKINJOY))    { err = TEXT_ExeObj_Joystick; goto END_ERROR; } }
    else if (co->ExeFrom & FROMHOTKEY) { if (! (flags & AZAFF_OKINHOTKEY)) { err = TEXT_ExeObj_HotKey;   goto END_ERROR; } }
    else if (co->ExeFrom & FROMAREXX)  { if (! (flags & AZAFF_OKINAREXX))  { err = TEXT_ExeObj_ARexx;    goto END_ERROR; } }

    if (co->ExeFrom & FROMMASTER) { if (! (flags & AZAFF_OKINMASTER)) { err = TEXT_ExeObj_Master; goto END_ERROR; } }

    if (AGP.RexxResult[0]) BufTruncate( AGP.RexxResult, 0 );
    AGP.RexxError = 0;

    if ((AGP.WorkFlags & WRF_MAC_RECORDING) && (flags & AZAFF_OKINMACRO) && (co->ExeFrom & (FROMMENU|FROMKEY|FROMAREXX)))
        {
        func_MacroAdd( co->RegGerm->Names[0], co->PArg.CmdLine );
        if (AGP.WorkFlags & WRF_MAC_QUIET) goto END_RETURN;
        }

    eng_DoAction( co, AZA_DO, actionparm );

//kprintf( "*** ---> '%ls %ls'\n", co->RegGerm->Names[0], co->PArg.CmdLine );

    if (co->ResCode == 0)   // si erreur ne pas appeler eng_DoAction() car il modifie ResCode
        {
#ifdef AZUR_PRO
        eng_DoAction( co, AZA_UNDO, actionparm );
        if (co->ResCode == 0)
#endif
            eng_DoAction( co, AZA_SETMENU, actionparm );
        }
  END_RETURN:
//kprintf( "+++%ld\n", (ULONG)co->ResCode );
    return( (ULONG)co->ResCode );

  END_ERROR:
    intAZ_SetCmdResult( co, err );
//kprintf( "---%ld\n", (ULONG)co->ResCode );
    return( (ULONG)co->ResCode );
}
