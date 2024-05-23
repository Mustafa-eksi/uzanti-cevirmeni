#pragma once
#include "backend.hpp"

enum PandocEncoding {
    ASCII,
    UTF_8,
};

struct PandocSettings {
    enum PandocEncoding encoding;
    bool hard_line_breaks; // +hard_line_breaks
    //size_t framerate;
};

// const std::
const std::vector<enum FileFormat> pandoc_all = {
    EPUB, HTML, JSON, TEX, MD, ODT, ODF, PDF, DOCX, DOC, TXT,
};

const std::map<enum FileFormat, std::vector<enum FileFormat>> PANDOC_CONVERSIONS = {
    {EPUB, pandoc_all}, {HTML, pandoc_all}, {JSON, pandoc_all}, {ODT, pandoc_all},
    {ODF, pandoc_all}, {TEX, pandoc_all}, {DOCX, pandoc_all}, {TXT, pandoc_all},
    {MD, pandoc_all}, {ORG, pandoc_all}, {RST, pandoc_all}, {OPML, pandoc_all}
};
/*
#define PANDOC_INPUT_COUNT 10
const char* PANDOC_INPUT[PANDOC_INPUT_COUNT] = {
    "epub", "html", "json", "tex", "md", "odt", "opml", "org", "rst", NULL
};

#define PANDOC_OUTPUT_COUNT 14
const char* PANDOC_OUTPUT[PANDOC_OUTPUT_COUNT] = {
    "epub", "html", "json", "tex", "md", "odt", "odf", "opml", "txt", "pdf", "org", "rst", "docx", NULL
};*/

enum Result pandoc_convert_single(const char* in_path, const char* out_path, struct PandocSettings settings);
