#include "ffmpeg.hpp"
#include "backend.hpp"

const char *ffmpeg_cmd_template = "ffmpeg -i \"%s\" \"%s\"";

std::string ffmpeg_parse_settings(struct FfmpegSettings settings) {
    std::string output;
    if (settings.framerate > 0)
        output += "-r " + std::to_string(settings.framerate);
    return output;
}

enum Result ffmpeg_convert_single(const char* in_path, const char* out_path, struct FfmpegSettings settings)
{
    enum Result res = UNKNOWN_ERROR;
    if (in_path == NULL || out_path == NULL)
        return res;

    std::string cmd = "ffmpeg -y -i \""+ (std::string)in_path +"\" "+ ffmpeg_parse_settings(settings) +" \""+ (std::string)out_path +"\"";
    FILE *ffmpeg_pipe = popen(cmd.c_str(), "r");
    if (!ffmpeg_pipe) {
        return UNKNOWN_ERROR;
    }

    if (pclose(ffmpeg_pipe) == 0)
        return SUCCESS;
    return res;
}

void ffmpeg_set_settings_widget(GtkWidget* grid) {
    gtk_grid_insert_column(GTK_GRID(grid), 0);
    gtk_grid_insert_row(GTK_GRID(grid), 0);
    GtkWidget *bx = label_spin(_("Framerate:"), 0, 10000, 5, 60);
    gtk_grid_attach(GTK_GRID(grid), bx, 0, 0, 1, 1);
}

struct FfmpegSettings ffmpeg_get_settings(GtkWidget* box) {
    GtkWidget* entry = gtk_widget_get_first_child(box);
    return {
        .framerate = (size_t) get_spin_value(entry),
    };
}
