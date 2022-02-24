#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

// Стандартные библиотеки

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "textstruct.h"
#include "menu.h"
#include "windowstruct.h"
#include "caret.h"
#include "viewscroll.h"
#include "init.h"


LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

TCHAR szClassName[ ] = _T("Reader");

// Точка входа
//  hThisInstance - описатель экземпляра, уникальное число, идентифицирующее программу.
// hPrevInstance - предыдущий экземпляр.
// lpszArgument - это указатель на оканчивающуюся нулем строку, в которой содержатся любые параметры, переданные в программу из командной строки.
// nCmdShow - число, показывающее, каким должно быть выведено на экран окно в начальный момент.
int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;//описатель созданного окна
    MSG messages;
    WNDCLASSEX wincl;

    wincl.hInstance = hThisInstance;//описатель экземпляра программы
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;// адрес оконной процедуры, которая используется для всех окон, созданных на основе этого класса
    wincl.style = CS_HREDRAW | CS_VREDRAW; // Автоматическая перерисовка после WM_SIZE
    wincl.cbSize = sizeof (WNDCLASSEX);

    // Инициализация ресурсов
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);;
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = "MYMENU";

    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;

    wincl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

    if (!RegisterClassEx (&wincl))
        return 0;

    AllocConsole();
    freopen("CONOUT$", "w", stdout);


    hwnd = CreateWindowEx (
           0,// улучшенный стиль окна
           szClassName,// указатель на зарегистрированное имя класса
           _T("Reader"), // указатель на имя окна
           WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, // WS_VSCROLL | WS_HSCROLL - верт. и гориз. скроллы
           CW_USEDEFAULT,// горизонтальная позиция окна
           CW_USEDEFAULT,// вертикальная позиция окна
           640, 480, // Начальные размеры окна
           HWND_DESKTOP,//описатель родительского окна
           NULL,//описатель меню окна
           hThisInstance,//описатель экземпляра программы
           lpszArgument// указатель на данные создания окна
           );

    ShowWindow (hwnd, nCmdShow);
    UpdateWindow(hwnd);//приводит к немедленному вызову оконной процедуры с сообщением WM_PAINT

    // цикл обработки сообщений
    // GetMessage - извлекает сообщение из очереди сообщений, 1,2,3 параметры,
    //NULL или 0, показывают, что программа получает все сообщения от всех окон, созданных этой программой.
    while (GetMessage (&messages, NULL, 0, 0))
    {
        //TranslateMessage - передает структуру msg обратно в Windows для преобразования какого-либо сообщения с клавиатуры
        TranslateMessage(&messages);
        // DispatchMessage - также передает структуру msg обратно в Windows. Windows отправляет сообщение для его обработки соответствующей оконной процедуре, WndProc.
        DispatchMessage(&messages);
    }

    return messages.wParam;
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Структуры
    static TextStruct ts;
    static WindowStruct ws;
    char * fileName;
    TEXTMETRIC tm;

    switch(message){
        // Создание окна
        case WM_CREATE:{

            ws.hdc = GetDC(hwnd);//получение описателя контекста устройства
            SelectObject(ws.hdc, GetStockObject(SYSTEM_FIXED_FONT));// устанавливает шрифт в контексте устройства
            // Получение параметров шрифта
            GetTextMetrics(ws.hdc, &tm);
			ReleaseDC(hwnd, ws.hdc);

			ws.cxChar = tm.tmAveCharWidth;
			//tmExternalLeading - величина пустого пространства, которое разработчик шрифта установил для использования между строками символов
            ws.cyChar = tm.tmHeight + tm.tmExternalLeading;

            initStructs(&ts, &ws);
            setClientSize(hwnd, &ws);
			// Изначально режим отображения - без вёрстки
			ws.mode = NON_LAYOUT;
			checkMenu(hwnd, IDM_NON_LAYOUT);
            /*
            Открытие файла через аргументы командной строки:
            * Если файл существует и его можно открыть, то открывает
            * Иначе, открывается пустая рабочая область
            */
            CREATESTRUCT * p = (CREATESTRUCT *) lParam;
            fileName = (char *) p-> lpCreateParams;

            if(strlen(fileName) > 0){
                fileOpen(hwnd, &ts, &ws, fileName);

                SendMessage(hwnd, WM_SIZE, wParam, lParam);
            }

            break;
        }

        // Изменение размеров окна
        case WM_SIZE:{
            // Получение размеров окна
            setClientSize(hwnd, &ws);
            InvalidateRect(hwnd, NULL, TRUE);

            if(ts.text != NULL && ((ws.mode == LAYOUT) || ts.isDeleted)){
                initMode(&ts, &ws);
                initText(&ts, &ws);
                if(ts.isDeleted){
                    ts.isDeleted = true;
                }
            }

            initScrolls(hwnd, &ts, &ws);

            // Чтобы текст в конце файла не заходил за пределы рабочей области
            if(ws.iVscrollMax != 0){
                scrollToLine(hwnd, &ts, &ws, min(ws.linePos, ts.lines - (LL)ws.maxViewV));
            }else{
                scrollToLine(hwnd, &ts, &ws, 0);
            }

            if(ts.text != NULL){
               goToSymbPos(hwnd, &ts, &ws);
            }

            break;
        }

        // Если окно в фокусе, то отобразить каретку
        case WM_SETFOCUS:{
            InvalidateRect(hwnd,NULL,TRUE);
            if(ts.text != NULL){
                setCaret(hwnd, &ws);
            }

            break;
        }

        // Если окно не в фокусе, то убрать каретку
        case WM_KILLFOCUS:{
            if(ts.text != NULL){
                deleteCaret(hwnd);
            }

            break;
        }

        // Обработка нажатий клавиш
        case WM_KEYDOWN:{
            if(ts.text == NULL){
                break;
            }

            // Если каретка за пределами рабочей области, то возвращаемся к ней
            goToCaret(hwnd, &ts, &ws);

            switch(wParam){
                case VK_HOME:{
                    SendMessage(hwnd, WM_HSCROLL, SB_TOP, 0L);
                    ws.hCaretPos = 0;

                    break;
                }

                case VK_END:{
                    SendMessage(hwnd, WM_HSCROLL, SB_BOTTOM, 0L);
                    ws.hCaretPos = min(getLength(&ts, ws.linePos + ws.vCaretPos), ws.maxViewH) - 1;

                    break;
                }

                case VK_UP:{
                    // Достигнута вехняя граница рабочей области
                    if(ws.vCaretPos == 0){
                        SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0L);
                    }

                    // Не первая строка
                    if(0 < ws.linePos + ws.vCaretPos){
                        ws.vCaretPos -= 1;

                        // Выход за границы новой строки
                        if(getLength(&ts, ws.linePos + ws.vCaretPos) < ws.iHscrollPos + ws.hCaretPos){
                            SendMessage(hwnd, WM_KEYDOWN, VK_END, 0L);
                        }
                    }

                    break;
                }

                case VK_DOWN:{
                    // Достигнута нижняя граница рабочей области
                    if(ws.maxViewV - 1 <= ws.vCaretPos){
                        SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
                    }

                    // Не последняя строка
                    if(ws.linePos + ws.vCaretPos < ts.lines - 1){
                        ws.vCaretPos += 1;

                        // Выход за границы новой строки
                        if(getLength(&ts, ws.linePos + ws.vCaretPos) < ws.iHscrollPos + ws.hCaretPos){
                            SendMessage(hwnd, WM_KEYDOWN, VK_END, 0L);
                        }
                    }

                    break;
                }

                case VK_LEFT:{
                    // Не начало рабочей зоны
                    if(0 < ws.hCaretPos){
                        ws.hCaretPos -= 1;
                    }
                    // Достигнута левая граница рабочей области
                    else{
                        // Достигнут 0-й символ строки
                        if(ws.iHscrollPos == 0){
                            if(0 < ws.linePos + ws.vCaretPos){
                                SendMessage(hwnd, WM_KEYDOWN, VK_UP, 0L);
                                SendMessage(hwnd, WM_KEYDOWN, VK_END, 0L);
                            }
                        }
                        else{
                            SendMessage(hwnd, WM_HSCROLL, SB_LINEUP, 0L);
                        }
                    }

                    break;
                }

                case VK_RIGHT:{

                    // Конец строки
                    if(ws.iHscrollPos + ws.hCaretPos >= getLength(&ts, ws.linePos + ws.vCaretPos) - 1){
                        if(ws.linePos + ws.vCaretPos < ts.lines - 1){
                            SendMessage(hwnd, WM_KEYDOWN, VK_HOME, 0L);
                            SendMessage(hwnd, WM_KEYDOWN, VK_DOWN, 0L);
                        }
                    }else{
                        // Не конец рабочей зоны
                        if(ws.hCaretPos < ws.maxViewH - 1){
                            ws.hCaretPos += 1;
                        }
                        else{
                        // Достигнута правая граница рабочей области
                            SendMessage(hwnd, WM_HSCROLL, SB_LINEDOWN, 0L);
                        }
                    }


                    break;
                }

                case VK_PRIOR:{
                    SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0L);

                    if(ws.iHscrollPos + ws.hCaretPos > getLength(&ts, ws.linePos + ws.vCaretPos) - 1){
                        SendMessage(hwnd, WM_KEYDOWN, VK_END, 0L);
                    }

                    break;
                }

                case VK_NEXT:{
                    SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0L);

                    if(ws.iHscrollPos + ws.hCaretPos > getLength(&ts, ws.linePos + ws.vCaretPos) - 1){
                        SendMessage(hwnd, WM_KEYDOWN, VK_END, 0L);
                    };

                    break;
                }

                case VK_DELETE:{
                    if(ts.length == 0){
                        break;
                    }
                    deleteSymb(&ts, &ws);
                    ts.isDeleted = true;
                    SendMessage(hwnd, WM_KEYDOWN, VK_LEFT, 0L);
                    SendMessage(hwnd, WM_SIZE, 0L, 0L);
                    break;
                }
            }

            ws.caretPlace = getCaretPlace(&ts, &ws);
            SetCaretPos(ws.hCaretPos * ws.cxChar, ws.vCaretPos * ws.cyChar);

            break;
        }

        // Горизонтальный скролл
        case WM_HSCROLL:{
            switch(LOWORD(wParam)){
                case SB_TOP:{
                    ws.iHscrollInc = -ws.iHscrollPos;

                    break;
                }

                case SB_BOTTOM:{
                    ws.iHscrollInc = getLength(&ts, ws.linePos + ws.vCaretPos) - ws.iHscrollPos - ws.maxViewH;

                    break;
                }

                case SB_LINEUP:{
                    ws.iHscrollInc = -1;

                    break;
                }

                case SB_LINEDOWN:{
                    ws.iHscrollInc = 1;

                    break;
                }

                case SB_THUMBTRACK:{
                    ws.iHscrollInc = HIWORD(wParam) - ws.iHscrollPos;
                    ws.hCaretPos -= ws.iHscrollInc;

                    break;
                }

                default:{
                    ws.iHscrollInc = 0;

                    break;
                }
            }

            scrollH(hwnd, &ws);
            ws.caretPlace = getCaretPlace(&ts, &ws);

            break;
        }

        // Вертикальный скролл
        case WM_VSCROLL:{
            switch(LOWORD(wParam)){
                case SB_LINEUP:{
                    if(0 <= ws.linePos - 1){
                        ws.vCaretPos += 1;

                        scrollToLine(hwnd, &ts, &ws, ws.linePos - 1);
                    }

                    break;
                }

                case SB_LINEDOWN:{
                    if(ws.linePos < ts.lines - ws.maxViewV){
                        ws.vCaretPos -= 1;

                        scrollToLine(hwnd, &ts, &ws, ws.linePos + 1);
                    }

                    break;
                }

                case SB_PAGEUP:{
                    scrollToLine(hwnd, &ts, &ws, max(0, ws.linePos - ws.maxViewV));

                    break;
                }

                case SB_PAGEDOWN:{
                    scrollToLine(hwnd, &ts, &ws, min(max(0, ts.lines - ws.maxViewV), ws.linePos + ws.maxViewV));

                    break;
                }

                case SB_THUMBTRACK:{
                    ws.iVscrollInc = HIWORD(wParam) - ws.iVscrollPos;

                    LL prevLinePos = ws.linePos;
                    scrollV(hwnd, &ts, &ws);
                    ws.vCaretPos -= ws.linePos - prevLinePos;

                    break;
                }
            }

            ws.caretPlace = getCaretPlace(&ts, &ws);

            break;
        }

        // Нажатие на кнопки меню
        case WM_COMMAND:{
            switch(LOWORD(wParam)){
                case IDM_OPEN:{
                    OPENFILENAME ofn;
                    TCHAR szFile[MAX_PATH];

                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.lpstrFile[0] = '\0';
                    ofn.nMaxFile = sizeof(szFile);

                    // Только .txt файлы
                    ofn.lpstrFilter = _T("Текстовые файлы (*.txt)\0*.txt\0");
                    ofn.nFilterIndex = 1;
                    ofn.lpstrFileTitle = NULL;
                    ofn.nMaxFileTitle = 255;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if(GetOpenFileName(&ofn) == TRUE){
                        initStructs(&ts, &ws);
                        deleteCaret(hwnd);
                        InvalidateRect(hwnd, NULL, TRUE);

                        fileName = ofn.lpstrFile;
                        fileOpen(hwnd, &ts, &ws, fileName);

                        setCaret(hwnd, &ws);

                        SendMessage(hwnd, WM_SIZE, 0L, 0L);
                    }

                    break;
                }

                case IDM_SAVE:{
                    OPENFILENAME ofn;
                    TCHAR szFile[MAX_PATH];

                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.lpstrFile[0] = '\0';
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFileTitle = NULL;
                    ofn.nMaxFileTitle = 255;
                    ofn.lpstrDefExt = "txt";
                    ofn.Flags = OFN_OVERWRITEPROMPT;

                    if( GetSaveFileName(&ofn) == TRUE){

                        fileName = ofn.lpstrFile;
                        fileSave(&ts, &ws, fileName);
                    }

                    break;
                }

                case IDM_QUIT:{
                    SendMessage(hwnd, WM_DESTROY, 0L, 0L);

                    break;
                }

                case IDM_LAYOUT:{
                    changeMode(hwnd, &ts, &ws, LAYOUT);

                    SendMessage(hwnd, WM_SIZE, 0L, 0L);

                    break;
                }

                case IDM_NON_LAYOUT:{
                    changeMode(hwnd, &ts, &ws, NON_LAYOUT);

                    SendMessage(hwnd, WM_SIZE, 0L, 0L);

                    break;
                }

                case IDM_ABOUT:{
                    info("This is a simple text reader", "About program");

                    break;
                }
            }

            break;
        }

        // Прокрутка колёсиком мыши
        case WM_MOUSEWHEEL:{
            if(ts.text == NULL){
                break;
            }

            if(GET_WHEEL_DELTA_WPARAM(wParam) < 0){
                SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
            }
            else{
                SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0L);
            }

            break;
        }

        // Отрисовка
        case WM_PAINT:{
            if(ts.text == NULL){
                break;
            }

            //printf("PAINT\n");

            ws.hdc = BeginPaint(hwnd, &ws.ps);//При обработке вызова BeginPaint, Windows обновляет фон рабочей области, если он еще не обновлен

            showText(&ts, &ws);

            EndPaint(hwnd, &ws.ps);

            break;
        }

        // Закрытия окна
        case WM_DESTROY:{
            // Очистка памяти, выделенной под структуры
            initStructs(&ts, &ws);
            deleteCaret(hwnd);

            PostQuitMessage(0);

            break;
        }

        default:{
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return 0;
}
