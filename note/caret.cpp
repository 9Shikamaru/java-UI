#include "caret.h"

// ѕоказать каретку
void setCaret(HWND hwnd, WindowStruct * ws){
    CreateCaret(hwnd, NULL, ws->cxChar, ws->cyChar);
    SetCaretPos(ws->hCaretPos * ws->cxChar, ws->vCaretPos * ws->cyChar);//устанавливает положение каретки в окне
    ShowCaret(hwnd);
}

// ”брать каретку
void deleteCaret(HWND hwnd){
    HideCaret(hwnd);
    DestroyCaret();
}

// ¬озвращает вью порт к каретке
void goToCaret(HWND hwnd, TextStruct * ts, WindowStruct * ws){
    // ≈сли каретка за границами рабочей области по горизонтали
    ws->iHscrollInc = 0;
    if(ws->maxViewH - 1 < ws->hCaretPos){
            ws->iHscrollInc = ws->hCaretPos - ws->maxViewH + 1;
        }
    else if(ws->hCaretPos < 0){
            ws->iHscrollInc = ws->hCaretPos;
        }
    scrollH(hwnd, ws);
    ws->hCaretPos -= ws->iHscrollInc;
    ws->iHscrollInc = 0;

    // ≈сли каретка за границами рабочей области по вертикали
    ws->lineInc = 0;
    if(ws->maxViewV - 1 < ws->vCaretPos){
            scrollToLine(hwnd, ts, ws, ws->linePos + ws->vCaretPos - ws->maxViewV + 1);
        }
    else if(ws->vCaretPos < 0){
            scrollToLine(hwnd, ts, ws, ws->linePos + ws->vCaretPos);
        }

    ws->vCaretPos -= ws->lineInc;
}

// возвращает номер символа на котором расположена каретка
LL getCaretPlace(TextStruct * ts, WindowStruct * ws){
    return ts->start[ws->linePos + ws->vCaretPos] + ws->iHscrollPos + ws->hCaretPos;
}
