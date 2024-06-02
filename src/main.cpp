#include <gtk/gtk.h>
#include <gtk/gtkorientable.h>
#include <gio/gcancellable.h>
#include <gio/gasyncresult.h>
#include <string.h>
#include "backends/backend.hpp"
#include "backends/ffmpeg.h"
#include "cevir.cpp"

GtkWidget *window;
GtkFileDialog *fd;
GCancellable *cancel;
GtkWidget *dd;
GtkWidget *convert_button;
std::vector<char*> paths;
enum ConverterProgram cp;
GtkWidget *sf_button, *filepath_label;
GtkAlertDialog* dialg;
GtkWidget *filepath_view;

void alert_clicked(GObject *source_object, GAsyncResult *res, gpointer data)
{
	GError *err;
	int click = gtk_alert_dialog_choose_finish(dialg, res, &err);
	printf("Alert clicked! %d\n", click);
}

void show_alert(const char* text, const char* detail) {
	dialg = gtk_alert_dialog_new(text, detail);
	gtk_alert_dialog_set_buttons(dialg, (const char* const[]) {"Tamam", NULL});
	cancel = g_cancellable_new();
	gtk_alert_dialog_choose(dialg, GTK_WINDOW(window), cancel, alert_clicked, NULL);
}

void async_callback(GObject *source_object, GAsyncResult *res, gpointer data)
{
	char* buff[BIG_BUFF] = {};
	GtkStringList *strlist;
	GError *err;

	auto files = gtk_file_dialog_open_multiple_finish(fd, res, &err);
	auto count = g_list_model_get_n_items(files);

	paths.clear();
	cp = UNSUPPORTED;
	for (size_t i = 0; i < count; i++) {
		auto gfile = (GFile*) g_list_model_get_item(files, i);
		if (!gfile)
			continue;
		auto filepath = g_file_get_path(gfile);
		//printf("file %ld: %s\n", i, filepath);
		paths.push_back(filepath);

		char *ext = get_extension(filepath);
		enum ConverterProgram tmp = (enum ConverterProgram) get_converter_from_extension(ext);
		check_available(&tmp);
		if (cp == UNSUPPORTED)
			cp = tmp;
		else if (cp != tmp)
			continue;
	}
	paths.push_back(NULL);
	if ((cp & IMAGEMAGICK) != 0) {
		create_dropdown_list(imagemagick_all, buff);
	} else if ((cp & FFMPEG) != 0) {
		create_dropdown_list(ffmpeg_all, buff);
	} /*else if((cp & LIBREOFFICE) != 0) { // Disabled due to instability
		create_dropdown_list(libreoffice_all, buff);
	}*/ else if ((cp & PANDOC) != 0) {
		create_dropdown_list(pandoc_all, buff);
	} else {
		show_alert("Seçilmiş hiçbir dosya için çevirmen program bulunamadı!", NULL);
		return;
	}

	strlist = gtk_string_list_new(buff);
	gtk_drop_down_set_model(GTK_DROP_DOWN(dd), G_LIST_MODEL(strlist));
	gtk_widget_set_visible(dd, true);
	gtk_widget_set_visible(convert_button, true);
	gtk_button_set_label(GTK_BUTTON(sf_button), "Dosyayı değiştirin");
	gtk_widget_set_visible(filepath_label, true);

	auto filepath_strlist = gtk_string_list_new(&paths[0]);
	auto filepath_model = gtk_no_selection_new(G_LIST_MODEL(filepath_strlist));
	gtk_list_view_set_model(GTK_LIST_VIEW(filepath_view), GTK_SELECTION_MODEL(filepath_model));
	//free(ext);
}

const char* result_to_str(enum Result r) {
	switch(r) {
		case SUCCESS: return "Başarılı";
		case UNKNOWN_ERROR: return "Bilinmeyen hata";
		case EXECUTABLE_NOT_FOUND: return "Çalıştırılabilir dosya bulunamadı";
		case NOT_ENOUGH_MEMORY: return "Yetersiz hafıza";
		case FILE_NOT_FOUND: return "Dosya bulunamadı";
		default: return "Bilinmeyen hata";
	}
}

void convert_clicked(GtkWidget *widget, gpointer data)
{
	guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(dd));
	GtkStringList *model = GTK_STRING_LIST(gtk_drop_down_get_model(GTK_DROP_DOWN(dd)));
	const char *ext = gtk_string_list_get_string(model, selected);
	enum Result res = SUCCESS;
	// TODO: Make this multithreaded.
	for (const auto& path : paths) {
		if (!path)
			continue;
		auto r = convert_helper(path, ext, cp);
		if (res == SUCCESS)
			res = r;
		printf("converted: %s\n", path);
	}
	if (res == SUCCESS) {
		show_alert("Bütün dosyalar başarıyla çevrildi!", NULL);
	} else {
		show_alert("Bazı dosyaların çevrimi başarısız oldu!: %s", result_to_str(res));
	}
}

