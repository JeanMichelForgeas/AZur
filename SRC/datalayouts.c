
#include "uiki:objchecktoggle.h"
#include "uiki:objradioexclude.h"

#include "lci:azur.h"

//--------------------------------------------- Search Replace

// extern void set_search_check();
// extern void uns_search_radio();

extern void set_search_radio();
extern void set_backw_radio();
extern void show_edit1char();
extern void verif_maxval();
extern void verif_maxval_signed();
extern void endisable_bkpname();
extern void endisable_autosauve();
extern void endisable_backup();
extern void Env1_SaveLFCR();
extern void Env1_SaveLFCRLF();
extern void check_left_margin();
extern void check_right_margin();
extern void inact_horiz_prop();
extern void inact_vert_prop();

struct AZLayout __far AZL_Search[] =
    {
    { 1, AZLID_FRAME,                   0, TEXT_TitleSearch_Aire,     0,0,  0, 0, 0,0,                0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Document, 0,0, 26,11, 0,0,                0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Bloc,     0,0, 26,11, 0,0, set_search_radio, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Curseur,  0,0, 26,11, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Arriere,  0,0, 26,12, 0,0,                0, 0, 0 },

    { 1, AZLID_FRAME,                   0, TEXT_TitleSearch_Exten,    0,0,  0, 0, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Pattern,  0,0, 26,12, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Convert,  0,0, 26,12, 0,0,                0, 0, 0 },

    { 1, AZLID_FRAME,                   0, TEXT_TitleSearch_Occur,    0,0,  0, 0, 0,0,                0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Toutes,   0,0, 26,11, 0,0,  set_backw_radio, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Prochaine,0,0, 26,11, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Montrer,  0,0, 26,12, 0,0,                0, 0, 0 },

    { 1, AZLID_FRAME,                   0, TEXT_TitleSearch_Sensi,    0,0,  0, 0, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_MajMin,   0,0, 26,12, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_Accents,  0,0, 26,12, 0,0,                0, 0, 0 },

    { 1, AZLID_FRAME,                   0, TEXT_TitleSearch_Restri,   0,0,  0, 0, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_DebLig,   0,0, 26,12, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_FinLig,   0,0, 26,12, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_DebMot,   0,0, 26,12, 0,0,                0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT, TEXT_TitleSearch_FinMot,   0,0, 26,12, 0,0,                0, 0, 0 },

    { 0,0,0,0,0,0,0,0,0,0 }
    };

//--------------------------------------------- Prefs Edit

struct AZLayout __far AZL_PrefsEdit1[] =
    {
    { 1, AZLID_FRAME   ,                0                                 , TEXT_Edit1_Ecriture    ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_Effacebloc  ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_Remplace    ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_PurgerEOL   ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_Motentier   ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_CorrigeMm   ,0,0, 26,12, 0,0, 0, 0, 0 },

    { 1, AZLID_FRAME   ,                0                                 , TEXT_Edit1_Indentation ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_Aucune      ,0,0, 26,11, 0,0, 0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_Normale     ,0,0, 26,11, 0,0, 0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_TTX         ,0,0, 26,11, 0,0, 0, 0, 0 },

    { 1, AZLID_FRAME   ,                0                                 , TEXT_Edit1_Tabulation  ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_Active      ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_TabEsp      ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_EspTab      ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT                , TEXT_Edit1_Longueur    ,0,0,  0, 0, 0,0, verif_maxval, 3, W2L(3,255) }, /* chars,max */

    { 1, AZLID_FRAME   ,                0                                 , TEXT_Edit1_Clignotement,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_Clignoter   ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT                , TEXT_Edit1_Dureeplein  ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) }, /* chars,max */
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT                , TEXT_Edit1_Dureevide   ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) }, /* chars,max */

    { 1, AZLID_FRAME   ,                0                                 , TEXT_Edit1_Affichage   ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_MontrerTab  ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT | AZLF_SAMELINE, TEXT_Edit1_CharTab     ,0,0,  0, 0, 0,0, show_edit1char, 0, W2L(3,255) }, /* chars,max */
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_MontrerVide ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT | AZLF_SAMELINE, TEXT_Edit1_CharVide    ,0,0,  0, 0, 0,0, show_edit1char, 0, W2L(3,255) }, /* chars,max */
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_MontrerEsp  ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT | AZLF_SAMELINE, TEXT_Edit1_CharEsp     ,0,0,  0, 0, 0,0, show_edit1char, 0, W2L(3,255) }, /* chars,max */
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_MontrerEOL  ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT | AZLF_SAMELINE, TEXT_Edit1_CharEOL     ,0,0,  0, 0, 0,0, show_edit1char, 0, W2L(3,255) }, /* chars,max */
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_MontrerEOF  ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT | AZLF_SAMELINE, TEXT_Edit1_CharEOF     ,0,0,  0, 0, 0,0, show_edit1char, 0, W2L(3,255) }, /* chars,max */
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Edit1_Numerote    ,0,0, 26,12, 0,0, 0, 0, 0 },

    { 1, AZLID_FRAME   ,                0                                 , TEXT_Edit1_Marges      ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT                 | AZLF_SUB_EQUAL, TEXT_Edit1_Gauche      ,0,0,  0, 0, 0,0, check_left_margin, 0, W2L(5,0xffff) }, /* chars,max */
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT | AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Edit1_Utilise     ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Edit1_Droite      ,0,0,  0, 0, 0,0, check_right_margin, 0, W2L(5,0xffff) }, /* chars,max */

    { 0,0,0,0,0,0,0,0,0,0 }
    };

