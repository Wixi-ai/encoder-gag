#pragma once

#include <so_5/all.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "../messages.hpp"
#include "../colors.hpp"
#include "../utils.hpp"
#include "../logger.hpp"

class ffmpeg_agent_t : public so_5::agent_t {
public:
    ffmpeg_agent_t(context_t ctx, so_5::mbox_t db_mbox);
    void so_define_agent() override;
    void so_evt_start() override;

private:
    void handleProcessVideo(const msg_process_video& msg);
    void handleCreateVaaBlocks(const msg_create_vaa_blocks& msg);
    
    // Определение параметров видео (заглушка / имитация)
    msg_video_params analyzeVideo(const std::string& file_path, const std::string& record_id, int request_id);
    
    // Создание VAA блоков (заглушка)
    std::vector<VaaBlock> generateVaaBlocks(const std::string& record_id, int duration);

    so_5::mbox_t m_db_mbox;
    int m_processed_count;
};
