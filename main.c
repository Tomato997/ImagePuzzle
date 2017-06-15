// clang puzzle.c $(pkg-config --cflags --libs gtk+-3.0) -Wall -g -o puzzle

#include "include.h"

widgets *w = NULL;

void menuRestartCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	// Start a new game
	startGame();
}

void menuDifficultyEasyCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	// Change difficulty
	w->difficulty = EASY;

	// Start a new game
	startGame();
}

void menuDifficultyNormalCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	// Change difficulty
	w->difficulty = NORMAL;

	// Start a new game
	startGame();
}

void menuDifficultyHardCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	// Change difficulty
	w->difficulty = HARD;

	// Start a new game
	startGame();
}

void menuHighScoreCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	g_print("menuHighScoreCallback...\n");
}

void menuQuitCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	g_application_quit(G_APPLICATION(w->app));
}

void menuAboutCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(w->window, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Image Puzzle v1.0\n\nCopyright (C) 2017 by Felix Knobl.");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

static void winDialogOkButtonClicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *nameEntry = (GtkWidget *)data;

	gchar *nameBuffer;
	nameBuffer = (gchar*)gtk_entry_get_text(GTK_ENTRY(nameEntry));

	if (strlen(nameBuffer) == 0)
	{
		return;
	}

	g_print("Name: %s\n", nameBuffer);

	gtk_window_close(w->winDialog);

	// Start a new game
	startGame();
}

// Map callbacks
const GActionEntry menuCallbacks[] =
{
	{"restart", menuRestartCallback, NULL, NULL, NULL},
	{"difficultyeasy", menuDifficultyEasyCallback, NULL, NULL, NULL},
	{"difficultynormal", menuDifficultyNormalCallback, NULL, NULL, NULL},
	{"difficultyhard", menuDifficultyHardCallback, NULL, NULL, NULL},
	{"highscore", menuHighScoreCallback, NULL, NULL, NULL},
	{"quit", menuQuitCallback, NULL, NULL, NULL},
	{"about", menuAboutCallback, NULL, NULL, NULL}
};

void createMenu(GtkWidget *box)
{
	// Create the menu
	GtkWidget *menubar;
	GMenu *menu;
	GMenu *gamemenu, *optionsmenu, *difficultymenu, *quitmenu;
	GMenu *helpmenu;

	menu = g_menu_new();

	// HELP MENU
	// Create the help menu
	helpmenu = g_menu_new();
	g_menu_append(helpmenu, "_About", "app.about");

	// Insert help menu to main menu
	g_menu_insert_submenu(menu, 0, "_Help", G_MENU_MODEL(helpmenu));
	g_object_unref(helpmenu);

	// GAME MENU
	// Create restart menu
	gamemenu = g_menu_new();
	g_menu_append(gamemenu, "_Restart", "app.restart");

	// Create options section with separator
	optionsmenu = g_menu_new();

	// Create difficulty submenu
	difficultymenu = g_menu_new();
	g_menu_append(difficultymenu, "_Easy", "app.difficultyeasy");
	g_menu_append(difficultymenu, "_Normal", "app.difficultynormal");
	g_menu_append(difficultymenu, "_Hard", "app.difficultyhard");

	g_menu_insert_submenu(optionsmenu, 0, "_Difficulty", G_MENU_MODEL(difficultymenu));
	g_menu_append(optionsmenu, "_Highscore", "app.highscore");
	g_menu_append_section(gamemenu, NULL, G_MENU_MODEL(optionsmenu));

	// Create quitmenu with separator
	quitmenu = g_menu_new();
	g_menu_append(quitmenu, "_Quit", "app.quit");
	g_menu_append_section(gamemenu, NULL, G_MENU_MODEL(quitmenu));

	// Insert game menu to main menu
	g_menu_insert_submenu(menu, 0, "_Game", G_MENU_MODEL(gamemenu));

	// Free objects
	g_object_unref(optionsmenu);
	g_object_unref(difficultymenu);
	g_object_unref(quitmenu);
	g_object_unref(gamemenu);

	// Add callbacks to the menus
	g_action_map_add_action_entries(G_ACTION_MAP(w->app), menuCallbacks, G_N_ELEMENTS(menuCallbacks), (gpointer)w);

	menubar = gtk_menu_bar_new_from_model(G_MENU_MODEL(menu));
	gtk_box_pack_start(GTK_BOX(box), menubar, false, false, 0);
}

void createGrid(GtkWidget *box)
{
	w->grid = gtk_grid_new();

	// Make all cells same width and height
	gtk_grid_set_column_homogeneous(w->grid, true);
	gtk_grid_set_row_homogeneous(w->grid, true);

	gtk_box_pack_start(GTK_BOX(box), w->grid, true, true, 0);
}

