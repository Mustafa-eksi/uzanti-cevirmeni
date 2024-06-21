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

union ConvertSettings get_settings_helper(enum ConverterProgram cp, GtkWidget* sw) {
    if (cp == IMAGEMAGICK)
        return { .i = imagemagick_get_settings(sw)};
    else if (cp == FFMPEG)
        return {.f =ffmpeg_get_settings(sw)};
    else if (cp == PANDOC)
        return {.p = pandoc_get_settings(sw)};
    return {};
}

void convert_helper(std::string file, std::string extension, std::string output_folder, enum ConverterProgram cp, union ConvertSettings settings, enum Result *res)
{
    enum Result r = EXECUTABLE_NOT_FOUND;
    enum ConverterProgram use_this = cp;
    check_available(&use_this);
    std::string output_path;

    size_t dot_pos = 0;
    for (size_t i = 0; i < file.size(); i++)
        if (file[i] == '.')
            dot_pos = i;
    if (output_folder.empty()) {
        output_path.assign(file, 0, dot_pos+1);
        output_path.append(extension);
    } else {
        size_t slash_pos = 0;
        for (size_t i = 0; i < file.size(); i++)
            if (file[i] == '/')
                slash_pos = i;
        output_path.assign(output_folder);
        output_path.append(file.substr(slash_pos, dot_pos-slash_pos));
        output_path += ".";
        output_path.append(extension);
    }

    if (use_this == IMAGEMAGICK)
        r = imagemagick_convert_single(file.c_str(), output_path.c_str(), settings.i);
    else if (use_this == FFMPEG)
        r = ffmpeg_convert_single(file.c_str(), output_path.c_str(), settings.f);
    else if (use_this == PANDOC)
        r = pandoc_convert_single(file.c_str(), output_path.c_str(), settings.p); // For testing
    else if (use_this == LIBREOFFICE)
        r = libreoffice_convert_single(file.c_str(), output_folder.c_str(), extension.c_str(), settings.l);

    if (*res == SUCCESS)
        *res = r;
}

int get_converter_from_extension(const char *extension)
{
    int res = UNSUPPORTED;
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
