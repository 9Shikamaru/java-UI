#include "textstruct.h"
#include "windowstruct.h"

// Инициализация текста
void initText(TextStruct * ts, WindowStruct * ws){
    LL lineSymbCount = 0;

    // Подсчёт кол-ва строк
     for(LL i = 0; i < ts->length; i++){
        if(ts->text[i] == '\n' ||
           (ws->mode == LAYOUT && lineSymbCount >= ws->maxViewH - 1)){
            lineSymbCount = 0;
            ts->lines++;
        }
        else{
            lineSymbCount++;
        }
    }

    ts->start = (LL *) malloc((ts->lines + 1) * sizeof(LL));
    ts->start[0] = 0;
    ts->start[ts->lines] = ts->length;

    LL line = 1;
    // Инициализация массива указателей на начала строк
    for(LL i = 0; i < ts->length - 1; i++){
        if(ts->text[i] == '\n' ||
           (ws->mode == LAYOUT && lineSymbCount >= ws->maxViewH - 1)){
            lineSymbCount = 0;

            ts->start[line] = i + 1;
            line++;
        }
        else{
            lineSymbCount++;
        }
    }

    // Подсчёт максимальной длины строки
    for(LL i = 0; i < ts->lines; i++){
        ts->maxLen = max(ts->maxLen, getLength(ts, i));
    }
}

// удаление символа
void deleteSymb(TextStruct * ts, WindowStruct * ws){
    LL curLine = ws->linePos + ws->vCaretPos;
    LL invisibleSymbsNum = invisibleSymbs(ts, curLine);
    // удаление пустой строки
    if((ts->start[curLine + 1] - ts->start[curLine]) == invisibleSymbsNum){
        deleteShift(ts, ws, invisibleSymbsNum);
        return;
    // удаление строки с одним символом
    }
    if((ts->start[curLine + 1] - ts->start[curLine]) == invisibleSymbsNum + 1){
        deleteShift(ts, ws, invisibleSymbsNum + 1);
        return;
    }
    deleteShift(ts, ws, 1);
}

void deleteShift(TextStruct * ts, WindowStruct * ws, LL shift){
    for(LL i=ws->caretPlace; i<ts->length - shift;i++){
        ts->text[i] = ts->text[i + shift];
    }
    ts->length-=shift;
}

// Длина строки
LL getLength(TextStruct * ts, LL i){
    LL len = ts->start[i + 1] - ts->start[i];
    LL visibleLen = len - invisibleSymbs(ts, i);

    if(visibleLen <= 0){
        return 1;
    }else{
        return visibleLen;
    }
}

LL invisibleSymbs(TextStruct * ts, LL i){
    LL invisibleSymbCount = 0;
    LL pos = ts->start[i + 1] - 1;
    int invisibleSymbsNum = 4;
    char invisibleSymbsArr [4] = {' ','\n','\r','\t'};
    bool isInvisible;
    while(pos > ts->start[i]){
        isInvisible = false;
        for(int j = 0;j < invisibleSymbsNum;j++){
            if(ts->text[pos] == invisibleSymbsArr[j]){
                isInvisible = true;
                break;
            }
        }
        if(isInvisible){
            invisibleSymbCount++;
            pos--;
        }else{
            break;
        }
    }
    return invisibleSymbCount;
}
