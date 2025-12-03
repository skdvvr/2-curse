#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <curses.h>
#include <cstring>
#include <stdbool.h>

// =====================================================
//                      CONSTANTS
// =====================================================

const int MAP_WIDTH = 80;
const int MAP_HEIGHT = 24;

// Прототипы для "моста" CreateLevel
class LevelManager;
LevelManager *gLevelMgr = NULL;
void CreateLevelBridge(int lvl);

// =====================================================
//                      CLASS OBJECT
// =====================================================

class TObject {
public:
    float x, y;
    float width, height;
    float vertSpeed;
    bool IsFly;
    char cType;
    float horizSpeed;

    TObject() {
        x = y = 0;
        width = height = 0;
        vertSpeed = 0;
        IsFly = false;
        cType = ' ';
        horizSpeed = 0.1f;
    }

    TObject(float px, float py, float w, float h, char t) {
        x = px;
        y = py;
        width = w;
        height = h;
        cType = t;
        vertSpeed = 0;
        IsFly = false;
        horizSpeed = 0.1f;
    }

    void SetPos(float px, float py) {
        x = px;
        y = py;
    }

    void Init(float px, float py, float w, float h, char t) {
        x = px;
        y = py;
        width = w;
        height = h;
        cType = t;
        vertSpeed = 0;
        horizSpeed = 0.1f;
        IsFly = false;
    }
};

// =====================================================
//                        MAP CLASS
// =====================================================

class GameMap {
public:
    char data[MAP_HEIGHT][MAP_WIDTH + 1];

    GameMap() {
        Clear();
    }

    void Clear() {
        for (int i = 0; i < MAP_WIDTH; i++)
            data[0][i] = ' ';
        data[0][MAP_WIDTH] = '\0';

        for (int j = 1; j < MAP_HEIGHT; j++)
            strcpy(data[j], data[0]);
    }

    void Show() {
        data[MAP_HEIGHT - 1][MAP_WIDTH - 1] = '\0';
        for (int j = 0; j < MAP_HEIGHT; j++)
            printw("%s", data[j]);
    }

    bool InBounds(int x, int y) {
        return (x >= 0 && x < MAP_WIDTH &&
                y >= 0 && y < MAP_HEIGHT);
    }

    void PutObject(const TObject &obj) {
        int ix = (int)round(obj.x);
        int iy = (int)round(obj.y);
        int iWidth  = (int)round(obj.width);
        int iHeight = (int)round(obj.height);

        for (int i = ix; i < ix + iWidth; i++) {
            for (int j = iy; j < iy + iHeight; j++) {
                if (InBounds(i, j)) {
                    if (data[j][i] == ' ')
                        data[j][i] = obj.cType;
                }
            }
        }
    }

    void PutScore(int score, int level) {
        char buf[30];
        snprintf(buf, sizeof(buf), "Score: %d Level: %d", score, level);
        int len = strlen(buf);

        for (int i = 0; i < len; i++)
            if (i + 5 < MAP_WIDTH)
                data[0][i + 5] = buf[i];
    }
};

// =====================================================
//                    OBJECT MANAGER
// =====================================================

class ObjectManager {
public:
    TObject mario;

    TObject *brick;
    int brickCount;

    TObject *moving;
    int movingCount;

    int score;
    int level;
    int maxlvl;

    GameMap *map;

    ObjectManager(GameMap *m) {
        map = m;
        brick = NULL;
        moving = NULL;
        brickCount = 0;
        movingCount = 0;
        score = 0;
        level = 1;
        maxlvl = 1;
    }

    TObject* NewBrick() {
        brickCount++;
        brick = (TObject*)realloc(brick, sizeof(TObject) * brickCount);
        return &brick[brickCount - 1];
    }

    TObject* NewMoving() {
        movingCount++;
        moving = (TObject*)realloc(moving, sizeof(TObject) * movingCount);
        return &moving[movingCount - 1];
    }

    void DeleteMoving(int i) {
        if (movingCount <= 0) return;
        movingCount--;
        moving[i] = moving[movingCount];
        moving = (TObject*)realloc(moving, sizeof(TObject) * movingCount);
    }

    bool IsCollision(const TObject &a, const TObject &b) {
        return ( (a.x + a.width) > b.x ) &&
               (  a.x < (b.x + b.width)) &&
               ( (a.y + a.height) > b.y ) &&
               (  a.y < (b.y + b.height) );
    }

    void PlayerDead(void (*CreateLevel)(int)) {
        napms(500);
        if (CreateLevel != NULL) CreateLevel(level);
    }

