# MACROS
# C Compiler options
# SAS C 5.0 or higher

FLAGS = -s -ccqstu -v -d -O

PGM = AZur:Objects/

.c.o:
    lc $(FLAGS) $*

.asm.o:
    genim2 -L -O$*.o -Iincludeg: $*.asm

#--------------------------------------------------

fake: $(PGM)AZDic  $(PGM)AZDicTrans

$(PGM)AZDic: cmdheader.o azdic.o
    blink cmdheader.o azdic.o to $(PGM)AZDic lib lib:debug.lib lib:uik.lib lib:amiga.lib lib:lc.lib SD ND

$(PGM)AZDicTrans: cmdheader.o azdictrans.o
    blink cmdheader.o azdictrans.o to $(PGM)AZDicTrans lib lib:debug.lib lib:uik.lib lib:amiga.lib lib:lc.lib SD ND
