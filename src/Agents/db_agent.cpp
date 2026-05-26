#include "../../include/agents/db_agent.hpp"

db_agent_t::db_agent_t(context_t ctx)
    : so_5::agent_t{std::move(ctx)}
    , m_db("records.db")
    , m_create_counter(0)
    , m_total_saved(0)
{}

void db_agent_t::so_define_agent() {
    // POST: создание записи
    so_subscribe_self().event([this](const msg_create_record& msg) {
        m_create_counter++;
        std::cout << COLOR_DB << "[" << current_time() << "] [DB] CREATE #" << m_create_counter << COLOR_RESET << std::endl;
        std::cout << COLOR_DB << "  id:   " << msg.id << COLOR_RESET << std::endl;
        std::cout << COLOR_DB << "  path: " << msg.file_path << COLOR_RESET << std::endl;
        std::cout << COLOR_DB << "  body: " << msg.request_body << COLOR_RESET << std::endl;

        bool success = m_db.saveRecord(msg.id, msg.file_path);
        
        msg_create_response response;
        response.id = msg.id;
        response.success = success;
        response.error_message = success ? "" : "Failed to save to database";
        so_5::send<msg_create_response>(msg.reply_to, response);
        
        if (success) {
            m_total_saved++;
            std::cout << COLOR_DB << "  result: SAVED (total: " << m_total_saved << ")" << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_DB << "  result: FAILED" << COLOR_RESET << std::endl;
        }
    });

    // GET ALL: получение всех записей
    so_subscribe_self().event([this](const msg_get_records& msg) {
        std::cout << COLOR_DB << "[" << current_time() << "] [DB] GET ALL RECORDS #" << msg.request_id << COLOR_RESET << std::endl;

        auto records = m_db.getAllRecords();
        std::cout << COLOR_DB << "  found: " << records.size() << " records" << COLOR_RESET << std::endl;

        msg_get_records_response response;
        response.request_id = msg.request_id;
        response.records = records;
        so_5::send<msg_get_records_response>(msg.reply_to, response);
    });
    
    // GET BY ID: получение одной записи
    so_subscribe_self().event([this](const msg_get_record_by_id& msg) {
        std::cout << COLOR_DB << "[" << current_time() << "] [DB] GET RECORD BY ID #" << msg.request_id << " id=" << msg.id << COLOR_RESET << std::endl;

        auto [found, id, path, created] = m_db.getRecordById(msg.id);
        
        msg_get_record_by_id_response response;
        response.request_id = msg.request_id;
        response.found = found;
        response.id = id;
        response.file_path = path;
        response.created_at = created;
        so_5::send<msg_get_record_by_id_response>(msg.reply_to, response);
        
        if (found) {
            std::cout << COLOR_DB << "  found: " << id << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_DB << "  not found" << COLOR_RESET << std::endl;
        }
    });
    
    // DELETE: удаление записи
    so_subscribe_self().event([this](const msg_delete_record_by_id& msg) {
        std::cout << COLOR_DB << "[" << current_time() << "] [DB] DELETE RECORD #" << msg.request_id << " id=" << msg.id << COLOR_RESET << std::endl;

        bool success = m_db.deleteRecordById(msg.id);
        
        msg_delete_record_by_id_response response;
        response.request_id = msg.request_id;
        response.success = success;
        response.error_message = success ? "" : "Record not found";
        so_5::send<msg_delete_record_by_id_response>(msg.reply_to, response);
        
        if (success) {
            std::cout << COLOR_DB << "  deleted" << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_DB << "  delete failed (not found)" << COLOR_RESET << std::endl;
        }
    });
}
