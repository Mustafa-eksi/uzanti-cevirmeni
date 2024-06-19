#include <gtk/gtk.h>
#include <gtk/gtkorientable.h>
#include <gio/gcancellable.h>
#include <gio/gasyncresult.h>
#include <string.h>
#include <stdio.h>
#include "backends/backend.hpp"
#include "backends/ffmpeg.h"
#include "backends/pandoc.h"
#include "cevir.cpp"

GtkWidget *window;
GtkWidget *fd;
GCancellable *cancel;
GtkWidget *dd;
GtkWidget *convert_button;
std::vector<char*> paths;
enum ConverterProgram cp;
GtkWidget *sf_button, *filepath_label;
GtkWidget* dialg;
GtkWidget *filepath_view;
GtkWidget *box2;
GtkWidget *settingsw;
GtkWidget* expander;

char* output_folder_path;
GtkWidget *output_fd;

#define DOSYA_SEC "Dosya seçin"
#define DOSYA_DEGISTIR "Seçilen dosyaları değiştirin"

void alert_clicked(GtkDialog* self, gint response_id, gpointer user_data)
{
	if (response_id == -4) return; // close response
	gtk_window_close(GTK_WINDOW(self));
}

void show_alert(const char* text, const char* detail) {
	dialg = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, text, detail);
	g_signal_connect(dialg, "response", G_CALLBACK(alert_clicked), NULL);
	gtk_window_present(GTK_WINDOW(dialg));
}

void sync_paths() {
	auto filepath_strlist = gtk_string_list_new(&paths[0]);
	auto filepath_model = gtk_no_selection_new(G_LIST_MODEL(filepath_strlist));
	gtk_list_view_set_model(GTK_LIST_VIEW(filepath_view), GTK_SELECTION_MODEL(filepath_model));
}

void no_files() {
	gtk_widget_set_visible(filepath_view, false);
	//gtk_widget_set_visible(dd, false);
	gtk_widget_set_visible(convert_button, false);
	gtk_button_set_label(GTK_BUTTON(sf_button), DOSYA_SEC);
	gtk_widget_set_visible(box2, false);
}

void async_callback(GtkDialog* self, gint response_id, gpointer user_data)
{
	if (response_id == -4) return;
	char* buff[BIG_BUFF] = {};
	GtkStringList *strlist;

	// auto files = gtk_file_dialog_open_multiple_finish(fd, res, &err);
	auto files = gtk_file_chooser_get_files(GTK_FILE_CHOOSER(fd));
	auto count = g_list_model_get_n_items(files);
	gtk_window_close(GTK_WINDOW(self));

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
	GtkWidget *first_c = gtk_widget_get_first_child(settingsw);
	while (first_c != NULL) {
		gtk_box_remove(GTK_BOX(settingsw), first_c);
		first_c = gtk_widget_get_first_child(settingsw);
	}
	if ((cp & IMAGEMAGICK) != 0) {
		create_dropdown_list(imagemagick_all, buff);
		imagemagick_set_settings_widget(settingsw);
	} else if ((cp & FFMPEG) != 0) {
		create_dropdown_list(ffmpeg_all, buff);
		ffmpeg_set_settings_widget(settingsw);
	} /*else if((cp & LIBREOFFICE) != 0) { // Disabled due to instability
		create_dropdown_list(libreoffice_all, buff);
	}*/ else if ((cp & PANDOC) != 0) {
		create_dropdown_list(pandoc_all, buff);
		pandoc_set_settings_widget(settingsw);
	} else {
		show_alert("Seçilmiş hiçbir dosya için çevirmen program bulunamadı!", NULL);
		no_files();
		return;
	}

	strlist = gtk_string_list_new(buff);
	gtk_drop_down_set_model(GTK_DROP_DOWN(dd), G_LIST_MODEL(strlist));
	gtk_widget_set_visible(box2, true);
	gtk_widget_set_visible(convert_button, true);
	gtk_button_set_label(GTK_BUTTON(sf_button), DOSYA_DEGISTIR);
	gtk_widget_set_visible(filepath_view, true);
	sync_paths();

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
		auto r = convert_helper(path, ext, output_folder_path, cp, settingsw);
		if (res == SUCCESS)
			res = r;
		//printf("converted: %s\n", path);
	}
	if (res == SUCCESS) {
		show_alert("Bütün dosyalar başarıyla çevrildi!", NULL);
	} else {
		show_alert("Bazı dosyaların çevrimi başarısız oldu!: %s", result_to_str(res));
	}
}