struct AZLayout __far AZL_PrefsEdit2[] =
    {
    { 1, AZLID_FRAME   ,                0 , TEXT_Edit2_Couleurs     ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT , TEXT_Edit2_Texte        ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT , TEXT_Edit2_Fond         ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT , TEXT_Edit2_Textebloc    ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT , TEXT_Edit2_Fondbloc     ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT , TEXT_Edit2_Curseurtexte ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT , TEXT_Edit2_Curseurfond  ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT , TEXT_Edit2_Ctextebloc   ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT , TEXT_Edit2_Cfondbloc    ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },

    { 1, AZLID_FRAME   ,                0                 , TEXT_Edit2_Blocs     ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT                 , TEXT_Edit2_Caractere ,0,0, 26,11, 0,0, 0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT                 , TEXT_Edit2_Mot       ,0,0, 26,11, 0,0, 0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT                 , TEXT_Edit2_Ligne     ,0,0, 26,11, 0,0, 0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT                 , TEXT_Edit2_Rectangle ,0,0, 26,11, 0,0, 0, 0, 0 },
    { 2, AZLID_GADRADIO, AZLF_TITLE_RIGHT                 , TEXT_Edit2_Rectplein ,0,0, 26,11, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT, TEXT_Edit2_LimiteX   ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(1,9) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT, TEXT_Edit2_LimiteY   ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(1,9) },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                 , TEXT_Edit2_Blocligne ,0,0, 26,12, 0,0, 0, 0, 0 },

    { 1, AZLID_FRAME   ,                0 , TEXT_Edit2_Plis         ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT, TEXT_Edit2_PliTexte ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT, TEXT_Edit2_PliFond  ,0,0,  0, 0, 0,0, verif_maxval, 0, W2L(3,255) },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT , TEXT_Edit2_PliSouligne  ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT , TEXT_Edit2_PliGras      ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT , TEXT_Edit2_PliItalique  ,0,0, 26,12, 0,0, 0, 0, 0 },

    { 0,0,0,0,0,0,0,0,0,0 }
    };

//--------------------------------------------- Prefs Environnement

