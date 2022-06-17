Path: cbmfra!cbmehq!cbmvax!carolyn
From: carolyn@cbmvax.cbm.commodore.com (Carolyn Scheppner - CATS)
Newsgroups: adsp.sw
Subject: Re: Iconstructs
Message-ID: <CKvnKz.Mwy@cbmvax.cbm.commodore.com>
Date: 7 Feb 94 23:22:10 GMT
References: <naasc.04f9@cnaas.adsp.sub.org>
Distribution: adsp
Organization: Commodore, West Chester, PA
Lines: 100

In article <naasc.04f9@cnaas.adsp.sub.org>,
Christopher Naas <cbmehq!cbmnor!cnaas!naasc@cbmvax.commodore.com> wrote:
>
>Exactly what parts of the iconstruct do I need to fill in order
>to get a savable icon?

Don't know.  I always fill them in pretty well.  I know you do not
need ToolWindow - it does nothing.  For a Tool icon, 0 for stack value
will probably get you default stack (untested).

Here is a sample of what I use.  This might be more comlex than
you need - it has a Default Tool, and has a ToolType array.
You could pass NULL for both of those if you like.

BTW - I probably generated this with IconEdit - but I can no longer
figure out the trick to get IconEdit save C source code.  Anybody
know the trick with V40 iconEdit ?  David ?

-------
/* Data for project icon for saved ILBM
 *
 */
UWORD ILBMI1Data[] =
{
/* Plane 0 */
    0x0000,0x0000,0x0000,0x0001,0x0000,0x0000,0x0000,0x0003,
    0x0FFF,0xFFFF,0xFFFF,0xFFF3,0x0FFF,0x0000,0x0000,0xFFF3,
    0x0FFC,0x0000,0x0000,0x3FF3,0x0FE0,0x0E80,0xF800,0x07F3,
    0x0F80,0x1C01,0x8C00,0x01F3,0x0F00,0x0001,0x8C00,0x00F3,
    0x0600,0x0000,0x0600,0x0063,0x0600,0x0003,0xBC00,0x0063,
    0x0600,0x0001,0xFC00,0x0063,0x0600,0x0000,0xFC00,0x0063,
    0x0600,0x1FC1,0xFE40,0x0063,0x0600,0x1DC1,0xFE20,0x0063,
    0x0600,0x1CE3,0xFF12,0x0063,0x0F00,0x1CE0,0x004F,0xC0F3,
    0x0F80,0x1CE0,0x002F,0x01F3,0x0FE0,0x0E78,0x423D,0x07F3,
    0x0FFC,0x0000,0x0000,0x3FF3,0x0FFF,0x0000,0x0000,0xFFF3,
    0x0FFF,0xFFFF,0xFFFF,0xFFF3,0x0000,0x0000,0x0000,0x0003,
    0x7FFF,0xFFFF,0xFFFF,0xFFFF,
/* Plane 1 */
    0xFFFF,0xFFFF,0xFFFF,0xFFFE,0xD555,0x5555,0x5555,0x5554,
    0xD000,0x0000,0x0000,0x0004,0xD3FC,0xFFFF,0xFFFF,0x3FC4,
    0xD3C0,0x0000,0x0000,0x03C4,0xD300,0x0100,0xF800,0x00C4,
    0xD300,0x0381,0xFC00,0x00C4,0xD080,0x0701,0xFC00,0x0104,
    0xD180,0xF883,0xFE00,0x0194,0xD181,0xDF80,0x4700,0x0194,
    0xD181,0xDF82,0x0180,0x0194,0xD180,0x6F82,0x00C0,0x0194,
    0xD180,0x0002,0x0020,0x0194,0xD180,0x0000,0x0000,0x0194,
    0xD180,0x0000,0x0002,0x0194,0xD080,0x0000,0xCC46,0xC104,
    0xD300,0x0000,0xCC2F,0x00C4,0xD300,0x0E78,0x883D,0x00C4,
    0xD3C0,0x0000,0x0000,0x03C4,0xD3FC,0xFFFF,0xFFFF,0x3FC4,
    0xD000,0x0000,0x0000,0x0004,0xD555,0x5555,0x5555,0x5554,
    0x8000,0x0000,0x0000,0x0000,
};

struct Image ILBMI1 =
{
    0, 0,			/* Upper left corner */
    64, 23, 2,			/* Width, Height, Depth */
    ILBMI1Data,			/* Image data */
    0x0003, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* Next image */
};

UBYTE *ILBMTools[] =
{
    "FILETYPE=ILBM",
    NULL
};

struct DiskObject ILBMobject =
{
    WB_DISKMAGIC,		/* Magic Number */
    WB_DISKVERSION,		/* Version */
    {				/* Embedded Gadget Structure */
	NULL,			/* Next Gadget Pointer */
	0, 0, 64, 24,		/* Left,Top,Width,Height */
	GFLG_GADGIMAGE | GADGBACKFILL,	        /* Flags */
	GACT_RELVERIFY | GACT_IMMEDIATE,	/* Activation Flags */
	GTYP_BOOLGADGET,		        /* Gadget Type */
	(APTR)&ILBMI1,	/* Render Image */
	NULL,			/* Select Image */
	NULL,			/* Gadget Text */
	NULL,			/* Mutual Exclude */
	NULL,			/* Special Info */
	0,			/* Gadget ID */
	NULL,			/* User Data */
    },
    WBPROJECT,			/* Icon Type */
    "Display",			/* Default Tool */
    ILBMTools,			/* Tool Type Array */
    NO_ICON_POSITION,		/* Current X */
    NO_ICON_POSITION,		/* Current Y */
    NULL,			/* Drawer Structure */
    NULL,			/* Tool Window */
    0				/* Stack Size */
};

-- 
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  /\/\   /\/\   Carolyn Scheppner - Technical Manager - CATS U.S.
 ( ^^ ) ( -- )  CATS - Commodore Applications and Technical Support 
  `--'   `--'   carolyn@cbmvax.cbm.commodore.com     BIX:cscheppner
