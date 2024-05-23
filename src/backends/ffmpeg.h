#pragma once
#include "backend.hpp"

struct FfmpegSettings {
    size_t framerate;
};

#define FFMPEG_COUNT 8
const char* FFMPEG_CONVERTABLE[FFMPEG_COUNT] = {
    "webm", "gif", "mp4", "mp3", "opus", "avi", "mov", NULL
};

const std::vector<enum FileFormat> ffmpeg_all = {
    WEBM, GIF, MP4, MP3, OPUS, AVI, MOV
};

const std::map<enum FileFormat, std::vector<enum FileFormat>> FFMPEG_CONVERSIONS = {
    {WEBM, ffmpeg_all}, {GIF, ffmpeg_all}, {MP4, ffmpeg_all}, {MP3, ffmpeg_all},
    {OPUS, ffmpeg_all}, {AVI, ffmpeg_all}, {MOV, ffmpeg_all},
};

enum Result ffmpeg_convert_single(const char* in_path, const char* out_path, struct FfmpegSettings settings);
