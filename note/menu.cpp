#include "menu.h"

// Пометить "галочкой" пункт меню
void checkMenu(HWND hwnd, int item){
    HMENU hMenu = GetMenu(hwnd);

    switch(item){
        case LAYOUT:{
            CheckMenuItem(hMenu, IDM_LAYOUT, MF_CHECKED);
            CheckMenuItem(hMenu, IDM_NON_LAYOUT, MF_UNCHECKED);

            break;
        }

        case NON_LAYOUT:{
            CheckMenuItem(hMenu, IDM_NON_LAYOUT, MF_CHECKED);
            CheckMenuItem(hMenu, IDM_LAYOUT, MF_UNCHECKED);

            break;
        }

        default:{
            CheckMenuItem(hMenu, item, MF_CHECKED);
        }
    }
}

