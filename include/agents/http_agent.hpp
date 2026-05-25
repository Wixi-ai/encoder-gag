#ifndef HTTP_AGENT_HPP
#define HTTP_AGENT_HPP

#include <so_5/all.hpp>
#include <httplib.h>
#include <atomic>
#include <future>
#include <unordered_map>
#include "../messages.hpp"

class http_agent_t : public so_5::agent_t {
public:
    http_agent_t(context_t ctx, so_5::mbox_t db_mbox);

    void so_evt_start() override;
    void so_evt_finish() override;

private:
    so_5::mbox_t m_db_mbox;
    std::unique_ptr<httplib::Server> m_server;
    std::thread m_server_thread;
    int m_request_counter;
    std::atomic<int> m_request_id_counter;
    std::unordered_map<int, std::shared_ptr<std::promise<msg_get_records_response>>> m_pending_requests;
    std::mutex m_pending_mutex;
};

#endif