void createStatusBar(GtkWidget *box)
{
	w->statusBar = gtk_statusbar_new();
	gtk_widget_set_size_request(w->statusBar, 500, 10);
	gtk_box_pack_start(GTK_BOX(box), w->statusBar, FALSE, FALSE, 0);
	w->statusID = gtk_statusbar_get_context_id(GTK_STATUSBAR(w->statusBar), "status");
}

void setStatusBarText(gpointer *text)
{
	gtk_statusbar_pop(GTK_STATUSBAR(w->statusBar), w->statusID);
	gtk_statusbar_push(GTK_STATUSBAR(w->statusBar), w->statusID, text);
}

gchar *createGameProgressBuffer()
{
	gchar *msg = g_malloc(MAX_STATUS_LENGTH);
	g_snprintf(msg, MAX_STATUS_LENGTH, "Clicks: %d, Time needed: %d seconds.", w->clicks, w->timeNeeded);
	return msg;
}

void setStatusBar()
{
	setStatusBarText(createGameProgressBuffer());
}

void checkWin()
{
	int x = 0;
	int y = 0;
	bool win = true;

	for (y = 0; y < w->fieldSize; y++)
	{
		for (x = 0; x < w->fieldSize; x++)
		{
			if (w->puzzleButtons[x][y]->currentX != w->puzzleButtons[x][y]->originalX ||
				w->puzzleButtons[x][y]->currentY != w->puzzleButtons[x][y]->originalY)
			{
				win = false;
				break;
			}
		}

		if (win == false)
		{
			break;
		}
	}

	if (!win)
	{
		return;
	}

	// Stop the timer
	stopTimer();

	// Create win dialog
	w->winDialog = gtk_application_window_new(w->app);
	gtk_window_set_application(GTK_WINDOW(w->winDialog), GTK_APPLICATION(w->app));
	gtk_window_set_position(GTK_WINDOW(w->winDialog), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(w->winDialog), "Image Puzzle");
	gtk_window_set_modal(GTK_WINDOW(w->winDialog), true);
	gtk_window_set_transient_for(GTK_WINDOW(w->winDialog), GTK_WINDOW(w->window));
	gtk_window_set_default_size(GTK_WINDOW(w->winDialog), 200, 100);
	gtk_window_set_deletable(GTK_WINDOW(w->winDialog), false);
	gtk_window_set_resizable(GTK_WINDOW(w->winDialog), false);

	// Create box in dialog
	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_container_add(GTK_CONTAINER(w->winDialog), box);

    // Create won label
	GtkWidget *wonLabel = gtk_label_new("You won!");
	gtk_box_pack_start(box, wonLabel, FALSE, FALSE, 0);

    // Create text label
	GtkWidget *textLabel = gtk_label_new(createGameProgressBuffer());
	gtk_box_pack_start(box, textLabel, FALSE, FALSE, 0);

	// Create input name label
	GtkWidget *inputNameLabel = gtk_label_new("Enter your name:");
	gtk_box_pack_start(box, inputNameLabel, FALSE, FALSE, 0);

	// Create entry field
	GtkWidget *nameEntry = gtk_entry_new();
	g_object_ref(nameEntry);
	gtk_box_pack_start(box, nameEntry, FALSE, FALSE, 0);

	GtkWidget *okButton = gtk_button_new_with_label("OK");
	g_signal_connect(okButton, "clicked", G_CALLBACK(winDialogOkButtonClicked), nameEntry);
	gtk_box_pack_start(box, okButton, FALSE, FALSE, 0);

	// Show win dialog
	gtk_widget_show_all(GTK_WIDGET(w->winDialog));
}

