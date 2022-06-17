/****************************************************************
 *
 *      File:       funcs_conv.c    conversions en tout genre
 *      Project:    AZur            editeur de texte
 *
 *      Created:    23/03/94        Jean-Michel Forgeas
 *
 ****************************************************************/


/****** Includes ************************************************/

#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:arg3_protos.h"

#include "lci:azur.h"
#include "lci:funcs.h"
#include "lci:funcsint.h"
#include "lci:eng_obj.h"
#include "lci:azur_protos.h"

#include "lci:objfile.h"


/****** Imported ************************************************/

extern struct Glob { ULONG toto; } __near GLOB;


/****** Exported ************************************************/


/****** Statics *************************************************/

/*               normal chars:       ¡¢£¤¥¦§\¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ */

static UBYTE __far accent_tab1[] = " ¡¢£¤¥¦§\¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿AAAAAAÆCEEEEIIIIÐNOOOOO×OUUUUYÞßaaaaaaæceeeeiiiionooooo÷ouuuuyþy";
static UBYTE __far accent_tab2[] =  "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°EA°°°°°E°°°E°°°°°°E°E°°°E°°°°°°°ea°°°°°e°°°e°°°°°°e°e°°°e°°e";

static UBYTE __far bit7_tab1[] =   " !cL.Y!.\"c.<--R-o#23'u..,1.>...?AAAAAAACEEEEIIIIDNOOOOOxOUUUUYPsaaaaaaaceeeeiiiionooooo/ouuuuypy";
static UBYTE __far bit7_tab2[] =    "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°EAE°°°°E°°°E°°°°°°E°E°°°E°°s°°°°eae°°°°e°°°e°°°°°°e°e°°°e°°e";

/****************************************************************
 *
 *      Utils
 *
 ****************************************************************/

UBYTE func_CharStripAccent( UBYTE ch, UBYTE *res )
{
  UBYTE len=1, ch1, index;

    if (ch > 0xA0)
        {
        index = ch - 0xA0;
        ch = accent_tab1[index];
        ch1 = accent_tab2[index];
        if (! (ch1 & 0x80)) len++;
        }
    if (res)
        { *res++ = len; *res++ = ch; *res = ch1; }
    return( ch );
}

UBYTE func_CharStripBit7( UBYTE ch, UBYTE *res )
{
  UBYTE len=1, ch1, index;

    if (ch & 0x80)
        {
        if (ch < 0xA0)
            { ch = '.'; len = 1; }
        else{
            index = ch - 0xA0;
            ch = bit7_tab1[index];
            ch1 = bit7_tab2[index];
            if (! (ch1 & 0x80)) len++;
            }
        }
    if (res)
        { *res++ = len; *res++ = ch; *res = ch1; }
    return( ch );
}

//-----------------------------------------------------------------------------------

static UBYTE * __far MntlKeys[] =
    {
    "<ENVOI>",
    "<RETOUR>",
    "<REPETITION>",
    "<GUIDE>",
    "<ANNULATION>",
    "<SOMMAIRE>",
    "<CORRECTION>",
    "<SUITE>",
    "<CONNEXION>",
    0
    };

static UBYTE * __far MntlCodes[] =
    {
    "\x01\x20",
    "\x01\x69",
    "\x01\x63",
    "\x02\x19\x23",
    "\x02\x19\x30",
    "\x01\x59",
    "\x01\x7c",
    "\x02\x19\x27",
    "\x01\x22",
    "\x01\x63",
    "\x02\x19\x30",
    "\x01\x3c",
    "\x01\x7e",
    "\x01\x60",
    "\x01\x52",
    "\x01\x7e",

    "\x02\x19\x30",
    "\x02\x19\x31",
    "\x01\x32",
    "\x01\x33",
    "\x01\x27",
    "\x01\x75",
    "\x01\x71",
    "\x01\x27",
    "\x01\x2c",
    "\x01\x31",
    "\x02\x19\x30",
    "\x01\x3e",
    "\x02\x19\x3c",
    "\x02\x19\x3d",
    "\x02\x19\x3e",
    "\x01\x3f",

    "\x03\x19\x41\x41",
    "\x03\x19\x42\x41",
    "\x03\x19\x43\x41",
    "\x01\x41",
    "\x03\x19\x48\x41",
    "\x01\x41",
    "\x01\x41",
    "\x03\x19\x4b\x43",
    "\x03\x19\x41\x45",
    "\x03\x19\x42\x45",
    "\x03\x19\x43\x45",
    "\x03\x19\x48\x45",
    "\x03\x19\x41\x49",
    "\x03\x19\x42\x49",
    "\x03\x19\x43\x49",
    "\x03\x19\x48\x49",

    "\x01\x44",
    "\x01\x4e",
    "\x03\x19\x41\x4f",
    "\x03\x19\x42\x4f",
    "\x03\x19\x43\x4f",
    "\x01\x4f",
    "\x03\x19\x48\x4f",
    "\x01\x78",
    "\x01\x30",
    "\x03\x19\x41\x55",
    "\x03\x19\x42\x55",
    "\x03\x19\x43\x55",
    "\x03\x19\x48\x55",
    "\x01\x59",
    "\x01\x70",
    "\x02\x19\x7b",

    "\x03\x19\x41\x61",
    "\x03\x19\x42\x61",
    "\x03\x19\x43\x61",
    "\x01\x61",
    "\x03\x19\x48\x61",
    "\x01\x61",
    "\x01\x61",
    "\x03\x19\x4b\x63",
    "\x03\x19\x41\x65",
    "\x03\x19\x42\x65",
    "\x03\x19\x43\x65",
    "\x03\x19\x48\x65",
    "\x03\x19\x41\x69",
    "\x03\x19\x42\x69",
    "\x03\x19\x43\x69",
    "\x03\x19\x48\x69",

    "\x01\x64",
    "\x01\x6e",
    "\x03\x19\x41\x6f",
    "\x03\x19\x42\x6f",
    "\x03\x19\x43\x6f",
    "\x01\x6f",
    "\x03\x19\x48\x6f",
    "\x02\x19\x38",
    "\x01\x30",
    "\x03\x19\x41\x75",
    "\x03\x19\x42\x75",
    "\x03\x19\x43\x75",
    "\x03\x19\x48\x75",
    "\x01\x79",
    "\x01\x70",
    "\x01\x79",
    };

