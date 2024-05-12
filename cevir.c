#include "cevir.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SMOL_BUFF 256
#define BIG_BUFF 1024

const char *which_error = "which: no %s", *which_cmd = "which %s";

enum Result test_cli(const char* app)
{
    enum Result res = EXECUTABLE_NOT_FOUND;
    char cmd[SMOL_BUFF] = {};
    sprintf(cmd, which_cmd, app);

    FILE *which_pipe = popen(cmd, "r");
    if (!which_pipe)
        return UNKNOWN_ERROR;

    char buff[SMOL_BUFF] = {};
    char unwanted_output[SMOL_BUFF] = {};
    sprintf(unwanted_output, which_error, app);

    if (fgets(buff, sizeof(buff), which_pipe) == NULL)
        goto out;

    if (buff[0] == '/')
        res = SUCCESS;
    else if (strncmp(buff, unwanted_output, strlen(unwanted_output)) != 0)
        res = UNKNOWN_ERROR;
out:
    pclose(which_pipe);
    return res;
}

enum Result convert_helper(const char *file, const char *extension, enum ConverterProgram cp)
{

    enum Result r = test_cli(EXECUTABLE_NAMES[cp]);
    if (r != SUCCESS)
        return r;

    size_t dot_pos = 0;
    for (size_t i = 0; i < strlen(file); i++)
        if (file[i] == '.')
            dot_pos = i;
    char *output_path = (char*) malloc((dot_pos+2+strlen(extension)+1)*sizeof(char));
    strncpy(output_path, file, dot_pos+1);
    strcpy(output_path+dot_pos+1, extension);

    if (cp == IMAGEMAGICK)
        return imagemagick_convert_single(file, output_path, (struct ImageMagickSettings){0});
    else if (cp == FFMPEG)
        return ffmpeg_convert_single(file, output_path, (struct FfmpegSettings){0});
    else if (cp == PANDOC)
        return pandoc_convert_single(file, output_path, (struct PandocSettings){0});
    return UNKNOWN_ERROR;
}

int get_converter_from_extension(const char *extension)
{
    int res = UNSUPPORTED;
    for (size_t i = 0; i < IMC_COUNT - 1; i++)
        if (strcmp(extension, IMAGEMAGICK_CONVERTABLE[i]) == 0)
            res |= IMAGEMAGICK;
    for (size_t i = 0; i < FFMPEG_COUNT - 1; i++)
        if (strcmp(extension, FFMPEG_CONVERTABLE[i]) == 0)
            res |= FFMPEG;
    for (size_t i = 0; i < PANDOC_INPUT_COUNT - 1; i++)
        if (strcmp(extension, PANDOC_INPUT[i]) == 0)
            res |= PANDOC;
    return res;
}

char *get_extension(const char* file)
{
    size_t dot_pos = 0;
    for (size_t i = 0; i < strlen(file); i++)
        if (file[i] == '.')
            dot_pos = i;
    char *extension = (char*) malloc((strlen(file)-dot_pos)*sizeof(char));
    strcpy(extension, file+dot_pos+1);
    return extension;
}

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
out:
    pclose(im_pipe);
    return res;
}

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

    char output[BIG_BUFF] = {};

    // TODO: Read ffmpeg output for logging and error detection
    // fgets(output, sizeof(output), ffmpeg_pipe);

    // FIXME: there can be cases of ffmpeg returning 0 even if an error occurred.
    if (pclose(ffmpeg_pipe) == 0)
        return SUCCESS;
    return res;
}

const char *pandoc_cmd_template = "pandoc \"%s\" -o \"%s\"";

enum Result pandoc_convert_single(const char* in_path, const char* out_path, struct PandocSettings settings) {
    enum Result res = UNKNOWN_ERROR;

    char *cmd = (char*) malloc((strlen(pandoc_cmd_template)-4+strlen(in_path)+strlen(out_path)+1)*sizeof(char));
    if (!cmd)
        return NOT_ENOUGH_MEMORY;
    sprintf(cmd, pandoc_cmd_template, in_path, out_path);

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
