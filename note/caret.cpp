#include "caret.h"

// �������� �������
void setCaret(HWND hwnd, WindowStruct * ws){
    CreateCaret(hwnd, NULL, ws->cxChar, ws->cyChar);
    SetCaretPos(ws->hCaretPos * ws->cxChar, ws->vCaretPos * ws->cyChar);//������������� ��������� ������� � ����
    ShowCaret(hwnd);
}

// ������ �������
void deleteCaret(HWND hwnd){
    HideCaret(hwnd);
    DestroyCaret();
}

// ���������� ��� ���� � �������
void goToCaret(HWND hwnd, TextStruct * ts, WindowStruct * ws){
    // ���� ������� �� ��������� ������� ������� �� �����������
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

    // ���� ������� �� ��������� ������� ������� �� ���������
    ws->lineInc = 0;
    if(ws->maxViewV - 1 < ws->vCaretPos){
            scrollToLine(hwnd, ts, ws, ws->linePos + ws->vCaretPos - ws->maxViewV + 1);
        }
    else if(ws->vCaretPos < 0){
            scrollToLine(hwnd, ts, ws, ws->linePos + ws->vCaretPos);
        }

    ws->vCaretPos -= ws->lineInc;
}

// ���������� ����� ������� �� ������� ����������� �������
LL getCaretPlace(TextStruct * ts, WindowStruct * ws){
    return ts->start[ws->linePos + ws->vCaretPos] + ws->iHscrollPos + ws->hCaretPos;
}
