#include "viewscroll.h"


// Ставит каретку на определённую позицию
void goToSymbPos(HWND hwnd, TextStruct * ts, WindowStruct * ws){
    LL symbLine = ws->linePos + ws->vCaretPos;
    LL ySymbPos = 0, xSymbPos = 0;

    for(LL i = 0; i <= ts->lines; ++i){
        if((ts->start[i] <= ws->caretPlace) && (ts->start[i + 1] > ws->caretPlace)){
            ySymbPos = i;
            xSymbPos = ws->caretPlace - ts->start[i];
            ws->lineInc = ySymbPos - symbLine;

            if(ws->lineInc >= (ws->maxViewV - ws->vCaretPos)){
                scrollToLine(hwnd, ts, ws, (ySymbPos - ws->maxViewV + (( (ws->cyClient % ws->cyChar ) > 0) ? 1 : 0 )));
            }
            else if(ws->lineInc <= -ws->vCaretPos){
                scrollToLine(hwnd, ts, ws, ySymbPos);
            }

            ws->iHscrollInc = xSymbPos - ws->iHscrollPos - ws->hCaretPos;
            scrollH(hwnd, ws);

            ws->hCaretPos = xSymbPos - ws->iHscrollPos;
            ws->vCaretPos = ySymbPos - ws->linePos;

            ws->caretPlace = getCaretPlace(ts, ws);
            SetCaretPos(ws->hCaretPos * ws->cxChar, ws->vCaretPos * ws->cyChar);
            ws->lineInc = 0;

            return;
        }
    }
}

// Линейное отображение: позиция скролла -> начальная строка
int getMap(TextStruct ts, WindowStruct ws, int scrollPos){
    return ( (scrollPos * (ts.lines - ws.maxViewV)) / ws.iVscrollMax ) +
            ( (( (scrollPos * (ts.lines - ws.maxViewV)) % ws.iVscrollMax ) > 0) ? 1 : 0 );
}

// Линейное отображение: строка -> позиция скролла
int getInverseMap(TextStruct ts, WindowStruct ws, LL linePos){
    return (linePos * (LL) ws.iVscrollMax) / ( ts.lines - (LL)(ws.maxViewV));
}

// Прокручивает скроллы до определённой строки
void scrollToLine(HWND hwnd, TextStruct * ts, WindowStruct * ws, LL linePos){
    LL prevLinePos = ws->linePos;
    ws->linePos = linePos;
    ws->lineInc = ws->linePos - prevLinePos;

    ws->iVscrollPos = getInverseMap(*ts, *ws, ws->linePos);

    ScrollWindow(hwnd, 0, -ws->lineInc * ws->cyChar, NULL, NULL);
    SetScrollPos(hwnd, SB_VERT, ws->iVscrollPos, TRUE);

    UpdateWindow(hwnd);
}

// Вертикальная прокрутка
void scrollV(HWND hwnd, TextStruct * ts, WindowStruct * ws){
    ws->iVscrollInc = max(-ws->iVscrollPos, min(ws->iVscrollInc, ws->iVscrollMax - ws->iVscrollPos));

    if(ws->iVscrollInc != 0){
        ws->iVscrollPos += ws->iVscrollInc;

        ws->lineInc = ws->linePos;
        ws->linePos = getMap(*ts, *ws, ws->iVscrollPos);
        ws->lineInc = ws->linePos - ws->lineInc;

        ScrollWindow(hwnd, 0, -ws->lineInc * ws->cyChar, NULL, NULL);
        SetScrollPos(hwnd, SB_VERT, ws->iVscrollPos, TRUE);

        UpdateWindow(hwnd);
    }
}

// Горизонтальная прокрутка
void scrollH(HWND hwnd, WindowStruct * ws){
    ws->iHscrollInc = max(-ws->iHscrollPos, min(ws->iHscrollInc, ws->iHscrollMax - ws->iHscrollPos));

    if(ws->iHscrollInc != 0){
        ws->iHscrollPos += ws->iHscrollInc;

        ScrollWindow(hwnd, -ws->iHscrollInc * ws->cxChar, 0, NULL, NULL);
        SetScrollPos(hwnd, SB_HORZ, ws->iHscrollPos, TRUE);

        UpdateWindow(hwnd);
    }
}