    void VertMove(TObject *obj, void (*CreateLevel)(int)) {
        obj->IsFly = true;
        obj->vertSpeed += 0.02f;
        obj->y += obj->vertSpeed;

        for (int i = 0; i < brickCount; i++) {
            if (IsCollision(*obj, brick[i])) {

                if (obj->vertSpeed > 0)
                    obj->IsFly = false;

                if ( (brick[i].cType == '?') &&
                     (obj->vertSpeed < 0) &&
                     (obj == &mario) )
                {
                    brick[i].cType = '-';
                    TObject *coin = NewMoving();
                    coin->Init(brick[i].x, brick[i].y - 2, 1, 1, '$');
                    coin->vertSpeed = -0.5f;
                }

                obj->y -= obj->vertSpeed;
                obj->vertSpeed = 0;

                if (brick[i].cType == '+') {
                    level++;
                    if (level > maxlvl) level = 1;
                    napms(500);
                    if (CreateLevel != NULL) CreateLevel(level);
                }
                break;
            }
        }
    }

    void HorizonMove(TObject *obj) {
        obj->x += obj->horizSpeed;

        for (int i = 0; i < brickCount; i++) {
            if (IsCollision(*obj, brick[i])) {
                obj->x -= obj->horizSpeed;
                obj->horizSpeed = -obj->horizSpeed;
                return;
            }
        }

        if (obj->cType == 'o') {
            TObject tmp = *obj;
            // вызываем VertMove без CreateLevel (NULL) — только чтобы проверить IsFly
            VertMove(&tmp, NULL);

            if (tmp.IsFly == true) {
                obj->x -= obj->horizSpeed;
                obj->horizSpeed = -obj->horizSpeed;
            }
        }
    }

    void HorizonMapShift(float dx) {
        mario.x -= dx;

        for (int i = 0; i < brickCount; i++) {
            if (IsCollision(mario, brick[i])) {
                mario.x += dx;
                return;
            }
        }

        mario.x += dx;

        for (int i = 0; i < brickCount; i++)
            brick[i].x += dx;

        for (int i = 0; i < movingCount; i++)
            moving[i].x += dx;
    }

    void MarioCollision(void (*CreateLevel)(int)) {
        for (int i = 0; i < movingCount; i++) {

            if (IsCollision(mario, moving[i])) {

                if (moving[i].cType == 'o') {

                    if (mario.IsFly == true &&
                        mario.vertSpeed > 0 &&
                        mario.y + mario.height < moving[i].y + moving[i].height * 0.5f)
                    {
                        score += 50;
                        DeleteMoving(i);
                        i--;
                        continue;
                    } else {
                        PlayerDead(CreateLevel);
                        return;
                    }
                }

                if (moving[i].cType == '$') {
                    score += 100;
                    DeleteMoving(i);
                    i--;
                    continue;
                }
            }
        }
    }
};

// =====================================================
//                    LEVEL MANAGER
// =====================================================

class LevelManager {
public:
    ObjectManager *mgr;

    LevelManager(ObjectManager *m) {
        mgr = m;
    }

