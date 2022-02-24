#ifndef TEXTSTRUCT_H_INCLUDED
#define TEXTSTRUCT_H_INCLUDED

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "windowstruct.h"

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))


struct TextStruct{
    char * text; // �����
    LL realLength; // �������� ���-�� ��������
    LL length;// ������� ���-�� ��������
    LL lines; // ���-�� �����
    LL maxLen; // ������������ ����� ������
    bool isDeleted;

    LL * start; // ������ �������� ����� �����
};

void initText(TextStruct * ts, WindowStruct * ws);
void deleteSymb(TextStruct * ts, WindowStruct * ws);
void deleteShift(TextStruct * ts, WindowStruct * ws, LL shift);
LL getLength(TextStruct * ts, LL i);
LL invisibleSymbs(TextStruct * ts, LL i);

#endif