void clicked(GtkWidget *widget, gpointer data)
{
	fd = gtk_file_chooser_dialog_new(_(""), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, _("Open"), "", (char*)NULL);
	// GListStore *gls = g_list_store_new(gtk_file_filter_get_type());

	char filterbuff[BIG_BUFF*8] = {0};
	size_t last_write = 0;
	GtkFileFilter *filter = gtk_file_filter_new();
	for (const auto& [k, v] : IMAGEMAGICK_CONVERSIONS) {
		if (FORMAT_EXTENSIONS.contains(k)) {
			gtk_file_filter_add_suffix(filter, FORMAT_EXTENSIONS.at(k).c_str());
		}
			//last_write += sprintf(filterbuff+last_write, "*.%s ", FORMAT_EXTENSIONS.at(k).c_str());
	}
	//gtk_file_filter_add_pattern(filter, filterbuff);
	gtk_file_filter_set_name(filter, "Image formats");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fd), filter);
	// g_list_store_append(gls, filter);

	last_write = 0;
	memset(filterbuff, 0, last_write);
	GtkFileFilter *filter_video = gtk_file_filter_new();
	for (const auto& [k, v] : FFMPEG_CONVERSIONS) {
		if (FORMAT_EXTENSIONS.contains(k))
			gtk_file_filter_add_suffix(filter_video, FORMAT_EXTENSIONS.at(k).c_str());
			//last_write += sprintf(filterbuff+last_write, "*.%s ", FORMAT_EXTENSIONS.at(k).c_str());
	}
	//gtk_file_filter_add_pattern(filter_video, filterbuff);
	gtk_file_filter_set_name(filter_video, "Audio/Video formats");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fd), filter_video);
	//g_list_store_append(gls, filter_video);

	last_write = 0;
	memset(filterbuff, 0, last_write);
	GtkFileFilter *filter_doc = gtk_file_filter_new();
	for (const auto& [k, v] : PANDOC_CONVERSIONS) {
		if (FORMAT_EXTENSIONS.contains(k))
			gtk_file_filter_add_suffix(filter_doc, FORMAT_EXTENSIONS.at(k).c_str());
			//last_write += sprintf(filterbuff+last_write, "*.%s ", FORMAT_EXTENSIONS.at(k).c_str());
	}
	//gtk_file_filter_add_pattern(filter_doc, filterbuff);
	gtk_file_filter_set_name(filter_doc, "Document formats");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fd), filter_doc);
	// g_list_store_append(gls, filter_doc);

	// gtk_file_dialog_set_filters(fd, G_LIST_MODEL(gls));
	g_signal_connect(fd, "response", G_CALLBACK(async_callback), NULL);
	gtk_window_present(GTK_WINDOW(fd));
	// gtk_file_dialog_open_multiple(fd, GTK_WINDOW(window), cancel, async_callback, NULL);
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

void filelist_setup(GtkSignalListItemFactory* self, GObject* object, gpointer user_data) {
	GtkWidget *bx = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	GtkWidget *lb = gtk_label_new(NULL);
	GtkWidget *bt = gtk_button_new();
	gtk_widget_set_halign(bt, GTK_ALIGN_END);
	gtk_widget_set_hexpand(bt, GTK_ALIGN_END);
	gtk_button_set_icon_name(GTK_BUTTON(bt), "delete");
	GtkListItem *li = GTK_LIST_ITEM(object);
	if (!li) {
		show_alert("Bilinmeyen bir hata oluştu!", NULL);
		return;
	}
	gtk_box_append(GTK_BOX(bx), lb);
	gtk_box_append(GTK_BOX(bx), bt);
	gtk_list_item_set_child(li, bx);
}

