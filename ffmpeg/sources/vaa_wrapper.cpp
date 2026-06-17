#include "ffmpeg/vaa_wrapper.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>
#include <random>
#include <iostream>

using json = nlohmann::json;

namespace vaa_wrapper {

static std::string get_ffprobe_path() {
    const char* path = std::getenv("FFPROBE_PATH");
    if (path && path[0] != '\0') return std::string(path);
    
    std::vector<std::string> paths = {
        "ffprobe",
        "/usr/bin/ffprobe",
        "/usr/local/bin/ffprobe",
        "C:/tools/ffprobe.exe",
        "C:/ffmpeg/bin/ffprobe.exe"
    };
    
    for (const auto& p : paths) {
        FILE* f = fopen(p.c_str(), "rb");
        if (f) {
            fclose(f);
            return p;
        }
    }
    return "ffprobe";
}

static std::string random_string(int len) {
    const char* chars = "0123456789abcdef";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::string result;
    for (int i = 0; i < len; ++i) {
        result += chars[dis(gen)];
    }
    return result;
}

static std::string msys2_to_win_path(const std::string& path) {
    if (path.empty()) return path;
    if (path.length() > 2 && path[1] == ':') return path;
    if (path.length() > 2 && path[0] == '/' && isalpha(path[1])) {
        std::string drive = path.substr(1, 1);
        drive[0] = toupper(drive[0]);
        std::string rest = path.substr(2);
        if (!rest.empty() && rest[0] != '/') rest = "/" + rest;
        return drive + ":" + rest;
    }
    return path;
}

VaaBlockData createBlock(int index, const std::string& type, int64_t pts, int64_t duration, const std::string& data) {
    VaaBlockData block;
    block.index = index;
    block.type = type;
    block.pts = pts;
    block.duration = duration;
    block.data = data.empty() ? (type == "video" ? "VIDEO_BLOCK_" + std::to_string(index) : "AUDIO_BLOCK_" + std::to_string(index)) : data;
    return block;
}

VaaBlock toMessageBlock(const VaaBlockData& block) {
    VaaBlock result;
    result.index = block.index;
    result.type = block.type;
    result.pts = block.pts;
    result.duration = block.duration;
    result.data = block.data;
    return result;
}

std::vector<VaaBlockData> generateBlocksForFile(
    const std::string& file_path,
    const std::string& record_id,
    int duration_seconds,
    int& block_counter)
{
    std::vector<VaaBlockData> blocks;
    int block_count = std::max(1, (duration_seconds + 9) / 10);
    
    // Генерируем видео блоки
    for (int i = 0; i < block_count; i++) {
        blocks.push_back(createBlock(block_counter++, "video", i * 10 * 1000, 10000));
    }
    
    // Генерируем аудио блоки
    for (int i = 0; i < block_count; i++) {
        blocks.push_back(createBlock(block_counter++, "audio", i * 10 * 1000, 10000));
    }
    
    return blocks;
}

std::vector<VaaBlock> toMessageBlocks(const std::vector<VaaBlockData>& blocks) {
    std::vector<VaaBlock> result;
    for (const auto& block : blocks) {
        result.push_back(toMessageBlock(block));
    }
    return result;
}

bool isValidBlock(const VaaBlockData& block) {
    return block.index >= 0 &&
           (block.type == "video" || block.type == "audio") &&
           block.pts >= 0 &&
           block.duration > 0;
}

std::string encodeBlockData(const std::string& data) {
    return data;
}

} // namespace vaa_wrapper
