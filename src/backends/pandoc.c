#include "pandoc.h"

const char *pandoc_cmd_template = "pandoc \"%s\" -o \"%s\"";

void pandoc_parse_settings(char* buffer, size_t buffer_size, struct PandocSettings settings) {
    //printf("\"%s\"\n", buffer);
    size_t last_line = 0;
    if (settings.hard_line_breaks) {
        last_line = sprintf(buffer, " --wrap=preserve ");
    }
    (void) last_line;
    //printf("\"%s\"\n", buffer);
}

enum Result pandoc_convert_single(const char* in_path, const char* out_path, struct PandocSettings settings) {
    enum Result res = UNKNOWN_ERROR;
    char cmd[BIG_BUFF] = {};
    /* char *cmd = (char*) malloc((strlen(pandoc_cmd_template)-4+strlen(in_path)+strlen(out_path)+1)*sizeof(char));
     *    if (!cmd)
     *        return NOT_ENOUGH_MEMORY;*/
    size_t lines = sprintf(cmd, pandoc_cmd_template, in_path, out_path);
    pandoc_parse_settings(cmd+lines, BIG_BUFF-lines, settings);
    FILE *pandoc_pipe = popen(cmd, "r");
    if (!pandoc_pipe) {
        return UNKNOWN_ERROR;
    }

    char output[BIG_BUFF] = {};

    // TODO: Read pandoc output for logging and error detection
    fgets(output, sizeof(output), pandoc_pipe);

    // FIXME: there can be cases of pandoc returning 0 even if an error occurred.
    if (pclose(pandoc_pipe) == 0)
        return SUCCESS;
    printf("Pandoc error: %s\n", output);
    return res;
}
