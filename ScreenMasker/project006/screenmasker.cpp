#include "stdafx.h"
#include "gpuProcessor.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
//#include "INIReader.h"
#include "iViewXAPI.h"
#include <windows.h>
#include <iostream>
#include <cmath>
#include <assert.h>


//--SM---
// global variables
UINT8 *bgBmpBytes;
int bgBmpWidth, bgBmpHeight;

UINT8 *stencilBmpBytes;
int stencilBmpWidth, stencilBmpHeight;

UINT32 desiredColor; // desired opaque color (replaces black on background pgm)
int stencilX, stencilY, isCoursor;

int mX, mY, mW, mH; // monitor rect

FILE* hLog;

HDC hdcOutBmp;
UINT32 *outBmpBytes;

HINSTANCE hinst;

const UINT WM_MYCOMMAND = 1001;

float gazeX = 0.0;
float gazeY = 0.0;

//--end--SM---



struct pixel {
	union {
		struct {
			/* 'a' unused, used for 32-bit alignment,
			* could also be used to store pixel alpha
			*/
			unsigned char b, g, r, a;
		};
		int val;
	};
	pixel() {
		val = 0;
	}
};
// Window client size
//const int width = 375;
//const int height = 375;
/* Target fps, though it's hard to achieve this fps
* without extra timer functionality unless you have
* a powerfull processor. Raising this value will
* increase the speed, though it will use up more CPU.
*/
const int fps = 250;
// Global Windows/Drawing variables
HBITMAP hbmp;
HANDLE hTickThread;
HWND hwnd;
HDC hdcMem;
// Pointer to pixels (will automatically have space allocated by CreateDIBSection
pixel *pixels;


UINT8* readPGM(LPWSTR filePath, int *w, int *h)
{
	char* buf = new char[1024]();
	FILE* f;
	char* ret;

	try
	{
		f = _wfopen(filePath, L"rb");
		if (f == NULL) throw L"failed to open file";

		// read: P5
		if (fgets(buf, 1024, f) != buf) throw L"header not read completely";
		if (ferror(f) || feof(f)) throw L"error reading header";
		if (memcmp(buf, "P5", 2) != 0) throw L"invalid header";

		// read: #comment
		if (fgets(buf, 1024, f) != buf) throw L"comment not read completely";
		if (ferror(f) || feof(f))  throw L"error reading comment";

		// read: width height
		if (fscanf(f, "%d %d", w, h) != 2)  throw L"width and height not read completely";
		if (ferror(f) || feof(f))  throw L"error reading width and height";

		// read rest of "width height" line
		if (fgets(buf, 1024, f) != buf)  throw L"end of width and height not read completely";
		if (ferror(f) || feof(f))  throw L"error reading end of width and height";

		// read: 255
		if (fgets(buf, 1024, f) != buf)  throw L"raw type indicator not read completely"; // is it?
		if (memcmp(buf, "255", 3) != 0)  throw L"invalid raw type indicator";
		if (ferror(f) || feof(f))  throw L"error reading raw type indicator";

		// read bytes for image
		delete buf;
		buf = new char[*w**h]();
		if (fread(buf, 1, *w**h, f) != *w**h)  throw L"image bytes not read completely"; // f u C(++)
		if (ferror(f) || feof(f))  throw L"error reading image bytes";

		ret = buf;
	}
	catch (wchar_t *errMsg)
	{
		std::cout << "Reading PGM failed: %ls\nFile: %ls";
		ret = NULL;
	}

	if (ret == NULL) delete buf;
	if (f != NULL) fclose(f);

	return (UINT8*) ret;
}


