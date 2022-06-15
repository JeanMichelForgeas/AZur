# MACROS
# C Compiler options
# SAS C 5.0 or higher

FLAGS = -s -ccqstu -v -d -O -b0

PGM = AZur:Utilities/

.c.o:
    lc $(FLAGS) $*

.asm.o:
    genim2 -L -O$*.o -Iincludeg: $*.asm

#--------------------------------------------------

fake: $(PGM)DicServ

$(PGM)DicServ: main.o direct.o dic.o dica.o search_fr.o
    blink astartup.obj main.o direct.o dic.o dica.o search_fr.o to $(PGM)DicServ lib lib:debug.lib lib:uikalone.lib lib:amiga.lib lib:lcnb.lib SD
