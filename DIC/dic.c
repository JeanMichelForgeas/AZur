/****************************************************************
 *
 *      Project:    Objet de commande AZur
 *
 *      Function:   Lecture et utilisation de dictionnaire
 *
 *      Created:    20/12/94 Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "dicserv.h"
#include "_pool_protos.h"

#include "Projects:UIK/INCLUDES/uik_protosint.h"


/****** Imported ************************************************/

extern struct Library *DOSBase;
extern struct Library *SysBase;

extern ULONG asm_SearchEntry( struct GroupHeader *gh, UBYTE *str, ULONG flags );
extern UBYTE asm_CharToUpper( UBYTE ch );

extern struct WordInfo *FoundTable[];

extern UBYTE LowerCharDef[];
extern UBYTE NormalCharDef[];
extern UBYTE LowerNoAccentCharDef[];
extern UBYTE NoAccentCharDef[];
extern UBYTE UpperCharDef[];
extern UBYTE WordCharDef[];

extern struct SignalSemaphore DicSem;
extern struct List DicList;
extern APTR Pool;


/****** Statics ************************************************/


/****** Exported ***********************************************/

ULONG TotalFound;


/****************************************************************
 *
 *      Static Routines
 *
 ****************************************************************/

static void del_dic( struct DicNode *node, ULONG force )
{
    if (node->UseCount > 0) node->UseCount--;

    if (node->UseCount==0 || force)
        {
        Remove( (struct Node *)node );
        AsmDeletePool( node->ln_DicPool, *((APTR*)4) );
        }
}

struct DicNode *find_dicname( UBYTE *dicname )
{
  struct DicNode *node;

    for (node=(struct DicNode *)DicList.lh_Head; node->ln_Succ; node = (struct DicNode *)node->ln_Succ)
        {
        if (StrNCCmp( dicname, node->ln_Name ) == 0) return( node );
        }
    return(0);
}

static struct DicNode *add_dicname( UBYTE *dicname )
{
    if (dicname)
        {
        struct DicNode *node;
        ULONG nodelen, len = StrLen( dicname );
        APTR pool;

        if (pool = (APTR) AsmCreatePool( MEMF_PUBLIC|MEMF_CLEAR, 1024, 1024, *((APTR*)4) ))
            {
            if (node = AsmAllocPooled( pool, (nodelen = sizeof(struct DicNode) + len + 1), *((APTR*)4) ))
                {
                node->ln_Name = (APTR)(((ULONG)node) + sizeof(struct DicNode));
                node->ln_DicPool = pool;
                StrCpy( node->ln_Name, dicname );
                AddTail( &DicList, (struct Node *)node );
                return( node );
                }
            else AsmDeletePool( pool, *((APTR*)4) );
            }
        }
    return(0);
}

//----------------------------------------------

ULONG dic_create( UBYTE *name )
{
  struct DicNode *node;
  ULONG errmsg=0;

    if (! find_dicname( name ))
        {
        if (! (node = add_dicname( name ))) errmsg = ERR_NOMEMORY;
        else node->UseCount++;
        }
    else errmsg = ERR_CANCELLED;

    return( errmsg );
}

ULONG dic_save( struct DicNode *node )
{
  ULONG errmsg=0, file, i, id;
  LONG filesize=0, written;
  struct GroupHeader *gh;

    if (node->ln_Flags & DNF_MODIFIED)
        {
        if (file = Open( node->ln_Name, MODE_NEWFILE ))
            {
            id = B2L('A','Z','D','I');
            if (Write( file, &id, 4 ) == 4)
                {
                filesize = 8;
                if (Write( file, &filesize, 4 ) == 4) // future longueur totale
                    {
                    for (i=0; i < 256; i++)
                        {
                        if (gh = node->ln_DicEntry[i])
                            {
                            if (gh->NumWords)   // reste-t-il des mots dans ce groupe ?
                                {
                                gh->GroupLen = BufLength( (APTR)gh );
                                if ((written = Write( file, gh, gh->GroupLen )) > 0)
                                    {
                                    filesize += written;
                                    if (written != gh->GroupLen) { errmsg = ERR_WRITEFILE; break; }
                                    }
                                else { errmsg = ERR_WRITEFILE; break; }
                                }
                            else{               // sinon on fait le ménage sans sauver
                                BufFreeS( (UBYTE**)&node->ln_DicEntry[i] );
                                }
                            }
                        }
                    Seek( file, 4, OFFSET_BEGINNING );
                    Write( file, &filesize, 4 );
                    }
                else errmsg = ERR_WRITEFILE;
                }
            else errmsg = ERR_WRITEFILE;

            Close( file );
            }
        else errmsg = ERR_OPENWRITEFILE;

        if (!errmsg) node->ln_Flags &= ~DNF_MODIFIED;
        }

    return( errmsg );
}