void onFrame(pixel *pixels) {
	
	/*
	// This is where all the drawing takes place
	pixel *p;
	// +0.005 each frame
	static float frameOffset = 0;
	float px; // % of the way across the bitmap
	float py; // % of the way down the bitmap
	for (int x = 0; x < width; ++x) {
	for (int y = 0; y < height; ++y) {
	p = &pixels[y * width + x];
	px = float(x) / float(width);
	py = float(y) / float(height);
	p->r = unsigned char(((cos(px + frameOffset * 10) / sin(py + frameOffset)) * cos(frameOffset * 3) * 10) * 127 + 127);
	p->g = ~p->r;
	p->b = 255;
	}
	}
	frameOffset += 0.005f;
	*/
	
	drawBackground(bgBmpWidth, bgBmpHeight, stencilBmpWidth, stencilBmpHeight, mW, mH, gazeX, gazeY, desiredColor, outBmpBytes);

	// "flush" what was just created, to screen
	static BLENDFUNCTION blendFunc = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	static POINT ptDst = { mX, mY };
	static POINT ptZero = { 0, 0 };
	static SIZE siz = { mW, mH };

	UpdateLayeredWindow(hwnd, NULL, &ptDst, &siz, hdcOutBmp, &ptZero, 0, &blendFunc, ULW_ALPHA);

}
DWORD WINAPI tickThreadProc(HANDLE handle) {
	// Give plenty of time for main thread to finish setting up
	Sleep( 50 );
	ShowWindow( hwnd, SW_SHOW );
	// Retrieve the window's DC
	HDC hdc = GetDC( hwnd );
	// Create DC with shared pixels to variable 'pixels'
	hdcMem = CreateCompatibleDC( hdc );
	HBITMAP hbmOld = (HBITMAP)SelectObject( hdcMem, hbmp );
	// Milliseconds to wait each frame
	int delay = 1000 / fps;
	for ( ;; ) {
		// Do stuff with pixels
		onFrame( pixels );
		// Draw pixels to window
		BitBlt( hdc, 0, 0, mW, mH, hdcMem, 0, 0, SRCCOPY );
		// Wait
		//Sleep( delay );
	}
	SelectObject( hdcMem, hbmOld );
	DeleteDC( hdc );
}
void MakeSurface(HWND hwnd) {
	/* Use CreateDIBSection to make a HBITMAP which can be quickly
	* blitted to a surface while giving 100% fast access to pixels
	* before blit.
	*/
	// Desired bitmap properties
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biWidth = mW;
	bmi.bmiHeader.biHeight =  -mH; // Order pixels from top to bottom
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32; // last byte not used, 32 bit for alignment
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;
	/*
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	bmi.bmiColors[0].rgbBlue = 0;
	bmi.bmiColors[0].rgbGreen = 0;
	bmi.bmiColors[0].rgbRed = 0;
	bmi.bmiColors[0].rgbReserved = 0;
	*/

	hbmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &outBmpBytes, NULL, 0x0);

	hdcOutBmp = CreateCompatibleDC(NULL); // FIXME: needed if on another monitor (graphic card)?
	SelectObject(hdcOutBmp, hbmp);

	/*
	HDC hdc = GetDC( hwnd );
	// Create DIB section to always give direct access to pixels
	//hbmp = CreateDIBSection( hdc, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0 );
	hbmp = CreateDIBSection( hdc, &bmi, DIB_RGB_COLORS, (void**)&outBmpBytes, NULL, 0 );




	DeleteDC( hdc );
	*/


	// Create a new thread to use as a timer
	hTickThread = CreateThread( NULL, NULL, &tickThreadProc, NULL, NULL, NULL );
}
LRESULT CALLBACK WndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch ( msg ) {
	case WM_CREATE:
		{
			MakeSurface( hwnd );
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( hwnd, &ps );
			// Draw pixels to window when window needs repainting
			BitBlt( hdc, 0, 0, mW, mH, hdcMem, 0, 0, SRCCOPY );
			EndPaint( hwnd, &ps );
		}
		break;
	case WM_CLOSE:
		{
			DestroyWindow( hwnd );
		}
		break;
	case WM_DESTROY:
		{
			TerminateThread( hTickThread, 0 );
			PostQuitMessage( 0 );
		}
		break;
	default:
		return DefWindowProc( hwnd, msg, wParam, lParam );
	}
	return 0;
}

LPWSTR toString(char text[]){
	wchar_t wtext[200];
	mbstowcs(wtext, text, strlen(text)+1);//Plus null
	LPWSTR ptr = wtext;
	return ptr;
}

