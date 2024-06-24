#include <stdbool.h>
#include <stdio.h>
#include "backends/ffmpeg.cpp"
#include "backends/ffmpeg.hpp"
#include "backends/imagemagick.cpp"
#include "backends/pandoc.cpp"
#include "backends/libreoffice.cpp"

enum ConverterProgram {
    UNSUPPORTED     = 0x0000,
    FFMPEG          = 0x0001,
    IMAGEMAGICK     = 0x0010,
    PANDOC          = 0x0100,
    LIBREOFFICE     = 0x1000
};

const std::map<enum ConverterProgram, const char*> EXECUTABLE_NAMES = {
    {FFMPEG, "ffmpeg"},
    {IMAGEMAGICK, "convert"},
    {PANDOC, "pandoc"},
    {LIBREOFFICE, "libreoffice"},
};

typedef unsigned long size_t;

union ConvertSettings {
    FfmpegSettings f;
    ImageMagickSettings i;
    PandocSettings p;
    LibreofficeSettings l;
};

enum Result test_cli(const char* app);
int get_converter_from_extension(const char *extension);
char *get_extension(const char* file);
union ConvertSettings get_settings_helper(enum ConverterProgram cp, GtkWidget* sw);
void convert_helper_threaded(struct CHparameters *p);
void convert_helper(std::string file, std::string extension, std::string output_folder, enum ConverterProgram cp, union ConvertSettings setting, enum Result *res);

// Image magick stuff


// Pandoc stuff

