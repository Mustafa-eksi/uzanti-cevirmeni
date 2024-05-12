#include <stdio.h>

enum ConverterProgram {
    UNSUPPORTED = 0x000,
    FFMPEG = 0x001,
    IMAGEMAGICK = 0x010,
    PANDOC = 0x100,
};

const char* EXECUTABLE_NAMES[] = {
    [FFMPEG] = "ffmpeg",
    [IMAGEMAGICK] = "magick",
    [PANDOC] = "pandoc"
};

enum Result {
    SUCCESS,
    UNKNOWN_ERROR,
    EXECUTABLE_NOT_FOUND,
    NOT_ENOUGH_MEMORY,
    FILE_NOT_FOUND,
};

typedef unsigned long size_t;

enum Result test_cli(const char* app);
int get_converter_from_extension(const char *extension);
char *get_extension(const char* file);
enum Result convert_helper(const char *file, const char *extension, enum ConverterProgram cp);

// Image magick stuff

const char *IM_compression[] = {
    "Default", "B44A", "B44", "BZip", "DWAA", "DWAB", "DXT1",
    "DXT3", "DXT5", "BC7", "Fax", "Group4", "JBIG1",
    "JBIG2", "JPEG2000", "JPEG", "LERC", "LosslessJPEG",
    "Lossless", "LZMA", "LZW", "None", "Piz", "Pxr24", "RLE",
    "RunlengthEncoded", "WebP", "ZipS", "Zip", "Zstd", NULL,
};
struct ImageMagickSettings {
    size_t compress;
};

#define IMC_COUNT 7
const char* IMAGEMAGICK_CONVERTABLE[IMC_COUNT] = {
    "gif", "png", "webp", "jpg", "jpeg", "svg", NULL
};

enum Result imagemagick_convert_single(const char* in_path, const char* out_path, struct ImageMagickSettings settings);

// Ffmpeg stuff

struct FfmpegSettings {
    size_t framerate;
};

#define FFMPEG_COUNT 8
const char* FFMPEG_CONVERTABLE[FFMPEG_COUNT] = {
    "webm", "gif", "mp4", "mp3", "opus", "avi", "mov", NULL
};

enum Result ffmpeg_convert_single(const char* in_path, const char* out_path, struct FfmpegSettings settings);

// Ffmpeg stuff

enum PandocEncoding {
    ASCII,
    UTF_8,
};

struct PandocSettings {
    enum PandocEncoding encoding;
    //size_t framerate;
};

#define PANDOC_INPUT_COUNT 10
const char* PANDOC_INPUT[PANDOC_INPUT_COUNT] = {
    "epub", "html", "json", "tex", "md", "odt", "opml", "org", "rst", NULL
};

#define PANDOC_OUTPUT_COUNT 13
const char* PANDOC_OUTPUT[PANDOC_OUTPUT_COUNT] = {
    "epub", "html", "json", "tex", "md", "odt", "odf", "opml", "txt", "pdf", "org", "rst", NULL
};

enum Result pandoc_convert_single(const char* in_path, const char* out_path, struct PandocSettings settings);
