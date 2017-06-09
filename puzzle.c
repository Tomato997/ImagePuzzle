// clang puzzle.c $(pkg-config --cflags --libs gtk+-3.0) -Wall -g -o puzzle
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

// globals
GtkWidget *label_output;
GtkWidget *input_entry;

typedef struct {
	GtkApplication *app;
	GtkWidget *window;
} bmd_widgets;

// callback executed when the "Okay" button is clicked
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
}

void bmd_quit_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	bmd_widgets *a = (bmd_widgets *) data;

	g_print ("Quit was clicked ...\n");
	g_application_quit (G_APPLICATION (a->app));
}

void bmd_open_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	bmd_widgets *a = (bmd_widgets *) data;

	//bmd_message_dialog (action, NULL, (gpointer) a);
	g_print ("Open was clicked ...\n");
}

void bmd_save_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	bmd_widgets *a = (bmd_widgets *) data;

	//bmd_message_dialog (action, NULL, (gpointer) a);
	g_print ("Save was clicked ...\n");
}

void bmd_saveAs_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	bmd_widgets *a = (bmd_widgets *) data;

	//bmd_message_dialog (action, NULL, (gpointer) a);
	g_print ("Save As was clicked ...\n");
}

// map actions to callbacks
const GActionEntry app_entries[] = {
	{ "open", bmd_open_callback, NULL, NULL, NULL },
	{ "save", bmd_save_callback, NULL, NULL, NULL },
	{ "saveAs", bmd_saveAs_callback, NULL, NULL, NULL },
	{ "quit", bmd_quit_callback, NULL, NULL, NULL }
};

/*
const GActionEntry app_entries[] = {
	{ "Restart", bmd_restart_callback, NULL, NULL, NULL },
	{ "Difficulty...", bmd_difficulty_callback, NULL, NULL, NULL },
	{ "High Score", bmd_highscore_callback, NULL, NULL, NULL },
	{ "Quit", bmd_quit_callback, NULL, NULL, NULL }
};*/



// app activate callback - creates the window
static void activate(GtkApplication* app, gpointer user_data)
{
	GtkWidget *box;
	bmd_widgets *a = (bmd_widgets *)user_data;

	a->window = gtk_application_window_new(app);
	gtk_window_set_application (GTK_WINDOW(a->window), GTK_APPLICATION (app));
	gtk_window_set_position (GTK_WINDOW(a->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title (GTK_WINDOW(a->window), "Image Puzzle");
	gtk_window_set_default_size(GTK_WINDOW (a->window), 500, 300);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER (a->window), box);

	// Menü
	GtkWidget *window;
	//GtkWidget *grid;
	//GtkWidget *label_name;
	GtkWidget *clr_button, *ok_button;

	GtkWidget *menubar;
	GMenu *menu, *filemenu, *editmenu, *helpmenu, *savemenu, *quitmenu;

	g_action_map_add_action_entries (G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), (gpointer)a);

	menu = g_menu_new();
	filemenu = g_menu_new();
	g_menu_append (filemenu, "_Open", "app.open");

	savemenu = g_menu_new();
	g_menu_append (savemenu, "_Save", "app.save");
	g_menu_append (savemenu, "Save As ...", "app.saveAs");

	g_menu_append_section (filemenu, NULL, G_MENU_MODEL (savemenu));
	quitmenu = g_menu_new();
	g_menu_append (quitmenu, "_Quit", "app.quit");
	g_menu_append_section (filemenu, NULL, G_MENU_MODEL (quitmenu));
	g_menu_insert_submenu (menu, 0, "_File", G_MENU_MODEL (filemenu));
	g_object_unref (savemenu);
	g_object_unref (quitmenu);
	g_object_unref (filemenu);

	menubar = gtk_menu_bar_new_from_model (G_MENU_MODEL (menu));
	gtk_box_pack_start (GTK_BOX (box), menubar, FALSE, FALSE, 0);


  // create a grid to be used as layout containerr
  //grid = gtk_grid_new();

	/*

  // output label
  label_output = gtk_label_new ("Hello?");
  gtk_grid_attach(GTK_GRID(grid), label_output, 0,0,2,1);

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
	gtk_widget_show_all (GTK_WIDGET (a->window));
}

int main (int argc, char **argv)
{
	int status;
	bmd_widgets *a = g_malloc (sizeof (bmd_widgets));

	a->app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (a->app, "activate", G_CALLBACK (activate), (gpointer) a);
	status = g_application_run (G_APPLICATION (a->app), argc, argv);
	g_object_unref (a->app);

	g_free (a);
	return status;
}