struct AZLayout __far AZL_PrefsEnv1[] =
    {
  /*---------------
    [Chargement] 0
      V Purge Esp   Flags2  AZP_STRIPLOAD   /* Enlève les espaces de fin de ligne lors du chargement */
      V CR > LF     Flags2  AZP_LOADCR2LF   /*  */
      V Icône       Flags2  AZP_LOADICON    /*  */
  */
    { 1, AZLID_FRAME   ,                0                 , TEXT_Env1__Chargement ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                 , TEXT_Env1_CPurgeEsp   ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                 , TEXT_Env1_CCRLF       ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                 , TEXT_Env1_CIcone      ,0,0, 26,12, 0,0, 0, 0, 0 },
  /*--------------
    [Sauvegarde] 4
      V Purge Esp   Flags2  AZP_STRIPSAVE   /* Enlève les espaces de fin de ligne lors de la sauvegarde */
      V LF > CR     Flags2  AZP_SAVELF2CR   /*  */
      V LF > CRLF   Flags2  AZP_SAVELF2CRLF /*  */
      V Icône       Flags1  AZP_SAVEICON    /* Crée une icône après avoir sauvegardé */
      S Outil       UBYTE   IconTool[64];
  */
    { 1, AZLID_FRAME   ,                0                 , TEXT_Env1__Sauvegarde ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                 , TEXT_Env1_SPurgeEsp   ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                 , TEXT_Env1_SLFCR       ,0,0, 26,12, 0,0, Env1_SaveLFCR, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                 , TEXT_Env1_SLFCRLF     ,0,0, 26,12, 0,0, Env1_SaveLFCRLF, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                 , TEXT_Env1_SIcone      ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTRNG, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT, TEXT_Env1_SOutil      ,0,0,  0, 0, 0,0, 0, 8, W2L(63,255) }, /* visibles, W2L(MaxChars,maxval) */
  /*------------
    [Autosauve] 10
      V Actif       Flags1  AZP_AUTOSAVE     /* Autosauve */
      S Nom         UBYTE   AutsName[16];
      S Répertoire  UBYTE   AutsDirName[64];
      S             UWORD   AutsIntSec;
      S             UWORD   AutsIntMod;
  */
    { 1, AZLID_FRAME   ,                0                 , TEXT_Env1__Autosave     ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT | AZLF_SUB_EQUAL                 , TEXT_Env1_ASActif        ,0,0, 26,12, 0,0, endisable_autosauve, 0, 0 },
    { 2, AZLID_GADSTRNG, AZLF_TITLE_LEFT  | AZLF_SUB_EQUAL | AZLF_SAMELINE | AZLF_SUB_RIGHT , TEXT_Env1_ASNom         ,0,0,  0, 0, 0,0, 0, 8, W2L(15,0) }, /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADSTWFI, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT                 , TEXT_Env1_ASRepertoire   ,0,0,  0, 0, 0,0, 0,19, W2L(0,1) },  /* visibles, W2L(,DirsOnly) */
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT, TEXT_Env1_ASIntSec       ,0,0,  0, 0, 0,0, verif_maxval, 5, W2L( 5,0xffff) }, /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT, TEXT_Env1_ASIntMod       ,0,0,  0, 0, 0,0, verif_maxval, 5, W2L( 5,0xffff) }, /* visibles, W2L(MaxChars,maxval) */
  /*------------
    [Backup] 16
      V Actif       Flags1  AZP_KEEPBKP     /* Recopie le fichier avant de sauvegarder */
      V Même nom    Flags1  AZP_BKPFILENAME /* Utilise le nom du fichier comme nom de backup */
      S Nom         UBYTE   BackupName[16];
      S Répertoire  UBYTE   BackupDirName[64];
      S             UBYTE   BackupRot;
  */
    { 1, AZLID_FRAME   ,                0                 , TEXT_Env1__Backup     ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT | AZLF_SUB_EQUAL                 , TEXT_Env1_Actif        ,0,0, 26,12, 0,0, endisable_backup, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT  | AZLF_SUB_EQUAL | AZLF_SAMELINE , 0      ,0,0, 26,12, 0,0, endisable_bkpname, 0, 0 },
    { 2, AZLID_GADSTRNG, AZLF_TITLE_LEFT  | AZLF_SUB_EQUAL | AZLF_SAMELINE | AZLF_SUB_RIGHT , 0         ,0,0,  0, 0, 0,0, 0, 8, W2L(15,0) }, /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADSTWFI, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT , TEXT_Env1_Repertoire   ,0,0,  0, 0, 0,0, 0,19, W2L(0,1) },  /* visibles, W2L(,DirsOnly) */
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT , TEXT_ENV1_Rotations    ,0,0,  0, 0, 0,0, verif_maxval, 3, W2L( 3,0xff) }, /* visibles, W2L(MaxChars,maxval) */
  /*-----------
    [Mode] 22
      V Minitel     Flags1  AZP_MINITEL     /* Indique que le port série est relié à un minitel */
      V Lecture     Flags1  AZP_READONLY    /* Pas de modification possible */
      S Priorité    UBYTE Priority;
      S DelClip     UBYTE DelClip;
  */
    { 1, AZLID_FRAME   ,                0                   , TEXT_Env1__Mode       ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                   , TEXT_Env1_Minitel     ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT | AZLF_SAMELINE | AZLF_SUB_RIGHT , TEXT_Env1_Lecture     ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT, TEXT_Env1_Priorite    ,0,0,  0, 0, 0,0, verif_maxval_signed, 2, W2L(4,127) }, /* chars,max */
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SUB_RIGHT, TEXT_Env1_DelClip     ,0,0,  0, 0, 0,0, verif_maxval, 2, W2L(3,255) }, /* chars,max */
  /*------------
    [Tailles] 27
      S Blocs de texte (Ko) UBYTE PuddleSize;
      S Buffers d'E/S (Ko)  ULONG IOBufLength;
      S Lignes de commande  UWORD HistCmdLineMax;
      S Nombre de Undo      ULONG UndoMaxNum;
      S Buffer de Undo (Ko) ULONG UndoMaxSize;
  */
    { 1, AZLID_FRAME   ,                0                 , TEXT_Env1__Tailles    ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT /** | AZLF_SUB_RIGHT **/, TEXT_Env1_Blocstexte  ,0,0,  0, 0, 0,0, verif_maxval, 5, W2L( 3,255) },    /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT /** | AZLF_SUB_RIGHT **/, TEXT_Env1_BuffersES   ,0,0,  0, 0, 0,0, verif_maxval, 5, W2L( 5,0xffff) }, /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT /** | AZLF_SUB_RIGHT **/, TEXT_Env1_LignesCom   ,0,0,  0, 0, 0,0, verif_maxval, 5, W2L( 5,0xffff) }, /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT /** | AZLF_SUB_RIGHT **/, TEXT_Env1_BufferUndo  ,0,0,  0, 0, 0,0, verif_maxval, 5, W2L( 5,0xffff) }, /* visibles, W2L(MaxChars,maxval) */

    { 0,0,0,0,0,0,0,0,0,0 }
    };

