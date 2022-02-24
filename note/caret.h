#ifndef CARET_H_INCLUDED
#define CARET_H_INCLUDED

#include <windows.h>
#include "textstruct.h"
#include "windowstruct.h"
#include "viewscroll.h"

void goToCaret(HWND hwnd, TextStruct * ts, WindowStruct * ws);
void setCaret(HWND hwnd, WindowStruct * ws);
void deleteCaret(HWND hwnd);
LL getCaretPlace(TextStruct * ts, WindowStruct * ws);

#endif
