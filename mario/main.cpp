#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <curses.h>
#include <cstring>
#include <stdbool.h>

const int mapwidth = 80;
const int mapHeight = 24;

typedef struct SObject{
    float x,y;
    float width, height;
    float vertSpeed;
    bool IsFly;
    char cType;
    float horizSpeed;
} TObject;

char map[mapHeight][mapwidth+1];
TObject mario;

TObject *brick = NULL;
int brickLength;

TObject *moving = NULL;
int movingLength;

int level = 1;
int score;
int maxlvl;

void ClearMap () {
    for (int i = 0; i < mapwidth; i++)
        map[0][i] = ' ';
    map[0][mapwidth] = '\0';
    for (int j = 1; j < mapHeight; j++)
        strcpy( map[j], map[0]);
}

void ShowMap(){
    map [mapHeight - 1][mapwidth - 1] = '\0';
    for (int j = 0; j <mapHeight; j++)
        printw("%s\n",map[j]);
}

void SetObjectPos(TObject *obj,float xPos, float yPos){
    obj->x = xPos;
    obj->y = yPos;
}

void InitObject(TObject *obj, float xPos, float yPos, float oWidth, float oHeight, char inType){
    SetObjectPos(obj, xPos, yPos);
    obj->width = oWidth;
    obj->height = oHeight;
    obj->vertSpeed = 0;
    obj->cType = inType;
    obj->horizSpeed = 0.1;
}

void CreateLevel (int lvl);

void PlayerDead(){
    napms(500);
    CreateLevel(level);
}

bool IsCollision(TObject o1, TObject o2);
void CreateLevel(int lvl);
TObject *GetNewMoving();

void VertMoveObject(TObject *obj){
    obj->IsFly = true;
    obj->vertSpeed += 0.02;
    SetObjectPos(obj, obj->x, obj->y + obj->vertSpeed);

    for (int i = 0; i < brickLength; i++)
        if (IsCollision(*obj, brick[i]))
        {
            if (obj->vertSpeed > 0)
                obj->IsFly = false;
            if ((brick[i].cType == '?') && (obj->vertSpeed < 0) && (obj == &mario)){
                brick[i].cType = '-';
                InitObject(GetNewMoving(), brick[i].x, brick[i].y-2,1,1,'$');
                moving[movingLength -1].vertSpeed = -0.5;
            }
            
            obj->y -=obj->vertSpeed;
            obj->vertSpeed = 0;

            if (brick[i].cType == '+'){
                level++;
                if (level > maxlvl) level = 1;
                napms(500);
                CreateLevel(level);
            }
            break;
        }
}

void DeleteMoving(int i){
    movingLength--;
    moving[i] = moving[movingLength];
    moving = (TObject*)realloc(moving, sizeof(TObject) *movingLength);
}

void MarioCollision(){
    for (int i = 0; i < movingLength; i++)
        if (IsCollision(mario, moving[i])){
            if (moving[i].cType == 'o'){
                if ( (mario.IsFly == true) && (mario.vertSpeed > 0) && (mario.y + mario.height < moving[i].y + moving[i].height * 0.5)){
                score += 50;
                DeleteMoving(i);
                i--;
                continue;
            }
            else 
                PlayerDead();
            }
            if (moving[i].cType == '$'){
                score += 100;
                DeleteMoving(i);
                i--;
                continue;
            }
        }
}

void HorizonMoveObject(TObject *obj){
    obj->x += obj->horizSpeed;
    
    for (int i = 0; i < brickLength; i++)
        if (IsCollision(*obj, brick[i])){
            obj->x -= obj->horizSpeed;
            obj->horizSpeed = -obj->horizSpeed;
            return;
        }
    if (obj->cType == 'o'){
        TObject tmp = *obj;
        VertMoveObject(&tmp);
        if (tmp.IsFly == true){
            obj->x -= obj->horizSpeed;
            obj->horizSpeed = -obj->horizSpeed;
        } 
    }
}

bool IsPosInMap(int x, int y){
    return ( (x >= 0) && (x<mapwidth) && (y >= 0) && (y < mapHeight) );
}

void PutObjectOnMap(TObject obj){
    int ix = (int)round(obj.x);
    int iy = (int)round(obj.y);
    int iWidth = (int)round(obj.width);
    int iHeight = (int)round(obj.height);
    
    for (int i = ix; i < (ix + iWidth); i++)
        for (int j = iy; j < (iy + iHeight); j++)
            if (IsPosInMap(i,j))
                if (map[j][i] == ' ') 
                    map[j][i] = obj.cType;
}

void HorizonMoveMap(float dx){
    mario.x -=dx;
    for (int i = 0; i < brickLength; i++)
        if (IsCollision(mario, brick[i])){
            mario.x += dx;
            return;
        }
    mario.x += dx;

    for (int i = 0; i < brickLength; i++)
        brick[i].x += dx;

    for (int i = 0; i < movingLength; i++)
        moving[i].x += dx;
}

bool IsCollision(TObject o1, TObject o2){
    return ((o1.x + o1.width) > o2.x) && (o1.x < (o2.x + o2.width)) &&
            ((o1.y + o1.height) > o2.y) && (o1.y < (o2.y + o2.height));
}

