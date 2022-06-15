# MACROS
# C Compiler options
# SAS C 5.0 or higher

FLAGS = -s -ccqstu -v -d -O -b0
# !!!!!!!! Enlever le -b0 pour AZMan !!!!!!!!!!!!!

PGM = AZur:Objects/
UTPGM = AZur:Utilities/

.c.o:
    lc $(FLAGS) $*

.asm.o:
    genim2 -L -O$*.o -Iincludeg: $*.asm

.asm.obj:
    genim2 -L -O$*.obj -Iincludeg: $*.asm

#--------------------------------------------------

fake: $(PGM)AZMan $(UTPGM)AZMan_MakeIndex

$(PGM)AZMan: cmdheader.obj AZMan.o
    blink cmdheader.obj AZMan.o to $(PGM)AZMan lib lib:debug.lib lib:uik.lib lib:amiga.lib lib:lc.lib SD ND

$(UTPGM)AZMan_MakeIndex: MakeIndex.o env.o
    blink lib:astartup.obj MakeIndex.o env.o to $(UTPGM)AZMan_MakeIndex lib lib:uik.lib lib:debug.lib lib:amiga.lib lib:lcnb.lib SC SD ND
