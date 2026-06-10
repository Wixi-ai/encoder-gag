#pragma once

#include <so_5/all.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "messages/messages.hpp"
#include "utils/colors.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"

struct CachedVideoInfo {
    std::string codec;
    int width;
    int height;
    double duration;
};

class ffmpeg_agent_t : public so_5::agent_t {
public:
    ffmpeg_agent_t(context_t ctx, so_5::mbox_t db_mbox);
    void so_define_agent() override;
    void so_evt_start() override;

private:
    void handleProcessVideo(const msg_process_video& msg);
    void handleCreateVaaBlocks(const msg_create_vaa_blocks& msg);
    
    msg_video_params analyzeVideo(const std::string& file_path, const std::string& record_id, int request_id);
    std::vector<VaaBlock> generateVaaBlocks(const std::string& record_id, int duration_seconds);

    so_5::mbox_t m_db_mbox;
    int m_processed_count;
    std::string m_ffprobe_path;
    
    static std::unordered_map<std::string, CachedVideoInfo> s_video_cache;
    static std::mutex s_cache_mutex;
};
