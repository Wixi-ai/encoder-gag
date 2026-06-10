#pragma once

#include <sqlite3.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

class ConnectionPool {
public:
    ConnectionPool(const std::string& db_path, int pool_size = 5);
    ~ConnectionPool();
    
    class Connection {
    public:
        Connection(sqlite3* db, ConnectionPool* pool);
        ~Connection();
        sqlite3* get() { return db_; }
    private:
        sqlite3* db_;
        ConnectionPool* pool_;
    };
    
    std::unique_ptr<Connection> acquire();
    
private:
    friend class Connection;
    void release(sqlite3* db);
    
    std::string db_path_;
    int pool_size_;
    std::queue<sqlite3*> connections_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
