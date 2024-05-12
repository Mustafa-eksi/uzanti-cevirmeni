#include <gtk/gtk.h>
#include <gtk/gtkorientable.h>
#include <gio/gcancellable.h>
#include <gio/gasyncresult.h>
#include <string.h>
#include "cevir.c"

GtkWidget *window;
GtkFileDialog *fd;
GCancellable *cancel;
GtkWidget *dd;
GtkWidget *convert_button;
char *path;
enum ConverterProgram cp;

void async_callback(GObject *source_object, GAsyncResult *res, gpointer data)
{
	GError *err;
	GFile *a = gtk_file_dialog_open_finish(fd, res, &err);
	path = g_file_get_path(a);
	if (!path) return;
	char *ext = get_extension(path);
	cp = get_converter_from_extension(ext);
	//printf("converter: '%s'\n", ext);
	GtkStringList *strlist;
	if ((cp & IMAGEMAGICK) != 0) {
		strlist = gtk_string_list_new(IMAGEMAGICK_CONVERTABLE);
	} else if ((cp & FFMPEG) != 0) {
		strlist = gtk_string_list_new(FFMPEG_CONVERTABLE);
	} else if ((cp & PANDOC) != 0) {
		strlist = gtk_string_list_new(PANDOC_OUTPUT);
	} else {
		// TODO: Show error alert.
		printf("ERROR!: no converter program had found for this extension! (%s)\n", ext);
		return;
	}
	gtk_drop_down_set_model(GTK_DROP_DOWN(dd), G_LIST_MODEL(strlist));
	gtk_widget_set_visible(dd, true);
	gtk_widget_set_visible(convert_button, true);
}

void convert_clicked(GtkWidget *widget, gpointer data)
{
	guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(dd));
	GtkStringList *model = GTK_STRING_LIST(gtk_drop_down_get_model(GTK_DROP_DOWN(dd)));
	const char *ext = gtk_string_list_get_string(model, selected);
	enum Result res = convert_helper(path, ext, cp);
	if (res == SUCCESS) {
		printf("Nice!\n");
	} else {
		printf("res: %d\n", res);
	}
}

void clicked(GtkWidget *widget, gpointer data)
{
	fd = gtk_file_dialog_new();
	cancel = g_cancellable_new();

	gtk_file_dialog_open(fd, GTK_WINDOW(window), cancel, async_callback, NULL);
}

void dd_setup(GtkSignalListItemFactory* self, GObject* object, gpointer user_data) {
	GtkWidget *lb = gtk_label_new(NULL);
	GtkListItem *li = GTK_LIST_ITEM(object);
	if (!li) {
		printf("Eyvah\n");
		return;
	}
	gtk_list_item_set_child(li, lb);
}

void dd_bind(GtkSignalListItemFactory* self, GObject* object, gpointer user_data) {
	GtkListItem *li = GTK_LIST_ITEM(object);
	if (!li) {
		printf("Eyvah\n");
		return;
	}
	GtkWidget *lb = gtk_list_item_get_child(li);
	GtkStringObject *strobj = GTK_STRING_OBJECT(gtk_list_item_get_item(li));
	const char *str = gtk_string_object_get_string(strobj);
	gtk_label_set_text(GTK_LABEL(lb), str);
}

void dd_unbind(GtkSignalListItemFactory* self, GObject* object, gpointer user_data) {

}

void dd_teardown(GtkSignalListItemFactory* self, GObject* object, gpointer user_data) {

}

static void activate(GtkApplication *app, gpointer user_data)
{
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Window");
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	/*gtk_widget_set_margin_start(box, 30);
	gtk_widget_set_margin_top(box, 30);
	gtk_widget_set_margin_bottom(box, 30);
	gtk_widget_set_margin_end(box, 30);
	gtk_widget_set_hexpand(box, true);*/
	gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(box, GTK_ALIGN_CENTER);

	GtkWidget *b = gtk_button_new();
	gtk_widget_set_valign(b, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(b, GTK_ALIGN_CENTER);

	gtk_button_set_label(GTK_BUTTON(b), "Dosya seçin");
	gtk_box_append(GTK_BOX(box), b);
	g_signal_connect(b, "clicked", G_CALLBACK(clicked), NULL);

	dd = gtk_drop_down_new(NULL, NULL);
	GtkListItemFactory *gslif = gtk_signal_list_item_factory_new();
	g_signal_connect(gslif, "setup", G_CALLBACK(dd_setup), NULL);
	g_signal_connect(gslif, "bind", G_CALLBACK(dd_bind), NULL);
	g_signal_connect(gslif, "unbind", G_CALLBACK(dd_unbind), NULL);
	g_signal_connect(gslif, "teardown", G_CALLBACK(dd_teardown), NULL);
	gtk_widget_set_visible(dd, false);
	gtk_box_append(GTK_BOX(box), dd);

	convert_button = gtk_button_new();
	gtk_widget_set_visible(convert_button, false);
	gtk_button_set_label(GTK_BUTTON(convert_button), "Convert!");
	g_signal_connect(convert_button, "clicked", G_CALLBACK(convert_clicked), NULL);
	gtk_box_append(GTK_BOX(box), convert_button);

	gtk_window_set_child(GTK_WINDOW(window), box);
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
