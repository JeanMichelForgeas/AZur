# MACROS
# C Compiler options
# SAS C 5.0 or higher

FLAGS = -s -ccqstu -v -d
PGM = AZur:Objects/

.c.o:
    lc $(FLAGS) $*

.asm.o:
    genim2 -L -O$*.o -Iincludeg: $*.asm

#------------------------------------

fake: cmdheader.obj $(PGM)AZTest $(PGM)AZTestReq $(PGM)AZTestVar $(PGM)AZColNum $(PGM)AZColDel $(PGM)AZColIns $(PGM)AZAccent $(PGM)AZBeKey $(PGM)AZCharTable $(PGM)AZFileProt

cmdheader.obj: cmdheader.asm
    genim2 -L -Ocmdheader.obj -Iincludeg: cmdheader.asm

$(PGM)AZTest: cmdheader.obj aztest.o
    blink cmdheader.obj aztest.o to $(PGM)AZTest lib lib:debug.lib lib:amiga.lib lib:lcnb.lib SD ND

$(PGM)AZTestReq: cmdheader.obj aztestreq.o
    blink cmdheader.obj aztestreq.o to $(PGM)AZTestReq lib lib:debug.lib lib:amiga.lib lib:lcnb.lib SD ND

$(PGM)AZTestVar: cmdheader.obj aztestvar.o
    blink cmdheader.obj aztestvar.o to $(PGM)AZTestVar lib lib:debug.lib lib:amiga.lib lib:lcnb.lib SD ND

$(PGM)AZColNum: cmdheader.obj azcolnum.o
    blink cmdheader.obj azcolnum.o to $(PGM)AZColNum lib lib:debug.lib lib:amiga.lib lib:lcnb.lib SD ND

$(PGM)AZColDel: cmdheader.obj azcoldel.o
    blink cmdheader.obj azcoldel.o to $(PGM)AZColDel lib lib:debug.lib lib:amiga.lib lib:lcnb.lib SD ND

$(PGM)AZColIns: cmdheader.obj azcolins.o
    blink cmdheader.obj azcolins.o to $(PGM)AZColIns lib lib:debug.lib lib:amiga.lib lib:lcnb.lib SD ND

$(PGM)AZAccent: cmdheader.obj azaccent.o
    blink cmdheader.obj azaccent.o to $(PGM)AZAccent lib lib:debug.lib lib:amiga.lib lib:lcnb.lib SD ND

$(PGM)AZBeKey: cmdheader.obj azbekey.o
    blink cmdheader.obj azbekey.o to $(PGM)AZBeKey lib lib:debug.lib lib:amiga.lib lib:lcnb.lib SD ND

$(PGM)AZCharTable: cmdheader.obj azchartable.o
    blink cmdheader.obj azchartable.o to $(PGM)AZCharTable lib lib:debug.lib lib:uik.lib lib:amiga.lib lib:lcnb.lib SD ND

$(PGM)AZFileProt: cmdheader.obj azfileprot.o
    blink cmdheader.obj azfileprot.o to $(PGM)AZFileProt lib lib:debug.lib lib:uik.lib lib:amiga.lib lib:lcnb.lib SD ND
