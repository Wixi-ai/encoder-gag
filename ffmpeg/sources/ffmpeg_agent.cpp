#include "ffmpeg/ffmpeg_agent.hpp"
#include "utils/constants.hpp"
#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>
#include <random>
#include <mutex>

using json = nlohmann::json;

std::unordered_map<std::string, CachedVideoInfo> ffmpeg_agent_t::s_video_cache;
std::mutex ffmpeg_agent_t::s_cache_mutex;

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

static std::string get_ffprobe_path() {
    return "C:/Users/tungiia/ffmpeg-release/ffmpeg-8.1.1-essentials_build/bin/ffprobe.exe";
}

ffmpeg_agent_t::ffmpeg_agent_t(context_t ctx, so_5::mbox_t db_mbox)
    : so_5::agent_t{std::move(ctx)}, m_db_mbox{std::move(db_mbox)}, m_processed_count(0)
{
    LOG_INFO("FFMPEG", "FFmpeg agent created");
    m_ffprobe_path = get_ffprobe_path();
    LOG_INFO("FFMPEG", "ffprobe path: " + m_ffprobe_path);
}

void ffmpeg_agent_t::so_evt_start() {
    LOG_INFO("FFMPEG", "FFmpeg agent started");
}

void ffmpeg_agent_t::so_define_agent() {
    so_subscribe_self().event([this](const msg_process_video &msg) { handleProcessVideo(msg); });
    so_subscribe_self().event([this](const msg_create_vaa_blocks &msg) { handleCreateVaaBlocks(msg); });
}

msg_video_params ffmpeg_agent_t::analyzeVideo(const std::string &file_path, const std::string &record_id, int request_id) {
    msg_video_params params;
    params.request_id = request_id;
    params.record_id = record_id;
    params.success = false;

    std::string win_path = msys2_to_win_path(file_path);
    
    // Проверка кеша
    {
        std::lock_guard<std::mutex> lock(s_cache_mutex);
        auto it = s_video_cache.find(win_path);
        if (it != s_video_cache.end()) {
            LOG_INFO("FFMPEG", "Cache hit: " + win_path);
            params.codec = it->second.codec;
            params.width = it->second.width;
            params.height = it->second.height;
            params.duration = it->second.duration;
            params.success = true;
            std::cout << COLOR_GREEN << "[" << current_time() << "] [FFMPEG] ✓ Video from cache: " 
                      << params.width << "x" << params.height << " " << params.codec 
                      << " duration=" << params.duration << "s" << COLOR_RESET << std::endl;
            return params;
        }
    }

    LOG_INFO("FFMPEG", "Analyzing: " + win_path);

    FILE* f = fopen(win_path.c_str(), "rb");
    if (!f) {
        params.error_message = "File not found: " + win_path;
        LOG_ERROR("FFMPEG", params.error_message);
        return params;
    }
    fclose(f);

    std::string temp_file = "C:/Users/tungiia/ffprobe_out_" + random_string(8) + ".json";
    std::string cmd = "\"" + m_ffprobe_path + "\" -v quiet -print_format json -show_streams -show_format \"" + win_path + "\" > \"" + temp_file + "\" 2>&1";
    
    int ret = std::system(cmd.c_str());
    
    if (ret != 0) {
        params.error_message = "ffprobe failed with code: " + std::to_string(ret);
        LOG_ERROR("FFMPEG", params.error_message);
        return params;
    }
    
    std::ifstream ifs(temp_file);
    if (!ifs.is_open()) {
        params.error_message = "Cannot open temp file: " + temp_file;
        LOG_ERROR("FFMPEG", params.error_message);
        return params;
    }
    
    std::string output((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();
    std::remove(temp_file.c_str());
    
    if (output.empty()) {
        params.error_message = "ffprobe output empty";
        LOG_ERROR("FFMPEG", params.error_message);
        return params;
    }

    try {
        json data = json::parse(output);
        for (const auto& stream : data["streams"]) {
            if (stream.contains("codec_type") && stream["codec_type"] == "video") {
                if (stream.contains("codec_name")) params.codec = stream["codec_name"].get<std::string>();
                if (stream.contains("width")) params.width = stream["width"].get<int>();
                if (stream.contains("height")) params.height = stream["height"].get<int>();
                break;
            }
        }
        if (data.contains("format") && data["format"].contains("duration")) {
            params.duration = std::stod(data["format"]["duration"].get<std::string>());
        }
        params.success = (params.width > 0 && params.height > 0);
        
        if (params.success) {
            std::lock_guard<std::mutex> lock(s_cache_mutex);
            s_video_cache[win_path] = {params.codec, params.width, params.height, params.duration};
        }
        
        std::cout << COLOR_GREEN << "[" << current_time() << "] [FFMPEG] ✓ Video: " 
                  << params.width << "x" << params.height << " " << params.codec 
                  << " duration=" << params.duration << "s" << COLOR_RESET << std::endl;
    } catch (const std::exception& e) {
        params.error_message = "JSON parse error: " + std::string(e.what());
        LOG_ERROR("FFMPEG", params.error_message);
    }
    return params;
}

std::vector<VaaBlock> ffmpeg_agent_t::generateVaaBlocks(const std::string &record_id, int duration_seconds) {
    std::vector<VaaBlock> blocks;
    int block_count = std::max(1, (duration_seconds + 9) / 10);
    
    for (int i = 0; i < block_count; i++) {
        blocks.push_back({i, "video", i * 10 * 1000, 10000, "VIDEO_BLOCK_" + std::to_string(i)});
    }
    for (int i = 0; i < block_count; i++) {
        blocks.push_back({i + block_count, "audio", i * 10 * 1000, 10000, "AUDIO_BLOCK_" + std::to_string(i)});
    }
    std::cout << COLOR_GREEN << "[" << current_time() << "] [FFMPEG] ✓ Generated " << blocks.size() 
              << " VAA blocks for " << record_id.substr(0,8) << COLOR_RESET << std::endl;
    return blocks;
}

void ffmpeg_agent_t::handleProcessVideo(const msg_process_video &msg) {
    m_processed_count++;
    LOG_INFO("FFMPEG", "Processing #" + std::to_string(m_processed_count) + " | id=" + msg.record_id.substr(0,8));
    
    auto params = analyzeVideo(msg.file_path, msg.record_id, msg.request_id);
    so_5::send<msg_video_params>(msg.reply_to, params);
    
    if (params.success) {
        auto blocks = generateVaaBlocks(msg.record_id, static_cast<int>(params.duration));
        msg_save_vaa_blocks save_msg{msg.record_id, blocks, msg.request_id, msg.reply_to};
        so_5::send<msg_save_vaa_blocks>(m_db_mbox, save_msg);
    }
}

void ffmpeg_agent_t::handleCreateVaaBlocks(const msg_create_vaa_blocks &msg) {
    LOG_INFO("FFMPEG", "Saving " + std::to_string(msg.blocks.size()) + " VAA blocks");
    msg_save_vaa_blocks save_msg{msg.record_id, msg.blocks, msg.request_id, msg.reply_to};
    so_5::send<msg_save_vaa_blocks>(m_db_mbox, save_msg);
}
