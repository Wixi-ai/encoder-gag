#ifndef DB_AGENT_HPP
#define DB_AGENT_HPP

#include <so_5/all.hpp>
#include <iostream>
#include "../../messages.hpp"
#include "../database.hpp"

class db_agent_t : public so_5::agent_t {
public:
    db_agent_t(context_t ctx)
        : so_5::agent_t{std::move(ctx)}
        , m_db("records.db")
    {}

    void so_define_agent() override {
        so_subscribe_self().event([this](const msg_hello& msg) {
            std::cout << "[DB Agent] Hello, " << msg.name << "!" << std::endl;
        });
        
        so_subscribe_self().event([this](const msg_bye& msg) {
            std::cout << "[DB Agent] Bye, " << msg.name << "!" << std::endl;
        });
        
        so_subscribe_self().event([this](const msg_create_record& msg) {
            std::cout << "\n========== DB AGENT ==========" << std::endl;
            std::cout << "Message: msg_create_record" << std::endl;
            std::cout << "  id: " << msg.id << std::endl;
            std::cout << "  file_path: " << msg.file_path << std::endl;
            std::cout << "  request_body: " << msg.request_body << std::endl;
            
            if (m_db.saveRecord(msg.id, msg.file_path)) {
                std::cout << "  status: SAVED TO DATABASE" << std::endl;
            } else {
                std::cout << "  status: SAVE FAILED" << std::endl;
            }
            std::cout << "==============================\n" << std::endl;
        });
        
        so_subscribe_self().event([this](const msg_get_records& msg) {
            std::cout << "[DB Agent] Get all records request" << std::endl;
            auto records = m_db.getAllRecords();
            std::cout << "[DB Agent] Found " << records.size() << " records" << std::endl;
        });
    }

private:
    Database m_db;
};

#endif
