#include <Windows.h>
#include <iostream>
#include <fstream>
#include <ctime>

#include "MatrixOperations.h"
#include "LysaNets.hpp"
#include "NeuralNetwork.hpp"
#include "NNClassifier.hpp"

using namespace std;
using namespace MatOp;
using namespace LysaNets;

int click_num = 0;
int number = 0;
bool verified = false;

// Global variables
BOOL g_bDrawing = FALSE;
POINT g_ptPrev = { 0 };
HPEN g_hPen = NULL;
HPEN hPen = NULL;
HDC hdc;
BITMAPINFO bmpInfo = { 0 };
HDC hdcMem;
HBITMAP hbmMem;
HBITMAP hBitmap;
HBITMAP hbmOld;

int *lpBits;
double *image;

const int N_classes = 10;
const int N_Hidden_units = 55;
const int N_features = 784;

int propagate(string param_file, string input_file)
{
    ifstream fnum(input_file);
    double **input_number = new double*[1];
    input_number[0] = new double[N_features];
    for(int i = 0; i < N_features; i++)
    {
        fnum >> input_number[0][i];
    }

    ifstream fin(param_file);
    Matrix W1(N_Hidden_units, N_features);
    double **X1 = new double*[N_Hidden_units];
    for(int i = 0; i < N_Hidden_units; i++)
    {
        X1[i] = new double[N_features];
        for(int j = 0 ; j < N_features; j++)
        {
            fin >> X1[i][j];
        }
    }
    W1.setM(X1);

    Matrix b1(N_Hidden_units, 1);
    double **y1 = new double*[N_Hidden_units];
    for(int i = 0; i < N_Hidden_units; i++)
    {
        y1[i] = new double[1];
        fin >> y1[i][0];
    }
    b1.setM(y1);

    Matrix W2(N_classes, N_Hidden_units);
    double **X2 = new double*[N_classes];
    for(int i = 0; i < N_classes; i++)
    {
        X2[i] = new double[N_Hidden_units];
        for(int j = 0 ; j < N_Hidden_units; j++)
        {
            fin >> X2[i][j];
        }
    }
    W2.setM(X2);

    Matrix b2(N_classes, 1);
    double **y2 = new double*[N_classes];
    for(int i = 0; i < N_classes; i++)
    {
        y2[i] = new double[1];
        fin >> y2[i][0];
    }
    b2.setM(y2);

    Sigmoid sigm;
    Matrix X_tmp(1, N_features);
    X_tmp.setM(input_number);
    Matrix X = X_tmp.T();
    X = X / 255;
    X.T().printM("output.txt");
//    Matrix y_tmp(m, 1);
//    y_tmp.setM(data.get_y());
//    Matrix y = y_tmp.T();
    //y_tmp.printM("output1.txt");


    Matrix * a = new Matrix[4];
    a[0] = (W1^X) + b1;   // Z1
    a[1] = leaky_relu(a[0], 0.01);   // A1
    a[2] = (W2^a[1]) + b2;// Z2
    a[3] = softmax(a[2]);   // A2
    Matrix labels = a[3].argmax(0).T();
    return labels.getElement(0,0);
}

double* resize_image(string filename) {
    int *img_280 = new int[280 * 280];
    double *img_28 = new double[28 * 28];

    // Read image file
    ifstream file(filename);
    if (file.is_open()) {
        for (int i = 0; i < 280 * 280; i++) {
            file >> img_280[i];
        }
        file.close();
    } else {
        cout << "Error: could not open file " << filename << endl;
        return nullptr;
    }

    // Resize image
    for (int i = 0; i < 28; i++) {
        for (int j = 0; j < 28; j++) {
            int x = i * 10;
            int y = j * 10;
            int sum = 0;
            for (int k = 0; k < 10; k++) {
                for (int l = 0; l < 10; l++) {
                    sum += img_280[(x + k) * 280 + (y + l)];
                }
            }
            img_28[i * 28 + j] = (int)(sum / 100);
        }
    }
    ofstream out("number28.txt");
    for (int i = 0; i < 28; i++)
    {
        for (int j = 0; j < 28; j++)
        {
            out << img_28[i * 28 + j] << ' ';
        }
        out << '\n';
    }
    // Return resized image
    return img_28;
}