void func_Amiga2Vdx( UBYTE **pptr, LONG start, ULONG convlen )
{
  LONG pos;
  UBYTE ch, *found, len;

    pos = start + convlen - 1;
    while (pos >= start)
        {
        ch = (*pptr)[pos];
        //------ est-ce une touche de fonction? (ENVOI, SUITE, etc...)
        if (ch == '>')
            {
            ULONG n, first;
            UBYTE *p, *name;
            for (p=*pptr, first=pos; first && p[first] != '<'; ) first--;
            if (p[first] == '<')
                {
                for (n=0; name=MntlKeys[n]; n++)
                    {
                    if (intAZ_IsSubStr( name, &p[first] )) break;
                    }
                if (name)
                    {
                    p[first] = 0x13;
                    p[first+1] = n + 'A';
                    BufCutS( pptr, first+2, pos-first-1 );

                    pos = first;
                    goto NEXT;
                    }
                }
            }

        //------ est-ce un code à transformer?
        if (ch >= 0xa0) found = MntlCodes[ch-0xa0];
        else if (ch == 0x60) found = "\x01\x27";
        else found = 0;

        if (found)
            {
            len = *found++;
            (*pptr)[pos] = *found++;// on remplace directement le premier code
            if (--len)              // si il y a plus d'un code, on insère
                {
                if (! BufPasteS( found, len, pptr, pos+1 )) break;
                AGP.FObj->WorkCol += len + 1;
                }
            }
      NEXT:
        pos--;
        }
}

UBYTE *func_Vdx2Amiga( UBYTE **pptr, ULONG start, ULONG len )
{
  UBYTE *udest, *src = *pptr;
  struct UIKPBuffer *up = (struct UIKPBuffer *)(((ULONG)src)-sizeof(struct UIKPBuffer));
  UBYTE ch, char2;

    if (! (udest = BufAllocP( up->Pool, 0, 10 ))) goto END_ERROR;
    if (! BufPasteS( src, start, &udest, 0 )) goto END_ERROR;

    while (TRUE)
        {
        if (len-- == 0) goto END_ERROR; else ch = *src++;

        rec_ge20:
            if (ch >= 0x20)
                {
                if (! BufPasteS( &ch, 1, &udest, intAZ_LineBufLen(udest) )) goto END_ERROR;
                continue;
                }
        rec_lt20:
            if (ch == 0x07) continue;
            if (ch == 0x13) goto rec_sep;
            if (ch == 0x1b) goto rec_esc;
            if (ch == 0x1f) goto rec_pro2;
            if (ch == 0x16) goto rec_ss2;
            if (ch == 0x19) goto rec_ss2;
            continue;

        rec_sep:
            if (len-- == 0) goto END_ERROR; else ch = *src++;
            if ((ch > 0x40) && (ch < 0x50))
                {
                UBYTE *ptr = MntlKeys[ch-'A'];
                if (! BufPasteS( ptr, StrLen(ptr), &udest, intAZ_LineBufLen(udest) )) goto END_ERROR;
                }
            if (ch < 0x20) goto rec_lt20;
            continue;

        rec_esc:
            if (len-- == 0) goto END_ERROR; else ch = *src++;
            if (ch == 0x3b) goto rec_pro3;
            if (ch == 0x3a) goto rec_pro2;
            if (ch == 0x39) goto rec_pro1;
            if (ch < 0x20) goto rec_lt20;
            continue;

        rec_pro3:
            if (len-- == 0) goto END_ERROR; else ch = *src++;
            if (ch < 0x20) goto rec_lt20;
        rec_pro2:
            if (len-- == 0) goto END_ERROR; else ch = *src++;
            if (ch < 0x20) goto rec_lt20;
        rec_pro1:
            if (len-- == 0) goto END_ERROR; else ch = *src++;
            if (ch < 0x20) goto rec_lt20;

        rec_ss2:
            if (len-- == 0) goto END_ERROR; else ch = *src++;
            if (ch < 0x20) goto rec_lt20;
            switch (ch)
                {
                case 0x41: char2=0; break;
                case 0x42: char2=1; break;
                case 0x43: char2=2; break;
                case 0x48: char2=3; break;
                case 0x44: char2=4; break;
                case 0x4a: char2=5; break;
                case 0x4b: char2=6; break;
                case 0x6a: ch  = 0xf0; goto rec_ge20;
                case 0x7a: ch  = 0xf1; goto rec_ge20;
                default:   ch |= 0x80; goto rec_ge20;
                }

            if (len-- == 0) goto END_ERROR; else ch = *src++;
            if (ch < 0x20) goto rec_lt20;
            switch (ch)
                {
                case 'A':
                case 'a':
                    if (char2 < 3) ch += char2 - 1;
                    else if (char2 == 3) ch += 3;
                    else if (char2 == 4) ch += 2;
                    else if (char2 == 5) ch += 4;
                    break;
                case 'E':
                case 'e':
                    if (char2 < 4) ch += char2 + 3;
                    break;
                case 'O':
                case 'o':
                    if (char2 < 3) ch += char2 + 3;
                    else if (char2 == 3) ch += 7;
                    else if (char2 == 4) ch += 6;
                    break;
                case 'U':
                case 'u':
                    if (char2 < 4) ch += char2 + 4;
                    break;
                case 'I':
                case 'i':
                    if (char2 < 4) ch += char2 + 3;
                    break;
                case 'C':
                case 'c':
                    if (char2 == 6) ch += 4;
                    break;
                default:
                    continue;
                }
            ch |= 0x80;
            goto rec_ge20;
        }

  END_ERROR:
    return( udest );
}

