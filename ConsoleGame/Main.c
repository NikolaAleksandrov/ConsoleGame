#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <direct.h>
#include <time.h>

#define MAX 255
COORD coord;
CHAR_INFO* mainchInfo;
int GetKey();
void InitProgram();
#pragma region Threads.h

HANDLE gDoneEvent = NULL;
HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);

void DeleteTimer();
#pragma endregion

#pragma region Conlsole.h

CHAR_INFO* get_screen(COORD* cd);
void set_cursor_visible(int Visible);
void put_screen(COORD* cd, CHAR_INFO* buffer);
#pragma endregion
char Caption[MAX];
typedef struct sRect
{
	int Left;
	int Top;
	int Bottom;
	int Right;
} *pRect;

struct sRect MainR;



int main() {

	int row = 5, col = 10;
	int incr = 1;

	char string[] = "Demo stirng";
	char* temp;
	unsigned char colour;

	InitProgram();
	set_cursor_visible(0);
	mainchInfo[row * coord.X + col].Char.AsciiChar = '*';
	mainchInfo[row * coord.X + col].Attributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	put_screen(&coord, mainchInfo);

	// string on row 6, start col 5

	for (row = 6, col = 5, temp = string; *temp != 0; col++, temp++) {
		mainchInfo[row * coord.X + col].Char.AsciiChar = *temp;
		mainchInfo[row * coord.X + col].Attributes = FOREGROUND_RED | FOREGROUND_INTENSITY;

	}
	put_screen(&coord, mainchInfo);

	// string on row 6, start col 5, colours changed

	colour = 1;
	for (row = 7, col = 5, temp = string; *temp != 0; col++, temp++) {
		mainchInfo[row * coord.X + col].Char.AsciiChar = *temp;
		mainchInfo[row * coord.X + col].Attributes = colour++;

	}

	row = 10;
	col = 0;

	mainchInfo[row * coord.X + col].Char.AsciiChar = '*';
	mainchInfo[row * coord.X + col].Attributes = 0x0F;
	put_screen(&coord, mainchInfo);
	SetTimer1();

	do {
		if (WaitForSingleObject(gDoneEvent, INFINITE) != WAIT_OBJECT_0) {
			printf("WaitForSingleObject failed (%d)\n", GetLastError());
			break;
		}
		ResetEvent(gDoneEvent);
		mainchInfo[row * coord.X + col].Char.AsciiChar = 0x20;
		mainchInfo[row * coord.X + col].Attributes = 0x00;
		if (col == coord.X - 1)
			incr = -1;
		else if (col == 0)
			incr = 1;
		col += incr;
		mainchInfo[row * coord.X + col].Char.AsciiChar = '*';
		mainchInfo[row * coord.X + col].Attributes = 0x0F;

	} while (!_kbhit());

	DeleteTimer();
	return 0;
}



void InitProgram()
{
	sprintf_s(Caption,100,"Project PIK II, DEMO");
	printf("%s", Caption);
	mainchInfo = get_screen(&coord);
	mainchInfo->Char.AsciiChar = 'K';
	mainchInfo->Attributes = 12;
	MainR.Left = 0;
	MainR.Top = 0;
	MainR.Right = coord.X;
	MainR.Bottom = coord.Y;
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	//init_console(KBValue);

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

void put_screen(COORD* cd, CHAR_INFO* buffer)
{
	HANDLE hCons;
	COORD c1;
	//	SMALL_RECT rect;
	CONSOLE_SCREEN_BUFFER_INFO ConsInfo;

	hCons = GetStdHandle(STD_OUTPUT_HANDLE);
	c1.X = 0;
	c1.Y = 0;
	GetConsoleScreenBufferInfo(hCons, &ConsInfo);

	if (WriteConsoleOutput(hCons, buffer, *cd, c1, &ConsInfo.srWindow) == 0) {
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

	put_screen(&coord, mainchInfo);

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
	if (!CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerRoutine, NULL, 100, 100, 0)) // approx. 100 ms
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
