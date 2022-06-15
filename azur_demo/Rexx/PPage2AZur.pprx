/** $VER: PPage2AZur.pprx 1.1 (7.2.95)
 **
 ** Ce script permet à ProPage d'utiliser AZur au lieu de
 ** "Article Editor" pour éditer le texte de la boite active
 ** (de l'ensemble des boites liées entre elles).
 **
 ** Il s'utilise en tant que "Genie" ProPage. Il faut donc le
 ** dire à ProPage avec l'option Import de sa requête des Genies.
 **
 ** Lorsqu'on sauvegarde sous AZur pour revenir dans ProPage,
 ** pour conserver les paragraphes il faut utiliser le menu :
 **                     "Projet/Sauver PPage"
 ** et puis ensuite fermer la fenêtre.
 **
 ** ATTENTION : dû à une limitation dans les commandes ProPage
 ** qui servent à insérer le texte, ce script ne fonctionne que
 ** si le texte à insérer fait moins de 32737 caractères.
 ** Sinon utilisez ArticleEditor...
 **/

OPTIONS RESULTS

state = ppm_GetState()
IF (WORD( state, 1 )) = 3 THEN CALL ppm_EndEdit()

CALL ppm_AutoUpdate(0)
tmpfile = "RAM:EditionProPage"
ppport = ADDRESS()

activebox = ppm_BoxNum()
IF (activebox = 0) THEN exit_msg( "Il faut une boite de texte active" )

arttext = ppm_GetArticleText(,1)

ADDRESS AZURMAST File2NewWin WIDTH 640 CREATE '"'tmpfile'"'
IF (RC > 0) THEN exit_msg( "Impossible de démarrer une fenêtre AZur" )
azurport = RESULT
ADDRESS VALUE azurport

/*-----------------------*/
Text2Win NOCURS arttext
SetPrefs LEFTMARGIN 0 RIGHTMARGIN 75 USEMARGINS ON SHOWEOL ON SHOWEOF ON AUTOFORMAT ON TAB2SPACE OFF SPACE2TAB OFF
Block ALL
TextFormat
SetModified OFF

/* attend la fermeture de la fenêtre AZur */
DO WHILE (SHOW( 'P', azurport ) ~= 0)
    ADDRESS COMMAND 'Wait >Nil: 1 SEC'
END
ADDRESS VALUE ppport
CALL ppm_PPageToFront()

IF (EXISTS( tmpfile )) THEN DO
    IF (OPEN( 'file', tmpfile, 'R' )) THEN DO
        CALL SEEK( 'file', 0, 'B' ); size = SEEK( 'file', 0, 'E' ); CALL SEEK( 'file', 0, 'B' );
        IF (size > 0) THEN DO
            arttext = READCH( 'file', size )
            CALL ppm_DeleteContents( activebox )
            CALL ppm_TextIntoBox( activebox, arttext ) /* ne fonctionne pas pour plus de 32000 caractères */
            /*
            CALL ppm_SetEdit( activebox )
            blocksize = 30000
            DO WHILE (size > 0)
                IF (size < blocksize) THEN blocksize = size
                CALL ppm_InsertText( SUBSTR(arttext,size-blocksize+1,blocksize) )
                size = size - blocksize
            END
            */
        END
        CALL CLOSE( 'file' )
    END
    ADDRESS COMMAND 'Delete >Nil:' tmpfile||'#?'
END
/*-----------------------*/

EXIT

exit_msg:
DO
    PARSE arg msg
    CALL ppm_GetUserText( 1, msg )
    EXIT
END
