#include "ffmpeg.h"

const char *ffmpeg_cmd_template = "ffmpeg -i \"%s\" \"%s\"";

enum Result ffmpeg_convert_single(const char* in_path, const char* out_path, struct FfmpegSettings settings)
{
    enum Result res = UNKNOWN_ERROR;

    char *cmd = (char*) malloc((strlen(ffmpeg_cmd_template)-4+strlen(in_path)+strlen(out_path)+1)*sizeof(char));
    if (!cmd)
        return NOT_ENOUGH_MEMORY;
    sprintf(cmd, ffmpeg_cmd_template, in_path, out_path);

    FILE *ffmpeg_pipe = popen(cmd, "r");
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
    GtkWidget *bx = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *lb = gtk_label_new("Kare sayısı:");
    GtkWidget *entry = gtk_spin_button_new_with_range(0, 10000, 5);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry), 60);
    gtk_box_append(GTK_BOX(bx), lb);
    gtk_box_append(GTK_BOX(bx), entry);
    gtk_box_append(GTK_BOX(box), bx);
}

struct FfmpegSettings ffmpeg_get_settings(GtkWidget* box) {
    GtkWidget* entry = gtk_widget_get_last_child(gtk_widget_get_first_child(box));
    return {
        .framerate = (size_t) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(entry)),
    };
}
