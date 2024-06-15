#include "imagemagick.h"

const char *im_file_not_found = "magick: unable to open image";

const char *magick_cmd_template = "magick \"%s\" \"%s\"";

enum Result imagemagick_convert_single(const char* in_path, const char* out_path, struct ImageMagickSettings settings)
{
    enum Result res = UNKNOWN_ERROR;

    char *cmd = (char*) malloc((strlen(magick_cmd_template)-4+strlen(in_path)+strlen(out_path)+1)*sizeof(char));
    if (!cmd)
        return NOT_ENOUGH_MEMORY;
    sprintf(cmd, magick_cmd_template, in_path, out_path);

    FILE *im_pipe = popen(cmd, "r");
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

void imagemagick_set_settings_widget(GtkWidget* box) {

}

struct ImageMagickSettings imagemagick_get_settings(GtkWidget* box) {
    return (struct ImageMagickSettings) {};
}