ULONG dic_load( UBYTE *name )
{
  ULONG errmsg=0, file, *pl;
  LONG filesize, memlen;
  UBYTE *mem;
  struct DicNode *node=0;

    if (! (node = find_dicname( name )))
        {
        if (file = Open( name, MODE_OLDFILE ))
            {
            memlen = filesize = Seek( file, Seek( file, 0, OFFSET_END ), OFFSET_BEGINNING );
            if (filesize >= 8)
                {
                if (mem = AllocMem( memlen, MEMF_PUBLIC|MEMF_CLEAR ))
                    {
                    if (Read( file, mem, filesize ) == filesize)
                        {
                        Close( file ); file = 0;
                        pl = (ULONG*)mem;
                        if (B2L('A','Z','D','I') == *pl++)
                            {
                            if (filesize == *pl++)
                                {
                                if (node = add_dicname( name ))
                                    {
                                    struct GroupHeader *grouphead = (struct GroupHeader *)pl;
                                    UBYTE *ptr;
                                    filesize -= 8;
                                    while (filesize > 0)
                                        {
                                        if (ptr = BufAllocP( node->ln_DicPool, grouphead->GroupLen, 100 ))
                                            {
                                            node->ln_DicEntry[grouphead->GroupChar] = (APTR)ptr;
                                            CopyMem( grouphead, ptr, grouphead->GroupLen );
                                            filesize -= grouphead->GroupLen;
                                            grouphead = (struct GroupHeader *)(((ULONG)grouphead) + grouphead->GroupLen);
                                            }
                                        else { errmsg = ERR_NOMEMORY; filesize = 0; }
                                        }
                                    }
                                else errmsg = ERR_NOMEMORY;
                                }
                            else errmsg = ERR_READFILE;
                            }
                        else errmsg = ERR_READFILE;
                        }
                    else errmsg = ERR_READFILE;
                    FreeMem( mem, memlen );
                    }
                else errmsg = ERR_NOMEMORY;
                }
            else errmsg = ERR_READFILE;
            if (file) Close( file );
            }
        else{
            errmsg = ERR_OPENFILE;
            if (IoErr() == ERROR_OBJECT_NOT_FOUND) errmsg = ERR_NOTFOUND;
            }
        }

    if (errmsg)
        {
        if (node) del_dic( node, 0 );
        }
    else{
        node->UseCount++;
        }

    return( errmsg );
}

ULONG dic_unload( struct DicNode *node )
{
  ULONG errmsg;

    if ((errmsg = dic_save( node )) == 0) del_dic( node, 0 );
    return( errmsg );
}

ULONG dic_unloadall( ULONG force )
{
  struct DicNode *node, *succ;

    for (node=(struct DicNode *)DicList.lh_Head; node->ln_Succ; )
        {
        succ = (struct DicNode *)node->ln_Succ;
        dic_save( node );
        del_dic( node, force );
        node = succ;
        }
    return(0);
}

ULONG dic_addentry( struct DicNode *node, UBYTE *str, UBYTE *str2, ULONG flags )
{
  UBYTE index, buf[300];
  struct WordInfo *wi2, *wi = (struct WordInfo *)buf;
  struct GroupHeader *grouphead;
  ULONG errmsg=0, len, entrylen;
  LONG i;

    if (*str == 0) { errmsg = ERR_CANCELLED; goto END_ERROR; }

    __builtin_memset( buf, 0, 300 );
    index = LowerNoAccentCharDef[*str];

    len = StrLen( str );        // +1 pour le 0 de fin, -1 car le 2ème fait partie de WordInfo;
    if (len & 1) len++;         // pour rester sur adresses paires
    wi->WordFlags = (UBYTE)flags;
    wi->WordLen   = len;

    entrylen = len;             // sans sizeof(struct WordInfo) car en assembleur "move.l (a2)+,d0" puis "addq EntryLen,a2"
    if (entrylen >= 255) { errmsg = ERR_CANCELLED; goto END_ERROR; }
    StrCpy( wi->Word, str );

    if (str2)
        {
        wi->WordFlags |= SF_STRING2;
        wi2 = (struct WordInfo *)(buf + sizeof(struct WordInfo) + entrylen);

        len = StrLen( str2 );   // +1 pour le 0 de fin, -1 car le 2ème fait partie de WordInfo;
        if (len & 1) len++;     // pour rester sur adresses paires
        wi2->WordLen = len;

        entrylen += sizeof(struct WordInfo) + len;
        if (entrylen >= 255) { errmsg = ERR_CANCELLED; goto END_ERROR; }
        StrCpy( wi2->Word, str2 );
        }

    wi->EntryLen = entrylen;

    if (! (grouphead = node->ln_DicEntry[index]))
        {
        if (grouphead = (struct GroupHeader *) BufAllocP( node->ln_DicPool, sizeof(struct GroupHeader), 100 ))
            {
            node->ln_DicEntry[index] = (APTR)grouphead;
            grouphead->GroupChar = index;
            }
        else errmsg = ERR_NOMEMORY;
        }
    if (grouphead)
        {
        if (grouphead = (struct GroupHeader *) BufPasteS( buf, sizeof(struct WordInfo)+entrylen, (UBYTE**)&node->ln_DicEntry[index], BufLength((UBYTE*)grouphead) ))
            {
            grouphead->NumWords++;
            node->ln_Flags |= DNF_MODIFIED;
            }
        else errmsg = ERR_NOMEMORY;
        }
  END_ERROR:
    return( errmsg );
}

