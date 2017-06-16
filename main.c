// clang main.c $(pkg-config --cflags --libs gtk+-3.0) -Wall -g -o main

#include "include.h"

void startGame();
void startTimer();
void stopTimer();

widgets *w = NULL;

// Loads an image from a file to the GdkPixbuf
GdkPixbuf *loadImage(const gchar *fileName)
{
    GdkPixbuf *buffer;
    GError *error = NULL;
    buffer = gdk_pixbuf_new_from_file(fileName, &error);

    if (error != NULL)
    {
		g_error_free(error);
		return NULL;
    }

    if (buffer == NULL)
    {
		return NULL;
    }

	return buffer;
}

// Displays a simple dialog with an "OK" button
void showSimpleDialog(const gchar *text)
{
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(GTK_WINDOW(w->window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, APP_NAME);

	if (dialog == NULL)
	{
		g_print("ERROR: Could not create a message dialog.\n\n");
		exit(-2);
	}

	gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog), text);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


// Returns the full path of the highscore file name
// Pointer musst be freed after use
// E.g. getEnvPathFileName("HOME", "imagepuzzle", "file.png");
//      Returns: "/home/fhtw/imagepuzzle/file.png"
char *getEnvPathFileName(const char *env, const char *path, const char *fileName)
{
	// Open the file if it exists
	char *returnValue = NULL;
	char *pReturnValue = NULL;
	char *envPath = NULL;

	// Get the environment path
	envPath = getenv(env);

	// Check the environment path
	if (envPath == NULL)
	{
		g_print("ERROR: getEnvPathFileName(): Could not get environment path!\n");
		return NULL;
	}

	// Check total path length and add 2 more chars for slash (must not exceed MAX_PATH_LENGTH - 1)
	if (strlen(envPath) + strlen(path) + strlen(fileName) + 2 > MAX_PATH_LENGTH - 1)
	{
		g_print("ERROR: getEnvPathFileName(): Return value exceeds %d chars.\n\n", MAX_PATH_LENGTH);
		return NULL;
	}

	// Create a buffer for the return value
	returnValue = (char *)malloc(MAX_PATH_LENGTH * sizeof(char));
	memset((void *)returnValue, 0, MAX_PATH_LENGTH * sizeof(char));

	// Copy envPath to return value
	strncpy(returnValue, envPath, strlen(envPath));

	// Move the pointer to the last character of the string
	pReturnValue = returnValue + strlen(returnValue) - 1;

	// Add a slash if needed
	if (*pReturnValue != '/')
	{
		// Character is not a slash
		// Increment pointer and set the next character to '/'
		*++pReturnValue = '/';
	}

	// Copy path to the end of the string
	strncpy(returnValue + strlen(returnValue), path, strlen(path));

	// Move the pointer to the last character of the string
	pReturnValue = returnValue + strlen(returnValue) - 1;

	// Add a slash if needed
	if (*pReturnValue != '/')
	{
		// Character is not a slash
		// Increment pointer and set the next character to '/'
		*++pReturnValue = '/';
	}

	// Copy fileName to the end of the string
	strncpy(returnValue + strlen(returnValue), fileName, strlen(fileName));

	pReturnValue = NULL;
	return returnValue;
}

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

static void highscoreDialogCloseButtonClicked(GtkWidget *widget, gpointer data)
{
	// Restore the previous state of the timer
	if (w->saveTimerStarted)
	{
		startTimer();
	}

	// Close the window
	gtk_window_close(GTK_WINDOW(w->highscoreDialog));
}

enum
{
   NAME_COLUMN = 0,
   CLICKS_COLUMN,
   TIME_COLUMN,
   N_COLUMNS
};

void setNoHighscores(GtkWidget *box)
{
	GtkWidget *noHSLabel = gtk_label_new("<span weight=\"bold\">There are currently no highscores.</span>");
	gtk_label_set_use_markup(GTK_LABEL(noHSLabel), true);
	gtk_box_pack_start(GTK_BOX(box), noHSLabel, true, true, 0);
}

void menuHighScoreCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	int n = 0;

	// Save the state of the timer
	w->saveTimerStarted = w->timerStarted;

	// Stop the timer if it is running
	if (w->timerStarted)
	{
		stopTimer();
	}

	// Create win dialog
	w->highscoreDialog = gtk_application_window_new(w->app);
	gtk_window_set_application(GTK_WINDOW(w->highscoreDialog), GTK_APPLICATION(w->app));
	gtk_window_set_position(GTK_WINDOW(w->highscoreDialog), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(w->highscoreDialog), "Highscore Table");
	gtk_window_set_modal(GTK_WINDOW(w->highscoreDialog), true);
	gtk_window_set_transient_for(GTK_WINDOW(w->highscoreDialog), GTK_WINDOW(w->window));
	gtk_window_set_default_size(GTK_WINDOW(w->highscoreDialog), 350, 500);
	gtk_window_set_resizable(GTK_WINDOW(w->highscoreDialog), false);

	// Create box in dialog
	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(w->highscoreDialog), box);

  	// Get the file name of the highscore file
	FILE *fs = NULL;
	char *fileName = NULL;
	char *currentLine = NULL;
	size_t len = 0;
	int lineNumber = 0;

	fileName = getEnvPathFileName("HOME", "", HIGHSCORE_FILE_NAME);

	if (fileName == NULL)
	{
		g_print("ERROR: Could not get highscore filename.\n\n");
		exit(-1);
	}

	#if defined DEBUG
		g_print("INFO: Highscore file is: '%s'\n", fileName);
	#endif

	// Open the file readonly
	fs = fopen(fileName, "r");

	if (fs == NULL)
	{
		// File not found or can not access it
		setNoHighscores(box);

		#if defined DEBUG
			g_print("INFO: Can not open file '%s'.\n\n", fileName);
		#endif
	}
	else
	{
		GtkListStore *listItems;
		GtkTreeIter iter;

		// Read each line of the highscore file
		while (getline(&currentLine, &len, fs) != -1)
		{
			char *name = NULL;
			char *clicks = NULL;
			char *time = NULL;

			// Is the line empty?
			if (strlen(currentLine) <= 1)
			{
				continue;
			}

			name = strtok(currentLine, "|");

			// Line valid?
			if (name == NULL)
			{
				continue;
			}

			clicks = strtok(NULL, "|");

			// Line valid?
			if (clicks == NULL)
			{
				continue;
			}

			time = strtok(NULL, "\n");

			// Line valid?
			if (time == NULL)
			{
				continue;
			}

			#if defined DEBUG
				g_print("Name: '%s' Clicks: '%s' Time: '%s'\n", name, clicks, time);
			#endif

			// Valid line. Increment lineNumber
			lineNumber++;

			// Is this the first highscore?
			if (lineNumber == 1)
			{
				// Create a new listItems collection
				listItems = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
			}

			// Add a new listview item to the collection
			gtk_list_store_append(listItems, &iter);
			gtk_list_store_set(listItems, &iter, NAME_COLUMN, name, CLICKS_COLUMN, clicks, TIME_COLUMN, time, -1);
		}

		// Close the file stream
		if (fclose(fs) != 0)
		{
			g_print("ERROR: fclose() failed.\n\n");
			exit(-1);
		}

		// Free the line buffer
		free(currentLine);

		// Free the filename buffer
		free(fileName);

		// Are there any highscores?
		if (lineNumber > 0)
		{
			// Create a new listview
			GtkWidget *listView;
			GtkCellRenderer *cellRenderer;
			GtkTreeViewColumn *currentColumn;

			listView = gtk_tree_view_new();

			// Create column headers
			cellRenderer = gtk_cell_renderer_text_new();
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(listView), -1, "Name", cellRenderer, "text", NAME_COLUMN, NULL);

			cellRenderer = gtk_cell_renderer_text_new();
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(listView), -1, "Clicks", cellRenderer, "text", CLICKS_COLUMN, NULL);

			cellRenderer = gtk_cell_renderer_text_new();
			gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(listView), -1, "Time", cellRenderer, "text", TIME_COLUMN, NULL);

			// Resize columns to take up all the space
			for (n = 0; n < N_COLUMNS; n++)
			{
				currentColumn = gtk_tree_view_get_column(GTK_TREE_VIEW(listView), n);

				if (currentColumn != NULL)
				{
					gtk_tree_view_column_set_sizing(currentColumn, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
					gtk_tree_view_column_set_expand(currentColumn, true);
				}
			}

			// Attach the list item collection to the listview
			gtk_tree_view_set_model(GTK_TREE_VIEW(listView), GTK_TREE_MODEL(listItems));

			// Unref listItems
			g_object_unref(GTK_TREE_MODEL(listItems));

			// Add the listview to the box
			gtk_box_pack_start(GTK_BOX(box), listView, true, true, 10);
		}
	}

	// Add close button
	GtkWidget *closeButton = gtk_button_new_with_label("Close");
	g_signal_connect(closeButton, "clicked", G_CALLBACK(highscoreDialogCloseButtonClicked), NULL);
	gtk_box_pack_start(GTK_BOX(box), closeButton, FALSE, FALSE, 0);

	// Show highscore dialog
	gtk_widget_show_all(GTK_WIDGET(w->highscoreDialog));
}

void menuQuitCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	g_application_quit(G_APPLICATION(w->app));
}

void menuAboutCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	showSimpleDialog(APP_COPYRIGHT);
}

static void winDialogOkButtonClicked(GtkWidget *widget, gpointer data)
{
	GtkWidget *nameEntry = (GtkWidget *)data;
	gchar *nameBuffer = (gchar *)gtk_entry_get_text(GTK_ENTRY(nameEntry));

	// Any name entered?
	if (strlen(nameBuffer) == 0)
	{
		showSimpleDialog("Please enter a name.");
		return;
	}

	// Check name length
	if (strlen(nameBuffer) > MAX_NAME_LENGTH)
	{
		showSimpleDialog("ERROR: The name is too long.");
		return;
	}

	// Save highscore
	FILE *fs = NULL;
	char *fileName = NULL;

	// Get the file name of the highscore file
	fileName = getEnvPathFileName("HOME", "", HIGHSCORE_FILE_NAME);

	if (fileName == NULL)
	{
		g_print("ERROR: Could not get highscore filename.\n\n");
		exit(-1);
	}

	#if defined DEBUG
		g_print("INFO: Highscore file is: '%s'\n", fileName);
	#endif

	// Open the file for Append access
	fs = fopen(fileName, "a");

	if (fs == NULL)
	{
		g_print("ERROR: Can not append file '%s'.\n\n", fileName);
		exit(-2);
	}

	// Append line to the highscore file
	if (fprintf(fs, "%s|%d|%d\n", nameBuffer, w->clicks, w->timeNeeded) == -1)
	{
		g_print("ERROR: Can not append the line.\n\n");

		// Try to close the file anyway
		fclose(fs);
		exit(-2);
	}

	// Close the file stream
	if (fclose(fs) != 0)
	{
		g_print("ERROR: fclose() failed.\n\n");
		exit(-1);
	}

	// Free the filename buffer
	free(fileName);

	gtk_window_close(GTK_WINDOW(w->winDialog));

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

void createMenu()
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
	gtk_box_pack_start(GTK_BOX(w->box), menubar, false, false, 0);
}

void createGrid()
{
	w->grid = gtk_grid_new();

	// Make all cells same width and height
	gtk_grid_set_column_homogeneous(GTK_GRID(w->grid), true);
	gtk_grid_set_row_homogeneous(GTK_GRID(w->grid), true);

	// Add the grid to the box
	gtk_box_pack_start(GTK_BOX(w->box), w->grid, true, true, 0);
}

void createStatusBar()
{
	w->statusBar = gtk_statusbar_new();
	gtk_widget_set_size_request(w->statusBar, 500, 10);
	gtk_box_pack_start(GTK_BOX(w->box), w->statusBar, FALSE, FALSE, 0);
	w->statusID = gtk_statusbar_get_context_id(GTK_STATUSBAR(w->statusBar), "status");
}

