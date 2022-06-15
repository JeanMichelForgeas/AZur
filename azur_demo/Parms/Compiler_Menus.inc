MENUS: APPEND
   MENU "DICE"
    ITEM "Fenêtre    VMake"      "Ctl+V,P"  ExeArexxScript DCC:Rexx/OpenFile.raz
    DIV
    ITEM "Compiler Courant"      "Ctl+V,C"  ExeArexxScript CONSOLE DCC:Rexx/VMake.raz Compile
    ITEM "Construire Projet"     "Ctl+V,B"  ExeArexxScript CONSOLE DCC:Rexx/VMake.raz Build
    ITEM "Construire & Executer" "Ctl+V,R"  ExeArexxScript CONSOLE DCC:Rexx/VMake.raz Run
    ITEM "Construire & Debugger" "Ctl+V,D"  ExeArexxScript CONSOLE DCC:Rexx/VMake.raz Debug
    DIV
    ITEM "Première erreur"       "Ctl+V,F"  ExeArexxScript CONSOLE DCC:Rexx/ErrorMove.raz First
    ITEM "Erreur précédente"                ExeArexxScript CONSOLE DCC:Rexx/ErrorMove.raz Prev
    ITEM "Erreur courante"       "Ctl+V,E"  ExeArexxScript CONSOLE DCC:Rexx/ErrorMove.raz Current
    ITEM "Erreur suivante"       "Ctl+V,N"  ExeArexxScript CONSOLE DCC:Rexx/ErrorMove.raz Next
    ITEM "Nettoier les erreurs"             ExeArexxScript DCC:Rexx/ErrorClear.raz
    BAR
    ITEM "Aide DICEHelp"      "Amiga+Help"  ExeArexxScript DCC:Rexx/DICEHelp.raz SYNC
    DIV
    ITEM "Corriger Index"                   ExeArexxScript CONSOLE ASYNC AZur:Rexx/DICEHelpIndex.raz
#