int __stdcall SampleCallbackFunction(SampleStruct sampleData)
{
	//std::cout << "From Callback X: " << sampleData.leftEye.gazeX << " Y: " << sampleData.leftEye.gazeY << std::endl;
	gazeX = sampleData.leftEye.gazeX;
	gazeY = sampleData.leftEye.gazeY;
	return 1;
}


int __stdcall TrackingMonitorCallbackFunction(ImageStruct imageData)
{
	//std::cout << "From TrackingMonitor Callback ImageSize: " << imageData.imageSize << " Width: " << imageData.imageWidth << " Height: " << imageData.imageHeight << std::endl;
	return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	//----------------
	hLog = fopen("log.txt", "w");
	LPCTSTR path = L".\\options.ini";

	TCHAR stecilPicturePathResult[4096];
	TCHAR maskedPicturePathResult[4096];
	TCHAR serverPortResult[255];
	TCHAR serverIPResult[255];
	TCHAR listenUDPPortResult[255];
	TCHAR listenUDPIPResult[255];
	TCHAR colorResult[255];
	TCHAR shiftXResult[255];
	TCHAR shiftYResult[255];
	TCHAR isEyeWindowShowResult[255];
	TCHAR isEyePositionWindowShowResult[255];
	TCHAR isCalibrateResult[255];

	char buffer[4096];

	GetPrivateProfileString(_T("Base"), _T("stecilPicturePath"), _T(""), stecilPicturePathResult, sizeof(buffer), path);
	GetPrivateProfileString(_T("Base"), _T("maskedPicturePath"), _T(""), maskedPicturePathResult, sizeof(buffer), path);
	GetPrivateProfileString(_T("Base"), _T("serverPort"), _T(""), serverPortResult, 255, path);
	GetPrivateProfileString(_T("Base"), _T("serverIP"), _T(""), serverIPResult, 255, path);
	GetPrivateProfileString(_T("Base"), _T("listenUDPPort"), _T(""), listenUDPPortResult, 255, path);
	GetPrivateProfileString(_T("Base"), _T("listenUDPIP"), _T(""), listenUDPIPResult, 255, path);
	GetPrivateProfileString(_T("Base"), _T("color"), _T(""), colorResult, 255, path);
	GetPrivateProfileString(_T("Base"), _T("shiftX"), _T(""), shiftXResult, 255, path);
	GetPrivateProfileString(_T("Base"), _T("shiftY"), _T(""), shiftYResult, 255, path);
	GetPrivateProfileString(_T("Base"), _T("isEyeWindowShow"), _T(""), isEyeWindowShowResult, 255, path);
	GetPrivateProfileString(_T("Base"), _T("isEyePositionWindowShow"), _T(""), isEyePositionWindowShowResult, 255, path);
	GetPrivateProfileString(_T("Base"), _T("isCalibrate"), _T(""), isCalibrateResult, 255, path);

	int shiftXint = _ttoi(shiftXResult);
	int shiftYint = _ttoi(shiftYResult);
	int serverPortint = _ttoi(serverPortResult);
	int listenUDPPortint = _ttoi(listenUDPPortResult);
	desiredColor = wcstoul(colorResult, NULL, 0x10);

	char serverIPChar[255];
	wcstombs(serverIPChar, serverIPResult, wcslen(serverIPResult) + 1);

	char listenUDPIPChar[255];
	wcstombs(listenUDPIPChar, listenUDPIPResult, wcslen(listenUDPIPResult) + 1);

	bool isEyeWindowShowBool = false;
	bool isEyePositionWindowShow = false;
	bool isCalibrate = false;

	if(_tcscmp(isEyeWindowShowResult, L"true") == 0){
		isEyeWindowShowBool = true;
	} 
	if(_tcscmp(isEyePositionWindowShowResult, L"true") == 0){
		isEyePositionWindowShow = true;
	}
	if(_tcscmp(isCalibrateResult, L"true") == 0){
		isCalibrate = true;
	}
	
	//getchar();
    //return 0;

	// screenMasker.exe 757575 checker4px.pgm stencil.pgm 23 45 Y"
	// parse arguments
	//E:\OrlovPA\screenMasker\screenmasker\screenMasker_bin\screenMasker_bin\screenMasker.exe 757575 bg.pgm stencil.pgm 0
	//LPWSTR *argc;
	//LPWSTR bgPath;
	//wchar_t newBgPath[] = L"E:\\OrlovPA\\screenMasker\\screenmasker\\screenMasker_bin\\screenMasker_bin\\bg.pgm";
	//wchar_t newBgPath[] = maskedPicturePathResult;
	//LPWSTR stencilPath;
	//wchar_t newStencilPath[] = L"E:\\OrlovPA\\screenMasker\\screenmasker\\screenMasker_bin\\screenMasker_bin\\stencil.pgm";

	//int localstencilX, localstencilY, localCoursor;

	//wchar_t myColor = '757575';
	//const wchar_t * myColorAdress = &myColor;
	
	
	//desiredColor = wcstoul(L"1a3399", NULL, 0x10);

	//desiredColor = 757575;

	isCoursor = 1;

	// read background and stencil to buffers
	UINT8 *bytes;
	int width;
	int height;

	//bytes = readPGM(bgPath, &width, &height);
	bytes = readPGM(maskedPicturePathResult, &width, &height);
	//bytes = readPGM(newBgPath, &width, &height);

	if(bytes == NULL)
	{
		return 1;
	}

	bgBmpBytes = bytes;
	bgBmpWidth = width;
	bgBmpHeight = height;

	//bytes = readPGM(newStencilPath, &width, &height);
	bytes = readPGM(stecilPicturePathResult, &width, &height);
	
	if(bytes == NULL){
		return 1;
	}

	stencilBmpBytes = bytes;
	stencilBmpWidth = width;
	stencilBmpHeight = height;
	//----------------

	POINT pt = {};
	HMONITOR hMon;
	MONITORINFO monitorInfo = {};
	GetCursorPos(&pt); 
	hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);

	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hMon, &monitorInfo);

	mX = monitorInfo.rcMonitor.left;
	mY = monitorInfo.rcMonitor.top;
	mW = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	mH = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	WNDCLASSEX wc;
	MSG msg;
	// Init wc
	wc.cbSize = sizeof( WNDCLASSEX );
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = NULL;
	wc.hIconSm = NULL;
	wc.lpszClassName = toString("animation_class");
	wc.lpszMenuName = NULL;

	// Register wc
	if ( !RegisterClassEx(&wc) ) {
		MessageBox( NULL, toString("Failed to register window class."), toString("Error"), MB_OK );
		return 0;
	}
	// Make window
	hwnd = CreateWindowEx(
		//WS_EX_APPWINDOW,
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
		toString("animation_class"),
		toString("Animation"),
		WS_POPUP | WS_SYSMENU,
		mX, mY, mW, mH,
		NULL, NULL, GetModuleHandle(0), NULL );

	assert(hwnd);


	/*
	RECT rcClient, rcWindow;
	POINT ptDiff;
	// Get window and client sizes
	GetClientRect( hwnd, &rcClient );
	GetWindowRect( hwnd, &rcWindow );
	// Find offset between window size and client size
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	// Resize client
	MoveWindow( hwnd, rcWindow.left, rcWindow.top, width + ptDiff.x, height + ptDiff.y, false);
	

	//UpdateLayeredWindow(hwnd, NULL, &ptDst, &siz, hdcOutBmp, &ptZero, 0, &blendFunc, ULW_ALPHA);

	//SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0x0, LWA_COLORKEY);
	*/

	ShowWindow( hwnd, SW_SHOWDEFAULT );
	UpdateWindow( hwnd );

	initGpuResources(	bgBmpBytes, 
		stencilBmpBytes, 
		bgBmpWidth, 
		bgBmpHeight, 
		stencilBmpHeight, 
		stencilBmpWidth, 
		mW, 
		mH	);


	//****************************
	AccuracyStruct accuracyData; 
	SystemInfoStruct systemInfoData; 
	CalibrationStruct calibrationData;
	int ret_calibrate = 0, ret_validate = 0, ret_connect = 0; 
	char c = ' ';
	char repeat = ' ';

	std::cout << "Output from iViewXAPI Demo"<< std::endl;

	// define logger 
	iV_SetLogger(1, "ScreenMasker.txt");
	std::cout << "ScreenMasker on iViewX API" << std::endl;

	// connect to iViewX 
	ret_connect = iV_Connect(serverIPChar, serverPortint, listenUDPIPChar, listenUDPPortint);
	

	switch(ret_connect)
	{
		case RET_SUCCESS:
			std::cout <<  "Connection was established successfully" << std::endl;

			// read out meta data from iViewX 
			std::cout << "GetSystemInfo: " << iV_GetSystemInfo(&systemInfoData) << std::endl;
			std::cout << "SystemInfo ETSystem: " << systemInfoData.iV_ETDevice << std::endl;
			std::cout << "SystemInfo iV_Version: " << systemInfoData.iV_MajorVersion << "." << systemInfoData.iV_MinorVersion << "." << systemInfoData.iV_Buildnumber << std::endl;
			std::cout << "SystemInfo API_Version: " << systemInfoData.API_MajorVersion << "." << systemInfoData.API_MinorVersion << "." << systemInfoData.API_Buildnumber << std::endl;
			std::cout << "SystemInfo samplerate: " << systemInfoData.samplerate << std::endl;

			break;
		case ERR_COULD_NOT_CONNECT:
			std::cout <<  "Connection could not be established" << std::endl;
			break;
		case ERR_WRONG_PARAMETER:
			std::cout <<  "Wrong Parameter used" << std::endl;
			break;
		default:
			std::cout <<  "Any other error appeared" << std::endl;
			return 0;
	}

	if(ret_connect == RET_SUCCESS)
	{
		calibrationData.method = 5;
		calibrationData.speed = 0;
		calibrationData.displayDevice = 0;
		calibrationData.targetShape = 20;
		calibrationData.foregroundBrightness = 250;
		calibrationData.backgroundBrightness = 100;
		calibrationData.autoAccept = 1;
		calibrationData.targetSize = 10;
		calibrationData.visualization = 1;
		strcpy(calibrationData.targetFilename, "");

		iV_SetupCalibration(&calibrationData);

		// start calibration
		//std::cout <<  "Do you want to calibrate? (y)es | (n)o" << std::endl;
		//c = getchar();

		if(isCalibrate){
			ret_calibrate = iV_Calibrate();

			switch(ret_calibrate){
				case RET_SUCCESS:
					std::cout <<  "Calibration done successfully" << std::endl;
					
					// start validation
					ret_validate = iV_Validate();

					break;
				case ERR_NOT_CONNECTED:
					std::cout <<  "iViewX is not reachable" << std::endl;
					break;
				case ERR_WRONG_PARAMETER:
					std::cout <<  "Wrong Parameter used" << std::endl;
					break;
				case ERR_WRONG_DEVICE:
					std::cout <<  "Not possible to calibrate connected Eye Tracking System" << std::endl;
					break;
				default:
					std::cout <<  "An unknown error appeared" << std::endl;
					break;
			}

		}

		// show accuracy only if validation was successful
		if(ret_validate == RET_SUCCESS){
			std::cout << "iV_GetAccuracy: " << iV_GetAccuracy(&accuracyData, 0) << std::endl;
			std::cout << "AccuracyData DevX: " << accuracyData.deviationLX << " DevY: " << accuracyData.deviationLY << std::endl;
			//getchar();

		}

		if(isEyeWindowShowBool){
			iV_ShowEyeImageMonitor();
		}
		
		if(isEyePositionWindowShow){
			iV_ShowTrackingMonitor();
			iV_ShowSceneVideoMonitor();
		}

		// start data output via callback function
		// define a callback function for receiving samples 
		iV_SetSampleCallback(SampleCallbackFunction);
		iV_SetTrackingMonitorCallback(TrackingMonitorCallbackFunction);

		while ( GetMessage(&msg, 0, 0, NULL) > 0 ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		/*
		getchar();

		iV_SetSampleCallback(NULL);
		iV_SetTrackingMonitorCallback(NULL);

		std::cout << "iV_Disconnect: " << iV_Disconnect() << std::endl;

		getchar();
		*/
	}

	freeGpuResources();
	fclose(hLog);

	return 0;
}

