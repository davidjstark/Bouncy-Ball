#include <cstdio>
#include <cmath>
#include <windows.h>
#include <windef.h>
#include <wingdi.h>
#include <fstream>
#include <iostream>
#include "winmessagemap.h"
#include <typeinfo>
#include <d2d1.h>
#include <d2d1helper.h>

using namespace std;

float circle_radius = 40.f;
struct position{
    float x;
    float y;
};
// declare physical state variables
struct position player_pos;
struct position *p_player_pos = &player_pos;
struct position prev_player_pos;
struct position *p_prev_player_pos = &prev_player_pos;
bool circle_selected=false;
bool gravity_on = false;
float gravity = 0.3;
float collision_elast = 0.95;
struct position circle_click;
struct position *p_circle_click = &circle_click;
struct position circle_speed;
struct position *p_circle_speed = &circle_speed;
float speed_max = 100;
float impart_power = 0.05;
bool sticky = false;
bool collided = false;
bool imparted = false;
int fps = 120;
// timing variables
LARGE_INTEGER ticks;
LARGE_INTEGER frequency;
long long prev_ticks;
long long interval;
bool right_click_down = false;
// declare direct graphics variables
ID2D1Factory *pFactory = NULL;
ID2D1HwndRenderTarget *pRenderTarget;
ID2D1SolidColorBrush *pBrush;
D2D1_COLOR_F color;
// screen variables
RECT screen_rect;
float screen_width;
float screen_height;
// menus and identifiers
HMENU optionsMenubar;
HMENU optionsMenu;
HMENU gravityMenu;
HMENU helpMenu;
#define IDM_BALLOPTIONS_FLAT 1
#define IDM_BALLOPTIONS_BOUNCY 2
#define IDM_BALLOPTIONS_SUPERBOUNCY 3
#define IDM_BALLOPTIONS_EXOTICBOUNCY 4
#define IDM_BALLOPTIONS_STICKY 5
#define IDM_BALLOPTIONS_CUSTOM 6
#define IDM_HELP 7
#define IDM_GRAVITY_NONE 8
#define IDM_GRAVITY_MOON 9
#define IDM_GRAVITY_EARTH 10
#define IDM_GRAVITY_SUN 11
#define IDM_GRAVITY_BLACKHOLE 12
#define IDM_GRAVITY_JUPITER 13
#define IDM_GRAVITY_ASTEROID 14
#define IDM_GRAVITY_CUSTOM 15
#define IDM_BALLOPTIONS_CUSTOM 16
bool shown_blackhole_warning = true;

LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