void setStatusBarText(const gchar *text)
{
	// Remove current message and push the new one
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
	gtk_box_pack_start(GTK_BOX(box), wonLabel, FALSE, FALSE, 0);

    // Create text label
	GtkWidget *textLabel = gtk_label_new(createGameProgressBuffer());
	gtk_box_pack_start(GTK_BOX(box), textLabel, FALSE, FALSE, 0);

	// Create input name label
	GtkWidget *inputNameLabel = gtk_label_new("Enter your name:");
	gtk_box_pack_start(GTK_BOX(box), inputNameLabel, FALSE, FALSE, 0);

	// Create entry field
	GtkWidget *nameEntry = gtk_entry_new();
	g_object_ref(nameEntry);
	gtk_box_pack_start(GTK_BOX(box), nameEntry, FALSE, FALSE, 0);

	GtkWidget *okButton = gtk_button_new_with_label("OK");
	g_signal_connect(okButton, "clicked", G_CALLBACK(winDialogOkButtonClicked), nameEntry);
	gtk_box_pack_start(GTK_BOX(box), okButton, FALSE, FALSE, 0);

	// Show win dialog
	gtk_widget_show_all(GTK_WIDGET(w->winDialog));
}

static void puzzleButtonClicked(GtkWidget *widget, gpointer data)
{
	puzzleButton *buttonStruct = (puzzleButton *)data;

	const gchar *text = gtk_button_get_label(GTK_BUTTON(buttonStruct->button));

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

		w->firstButtonClicked = false;

		if (w->firstButton == buttonStruct)
		{
			showSimpleDialog("Cannot swap the same button!");
			return;
		}

		w->secondButton = buttonStruct;

		#if defined DEBUG
			g_print("First  button x = %d, y = %d\n", w->firstButton->currentX, w->firstButton->currentY);
			g_print("Second button x = %d, y = %d\n\n", w->secondButton->currentX, w->secondButton->currentY);
		#endif

		// Increase the reference count
		g_object_ref(w->firstButton->button);
		g_object_ref(w->secondButton->button);

		// Remove the buttons from the grid
		gtk_container_remove(GTK_CONTAINER(w->grid), w->firstButton->button);
		gtk_container_remove(GTK_CONTAINER(w->grid), w->secondButton->button);

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
	#if defined DEBUG2
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
		showSimpleDialog("Sorry. Time ran out. You have lost! :(\n\nClick 'OK' to start a new game!");

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

	w->timerStarted = true;
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

	w->timerStarted = false;
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
			// Is struct NULL?
			if (w->puzzleButtons[x][y] == NULL)
			{
				continue;
			}

			// Is button widget NULL?
			if (w->puzzleButtons[x][y]->button == NULL)
			{
				continue;
			}

			// Remove the button from container
			gtk_container_remove(GTK_CONTAINER(w->grid), w->puzzleButtons[x][y]->button);

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
			gtk_button_set_label(GTK_BUTTON(w->puzzleButtons[x][y]->button), labelBuffer);
			w->puzzleButtons[x][y]->currentX = randomX;
			w->puzzleButtons[x][y]->currentY = randomY;
			w->puzzleButtons[x][y]->originalX = x;
			w->puzzleButtons[x][y]->originalY = y;

			// Attach button to grid
			gtk_grid_attach(GTK_GRID(w->grid), w->puzzleButtons[x][y]->button, randomX, randomY, 1, 1);

			// Decrease the reference count
			g_object_unref(w->puzzleButtons[x][y]->button);

			// Show the button
			gtk_widget_show(w->puzzleButtons[x][y]->button);

			n++;
		}
	}

	// Get window width
	int currentWidth = 0, currentHeight = 0;
	gtk_window_get_size(GTK_WINDOW(w->window), &currentWidth, &currentHeight);

	// Set window height = width + 50
	gtk_window_resize(GTK_WINDOW(w->window), currentWidth, currentWidth + 50);

	// Update status bar
	setStatusBar();

	// Start the timer
	startTimer();
}

// App activate callback
static void activate(GtkApplication *app, gpointer data)
{
	// Create the window
	widgets *w = (widgets *)data;

	w->window = gtk_application_window_new(app);
	gtk_window_set_application(GTK_WINDOW(w->window), GTK_APPLICATION(app));
	gtk_window_set_position(GTK_WINDOW(w->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(w->window), APP_NAME);
	gtk_window_set_default_size(GTK_WINDOW(w->window), 500, 550);
	gtk_window_set_default_icon_from_file("icon.png", NULL);

	// Create a vertical box
	w->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(w->window), w->box);

	createMenu();

	createGrid();

	createStatusBar();

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
