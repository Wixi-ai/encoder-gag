#ifndef DB_AGENT_HPP
#define DB_AGENT_HPP

#include <so_5/all.hpp>
#include <iostream>
#include "../../messages.hpp"
#include "../database.hpp"
#include "../colors.hpp"
#include "../utils.hpp"

class db_agent_t : public so_5::agent_t {
public:
    db_agent_t(context_t ctx)
        : so_5::agent_t{std::move(ctx)}
        , m_db("records.db")
        , m_create_counter(0)
        , m_total_saved(0)
    {}

    void so_define_agent() override {
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
        
        // НОВЫЙ ОБРАБОТЧИК для GET запроса
        so_subscribe_self().event([this](const msg_get_records& msg) {
            std::cout << COLOR_DB << "[" << current_time() << "] [DB] GET RECORDS request" << COLOR_RESET << std::endl;
            
            auto records = m_db.getAllRecords();
            std::cout << COLOR_DB << "  found: " << records.size() << " records" << COLOR_RESET << std::endl;
            
            // Формируем ответ
            msg_get_records_response response;
            response.records = records;
            
            // Отправляем ответ обратно HTTP-агенту
            so_5::send<msg_get_records_response>(msg.reply_to, response);
        });
    }

private:
    Database m_db;
    int m_create_counter;
    int m_total_saved;
};

#endif
