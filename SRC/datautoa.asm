*************************************
*
*           Automates
*
*************************************


        INCLUDE "uiki:uikdrawauto.i"


        XDEF    _AutoDrawBox_Light


****************************************************************
*
        SECTION data,DATA
*
****************************************************************/

;------------------- Light Box
_AutoDrawBox_Light
    dc.w  IF_RATIO2_SKIP_N,(.2-.1)
.1  dc.w    COL_WHITE
    dc.w    MOVE_Xy
    dc.w    DRAW_xy
    dc.w    DRAW_xY
    dc.w    COL_BLACK
    dc.w    x_ADD_1
    dc.w    MOVE_xY
    dc.w    DRAW_XY
    dc.w    IF_LOOK_SKIP_N,8
    dc.w        y_ADD_1
    dc.w        DRAW_Xy
    dc.w            SKIP_N,4
    dc.w        DRAW_Xy
    dc.w        y_ADD_1
    dc.w    Y_SUB_1
    dc.w    X_SUB_1
    dc.w    COL_WHITE
    dc.w    END_AUTO
    ;---------------
.2  dc.w    X_SUB_1
    dc.w    COL_WHITE
    dc.w    MOVE_Xy
    dc.w    DRAW_xy
    dc.w    DRAW_xY
    dc.w    x_ADD_1
    dc.w    MOVE_xy
    dc.w    DRAW_xY
    dc.w    COL_BLACK
    dc.w    X_ADD_1
    dc.w    DRAW_XY
    dc.w    DRAW_Xy
    dc.w    X_SUB_1
    dc.w    y_ADD_1
    dc.w    MOVE_Xy
    dc.w    DRAW_XY
    dc.w    Y_SUB_1
    dc.w    X_SUB_1
    dc.w    x_ADD_1
    dc.w    COL_WHITE
    dc.w    END_AUTO

