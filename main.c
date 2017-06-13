// clang puzzle.c $(pkg-config --cflags --libs gtk+-3.0) -Wall -g -o puzzle

#include "include.h"

#define MAX_STATUS_LENGTH	256

#define EASY_MAX_BUTTONS	4
#define NORMAL_MAX_BUTTONS	8
#define HARD_MAX_BUTTONS	12


const int difficultyMaxButtons[3] = {EASY_MAX_BUTTONS, NORMAL_MAX_BUTTONS, HARD_MAX_BUTTONS};

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
	GtkWidget *statusBar;
	guint statusID;
	guint difficulty;
	GtkWidget *grid;
	puzzleButton *puzzleButtons[HARD_MAX_BUTTONS][HARD_MAX_BUTTONS];
	puzzleButton *firstButton;
	puzzleButton *secondButton;
	bool firstButtonClicked;
} widgets;

widgets *w = NULL;

// callback executed when the "Okay" button is clicked
/*
static void ok_clicked(GtkWidget *widget, gpointer data)
{
  gchar *buffer1, *buffer2;

  // obtain text from the entry box
  buffer1 = (gchar*)gtk_entry_get_text(GTK_ENTRY(input_entry));
  // allocate memory for the final text
  buffer2 = g_malloc(sizeof(gchar)*(strlen(buffer1)+7));
  // assemble the final text
  sprintf(buffer2,"Hello %s!", buffer1);
  // write the final text to the label on top
  gtk_label_set_text(GTK_LABEL(label_output), buffer2);
  // free the memory
  g_free(buffer2);
}

// callback that is executed when the "Clear" button is clicked
static void clr_clicked(GtkWidget *widget, gpointer data)
{
  // clear the entry box
  gtk_entry_set_text(GTK_ENTRY(input_entry), "");
  // clear the label
  gtk_label_set_text(GTK_LABEL(label_output), "Hello ?");
}*/


void menuRestartCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *w = (widgets *)data;

	//bmd_message_dialog (action, NULL, (gpointer) a);
	g_print("menuRestartCallback...\n");
}

void menuDifficultyCallback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *w = (widgets *)data;

	//bmd_message_dialog (action, NULL, (gpointer) a);
	g_print ("menuDifficultyCallback...\n");
}

void menuDifficultyEasyCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	w->difficulty = 0;
	g_print("menuDifficultyEasyCallback...\n");
	setStatusBar(w->difficulty, 0);
}

void menuDifficultyNormalCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	w->difficulty = 1;
	g_print("menuDifficultyNormalCallback...\n");
	setStatusBar(w->difficulty, 0);
}

void menuDifficultyHardCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	w->difficulty = 2;
	g_print("menuDifficultyHardCallback...\n");
	setStatusBar(w->difficulty, 0);
}

void menuHighScoreCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *w = (widgets *)data;

	//message_dialog (action, NULL, (gpointer) w);
	g_print("menuHighScoreCallback...\n");
}

void menuQuitCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *w = (widgets *)data;

	g_print("menuQuitCallback...\n");
	g_application_quit(G_APPLICATION(w->app));
}

