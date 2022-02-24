#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include <windows.h>
#include "windowstruct.h"
#include "servicestruct.h"

#define IDM_OPEN 1
#define IDM_QUIT 3
#define IDM_LAYOUT 4
#define IDM_NON_LAYOUT 5
#define IDM_ABOUT 6
#define IDM_SAVE 7

void checkMenu(HWND hwnd, int item);

#endif
