#ifndef WINDOWSTRUCT_H_INCLUDED
#define WINDOWSTRUCT_H_INCLUDED

#define LAYOUT 1
#define NON_LAYOUT 2

typedef long long LL;


struct WindowStruct{
    HDC hdc; // ���������� ��������� ����������, ��������� ��� ��������� � ������� �������
    PAINTSTRUCT ps; // ���������� ���������� �����������

    int mode; // ����� �����������

    int cxChar, cyChar; // ������� ����� ����� �� x � y
    int cxClient, cyClient; // ������� ������� ������� �� x � y
    int maxViewV, maxViewH;// ���������� ����� � �������� � ������� �������

    int iVscrollPos, iVscrollMax, iHscrollPos, iHscrollMax, iVscrollInc, iHscrollInc; // ���������� ���������

    int linePos, lineInc;

    int hCaretPos, vCaretPos;
    LL caretPlace;
};

#endif
