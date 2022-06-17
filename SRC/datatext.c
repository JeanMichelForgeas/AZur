
extern UBYTE * __far VersionText;


UBYTE * __far LangEnglish[] =
    {
    "",
#ifdef AZUR_DEMO
    "AZur_Demo, Copyright � 1994 Jean-Michel Forgeas\n            All Rights Reserved\n\n                                \nDemonstration program, sale forbidden",
#else
    "AZur, Copyright � 1994 Jean-Michel Forgeas\n            All Rights Reserved\n\n                                \nCommercial program, distribution forbidden",
#endif
    "AZur Help",
    "The wonderfull help on your screen real soon now\n",
    "UnTitled",
    "%ls %ls not found",
    "Open File",
    "Save As File",
    "Backup File",
    "File where to save characters",
    "File where to save Blocks",
    "Parsing error: ",
    "parsing ok",
    "Line too long",
    "Bad template",
    "Required argument missing",
    "Argument line invalid or too long",
    "Argument after '=' or keyword missing",
    "Not enough memory",
    "Too many arguments",
    "Bad number",                                            // 20
    "No command line",
    "Type 'AZur ?' to have the template",
    "Not enough memory for object",
    "Object no found",
    "The contents of this window has been modified.\n\n"
        "Do you want to ",
    "Quit, Save & Quit,\n"
        "or Cancel this request?",
    "open a file?",
    "clear the contents?",
    "Command cancelled",
    "Can't open this file",     // 30
    "Read error with file",
    "_Yes",
    "_No",
    "_Cancel",
    "_Open",
    "_New",
    "_Replace",
    "_Skip",
    "E_xchange",
    "B_y",                      // 40
    "Loading file:\n\"%ls\"",
    "Stop",
    "Cannot create command",
    "Result of",
    "Sorry, I'm illegal",
    "S",    // Sensitive
    "I",    // Insensitive
    "I",    // Insert
    "R",    // Replace
    "M",    // Modified
    " ",    // Unmodified
    "_Absolute",
    "_Relative",
    "_Line",
    "C_olumn",
    "Position",
    "Invalid from a macro",
    "Invalid from ARexx",
    "Invalid from a menu",
    "Invalid from a key",       // 60
    "Invalid from the master process",
    "Invalid from a hot key",
    "Text cannot be modified",
    "Can't open in write mode",
    "Write error with file",
    "Command not registered",
    "Position out of limits",
    "Cannot open AZur console",
    "AZur console in use",
    "Object germ not found",    // 70
    "Object germ in use",
    "Max number of lines reached",
    "Bad version, %ls %ld.%ld asked",
    "Error while saving backup file",
    "Bad value given to a parameter",  // 75
    "Load parameters",
    "Internal macro empty",
    "Error in %ls, line : %.20ls...",
    "Replace",
    "Search",                    // 80
    "_Search",
    "Information",
    "", /* VersionText */
    "AZur",
    "Select a font",
    "Preferences Print",
    "Enter a number",
    "Hex number must begin\nwith 0x, x, or $",
    "_OK",
    "AZur Colors",      // 90
    "Interface Font",
    "View Font",
    "Hex String",
    "Enter a list of hex digits which\nwill be taken as pairs",
    "The font is too high for\nat least one view",
    "Invalid from the mouse",
    "Invalid from the joystick",
    "The font for views must have a fixed width",
    "AZur Screen",
    "Error creating sub task",    // 100
    "No block defined",
    "Cannot open Clipboard",
    "Write error to Clipboard",
    "Read error from Clipboard",
    "Clip unit greater than 255",
    "File to load into the Clipboard",
    "File where to save the Clipboard",
    "Source file to copy from",
    "Dest file to copy to",
    "Nothing to search",          // 110
    "Cannot send command to ARexx",
    "Cannot execute DOS command",
    "Macro file to load",
    "File to save macro",
    "Preferences Window", // 115
    "Preferences Search",
    "Name",
    "\nAn autosave file is newer :\n  %ls%ls  %ls%ls %ls\n  %ls  %ls%ls %ls\n\nDo you prefer to open it ?\n ",
//    "\nAn autosave file \"%ls\" is newer\ndo you prefer to open it ?\n ", // 118
    "Conversion type missing",
    "String found %ld times",     // 120
    "String not found",
    "First char is not a word char",
    "Last char is not a word char",
    "String replaced %ld times",
    "_Quit",
    "_Save",
    "_Cancel",
    "Offset",
    "Bad hexadecimal digit",
    " Area ",                      // 130
    "_Document",
    "_Block",
    "C_ursor",
    "Bac_kward",
    " Extension ",
    "Pattern _?",
    "C_onvert C",
    " Occurences ",
    "A_ll",
    "N_ext",                       // 140
    "S_how",
    " Sensitivity ",
    "C_ase",
    "Accen_ts",
    " Restriction ",
    "Start L _<",
    "End   L _>",
    "Start W _[",
    "End   W _]",
    "ARexx port locked",           //150
    "Match : no pair or incomplete pair",
    "Must handle only one block",
    "ARexx Script to execute",
    "Add possible parameters for the ARexx script",
    "Display mode for the AZur screen",
    "Save Prefs",
    "Load Prefs",
    "Cannot open DOS console",
    "DOS console in use",

    "DOS Script to execute",                        // 160
    "Add possible parameters for the DOS script",
    "Preferences Edit",

    " Type ",
    "_Delete Bloc",
    "_Replace",
    "_Strip EOL",
    "Auto Format",
    "", //_Correct Case",

    " Indent ",
    "_No",
    "_AZur",
    "TT_X",

    " Tabulation ",
    "Ac_tive",
    "Ta_b > Spc",
    "S_pc > Tab",
    "Length",

    " Blinking ",
    "Bl_ink",
    "Time ON",
    "Time OFF",

    " Display ",
    "Show Tab ",
    "a",
    "Show Empty ",
    "b",
    "Show Space ",
    "c",
    "Show EO_L ",
    "d",
    "Show EO_F ",
    "e",
    "N_umber lines",

    " Margins ",
    "L",
    "", // Use
    "R",

    "Colors, Blocks, Folds",

    " Colors ",
    "_Text",
    "_Background",
    "Block T_ext",
    "Block B_g",
    "Cursor Te_xt",
    "Curso_r Bg",
    "C Blo_ck Text",
    "C Bloc_k Bg",

    " Blocks ",
    "B_yte",
    "_Word",
    "_Line",
    "Rect_angle",
    "F_ull Rect",
    "LineBl_ock",
    "_X Start �",
    "_Y Start �",

    //------ Env1
    "Preferences Environment",      // 217
    "ARe_xx & Files",

    " Save ",
    "P_urge Spc",                    // 220
    "_LF > CR",
    "L_F > CRLF",
    "_Icon",
    "_Tool",

    " Load ",
    "_Purge Spc",
    "_CR > LF",
    "Ic_on",

    " Backup ",
    "_Active",                       // 230
    "",
    "Rotations",
    "Do not upper case port name",
    "Dir",

    " Mode ",
    "_Minitel",
    "R_ead only",
    "Priorit_y",
    "DelClip",

    " Sizes ",                      // 240
    "Text Block (KB)",
    "I/O Buffer (KB)",
    "Command Lines",
    "",
    "Undo Buffer (KB)",

    //------ Env2
    " ARexx Ports ",
    "_Character for Separation: ",
    "",
    "_Number First Port",
    "ARexx Port for _Master task",            // 250
    "ARexx Port for _Editing task",

    " Files ",
    "_Selection pattern",
    "_Parameters file",
    "_ARexx Console",
    "_DOS Console",

    "No bookmark defined",
    "Bookmark Name",
    "Bookmarks",
    "Match string not found",                 // 260
    "Request currently in use",
    "The file \"%ls\" is read OK.\nThe autosave file \"%ls\"\nis perhaps useless,\n\n����Do you want to delete it ?",

    " Autosave ",
    "Active",
    "Name",
    "Dir",
    "Seconds",
    "Modifications",
    "Rotations",
    "Menu Shortcut '%ls' defined %ld times",  // 270
    "No backup because of same name",
    "No autosave because of same name",
    "Invalid directory",
    "Select a directory",
    "Priority out of limits : -128 to +127",
    "Set priority",
    "Current priority is %ld\nEnter a value from -128 to +127",
    " Folds ",
    "Text color",
    "Bg color",                               // 280
    "Underlined",
    "Bold",
    "Italic",
    "Characters: %ld,   Words: %ld\nCharacters into words: %ld, by word : %ld\n\nCurrent line: %ld\nTotal lines: %ld,   Lines not empty: %ld\nVisibles lines: %ld,   Folded lines: %ld\n\nFolds: %ld,   Blocks: %ld,    Bookmarks: %ld\nMax line width: %ld,   Average line width: %ld\nAverage width for not empty lines: %ld\n\nPublic screen name: %ls\nARexx port name: %ls\nARexx Master port name: %ls",
    "Information",

    " Fonts ",
    "Interface",
    "Views",
    " Open ",
    "Front screen",
    "AZur screen",
    "Full screen",
    "X",
    "Y",
    "Width",
    "Height",
    " Window ",
    "Status bar",
    "Command line",
    " Bottom border ",
    "Stop",
    "Arrows",
    "Prop",
    " Right border ",
    "Colors Palette",
    "Colors",  /* palette */
    "R",
    "G",
    "B",
    "_Restore",
    "Odd number off \" or ' in line: %ls",
    "The file is newer than its icon. Do you want to use\nthe icon information?\n(information about folds, paragraphs, etc...)",
    "Do you want to delete this old icon?",
    "The default tool of the icon is unknown:\n    Icon tool: \"%ls\"\n    AZur tool: \"%ls\"\nDoes it have to be replaced by the AZur one?",
    "Do you confirm the automatic backup for:\n    '%ls'",
    "F",
    "-",
    "D",
    "-",
    "Enter password to decrunch\nthis PowerPacked file:",
    "Error while loading compressed file:\n    %ls\nDo you want to load it the normal way?",
    "Error while saving or compressing file:\n    %ls\nDo you want to save it the normal way?",
    "Impossible to append data to a file\nwhen in compress mode",
    "A file '%ls' already exists.\nDo you want to continue and overwrite it?",
    "The file has no icon yet.\nDo you want to create one?",
    "",
    0
    };