static void puzzleButtonClicked(GtkWidget *widget, gpointer data)
{
	puzzleButton *buttonStruct = (puzzleButton *)data;

	gchar *text = gtk_button_get_label(buttonStruct->button);

	w->clicks++;
	setStatusBar();

	if (!w->firstButtonClicked)
	{
		// First button clicked
		#if defined DEBUG
			g_print("First button: %s\n", text);
		#endif

		w->firstButton = buttonStruct;
		w->firstButtonClicked = true;
	}
	else
	{
		// Second button clicked
		#if defined DEBUG
			g_print("Second button: %s\n", text);
		#endif

		if (w->firstButton == buttonStruct)
		{
			g_print("WARNING: Cannot swap same button! :-?\n");
			setStatusBarText("Cannot swap same button! Choose another one!");
			return;
		}

		w->secondButton = buttonStruct;
		w->firstButtonClicked = false;

		#if defined DEBUG
			g_print("First  button x = %d, y = %d\n", w->firstButton->currentX, w->firstButton->currentY);
			g_print("Second button x = %d, y = %d\n\n", w->secondButton->currentX, w->secondButton->currentY);
		#endif

		// Increase the reference count
		g_object_ref(w->firstButton->button);
		g_object_ref(w->secondButton->button);

		// Remove the buttons from the grid
		gtk_container_remove(w->grid, w->firstButton->button);
		gtk_container_remove(w->grid, w->secondButton->button);

		// Swap currentX and currentY of the 2 buttons
		int tempX = w->firstButton->currentX;
		int tempY = w->firstButton->currentY;

		w->firstButton->currentX = w->secondButton->currentX;
		w->firstButton->currentY = w->secondButton->currentY;

		w->secondButton->currentX = tempX;
		w->secondButton->currentY = tempY;

		// Attach button in reverse order to the grid
		gtk_grid_attach(GTK_GRID(w->grid), w->firstButton->button, w->firstButton->currentX, w->firstButton->currentY, 1, 1);
		gtk_grid_attach(GTK_GRID(w->grid), w->secondButton->button, w->secondButton->currentX, w->secondButton->currentY, 1, 1);

		g_object_unref(w->firstButton->button);
		g_object_unref(w->secondButton->button);

		#if defined DEBUG
			g_print("First  button x = %d, y = %d\n", w->firstButton->currentX, w->firstButton->currentY);
			g_print("Second button x = %d, y = %d\n\n", w->secondButton->currentX, w->secondButton->currentY);
		#endif

		checkWin();
	}
}

bool checkFieldFree(int checkX, int checkY)
{
	int x = 0;
	int y = 0;
	bool fieldFree = true;

	for (y = 0; y < w->fieldSize; y++)
	{
		for (x = 0; x < w->fieldSize; x++)
		{
			if (w->puzzleButtons[x][y]->currentX == checkX && w->puzzleButtons[x][y]->currentY == checkY)
			{
				fieldFree = false;
				break;
			}
		}

		if (!fieldFree)
		{
			break;
		}
	}

	return fieldFree;
}

