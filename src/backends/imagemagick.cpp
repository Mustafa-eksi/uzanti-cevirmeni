#include "imagemagick.h"
#include "backend.hpp"
#include "gtk/gtkdropdown.h"

const char *im_file_not_found = "convert-im6.q16: unable to open image";

const char *magick_cmd_template = "convert \"%s\" \"%s\"";

std::string imagemagick_parse_settings(struct ImageMagickSettings settings) {
    std::string res = "";
    if (settings.grayscale)
        res += "-colorspace Gray ";
    if (settings.flip_horizontal)
        res += "-flip ";
    if (settings.flip_vertical)
        res += "-flop ";
    if (settings.rotation != 0)
        res += "-rotate "+std::to_string(settings.rotation)+" ";
    if (settings.resize != 100 && settings.resize > 0)
        res += "-resize "+std::to_string(settings.resize)+"% ";
    if (settings.compress > 0 && settings.compress < IM_COMPRESSION_LEN)
        res += "-compress "+ (std::string)IM_compression[settings.compress] +" ";
    if (settings.quality != 92)
        res += "-quality " +std::to_string(settings.quality)+" ";
    return res;
}

enum Result imagemagick_convert_single(const char* in_path, const char* out_path, struct ImageMagickSettings settings)
{
    enum Result res = UNKNOWN_ERROR;
    if (in_path == NULL || out_path == NULL)
        return res;

    std::string cmd = "convert \""+ (std::string)in_path +"\" "+ imagemagick_parse_settings(settings) +" \""+ (std::string)out_path +"\"";
    printf("cmd: %s\n", cmd.c_str());
    FILE *im_pipe = popen(cmd.c_str(), "r");
    if (!im_pipe) {
        return UNKNOWN_ERROR;
    }

    char output[BIG_BUFF] = {};
    fgets(output, sizeof(output), im_pipe);
    if (strncmp(output, im_file_not_found, strlen(im_file_not_found)) == 0)
        res = FILE_NOT_FOUND;
    else
        res = SUCCESS;
//out:
    pclose(im_pipe);
    return res;
}

void imagemagick_set_settings_widget(GtkWidget* grid) {
    gtk_grid_insert_column(GTK_GRID(grid), 0);
    gtk_grid_insert_column(GTK_GRID(grid), 1);
    gtk_grid_insert_row(GTK_GRID(grid), 0);
    gtk_grid_insert_row(GTK_GRID(grid), 1);
    gtk_grid_insert_row(GTK_GRID(grid), 2);
    gtk_grid_insert_row(GTK_GRID(grid), 3);

    GtkWidget *grayw = gtk_check_button_new_with_label(_("Grayscale"));
    gtk_grid_attach(GTK_GRID(grid), grayw, 0, 0, 1, 1);

    GtkWidget *fliphw = gtk_check_button_new_with_label(_("Flip horizontally"));
    gtk_grid_attach(GTK_GRID(grid), fliphw, 0, 1, 1, 1);

    GtkWidget *flipvh = gtk_check_button_new_with_label(_("Flip vertically"));
    gtk_grid_attach(GTK_GRID(grid), flipvh, 0, 2, 1, 1);

    GtkWidget *compressionw = label_drop(_("Compression"), IM_compression);
    gtk_grid_attach(GTK_GRID(grid), compressionw, 0, 3, 1, 1);

    GtkWidget *rotbox = label_spin(_("Rotation:"), -360, 360, 45, 0);
    gtk_grid_attach(GTK_GRID(grid), rotbox, 1, 0, 1, 1);
    GtkWidget *resizew = label_spin(_("Resize (%):"), 0, 1000, 25, 100);
    gtk_grid_attach(GTK_GRID(grid), resizew, 1, 1, 1, 1);
    GtkWidget *qualityw = label_spin(_("Quality:"), 0, 100, 5, 92);
    gtk_grid_attach(GTK_GRID(grid), qualityw, 1, 3, 1, 1);

}

struct ImageMagickSettings imagemagick_get_settings(GtkWidget* box) {
    GtkWidget *grayw = gtk_widget_get_first_child(box);
    GtkWidget *fliphw = gtk_widget_get_next_sibling(grayw);
    GtkWidget *flipvw = gtk_widget_get_next_sibling(fliphw);
    GtkWidget *compressionw = gtk_widget_get_next_sibling(flipvw);
    GtkWidget *rotbox = gtk_widget_get_next_sibling(compressionw);
    GtkWidget *resizew = gtk_widget_get_next_sibling(rotbox);
    GtkWidget *qualityw = gtk_widget_get_next_sibling(resizew);

    return (struct ImageMagickSettings) {
        .compress = get_drop_value(compressionw),
        .grayscale = (bool) gtk_check_button_get_active(GTK_CHECK_BUTTON(grayw)),
        .flip_horizontal = (bool) gtk_check_button_get_active(GTK_CHECK_BUTTON(fliphw)),
        .flip_vertical = (bool) gtk_check_button_get_active(GTK_CHECK_BUTTON(flipvw)),
        .rotation = get_spin_value(rotbox),
        .resize = get_spin_value(resizew),
        .quality = get_spin_value(qualityw),
    };
}
