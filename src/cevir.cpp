#include "cevir.h"
#include "backends/backend.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

enum Result check_available(enum ConverterProgram *inout) {
    for (const auto& [k, v] : EXECUTABLE_NAMES) {
        if ((k & *inout) == 0) continue;
        if (test_cli(v) == SUCCESS) {
            *inout = k;
            return SUCCESS;
        }
    }
    return EXECUTABLE_NOT_FOUND;
}

enum Result convert_helper(const char *file, const char *extension, enum ConverterProgram cp)
{
    enum Result r = EXECUTABLE_NOT_FOUND;
    enum ConverterProgram use_this = cp;
    check_available(&use_this);

    size_t dot_pos = 0;
    for (size_t i = 0; i < strlen(file); i++)
        if (file[i] == '.')
            dot_pos = i;
    char *output_path = (char*) malloc((dot_pos+2+strlen(extension)+1)*sizeof(char));
    strncpy(output_path, file, dot_pos+1);
    strcpy(output_path+dot_pos+1, extension);
    /*size_t slash_pos = 0;
    for (size_t i = 0; i < strlen(file); i++)
        if (file[i] == '/' && i != 0 && file[i-1] != '\\')
            slash_pos = i;
    char* output_folder = (char*) malloc(strlen(file)*sizeof(char)+1);
    memset(output_folder, 0, strlen(file)*sizeof(char)+1);
    strncpy(output_folder, file, slash_pos);*/

    if (use_this == IMAGEMAGICK)
        r = imagemagick_convert_single(file, output_path, (struct ImageMagickSettings){0});
    else if (use_this == FFMPEG)
        r = ffmpeg_convert_single(file, output_path, (struct FfmpegSettings){0});
    else if (use_this == PANDOC)
        r = pandoc_convert_single(file, output_path, (struct PandocSettings){.hard_line_breaks = true}); // For testing
    else if (use_this == LIBREOFFICE)
        r = libreoffice_convert_single(file, "/home/mustafa/Desktop", extension, (struct LibreofficeSettings) {});

    //free(output_path);
    //free(output_folder);
    return r;
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
    for (const auto& [key, val] : FORMAT_EXTENSIONS) {
        if (strcmp(val.c_str(), extension) != 0) continue;
        printf("'%s' == '%s'\n", val.c_str(), extension);
        if (LIBREOFFICE_CONVERSIONS.contains(key))
            res |= LIBREOFFICE;
        if (PANDOC_CONVERSIONS.contains(key))
            res |= PANDOC;
        if (FFMPEG_CONVERSIONS.contains(key))
            res |= FFMPEG;
        if (IMAGEMAGICK_CONVERSIONS.contains(key))
            res |= IMAGEMAGICK;
    }

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

void create_dropdown_list(std::vector<enum FileFormat> formats, char** out) {
    size_t ind = 0;
    for (auto f : formats)
        out[ind++] = (char*) FORMAT_EXTENSIONS.find(f)->second.c_str();
}
