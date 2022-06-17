/*-------------------------------------------------------------------*
 *                                                                   *
 *    Hashtable.c :                                                  *
 *                                                                   *
 *       Made by Frederic Denis                                      *
 *       Login   <fred@epita.fr>                                     *
 *                                                                   *
 *       Copyright (c) 1993, All rights reserved.                    *
 *                                                                   *
 *-------------------------------------------------------------------*
 *       Strings/data hashing.                                       *
 *-------------------------------------------------------------------*
 *       Started on  Thu May 13 19:07:09 1993                        *
 *       Last update Sat May 15 00:23:56 1993                        *
 *       Last update 07 October 1993 JM Forgeas (pooled allocations) *
 *-------------------------------------------------------------------*/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:Hash.h"
#include "lci:azur.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:_pool_protos.h"

extern struct Glob { ULONG toto; } __near GLOB;

/* Hashing function taken from "Compilers..." from Aho/Sethi/Ullman */
static ULONG __asm HPJW( register __a0 char *C )
{
  char *p;
  ULONG h = 0, g;

    for (p=C; *p != EOS; p++)
    {
        h = (h << 24) + (*p);
        if ((g = h & 0xf0000000) != 0)
            {
            h = h ^ (g >> 24);
            h = h ^ g;
            }
    }
    return( h % PRIME );
}

HashRoot *Hash_Init( void (*FreeFunct)() )
{
  HashRoot *root;
  APTR pool;
  ULONG i;

    if (! (pool = AsmCreatePool( MEMF_ANY, 1024, 1024, ExecBase ))) return(0);
    if (! (root = AsmAllocPooled( pool, sizeof(HashRoot), ExecBase ))) { AsmDeletePool( pool, ExecBase ); return(0); }
    root->Pool = pool;
    root->FreeFunct = FreeFunct;
    for (i=0; i < PRIME; i++) root->Table[i] = 0;
    return( root );
}

APTR Hash_Add( HashRoot *root, UBYTE *name, ULONG size )
{
  ULONG HashCode;
  HashNode *node, *current;

    if (! (node = AsmAllocPooled( root->Pool, sizeof(HashNode)+size, ExecBase ))) return(0);
    node->Symbol = name;
    node->Size = sizeof(HashNode)+size;

    HashCode = HPJW( node->Symbol );
    node->Prev = 0;
    node->Next = current = root->Table[HashCode];
    root->Table[HashCode] = node;
    if (current) current->Prev = node;

    return( (APTR)(((ULONG)node)+sizeof(HashNode)) );
}

/*
void PrintHash( HashRoot *root )
{
  HashNode *current;
  UWORD i;

    for (i=0; i < PRIME; i++)
        {
kprintf( "%3.3ld : ", i );
        current = root->Table[i];
        while (current)
            {
kprintf( "%ls ", current->Symbol );
            current = current->Next;
            }
kprintf( "\n" );
        }
}
*/

APTR Hash_Find( HashRoot *root, char *name )
{
  HashNode *current;

    //------ Fast search
    current = root->Table[HPJW(name)];
    while (current)
        {
        if (intAZ_StrNCEqual( current->Symbol, name )) return( (APTR)(((ULONG)current)+sizeof(HashNode)) );
        current = current->Next;
        }
    return(0);
}

void Hash_Remove( HashRoot *root, APTR ptr )
{
  HashNode *current;

    current = (HashNode *)(((ULONG)ptr) - sizeof(HashNode));

    if (current->Prev)
        {
        current->Prev->Next = current->Next;
        if (current->Next) current->Next->Prev = current->Prev;
        }
    else{
        root->Table[HPJW(current->Symbol)] = current->Next;
        if (current->Next) current->Next->Prev = 0;
        }

    if (root->FreeFunct) (root->FreeFunct)( (APTR)(((ULONG)current)+sizeof(HashNode)) );
    AsmFreePooled( root->Pool, current, current->Size, ExecBase );
}

void Hash_Free( HashRoot *root )
{
  HashNode *current, *ToFree;
  int i;

    if (root)
        {
        if (root->FreeFunct)
            {
            for (i=0; i < PRIME; i++)
                {
                current = root->Table[i];
                while (current)
                    {
                    ToFree = current;
                    current = current->Next;
                    (root->FreeFunct)( (APTR)(((ULONG)ToFree)+sizeof(HashNode)) );
                    }
                }
            }
        if (root->Pool) AsmDeletePool( root->Pool, ExecBase );
        }
}
