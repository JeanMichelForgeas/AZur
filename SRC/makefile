# MACROS
# C Compiler options
# SAS C 5.0 or higher

# -O enlevé le
# _no_optimize.c doit être compilé sans -O car bug optimiseur

FLAGS = -Hram:headers -s -ccqstu -v -d -O
UC = UC:
PGM = EXE:
TOBJ = _test.o eng_actionsa.o
OBJ = arexx.o cmd_curs.o cmd_div.o cmd_exe.o cmd_file.o cmd_prefs.o cmd_req.o cmd_sort.o cmd_text.o cmd_tlgerms.o cmd_tlnames.o cmd_tltable.o cmd_tltemplates.o cmd_win.o \
    datachip.o datalayouts.o datalogos.o datatext.o datautoa.o \
    eng_actions.o eng_actionsa.o eng_obj.o eng_obja.o \
    filedef.o filedef_equal.o filedef_key.o filedef_menu.o filedef_mouse.o \
    funcs_block.o funcs_bookm.o funcs_clip.o funcs_conv.o funcs_curs.o funcs_display.o funcs_div.o funcs_diva.o funcs_fileio.o funcs_fold.o funcs_list.o funcs_macro.o funcs_pointeur.o funcs_prefs.o funcs_reqs.o funcs_search.o funcs_table.o funcs_text.o funcs_text2.o \
    hash.o hasha.o lock.o main.o objfile.o objmenu.o objsplit.o objstat.o objview.o objzone.o seglist.o \
    _asyncio.o _no_optimize.o
XXX = /
PAT = "#?.(c|asm|s|h|i)"

.c.o:
    lc:lc $(FLAGS) $*

.asm.o:
#    genim2 -L -O$*.o -Iincludeg: $*.asm
    uc:assemble -d -iD:Usr/devel/h/ -l -o+ -o=$*.o -p=68000 -u $*.asm

#------------------------------------

fake :  main.c main2.c funcs_table.c cmd_tltable.c $(PGM)AZur $(PGM)Utilities/AZurQuick $(UC)AZPatch

main.c: datatemplate2.c
    Touch main.c

main2.c: datatemplate2.c
    Touch main2.c

funcs_table.c: $(XXX)funcsdef $(XXX)makefuncs.rexx
    rx $(XXX)makefuncs.rexx $(XXX)funcsdef funcs_table.c lci:jump.h ram:xxx lci:funcs.h
    join lci:jump.h ram:xxx to $(PGM)DevelKit/Includes/funcs.h
    delete ram:xxx

cmd_tltable.c: $(XXX)objlist.txt
    rx $(XXX)makeobj.rexx $(XXX)objlist.txt lci:cmd_tlflags.h cmd_tltable.c cmd_tlnames.c cmd_tltemplates.c
    touch cmd_tlgerms.c

# ne pas mettre smalldata car rstartup.o contient des datas qui seraient mergées

$(PGM)AZur: $(OBJ) lib:arg3.lib
    lc:blink with makelink
    c:protect $(PGM)AZur +p

$(PGM)Utilities/AZurQuick: main2.o
    lc:blink LIB:rstartup.o main2.o TO $(PGM)Utilities/AZurQuick LIBRARY LIB:debug.lib LIB:arg3.lib lib:uik.lib lib:lc.lib lib:amiga.lib SC
    c:protect $(PGM)Utilities/AZurQuick +p

$(UC)AZPatch: _patch.o _patcha.o
    lc:blink lib:astartup.obj _patch.o _patcha.o to $(UC)AZPatch lib lib:debug.lib lib:amiga.lib lib:lcnb.lib

$(PGM)test: $(TOBJ)
    lc:blink lib:c.o $(TOBJ) to $(PGM)test lib lib:debug.lib lib:lc.lib lib:amiga.lib
