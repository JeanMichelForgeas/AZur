/*
**      $Id: RAZ_ErrorParse.rexx,v 1.5 1994/11/20 23:54:32 DBernard Rel $
**
**      DICE Error Parsing Script.  Script for AZur Editor (©1994 J.M. Forgeas).
**
**      From TurboText's original by John Toebes
**
**  Notes: This assumes that your DCC:Config/DCC.Config file contains the
**         following line:
**  cmd= rx DCC:Rexx/RAZ_ErrorParse.rexx %e "%n" "%c" "%f" "%0"
*/

OPTIONS RESULTS

PARSE ARG EFile '"' Fn '" "' CurDir '" "' CFile '" "' VPort '"'
IF VPort = '?' THEN VPort = ''

portname = 'DICE_ERROR_PARSER'  /* DICEHelp's port name */

IF ~show('p',portname) then DO
    ADDRESS COMMAND 'RUN >NIL: <NIL: DError REXXSTARTUP'

    DO i = 1 TO 6
        IF ~show('p',portname) THEN
            ADDRESS COMMAND 'wait 1'
    END

    IF ~show('p',portname) then DO
        SAY "Dice Error Parser (DERROR) program not found!"
        ADDRESS COMMAND 'type' EFile
        EXIT
    END
END

/**
 ** Get the error messages loaded in.
 ** This will return a list of lines within the file that have
 ** errors associated with them (if any)
 **/
ADDRESS DICE_ERROR_PARSER LOAD EFile '"'CurDir'" "'Fn'" "'VPort'"'
LINES = RESULT

/**
 ** Go through CFile
 **/
IF LINES ~= '' & CFile ~= '' THEN DO
    ADDRESS DICE_ERROR_PARSER 'TTXSAME "'CFile'" "'VPort'"'
    EPort = RESULT
    IF RC ~= 0 THEN DO
        SAY 'Unable to open file' CFile 'for editing'
        EXIT
    END

    ADDRESS VALUE EPort

    'BMRemove' "PAT DERR_#?"

    'LOCK D ON'
    DO I = 1 to WORDS(LINES)
        L = word(LINES, I)
        'BMAdd' "DERR_"||L "FOLDS LINE" L "COL 1"
    END
    'LOCK D OFF'

END

/**
 ** Lastly, go to the first error message
 **/

ADDRESS DICE_ERROR_PARSER Current E
IF RC ~= 0 THEN
   DO
      ADDRESS VALUE EPort 'REQMULTI' "TEXT Plus d''erreurs !"
      EXIT 0
   END

IF E.LINE = 0 THEN DO
    IF LEFT(E.TEXT, 5) = 'DLINK' THEN DO
        TT = TRANSLATE(E.STRING, '-', '"')
        /* This is a DLINK error, we need to handle it special */
        SAY 'There were DLINK Errors'
        ADDRESS COMMAND 'TYPE' EFILE
        EXIT 0
    END
END

ADDRESS DICE_ERROR_PARSER 'TTXSAME "'E.FPATH'" "'E.ARGS'"'
IF RC ~= 0 THEN DO
    SAY 'Unable to open' E.FPATH
    EXIT 0
END

Port = RESULT
ADDRESS VALUE Port
if E.Col = 0 THEN E.Col = 1
'BM' "DERR_"||E.Line
'Cursor' "FOLDS COL" E.Col
'REQMULTI' "TEXT" E.STRING