//---------------------------------------------------------------------------------------

UBYTE * __far LangOther[] =
    {
    "",
#ifdef AZUR_DEMO
    "AZur_Demo, Copyright � 1994 Jean-Michel Forgeas\n           Tous Droits R�serv�s\n\n                                \nProgramme de d�monstration, vente interdite",
#else
    "AZur, Copyright � 1994 Jean-Michel Forgeas  \n           Tous Droits R�serv�s\n\n                                \nProgramme commercial, distribution interdite",
#endif
    "AZur Aide",
    "La merveilleuse aide de AZur tr�s bient�t sur vos �crans!\n",
    "SansTitre",
    "%ls %ls non trouv�",
    "Ouverture de fichier",
    "Sauvegarde dans fichier",


    "Fichier de backup",
    "Fichier o� sauver les caract�res",
    "Fichier o� sauver les blocs",  // 10
    "Erreur : ",
    "OK",
    "Ligne d'arguments trop longue",
    "Template non valide",
    "Argument obligatoire manquant",


    "Ligne d'arguments incorrecte ou trop longue",
    "Argument de mot-cl� manquant",
    "M�moire insuffisante",
    "Nombre d'arguments incorrect",
    "Chiffre non valide",             // 20
    "Pas de ligne d'arguments",
    "Tapez 'AZur ?' pour avoir le mod�le",
    "Pas assez de m�moire pour l'objet",


    "Objet non trouv�",
    "Le contenu de cette fen�tre a �t� modifi�.\n\n"
        "Voulez-vous ",
    "Quitter, Sauver & Quitter,\n"
        "ou Annuler cette requ�te?",

    "ouvrir un fichier?",
    "effacer son contenu?",
    "Commande abandonn�e",
    "Ouverture impossible du fichier",  // 30
    "Erreur de lecture du fichier",


    "_Oui",
    "_Non",
    "A_nnuler",
    "_Ouvrir",
    "Nou_veau",
    "_Remplacer",
    "_Sauter",
    "_Changer",


    "_Par",             // 40
    "Chargement du fichier :\n\"%ls\"",
    "Arr�ter",
    "Cr�ation impossible de la commande",
    "R�sultat de",
    "D�sol�e, je suis ill�gale",
    "S",    // Sensitive
    "I",    // Insensitive


    "I",    // Insert
    "R",    // Replace
    "M",    // Modified      // 50
    " ",    // Unmodified
    "_Absolu",
    "_Relatif",
    "_Ligne",
    "_Colonne",


    "Position",
    "Invalide depuis une macro",
    "Invalide depuis ARexx",
    "Invalide depuis un menu",
    "Invalide depuis une touche",   // 60
    "Invalide depuis le ma�tre process",
    "Invalide depuis un raccourci clavier",
    "Le texte ne peut pas �tre modifi�",


    "Ouverture impossible du fichier en �criture",
    "Erreur d'�criture dans le fichier",
    "Commande non r�pertori�e",
    "Position hors limites",
    "Ouverture de la console AZur impossible",
    "Console AZur en cours d'utilisation",
    "Germe d'objet introuvable",    // 70
    "Germe d'objet en cours d'utilisation",


    "Nombre maximum de lignes atteint",
    "Mauvaise version, %ls %ld.%ld demand�e",
    "Erreur durant la copie de sauvegarde",
    "Mauvaise valeur donn�e � un param�tre", // 75
    "Chargement des param�tres",
    "Macro interne vide",
    "Erreur dans %ls, ligne : %.20ls...",
    "Remplacer",


    "Chercher",                     // 80
    "_Chercher",
    "Information",
    "", /* VersionText */
    "AZur",
    "Choix d'une police",
    "Pr�f�rences : Impression",
    "Entrez un nombre",


    "Un nombre h�xa doit commencer\npar 0x, x, ou $",
    "_OK",
    "Couleurs AZur",      // 90
    "Police d'interface",
    "Police du texte",
    "Cha�ne h�xa",
    "Entrez une liste de chiffres h�xa qui\nseront pris par paires",
    "La fonte est trop haute pour\nau moins une vue",


    "Invalide depuis la souris",
    "Invalide depuis le joystick",
    "La fonte pour les vues doit avoir une largeur fixe",
    "Ecran AZur",
    "Erreur � la cr�ation de la t�che fille",   // 100
    "Aucun bloc n'est d�fini",
    "Ouverture Clipboard impossible",
    "Erreur d'�criture dans le Clipboard",


    "Erreur de lecture du Clipboard",
    "Num�ro de Clipboard sup�rieur � 255",
    "Fichier � charger dans le Clipboard",
    "fichier o� sauver le Clipboard",
    "Fichier source � copier",
    "Fichier de destination",
    "Pas de cha�ne � chercher",              // 110
    "Impossible d'envoyer la commande � ARexx",


    "Impossible d'ex�cuter la commande ou le script DOS",
    "Fichier Macro � charger",
    "Fichier o� sauver la Macro",
    "Pr�f�rences : Fen�tres", // 115
    "Pr�f�rences : Recherche",
    "Nom",
    "\nUn fichier autosauve est plus r�cent :\n  %ls%ls  %ls%ls %ls\n  %ls  %ls%ls %ls\n\nPr�f�rez-vous l'ouvrir ?\n ",
//    "\nUn fichier autosauve \"%ls\" est plus\nr�cent : pr�f�rez-vous l'ouvrir ?\n ", // 118
    "Type de conversion manquant",


    "Cha�ne trouv�e %ld fois",          // 120
    "Cha�ne introuvable",
    "Premier caract�re pas dans un mot",
    "Dernier caract�re pas dans un mot",
    "Cha�ne remplac�e %ld fois",
    "_Quitter",
    "_Sauver",
    "_Annuler",

    "Offset",
    "Mauvais chiffre hexad�cimal",
    " Aire ",                         // 130
    "_Document",
    "_Bloc",
    "C_urseur",
    "_Arri�re",
    " Extension ",

    "Jo_ker ?",
    "C_onvert C",
    " Occurences ",
    "_Toutes",
    "Proc_haine",                    // 140
    "_Montrer",
    " Sensibilit� ",
    "Ma_j/min",

    "Acc_ents",
    " Restriction ",
    "D�b lig _<",
    "Fin lig _>",
    "D�b mot _[",
    "Fin mot _]",
    "Port ARexx verrouill�",         // 150
    "Match : Pas de paire ou paire incompl�te",
    "Il ne faut qu'un seul bloc",
    "Script ARexx � ex�cuter",
    "Ajouter des param�tres �ventuels pour le script ARexx",
    "Mode d'affichage pour l'�cran AZur",
    "Sauver Prefs",
    "Charger Prefs",
    "Ouverture de la console DOS impossible",
    "Console DOS en cours d'utilisation",

    "Script DOS � ex�cuter",                                // 160
    "Ajouter des param�tres �ventuels pour le script DOS",
    "Pr�f�rences : Edition",

    " Ecriture ",
    "S_upprime bloc",
    "_Remplace",
    "_Purger EOL",
    "Auto format",
    "", //_Corrige Mm",

    " Indentation ",
    "Aucu_ne",               // 170
    "_AZur",
    "TT_X",

    " Tabulation ",
    "Ac_tive",
    "Ta_b > Esp",
    "E_sp > Tab",
    "Lon_gueur",

    " Clignotement ",
    "Cl_ignoter",
    "Dur�e plein",          // 180
    "Dur�e vide",

    " Affichage ",
    "Montrer Tab ",
    "a",
    "Montrer Vide ",
    "b",
    "Montrer Esp ",
    "c",
    "Montrer EO_L ",
    "d",                    // 190
    "Montrer EO_F ",
    "e",
    "Num_�roter lignes",

    " Marges ",
    "G",
    "",  // utilise
    "D",

    "C_ouleurs, Blocs, Plis",

    " Couleurs ",
    "_Texte",                // 200
    "_Fond",
    "T_exte bloc",
    "F_ond bloc",
    "Curseur te_xte",
    "Curseur fo_nd",
    "C texte bloc",
    "C fon_d bloc",

    " Blocs ",
    "_Caract�re",
    "_Mot",                  // 210
    "_Ligne",
    "_Rectangle",
    "Rect _Plein",
    "_Bloc ligne",
    "D�but _X �",
    "D�but _Y �",

    //------ Env1
    "Pr�f�rences : Environnement",  // 217
    "ARe_xx & Fichiers",

    " Sauvegarde ",
    "P_urge Esp",                    // 220
    "_LF > CR",
    "L_F > CRLF",
    "_Ic�ne",
    "_Outil",

    " Chargement ",
    "_Purge Esp",
    "_CR > LF",
    "Ic�n_e",

    " Backup ",
    "_Actif",                        // 230
    "",
    "Rotations",
    "Laisse les minuscules du nom du port",
    "R�p",

    " Mode ",
    "Mi_nitel",
    "_Lecture",
    "Priorit�",
    "Clip d'effacement",

    " Tailles ",                    // 240
    "Bloc de texte (KO)",
    "Bloc d'E/S (KO)",
    "Lignes de commandes",
    "",
    "Bloc pour Undo (KO)",

    //------ Env2
    " Ports ARexx ",
    "_Caract�re de s�paration : ",
    "",
    "_Num�rote le premier",
    "Port ARexx de la t�che _Ma�tresse",            // 250
    "Port ARexx de la t�che d'_Edition",

    " Fichiers ",
    "Motif de _S�lection",
    "Fichier de _Param�tres",
    "Console _ARexx",
    "Console _DOS",

    "Aucune marque n'est d�finie",
    "Nom de la marque",
    "Marques",
    "Cha�ne compl�mentaire pas trouv�e",            // 260
    "Requ�te en cours d'utilisation",
    "Le fichier \"%ls\" est bien lu.\nLe fichier autosauve \"%ls\"\nest peut-�tre inutile,\n\n����Voulez-vous l'effacer ?",

    " Autosauve ",
    "Actif",
    "Nom",
    "R�p",
    "Secondes",
    "Modifications",
    "Rotations",
    "Raccourcis de menu '%ls' d�fini %ld fois",    // 270
    "Pas de backup car m�me nom",
    "Pas de sauvegarde automatique car m�me nom",
    "R�pertoire invalide",
    "S�lectionner un r�pertoire",
    "Priorit� hors limites : -128 � +127",
    "Priorit�",
    "La priorit� actuelle est %ld\nEntrez une valeur de -128 � +127",
    " Plis ",
    "Coul Texte",
    "Coul Fond",                                   // 280
    "Soulign�",
    "Gras",
    "Italique",
    "Caract�res : %ld,   Mots : %ld\nCaract�res dans les mots : %ld, par mot : %ld\n\nLigne courante : %ld\nTotal lignes : %ld,   Lignes non vides : %ld\nLignes visibles : %ld,   Lignes repli�es : %ld\n\nPlis : %ld,   Blocs : %ld,    Marques : %ld\nLargeur max : %ld,   Largeur moyenne : %ld\nLargeur moyenne des lignes non vides : %ld\n\nNom de l'�cran public : %ls\nNom du port ARexx : %ls\nNom du port ARexx ma�tre : %ls",
    "Information",

    " Police de caract�res ",
    "Interface",
    "Vues",
    " Ouverture ",
    "Ecran de devant",
    "Ecran AZur",
    "Plein �cran",
    "X",
    "Y",
    "Largeur",
    "Hauteur",
    " Fen�tre ",
    "Barre de status",
    "Ligne de commande",
    " Bord horizontal ",
    "But�e",
    "Fl�ches",
    "Prop",
    " Vertical ",
    "Palette de couleurs",
    "Couleurs",   /* palette */
    "R",
    "V",
    "B",
    "_Remettre",
    "Nombre impair de \" ou ' dans la ligne : %ls",
    "Le fichier est plus r�cent que son ic�ne.\nVoulez-vous utiliser les informations de l'ic�ne ?\n(informations sur les plis, paragraphes, etc)",
    "Voulez-vous effacer cette vielle ic�ne ?",
    "L'outil par d�faut de l'ic�ne est inconnu :\n    Ic�ne : \"%ls\"\n    AZur  : \"%ls\"\nDoit-il �tre remplac� par celui de AZur ?",
    "Confirmez-vous la sauvegarde automatique pour :\n    '%ls'",
    "F",
    "-",
    "D",
    "-",
    "Entrez le mot de passe pour d�compacter\nce fichier PowerPack� :",
    "Erreur durant le chargement du fichier compress� :\n    %ls\nVoulez-vous essayer de le charger normalement ?",
    "Erreur durant la sauvegarde/compression du fichier :\n    %ls\nVoulez-vous essayer de le sauvegarder normalement ?",
    "Impossible d'�crire � la fin d'un fichier\nen mode compression",
    "Un fichier '%ls' existe.\nVoulez-vous continuer et le remplacer ?",
    "Le fichier n'a pas encore d'ic�ne.\nVoulez-vous en cr�er un ?",
    "",
    };
