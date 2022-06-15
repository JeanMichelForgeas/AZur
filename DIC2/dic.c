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
extern UBYTE IndexCharDef[];

extern struct SignalSemaphore DicSem;
extern struct List DicList;
extern APTR Pool;


/****** Statics ************************************************/

static UBYTE * __far SuffixTable[] = {
    "tion",
    "tions",
    "teur",
    "teurs",
    "trice",
    "trices",
    "ment",
    "ments",
    "ssiez",
    "ssions",
    "ssent",
    "sses",
    "rent",
    "sse",
    "ient",
    "ntes",
    "nte",
    "nts",
    "ions",
    "iez",
    "ons",
    "ont",
    "ez",
    "nt",
    "it",
    "is",
    "i",
    "z",
    "e",
    "es",
    "s",
    "x",
    };


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

/****************************************************************
 *
 *      Global Routines
 *
 ****************************************************************/

ULONG dic_save( struct DicNode *node )
{
  ULONG errmsg=0, file, i, id, len;
  LONG filesize=0;
  struct GroupHeader *gh;
  struct FileGroupHeader FGH;

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
                    for (i=0; i < NUMGH; i++)
                        {
                        gh = &node->ln_DicEntry[i];
                        if (gh->NumWords && (gh->GroupFlags & GHF_USEABLE))   // reste-t-il des mots dans ce groupe ?
                            {
                            len = BufLength( gh->GroupWords );
                            FGH.FGroupLen = sizeof(struct FileGroupHeader) + len;
                            FGH.FNumWords = gh->NumWords;
                            FGH.FIndex    = i;
                            if (Write( file, &FGH, sizeof(struct FileGroupHeader) ) == sizeof(struct FileGroupHeader))
                                {
                                if (Write( file, gh->GroupWords, len ) == len)
                                    {
                                    filesize += FGH.FGroupLen;
                                    }
                                else { errmsg = ERR_WRITEFILE; break; }
                                }
                            else { errmsg = ERR_WRITEFILE; break; }
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

/*  Fichier

    AZDI            4
    filesize        4
    FileGroupHeader struct FileGroupHeader  Entrée 0 = autres entrées
        Datas       struct WordInfo ...
    FileGroupHeader struct FileGroupHeader  Entrée A
        Datas       struct WordInfo ...
    ...                                     Entrée B...
*/

static ULONG dic_open_verif( UBYTE *name, ULONG *pfile, struct DicNode *node )
{
  ULONG errmsg=0, filesize, readbuf[2];

    if (*pfile = Open( name, MODE_OLDFILE ))
        {
        filesize = Seek( *pfile, Seek( *pfile, 0, OFFSET_END ), OFFSET_BEGINNING );

        if (node->FileSize) // au premier Open() node->FileSize est à zéro
            {
            if (node->FileSize != filesize) errmsg = ERR_READFILE;
            }
        else node->FileSize = filesize;

        if (! errmsg)
            {
            if (filesize >= (8+sizeof(struct GroupHeader)))
                {
                if (Read( *pfile, readbuf, 8 ) == 8)
                    {
                    if (readbuf[0] != B2L('A','Z','D','I') || readbuf[1] != filesize)
                        errmsg = ERR_READFILE;
                    }
                else errmsg = ERR_READFILE;
                }
            else errmsg = ERR_READFILE;
            }
        }
    else{
        errmsg = ERR_OPENFILE;
        if (IoErr() == ERROR_OBJECT_NOT_FOUND) errmsg = ERR_NOTFOUND;
        }
    return( errmsg );
}

static ULONG dic_load_index( struct DicNode *node, ULONG index )
{
  struct FileGroupHeader FGH;
  struct GroupHeader *gh = &node->ln_DicEntry[index];
  ULONG errmsg=0, file=0;
  UBYTE *ub;

    if (gh->GroupLen && !(gh->GroupFlags & GHF_USEABLE))
        {
        if ((errmsg = dic_open_verif( node->ln_Name, &file, node )) == 0)
            {
            if (Seek( file, gh->FileOffset, OFFSET_BEGINNING ) != -1)
                {
                if (Read( file, &FGH, sizeof(struct FileGroupHeader) ) == sizeof(struct FileGroupHeader))
                    {
                    if (FGH.FGroupLen == sizeof(struct FileGroupHeader) + gh->GroupLen)
                        {
                        if (ub = BufAllocP( node->ln_DicPool, gh->GroupLen, 100 ))
                            {
                            if (Read( file, ub, gh->GroupLen ) == gh->GroupLen)
                                {
                                gh->GroupWords = ub; // gh->GroupLen écrasé car union
                                gh->GroupFlags |= GHF_USEABLE;
                                }
                            else errmsg = ERR_READFILE;
                            if (errmsg) BufFree( ub );
                            }
                        else errmsg = ERR_NOMEMORY;
                        }
                    else errmsg = ERR_NOTFOUND;
                    }
                else errmsg = ERR_READFILE;
                }
            else errmsg = ERR_READFILE;
            }
        if (file) Close( file );
        }
    return( errmsg );
}

ULONG dic_load( UBYTE *name )
{
  ULONG errmsg=0, file=0, offset;
  struct DicNode *node=0;
  struct FileGroupHeader FGH;
  struct GroupHeader *gh;

    if (! (node = find_dicname( name )))
        {
        if (node = add_dicname( name ))
            {
            if ((errmsg = dic_open_verif( node->ln_Name, &file, node )) == 0)
                {
                offset = 8;
                while (TRUE)
                    {
                    if (Read( file, &FGH, sizeof(struct FileGroupHeader) ) == sizeof(struct FileGroupHeader))
                        {
                        if (FGH.FIndex < NUMGH)
                            {
                            gh = &node->ln_DicEntry[FGH.FIndex];
                            gh->GroupLen = FGH.FGroupLen - sizeof(struct FileGroupHeader);
                            gh->NumWords = FGH.FNumWords;
                            gh->FileOffset = offset;
                            }

                        offset += FGH.FGroupLen;
                        if (offset == node->FileSize) break; // node->FileSize est mis par dic_open_verif()

                        if (Seek( file, FGH.FGroupLen - sizeof(struct FileGroupHeader), OFFSET_CURRENT ) == -1)
                            { errmsg = ERR_READFILE; break; }
                        }
                    else { errmsg = ERR_READFILE; break; }
                    }
                }
            if (file) Close( file );
            }
        else errmsg = ERR_NOMEMORY;

        if (errmsg)
            {
            if (node) del_dic( node, 0 );
            }
        else{
            node->ln_Flags |= DNF_LOADED;
            }
        }

    if (! errmsg) node->UseCount++;

    return( errmsg );
}

ULONG dic_create( UBYTE *name )
{
  struct DicNode *node;
  ULONG errmsg=0;

    if (! find_dicname( name ))
        {
        if (! (node = add_dicname( name ))) errmsg = ERR_NOMEMORY;
        else{
            node->UseCount++;
            node->ln_Flags |= DNF_MODIFIED;
            if (errmsg = dic_save( node )) del_dic( node, 0 );
            }
        }
    else errmsg = ERR_CANCELLED;

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
        if (force)
            {
            dic_save( node );
            del_dic( node, force );
            }
        else{
            if (dic_save( node ) == 0) del_dic( node, 0 );
            }
        node = succ;
        }
    return(0);
}

LONG equal_ncna( UBYTE *str, struct WordInfo *wi )
{
  UBYTE c1, c2, *p = wi->Word;

    c1 = LowerNoAccentCharDef[*str++];
    c2 = LowerNoAccentCharDef[*p++];
    if (c1 != c2) { if (c1 > c2) return(-1); if (c1 < c2) return(1); }
    while (TRUE)
        {
        if (*str == 0)
            {
            if (*p == 0) return(0); else return(1);
            }
        else{
            if (*p == 0)
                {
                UBYTE *s1, *s2, bit;
                UWORD flags = wi->DicFlags;
                for (bit=0; flags && (bit < 16); bit++)
                    {
                    if (flags & (1 << bit))
                        {
                        s1 = str;
                        s2 = SuffixTable[bit];
                        while (TRUE)
                            {
                            if (*s1 == 0)
                                {
                                if (*s2 == 0) return(0); else return(1);
                                }
                            else{
                                if (*s2 == 0) return(-1);
                                else{
                                    c1 = LowerNoAccentCharDef[*s1++];
                                    c2 = LowerNoAccentCharDef[*s2++];
                                    if (c1 != c2) { if (c1 > c2) return(-1); if (c1 < c2) return(1); }
                                    }
                                }
                            }
                        flags &= ~(1 << bit);
                        }
                    }
                return(-1);
                }
            else{
                c1 = LowerNoAccentCharDef[*str++];
                c2 = LowerNoAccentCharDef[*p++];
                if (c1 != c2) { if (c1 > c2) return(-1); if (c1 < c2) return(1); }
                }
            }
        }
}

LONG equal_exact( UBYTE *str, struct WordInfo *wi )
{
  UBYTE c1, c2, *p = wi->Word;

    c1 = *str++;
    c2 = *p++;
    if (c1 != c2) { if (c1 > c2) return(-1); if (c1 < c2) return(1); }
    while (TRUE)
        {
        if (*str == 0)
            {
            if (*p == 0) return(0); else return(1);
            }
        else{
            if (*p == 0)
                {
                UBYTE *s1, *s2, bit;
                UWORD flags = wi->DicFlags;
                for (bit=0; flags && (bit < 16); bit++)
                    {
                    if (flags & (1 << bit))
                        {
                        s1 = str;
                        s2 = SuffixTable[bit];
                        while (TRUE)
                            {
                            if (*s1 == 0)
                                {
                                if (*s2 == 0) return(0); else return(1);
                                }
                            else{
                                if (*s2 == 0) return(-1);
                                else{
                                    c1 = *s1++;
                                    c2 = *s2++;
                                    if (c1 != c2) { if (c1 > c2) return(-1); if (c1 < c2) return(1); }
                                    }
                                }
                            }
                        flags &= ~(1 << bit);
                        }
                    }
                return(-1);
                }
            else{
                c1 = *str++;
                c2 = *p++;
                if (c1 != c2) { if (c1 > c2) return(-1); if (c1 < c2) return(1); }
                }
            }
        }
}

ULONG dic_addentry( struct DicNode *node, UBYTE *str, UBYTE *str2, ULONG wordflags, ULONG dicflags )
{
  UBYTE *ptr, index;
  struct WordInfo *wi;
  struct Word2Info *wi2;
  struct GroupHeader *gh;
  ULONG errmsg=0, len, len2=0, entrylen, entrylen2=0;

    if (*str == 0) { errmsg = ERR_CANCELLED; goto END_ERROR; }

    len = StrLen( str );    // +1 pour le 0 de fin, -1 car le 2ème fait partie de WordInfo;
    if (len & 1) len++;     // pour rester sur adresses paires
    entrylen = sizeof(struct WordInfo) + len;

    if (str2)
        {
        if (len2 = StrLen( str2 ))  // +1 pour le 0 de fin, -1 car le 2ème fait partie de WordInfo
            {
            wordflags |= SF_STRING2;
            if (len2 & 1) len2++;   // pour rester sur adresses paires
            entrylen2 = sizeof(struct Word2Info) + len2;
            }
        }

    index = IndexCharDef[*str];
    gh = &node->ln_DicEntry[index];
    if (! (gh->GroupFlags & GHF_USEABLE))
        {
        ULONG alloc = 0;

        if (node->ln_Flags & DNF_LOADED)
            {
            if (gh->GroupLen) errmsg = dic_load_index( node, index ); // si existe dans fichier
            else alloc = 1;
            }
        else alloc = 1;

        if (alloc)
            {
            if (ptr = BufAllocP( node->ln_DicPool, 0, 100 ))
                {
                gh->GroupWords = ptr;
                gh->GroupFlags |= GHF_USEABLE;
                }
            else errmsg = ERR_NOMEMORY;
            }
        }

    if (!errmsg && (gh->GroupFlags & GHF_USEABLE))
        {
        ULONG i, c2index, c2offset, maxoffset, addoffset, buflen, addlen, insert;
        LONG equal;

        buflen = BufLength( gh->GroupWords );
        c2index = IndexCharDef[str[1]];
        c2offset = gh->C2Offsets[c2index];

        for (maxoffset=0, i=c2index+1; i < NUMGH; i++) if (maxoffset = gh->C2Offsets[i]) break;
        if (maxoffset == 0) { maxoffset = buflen; } // si 0, on se mettra à la fin

        if (c2offset == 0) { c2offset = maxoffset; } // si 0, on se met avant le suivant

        //--- Si n'existe pas déjà, trouve l'emplacement où insérer
        insert = 1;
        for (addoffset=c2offset; addoffset < maxoffset; )
            {
            wi = (struct WordInfo *)(gh->GroupWords + addoffset);

            equal = equal_ncna( str, wi );  // comp nocase noaccent
            if (equal > 0) break; // on dépasse, donc insérer à partir de addoffset
            if (equal == 0)
                {
                if (equal_exact( str, wi ) == 0) // existe déjà
                    {
                    insert = 0;
                    wi->DicFlags |= dicflags;
                    break;
                    }
                }

            addoffset += wi->EntryLen;
            }

        //--- Fait la place pour la nouvelle entrée
        if (insert)
            {
            addlen = entrylen + entrylen2;
            if (ptr = BufResizeS( &gh->GroupWords, buflen + addlen ))
                {
                //--- Repousse les entrées suivantes dans la table
                for (i=c2index+1; i < NUMGH; i++) if (gh->C2Offsets[i]) gh->C2Offsets[i] += addlen;
                gh->C2Offsets[c2index] = c2offset;

                //--- Repousse les entrées dans le buffer
                ptr += addoffset;
                if (addoffset < buflen) UIK_MemCopy( ptr, ptr+addlen, buflen-addoffset );

                //--- Insère la nouvelle entrée
                wi = (struct WordInfo *)ptr;
                wi->WordFlags = (UBYTE)wordflags;
                wi->EntryLen  = addlen;
                wi->DicFlags  = dicflags;
                wi->WordLen   = len;
                StrCpy( wi->Word, str );

                if (len2)
                    {
                    wi2 = (struct Word2Info *)(ptr + sizeof(struct WordInfo) + entrylen);
                    wi2->WordLen = len2;
                    StrCpy( wi2->Word, str2 );
                    }

                gh->NumWords++;
                gh->GroupFlags |= GHF_MODIFIED;
                node->ln_Flags |= DNF_MODIFIED;
                }
            else errmsg = ERR_NOMEMORY;
            }
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
  ULONG numfound=0, i, buflen, index;

    if (*str) // si chaîne non vide
        {
        if (! (flags & FUF_CASE)) asm_StrToLower( str );
        if (! (flags & FUF_ACCENT)) asm_StrStripAccent( str );

        gh = &node->ln_DicEntry[index=IndexCharDef[*str]];
        if (!(gh->GroupFlags & GHF_USEABLE) && (node->ln_Flags & DNF_LOADED))
            {
            dic_load_index( node, index );
            }

        if (gh->GroupFlags & GHF_USEABLE)
            {
            if (gh->NumWords) numfound = asm_SearchEntry( gh, str, flags );

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
        }
    TotalFound += numfound;
    if (TotalFound && (flags & FUF_SEARCHONE)) return( OK_FOUNDONE );
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
            for (i=0; !errmsg && i < NUMGH; i++)
                {
                if (gh = &node->ln_DicEntry[i])
                    {
                    if (!(gh->GroupFlags & GHF_USEABLE) && (node->ln_Flags & DNF_LOADED))
                        {
                        dic_load_index( node, i );
                        }
                    if (gh->GroupFlags & GHF_USEABLE)
                        {
                        for (j=0, wi=(struct WordInfo *)gh->GroupWords; !errmsg && j < gh->NumWords; j++, wi=(struct WordInfo *)(((ULONG)wi)+sizeof(struct WordInfo)+wi->EntryLen))
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
                            }
                        }
                    else{
                        if (Write( file, "Word group not useable\n", 23 ) != 23) { errmsg = ERR_WRITEFILE; break; }
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