ULONG dic_deleteentry( struct DicNode *node, UBYTE *str )
{
    return(0);
}

ULONG dic_searchentry( struct DicNode *node, UBYTE *str, ULONG flags, UBYTE **pub )
{
  struct WordInfo *wi;
  struct GroupHeader *gh;
  ULONG numfound=0, i, buflen;
  UBYTE index;

    if (index = LowerNoAccentCharDef[*str]) // test chaîne vide
        {
        if (! (flags & FUF_CASE)) asm_StrToLower( str );
        if (! (flags & FUF_ACCENT)) asm_StrStripAccent( str );

        if (gh = node->ln_DicEntry[index])
            {
            numfound = asm_SearchEntry( gh, str, flags );
            }

        if (numfound && pub)
            {
            for (i=0; i < numfound; i++)
                {
                wi = FoundTable[i];
                if (buflen = BufLength(*pub)) BufPasteS( "\n", 1, pub, buflen++ );
                BufPasteS( wi->Word, StrLen(wi->Word), pub, buflen );
                if (flags & FUF_TRANSLATE)
                    {
                    BufPasteS( " : ", 3, pub, BufLength(*pub) );
                    wi = (struct WordInfo *)(((ULONG)wi) + sizeof(struct WordInfo) + wi->WordLen);
                    BufPasteS( wi->Word, StrLen(wi->Word), pub, BufLength(*pub) );
                    }
                }
            }
        }
    TotalFound += numfound;
    return(0);
}

ULONG dic_list( struct DicNode *node, UBYTE *filename )
{
  struct WordInfo *wi, *wi2;
  struct GroupHeader *gh;
  ULONG file, i, j, errmsg=0, len;
  UBYTE buf[300];

    if (file = Open( filename, MODE_OLDFILE ))
        {
        Seek( file, 0, OFFSET_END );
        sprintf( buf, "------ \"%ls\"\n", node->ln_Name );
        if (Write( file, buf, StrLen(buf) ) > 0)
            {
            for (i=0; !errmsg && i < 256; i++)
                {
                if (gh = node->ln_DicEntry[i])
                    {
                    for (j=0, wi=(struct WordInfo *)(((ULONG)gh)+sizeof(struct GroupHeader)); !errmsg && j < gh->NumWords; j++, wi=(struct WordInfo *)(((ULONG)wi)+sizeof(struct WordInfo)+wi->EntryLen))
                        {
                        StrCpy( buf, "\"" );
                        StrCat( buf, wi->Word );
                        if (wi->WordFlags & SF_STRING2)
                            {
                            StrCat( buf, "\" : \"" );
                            wi2 = (struct WordInfo *)(((ULONG)wi) + sizeof(struct WordInfo) + wi->WordLen);
                            StrCat( buf, wi2->Word );
                            }
                        StrCat( buf, "\"\n" );
                        len = StrLen( buf );
                        if (Write( file, buf, len ) != len) { errmsg = ERR_WRITEFILE; break; }
                        /*
                        len = StrLen( wi->Word );
                        if (Write( file, wi->Word, len ) == len)
                            {
                            if (wi->WordFlags & SF_STRING2)
                                {
                                if (Write( file, " : ", 3 ) == 3)
                                    {
                                    wi2 = (struct WordInfo *)(((ULONG)wi) + sizeof(struct WordInfo) + wi->WordLen);
                                    len = StrLen( wi2->Word );
                                    if (Write( file, wi2->Word, len ) != len)  errmsg = ERR_WRITEFILE;
                                    }
                                else errmsg = ERR_WRITEFILE;
                                }
                            }
                        else errmsg = ERR_WRITEFILE;
                        if (Write( file, "\n", 1 ) != 1) errmsg = ERR_WRITEFILE;
                        */
                        }
                    }
                }
            }
        else errmsg = ERR_WRITEFILE;
        Close( file );
        }
    else errmsg = ERR_OPENWRITEFILE;

  END_ERROR:
    return( errmsg );
}