void TimerElapsed(void)
{
	#if defined DEBUG
		g_print("INFO: TimerElapsed()\n");
	#endif

	// Handle the times
	w->timeNeeded++;
	w->countDown--;

	// Update status bar
	setStatusBar();

	// Time out?
	if (w->countDown <= 0)
	{
		// Stop the timer
		stopTimer();

		// Show lose dialog
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new(w->window, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Sorry. Time ran out. You have lost! :(\n\nClick 'OK' to start a new game!");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		// Start a new game
		startGame();
	}
}

void startTimer()
{
	struct itimerval timerSettings;

	if (signal(SIGALRM, (void(*)(int))TimerElapsed) == SIG_ERR)
	{
		g_print("ERROR: Unable to set signal.\n\n");
		exit(-1);
	}

	timerSettings.it_value.tv_sec  = TIMER_INTERVAL / 1000;
	timerSettings.it_value.tv_usec = (TIMER_INTERVAL * 1000) % 1000000;
	timerSettings.it_interval = timerSettings.it_value;

	if (setitimer(ITIMER_REAL, &timerSettings, NULL) == -1)
	{
		g_print("ERROR: Error calling setitimer().\n\n");
		exit(-1);
	}
}

void stopTimer()
{
	struct itimerval timerSettings;

	timerSettings.it_value.tv_sec  = 0;
	timerSettings.it_value.tv_usec = 0;
	timerSettings.it_interval = timerSettings.it_value;

	if (setitimer(ITIMER_REAL, NULL, NULL) == -1)
	{
		g_print("ERROR: Error calling setitimer().\n\n");
		exit(-1);
	}
}

void startGame()
{
	#if defined DEBUG
		g_print("INFO: startGame()\n");
	#endif

	int x = 0;
	int y = 0;
	int n = 1;

	int randomX = 0;
	int randomY = 0;

	// Init a new game
	w->firstButton = NULL;
	w->secondButton = NULL;
	w->firstButtonClicked = false;
	w->clicks = 0;
	w->timeNeeded = 0;

	// Set difficulty specified settings
	switch (w->difficulty)
	{
		case EASY:
			w->fieldSize = SIZE_EASY;
			w->countDown = TIME_EASY;
			break;

		case NORMAL:
			w->fieldSize = SIZE_NORMAL;
			w->countDown = TIME_NORMAL;
			break;

		case HARD:
			w->fieldSize = SIZE_HARD;
			w->countDown = TIME_HARD;
			break;

		default:
			// Should never happen
			g_print("FATAL ERROR: Game difficulty not set! :(\n\n");
			exit(-1000);
			break;
	}

	// Init random number generator
	srand(time(NULL));

	// Safely destroy all button widgets and struct
	for (y = 0; y < SIZE_HARD; y++)
	{
		for (x = 0; x < SIZE_HARD; x++)
		{
			if (w->puzzleButtons[x][y] == NULL)
			{
				continue;
			}

			if (w->puzzleButtons[x][y]->button == NULL)
			{
				continue;
			}

			// Remove the button from container
			gtk_container_remove(w->grid, w->puzzleButtons[x][y]->button);

			// Invalidate the button pointer
			w->puzzleButtons[x][y]->button = NULL;

			// Free the struct
			g_free(w->puzzleButtons[x][y]);

			// Invalidate the struct pointer
			w->puzzleButtons[x][y] = NULL;
		}
	}

	// Loop through the fields
	for (y = 0; y < w->fieldSize; y++)
	{
		for (x = 0; x < w->fieldSize; x++)
		{
			// Allocate memory and create a new button
			w->puzzleButtons[x][y] = g_malloc(sizeof(puzzleButton));
			w->puzzleButtons[x][y]->button = gtk_button_new();

			// Set callback
			g_signal_connect(w->puzzleButtons[x][y]->button, "clicked", G_CALLBACK(puzzleButtonClicked), w->puzzleButtons[x][y]);

			// Increase the reference count
			g_object_ref(w->puzzleButtons[x][y]->button);

			// Init all button positions with -1
			w->puzzleButtons[x][y]->currentX  = -1;
			w->puzzleButtons[x][y]->currentY  = -1;
			w->puzzleButtons[x][y]->originalX = -1;
			w->puzzleButtons[x][y]->originalY = -1;
		}
	}

	// Place buttons to random positions
	for (y = 0; y < w->fieldSize; y++)
	{
		for (x = 0; x < w->fieldSize; x++)
		{
			char labelBuffer[8];
			snprintf(labelBuffer, sizeof(labelBuffer), "%d", n);

			// Check free position
			do
			{
				randomX = rand() % w->fieldSize;
				randomY = rand() % w->fieldSize;
			} while (!checkFieldFree(randomX, randomY));

			// Set button properties
			gtk_button_set_label(w->puzzleButtons[x][y]->button, labelBuffer);
			w->puzzleButtons[x][y]->currentX = randomX;
			w->puzzleButtons[x][y]->currentY = randomY;
			w->puzzleButtons[x][y]->originalX = x;
			w->puzzleButtons[x][y]->originalY = y;

			// Attach button to grid
			gtk_grid_attach(GTK_GRID(w->grid), w->puzzleButtons[x][y]->button, randomX, randomY, 1, 1);

			// Decrease the reference count
			g_object_unref(w->puzzleButtons[x][y]->button);

			gtk_widget_show(w->puzzleButtons[x][y]->button);

			n++;
		}
	}

	// Get window width
	int currentWidth = 0, currentHeight = 0;
	gtk_window_get_size(w->window, &currentWidth, &currentHeight);

	// Set window height = width + 50
	gtk_window_resize(w->window, currentWidth, currentWidth + 50);

	// Update status bar
	setStatusBar();

	// Start the timer
	startTimer();
}

/*void my_getsize(GtkWidget *widget, GtkAllocation *allocation, void *data)
{
	g_print("resize!\n");
}
*/

// app activate callback - creates the window
static void activate(GtkApplication *app, gpointer data)
{
	// Create the window
	widgets *w = (widgets *)data;

	w->window = gtk_application_window_new(app);
	gtk_window_set_application(GTK_WINDOW(w->window), GTK_APPLICATION(app));
	gtk_window_set_position(GTK_WINDOW(w->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(w->window), "Image Puzzle");
	gtk_window_set_default_size(GTK_WINDOW(w->window), 500, 550);
	gtk_window_set_default_icon_from_file("icon.png", NULL);

	//g_signal_connect(w->window, "size-allocate", G_CALLBACK(my_getsize), NULL);

	// Create a vertical box
	//GtkWidget *box;
	w->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(w->window), w->box);

	createMenu(w->box);

	// create a grid to be used as layout container
	createGrid(w->box);

	createStatusBar(w->box);

  	startGame();

  	gtk_widget_show_all(GTK_WIDGET(w->window));
}

int main (int argc, char **argv)
{
	int status;

	w = g_malloc(sizeof(widgets));

	// Set App default settings
	w->difficulty = EASY;

	// Create the application
	w->app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(w->app, "activate", G_CALLBACK(activate), (gpointer)w);
	status = g_application_run(G_APPLICATION(w->app), argc, argv);



	g_object_unref(w->app);

	g_free(w);
	return status;
}