void PrintBitmap(BITMAPINFOHEADER bmInfoHeader)
{
    ofstream fout("drawn_number.txt");
    int offset;
    for (int i = 0; i < bmInfoHeader.biHeight; i++)
    {
        for (int j = 0; j < bmInfoHeader.biWidth; j++)
        {
            COLORREF pixelColor = GetPixel(hdcMem, j, i);
            int brightness = (GetRValue(pixelColor) + GetGValue(pixelColor) + GetBValue(pixelColor)) / 3;
            fout << brightness << ' ';
//            offset = (j * bmInfoHeader.biWidth + i) * 3;
//            fout << (int)(pData[offset] + pData[offset+1] + pData[offset+2]) / 3 << ' ';
        }
        fout << '\n';
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg)
    {

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case BN_CLICKED:
                click_num ++;
                PrintBitmap(bmpInfo.bmiHeader);
                image = resize_image("drawn_number.txt");
                int res = propagate("final_parameters.txt", "number28.txt");
                string s = "Verified!";

                if (res == number)
                {
                    MessageBox(hwnd, "Verified!", "Message", MB_OK);
                    verified = true;
                }
                else
                {
                    number = rand() % 10;
                }

                hdc = GetDC(hwnd);
                // Clear the drawing field with black brush
                RECT rect;
                HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
                SelectObject(hdc, hBrush);
                GetClientRect(hwnd, &rect);
                FillRect(hdcMem, &rect, hBrush);

                // Invalidate the client area to trigger a WM_PAINT message
                ReleaseDC(hwnd, hdc);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
        }
        break;

    case WM_CREATE:
    {
        hdc = GetDC(hwnd);
        hdcMem = CreateCompatibleDC(NULL);
        hbmMem = CreateCompatibleBitmap(GetDC(hwnd), 280, 280);
        SelectObject(hdcMem, hbmMem);
        number = rand() % 10;
        // Create pen for drawing
        g_hPen = CreatePen(PS_SOLID, 20, RGB(255, 255, 255));
        break;
    }

    case WM_SIZE:
    {
        SelectObject(hdcMem, NULL);
        DeleteObject(hbmMem);
        hdc = GetDC(hwnd);
        hbmMem = CreateCompatibleBitmap(GetDC(hwnd), 280, 280);
        ReleaseDC(hwnd, hdc);
        SelectObject(hdcMem, hbmMem);
        break;
    }

    case WM_LBUTTONDOWN:
        // Start drawing
        g_ptPrev.x = LOWORD(lParam);
        g_ptPrev.y = HIWORD(lParam);
        if (g_ptPrev.x >= 390 && g_ptPrev.x <= 650 && g_ptPrev.y >= 110 && g_ptPrev.y <= 370)
        {

//            MoveToEx(hdcMem, g_ptPrev.x, g_ptPrev.y, NULL);
            SetCapture(hwnd);
            g_bDrawing = TRUE;
        }
        break;

    case WM_MOUSEMOVE:
        if (g_bDrawing)
        {
            // Get current mouse position
            POINT ptCurrent = { LOWORD(lParam), HIWORD(lParam) };

            if (ptCurrent.x >= 390 && ptCurrent.x <= 650 && ptCurrent.y >= 110 && ptCurrent.y <= 370)
            {
                HDC hdc = GetDC(hwnd);
                HPEN hOldPenMem = (HPEN)SelectObject(hdcMem, g_hPen);
                MoveToEx(hdcMem, g_ptPrev.x-380, g_ptPrev.y - 100, NULL);
                LineTo(hdcMem,  ptCurrent.x - 380, ptCurrent.y - 100);

                SelectObject(hdcMem, hOldPenMem);
                ReleaseDC(hwnd, hdcMem);


                // Draw line from previous position to current position

                HPEN hOldPen = (HPEN)SelectObject(hdc, g_hPen);

                MoveToEx(hdc, g_ptPrev.x, g_ptPrev.y, NULL);
                LineTo(hdc, ptCurrent.x, ptCurrent.y);

                SelectObject(hdc, hOldPen);
                ReleaseDC(hwnd, hdc);

                // Update previous position
                g_ptPrev = ptCurrent;
            }
        }
        break;

    case WM_LBUTTONUP:
    {
        // Stop drawing
        g_bDrawing = FALSE;
        ReleaseCapture();

        // Save the bitmap to a file
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth = 280;
        bmpInfo.bmiHeader.biHeight = 280;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 24;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        bmpInfo.bmiHeader.biSizeImage = 0;
        bmpInfo.bmiHeader.biXPelsPerMeter = 0;
        bmpInfo.bmiHeader.biYPelsPerMeter = 0;
        bmpInfo.bmiHeader.biClrUsed = 0;
        bmpInfo.bmiHeader.biClrImportant = 0;

        // Get the bitmap bits and write them to a file
        hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
        GetDIBits(hdcMem, hbmMem, 0, 280, NULL, &bmpInfo, DIB_RGB_COLORS);
        lpBits = new int[bmpInfo.bmiHeader.biSizeImage];
        GetDIBits(hdcMem, hbmMem, 0, 280, lpBits, &bmpInfo, DIB_RGB_COLORS);
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        BitBlt(hdc, 390, 110, 280, 280, hdcMem, 0, 0, NULL);

        LOGFONT lf;
        ZeroMemory(&lf, sizeof(LOGFONT));
        lf.lfHeight = 18; // set font height
        lf.lfWidth = 10; // set font width
        lf.lfWeight = FW_NORMAL;
        lf.lfCharSet = DEFAULT_CHARSET;
        lstrcpy(lf.lfFaceName, TEXT("Times New Roman"));

        HFONT hFont = CreateFontIndirect(&lf);
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        RECT rect;
        rect.left = 270;
        rect.top = 80;
        rect.right = 800;
        rect.bottom = 800;

        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkColor(hdc, RGB(255, 255, 255));
        string s = "To confirm that you are human, draw the number " + to_string(number);
        DrawText(hdc, TEXT(s.c_str()), -1, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
    {
        // Cleanup resources
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    srand(time(0));
    // Register the window class.
    const char CLASS_NAME[] = "Digit Captcha";

    WNDCLASS wc = { 0 };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Digit Captcha",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    // Create a button in the window.
    HWND button = CreateWindow(
        "BUTTON",
        "Verify",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        435, 400, 200, 50,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (button == NULL)
    {
        return 0;
    }

    // Show the window.
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    // Run the message loop.
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (verified)
            break;
    }
    return 0;
}




















