#ifndef INIT_H_INCLUDED
#define INIT_H_INCLUDED

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "textstruct.h"
#include "windowstruct.h"
#include "menu.h"
#include "caret.h"

void changeMode(HWND hwnd, TextStruct * ts, WindowStruct * ws, int mode);
void initScrolls(HWND hwnd, TextStruct * ts, WindowStruct * ws);
void initStructs(TextStruct * ts, WindowStruct * ws);
void initMode(TextStruct * ts, WindowStruct * ws);
void setClientSize(HWND hwnd, WindowStruct * ws);
void fileOpen(HWND hwnd, TextStruct * ts, WindowStruct * ws, char * fileName);
void fileSave(TextStruct * ts, WindowStruct * ws, char * fileName);
void showText(TextStruct * ts, WindowStruct * ws);
void info(char * str, char * caption);
void error(char * str);

#endif