void menuAboutCallback(GSimpleAction *action, GVariant *parameter, gpointer data)
{
	widgets *w = (widgets *)data;

	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(w->window, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Image Puzzle v1.0\n\nCopyright (C) 2017 by Felix Knobl.");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

// map actions to callbacks
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

	/*GtkWidget *label_output1;
	label_output1 = gtk_label_new ("=== 1 ===");

	GtkWidget *label_output2;
	label_output2 = gtk_label_new ("=== 2 ===");

	GtkWidget *label_output3;
	label_output3 = gtk_label_new ("=== 3 ===");
	gtk_grid_attach(GTK_GRID(grid), label_output1, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), label_output2, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), label_output3, 2, 2, 1, 1);

	*/
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
	gtk_statusbar_push (GTK_STATUSBAR(w->statusBar), w->statusID, text);
}

void setStatusBar(int clicks, unsigned long time)
{
	gchar *msg = g_malloc(MAX_STATUS_LENGTH);

	g_snprintf(msg, MAX_STATUS_LENGTH, "Clicks: %d, Time needed: %lu", clicks, time);
	setStatusBarText(msg);
}

static void puzzleButtonClicked(GtkWidget *widget, gpointer data)
{
	puzzleButton *buttonStruct = (puzzleButton *)data;
	//GtkButton    *button       = (GtkButton *)widget;

	gchar *text = gtk_button_get_label(buttonStruct->button);

	if (!w->firstButtonClicked)
	{
		// First button clicked
		g_print("First button: %s\n", text);
		w->firstButton = buttonStruct;
		w->firstButtonClicked = true;
	}
	else
	{
		// Second button clickd
		g_print("Second button: %s\n", text);

		if (w->firstButton == buttonStruct)
		{
			g_print("WARNING: Cannot swap same button! :-?\n");
			setStatusBarText("Cannot swap same button! Choose another one!");
			return;
		}

		w->secondButton = buttonStruct;
		w->firstButtonClicked = false;

		g_print("First  button x = %d, y = %d\n", w->firstButton->currentX, w->firstButton->currentY);
		g_print("Second button x = %d, y = %d\n\n", w->secondButton->currentX, w->secondButton->currentY);

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

		g_print("First  button x = %d, y = %d\n", w->firstButton->currentX, w->firstButton->currentY);
		g_print("Second button x = %d, y = %d\n\n", w->secondButton->currentX, w->secondButton->currentY);
	}
}

void gameStart()
{
	int x = 0;
	int y = 0;
	int n = 1;

	// Init a new game
	w->firstButton = NULL;
	w->secondButton = NULL;
	w->firstButtonClicked = false;

	for (y = 0; y < difficultyMaxButtons[w->difficulty]; y++)
	{
		for (x = 0; x < difficultyMaxButtons[w->difficulty]; x++)
		{
			char labelBuffer[8];
			snprintf(labelBuffer, sizeof(labelBuffer), "%d", n);

			w->puzzleButtons[x][y] = g_malloc(sizeof(puzzleButton));
			w->puzzleButtons[x][y]->button = gtk_button_new_with_label(labelBuffer);
			w->puzzleButtons[x][y]->currentX = x;
			w->puzzleButtons[x][y]->currentY = y;

			gtk_grid_attach(GTK_GRID(w->grid), w->puzzleButtons[x][y]->button, x, y, 1, 1);
			g_signal_connect(w->puzzleButtons[x][y]->button, "clicked", G_CALLBACK(puzzleButtonClicked), w->puzzleButtons[x][y]);

			n++;
		}
	}
}

// app activate callback - creates the window
static void activate(GtkApplication *app, gpointer data)
{
	// Create the window
	widgets *w = (widgets *)data;

	w->window = gtk_application_window_new(app);
	gtk_window_set_application(GTK_WINDOW(w->window), GTK_APPLICATION(app));
	gtk_window_set_position(GTK_WINDOW(w->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(w->window), "Image Puzzle");
	gtk_window_set_default_size(GTK_WINDOW(w->window), 500, 500);

	gtk_window_set_default_icon_from_file("icon.png", NULL);

	// Create a vertical box
	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(w->window), box);

	createMenu(box);

	// create a grid to be used as layout containerr
	createGrid(box);

	createStatusBar(box);

	setStatusBar(w->difficulty, 0);

	gameStart();

  //GtkWidget *clr_button, *ok_button;
	/*

  // output label


  // name label
  label_name = gtk_label_new ("Name:");
  gtk_grid_attach(GTK_GRID(grid), label_name, 0,1,1,1);

  // text entry
  input_entry = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), input_entry, 1,1,1,1);

  // CLEAR button
  clr_button = gtk_button_new_with_mnemonic("_Clear");
  gtk_grid_attach(GTK_GRID(grid), clr_button, 0,2,1,1);
  // connect a signal when the button is clicked -> invoke clr_clicked() callback
  g_signal_connect(clr_button, "clicked", G_CALLBACK(clr_clicked), NULL);

  // OKAY button
  ok_button = gtk_button_new_with_mnemonic("_Okay");
  gtk_grid_attach(GTK_GRID(grid), ok_button, 0,3,1,1);
  // connect a signal when the button is clicked -> invoke cok_clicked() callback
  g_signal_connect(ok_button, "clicked", G_CALLBACK(ok_clicked), NULL);

	 *
	 */
  //gtk_widget_show_all (window);
	gtk_widget_show_all(GTK_WIDGET(w->window));
}

int main (int argc, char **argv)
{
	int status;

	w = g_malloc(sizeof(widgets));

	g_print("Size: %d\n", sizeof(widgets));

	w->difficulty = 1;

	// Create the application
	w->app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(w->app, "activate", G_CALLBACK(activate), (gpointer)w);
	status = g_application_run(G_APPLICATION(w->app), argc, argv);
	g_object_unref(w->app);

	g_free(w);
	return status;
}