//-----------------------------------------------------------------------------------

static UBYTE __far tca[256];
static UBYTE __far tci[256];
static UBYTE __far tdata[] = {
    32,173,155,156,32,157,32,21,32,32,166,174,170,32,32,247,
    248,241,32,32,32,230,32,32,32,32,167,175,172,171,32,168,
    65,65,65,65,142,143,146,128,69,144,69,69,73,73,73,73,
    32,165,79,79,79,79,153,32,237,85,85,85,154,32,32,225,
    133,160,131,65,132,134,145,135,138,130,136,137,141,161,140,139,
    235,164,149,162,147,79,148,32,233,151,163,150,129,89,232,152
    };

static void do_tables()
{
  UWORD i;
  UBYTE *ci=tci, *ca=tca, *dat=tdata;

    for (i=160; i < 256; i++)
        { ca[i] = dat[i-160]; }
    for (i=128; i < 256; i++)
        { if (ca[i] > 127) ci[ca[i]] = i; else ci[ca[i]] = 64; }
}

static void transcode( UBYTE *buf, ULONG len, ULONG AMTOIBM )
{
  ULONG i;
  UBYTE b, *arr, lim;

    if (tca[160] == 0) do_tables(); // non protégé du multi-tâches, pas nécessaire

    if (AMTOIBM) { arr = tca; lim = 159; }
    else { arr = tci; lim = 127; }

    for (i=0; i < len; i++)
        {
        b = buf[i];
        if (b > lim) buf[i] = arr[b];
        }
}

void func_MSDOS2Amiga( UBYTE **pptr, ULONG start, ULONG len )
{
    transcode( *pptr + start, len, 0 );
}

void func_Amiga2MSDOS( UBYTE **pptr, ULONG start, ULONG len )
{
    transcode( *pptr + start, len, 1 );
}

//-----------------------------------------------------------------------------------

UBYTE *func_C2Bin( UBYTE *src, ULONG len )
{
  ULONG ublen, res;
  UBYTE buf[4], ch, *ub, *end;

    if (ub = BufAllocP( AGP.Pool, len, len ))
        {
        ublen = 0;
        for (end=src+len; src < end; )
            {
            ch = *src++;
            if (ch == '\\' && *src)
                {
                ch = *src; len = 1;
                switch (ch)
                    {
                    case 'n': ch = '\n'; break;
                    case 't': ch = '\t'; break;
                    case 'r': ch = '\r'; break;
                    default:
                        if (ch == 'x' || ch == 'X' || ch == '$' || (ch >= '0' && ch <= '9'))
                            {
                            buf[0] = ch; buf[1] = *(src+1); buf[2] = *(src+2); buf[3] = 0;
                            //if (ch >= '0' && ch <= '9') buf[2] = 0;
                            if ((len = ARG_StrToLong( buf, &res )) > 0) ch = res;
                            }
                        break;
                    }
                src += len;
                }
            ub[ublen++] = ch;
            }
        return( BufResize( ub, ublen ) );
        }
    return(0);
}
