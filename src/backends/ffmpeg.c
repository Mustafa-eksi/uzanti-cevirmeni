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
