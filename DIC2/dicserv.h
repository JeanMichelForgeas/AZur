
#define B2L(a,b,c,d)    ((ULONG)((((UBYTE)(a))<<24)|(((UBYTE)(b))<<16)|(((UBYTE)(c))<<8)|((UBYTE)(d))))

extern ULONG direct_Lock( void );
extern void  direct_Unlock( void );
extern ULONG direct_Create( UBYTE **name );
extern ULONG direct_Load( UBYTE **name );
extern ULONG direct_Unload( UBYTE **name );
extern ULONG direct_UnloadAll( ULONG force );
extern ULONG direct_Save( UBYTE **name );
extern ULONG direct_AddEntry( UBYTE **name, UBYTE *str, UBYTE *str2, ULONG flags );
extern ULONG direct_DeleteEntry( UBYTE **name, UBYTE *str );
extern ULONG direct_SearchEntry( UBYTE **name, UBYTE *str, ULONG cmd, UBYTE **ubuf );
extern ULONG direct_List( UBYTE **name, UBYTE *filename );
extern struct DicNode *direct_Find( UBYTE *dicname );

#define PORTNAME    "DicServ_Single"
#define REXXNAME    "DicServ_Rexx"

#define ERR_NOMEMORY        1
#define ERR_NODICSERV       2
#define ERR_OPENFILE        3
#define ERR_READFILE        4
#define ERR_CANCELLED       5
#define ERR_OPENWRITEFILE   6
#define ERR_WRITEFILE       7
#define ERR_SEARCHNOTFOUND  8
#define ERR_NOTFOUND        9
#define OK_FOUNDONE        10

struct DicPort
    {
    struct MsgPort Port;
    ULONG (*Lock)( void );
    void  (*Unlock)( void );
    struct DicNode *(*Find)( UBYTE * );
    ULONG (*Create)( UBYTE ** );
    ULONG (*Load)( UBYTE ** );
    ULONG (*Unload)( UBYTE ** );
    ULONG (*UnloadAll)( ULONG force );
    ULONG (*Save)( UBYTE ** );
    ULONG (*AddEntry)( UBYTE **name, UBYTE *str, UBYTE *str2, ULONG flags );
    ULONG (*DeleteEntry)( UBYTE **name, UBYTE *str );
    ULONG (*SearchEntry)( UBYTE **name, UBYTE *str, ULONG flags, UBYTE **ubuf );
    ULONG (*List)( UBYTE **name, UBYTE *filename );
    };

/* Funcs Flags */
#define FUF_TRANSLATE   0x0001
#define FUF_CASE        0x0002
#define FUF_ACCENT      0x0004
#define FUF_SEARCHONE   0x0008

#define DicServ_Lock(a)                 (((struct DicPort *)a)->Lock)()
#define DicServ_Unlock(a)               (((struct DicPort *)a)->Unlock)()
#define DicServ_Find(a,b)               (((struct DicPort *)a)->Find)(b)
#define DicServ_Create(a,b)             (((struct DicPort *)a)->Create)(b)
#define DicServ_Load(a,b)               (((struct DicPort *)a)->Load)(b)
#define DicServ_Unload(a,b)             (((struct DicPort *)a)->Unload)(b)
#define DicServ_UnloadAll(a,b)          (((struct DicPort *)a)->UnloadAll)(b)
#define DicServ_Save(a,b)               (((struct DicPort *)a)->Save)(b)
#define DicServ_AddEntry(a,b,c,d,e)     (((struct DicPort *)a)->AddEntry)(b,c,d,e)
#define DicServ_DeleteEntry(a,b,c)      (((struct DicPort *)a)->DeleteEntry)(b,c)
#define DicServ_SearchEntry(a,b,c,d,e)  (((struct DicPort *)a)->SearchEntry)(b,c,d,e)
#define DicServ_List(a,b,c)             (((struct DicPort *)a)->List)(b,c)

#define NUMGH ('Z'-'A'+2)

struct FileGroupHeader
    {
    ULONG   FGroupLen;   // pour stockage fichier (lui-même y-comris)
    UWORD   FNumWords;
    UBYTE   FIndex;
    UBYTE   FGroupPad;
    };

struct GroupHeader
    {
    union {
        UBYTE   *UGroupWords; // si GHF_USEABLE, pointeur sur l'UIKBuffer
        ULONG   UGroupLen;    // si pas GHF_USEABLE, longueur du GroupHeader dans le fichier (taille de la structure y compris)
    } truc;
    ULONG   FileOffset;  // offset dans le fichier pour recharger le groupe instantanément
    UWORD   NumWords;
    UBYTE   GroupFlags;
    UBYTE   GroupPad;
    ULONG   C2Offsets[NUMGH];
    };
/* Flags de GroupHeader */
#define GHF_USEABLE     0x01    /* le groupe est chargé : GroupWords est un pointeur */
#define GHF_MODIFIED    0x02    /* l'image en mémoire a été modifiée */

#define GroupWords truc.UGroupWords
#define GroupLen truc.UGroupLen

struct DicNode
    {
    struct  Node *ln_Succ;  /* Pointer to next (successor) */
    struct  Node *ln_Pred;  /* Pointer to previous (predecessor) */
    UWORD   ln_pad;
    UWORD   ln_Flags;
    char    *ln_Name;       /* ID string, null terminated */
    APTR    ln_DicPool;
    struct GroupHeader ln_DicEntry[NUMGH];
    ULONG   UseCount;
    ULONG   FileSize;
    };
/* Flags des dictionnaires */
#define DNF_LOADED      0x0001    /* un dico a été lu sur disque */
#define DNF_MODIFIED    0x0002    /* on peut sauver */

struct WordInfo
    {
    UBYTE   WordFlags;  // en asm : << 8 --> comme sur un word
    UBYTE   EntryLen;   // taille de tous les WordInfo de cette entry
    UWORD   DicFlags;   // flags servant à la compression par suffixes, etc...
    UBYTE   WordLen;    // offset à ajouter pour avoir le WordInfo suivant
    UBYTE   Word[1];
    };

struct Word2Info
    {
    UBYTE   WordLen;    // offset à ajouter pour avoir le WordInfo suivant
    UBYTE   Word[1];
    };

/* WordFlags */
#define SF_STRING2      0x01    /* une 2ème chaîne suit (remplacement pour traduction, ...) */
#define SF_MULTIWORDS   0x40    /* plusieurs mots séparés par des \n */
