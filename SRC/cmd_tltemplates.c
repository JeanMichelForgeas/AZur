static UBYTE __far CmdTmpl_kprintf[] = "TEXT/F";
static UBYTE __far CmdTmpl_Abbreviate[] = "DO=REPLACE/S,DEF=DEFINE/S,AB=ABBREV,FT=FULLTEXT/F,LOAD/K,SAVE/K";
static UBYTE __far CmdTmpl_AddParm[] = "TYPE,PARM/F";
static UBYTE __far CmdTmpl_Block2CB[] = "CLIP=UNIT/N";
static UBYTE __far CmdTmpl_Block2File[] = "NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,NAME,PAT=PATTERN/K";
static UBYTE __far CmdTmpl_Block2Mac[] = "CLEAR/S,AP=APPEND/S";
static UBYTE __far CmdTmpl_Block[] = "CLIP=UNIT/K/N,CLEAR=OFF/S,ERASE=DEL/S,BC=BCHAR/S,BW=BWORD/S,BL=BLINE/S,BR=BRECT=BVERT/S,BF=BFULLRECT/S,ALL/S,MC=MOVECURS/S,START/S,STARTDO/S,FOLDS/S,REQ=REQUEST/S,LINE/N,COL=COLUMN/N,PAGE/S,REL=RELATIVE/S,WORD/N,TAB/N,OF=OFFSET/N,UP/S,LEFT/S,W=WRAP/S,ENDWORD/S,LINES/N,CHARS/N,PO=POINTER/S,EC=EXTCURS/S,CONT=CONTINUE/S";
static UBYTE __far CmdTmpl_BMAdd[] = "RESET/S,BLOCK/S,NAME=BOOKMARK,CLEAR=OFF/S,MC=MOVECURS/S,DELPROOF/S,FOLDS/S,REQ=REQUEST/S,LINE/N,COL=COLUMN/N,PAGE/S,REL=RELATIVE/S,WORD/N,TAB/N,OF=OFFSET/N,UP/S,LEFT/S,W=WRAP/S,ENDWORD/S,LINES/N,CHARS/N,PO=POINTER/S";
static UBYTE __far CmdTmpl_BMCursor[] = "NAME=BOOKMARK,NUM=ID/N/K,CLEAR=OFF/S";
static UBYTE __far CmdTmpl_BMRemove[] = "NAME=BOOKMARK,NUM=ID/N/K,ALL/S,PAT=PATTERN/K";
static UBYTE __far CmdTmpl_CB2File[] = "CLIP=UNIT/K/N,NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,NAME,PAT=PATTERN/K";
static UBYTE __far CmdTmpl_CB2Win[] = "CLIP=UNIT/N,RECT=VERTICAL/S,REPL=OVERLAY/S,CC=CONVERT/S,NC=NOCURS/S";
static UBYTE __far CmdTmpl_CharConvert[] = "CHAR/S,WORD/S,LINE/S,ALL/S,LOWER/S,UPPER/S,TOGGLE/S,NOACCENT/S,ASCII7/S,VDX2AM/S,AM2VDX/S,MS2AM/S,AM2MS/S,ROT=ROTATE/S,XORKEY/K,NC=NOCURS/S";
static UBYTE __far CmdTmpl_CorrectWord[] = "WORD/F";
static UBYTE __far CmdTmpl_CorrectWordCase[] = "WORD/F";
static UBYTE __far CmdTmpl_Cursor[] = "FOLDS/S,REQ=REQUEST/S,LINE/N,COL=COLUMN/N,PAGE/S,REL=RELATIVE/S,WORD/N,TAB/N,OF=OFFSET/N,UP/S,LEFT/S,W=WRAP/S,ENDWORD/S,LINES/N,CHARS/N,PO=POINTER/S,CLEAR=OFF/S,PB=PREVBLOCK/S,NB=NEXTBLOCK/S,PM=PREVMARK/S,NM=NEXTMARK/S,PF=PREVFOLD/S,NF=NEXTFOLD/S";
static UBYTE __far CmdTmpl_Del[] = "NUM/N,CHARS/S,LINES/S,WORDS/S,PREV=BACK=PREVIOUS/S,BOUND/S,LINE/N,COL=COLUMN/N,PAGE/S,REL=RELATIVE/S,WORD/N,TAB/N,OF=OFFSET/N,UP/S,LEFT/S,W=WRAP/S,ENDWORD/S,NC=NOCURS/S,NOCLIP/S";
static UBYTE __far CmdTmpl_ExeARexxCmd[] = "BLOCK/S,CONSOLE/S,ASYNC/S,BA=BLOCKARG/S,CMD=COMMAND=MACRO/F";
static UBYTE __far CmdTmpl_ExeARexxScript[] = "BLOCK/S,CONSOLE/S,ASYNC/S,BA=BLOCKARG/S,PAT=PATTERN/K,SCRIPT=NAME/F";
static UBYTE __far CmdTmpl_ExeDOSCmd[] = "BLOCK/S,CONSOLE/S,ASYNC/S,BA=BLOCKARG/S,PORT/S,SCREEN/S,CMD=COMMAND=NAME/F";
static UBYTE __far CmdTmpl_ExeDOSScript[] = "BLOCK/S,CONSOLE/S,ASYNC/S,BA=BLOCKARG/S,PORT/S,SCREEN/S,PAT=PATTERN/K,SCRIPT=NAME/F";
static UBYTE __far CmdTmpl_File2CB[] = "NAME,CLIP=UNIT/K/N,PAT=PATTERN/K";
static UBYTE __far CmdTmpl_File2File[] = "NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,FROM=NAME,TO=DEST,PAT=PATTERN/K,DESTPAT=DESTPATTERN/K";
static UBYTE __far CmdTmpl_File2Mac[] = "AP=APPEND/S,NAME,PAT=PATTERN/K";
static UBYTE __far CmdTmpl_File2NewWin[] = "ICONIFY/S,FILEREQ/S,CMD=FC=FROMCMD/K,F=FROM=MACRO/K,PN=PORTNAME/K,NW=NOWINDOW/S,RO=READONLY/S,PRI=PRIORITY/K/N,PAT=PATTERN/K,NAME,BLOCK/S,LEFT/K/N,TOP/K/N,WIDTH/K/N,HEIGHT/K/N,CREATE/S,NI=NOICON/S,CURSNAME/S";
static UBYTE __far CmdTmpl_File2Parms[] = "NAME,PAT=PATTERN/K";
static UBYTE __far CmdTmpl_File2Prefs[] = "DEFAULT/S,NAME";
static UBYTE __far CmdTmpl_File2Win[] = "QUIET=FORCE/S,CLEAR/S,SAME/S,CREATE=SC=SETCURRENT/S,NAME=FILENAME,RECT=VERTICAL/S,REPL=OVERLAY/S,CC=CONVERT/S,NC=NOCURS/S,RO=READONLY/S,BLOCK/S,PAT=PATTERN/K,NI=NOICON/S,CURSNAME/S,ND=NODECOMPRESS/S";
static UBYTE __far CmdTmpl_Fold[] = "LINE/N,ENDLINE/N,SET/S,ADDSAME/S,UNSET/S,ON/S,OFF/S,TOGGLE/S,SUB=NESTED/S,ALL/S";
static UBYTE __far CmdTmpl_GetBlock[] = "NUM=ID/N,ALL/S";
static UBYTE __far CmdTmpl_GetBlockInfo[] = "NUM=ID/N";
static UBYTE __far CmdTmpl_GetBMInfo[] = "NAME=BOOKMARK,NUM=ID/N/K";
static UBYTE __far CmdTmpl_GetError[] = "ERROR/N,LAST/S";
static UBYTE __far CmdTmpl_GetFold[] = "NUM=FOLDNUM/A/N,SUB/S,FSL=FIRSTSUBLINE/S";
static UBYTE __far CmdTmpl_GetFoldInfo[] = "FULL/S";
static UBYTE __far CmdTmpl_GetLine[] = "STARTCOL/N,ENDCOL/N,LINE/N";
static UBYTE __far CmdTmpl_GetLineInfo[] = "LINE/N";
static UBYTE __far CmdTmpl_GetPort[] = "NAME";
static UBYTE __far CmdTmpl_GetPortList[] = "OI=ONLYICONS/S,NI=NOTICONS/S";
static UBYTE __far CmdTmpl_GetPos[] = "LINE/S,COL=COLUMN/S,IN=INFOLD/S,FOLDS/S";
static UBYTE __far CmdTmpl_GetPrefs[] = "ATTR/A/K/F";
static UBYTE __far CmdTmpl_GetScreenList[] = "PUBLIC/S";
static UBYTE __far CmdTmpl_GetScreenInfo[] = "NOBAR/S";
static UBYTE __far CmdTmpl_GetTaskInfo[] = "HEX/S";
static UBYTE __far CmdTmpl_GetText[] = "SL=STARTLINE/N,SC=STARTCOL/N,EL=ENDLINE/N,EC=ENDCOL/N,RECT=VERT/S";
static UBYTE __far CmdTmpl_GetVar[] = "NAME/A";
static UBYTE __far CmdTmpl_GetViewInfo[] = "NUM=VIEWNUM/N";
static UBYTE __far CmdTmpl_GetWinInfo[] = "WINDOW/S";
static UBYTE __far CmdTmpl_GetWinList[] = "OI=ONLYICONS/S,NI=NOTICONS/S";
static UBYTE __far CmdTmpl_GetWord[] = "LINE/N,COL=COLUMN/N,PREV/S,NEXT/S,MC=MOVECURS/S";
static UBYTE __far CmdTmpl_Help[] = "COMMAND/A,VB=VERBOSE/S,FULL/S,T=TEMPLATE/S,S=SYNONYM/S,F=FLAGS/S";
static UBYTE __far CmdTmpl_Indent[] = "BACK/S,SPACE/S";
static UBYTE __far CmdTmpl_Load[] = "NAME/F/A";
static UBYTE __far CmdTmpl_Lock[] = "D=DISPLAY/S,I=INPUT/S,M=MODIFY/S,S=SAVE/S,B=STATUS/S,A=AREXX/S,ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_Mac2File[] = "NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,NAME,PAT=PATTERN/K";
static UBYTE __far CmdTmpl_MacPlay[] = "COUNT/N";
static UBYTE __far CmdTmpl_MacRecord[] = "AP=APPEND/S,QUIET/S";
static UBYTE __far CmdTmpl_Match[] = "FULL/S,NC=NOCURS/S,PAIRS/A/F";
static UBYTE __far CmdTmpl_Prefs2File[] = "DEFAULT/S,NAME";
static UBYTE __far CmdTmpl_Qualifier[] = "CTRL/S,SHIFT/S,ALT/S,AMIGA/S,QUAL1/S,QUAL2/S,QUAL3/S,LOCK1/S,LOCK2/S,LOCK3/S,ANY/S,ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_Replace[] = "PREFS/S,RESET/S,FB=FROMBLOCK/S,CASE/S,ACC=ACCENT/S,ALL/S,SINGLE/S,BACK=BACKWARD=PREV=PREVIOUS/S,FORW=FORWARD=NEXT/S,FULL/S,FC=FROMCURS/S,IB=INTOBLOCK/S,PAT=PATTERN/S,CC=CONVERT/S,CLEAR=OFF/S,TB=TOBLOCKS/S,NB=NOBLOCK/S,SOL/S,EOL/S,SOW/S,EOW/S,REQ=REQUEST=PROMPT/S,NC=NOCURS/S,SEARCH=FIND=TEXT=REPL=REPLACE=CHANGE,WITH=BY,NV=NOVERIFY/S";
static UBYTE __far CmdTmpl_ReqClose[] = "REQUESTER/A";
static UBYTE __far CmdTmpl_ReqFile[] = "TITLE=PROMPT,PATH,PAT=PATTERN,DIRMODE/S,AS=ASSIGN/S,EXIST/S,SI=SHOWICONS/S";
static UBYTE __far CmdTmpl_ReqList[] = "TITLE,CONV=CONVERT/S,SORT/S,STR/S,DS=DEFSTR=DEFAULT/K,SEL=SELECT/K,LIST=TEXT=PROMPT/A/F";
static UBYTE __far CmdTmpl_ReqMulti[] = "TITLE,CONV=CONVERT/S,REQWIN/S,ACT=STRACTIVATE/S,STR1/S,T1=TITLESTR1,D1=DEFSTR1=DEFAULT,STR2/S,T2=TITLESTR2,D2=DEFSTR2=DEFAULT2,B1=BUTOK/K,B2=BUTMID/K,B3=BUTCANCEL/K,STDOK/S,STDCANCEL/S,TEXT=PROMPT/F";
static UBYTE __far CmdTmpl_ReqOpen[] = "REQUESTER/A";
static UBYTE __far CmdTmpl_RexxNameObtain[] = "NAME/A";
static UBYTE __far CmdTmpl_RexxNameRelease[] = "NAME/A";
static UBYTE __far CmdTmpl_RexxNameClear[] = "NAME/A";
static UBYTE __far CmdTmpl_Search[] = "PREFS/S,RESET/S,FB=FROMBLOCK/S,CASE/S,ACC=ACCENT/S,ALL/S,SINGLE/S,BACK=BACKWARD=PREV=PREVIOUS/S,FORW=FORWARD=NEXT/S,FULL/S,FC=FROMCURS/S,IB=INTOBLOCK/S,PAT=PATTERN/S,CC=CONVERT/S,CLEAR=OFF/S,TB=TOBLOCKS/S,NB=NOBLOCK/S,SOL/S,EOL/S,SOW/S,EOW/S,REQ=REQUEST=PROMPT/S,NC=NOCURS/S,SEARCH=FIND=TEXT";
static UBYTE __far CmdTmpl_SetModified[] = "ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_SetCompress[] = "ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_SetCurrentDir[] = "NAME";
static UBYTE __far CmdTmpl_SetResident[] = "ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_SetDebug[] = "ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_SetFilePath[] = "NAME/A";
static UBYTE __far CmdTmpl_SetPrefs[] = "ATTR/A/K/F";
static UBYTE __far CmdTmpl_SetPri[] = "PRIORITY/N";
static UBYTE __far CmdTmpl_SetQuoteMode[] = "ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_SetStatusBar[] = "TEMPORARY/S,TEXT";
static UBYTE __far CmdTmpl_SetVar[] = "NAME/A,WITH=CONTENTS/F";
static UBYTE __far CmdTmpl_SetWriteReplace[] = "ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_Space2Tab[] = "WIDTH/N";
static UBYTE __far CmdTmpl_Sort[] = "SL=STARTLINE/N,EL=ENDLINE/N,SC=STARTCOL/N,EC=ENDCOL/N,GL=GROUPLINES/N,GO=GROUPOFFSET/N,ALL/S,CASE/S,ACCENT/S,DATE/S,DS=DESCENDING/S";
static UBYTE __far CmdTmpl_Surround[] = "F=FIRST/A,L=LAST,M=MIDDLE,C=CORNER,LINE/S,RECT/S,SL=STARTLINE/N,SC=STARTCOL/N,EL=ENDLINE/N,EC=ENDCOL/N,OV=OVERLAY/S,NC=NOCURS/S,CC=CONVERT/S";
static UBYTE __far CmdTmpl_Tab2Space[] = "WIDTH/N";
static UBYTE __far CmdTmpl_Template[] = "TEMPLATE";
static UBYTE __far CmdTmpl_Text2CB[] = "CLIP=UNIT/K/N,CC=CONVERT/S,TEXT/F";
static UBYTE __far CmdTmpl_Text2File[] = "NB=NOBACKUP/S,NI=NOICON/S,AP=APPEND/S,NAME,PAT=PATTERN/K,CC=CONVERT/S,TEXT/F";
static UBYTE __far CmdTmpl_Text2Win[] = "LINE/K/N,COL=COLUMN/K/N,PAGE/S,REL=RELATIVE/S,WORD/K/N,TAB/K/N,OF=OFFSET/K/N,UP/S,LEFT/S,W=WRAP/S,ENDWORD/S,IP=INDENTPREV/S,I=INDENT=STAY/S,IN=INDENTNEXT/S,PREFS/S,NC=NOCURS/S,RECT=VERTICAL/S,REPL=OVERLAY/S,CC=CONVERT/S,NF=NOFORMAT/S,TEXT/F";
static UBYTE __far CmdTmpl_TextFormat[] = "JP=JOINPARA/S";
static UBYTE __far CmdTmpl_TextJustify[] = "C=CENTER/S,F=FLUSH/S,L=LEFT/S,R=RIGHT/S";
static UBYTE __far CmdTmpl_Undo[] = "ALL/S,TIMES/N";
static UBYTE __far CmdTmpl_Unload[] = "FORCE/S,NAME/F/A";
static UBYTE __far CmdTmpl_ViewActivate[] = "NUM=VIEWNUM/N";
static UBYTE __far CmdTmpl_ViewCenter[] = "ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_ViewDown[] = "VIEWS/N";
static UBYTE __far CmdTmpl_ViewScroll[] = "LINES/N";
static UBYTE __far CmdTmpl_ViewSize[] = "LINES/N/A";
static UBYTE __far CmdTmpl_ViewSplit[] = "ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_ViewUp[] = "VIEWS/N";
static UBYTE __far CmdTmpl_ViewUpdate[] = "ALL/S";
static UBYTE __far CmdTmpl_Win2CB[] = "CLIP=UNIT/N";
static UBYTE __far CmdTmpl_Win2File[] = "NB=NOBACKUP/S,NI=NOICON/S,SAME/S,AP=APPEND/S,SC=SETCURRENT/S,MODIFIED/S,NAME,PAT=PATTERN/K,PAR=PARAGRAPH/S,NT=NOSPACE2TAB/S,CF=CONFIRM/S";
static UBYTE __far CmdTmpl_WinChange[] = "LEFT=LEFTEDGE/N,TOP=TOPEDGE/N,WIDTH/N,HEIGHT/N";
static UBYTE __far CmdTmpl_WinClear[] = "QUIET=FORCE/S";
static UBYTE __far CmdTmpl_WinClose[] = "QUIET=FORCE/S,ALL/S,SAVE/S";
static UBYTE __far CmdTmpl_WinIconify[] = "ON/S,OFF/S,TOGGLE/S";
static UBYTE __far CmdTmpl_WinZoom[] = "FULL/S,ON/S,OFF/S,TOGGLE/S";