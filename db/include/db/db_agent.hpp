#pragma once

#include <so_5/all.hpp>
#include <iostream>
#include "messages/messages.hpp"
#include "database/database.hpp"
#include "utils/colors.hpp"
#include "utils/utils.hpp"

class db_agent_t : public so_5::agent_t {
public:
    db_agent_t(context_t ctx, const std::string& db_path = "records.db");
    void so_define_agent() override;

private:
    Database m_db;
    int m_create_counter;
    int m_total_saved;
};
