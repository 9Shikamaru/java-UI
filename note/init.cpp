#include "init.h"
#include "caret.h"

// отрисовка текста
void showText(TextStruct * ts, WindowStruct * ws){
    LL  y, bias, len, lastLine;

    SelectObject(ws->hdc, GetStockObject(SYSTEM_FIXED_FONT));

    lastLine = min(ws->linePos + ws->maxViewV + 1, ts->lines);

    for(LL i = ws->linePos; i < lastLine; i++){
        bias = ts->start[i] + ws->iHscrollPos;
        len = min(ws->maxViewH + 1, max(getLength(ts, i) - ws->iHscrollPos, 0));
        y = ws->cyChar * (i - ws->linePos);

        TextOut(ws->hdc, 0, y, ts->text + bias, len);
    }
}

// Открытие файла и инициализация структур
void fileOpen(HWND hwnd, TextStruct * ts, WindowStruct * ws, char * fileName){
    char * buf = NULL;
    FILE * file = fopen(fileName, "rb");

    if(file != NULL){
        fseek(file, 0, SEEK_END);
        LL sz = ftell(file);
        ts->realLength = sz + 2;
        fseek(file, 0, SEEK_SET);

        buf = (char *) calloc(ts->realLength, sizeof(char));

        if(buf != NULL){
            fread(buf, sizeof(char), sz, file);
            buf[sz] = '\r';
            buf[sz + 1] = '\n';
        }
        else{
            error("Cannot allocate buffer!");
        }
    }
    else{
        error("Cannot open file!");
    }

    fclose(file);

    if(buf == NULL){
        return;
    }

    ts->length = ts->realLength;
    ts->text = buf;
    initText(ts, ws);
}

// Сохранение файла
void fileSave(TextStruct * ts, WindowStruct * ws, char * fileName){
    FILE * file = fopen(fileName, "w");
    LL out;
    if (file != NULL) {
       out = fwrite(ts->text, sizeof(char), ts->length, file);
    }else{
        error("Cannot open file!");
    }
    fclose(file);
}

// Задание скролов

void initScrolls(HWND hwnd, TextStruct * ts, WindowStruct * ws){
    //SHRT_MAX - Maximum value for an object of type short int
    ws->iVscrollMax = min(SHRT_MAX, max(ts->lines - (LL)ws->maxViewV, 0));
    ws->iVscrollPos = min(ws->iVscrollPos, ws->iVscrollMax);

    SetScrollRange(hwnd, SB_VERT, 0, ws->iVscrollMax, FALSE);
    SetScrollPos(hwnd, SB_VERT, ws->iVscrollPos, TRUE);

    // Если режим без вёрстки, то задаём диапозон для горизонтального скролла
    if(ws->mode == NON_LAYOUT){
        ws->iHscrollMax = max(0, ts->maxLen - (LL)ws->maxViewH + 1);
        ws->iHscrollPos = min(ws->iHscrollPos, ws->iHscrollMax);
    }
    else{
        ws->iHscrollMax = 0;
        ws->iHscrollPos = 0;
    }

    SetScrollRange(hwnd, SB_HORZ, 0, ws->iHscrollMax, FALSE);
    SetScrollPos(hwnd, SB_HORZ, ws->iHscrollPos, TRUE);
}

// Очистка памяти под все структуры
void initStructs(TextStruct * ts, WindowStruct * ws){
    if(ts->text != NULL){
        free(ts->text);
        ts->text = NULL;
    }

    ws->linePos = 0;
    ws->lineInc = 0;

    ws->hCaretPos = 0;
    ws->vCaretPos = 0;
    ws->caretPlace = 0;

    ts->length = 0;
    ts->realLength = 0;
    ts->isDeleted = false;

    initMode(ts, ws);
}

// Очистка памяти под структуры при смене режима
void initMode(TextStruct * ts, WindowStruct * ws){
    if(ts->start != NULL){
        free(ts->start);
        ts->start = NULL;
    }
    ts->lines = 0;
    ts->maxLen = 0;
}

// Нажатие на один из пунктов меню для смена режима
void changeMode(HWND hwnd, TextStruct * ts, WindowStruct * ws, int mode){
    if(ws->mode == mode){
        return;
    }

    ws->mode = mode;
    checkMenu(hwnd, mode);

    if(ts->text != NULL){
        ws->caretPlace = getCaretPlace(ts, ws);
        initMode(ts, ws);
        initText(ts, ws);
    }
}

void setClientSize(HWND hwnd, WindowStruct * ws){
    RECT rect;// структура "прямоугольник",. Она имеет четыре поля типа LONG, имена полей: left, top, right и bottom.
    GetClientRect(hwnd, &rect);//помещает в эти четыре поля размер рабочей области окна. Поля left и top всегда устанавливаются в 0

    ws->cxClient = rect.right - rect.left;
    ws->cyClient = rect.bottom - rect.top;
    ws->maxViewH = ws->cxClient/ws->cxChar;
    ws->maxViewV = ws->cyClient/ws->cyChar;
}


void info(char * str, char * caption){
    MessageBox(NULL, str, caption, MB_ICONINFORMATION);
}

void error(char * str){
    MessageBox(NULL, str, "Error", MB_ICONERROR);
}
