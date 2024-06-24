#include "libreoffice.hpp"
#include "backend.hpp"

const char *libreoffice_cmd_template = "libreoffice --convert-to \"%s\" --outdir \"%s\" \"%s\"";

std::string escape_space(std::string a) {
    std::string out;
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] == ' ')
            out += '\\';
        out += a[i];
    }
    return out;
}

enum Result libreoffice_convert_single(std::string in_path, std::string output_folder, std::string extension, struct LibreofficeSettings settings) {
    char outb[BIG_BUFF] = {};
    enum Result res = UNKNOWN_ERROR;

    std::string cmd = "libreoffice --convert-to "+extension+" --outdir "+escape_space(output_folder)+" "+escape_space(in_path)+"";

    FILE *libreoffice_pipe = popen(cmd.c_str(), "r");
    if (!libreoffice_pipe)
        return res;

    // TODO: Read ffmpeg output for logging and error detection
    // fgets(output, sizeof(output), ffmpeg_pipe);

    // FIXME: there can be cases of ffmpeg returning 0 even if an error occurred.
    if (pclose(libreoffice_pipe) == 0)
        res = SUCCESS;

    // change_extension(in_path, extension, outb);
    /*if (!outb[0] || access(outb, F_OK) != 0)
        res = UNKNOWN_ERROR;*/

    return res;
}

void libreoffice_set_settings_widget(GtkWidget* box) {
}

struct LibreofficeSettings libreoffice_get_settings(GtkWidget* box) {
    return (struct LibreofficeSettings) {};
}

