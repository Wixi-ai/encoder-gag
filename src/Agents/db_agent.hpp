#ifndef DB_AGENT_HPP
#define DB_AGENT_HPP

#include <so_5/all.hpp>
#include <iostream>
#include "../../messages.hpp"
#include "../database.hpp"
#include "../colors.hpp"

class db_agent_t : public so_5::agent_t {
public:
    db_agent_t(context_t ctx)
        : so_5::agent_t{std::move(ctx)}
        , m_db("records.db")
    {}

    void so_define_agent() override {
        so_subscribe_self().event([this](const msg_hello& msg) {
            std::cout << COLOR_DB << "[DB Agent] Hello, " << msg.name << "!" << COLOR_RESET << std::endl;
        });
        
        so_subscribe_self().event([this](const msg_bye& msg) {
            std::cout << COLOR_DB << "[DB Agent] Bye, " << msg.name << "!" << COLOR_RESET << std::endl;
        });
        
        so_subscribe_self().event([this](const msg_create_record& msg) {
            std::cout << COLOR_DB << "\n========== DB AGENT ==========" << COLOR_RESET << std::endl;
            std::cout << COLOR_DB << "Message: msg_create_record" << COLOR_RESET << std::endl;
            std::cout << COLOR_DB << "  id: " << msg.id << COLOR_RESET << std::endl;
            std::cout << COLOR_DB << "  file_path: " << msg.file_path << COLOR_RESET << std::endl;
            std::cout << COLOR_DB << "  request_body: " << msg.request_body << COLOR_RESET << std::endl;
            
            if (m_db.saveRecord(msg.id, msg.file_path)) {
                std::cout << COLOR_DB << "  status: SAVED TO DATABASE" << COLOR_RESET << std::endl;
            } else {
                std::cout << COLOR_DB << "  status: SAVE FAILED" << COLOR_RESET << std::endl;
            }
            std::cout << COLOR_DB << "==============================\n" << COLOR_RESET << std::endl;
        });
        
        so_subscribe_self().event([this](const msg_get_records& msg) {
            std::cout << COLOR_DB << "[DB Agent] Get all records request" << COLOR_RESET << std::endl;
            auto records = m_db.getAllRecords();
            std::cout << COLOR_DB << "[DB Agent] Found " << records.size() << " records" << COLOR_RESET << std::endl;
        });
    }

private:
    Database m_db;
};

#endif
