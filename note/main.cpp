#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

// ����������� ����������

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

// ����� �����
//  hThisInstance - ��������� ����������, ���������� �����, ���������������� ���������.
// hPrevInstance - ���������� ���������.
// lpszArgument - ��� ��������� �� �������������� ����� ������, � ������� ���������� ����� ���������, ���������� � ��������� �� ��������� ������.
// nCmdShow - �����, ������������, ����� ������ ���� �������� �� ����� ���� � ��������� ������.
int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;//��������� ���������� ����
    MSG messages;
    WNDCLASSEX wincl;

    wincl.hInstance = hThisInstance;//��������� ���������� ���������
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;// ����� ������� ���������, ������� ������������ ��� ���� ����, ��������� �� ������ ����� ������
    wincl.style = CS_HREDRAW | CS_VREDRAW; // �������������� ����������� ����� WM_SIZE
    wincl.cbSize = sizeof (WNDCLASSEX);

    // ������������� ��������
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
           0,// ���������� ����� ����
           szClassName,// ��������� �� ������������������ ��� ������
           _T("Reader"), // ��������� �� ��� ����
           WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, // WS_VSCROLL | WS_HSCROLL - ����. � �����. �������
           CW_USEDEFAULT,// �������������� ������� ����
           CW_USEDEFAULT,// ������������ ������� ����
           640, 480, // ��������� ������� ����
           HWND_DESKTOP,//��������� ������������� ����
           NULL,//��������� ���� ����
           hThisInstance,//��������� ���������� ���������
           lpszArgument// ��������� �� ������ �������� ����
           );

    ShowWindow (hwnd, nCmdShow);
    UpdateWindow(hwnd);//�������� � ������������ ������ ������� ��������� � ���������� WM_PAINT

    // ���� ��������� ���������
    // GetMessage - ��������� ��������� �� ������� ���������, 1,2,3 ���������,
    //NULL ��� 0, ����������, ��� ��������� �������� ��� ��������� �� ���� ����, ��������� ���� ����������.
    while (GetMessage (&messages, NULL, 0, 0))
    {
        //TranslateMessage - �������� ��������� msg ������� � Windows ��� �������������� ������-���� ��������� � ����������
        TranslateMessage(&messages);
        // DispatchMessage - ����� �������� ��������� msg ������� � Windows. Windows ���������� ��������� ��� ��� ��������� ��������������� ������� ���������, WndProc.
        DispatchMessage(&messages);
    }

    return messages.wParam;
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // ���������
    static TextStruct ts;
    static WindowStruct ws;
    char * fileName;
    TEXTMETRIC tm;

    switch(message){
        // �������� ����
        case WM_CREATE:{

            ws.hdc = GetDC(hwnd);//��������� ��������� ��������� ����������
            SelectObject(ws.hdc, GetStockObject(SYSTEM_FIXED_FONT));// ������������� ����� � ��������� ����������
            // ��������� ���������� ������
            GetTextMetrics(ws.hdc, &tm);
			ReleaseDC(hwnd, ws.hdc);

			ws.cxChar = tm.tmAveCharWidth;
			//tmExternalLeading - �������� ������� ������������, ������� ����������� ������ ��������� ��� ������������� ����� �������� ��������
            ws.cyChar = tm.tmHeight + tm.tmExternalLeading;

            initStructs(&ts, &ws);
            setClientSize(hwnd, &ws);
			// ���������� ����� ����������� - ��� ������
			ws.mode = NON_LAYOUT;
			checkMenu(hwnd, IDM_NON_LAYOUT);
            /*
            �������� ����� ����� ��������� ��������� ������:
            * ���� ���� ���������� � ��� ����� �������, �� ���������
            * �����, ����������� ������ ������� �������
            */
            CREATESTRUCT * p = (CREATESTRUCT *) lParam;
            fileName = (char *) p-> lpCreateParams;

            if(strlen(fileName) > 0){
                fileOpen(hwnd, &ts, &ws, fileName);

                SendMessage(hwnd, WM_SIZE, wParam, lParam);
            }

            break;
        }

        // ��������� �������� ����
        case WM_SIZE:{
            // ��������� �������� ����
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

            // ����� ����� � ����� ����� �� ������� �� ������� ������� �������
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

        // ���� ���� � ������, �� ���������� �������
        case WM_SETFOCUS:{
            InvalidateRect(hwnd,NULL,TRUE);
            if(ts.text != NULL){
                setCaret(hwnd, &ws);
            }

            break;
        }

        // ���� ���� �� � ������, �� ������ �������
        case WM_KILLFOCUS:{
            if(ts.text != NULL){
                deleteCaret(hwnd);
            }

            break;
        }

        // ��������� ������� ������
        case WM_KEYDOWN:{
            if(ts.text == NULL){
                break;
            }

            // ���� ������� �� ��������� ������� �������, �� ������������ � ���
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
                    // ���������� ������ ������� ������� �������
                    if(ws.vCaretPos == 0){
                        SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0L);
                    }

                    // �� ������ ������
                    if(0 < ws.linePos + ws.vCaretPos){
                        ws.vCaretPos -= 1;

                        // ����� �� ������� ����� ������
                        if(getLength(&ts, ws.linePos + ws.vCaretPos) < ws.iHscrollPos + ws.hCaretPos){
                            SendMessage(hwnd, WM_KEYDOWN, VK_END, 0L);
                        }
                    }

                    break;
                }

                case VK_DOWN:{
                    // ���������� ������ ������� ������� �������
                    if(ws.maxViewV - 1 <= ws.vCaretPos){
                        SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
                    }

                    // �� ��������� ������
                    if(ws.linePos + ws.vCaretPos < ts.lines - 1){
                        ws.vCaretPos += 1;

                        // ����� �� ������� ����� ������
                        if(getLength(&ts, ws.linePos + ws.vCaretPos) < ws.iHscrollPos + ws.hCaretPos){
                            SendMessage(hwnd, WM_KEYDOWN, VK_END, 0L);
                        }
                    }

                    break;
                }

                case VK_LEFT:{
                    // �� ������ ������� ����
                    if(0 < ws.hCaretPos){
                        ws.hCaretPos -= 1;
                    }
                    // ���������� ����� ������� ������� �������
                    else{
                        // ��������� 0-� ������ ������
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

                    // ����� ������
                    if(ws.iHscrollPos + ws.hCaretPos >= getLength(&ts, ws.linePos + ws.vCaretPos) - 1){
                        if(ws.linePos + ws.vCaretPos < ts.lines - 1){
                            SendMessage(hwnd, WM_KEYDOWN, VK_HOME, 0L);
                            SendMessage(hwnd, WM_KEYDOWN, VK_DOWN, 0L);
                        }
                    }else{
                        // �� ����� ������� ����
                        if(ws.hCaretPos < ws.maxViewH - 1){
                            ws.hCaretPos += 1;
                        }
                        else{
                        // ���������� ������ ������� ������� �������
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

        // �������������� ������
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

        // ������������ ������
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

        // ������� �� ������ ����
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

                    // ������ .txt �����
                    ofn.lpstrFilter = _T("��������� ����� (*.txt)\0*.txt\0");
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

        // ��������� �������� ����
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

        // ���������
        case WM_PAINT:{
            if(ts.text == NULL){
                break;
            }

            //printf("PAINT\n");

            ws.hdc = BeginPaint(hwnd, &ws.ps);//��� ��������� ������ BeginPaint, Windows ��������� ��� ������� �������, ���� �� ��� �� ��������

            showText(&ts, &ws);

            EndPaint(hwnd, &ws.ps);

            break;
        }

        // �������� ����
        case WM_DESTROY:{
            // ������� ������, ���������� ��� ���������
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
