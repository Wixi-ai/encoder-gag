#include "../../include/agents/ffmpeg_agent.hpp"
#include "../../include/constants.hpp"

ffmpeg_agent_t::ffmpeg_agent_t(context_t ctx, so_5::mbox_t db_mbox)
    : so_5::agent_t{std::move(ctx)}
    , m_db_mbox{std::move(db_mbox)}
    , m_processed_count(0)
{
    LOG_INFO("FFMPEG", "FFmpeg agent created");
}

void ffmpeg_agent_t::so_evt_start() {
    LOG_INFO("FFMPEG", "FFmpeg agent started");
}

void ffmpeg_agent_t::so_define_agent() {
    so_subscribe_self().event([this](const msg_process_video& msg) {
        handleProcessVideo(msg);
    });
    
    so_subscribe_self().event([this](const msg_create_vaa_blocks& msg) {
        handleCreateVaaBlocks(msg);
    });
}

msg_video_params ffmpeg_agent_t::analyzeVideo(const std::string& file_path, const std::string& record_id, int request_id) {
    msg_video_params params;
    params.request_id = request_id;
    params.record_id = record_id;
    params.success = true;
    
    // Имитация анализа видео (заглушка)
    params.codec = "h264";
    params.width = 1920;
    params.height = 1080;
    params.duration = 60.0;
    params.error_message = "";
    
    std::cout << COLOR_MAGENTA << "[" << current_time() << "] [FFMPEG] Analyzed video: " 
              << file_path << " -> " << params.width << "x" << params.height 
              << " codec=" << params.codec << COLOR_RESET << std::endl;
    
    return params;
}

std::vector<VaaBlock> ffmpeg_agent_t::generateVaaBlocks(const std::string& record_id, int duration) {
    std::vector<VaaBlock> blocks;
    
    int block_count = (duration + 9) / 10;
    
    for (int i = 0; i < block_count; i++) {
        VaaBlock block;
        block.index = i;
        block.type = "video";
        block.pts = i * 10 * 1000;
        block.duration = 10000;
        block.data = "VIDEO_BLOCK_DATA_" + std::to_string(i);
        blocks.push_back(block);
    }
    
    for (int i = 0; i < block_count; i++) {
        VaaBlock block;
        block.index = i + block_count;
        block.type = "audio";
        block.pts = i * 10 * 1000;
        block.duration = 10000;
        block.data = "AUDIO_BLOCK_DATA_" + std::to_string(i);
        blocks.push_back(block);
    }
    
    std::cout << COLOR_MAGENTA << "[" << current_time() << "] [FFMPEG] Generated " << blocks.size() 
              << " VAA blocks for record " << record_id << COLOR_RESET << std::endl;
    
    return blocks;
}

void ffmpeg_agent_t::handleProcessVideo(const msg_process_video& msg) {
    m_processed_count++;
    LOG_INFO("FFMPEG", "Processing video #" + std::to_string(m_processed_count) 
             + " record_id=" + msg.record_id + " path=" + msg.file_path);
    
    auto params = analyzeVideo(msg.file_path, msg.record_id, msg.request_id);
    
    so_5::send<msg_video_params>(msg.reply_to, params);
    
    auto blocks = generateVaaBlocks(msg.record_id, static_cast<int>(params.duration));
    
    msg_create_vaa_blocks vaa_msg;
    vaa_msg.record_id = msg.record_id;
    vaa_msg.blocks = blocks;
    vaa_msg.request_id = msg.request_id;
    vaa_msg.reply_to = msg.reply_to;
    
    so_5::send<msg_create_vaa_blocks>(so_direct_mbox(), vaa_msg);
}

void ffmpeg_agent_t::handleCreateVaaBlocks(const msg_create_vaa_blocks& msg) {
    LOG_INFO("FFMPEG", "Saving " + std::to_string(msg.blocks.size()) + " VAA blocks to DB for record " + msg.record_id);
    
    msg_vaa_blocks_response response;
    response.request_id = msg.request_id;
    response.success = true;
    response.error_message = "";
    
    so_5::send<msg_vaa_blocks_response>(msg.reply_to, response);
}
