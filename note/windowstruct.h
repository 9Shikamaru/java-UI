#ifndef WINDOWSTRUCT_H_INCLUDED
#define WINDOWSTRUCT_H_INCLUDED

#define LAYOUT 1
#define NON_LAYOUT 2

typedef long long LL;


struct WindowStruct{
    HDC hdc; // Дескриптор контекста устройства, необходим для рисования в рабочей области
    PAINTSTRUCT ps; // Дескриптор устройства отображения

    int mode; // Режим отображения

    int cxChar, cyChar; // Размеры одной буквы по x и y
    int cxClient, cyClient; // Размеры рабочей области по x и y
    int maxViewV, maxViewH;// Количество строк и столбцов в рабочей области

    int iVscrollPos, iVscrollMax, iHscrollPos, iHscrollMax, iVscrollInc, iHscrollInc; // Координаты ползунков

    int linePos, lineInc;

    int hCaretPos, vCaretPos;
    LL caretPlace;
};

#endif