    void CreateLevel(int lvl) {
        // очистка массивов (как в оригинале)
        mgr->brickCount = 0;
        mgr->brick = (TObject*)realloc(mgr->brick, 0);

        mgr->movingCount = 0;
        mgr->moving = (TObject*)realloc(mgr->moving, 0);

        mgr->mario.Init(5, 5, 2, 2, '@');
        mgr->score = 0;
        mgr->level = lvl;

        if (lvl == 1) {
            mgr->NewBrick()->Init(0, 20, 80, 4, '#');
            mgr->NewBrick()->Init(10, 15, 5, 5, '#');
            mgr->NewBrick()->Init(20, 10, 5, 5, '#');
            mgr->NewBrick()->Init(30, 15, 5, 5, '#');
            mgr->NewBrick()->Init(40, 10, 5, 5, '#');
            mgr->NewBrick()->Init(50, 15, 5, 5, '#');
            mgr->NewBrick()->Init(60, 10, 5, 5, '#');
            mgr->NewBrick()->Init(70, 15, 5, 5, '#');
            mgr->NewBrick()->Init(15, 10, 2, 2, '?');
            mgr->NewBrick()->Init(45, 10, 2, 2, '?');
            mgr->NewBrick()->Init(75, 5, 2, 2, '+');

            mgr->NewMoving()->Init(25, 10, 2, 2, 'o');
            mgr->NewMoving()->Init(55, 10, 2, 2, 'o');
        }

        if (lvl == 2) {
            mgr->NewBrick()->Init(0, 20, 80, 4, '#');
            mgr->NewBrick()->Init(10, 15, 5, 5, '#');
            mgr->NewBrick()->Init(20, 10, 5, 5, '#');
            mgr->NewBrick()->Init(30, 15, 5, 5, '#');
            mgr->NewBrick()->Init(40, 10, 5, 5, '#');
            mgr->NewBrick()->Init(50, 15, 5, 5, '#');
            mgr->NewBrick()->Init(60, 10, 5, 5, '#');
            mgr->NewBrick()->Init(70, 15, 5, 5, '#');
            mgr->NewBrick()->Init(15, 10, 2, 2, '?');
            mgr->NewBrick()->Init(45, 10, 2, 2, '?');
            mgr->NewBrick()->Init(75, 5, 2, 2, '+');

            mgr->NewMoving()->Init(15, 19, 2, 2, 'o');
            mgr->NewMoving()->Init(35, 19, 2, 2, 'o');
            mgr->NewMoving()->Init(55, 19, 2, 2, 'o');
            mgr->NewMoving()->Init(75, 19, 2, 2, 'o');
        }

        if (lvl == 3) {
            mgr->NewBrick()->Init(0, 20, 80, 4, '#');
            mgr->NewBrick()->Init(10, 15, 5, 5, '#');
            mgr->NewBrick()->Init(20, 10, 5, 5, '#');
            mgr->NewBrick()->Init(30, 5, 5, 5, '#');
            mgr->NewBrick()->Init(40, 10, 5, 5, '#');
            mgr->NewBrick()->Init(50, 15, 5, 5, '#');
            mgr->NewBrick()->Init(60, 10, 5, 5, '#');
            mgr->NewBrick()->Init(70, 5, 5, 5, '#');

            mgr->NewBrick()->Init(15, 10, 2, 2, '?');
            mgr->NewBrick()->Init(45, 10, 2, 2, '?');
            mgr->NewBrick()->Init(75, 5, 2, 2, '+');

            mgr->NewMoving()->Init(10, 19, 2, 2, 'o');
            mgr->NewMoving()->Init(30, 14, 2, 2, 'o');
            mgr->NewMoving()->Init(50, 19, 2, 2, 'o');
            mgr->NewMoving()->Init(70, 14, 2, 2, 'o');
        }

        mgr->maxlvl = 3;
    }
};

// =====================================================
//             CreateLevel Bridge (global wrapper)
// =====================================================

void CreateLevelBridge(int lvl) {
    if (gLevelMgr != NULL) {
        gLevelMgr->CreateLevel(lvl);
    }
}

// =====================================================
//                      MAIN LOOP
// =====================================================

int main() {
    initscr();
    curs_set(0);
    noecho();
    nodelay(stdscr, true);
    keypad(stdscr, true);

    int rows = 24, cols = 80;
    getmaxyx(stdscr, rows, cols);

    if (rows < MAP_HEIGHT || cols < MAP_WIDTH) {
        endwin();
        printf("Terminal is too small. Minimum size: %dx%d\n", MAP_WIDTH, MAP_HEIGHT);
        printf("Current size: %dx%d\n", cols, rows);
        return 1;
    }

    GameMap gameMap;
    ObjectManager mgr(&gameMap);
    LevelManager lvlMgr(&mgr);

    // Устанавливаем глобальную ссылку на менеджер уровней для "моста"
    gLevelMgr = &lvlMgr;

    // Первый запуск уровня
    lvlMgr.CreateLevel(mgr.level);

    int key = -1;

    do {
        gameMap.Clear();

        key = getch();

        if (!mgr.mario.IsFly && key == ' ')
            mgr.mario.vertSpeed = -0.8f;

        if (key == 'a')
            mgr.HorizonMapShift(1);

        if (key == 'd')
            mgr.HorizonMapShift(-1);

        if (mgr.mario.y > MAP_HEIGHT)
            mgr.PlayerDead(CreateLevelBridge);

        mgr.VertMove(&mgr.mario, CreateLevelBridge);
        mgr.MarioCollision(CreateLevelBridge);

        for (int i = 0; i < mgr.brickCount; i++)
            gameMap.PutObject(mgr.brick[i]);

        for (int i = 0; i < mgr.movingCount; i++) {
            mgr.VertMove(&mgr.moving[i], CreateLevelBridge);
            mgr.HorizonMove(&mgr.moving[i]);

            if (mgr.moving[i].y > MAP_HEIGHT) {
                mgr.DeleteMoving(i);
                i--;
                continue;
            }

            gameMap.PutObject(mgr.moving[i]);
        }

        gameMap.PutObject(mgr.mario);
        gameMap.PutScore(mgr.score, mgr.level);

        clear();
        gameMap.Show();
        napms(30);

    } while (key != 27);

    endwin();
    return 0;
}
