#include "libreoffice.h"
#include "backend.hpp"

const char *libreoffice_cmd_template = "libreoffice --convert-to \"%s\" --outdir \"%s\" \"%s\"";

enum Result libreoffice_convert_single(const char* in_path, const char* output_folder, const char* extension, struct LibreofficeSettings settings) {
    enum Result res = UNKNOWN_ERROR;
    char outb[BIG_BUFF] = {};

    char *cmd = (char*) malloc((strlen(libreoffice_cmd_template)-4+strlen(in_path)+strlen(extension)+1)*sizeof(char));
    if (!cmd)
        return NOT_ENOUGH_MEMORY;
    sprintf(cmd, libreoffice_cmd_template, extension, output_folder, in_path);

    printf("using libreoffice: '%s'\n", cmd);
    FILE *libreoffice_pipe = popen(cmd, "r");
    if (!libreoffice_pipe)
        goto out;

    //char output[BIG_BUFF] = {};

    // TODO: Read ffmpeg output for logging and error detection
    // fgets(output, sizeof(output), ffmpeg_pipe);

    // FIXME: there can be cases of ffmpeg returning 0 even if an error occurred.
    if (pclose(libreoffice_pipe) == 0)
        res = SUCCESS;
    // change_extension(in_path, extension, outb);
    if (!outb[0] || access(outb, F_OK) != 0)
        res = UNKNOWN_ERROR;
out:
    return res;
}
