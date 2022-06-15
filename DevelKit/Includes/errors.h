
#define TEXT_ERR_NotFound            5 /* ...Non trouvé */
#define TEXT_PARSE_ERROR            11 /* Erreur */
#define TEXT_PARSE_ERRORBASE        12 /* OK */
#define TEXT_PARSE_LONGLINE         13 /* Ligne d'arguments trop longue */
#define TEXT_PARSE_BADTEMPLATE      14 /* Template non valide */
#define TEXT_PARSE_REQARGMISSING    15 /* Argument obligatoire manquant */
#define TEXT_PARSE_BADLINE          16 /* Ligne d'arguments incorrecte ou trop longue */
#define TEXT_PARSE_KWARGMISSING     17 /* Argument de mot-clé manquant */
#define TEXT_NOMEMORY               18 /* Mémoire insuffisante */
#define TEXT_PARSE_TOOMANYARGS      19 /* Nombre d'arguments trop grand */
#define TEXT_PARSE_BADNUMBER        20 /* Chiffre non valide */
#define TEXT_PARSE_NOCMDLINE        21 /* Pas de ligne d'arguments */
#define TEXT_OBJ_MEMORYOBJ          23 /* Pas assez de mémoire pour l'objet */
#define TEXT_OBJ_NOTFOUND           24 /* Objet non trouvé */
#define TEXT_CMDCANCELLED           29 /* Commande abandonnée */
#define TEXT_ERR_OPENFILE           30 /* Ouverture impossible du fichier */
#define TEXT_ERR_READFILE           31 /* Erreur de lecture du fichier */
#define TEXT_CannotExeCmd           43 /* Création impossible de la commande */
#define TEXT_IllegalFunc            45 /* Commande illégale */
#define TEXT_ExeObj_Macro           57 /* Invalide depuis une macro */
#define TEXT_ExeObj_ARexx           58 /* Invalide depuis ARexx */
#define TEXT_ExeObj_Menu            59 /* Invalide depuis un menu */
#define TEXT_ExeObj_Key             60 /* Invalide depuis une touche */
#define TEXT_ExeObj_Mouse           96 /* Invalide depuis la souris */
#define TEXT_ExeObj_Joystick        97 /* Invalide depuis le joystick */
#define TEXT_ExeObj_Master          61 /* Invalide depuis le maître process */
#define TEXT_ExeObj_HotKey          62 /* Invalide depuis une hotkey */
#define TEXT_ExeObj_Modify          63 /* Le texte ne peut pas être modifié */
#define TEXT_ERR_OPENWRITEFILE      64 /* Ecriture impossible dans le fichier */
#define TEXT_ERR_WRITEFILE          65 /* Erreur d'écriture dans le fichier */
#define TEXT_CmdNotRegistered       66 /* Commande non répertoriée */
#define TEXT_BadPosition            67 /* Position hors limites */
#define TEXT_ERR_OpenConsole        68 /* Ouverture de la console AZur impossible */
#define TEXT_ERR_ConsoleInUse       69 /* Console AZur en cours d'utilisation */
#define TEXT_GermNotFound           70 /* Germe d'objet introuvable */
#define TEXT_GermInUse              71 /* Germe d'objet en cours d'utilisation */
#define TEXT_MaxLinesReached        72 /* Nombre maximum de lignes atteint */
#define TEXT_BadVersion             73 /* Version de l'objet inférieure à celle demandée */
#define TEXT_ERR_BackupFile         74 /* Erreur durant la copie de sauvegarde ".bkp" */
#define TEXT_ERR_BadValue           75 /* Mauvaise valeur donnée à un paramètre */
#define TEXT_ERR_NoRecMacro         77 /* Pas de macro enregistrée */
#define TEXT_ERR_ParmLine           78 /* Erreur dans une ligne du fichier de paramétrage */
#define TEXT_FONTTOOHIGH            95 /* Fonte trop haute pour loger dans les vues */
#define TEXT_PROPFONT               98 /* La fonte pour les vues doit avoir une largeur fixe */
#define TEXT_ERROR_CreateTask      100 /* Erreur lors de la création d'une tâche d'édition */
#define TEXT_NoBlockDefined        101 /* Aucun bloc n'est défini */
#define TEXT_ERR_OpenClip          102 /* Erreur d'ouverture du Clipboard */
#define TEXT_ERR_WriteClip         103 /* Erreur d'écriture dans le Clipboard */
#define TEXT_ERR_ReadClip          104 /* Erreur de lecture du Clipboard */
#define TEXT_ERR_BadClipUnit       105 /* Mauvaise unité spécifiée pour le Clipboard */
#define TEXT_ERR_Nothing2Search    110 /* Pas de chaîne de recherche */
#define TEXT_ERR_CantSendARexx     111 /* Impossible d'envoyer la commande à ARexx */
#define TEXT_ERR_CantSendDOS       112 /* Impossible d'exécuter la commande ou le script DOS */
#define TEXT_ERR_NoConversion      119 /* Aucune conversion demandée */
#define TEXT_Search_NotFound       121 /* Chaîne à chercher non trouvée */
#define TEXT_ERR_FirstCharNotWord  122 /* Le premier caractère à chercher n'est pas à un mot */
#define TEXT_ERR_LastCharNotWord   123 /* Le dernier caractère à chercher n'est pas à un mot */
#define TEXT_ERR_BadHexDigit       129 /* Chiffre hexadécimal invalide */
#define TEXT_ERR_ARexxLocked       150 /* Port ARexx verrouillé */
#define TEXT_ERR_MatchPairs        151 /* Match : Pas de paire ou paire incomplète */
#define TEXT_ERR_TooManyBlocks     152 /* Il faut un seul bloc */
#define TEXT_ERR_DOSOpenConsole    158 /* Ouverture de la console DOS impossible */
#define TEXT_ERR_DOSConsoleInUse   159 /* Console DOS en cours d'utilisation */
#define TEXT_NoBMDefined           257 /* Aucune marque n'est définie */
#define TEXT_ERR_MatchNotFound     260 /* Chaîne complémentaire pas trouvée */
#define TEXT_ERR_RequestInUse      261 /* Requête en cours d'utilisation */
#define TEXT_ERR_MenuKeyTwice      270 /* Raccourcis clavier de menu défini deux fois */
#define TEXT_ERR_SameBkpName       271 /* Pas de backup car même nom */
#define TEXT_ERR_SameAutsName      272 /* Pas de sauvegarde automatique car même nom */
#define TEXT_ERR_InvalidDir        273 /* Répertoire invalide */
#define TEXT_ERR_InvalidPri        275 /* Priorité hors limites : -128 à +127 */
#define TEXT_ERR_MatchQuotes       311 /* Nombre impaire de guillemets sur une ligne du fichier de paramètres */
#define TEXT_ERR_AppendCompress    323 /* Impossible d'écrire à la fin d'un fichier compressé */