void clicked(GtkWidget *widget, gpointer data)
{
	fd = gtk_file_dialog_new();
	cancel = g_cancellable_new();
	GListStore *gls = g_list_store_new(gtk_file_filter_get_type());

	GtkFileFilter *filter_all = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter_all, "*");
	gtk_file_filter_set_name(filter_all, "All files");
	g_list_store_append(gls, filter_all);

	/* These are just mime types from https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
	 * I tried using suffixes and patterns but they don't work with KDE's file picker.
	 */

	GtkFileFilter *filter = gtk_file_filter_new();
	for (const auto& [k, v] : IMAGEMAGICK_CONVERSIONS) {
		if (FORMAT_MIME_TYPES.contains(k))
			gtk_file_filter_add_mime_type(filter, FORMAT_MIME_TYPES.at(k).c_str());
	}
	gtk_file_filter_set_name(filter, "Image formats");
	g_list_store_append(gls, filter);

	GtkFileFilter *filter_video = gtk_file_filter_new();
	for (const auto& [k, v] : FFMPEG_CONVERSIONS) {
		if (FORMAT_MIME_TYPES.contains(k))
			gtk_file_filter_add_mime_type(filter_video, FORMAT_MIME_TYPES.at(k).c_str());
	}
	gtk_file_filter_set_name(filter_video, "Audio/Video formats");
	g_list_store_append(gls, filter_video);

	GtkFileFilter *filter_doc = gtk_file_filter_new();
	for (const auto& [k, v] : PANDOC_CONVERSIONS) {
		if (FORMAT_MIME_TYPES.contains(k))
			gtk_file_filter_add_mime_type(filter_doc, FORMAT_MIME_TYPES.at(k).c_str());
	}
	gtk_file_filter_set_name(filter_doc, "Document formats");
	g_list_store_append(gls, filter_doc);

	gtk_file_dialog_set_filters(fd, G_LIST_MODEL(gls));
	gtk_file_dialog_open_multiple(fd, GTK_WINDOW(window), cancel, async_callback, NULL);
}

void dd_setup(GtkSignalListItemFactory* self, GObject* object, gpointer user_data) {
	GtkWidget *lb = gtk_label_new(NULL);
	GtkListItem *li = GTK_LIST_ITEM(object);
	if (!li) {
		show_alert("Bilinmeyen bir hata oluştu!", NULL);
		return;
	}
	gtk_list_item_set_child(li, lb);
}

void dd_bind(GtkSignalListItemFactory* self, GObject* object, gpointer user_data) {
	GtkListItem *li = GTK_LIST_ITEM(object);
	if (!li) {
		show_alert("Bilinmeyen bir hata oluştu!", NULL);
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
	gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(box, GTK_ALIGN_CENTER);

	/*auto filepath_strlist = gtk_string_list_new(&paths[0]);
	auto filepath_model = gtk_single_selection_new(G_LIST_MODEL(filepath_strlist));*/
	auto filepath_factory = gtk_signal_list_item_factory_new();
	g_signal_connect(filepath_factory, "setup", G_CALLBACK(dd_setup), NULL);
	g_signal_connect(filepath_factory, "bind", G_CALLBACK(dd_bind), NULL);
	filepath_view = gtk_list_view_new(NULL, filepath_factory);
	gtk_box_append(GTK_BOX(box), filepath_view);

	sf_button = gtk_button_new();
	gtk_widget_set_valign(sf_button, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(sf_button, GTK_ALIGN_CENTER);

	gtk_button_set_label(GTK_BUTTON(sf_button), "Dosya seçin");
	g_signal_connect(sf_button, "clicked", G_CALLBACK(clicked), NULL);
	gtk_box_append(GTK_BOX(box), sf_button);

	GtkWidget *box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_widget_set_margin_top(box2, 20);
	gtk_widget_set_halign(box2, GTK_ALIGN_CENTER);
	dd = gtk_drop_down_new(NULL, NULL);
	GtkListItemFactory *gslif = gtk_signal_list_item_factory_new();
	g_signal_connect(gslif, "setup", G_CALLBACK(dd_setup), NULL);
	g_signal_connect(gslif, "bind", G_CALLBACK(dd_bind), NULL);
	g_signal_connect(gslif, "unbind", G_CALLBACK(dd_unbind), NULL);
	g_signal_connect(gslif, "teardown", G_CALLBACK(dd_teardown), NULL);
	gtk_widget_set_visible(dd, false);
	gtk_box_append(GTK_BOX(box2), dd);

	convert_button = gtk_button_new();
	gtk_widget_set_visible(convert_button, false);
	gtk_button_set_label(GTK_BUTTON(convert_button), "Çevir!");
	g_signal_connect(convert_button, "clicked", G_CALLBACK(convert_clicked), NULL);
	gtk_box_append(GTK_BOX(box2), convert_button);

	gtk_box_append(GTK_BOX(box), box2);
	gtk_window_set_child(GTK_WINDOW(window), box);
	gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
	GtkApplication *app;
	int status;

	app = gtk_application_new("io.meksi.uzanti_cevirici", G_APPLICATION_HANDLES_OPEN);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	//status = g_application_run_with_args(G_APPLICATION(app), argc, argv);
	status = g_application_run(G_APPLICATION(app), 1, argv);

	g_object_unref(app);
	return status;
}