struct AZLayout __far AZL_PrefsEnv2[] =
    {
  /*
    [Port ARexx] 0
      V Caractère de séparation : Flags1 AZP_REXXNAMESEP
      S                         UBYTE   CharRexxDot
      V Numérote le premier     Flags1 AZP_REXXNAMENUM
      V Laisse les minuscules   Flags2 AZP_LEAVEREXCASE
      S Port ARexx Maître       UBYTE   MastPortName[16];
      S Port Arexx d'édition    UBYTE   PortName[16];
  */
    { 1, AZLID_FRAME   ,               0                 , TEXT_Env2__PortARexx  ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                , TEXT_Env2_CaractereSep,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTRNG, AZLF_TITLE_LEFT | AZLF_SAMELINE , TEXT_Env2_PVide       ,0,0,  0, 0, 0,0, 0, 0, W2L(1,0) }, /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                , TEXT_Env2_Numerote    ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                , TEXT_Env2_Minuscules  ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTRNG, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT, TEXT_Env2_PortMaitre  ,0,0,  0, 0, 0,0, 0, 0, W2L(15,0) }, /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADSTRNG, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT, TEXT_Env2_PortEdition ,0,0,  0, 0, 0,0, 0, 0, W2L(15,0) }, /* visibles, W2L(MaxChars,maxval) */
  /*
    [Fichiers] 7
      S Motif de sélection      UBYTE   PatOpen[64];
      S Fichier de paramètres   UBYTE   ParmFile[64];
      S Console ARexx           UBYTE   ARexxConsoleDesc[96];
      S Console DOS             UBYTE   DOSConsoleDesc[96];
  */
    { 1, AZLID_FRAME   ,               0                  , TEXT_Env2__Fichiers   ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTRNG, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT , TEXT_Env2_Motif       ,0,0,  0, 0, 0,0, 0, 27, W2L(63,0) }, /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADSTWFI, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT , TEXT_Env2_Parametres  ,0,0,  0, 0, 0,0, 0, 27, W2L(0,0) }, /* visibles, W2L(,DirsOnly) */
    { 2, AZLID_GADSTRNG, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT , TEXT_Env2_ConARexx    ,0,0,  0, 0, 0,0, 0, 35, W2L(95,0) }, /* visibles, W2L(MaxChars,maxval) */
    { 2, AZLID_GADSTRNG, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT , TEXT_Env2_ConDOS      ,0,0,  0, 0, 0,0, 0, 35, W2L(95,0) }, /* visibles, W2L(MaxChars,maxval) */

    { 0,0,0,0,0,0,0,0,0,0 }
    };

//------------------------------------------- Prefs Window

