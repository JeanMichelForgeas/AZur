/*-------------------------------------------------------------------*
 *                                                                   *
 *    Hashtable.h :                                                  *
 *                                                                   *
 *       Made by Frederic Denis                                      *
 *       Login   <fred@epita.fr>                                     *
 *                                                                   *
 *       Copyright (c) 1993, All rights reserved.                    *
 *                                                                   *
 *-------------------------------------------------------------------*
 *       Hashing data types.                                         *
 *-------------------------------------------------------------------*
 *       Started on  Thu May 13 19:15:30 1993                        *
 *       Last update Fri Jul 23 00:38:17 1993                        *
 *-------------------------------------------------------------------*/

#define PRIME 211
#define EOS '\0'

typedef struct _HashNode
{
    struct _HashNode    *Prev;    /* Previous symbol */
    struct _HashNode    *Next;    /* Next symbol */
    char                *Symbol;  /* Symbol... */
    ULONG               Size;     /* Total size : Node + extra */
} HashNode;

typedef struct _HashRoot
{
    HashNode    *Table[PRIME];
    APTR        Pool;
    void        (*FreeFunct)();
} HashRoot;

/* HashAdd modes */

/* Function prototypes */

HashRoot *Hash_Init( void (*FreeFunct)() );
APTR Hash_Add( HashRoot *root, UBYTE *name, ULONG size );
APTR Hash_Find( HashRoot *root, char *name );
void Hash_Remove( HashRoot *root, APTR ptr );
void Hash_Free( HashRoot *root );
