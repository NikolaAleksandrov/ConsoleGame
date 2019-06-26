#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <direct.h>
#include <time.h>
#include <Tchar.h>


int GetKey();
#pragma region Threads.h

HANDLE gDoneEvent = NULL;
HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;
HANDLE wHnd;    // Handle to write to the console.
HANDLE rHnd;    // Handle to read from the console.

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);

void DeleteTimer();
#pragma endregion

#pragma region Conlsole.h
#define consoleWidth 190
#define consoleHeight 40
CHAR_INFO charInfo[consoleHeight * consoleWidth];
CHAR_INFO* get_screen(COORD* cd);
void set_cursor_visible(int Visible);
void drawScreen();
#pragma endregion

typedef struct sRect
{
	COORD Top;
	COORD Bottom;
} sRect;
typedef struct sRect* pRect;
void setPixel(COORD c, WCHAR symbol, CHAR_INFO* buff) {
	buff[(c.Y) * consoleHeight + c.X].Char.UnicodeChar = symbol;
	buff[(c.Y) * consoleHeight + c.X].Attributes = 0x0F;
}
void drawRec(sRect rec, CHAR_INFO* buff) {

	size_t width = abs((rec.Top.X) - (rec.Bottom.X));
	size_t height= abs((rec.Top.Y) - (rec.Bottom.Y));
	setPixel(rec.Top, 0xC9, buff);
	setPixel((COORD) { rec.Bottom.X, rec.Top.Y }, 0xBB, buff);
	setPixel((COORD) { rec.Top.X, rec.Bottom.Y }, 0xC8, buff);
	setPixel(rec.Bottom, 0xBC, buff);

	for (size_t i = 1; i < width; i++)
	{
		setPixel((COORD) { rec.Top.X+i, rec.Top.Y }, 0xCD, buff);
		setPixel((COORD) { rec.Bottom.X-i, rec.Bottom.Y }, 0xCD, buff);
	}
	for (size_t i = 1; i < height; i++)
	{
		setPixel((COORD) { rec.Top.X, rec.Top.Y + i }, 0xBA, buff);
		setPixel((COORD) { rec.Bottom.X, rec.Bottom.Y - i }, 0xBA, buff);
	}
}


#pragma region Console
//-cnsl---------------------Console.cpp
CHAR_INFO* get_screen(COORD* cd)
{
	HANDLE hCons;
	CONSOLE_SCREEN_BUFFER_INFO ConsInfo;
	CHAR_INFO* buffer;
	SMALL_RECT rect;
	COORD c1;
	hCons = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hCons, &ConsInfo);
	c1.X = 0;
	c1.Y = 0;
	rect = ConsInfo.srWindow;
	buffer = (CHAR_INFO*)calloc(ConsInfo.dwSize.X * ConsInfo.dwSize.Y, sizeof(CHAR_INFO));
	if (ReadConsoleOutput(hCons, buffer, ConsInfo.dwSize, c1, &ConsInfo.srWindow) == 0) {
		printf("Failed reading %d\n", GetLastError());
		GetKey();
		return NULL;
	}
	if (cd != NULL)
		* cd = ConsInfo.dwSize;
	return buffer;
}

void set_cursor_visible(int Visible)
{
	HANDLE hCons;
	CONSOLE_CURSOR_INFO c;
	hCons = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleCursorInfo(hCons, &c);
	c.bVisible = Visible;
	SetConsoleCursorInfo(hCons, &c);
	return;
}

void drawScreen()
{
	HANDLE hCons;

	hCons = GetStdHandle(STD_OUTPUT_HANDLE);

	COORD charBufSize = { consoleHeight, consoleWidth };
	COORD characterPos = { 0, 0 };
	SMALL_RECT writeArea = { 0, 0, consoleHeight, consoleWidth };
	if (WriteConsoleOutputA(wHnd, charInfo, charBufSize, characterPos, &writeArea) == 0) {
		printf("Failed writing\n");
		return;
	}
	return;
}

int GetKey()
{
	int ch;
	ch = _getch();
	if (ch == 0 || ch == 224) {
		ch = _getch();
		ch += 256;
	}
	return ch;
}
#pragma endregion

#pragma region Threads

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{

	drawScreen();

	SetEvent(gDoneEvent);
}

int SetTimer1()
{

	int arg = 0;

	// Use an event object to track the TimerRoutine execution
	gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == gDoneEvent)
	{
		printf("CreateEvent failed (%d)\n", GetLastError());
		return 1;
	}

	// Create the timer queue.
	hTimerQueue = CreateTimerQueue();
	if (NULL == hTimerQueue)
	{
		printf("CreateTimerQueue failed (%d)\n", GetLastError());
		return 2;
	}

	// Set a timer to call the timer routine in 10 seconds.
	if (!CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerRoutine, NULL, 10, 10, 0)) // approx. 100 ms
	{
		printf("CreateTimerQueueTimer failed (%d)\n", GetLastError());
		return 3;
	}
	return 0;
}

void DeleteTimer()
{
	DeleteTimerQueueTimer(hTimerQueue, hTimer, NULL);
	CloseHandle(gDoneEvent);

	// Delete all timers in the timer queue.
	if (!DeleteTimerQueue(hTimerQueue))
		printf("DeleteTimerQueue failed (%d)\n", GetLastError());
}
#pragma endregion

int main(int argc, _TCHAR* argv[]) {
	//------input-------
	COORD speedV;
	speedV.X = 1;
	speedV.Y = 0;
	COORD position;
	position.X = 0;
	position.Y = 3;
	//--------next step validation----------
	wHnd = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTitle(TEXT("Win32 Console Control Demo"));

	SMALL_RECT windowSize = { 0, 0, consoleWidth-1, consoleHeight-1 };

	SetConsoleWindowInfo(wHnd, TRUE, &windowSize);

	COORD bufferSize = { consoleWidth, consoleHeight };

	SetConsoleScreenBufferSize(wHnd, bufferSize);

	sRect a;
	a.Top.Y = 4;
	a.Top.X = 2;
	a.Bottom.X = 30;
	a.Bottom.Y = 20;
	int i;
	drawRec(a, charInfo);
	drawScreen();
	set_cursor_visible(0);

	SetTimer1();

	int incrX=1;
	int incrY = 1;
	int width = abs(a.Top.X - a.Bottom.X);
	int col = a.Top.X + abs(a.Top.X - a.Bottom.X) / 2;
	int row = a.Top.Y + abs(a.Top.Y - a.Bottom.Y) / 2;
	
	while (!_kbhit()) {
		if (WaitForSingleObject(gDoneEvent, INFINITE) != WAIT_OBJECT_0) {
			printf("WaitForSingleObject failed (%d)\n", GetLastError());
			break;
		}
		ResetEvent(gDoneEvent);
		setPixel((COORD) { col, row }, ' ', charInfo);
		
		if (col >= a.Bottom.X - 1)
			incrX = -1;
		else if (col <= a.Top.X + 1)
			incrX = 1;

		if (row >= a.Bottom.Y - 1)
			incrY = -1;
		else if (row <= a.Top.Y + 1)
			incrY = 1;

		col += incrX * speedV.X;
		row += incrY * speedV.Y;
		setPixel((COORD) { col, row }, '*', charInfo);
	}

	DeleteTimer();
	return 0;

}