#ifndef INCLUDE_H
#define INCLUDE_H

	#include <stdlib.h>
	#include <signal.h>
	#include <stdio.h>
	#include <stdbool.h>
	#include <string.h>
	#include <gtk/gtk.h>
	#include <gdk/gdkkeysyms.h>
	#include <time.h>
	#include <sys/time.h>

	//#define DEBUG

	#define MAX_STATUS_LENGTH	256

	#define TIME_EASY			60
	#define TIME_NORMAL			300
	#define TIME_HARD			600

	#define SIZE_EASY			4
	#define SIZE_NORMAL			8
	#define SIZE_HARD			12

	#define TIMER_INTERVAL		1000

	typedef enum
	{
		EASY = 0,
		NORMAL,
		HARD
	} gameDifficulty;

	typedef struct
	{
		GtkWidget *button;
		int currentX;
		int currentY;
		int originalX;
		int originalY;
	} puzzleButton;

	typedef struct
	{
		GtkApplication *app;
		GtkWidget *window;
		GtkWidget *winDialog;
		GtkWidget *statusBar;
		guint statusID;
		guint fieldSize;
		gameDifficulty difficulty;
		GtkWidget *box;
		GtkWidget *grid;
		puzzleButton *puzzleButtons[SIZE_HARD][SIZE_HARD];
		puzzleButton *firstButton;
		puzzleButton *secondButton;
		bool firstButtonClicked;
		int clicks;
		int timeNeeded;
		int countDown;
	} widgets;



	extern widgets *w;

#endif