TObject *GetNewBrick(){
    brickLength++;
    brick = (TObject*)realloc(brick, sizeof(TObject) *brickLength);
    return brick + brickLength - 1;
}

TObject *GetNewMoving(){
    movingLength++;
    moving = (TObject*)realloc(moving, sizeof(TObject) *movingLength);
    return moving + movingLength - 1;
}

void PutScoreOnMap(){
    char c[30];
    snprintf(c, sizeof(c), "Score: %d Level: %d", score, level);
    int len = strlen(c);
    for (int i = 0; i< len; i++){
        if (i+5 < mapwidth) {
            map[0][i+5] = c[i];
        }
    }
}

void CreateLevel (int lvl){
    brickLength = 0;
    brick = (TObject*)realloc(brick, 0);
    movingLength = 0;
    moving = (TObject*)realloc(moving, 0);

    InitObject(&mario, 5, 5, 2, 2, '@');
    score = 0;

    if (lvl == 1){
        InitObject(GetNewBrick(), 0, 20, 80, 4, '#');
        InitObject(GetNewBrick(), 10, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 20, 10, 5, 5, '#');
        InitObject(GetNewBrick(), 30, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 40, 10, 5, 5, '#');
        InitObject(GetNewBrick(), 50, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 60, 10, 5, 5, '#');
        InitObject(GetNewBrick(), 70, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 15, 10, 2, 2, '?');
        InitObject(GetNewBrick(), 45, 10, 2, 2, '?');
        InitObject(GetNewBrick(), 75, 5, 2, 2, '+');

        InitObject(GetNewMoving(), 25, 19, 1, 1, 'o');
        InitObject(GetNewMoving(), 55, 19, 1, 1, 'o');
    }

    if (lvl == 2){
        InitObject(GetNewBrick(), 0, 20, 80, 4, '#');
        InitObject(GetNewBrick(), 10, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 20, 10, 5, 5, '#');
        InitObject(GetNewBrick(), 30, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 40, 10, 5, 5, '#');
        InitObject(GetNewBrick(), 50, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 60, 10, 5, 5, '#');
        InitObject(GetNewBrick(), 70, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 15, 10, 2, 2, '?');
        InitObject(GetNewBrick(), 45, 10, 2, 2, '?');
        InitObject(GetNewBrick(), 75, 5, 2, 2, '+');

        InitObject(GetNewMoving(), 15, 19, 1, 1, 'o');
        InitObject(GetNewMoving(), 35, 19, 1, 1, 'o');
        InitObject(GetNewMoving(), 55, 19, 1, 1, 'o');
        InitObject(GetNewMoving(), 75, 19, 1, 1, 'o');
    }

    if (lvl == 3){
        InitObject(GetNewBrick(), 0, 20, 80, 4, '#');
        InitObject(GetNewBrick(), 10, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 20, 10, 5, 5, '#');
        InitObject(GetNewBrick(), 30, 5, 5, 5, '#');
        InitObject(GetNewBrick(), 40, 10, 5, 5, '#');
        InitObject(GetNewBrick(), 50, 15, 5, 5, '#');
        InitObject(GetNewBrick(), 60, 10, 5, 5, '#');
        InitObject(GetNewBrick(), 70, 5, 5, 5, '#');
        InitObject(GetNewBrick(), 15, 10, 2, 2, '?');
        InitObject(GetNewBrick(), 45, 10, 2, 2, '?');
        InitObject(GetNewBrick(), 75, 5, 2, 2, '+');

        InitObject(GetNewMoving(), 10, 19, 1, 1, 'o');
        InitObject(GetNewMoving(), 30, 14, 1, 1, 'o');
        InitObject(GetNewMoving(), 50, 19, 1, 1, 'o');
        InitObject(GetNewMoving(), 70, 14, 1, 1, 'o');
    }
    maxlvl = 3;
}

int main(){
    initscr();
    curs_set(0);
    noecho();
    nodelay(stdscr, true);
    keypad(stdscr,true);

    int rows,cols;
    getmaxyx(stdscr,rows,cols);

    if (rows < mapHeight || cols < mapwidth){
        endwin();
        printf("Terminal is too small. Minimum size: %dx%d\n", mapwidth, mapHeight);
        printf("Current size: %dx%d\n", cols, rows);
        return 1;
    }

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
    }
    
    CreateLevel(level);

    int key = -1;

    do{
        ClearMap();

        key = getch();

        if (!mario.IsFly && key == ' ') 
            mario.vertSpeed = -0.8;
        if (key == 'a') 
            HorizonMoveMap(0.5);
        if (key == 'd') 
            HorizonMoveMap(-0.5);

        if (mario.y > mapHeight) 
            PlayerDead();

        VertMoveObject(&mario);
        MarioCollision();

        for (int i = 0; i < brickLength; i++)
            PutObjectOnMap(brick[i]);
        for (int i = 0; i < movingLength; i++)
        {
            VertMoveObject(moving + i);
            HorizonMoveObject(moving + i);
            if (moving[i].y > mapHeight){
                DeleteMoving(i);
                i--;
                continue;
            }
            PutObjectOnMap(moving[i]);
        }
        PutObjectOnMap(mario);
        PutScoreOnMap();

        clear();
        ShowMap();

        napms(30);
    }
    while (key != 27);

    endwin();
    
    return 0;
}