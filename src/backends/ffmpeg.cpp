#include "ffmpeg.h"

const char *ffmpeg_cmd_template = "ffmpeg -i \"%s\" \"%s\"";

enum Result ffmpeg_convert_single(const char* in_path, const char* out_path, struct FfmpegSettings settings)
{
    enum Result res = UNKNOWN_ERROR;
    if (in_path == NULL || out_path == NULL)
        return res;

    std::string cmd = "ffmpeg -i \""+ (std::string)in_path +"\" \""+ (std::string)out_path +"\"";

    FILE *ffmpeg_pipe = popen(cmd.c_str(), "r");
    if (!ffmpeg_pipe) {
        return UNKNOWN_ERROR;
    }
    //char output[BIG_BUFF] = {};
    // TODO: Read ffmpeg output for logging and error detection
    // fgets(output, sizeof(output), ffmpeg_pipe);

    // FIXME: there can be cases of ffmpeg returning 0 even if an error occurred.
    if (pclose(ffmpeg_pipe) == 0)
        return SUCCESS;
    return res;
}

void ffmpeg_set_settings_widget(GtkWidget* box) {
    GtkWidget *bx = label_spin(_("Framerate:"), 0, 10000, 5, 60);
    gtk_box_append(GTK_BOX(box), bx);
}

struct FfmpegSettings ffmpeg_get_settings(GtkWidget* box) {
    GtkWidget* entry = gtk_widget_get_last_child(gtk_widget_get_first_child(box));
    return {
        .framerate = (size_t) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(entry)),
    };
}
