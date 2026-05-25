#include "../../include/agents/db_agent.hpp"

db_agent_t::db_agent_t(context_t ctx)
    : so_5::agent_t{std::move(ctx)}
    , m_db("records.db")
    , m_create_counter(0)
    , m_total_saved(0)
{}

void db_agent_t::so_define_agent() {
    so_subscribe_self().event([this](const msg_hello& msg) {
        std::cout << COLOR_DB << "[" << current_time() << "] [DB] HELLO from: " << msg.name << COLOR_RESET << std::endl;
    });
    
    so_subscribe_self().event([this](const msg_bye& msg) {
        std::cout << COLOR_DB << "[" << current_time() << "] [DB] BYE to: " << msg.name << COLOR_RESET << std::endl;
    });
    
    so_subscribe_self().event([this](const msg_create_record& msg) {
        m_create_counter++;
        
        std::cout << COLOR_DB << "[" << current_time() << "] [DB] CREATE #" << m_create_counter << COLOR_RESET << std::endl;
        std::cout << COLOR_DB << "  id:   " << msg.id << COLOR_RESET << std::endl;
        std::cout << COLOR_DB << "  path: " << msg.file_path << COLOR_RESET << std::endl;
        std::cout << COLOR_DB << "  body: " << msg.request_body << COLOR_RESET << std::endl;
        
        if (m_db.saveRecord(msg.id, msg.file_path)) {
            m_total_saved++;
            std::cout << COLOR_DB << "  result: SAVED (total: " << m_total_saved << ")" << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_DB << "  result: FAILED" << COLOR_RESET << std::endl;
        }
    });
    
    so_subscribe_self().event([this](const msg_get_records& msg) {
        std::cout << COLOR_DB << "[" << current_time() << "] [DB] GET RECORDS request #" << msg.request_id << COLOR_RESET << std::endl;
        
        auto records = m_db.getAllRecords();
        std::cout << COLOR_DB << "  found: " << records.size() << " records" << COLOR_RESET << std::endl;
        
        msg_get_records_response response;
        response.request_id = msg.request_id;
        response.records = records;
        
        so_5::send<msg_get_records_response>(msg.reply_to, response);
    });
}
