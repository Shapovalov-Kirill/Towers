//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include <math.h>
// секция данных игры  
typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;//хэндл к спрайту шарика 
    bool active;
    int fall_steps;
} sprite;
const int tower_size_x = 3, tower_size_y = 5;
sprite tower0[tower_size_x][tower_size_y];
sprite tower1[tower_size_x][tower_size_y];
sprite racket;//ракетка игрока
sprite enemy;//ракетка противника
sprite ball;//шарик
sprite ball_enemy; //шарик противника
POINT mouse_cords;
struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения
HBITMAP hBrick;

//cекция кода
void InitTower1() {
    for (int i = 0; i < tower_size_x; i++) 
    {
        for (int j = 0; j < tower_size_y; j++)
        {
            tower1[i][j].hBitmap = hBrick;
            tower1[i][j].width = window.width / 50;
            tower1[i][j].height = window.width / 20;
            tower1[i][j].x = tower1[i][j].width  * i + window.width / 5 * 4;
            tower1[i][j].y = tower1[i][j].height * j + window.height - tower1[i][j].height * tower_size_y;
            tower1[i][j].active = true;
            tower1[i][j].fall_steps = 0;

        }
    }

}
void InitTower0() {
    for (int i = 0; i < tower_size_x; i++)
    {
        for (int j = 0; j < tower_size_y; j++)
        {
            tower0[i][j].hBitmap = hBrick;
            tower0[i][j].width = window.width / 50;
            tower0[i][j].height = window.width / 20;
            tower0[i][j].x = tower0[i][j].width * i + window.width / 5;
            tower0[i][j].y = tower0[i][j].height * j + window.height - tower0[i][j].height * tower_size_y;
            tower0[i][j].active = true;
            tower1[i][j].fall_steps = 0;
        }
    }

}

void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    ball_enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBrick = racket.hBitmap;


    //------------------------------------------------------
    InitTower1();
    InitTower0();
    racket.width = 105;
    racket.height = 300;
    racket.speed = 30;//скорость перемещения ракетки
    racket.x = window.width / 5 + racket.width;//ракетка посередине окна
    racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки

    enemy.x = window.width / 5 * 4;//х координату оппонета ставим в ту же точку что и игрока
    enemy.y = window.height - racket.height;
    enemy.width = 105;
    enemy.height = 300;
    ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
    ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
    ball.speed = 38;
    ball.rad = 20;
    ball.x = racket.x;//x координата шарика - на середие ракетки
    ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки
    ball_enemy.x = enemy.x;
    ball_enemy.speed = 38;
    ball_enemy.y = enemy.y;
    ball_enemy.dx = -0.5;
    ball_enemy.dy = -0.74;
    game.score = 0;
    game.balls = 9;
}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
   // PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содержит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
}

void ShowScore()
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//буфер для текста
    _itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));
    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
    bool a = true;
    for (int i = 0; i < tower_size_x; i++)
    {
        for (int j = 0; j < tower_size_y; j++)
        {
            if (tower0[i][j].active)
                a = false;
            else
                continue;
        }
    }
    if (a)
    {
        TextOutA(window.context, window.width / 2 - 100, window.height / 2 - 25, "You Lose", 8);
        game.action = false;
    }
}

int enemyAttackTime = 0;