void remove_file(GtkWidget *widget, gpointer data) {
	GtkListItem *li = GTK_LIST_ITEM(data);
	guint p = gtk_list_item_get_position(li);
	paths.erase(paths.begin() + p);
	if (paths.size() <= 1) // there's NULL too
		no_files();
	else
		sync_paths();
}

void filelist_bind(GtkSignalListItemFactory* self, GObject* object, gpointer user_data) {
	GtkListItem *li = GTK_LIST_ITEM(object);
	if (!li) {
		show_alert("Bilinmeyen bir hata oluştu!", NULL);
		return;
	}
	GtkWidget *bx = gtk_list_item_get_child(li);
	GtkWidget *lb = gtk_widget_get_first_child(bx);
	GtkWidget *bt = gtk_widget_get_last_child(bx);
	g_signal_connect(bt, "clicked", G_CALLBACK(remove_file), li);

	GtkStringObject *strobj = GTK_STRING_OBJECT(gtk_list_item_get_item(li));
	const char *str = gtk_string_object_get_string(strobj);
	gtk_label_set_text(GTK_LABEL(lb), str);
}

void output_folder_async(GtkDialog* self, gint response_id, gpointer user_data) {
	if (response_id == -4) return; // close
	auto folder = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(output_fd));
	if (folder) {
		output_folder_path = g_file_get_path(folder);
		printf("path: %s\n", output_folder_path);
		gtk_window_close(GTK_WINDOW(self));
	}
}

void output_folder_clicked(GtkWidget *widget, gpointer data) {
	output_fd = gtk_file_chooser_dialog_new(_(""), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, _("Select"), "", (char*)NULL);
	g_signal_connect(output_fd, "response", G_CALLBACK(output_folder_async), NULL);
	gtk_window_present(GTK_WINDOW(output_fd));
	// output_fd = gtk_file_dialog_new();
	// gtk_file_dialog_select_folder(output_fd, GTK_WINDOW(window), cancel, output_folder_async, NULL);
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
	g_signal_connect(filepath_factory, "setup", G_CALLBACK(filelist_setup), NULL);
	g_signal_connect(filepath_factory, "bind", G_CALLBACK(filelist_bind), NULL);
	filepath_view = gtk_list_view_new(NULL, filepath_factory);
	gtk_box_append(GTK_BOX(box), filepath_view);

	sf_button = gtk_button_new();
	gtk_widget_set_valign(sf_button, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(sf_button, GTK_ALIGN_CENTER);

	gtk_button_set_label(GTK_BUTTON(sf_button), DOSYA_SEC);
	g_signal_connect(sf_button, "clicked", G_CALLBACK(clicked), NULL);
	gtk_box_append(GTK_BOX(box), sf_button);

	box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_widget_set_margin_top(box2, 20);
	gtk_widget_set_halign(box2, GTK_ALIGN_CENTER);
	gtk_widget_set_visible(box2, false);
	GtkWidget *box3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_widget_set_halign(box3, GTK_ALIGN_CENTER);
	dd = gtk_drop_down_new(NULL, NULL);
	GtkListItemFactory *gslif = gtk_signal_list_item_factory_new();
	g_signal_connect(gslif, "setup", G_CALLBACK(dd_setup), NULL);
	g_signal_connect(gslif, "bind", G_CALLBACK(dd_bind), NULL);
	GtkWidget *ddlb = gtk_label_new("Çıktı uzantısı:");
	gtk_box_append(GTK_BOX(box3), ddlb);
	gtk_box_append(GTK_BOX(box3), dd);
	gtk_box_append(GTK_BOX(box2), box3);

	expander = gtk_expander_new("Daha fazla ayar");
	settingsw = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_expander_set_child(GTK_EXPANDER(expander), settingsw);
	gtk_box_append(GTK_BOX(box2), expander);

	GtkWidget *output_folder_button = gtk_button_new();
	gtk_button_set_label(GTK_BUTTON(output_folder_button), "Çıktı klasörü seçin");
	g_signal_connect(output_folder_button, "clicked", G_CALLBACK(output_folder_clicked), NULL);
	gtk_box_append(GTK_BOX(box2), output_folder_button);

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