struct AZLayout __far AZL_PrefsWin1[] =
    {
  /*
    [Police de caractères]
        F Interface         UBYTE FontName[32];     UBYTE FontHeight;
        F Vues              UBYTE ViewFontName[32]; UBYTE ViewFontHeight;
  */
    { 1, AZLID_FRAME   ,               0                 , TEXT_Win1__Police ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTFO1, AZLF_TITLE_LEFT                                 , TEXT_Win1_Interface,0,0,  0, 0, 0,0, 0, 22, 0 },
    { 2, AZLID_GADSTFO2, AZLF_TITLE_LEFT | AZLF_SAMELINE | AZLF_SUB_RIGHT, TEXT_Win1_Vues     ,0,0,  0, 0, 0,0, 0, 22, 0 }, /* visibles, nil */

  /*
    [Ouverture]
        V Ecran de devant   ULONG Flags1;   AZP_FRONTSCREEN /* L'ouverture à été demandée sur l'écran de devant */
        V Ecran AZur                        AZP_AZURSCREEN  /* L'ouverture à été demandée sur un écran AZur */
        V Plein écran                       AZP_FULLSCREEN  /* La fenêtre doit s'ouvrir en plein écran */
        S X                 struct AZViewInfo VI.ViewBox.Left;
        S Y                 struct AZViewInfo VI.ViewBox.Top;
        S Largeur           struct AZViewInfo VI.ViewBox.Width;
        S Hauteur           struct AZViewInfo VI.ViewBox.Height;
  */
    { 1, AZLID_FRAME   ,               0                                  , TEXT_Win1__Ouverture  ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                 | AZLF_SUB_EQUAL, TEXT_Win1_EcranDevant ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT | AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Win1_EcranAZur   ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT  | AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Win1_PleinEcran  ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT                 | AZLF_SUB_EQUAL, TEXT_Win1_X           ,0,0,  0, 0, 0,0, 0, 5, W2L(5,-1) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_RIGHT | AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Win1_Y           ,0,0,  0, 0, 0,0, 0, 5, W2L(5,-1) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Win1_Largeur     ,0,0,  0, 0, 0,0, 0, 5, W2L(5,-1) },
    { 2, AZLID_GADSTNUM, AZLF_TITLE_LEFT  | AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Win1_Hauteur     ,0,0,  0, 0, 0,0, 0, 5, W2L(5,-1) },

  /*
    [Fenêtre]
        V Barre de status   ULONG EleMask;  BUMF_STATUSBAR
        V Ligne de commande                 BUMF_CMDLINE
  */
    { 1, AZLID_FRAME   ,               0                                  , TEXT_Win1__Fenetre    ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                                 , TEXT_Win1_BarreStatus ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT  | AZLF_SAMELINE | AZLF_SUB_RIGHT, TEXT_Win1_LigneCmd    ,0,0, 26,12, 0,0, 0, 0, 0 },

  /*
    [Bord horizontal]
        V Butée             ULONG EleMask;  BUMF_BUTLEFT
        V Flêches                           BUMF_LEFTRIGHT1
        V Prop                              BUMF_HORIZPROP
        V Flêches                           BUMF_LEFTRIGHT2
        V Butée                             BUMF_BUTRIGHT
  */
    { 1, AZLID_FRAME   ,               0                                 , TEXT_Win1__BordHoriz ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT                | AZLF_SUB_EQUAL, TEXT_Win1_Butee      ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT| AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Win1_Fleches    ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_RIGHT| AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Win1_Prop       ,0,0, 26,12, 0,0, inact_horiz_prop, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT | AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Win1_Fleches    ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT | AZLF_SAMELINE | AZLF_SUB_EQUAL, TEXT_Win1_Butee      ,0,0, 26,12, 0,0, 0, 0, 0 },

  /*
    [Bord vertical]
        V Butée             ULONG EleMask;  BUMF_BUTUP
        V Flêches                           BUMF_UPDOWN1
        V Prop                              BUMF_VERTPROP
        V Flêches                           BUMF_UPDOWN2
        V Butée                             BUMF_BUTDOWN
  */
    { 1, AZLID_FRAME   ,               0                 , TEXT_Win1__Vertical  ,0,0,  0, 0, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT, TEXT_Win1_Butee      ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT, TEXT_Win1_Fleches    ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT, TEXT_Win1_Prop       ,0,0, 26,12, 0,0, inact_vert_prop, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT, TEXT_Win1_Fleches    ,0,0, 26,12, 0,0, 0, 0, 0 },
    { 2, AZLID_GADCHECK, AZLF_TITLE_LEFT | AZLF_SUB_RIGHT, TEXT_Win1_Butee      ,0,0, 26,12, 0,0, 0, 0, 0 },

    { 0,0,0,0,0,0,0,0,0,0 }
    };