void ProcessInput()
{

    if (GetCursorPos(&mouse_cords))
    {
        //cursor position now in p.x and p.y
    }
    if (ScreenToClient(window.hWnd, &mouse_cords))
    {
        //p.x and p.y are now relative to hwnd's client area
    }

    /*
    if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;
    */
    if (!game.action && GetAsyncKeyState(VK_LBUTTON))
    {
        game.action = true;
        ProcessSound("bounce.wav");
        ball.x = racket.x;//x координата шарика - на середие ракетки
        ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки
        ball.dx = mouse_cords.x - ball.x;
        ball.dy = mouse_cords.y - ball.y;
        float len = sqrt(ball.dx * ball.dx + ball.dy * ball.dy);
        ball.dx = ball.dx / len;
        ball.dy = ball.dy / len;
    }
    int t = 0;
    if (ball_enemy.active == true)
    {
        enemyAttackTime = timeGetTime();
    }
    if (ball_enemy.active == false)
    {
        t = timeGetTime();
    }
    if (t - enemyAttackTime > 800)
        ball_enemy.active = true;
}
void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон


    for (int i = 0; i < tower_size_x; i++)
    {
        for (int j = 0; j < tower_size_y; j++)
        {
            if (tower0[i][j].active)
            {
                ShowBitmap(window.context, tower0[i][j].x, tower0[i][j].y, tower0[i][j].width, tower0[i][j].height, tower0[i][j].hBitmap);
            }

            if (tower1[i][j].active)
            {
                float stp = tower1[i][j].fall_steps;
                float a = tower1[i][j].height * stp/10.;
                ShowBitmap(window.context, tower1[i][j].x, tower1[i][j].y+a, tower1[i][j].width, tower1[i][j].height, tower1[i][j].hBitmap);
            }
        }
    }

    ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик
    ShowBitmap(window.context, ball_enemy.x - ball.rad, ball_enemy.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);
}

//void CheckWalls()
//{
//    if (ball.x < ball.rad || ball.x > window.width - ball.rad)
//    {
//        ball.dx *= -1;
//        ProcessSound("bounce.wav");
//    }
//}

void CheckRoof()
{
    if (ball.y < ball.rad + racket.height)
    {
        ball.dy *= -1;
        ProcessSound("bounce.wav");
    }
}

bool tail = false;

void CheckFloor()
{
    if (ball.y > window.height - ball.rad - racket.height)//шарик пересек линию отскока - горизонталь ракетки
    {
        if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//шарик отбит, и мы не в режиме обработки хвоста
        {
            game.score++;//за каждое отбитие даем одно очко
             ball.speed += 5. / game.score;//но увеличиваем сложность - прибавляем скорости шарику
            ball.dy *= -1;//отскок
            racket.width -= 10. / game.score;//дополнительно уменьшаем ширину ракетки - для сложности
            ProcessSound("bounce.wav");//играем звук отскока
        }
        else
        {//шарик не отбит

            tail = true;//дадим шарику упасть ниже ракетки

            if (ball.y - ball.rad > window.height)//если шарик ушел за пределы окна
            {
                game.balls--;//уменьшаем количество "жизней"

                ProcessSound("fail.wav");//играем звук

                if (game.balls < 0) { //проверка условия окончания "жизней"

                    MessageBoxA(window.hWnd, "game over", "", MB_OK);//выводим сообщение о проигрыше
                    InitGame();//переинициализируем игру
                }

                ball.dy = (rand() % 65 + 35) / 100.;//задаем новый случайный вектор для шарика
                ball.dx = -(1 - ball.dy);
                ball.x = racket.x;//инициализируем координаты шарика - ставим его на ракетку
                ball.y = racket.y - ball.rad;
                game.action = false;//приостанавливаем игру, пока игрок не нажмет пробел
                tail = false;
            }
        }
    }
}


