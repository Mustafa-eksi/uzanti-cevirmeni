#include <gtk/gtk.h>
#include <gio/gcancellable.h>
#include <gio/gasyncresult.h>

GtkWidget *window;
GtkFileDialog *fd;
GCancellable *cancel;

enum ConverterPrograms {
	FFMPEG,
	IMAGEMAGICK,
	PANDOC
};



void async_callback(GObject *source_object, GAsyncResult *res, gpointer data)
{
	GError *err;
	GFile *a = gtk_file_dialog_open_finish(fd, res, &err);
	char *path = g_file_get_path(a);
	if (!path) return;

}


void clicked(GtkWidget *widget, gpointer data)
{
	fd = gtk_file_dialog_new();
	cancel = g_cancellable_new();

	gtk_file_dialog_open(fd, GTK_WINDOW(window), cancel, async_callback, NULL);
}

static void activate(GtkApplication *app, gpointer user_data)
{
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Window");
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

	GtkWidget *b = gtk_button_new();

	gtk_button_set_label(GTK_BUTTON(b), "Dosya seçin");
	gtk_window_set_child(GTK_WINDOW(window), b);
	g_signal_connect(b, "clicked", G_CALLBACK(clicked), NULL);

	gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
	GtkApplication *app;
	int status;

	app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
