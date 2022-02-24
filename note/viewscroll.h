#ifndef VIEWSCROLL_H_INCLUDED
#define VIEWSCROLL_H_INCLUDED

#include <windows.h>
#include "textstruct.h"
#include "windowstruct.h"
#include "caret.h"

void scrollToLine(HWND hwnd, TextStruct * ts, WindowStruct * ws, LL linePos);
int getMap(TextStruct ts, WindowStruct ws, int scrollPos);
int getInverseMap(TextStruct ts, WindowStruct ws, LL linePos);
void scrollV(HWND hwnd, TextStruct * ts, WindowStruct * ws);
void scrollH(HWND hwnd, WindowStruct * ws);
void goToSymbPos(HWND hwnd, TextStruct * ts, WindowStruct * ws);

#endif