void CheckWalls()
{
    if (ball.x < 0 || ball.x > window.width || ball.y > window.height)
    {
        game.balls--;//уменьшаем количество "жизней"
        ProcessSound("fail.wav");//играем звук
        if (game.balls < 0) { //проверка условия окончания "жизней"

            MessageBoxA(window.hWnd, "game over", "", MB_OK);//выводим сообщение о проигрыше
            InitGame();//переинициализируем игру
        }
        game.action = false;//приостанавливаем игру, пока игрок не нажмет пробел
        ball.x = racket.x;//инициализируем координаты шарика - ставим его на ракетку
        ball.y = racket.y - ball.rad;
    }
    if (ball_enemy.x < 0 || ball_enemy.x > window.width || ball_enemy.y > window.height)
        ball_enemy.active = false;
    if (ball_enemy.active == false)
    {
        ball_enemy.x = enemy.x;
        ball_enemy.y = enemy.y;
        double min = 0.0;
        double max = 1.0;
        ball_enemy.dx = -(rand() % 10 / 100. + 0.41);
        ball_enemy.dy = -(rand() % 15 / 100. + 0.62);
        
        
    }
}
void ProcessRoom()
{
    CheckWalls();



    for (int i = 0; i < tower_size_x; i++)
    {
        for (int j = 0; j < tower_size_y; j++)
        {
            if (ball.x > tower1[i][j].x and ball.x < tower1[i][j].x + tower1[i][j].width 
                and ball.y > tower1[i][j].y and ball.y < tower1[i][j].y + tower1[i][j].height)
            {
                if (tower1[i][j].active)
                {

                    ball.dx *= -1;
                    ball.dy *= 1;
                    ball.dx *= 0.4;
                    tower1[i][j].active = false;
                }
            }
        }
    }

    for (int i = 0; i < tower_size_x; i++)
    {
        for (int j = 0; j < tower_size_y; j++)
        {
            if (ball_enemy.x > tower0[i][j].x and ball_enemy.x < tower0[i][j].x + tower0[i][j].width
                and ball_enemy.y > tower0[i][j].y and ball_enemy.y < tower0[i][j].y + tower0[i][j].height)
            {
                if (tower0[i][j].active)
                {

                    ball_enemy.dx *= -1;
                    ball_enemy.dy *= 1;
                    ball_enemy.dx *= 0.4;
                    tower0[i][j].active = false;
                }
            }
        }
    }

    for (int i = 0; i < tower_size_x; i++)
    {
        for (int j = tower_size_y-2; j >=0 ; j--)
        {
                if (tower1[i][j].active == true && tower1[i][j+1].active == false)
                {    
                    if (tower1[i][j].fall_steps == 10)
                    {
                        tower1[i][j].fall_steps = 0;
                        tower1[i][j].active = false;

                        tower1[i][j+1].fall_steps = 0;
                        tower1[i][j+1].active = true;                    
                    }
                    else
                    {
                        tower1[i][j].fall_steps++;
                        
                    }

                    return;
                }
        }
    }

    for (int i = 0; i < tower_size_x; i++)
    {
        for (int j = tower_size_y - 2; j >= 0; j--)
        {
            if (tower0[i][j].active == true && tower0[i][j + 1].active == false)
            {
                if (tower0[i][j].fall_steps == 10)
                {
                    tower0[i][j].fall_steps = 0;
                    tower0[i][j].active = false;

                    tower0[i][j + 1].fall_steps = 0;
                    tower0[i][j + 1].active = true;
                }
                else
                {
                    tower0[i][j].fall_steps++;

                }

                return;
            }
        }
    }
} 


void ProcessBall()
{
    if (game.action)
    {
        //если игра в активном режиме - перемещаем шарик
        ball.x += ball.dx * ball.speed;
        ball.y += ball.dy * ball.speed;
        ball.dy += 0.025;
        ball.dx *= 0.999;
        ball.dy *= 0.999;
    }
    else
    {
        //иначе - шарик "приклеен" к ракетке
        ball.x = racket.x;
    }
    ball_enemy.x += ball_enemy.dx * ball_enemy.speed;
    ball_enemy.y += ball_enemy.dy * ball_enemy.speed;
    ball_enemy.dy += 0.025;
    ball_enemy.dx *= 0.999;
    ball_enemy.dy *= 0.999;
}

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    //mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    //ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowScore();//рисуем очик и жизни
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)


        ProcessInput();//опрос клавиатуры
        ProcessBall();//перемещаем шарик
        ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
        
    }

}
