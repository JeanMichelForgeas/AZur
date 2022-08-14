# AZur
Text editor for Amiga OS in 1995

For more documentation please see in azur_demo/man/

I could not find my complete doc nor the full set of ARexx scripts provided with the commercial version.

Please if somebody have them, can you add them here or send them to me ?
Thanks a ot :)
 
 

                         FUNCTIONALITIES



Fully configurable, multi-process, one per edited file, ARexx, multi-clipboards, rectangular blocks, multi-blocks, bookmarks, macros... Commands are objects, and a dev kit allows to create your own external commands. Even UIK objects (UI, ...) can be used in your command objects.


### FOLDS

Any part of the text can be folded. We always fold whole lines. 
When creating a fold, its limits can be specified in different ways by the arguments of the command or according to a drawn block.

Folds can be nested. Two nested folds cannot have the same start line.

The same command is used for all operations on folds, it is the "Fold" command.

The "Fold TOGGLE" command attached to the mouse buttons as well as the possibility of nesting allow to use AZur as an "idea processor" program, i.e. to fold/unfold paragraphs and paragraphs and sub-paragraphs with a single mouse click.

Working with multiple blocks allows to create several folds at the same time, or to act on several existing folds at the same time: fold/unfold/remove.


### BLOCKS

AZur allows multiple blocks, i.e. as many blocks as you want in the text at the same time. Moreover they can be of
different types :
    - by whole line
    - by whole word
    - per character
    - rectangular

Many commands have a different behavior depending on whether a block is rectangular or not.
For example "Indent" will push back from the left boundary of the left edge of the block, "Surround" will surround both sides instead of just one, etc.
As a general rule, a rectangular block will restrict the actions within the actions inside the rectangle.


### BOOKMARKS

You can mark any place in a text, and give a name to this mark.
The name can be up to 54 characters long.
The name of a bookmark can be taken automatically from a block or from the word or in the word just under the cursor.

Each time the text is modified, the marks located downstream (after the change) automatically move to follow the marked text.


### MACROS

AZur contains an internal receptacle where a sequence of commands can be stored. This receptacle is called the "Internal Macro".

In order to be taken into account during the recording by MacRecord, the commands must come from one of three sources:
    - from a menu,
    - a key,
    - or from ARexx.
Not everything that is done with the mouse is recorded in the macro.
For example, to draw a block with the macro, during the recording you have to draw the block with the keyboard (CTRL + arrow keys).

Once filled in, the internal macro of AZur can be transformed into text to be inserted in the edited text (Mac2Win), or saved in a file (Mac2File).


### VARIABLES

Each master file has its own set of variables that can be considered as local by the master process, but global by its children process.

They can be used in two ways:
- in ARexx scripts through commands (GetVar, SetVar, etc...)
- in the command objects themselves, even the external ones thanks to functions of the the AZur development set : AZ_Var...().


### FORMATTING AND MARGINS

The preferences allow to activate the margins. These margins are only taken into account during specific operations : formatting and justification of the text.

They act from the line under the cursor to the line ended by the end-of-line character (up to the end of the paragraph).
If a block is drawn, its limits define those of these commands:
- a rectangular block: its upper and lower limits define the lines taken into account, and its left and right limits define : either new margins for TextFormat, or the width of the action for TextJustify.
- a block other than rectangular: same except that the left and right margins are those of the preferences. If the preference margins are not activated, the left limit is given by the start column of the first line of the block, and the right limit is given by the end column of the last line of the block.

Warning: the first lines of a paragraph behave a little differently from the other lines: in order to keep their indentation the spaces at the beginning of the line are not deleted either by reformatting the line or by justification. However, there is a way to treat them in the same way as the others: the use of a rectangular block.


### COMPRESSION

AZur can read files compressed with PowerPacker and save them by compressing them. In the future new compression modes will be added.

The library "powerpacker.library" is required, version 35 or higher to be able to save.

Translated with www.DeepL.com/Translator (free version)


### AREXX

Full implementation of ARexx communications, and a direct ARexx console.
