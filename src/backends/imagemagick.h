#include "backend.hpp"

const char *IM_compression[] = {
    "Default", "B44A", "B44", "BZip", "DWAA", "DWAB", "DXT1",
    "DXT3", "DXT5", "BC7", "Fax", "Group4", "JBIG1",
    "JBIG2", "JPEG2000", "JPEG", "LERC", "LosslessJPEG",
    "Lossless", "LZMA", "LZW", "None", "Piz", "Pxr24", "RLE",
    "RunlengthEncoded", "WebP", "ZipS", "Zip", "Zstd", NULL,
};
struct ImageMagickSettings {
    size_t compress;
    bool grayscale, flip_horizontal, flip_vertical;
    int rotation, resize, quality;
};


const std::vector<enum FileFormat> imagemagick_all = {
    GIF, PNG, WEBP, JPG, JPEG, SVG, XCF
};

const std::map<enum FileFormat, std::vector<enum FileFormat>> IMAGEMAGICK_CONVERSIONS = {
    {GIF, imagemagick_all}, {PNG, imagemagick_all}, {WEBP, imagemagick_all}, {JPG, imagemagick_all},
    {SVG, imagemagick_all}, {XCF, imagemagick_all}
};

enum Result imagemagick_convert_single(const char* in_path, const char* out_path, struct ImageMagickSettings settings);
void imagemagick_set_settings_widget(GtkWidget* box);
struct ImageMagickSettings imagemagick_get_settings(GtkWidget* box);
