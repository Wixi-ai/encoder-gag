#include "database/connection_pool.hpp"
#include <iostream>

ConnectionPool::ConnectionPool(const std::string& db_path, int pool_size)
    : db_path_(db_path), pool_size_(pool_size)
{
    for (int i = 0; i < pool_size_; ++i) {
        sqlite3* db = nullptr;
        if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
            connections_.push(db);
        } else {
            std::cerr << "[ConnectionPool] Failed to open connection " << i << std::endl;
        }
    }
    std::cout << "[ConnectionPool] Created " << connections_.size() << " connections" << std::endl;
}

ConnectionPool::~ConnectionPool() {
    while (!connections_.empty()) {
        sqlite3* db = connections_.front();
        connections_.pop();
        sqlite3_close(db);
    }
}

ConnectionPool::Connection::Connection(sqlite3* db, ConnectionPool* pool)
    : db_(db), pool_(pool) {}

ConnectionPool::Connection::~Connection() {
    if (db_) {
        pool_->release(db_);
    }
}

std::unique_ptr<ConnectionPool::Connection> ConnectionPool::acquire() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !connections_.empty(); });
    sqlite3* db = connections_.front();
    connections_.pop();
    return std::make_unique<Connection>(db, this);
}

void ConnectionPool::release(sqlite3* db) {
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.push(db);
    cv_.notify_one();
}
// Добавляем вывод в acquire
// В конце файла перед последней скобкой
