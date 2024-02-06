#include <cstdio>
#include <windows.h>
#include <windef.h>
#include <wingdi.h>
#include <fstream>
#include <iostream>
#include "winmessagemap.h"
#include <typeinfo>
using namespace std;



LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
COLORREF bgColours[] = {RGB(255,255,255),RGB(255,0,0),RGB(0,255,0),RGB(0,0,255),RGB(0,0,0)};
int colourIndex = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{


    // Define a window class and register it to be used in calls to CreateWindowEx
    char CLASS_NAME[] = "window";

    WNDCLASS wc = {}; // init all parameters to 0

    wc.lpfnWndProc = WindowProc; // Window procedure
    wc.hInstance = hInstance; //Handle Instance from WinMain
    wc.lpszClassName = CLASS_NAME; // string to associate with class
    wc.hbrBackground = CreateSolidBrush(bgColours[colourIndex]);

    RegisterClass(&wc);
    // Create Window
    HWND hwnd = CreateWindowEx(
        0, // EXTENDED WINDOW STYLE
        CLASS_NAME, // CLASS NAME STRING IN wc (the WNDCLASS object)
        "My First Window", // WINDOW TITLE TEXT
        WS_OVERLAPPEDWINDOW, // WINDOW STYLE
        CW_USEDEFAULT, // x position upper left corner
        CW_USEDEFAULT, // y position upper left corner
        900, // width of window
        600, // height of window
        NULL, // Handle to Parent Window
        NULL, // Handle to a menu for the window
        hInstance, // Handle Instance from WinMain
        NULL // LPVOID Additional application data
        );

    if(hwnd == NULL)
    {
        printf("Failed to Create Window");
        return 0;
    }



    ShowWindow(hwnd,nCmdShow);
    // Run the message loop
    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;

}



LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    const char* message = wmTranslation[uMsg];
    if (message!=NULL)
    {
        printf("%s\n",message);
    }
    switch(uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH bgBrush = CreateSolidBrush(bgColours[colourIndex]);
            FillRect(hdc,&ps.rcPaint,bgBrush);
            EndPaint(hwnd,&ps);
            return 0;
        }
        case WM_DESTROY:
        {
            DestroyWindow(hwnd);
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            colourIndex = (colourIndex+1)%5;
            RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE);
            return 0;
        }
        case WM_MOUSEFIRST:
        {
            HCURSOR newCursor = LoadCursorA(NULL,IDC_ARROW);
            SetCursor(newCursor);
            return 0;
        }
    }
    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}


