
extern ULONG __far DOSBase;

static UBYTE __far Key1[] = "Muriele";

static UBYTE __far versiontag[] = "\0$VER: Patch 1.1 (1.10.94)";

static UBYTE __far Search[] = "\x70\x03\x28\x6F\x00\x08\x20\x6C\x00\x04\xE7\x80\x20\x70\x08\x08\x42\x28\xFF\xFC\x20\x10\x20\xAF\x00\x0C\x90\x88";

static UBYTE __far Key2[] = "Hanke";

static ULONG search( UBYTE *mem, UBYTE *end, ULONG *poffset )
{
  UBYTE *ptr, *found = 0;
  ULONG i, num = 0;

    for (ptr=mem; ptr < end; ptr++)
        {
        if (*ptr == *Search)
            {
            for (i=1; i < sizeof(Search); i++)
                {
                if (ptr[i] != Search[i]) break;
                }
            ptr += i;
            if (i == sizeof(Search)) { found = ptr; num++; }
            }
        }

    if (num) *poffset = found - mem;
    return( num );
}

void main( int argc, UBYTE *argv[] )
{
  UBYTE *filename=0, *username=0;
  ULONG file=0, filelen, offset, num;
  UBYTE *mem=0, replace[50]; // laisser sur la pile pour éviter de trouver la clef avec des 0

    if (argc == 2) { filename = argv[1]; }
    else if (argc == 3) { filename = argv[1]; username = argv[2]; }
    else { fprintf( Output(), "Usage: Patch file [user]\n" ); goto END_ERROR; }

    if (! (file = Open( filename, MODE_OLDFILE )))
        { fprintf( Output(), "Can't open %ls\n", filename ); goto END_ERROR; }

    filelen = Seek( file, Seek( file, 0, OFFSET_END ), OFFSET_BEGINNING );
    if (! (mem = AllocMem( filelen, MEMF_PUBLIC )))
        { fprintf( Output(), "Not enough memory\n" ); goto END_ERROR; }

    if (Read( file, mem, filelen ) != filelen)
        { fprintf( Output(), "Read error\n" ); goto END_ERROR; }

    //---------- recherche

    num = search( mem, mem+filelen, &offset );
    if (num != 1)
        { fprintf( Output(), "Found %ld times\n", num ); goto END_ERROR; }

    if (username && username[0])
        {
        //------ Codage
        strcpy( replace, username );
        DecodeText( Key1, replace, 50 );
        DecodeText( Key2, replace, 50 );

        if (Seek( file, offset, OFFSET_BEGINNING ) == -1)
            { fprintf( Output(), "Seek error\n" ); goto END_ERROR; }
        if (Write( file, replace, 50 ) != 50)
            { fprintf( Output(), "Write error\n" ); goto END_ERROR; }
        fprintf( Output(), "Write OK\n" );
        }
    else{
        //------ Décodage
        if (Seek( file, offset, OFFSET_BEGINNING ) == -1)
            { fprintf( Output(), "Seek error\n" ); goto END_ERROR; }
        if (Read( file, replace, 50 ) != 50)
            { fprintf( Output(), "Read error\n" ); goto END_ERROR; }

        DecodeText( Key2, replace, 50 );
        DecodeText( Key1, replace, 50 );
        replace[50-1] = 0;
        fprintf( Output(), "Owner name is: '%ls'\n", replace );
        }


  END_ERROR:
    if (file) Close( file );
    if (mem) FreeMem( mem, filelen );
}
