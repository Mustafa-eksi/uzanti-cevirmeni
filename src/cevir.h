#include <stdbool.h>
#include <stdio.h>
#include "backends/ffmpeg.c"
#include "backends/imagemagick.c"
#include "backends/pandoc.c"
#include "backends/libreoffice.c"

enum ConverterProgram {
    UNSUPPORTED     = 0x0000,
    FFMPEG          = 0x0001,
    IMAGEMAGICK     = 0x0010,
    PANDOC          = 0x0100,
    LIBREOFFICE     = 0x1000
};

const std::map<enum ConverterProgram, const char*> EXECUTABLE_NAMES = {
    {FFMPEG, "ffmpeg"},
    {IMAGEMAGICK, "magick"},
    {PANDOC, "pandoc"},
    {LIBREOFFICE, "libreoffice"},
};

typedef unsigned long size_t;

enum Result test_cli(const char* app);
int get_converter_from_extension(const char *extension);
char *get_extension(const char* file);
enum Result convert_helper(const char *file, const char *extension, enum ConverterProgram cp);
const char* const* create_dropdown_list(std::vector<enum FileFormat> formats);

// Image magick stuff


// Pandoc stuff