float vector_distance(float x1,float x2,float y1, float y2)
{
    return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    // Define a window class and register it to be used in calls to CreateWindowEx
    char CLASS_NAME[] = "window";

    WNDCLASS wc = {}; // init all parameters to 0

    wc.lpfnWndProc = WindowProc; // Window procedure
    wc.hInstance = hInstance; //Handle Instance from WinMain
    wc.lpszClassName = CLASS_NAME; // string to associate with class
    wc.hbrBackground = CreateSolidBrush(RGB(0,0,255));
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);

    RegisterClass(&wc);
    // Create Window
    HWND hwnd = CreateWindowEx(
        0, // EXTENDED WINDOW STYLE
        CLASS_NAME, // CLASS NAME STRING IN wc (the WNDCLASS object)
        "Bouncy Ball", // WINDOW TITLE TEXT
        WS_OVERLAPPEDWINDOW, // WINDOW STYLE
        CW_USEDEFAULT, // x position upper left corner
        CW_USEDEFAULT, // y position upper left corner
        1280, // width of window
        720, // height of window
        NULL, // Handle to Parent Window
        NULL, // Handle to a menu for the window
        hInstance, // Handle Instance from WinMain
        NULL // LPVOID Additional application data
        );

    if(hwnd == NULL)
    {
        //printf("Failed to Create Window");
        return 0;
    }


    // initialise physical state parameters
    screen_width = static_cast<float>(screen_rect.right-screen_rect.left);
    screen_height = static_cast<float>(screen_rect.bottom-screen_rect.top);
    p_player_pos->x = screen_width/2;
    p_player_pos->y = screen_height/2;
    p_prev_player_pos->x = p_player_pos->x;
    p_prev_player_pos->y = p_player_pos->x;


    ShowWindow(hwnd,nCmdShow);
    // Run the message loop
    MSG msg = {};
    QueryPerformanceFrequency(&frequency);
    interval = frequency.QuadPart/fps;
    QueryPerformanceCounter(&ticks);
    prev_ticks = ticks.QuadPart;
    while(msg.message!=WM_QUIT)
    {
        if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }


        QueryPerformanceCounter(&ticks);
        if(ticks.QuadPart-prev_ticks>interval)
        {
            p_prev_player_pos->x = p_player_pos->x;
            p_prev_player_pos->y = p_player_pos->x;
            if(gravity_on)
            {
                p_circle_speed->y += gravity;
            }
            if(sticky&&collided&&!imparted)
            {
                p_circle_speed->x=0;
                p_circle_speed->y=0;
            }
            imparted = false;
            p_player_pos->x = p_player_pos->x + p_circle_speed->x;
            p_player_pos->y = p_player_pos->y + p_circle_speed->y;
            float speed = sqrt(pow(p_circle_speed->x,2)+pow(p_circle_speed->y,2));
            float temp_elast = collision_elast;
            //printf("%f\n",speed);
            if (speed > speed_max && collision_elast > 1)
            {
                collision_elast = 1;
            }
            if(p_player_pos->x-circle_radius<0)
            {
                p_circle_speed->x = abs(p_circle_speed->x)*collision_elast;
                p_circle_speed->y = p_circle_speed->y*collision_elast;
                p_player_pos->x=circle_radius;
                collided = true;
            }
            else if(p_player_pos->x+circle_radius>screen_width)
            {
                p_circle_speed->x = -abs(p_circle_speed->x)*collision_elast;
                p_circle_speed->y = p_circle_speed->y*collision_elast;
                p_player_pos->x=screen_width-circle_radius;
                collided = true;
            }
            if(p_player_pos->y-circle_radius<0)
            {
                p_circle_speed->y = abs(p_circle_speed->y)*collision_elast;
                p_circle_speed->x = p_circle_speed->x*collision_elast;
                p_player_pos->y=circle_radius;
                collided = true;
            }
            else if(p_player_pos->y+circle_radius>screen_height)
            {
                p_circle_speed->y = -abs(p_circle_speed->y)*collision_elast;
                p_circle_speed->x = p_circle_speed->x*collision_elast;
                collided = true;
                p_player_pos->y=screen_height-circle_radius;
            }
            if(p_player_pos->x-circle_radius>0 && p_player_pos->x+circle_radius<screen_width && p_player_pos->y-circle_radius>0 && p_player_pos->y+circle_radius<screen_height)
            {
                collided = false;
            }
            collision_elast = temp_elast;
            if(right_click_down)
            {
                POINT mouse_pos;
                GetCursorPos(&mouse_pos);
                ScreenToClient(hwnd,&mouse_pos);
                gravity_on = false;
                p_circle_speed->x = 0;
                p_circle_speed->y = 0;
                p_player_pos->x = (float)mouse_pos.x;
                p_player_pos->y = (float)mouse_pos.y;
            }
            InvalidateRect(hwnd,NULL,false);
            UpdateWindow(hwnd);
            prev_ticks = ticks.QuadPart;
        }

    }

    return 0;

}





LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{

//    const char* message = wmTranslation[uMsg];
//    if (message!=NULL)
//    {
//        printf("%s\n",message);
//    }
    switch(uMsg)
    {
        case WM_CREATE:
        {
            // initialise direct graphics objects
            GetClientRect(hwnd,&screen_rect);
            D2D1_SIZE_U size = D2D1::SizeU(screen_rect.right,screen_rect.bottom);
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,&pFactory);//400 ticks
            pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),D2D1::HwndRenderTargetProperties(hwnd,size),&pRenderTarget);//300000 ticks
            color = D2D1::ColorF(1.0f,0,0);
            pRenderTarget->CreateSolidColorBrush(color,&pBrush);
            // Menu Stuff
            optionsMenubar = CreateMenu();
            optionsMenu = CreateMenu();
            helpMenu = CreateMenu();
            gravityMenu = CreateMenu();

            AppendMenu(optionsMenu,MF_STRING,IDM_BALLOPTIONS_STICKY,"Sticky");
            AppendMenu(optionsMenu,MF_STRING,IDM_BALLOPTIONS_FLAT,"Flat");
            AppendMenu(optionsMenu,MF_STRING,IDM_BALLOPTIONS_BOUNCY,"Bouncy");
            AppendMenu(optionsMenu,MF_STRING,IDM_BALLOPTIONS_SUPERBOUNCY,"Super-Bouncy");
            AppendMenu(optionsMenu,MF_STRING,IDM_BALLOPTIONS_EXOTICBOUNCY,"Exotic-Bouncy");
            AppendMenu(helpMenu,MF_STRING,IDM_HELP,"How to Play");
            AppendMenu(gravityMenu,MF_STRING,IDM_GRAVITY_NONE,"None");
            AppendMenu(gravityMenu,MF_STRING,IDM_GRAVITY_ASTEROID,"Asteroid");
            AppendMenu(gravityMenu,MF_STRING,IDM_GRAVITY_MOON,"Moon");
            AppendMenu(gravityMenu,MF_STRING,IDM_GRAVITY_EARTH,"Earth");
            AppendMenu(gravityMenu,MF_STRING,IDM_GRAVITY_JUPITER,"Jupiter");
            AppendMenu(gravityMenu,MF_STRING,IDM_GRAVITY_SUN,"Sun");
            AppendMenu(gravityMenu,MF_STRING,IDM_GRAVITY_BLACKHOLE,"Black Hole");
            AppendMenu(optionsMenubar,MF_POPUP, (UINT_PTR) optionsMenu,"Ball Options");
            AppendMenu(optionsMenubar,MF_POPUP,(UINT_PTR) gravityMenu,"Gravity Options");
            AppendMenu(optionsMenubar,MF_POPUP,(UINT_PTR) helpMenu,"Help");
            CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_BOUNCY,MF_CHECKED);
            CheckMenuItem(gravityMenu,IDM_GRAVITY_EARTH,MF_CHECKED);
            SetMenu(hwnd,optionsMenubar);
            return 0;
        }
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDM_BALLOPTIONS_FLAT:
                {
                    collision_elast = 0.5;
                    sticky = false;
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_FLAT,MF_CHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_BOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_SUPERBOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_EXOTICBOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_STICKY,MF_UNCHECKED);
                    break;
                }
                case IDM_BALLOPTIONS_BOUNCY:
                {
                    collision_elast = 0.95;
                    sticky = false;
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_FLAT,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_BOUNCY,MF_CHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_SUPERBOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_EXOTICBOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_STICKY,MF_UNCHECKED);
                    break;
                }
                case IDM_BALLOPTIONS_SUPERBOUNCY:
                {
                    collision_elast = 1;
                    sticky = false;
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_FLAT,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_BOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_SUPERBOUNCY,MF_CHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_EXOTICBOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_STICKY,MF_UNCHECKED);
                    break;
                }
                case IDM_BALLOPTIONS_EXOTICBOUNCY:
                {
                    collision_elast = 1.04;
                    sticky = false;
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_FLAT,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_BOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_SUPERBOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_EXOTICBOUNCY,MF_CHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_STICKY,MF_UNCHECKED);
                    break;
                }
                case IDM_BALLOPTIONS_STICKY:
                {
                    collision_elast = 0;
                    sticky = true;
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_FLAT,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_BOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_SUPERBOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_EXOTICBOUNCY,MF_UNCHECKED);
                    CheckMenuItem(optionsMenu,IDM_BALLOPTIONS_STICKY,MF_CHECKED);
                    break;
                }
                case IDM_HELP:
                {
                    MessageBox(hwnd,"Left Click on the ball, drag, then release to fling the ball in the opposite direction\n"
                               "Right Click to reset the ball at the location of the cursor with no gravity","How to Play",MB_ICONQUESTION);
                    break;
                }
                case IDM_GRAVITY_NONE:
                {
                    if(p_player_pos->y == screen_height-circle_radius)
                    {
                        p_circle_speed->y = 0;
                    }
                    gravity = 0;
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_NONE,MF_CHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_MOON,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_EARTH,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_SUN,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_BLACKHOLE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_JUPITER,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_ASTEROID,MF_UNCHECKED);
                    break;
                }
                case IDM_GRAVITY_MOON:
                {
                   if(p_player_pos->y == screen_height-circle_radius)
                    {
                        p_circle_speed->y = 0;
                    }
                    gravity = 0.1;
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_NONE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_MOON,MF_CHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_EARTH,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_SUN,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_BLACKHOLE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_JUPITER,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_ASTEROID,MF_UNCHECKED);
                    break;
                }
                case IDM_GRAVITY_EARTH:
                {
                    if(p_player_pos->y == screen_height-circle_radius)
                    {
                        p_circle_speed->y = 0;
                    }
                    gravity = 0.3;
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_NONE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_MOON,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_EARTH,MF_CHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_SUN,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_BLACKHOLE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_JUPITER,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_ASTEROID,MF_UNCHECKED);
                    break;
                }
                case IDM_GRAVITY_JUPITER:
                {
                    if(p_player_pos->y == screen_height-circle_radius)
                    {
                        p_circle_speed->y = 0;
                    }
                    gravity = 1;
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_NONE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_MOON,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_EARTH,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_SUN,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_BLACKHOLE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_JUPITER,MF_CHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_ASTEROID,MF_UNCHECKED);
                    break;
                }
                case IDM_GRAVITY_SUN:
                {
                    if(p_player_pos->y == screen_height-circle_radius)
                    {
                        p_circle_speed->y = 0;
                    }
                    gravity = 10;
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_NONE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_MOON,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_EARTH,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_SUN,MF_CHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_BLACKHOLE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_JUPITER,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_ASTEROID,MF_UNCHECKED);
                    break;
                }
                case IDM_GRAVITY_BLACKHOLE:
                {
                    if(p_player_pos->y == screen_height-circle_radius)
                    {
                        p_circle_speed->y = 0;
                    }
                    gravity = 1000;
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_NONE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_MOON,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_EARTH,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_SUN,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_BLACKHOLE,MF_CHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_JUPITER,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_ASTEROID,MF_UNCHECKED);
                    if(!shown_blackhole_warning)
                    {
                        MessageBox(hwnd,"Warning! You may break the ball.","Black Hole",MB_ICONWARNING);
                        shown_blackhole_warning = true;
                    }
                    break;
                }
                case IDM_GRAVITY_ASTEROID:
                {
                    if(p_player_pos->y == screen_height-circle_radius)
                    {
                        p_circle_speed->y = 0;
                    }
                    gravity = 0.01;
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_NONE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_MOON,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_EARTH,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_SUN,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_BLACKHOLE,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_JUPITER,MF_UNCHECKED);
                    CheckMenuItem(gravityMenu,IDM_GRAVITY_ASTEROID,MF_CHECKED);
                    break;
                }
            }
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hc = BeginPaint(hwnd, &ps);
            pRenderTarget->BeginDraw();
            pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Blue));
            D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                D2D1::Point2F(p_player_pos->x,p_player_pos->y),
                circle_radius,
                circle_radius
            );
            pRenderTarget->FillEllipse(ellipse,pBrush);
            pRenderTarget->EndDraw();
            EndPaint(hwnd,&ps);
            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            POINT mouse_pos;
            GetCursorPos(&mouse_pos);
            ScreenToClient(hwnd,&mouse_pos);
            if(vector_distance((float)mouse_pos.x,p_player_pos->x,(float)mouse_pos.y,p_player_pos->y)<circle_radius && circle_selected == false)
            {
                gravity_on = false;
                circle_selected = true;
                p_circle_speed->x = 0;
                p_circle_speed->y = 0;
                p_circle_click->x = (float)mouse_pos.x;
                p_circle_click->y = (float)mouse_pos.y;
                SetCapture(hwnd);
            }
            return 0;
        }
        case WM_LBUTTONUP:
        {
            POINT mouse_pos;
            GetCursorPos(&mouse_pos);
            ScreenToClient(hwnd,&mouse_pos);
            gravity_on = true;
            if(circle_selected == true)
            {
                //printf("Speed imparted");
                circle_selected = false;
                p_circle_speed->x = impart_power*(p_circle_click->x-(float)mouse_pos.x);
                p_circle_speed->y = impart_power*(p_circle_click->y-(float)mouse_pos.y);
                imparted = true;
                ReleaseCapture();
            }
            return 0;
        }
        case WM_RBUTTONDOWN:
        {
            right_click_down = true;
            SetCapture(hwnd);
            return 0;
        }
        case WM_RBUTTONUP:
        {
            right_click_down = false;
            ReleaseCapture();
            return 0;
        }
        case WM_SIZE:
        {
            GetClientRect(hwnd,&screen_rect);
            D2D1_SIZE_U size = D2D1::SizeU(screen_rect.right,screen_rect.bottom);
            pRenderTarget->Resize(size);
            if(wParam != SIZE_MINIMIZED)
            {
                screen_width = static_cast<float>(screen_rect.right-screen_rect.left);
                screen_height = static_cast<float>(screen_rect.bottom-screen_rect.top);
            }
            InvalidateRect(hwnd,NULL,false);
            return 0;
        }

    }
    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}


